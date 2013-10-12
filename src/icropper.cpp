#include "icropper.h"
#include <cassert>
#include <algorithm>
#include <math.h>
#include "tinyxml2.h"
#include "CUtils.h"

namespace icropper {

//////////////////////////////////////////////////////////////////////////

//
// traversal FreeImage Pixels
//
template<typename F>
void traversal_pixels(fipImage* image, F handler)
{
	int bytespp = image->getLine() / image->getWidth();

	for(unsigned int y = 0; y < image->getHeight(); y++) 
	{
		BYTE *bits = image->getScanLine(y);
		for(unsigned int x = 0; x < image->getWidth(); x++) 
		{
			// handle pixel
			// y is reversed
			handler(bits, x, image->getHeight() - y - 1);

			// jump to next pixel
			bits += bytespp;
		}
	}
}

inline unsigned int next_power_of_two(unsigned int x)
{
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return x + 1;
}

//////////////////////////////////////////////////////////////////////////

Image::Image()
: m_raw_image(NULL)
, m_root_rect(NULL)
{

}

Image::~Image()
{
	delete m_root_rect;
	delete m_raw_image;
}

Image* Image::createWithFileName(const char* filename, const char* path /*= NULL*/)
{
	std::string fullpath;
	if (path)
	{
		fullpath = path;
		fullpath += "/";
	}
	fullpath += filename;
	
	fipImage* fimage = new fipImage;
	if (fimage->load(fullpath.c_str()))
	{
		Image* image = new Image;
		image->m_filename = filename;
		image->m_raw_image = fimage;
		image->m_raw_size = Size(fimage->getWidth(), fimage->getHeight());

		return image;
	}
	delete fimage;
	return NULL;
}

bool Image::crop()
{
	// already cropped
	if (m_root_rect)
		return false;

	if (getOptions().is_scaled())
	{
		Size scaled_size = Size(
			(int)(m_raw_size.width * getOptions().scale_ratio + 0.5f), 
			(int)(m_raw_size.height * getOptions().scale_ratio + 0.5f));

		m_raw_image->rescale(scaled_size.width, scaled_size.height, FILTER_BOX);
		m_raw_size = scaled_size;
	}

	m_root_rect = new ImageRect(new fipImage(*m_raw_image), this, NULL, Position(0, 0));

	m_root_rect->cropWithFixedSize(getOptions().block_size);
	m_root_rect->getLeafRects(m_rects);
	return true;
}

//////////////////////////////////////////////////////////////////////////


ImageRect::ImageRect(fipImage* image, Image* image_info, ImageRect* parent, Position rel_pos /*= Position(0, 0)*/)
: m_image(image)
, m_image_info(image_info)
, m_parent(parent) 
, m_is_rotated(false)
{
	assert(image && "Image must not NULL!");
	m_zone.size = Size(image->getWidth(), image->getHeight());
	//_initCropUnusedBorder();
	m_zone.pos.add(rel_pos);
}

ImageRect::~ImageRect()
{
	for (auto child : m_children)
	{
		delete child;
	}
	m_children.clear();

	assert(m_image);
	delete m_image;
	m_image = NULL;
}

float ImageRect::getSolidPixelsRatio()
{
	assert(m_image && "Error: Image is NULL!");

	unsigned int used = 0, unused = 0, opacity = 0, total = 0;
	_getPixelCount(used, unused, opacity, total);

	return used * 1.0f / total;
}

float ImageRect::getOpacityPixelsRatio()
{
	assert(m_image && "Error: Image is NULL!");

	unsigned int used = 0, unused = 0, opacity = 0, total = 0;
	_getPixelCount(used, unused, opacity, total);

	return opacity * 1.0f / total;
}

float ImageRect::getSavedAreaRatio()
{
	RectList rects;
	getLeafRects(rects);

	if ( rects.empty() )
		return 0.0f;

	unsigned int sub_rect_areas = 0;
	for (auto rect: rects)
	{
		sub_rect_areas += rect->getRelativeZone().size.area();
	}

	assert(sub_rect_areas <= getRelativeZone().size.area());

	return 1.0f - 1.0f * sub_rect_areas / getRelativeZone().size.area();
}

bool ImageRect::isFullTransparent()
{
	assert(m_image && "Image is NULL!");

	// traversal pixels
	if (m_zone.isZero())
		return true;

	unsigned int used = 0, unused = 0, opacity = 0, total = 0;
	_getPixelCount(used, unused, opacity, total);

	return used == 0;
}

void ImageRect::rotate()
{
	assert(isLeafRect() && "Error: Must Rotate a Leaf Rect!");
	m_image->rotate(getImageInfo()->getOptions().rotate_degress);
	m_is_rotated = true;
}

int ImageRect::getDepth()
{
	int depth = 0;
	ImageRect* parent = getParent();
	while (parent)
	{
		depth++;
		parent = parent->getParent();
	}

	return depth;
}

void ImageRect::getLeafRects(RectList& rects)
{
	if (isLeafRect())
	{
		rects.push_back(this);
	}
	else
	{
		for (auto child: m_children)
		{
			child->getLeafRects(rects);
		}
	}
}

Zone ImageRect::getAbsZone()
{
	if (isRootRect())
	{
		return m_zone;
	}

	Zone abs_zone = m_zone;
	Zone parent_zone = getParent()->getAbsZone();
	abs_zone.pos.add(parent_zone.pos);

	return abs_zone;
}

void ImageRect::cropWithFixedSize(Size block_size)
{
	assert(isLeafRect() && "Error: Can Only Crop a Leaf Rect!");
	assert(!isRotated() && "Error: Can not Crop a Rotated Rect!");

	for (int y = 0; y < m_zone.size.height; y+=block_size.height)
	{
		for (int x = 0; x < m_zone.size.width; x+=block_size.width)
		{
			int width = min(block_size.width, m_zone.size.width - x);
			int height = min(block_size.height, m_zone.size.height - y);

			fipImage* sub_img = new fipImage(FIT_BITMAP, width, height);
			m_image->copySubImage(*sub_img, x, y, x+width, y+height);

			ImageRect* sub_rect = new ImageRect(sub_img, m_image_info, this, Position(x, y));
			if (sub_rect->isFullTransparent()) // invalid rect
			{
				delete sub_rect;
				continue;
			}

			sub_rect->cropHalving();
			m_children.push_back(sub_rect);
		}
	}
}

void ImageRect::cropHalving()
{
	assert(!isRotated() && "Error: Can not Crop a Rotated Rect!");

	// no need to crop
	if (getDepth() >= getImageInfo()->getOptions().crop_depth
		|| getSolidPixelsRatio() >= getImageInfo()->getOptions().crop_usage_ratio
		|| m_zone.size.area() <= getImageInfo()->getOptions().min_area)
		return;

	// crop half the size
	Size crop_size((m_zone.size.width+1) >> 1, (m_zone.size.height+1) >> 1);
	cropWithFixedSize(crop_size);
}

void ImageRect::_initCropUnusedBorder()
{
	Position left_top;
	Position bottom_right;
	left_top.x = m_image->getWidth() - 1;
	left_top.y = m_image->getHeight() - 1;
	bottom_right.x = 0;
	bottom_right.y = 0;

	traversal_pixels(m_image,
			[&](BYTE* bits, unsigned int x, unsigned int y) 
			{
				if (bits[FI_RGBA_ALPHA])
				{
					left_top.x = min(left_top.x, (int)x);
					left_top.y = min(left_top.y, (int)y);
					bottom_right.x = max(bottom_right.x, (int)x);
					bottom_right.y = max(bottom_right.y, (int)y);
				}
			}
		);

	m_zone.pos = left_top;
	m_zone.size = Size(bottom_right.x - left_top.x + 1, bottom_right.y - left_top.y + 1);

	// if have unused border
	if (left_top == Position(0,0) 
		&& bottom_right.x == m_image->getWidth() - 1
		&& bottom_right.y == m_image->getHeight() - 1)
	{
		// do nothing
	}
	else
	{
		// crop border
		BOOL retv = m_image->crop(left_top.x, left_top.y, bottom_right.x + 1, bottom_right.y + 1);
		assert(retv && "Crop Border Failed!");
	}
	
}

void ImageRect::_getPixelCount(unsigned int& used, unsigned int& unused, unsigned int& opacity, unsigned int& total)
{
	assert(m_image && "Image is NULL!");
	used = 0;
	unused = 0;
	opacity = 0;
	total = m_image->getHeight() * m_image->getWidth();

	traversal_pixels(m_image,
			[&](BYTE* bits, unsigned int, unsigned int) 
			{
				if (/*bits[FI_RGBA_RED] || bits[FI_RGBA_GREEN] || bits[FI_RGBA_BLUE] || */bits[FI_RGBA_ALPHA])
				{
					used++;
					if (bits[FI_RGBA_ALPHA] == 255)
						opacity++;
				}
				else
				{
					unused++;
				}
			}
		);
	
	assert(total == (used + unused) && "Error: total != (used + unused) !!");
}


//////////////////////////////////////////////////////////////////////////

Compositor::Compositor()
{
}

Compositor::~Compositor()
{
	_clearSlices();
	_clearTextures();
	_clearImages();
}

void Compositor::reset()
{
	m_rects.clear();
	_clearSlices();
	_clearTextures();
	_clearImages();
}

bool Compositor::addImage(Image* image)
{
	assert(image && "Image Must Not NULL!");
	if (image->getRects().empty()) // maybe not cropped yet!
		return false;
	if (m_image_slices.find(image) != m_image_slices.end()) // image already add to compositor 
		return false;
	m_image_slices.insert(ImageSlices::value_type(image, new SliceArray));
	m_rects.insert(m_rects.end(), image->getRects().begin(), image->getRects().end());

	if (m_file_prefix.empty())
	{
		m_file_prefix = image->getFileName();
		m_file_prefix = m_file_prefix.substr(0, m_file_prefix.find_last_of('.'));
	}

	return true;
}

bool Compositor::composit()
{
	// clear context
	_clearSlices();
	_clearTextures();

	// empty
	if (m_rects.empty())
		return false;

	// sort rects
	std::sort(m_rects.begin(), m_rects.end(), 
			[](ImageRect* lhs, ImageRect* rhs)
			{
				return lhs->getRelativeZone().size.area() > rhs->getRelativeZone().size.area();
			}
		);

	// insert rects
	for (auto it = m_rects.begin(); it != m_rects.end(); it++)
	{
		if (!_insertRect(*it))
		{
			// insert failed! create new texture!
			int texture_width = _getMostSuitableWidth(it, m_rects.end());
			texture_width = texture_width > getOptions().max_texture_size 
				? getOptions().max_texture_size : texture_width;
			_createTexture(texture_width);

			if (!_insertRect(*it))
			{
				assert(false&&"Error!");
				return false;
			}
		}
	}

	// final: print to textures
	return _printToTextures();
}

float Compositor::getUsageRatio()
{
	if (m_textures.empty())
		return 0.0f;

	unsigned int slice_area_total = 0;
	unsigned int texture_area_total = 0;
	for (size_t i = 0; i < m_textures.size(); i++)
	{
		texture_area_total += m_textures[i]->getWidth() * m_textures[i]->getHeight();
		for (auto slice: *m_texture_slices[i])
		{
			slice_area_total += slice->zone.size.area();
		}
	}

	assert(texture_area_total > 0 && "Error: Invalid Textures!");
	return 1.0f * slice_area_total / texture_area_total;
}

float Compositor::getUsageRatioForTexture(int idx)
{
	if (idx >= (int)m_textures.size())
		return 0.0f;

	unsigned int area_total = 0;
	SliceArray& slices = *m_texture_slices[idx];
	for (auto slice: slices)
	{
		area_total += slice->zone.size.area();
	}

	return 1.0f * area_total / (m_textures[idx]->getWidth() * m_textures[idx]->getHeight());
}

bool Compositor::saveTextures(const char* path /*= NULL*/)
{
	assert(m_file_prefix.c_str() && "Error: Must Specify a File Prefix!");
	static char buf[256];
	std::string fullpath;
	if (path)
	{
		fullpath = path;
		fullpath += "/";
	}

	int idx = 0;
	for (auto texture: m_textures)
	{
		sprintf_s(buf, 256, ICROPPER_FILE_TEXTURE_FORMAT, (fullpath+m_file_prefix).c_str(), idx, ICROPPER_DEFAULT_RAW_TEXTURE_SUFFIX);
		CUtils::builddir(buf);
		BOOL retv = texture->save(buf);
		if (retv == FALSE)
			return false;
		idx++;
	}
	return true;
}

bool Compositor::saveToXML(const char* path /*= NULL*/)
{
	assert(m_file_prefix.c_str() && "Error: Must Specify a File Prefix!");
	static char buf[256];

	std::string fullpath;
	if (path)
	{
		fullpath = path;
		fullpath += "/";
	}
	
	namespace tx2 = tinyxml2;

	tx2::XMLDocument doc;
	tx2::XMLDeclaration* decl = doc.NewDeclaration();
	doc.InsertFirstChild(decl);
	doc.InsertEndChild(doc.NewComment(ICROPPER_FILE_COMMENT));
	tx2::XMLElement* root = doc.NewElement(ICROPPER_FILE_ROOT_NODE);
	doc.InsertEndChild(root);
	root->SetAttribute("version", ICROPPER_VERSION);

	// 1.infos
	tx2::XMLElement* infos = doc.NewElement(ICROPPER_FILE_INFO_ROOT_NODE);
	root->InsertEndChild(infos);

	{
		tx2::XMLElement* info = doc.NewElement(ICROPPER_FILE_INFO_NODE);
		infos->InsertEndChild(info);
		sprintf_s(buf, 256, "%d%%", int(getUsageRatio() * 100 + 0.5f));
		info->SetAttribute("usage", buf);
	}

	// 2.textures
	tx2::XMLElement* textures = doc.NewElement(ICROPPER_FILE_TEXTURES_NODE);
	root->InsertEndChild(textures);
	textures->SetAttribute("size", m_textures.size());

	for (size_t i = 0; i < m_textures.size(); i++)
	{
		tx2::XMLElement* texture_element = doc.NewElement(ICROPPER_FILE_TEXTURE_NODE);
		textures->InsertEndChild(texture_element);

		texture_element->SetAttribute("id", i);
		sprintf_s(buf, 256, "ICROPPER_FILE_TEXTURE_FORMAT", (/*fullpath + */m_file_prefix).c_str(), i, getOptions().texture_file_suffix.c_str());
		texture_element->SetAttribute("file", buf);
		sprintf_s(buf, 256, "%d%%", int(getUsageRatioForTexture(i) * 100 + 0.5f));
		texture_element->SetAttribute("usage", buf);
	}

	// 3.images
	tx2::XMLElement* images = doc.NewElement(ICROPPER_FILE_IMAGES_NODE);
	root->InsertEndChild(images);
	images->SetAttribute("size", m_image_slices.size());
	images->SetAttribute("axis_y", getOptions().flip_axis_y ? "ascent" : "descent");

	for (auto image: m_image_slices)
	{
		Image* image_info = image.first;
		tx2::XMLElement* image_node = doc.NewElement(ICROPPER_FILE_IMAGE_NODE);
		images->InsertEndChild(image_node);
		image_node->SetAttribute("name", image_info->getFileName().c_str());
		image_node->SetAttribute("width", image_info->getSize().width);
		image_node->SetAttribute("height", image_info->getSize().height);
		if (image_info->getOptions().is_scaled())
			image_node->SetAttribute("scale", image_info->getOptions().scale_ratio);
		size_t slice_size = 0;
		for (auto slice: *image.second)
		{
			tx2::XMLElement* rect_node = doc.NewElement(ICROPPER_FILE_RECT_NODE);
			image_node->InsertEndChild(rect_node);

			rect_node->SetAttribute("id", slice->texture_id);
			rect_node->SetAttribute("texture_x", slice->zone.pos.x);
			rect_node->SetAttribute("texture_y", slice->zone.pos.y);
			Zone abs_zone = slice->rect->getAbsZone();
			rect_node->SetAttribute("image_x", abs_zone.pos.x);
			if (getOptions().flip_axis_y)
				rect_node->SetAttribute("image_y",  image_info->getSize().height - (abs_zone.pos.y + abs_zone.size.height) - 1);
			else
				rect_node->SetAttribute("image_y", abs_zone.pos.y);
			rect_node->SetAttribute("width", abs_zone.size.width);
			rect_node->SetAttribute("height", abs_zone.size.height);
			if (slice->rect->isRotated())
				rect_node->SetAttribute("rotate", slice->rect->isRotated());
			slice_size++;
		}
		image_node->SetAttribute("size", slice_size);
	}

	// 4.actions
	tx2::XMLElement* actions = doc.NewElement(ICROPPER_FILE_ACTIONS_NODE);
	root->InsertEndChild(actions);

	// save to file
	fullpath += m_file_prefix + "." + getOptions().xml_file_suffix;
	CUtils::builddir(fullpath.c_str());
	return tx2::XML_SUCCESS == doc.SaveFile(fullpath.c_str());
}

bool Compositor::saveToBin(const char* path /*= NULL*/)
{
	CUtils::builddir(path);
	return true;
}

void Compositor::_clearTextures()
{
	for (auto texture_slice: m_texture_slices)
	{
		delete texture_slice;
	}
	m_texture_slices.clear();
	for (auto texture: m_textures)
	{
		delete texture;
	}
	m_textures.clear();
}

void Compositor::_clearImages()
{
	for (auto image_slice: m_image_slices)
	{
		delete image_slice.second;
	}
	m_image_slices.clear();
}

void Compositor::_clearSlices()
{
	for (auto slice: m_used_slices)
	{
		delete slice;
	}
	for (auto slice: m_free_slices)
	{
		delete slice;
	}
	m_used_slices.clear();
	m_free_slices.clear();
}

int Compositor::_getMostSuitableWidth(RectArray::iterator begin, RectArray::iterator end)
{
	int rects_area = 0;
	for (auto it = begin; it != end; it++)
	{
		rects_area += (*it)->getRelativeZone().size.area();
	}

	Size block_size = (*begin)->getImageInfo()->getOptions().block_size;

	int ewidth = (int)(sqrt(1.0f * rects_area * block_size.width / block_size.height) + 0.5f);
	ewidth = ewidth + (ewidth / block_size.width + 1) * getOptions().texture_padding;
	ewidth = block_size.width >= block_size.height ? ewidth : ewidth * block_size.height / block_size.width;
	ewidth = max(max(127, ewidth), max(block_size.width, block_size.height));
	ewidth = next_power_of_two(ewidth);

	float eratio = 1.0f * rects_area / (ewidth * ewidth);
	float threshold = 0.0f;
	if (!getOptions().force_single_texture)
	{
		if (ewidth > 512)
			threshold = 0.6f;
		else if (ewidth > 128)
			threshold = 0.4f;
		else
			threshold = 0.0f;
	}

	ewidth = eratio > threshold ? ewidth : (ewidth >> 1);
	return ewidth;
}

bool Compositor::_insertRect(ImageRect* rect)
{
	Size rect_size = rect->getSize();
	Size rect_size_rotate(rect_size.height, rect_size.width);
	Slice* slice = _findFreeSlice(rect_size);
	if (!slice)
	{
		if (getOptions().enable_rotate)
			slice = _findFreeSlice(rect_size_rotate);
		if (!slice)
		{
			return false;
		}
		else
		{
			rect->rotate();
		}
	}

	slice->rect = rect;
	m_used_slices.push_back(slice);

	return true;
}

void Compositor::_createTexture(int texture_width)
{
	int texture_id = m_textures.size();

	Slice* slice = new Slice(texture_id);
	slice->zone.pos = Position(getOptions().texture_padding, getOptions().texture_padding);
	slice->zone.size = Size(
		texture_width - getOptions().texture_padding*2, 
		texture_width - getOptions().texture_padding*2);
	m_free_slices.push_back(slice);

	fipImage* image = new fipImage(FIT_BITMAP, texture_width, texture_width, 32);
	m_textures.push_back(image);
	m_texture_slices.push_back(new SliceArray);
	assert( m_textures.size() == m_texture_slices.size());
}

Compositor::Slice* Compositor::_findFreeSlice(Size rect_size)
{
	Slice* slice = NULL;
	for (auto it = m_free_slices.begin(); it != m_free_slices.end(); it++)
	{
		if ((*it)->zone.contains(rect_size))
		{
			slice = *it;
			m_free_slices.erase(it);
			break;
		}
	}

	if (slice)
	{
		int right = slice->zone.pos.x + rect_size.width;
		int bottom = slice->zone.pos.y + rect_size.height;
		int bottom_width = slice->zone.size.width;
		int right_height = slice->zone.size.height;
		if (rect_size.width >= rect_size.height)
		{
			bottom_width = rect_size.width;
		}
		else
		{
			right_height = rect_size.height;
		}
		
		Slice* bottom_slice = new Slice(slice->texture_id);
		bottom_slice->zone.pos = Position(slice->zone.pos.x, bottom + getOptions().texture_padding);
		bottom_slice->zone.size = Size(
			bottom_width, 
			slice->zone.size.height - rect_size.height - getOptions().texture_padding);

		if (bottom_slice->zone.size.height > getOptions().texture_padding)
			m_free_slices.push_back(bottom_slice);

		Slice* right_slice = new Slice(slice->texture_id);
		right_slice->zone.pos = Position(right + getOptions().texture_padding, slice->zone.pos.y);
		right_slice->zone.size = Size(
			slice->zone.size.width - rect_size.width - getOptions().texture_padding, 
			right_height);

		if (bottom_slice->zone.size.width > getOptions().texture_padding)
			m_free_slices.push_back(right_slice);

		slice->zone.size = rect_size;
	}

	return slice;
}

bool Compositor::_printToTextures()
{
	for (auto slice: m_used_slices)
	{
		assert(slice->texture_id < (int)m_textures.size() && "Error: Invalid Texture ID!");
		fipImage* texture = m_textures[slice->texture_id];
		fipImage* rect = slice->rect->getImage();
		assert(rect && "Error: Invalid Slice Rect!");

		BOOL retv = texture->pasteSubImage(*rect, slice->zone.pos.x, slice->zone.pos.y);
		if (retv == FALSE)
		{
			assert(0 && "Error: Paste Sub Image Failed!");
			return false;
		}

		m_texture_slices[slice->texture_id]->push_back(slice); // add to texture-slice map
		if (m_image_slices.find(slice->rect->getImageInfo()) == m_image_slices.end())
		{
			assert(false && "Error: Invalid Image Name!");
			return false;
		}
		m_image_slices[slice->rect->getImageInfo()]->push_back(slice); // add to image-slice map
	}

	// sort texture-slice map
	for (auto slices: m_texture_slices)
	{
		std::sort(slices->begin(), slices->end(), 
			[](Slice* lhs, Slice* rhs)
			{
				return lhs->zone.pos.y == rhs->zone.pos.y ? 
					lhs->zone.pos.x < rhs->zone.pos.x:
					lhs->zone.pos.y < rhs->zone.pos.y;
			}
		);
	}

	// sort image-slice map
	for (auto image_slices: m_image_slices)
	{
		auto slices = image_slices.second;
		std::sort(slices->begin(), slices->end(), 
			[](Slice* lhs, Slice* rhs)
			{
				return lhs->texture_id == rhs->texture_id ? 
					(lhs->zone.pos.y == rhs->zone.pos.y ? 
						lhs->zone.pos.x < rhs->zone.pos.x:
						lhs->zone.pos.y < rhs->zone.pos.y) :
					lhs->texture_id < rhs->texture_id;
			}
		);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

int test_cropper()
{
	Image* image = Image::createWithFileName("test.png");
	Image* image1 = Image::createWithFileName("test1.png");
	if (!image || !image1)
		return -1;

	//image->getOptions().scale_ratio = 0.8f;
	//image->getOptions().block_size = Size(50, 50);

	image->crop();
	image1->crop();

	Compositor comp;
	if (!comp.addImage(image))
		return -1;
	if (!comp.addImage(image1))
		return -1;
	comp.composit();

	bool retv = comp.saveTextures();
	retv = comp.saveToXML();

	return 0;
}

} // icropper
