#ifndef ICROPPER_H_
#define ICROPPER_H_

//
// TODO: animation/action support?
// TODO: (icb)binary file format support
// TODO: fixed texture size support
// TODO: only single texture may not work
//

#include <string>
#include <list>
#include <vector>
#include <map>

#include <FreeImage/FreeImagePlus.h>

#define ICROPPER_DEFAULT_CROP_BLOCK_WIDTH	100
#define ICROPPER_DEFAULT_CROP_BLOCK_HEIGHT	100
#define ICROPPER_DEFAULT_CROP_MIN_AREA		1000
#define ICROPPER_DEFAULT_THRESHOLD_DEPTH	4
#define ICROPPER_DEFAULT_THRESHOLD_USAGE	0.6f
#define ICROPPER_DEFAULT_ROTATE_DEGREES		-90.0f

#define ICROPPER_DEFAULT_MAX_TEXTURE_SIZE	2048
#define ICROPPER_DEFAULT_TEXTURE_PADDING	1

#define ICROPPER_DEFAULT_RAW_TEXTURE_SUFFIX "png"
#define ICROPPER_DEFAULT_TEXTURE_SUFFIX		"png"
#define ICROPPER_DEFAULT_XML_FILE_SUFFIX	"xml"
#define ICROPPER_DEFAULT_BIN_FILE_SUFFIX	"icb"

#define ICROPPER_FILE_COMMENT				"ICropper Description File, RenRen Games."
#define ICROPPER_VERSION					"0.1"

#define ICROPPER_FILE_ROOT_NODE				"icropper"
#define ICROPPER_FILE_INFO_ROOT_NODE		"infos"
#define ICROPPER_FILE_INFO_NODE				"info"
#define ICROPPER_FILE_TEXTURES_NODE			"textures"
#define ICROPPER_FILE_TEXTURE_NODE			"texture"
#define ICROPPER_FILE_IMAGES_NODE			"images"
#define ICROPPER_FILE_IMAGE_NODE			"image"
#define ICROPPER_FILE_RECT_NODE				"rect"
#define ICROPPER_FILE_ACTIONS_NODE			"actions"
#define ICROPPER_FILE_ANIM_NODE				"anim"
#define ICROPPER_FILE_FRAME_NODE			"frame"

namespace icropper {

// reps a left-top position 
struct Position
{
	Position() : x(0), y(0) {}
	Position(int _x, int _y) : x(_x), y(_y) {}

	inline void add(const Position& rhs)
	{
		x += rhs.x;
		y += rhs.y;
	}

	inline bool operator==(const Position& rhs)
	{
		return x == rhs.x && y == rhs.y;
	}

	int x;
	int y;
};

struct Size
{
	Size() : width(0), height(0) {}
	Size(int _width, int _height) : width(_width), height(_height) {}

	inline bool isZero() { return width == 0 || height == 0; }
	inline unsigned int area() { return width * height; }

	int width;
	int height;
};

struct Zone
{
	Position pos;
	Size size;

	inline bool isZero() { return size.isZero(); }
	inline bool contains(Size rect_size) { return size.width >= rect_size.width && size.height >= rect_size.height; }
};

struct CropOptions
{
	CropOptions()
		: block_size(ICROPPER_DEFAULT_CROP_BLOCK_WIDTH, ICROPPER_DEFAULT_CROP_BLOCK_HEIGHT)
		, min_area(ICROPPER_DEFAULT_CROP_MIN_AREA)
		, crop_depth(ICROPPER_DEFAULT_THRESHOLD_DEPTH)
		, crop_usage_ratio(ICROPPER_DEFAULT_THRESHOLD_USAGE)
		, rotate_degress(ICROPPER_DEFAULT_ROTATE_DEGREES)
		, scale_ratio(1.0f)
	{
	}

	inline bool is_scaled() { return scale_ratio < 0.99f || scale_ratio > 1.01f; }

	Size	block_size;
	unsigned int min_area;
	int		crop_depth;
	float	crop_usage_ratio;
	float	rotate_degress;
	float	scale_ratio;
};


struct CompositorOptions
{
	CompositorOptions() 
		: max_texture_size(ICROPPER_DEFAULT_MAX_TEXTURE_SIZE)
		, texture_padding(ICROPPER_DEFAULT_TEXTURE_PADDING)
		, texture_file_suffix(ICROPPER_DEFAULT_TEXTURE_SUFFIX)
		, xml_file_suffix(ICROPPER_DEFAULT_XML_FILE_SUFFIX)
		, icb_file_suffix(ICROPPER_DEFAULT_BIN_FILE_SUFFIX)
		, force_single_texture(false)
		, flip_axis_y(true)
		, enable_rotate(true)
		, fixed_texture_size(0)
		
	{
	}
	
	int max_texture_size;
	int texture_padding;
	std::string texture_file_suffix;
	std::string xml_file_suffix;
	std::string icb_file_suffix;
	bool force_single_texture;
	bool flip_axis_y;
	bool enable_rotate;
	int fixed_texture_size;				// if use fixed texture size. 0 reps invalid.
};


typedef std::list<class ImageRect*> RectList;

//
// Raw Image
//
class Image
{
public:
	Image();
	~Image();
	static Image* createWithFileName(const char* filename, const char* path = NULL);

	inline const std::string& getFileName() const { return m_filename; }
	inline const Size& getSize() const { return m_raw_size; }
	inline fipImage* getRawImage() { return m_raw_image; }
	inline class ImageRect* getRootRect() { return m_root_rect; }

	bool crop();
	inline RectList& getRects() { return m_rects; }
	inline CropOptions& getOptions() { return m_options; }

private:
	std::string m_filename;
	fipImage* m_raw_image;
	Size m_raw_size;
	class ImageRect* m_root_rect;
	RectList m_rects;
	CropOptions m_options;
};

//
// A Rect reps a SubImage
//
class ImageRect
{
public:
	ImageRect(fipImage* image, Image* image_info, ImageRect* parent, Position rel_pos = Position(0, 0));
	~ImageRect();

	// image info
	inline Image* getImageInfo() { return m_image_info; }
	inline fipImage* getImage() { return m_image; }
	float getSolidPixelsRatio(); // 0.0f ~ 1.0f; pixels not-transparent / total
	float getOpacityPixelsRatio();
	float getSavedAreaRatio(); // 0.0f ~ 1.0f; area saved ratio
	bool isFullTransparent();
	void rotate();
	inline bool isRotated() { return m_is_rotated; }

	// tree info
	inline ImageRect* getParent() { return m_parent; }
	inline RectList* getChildren() { return &m_children; }
	int getDepth();
	inline bool isRootRect() { return m_parent == NULL; }
	inline bool isLeafRect() { return m_children.empty(); }
	void getLeafRects(RectList& rects);

	// zone info
	inline Zone getRelativeZone() { return m_zone; }
	inline Size getSize() { return m_zone.size; }
	Zone getAbsZone();

	// crop utils
	void cropWithFixedSize(Size block_size);
	void cropHalving();

private:
	void _initCropUnusedBorder();
	void _getPixelCount(unsigned int& used, unsigned int& unused, unsigned int& opacity, unsigned int& total);

private:
	fipImage* m_image;
	Zone m_zone;
	bool m_is_rotated;	// rotate 90 degrees
	Image* m_image_info;

	ImageRect* m_parent;
	RectList m_children;
};

typedef std::vector<fipImage*> TextureArray;

//
// Rect Compositor
//
class Compositor
{
public:
	struct Slice
	{
		Slice(int tid) : texture_id(tid), rect(NULL) {}

		const int texture_id;
		Zone zone;
		ImageRect* rect;
	};

	typedef std::vector<ImageRect*> RectArray;
	typedef std::vector<Slice*> SliceArray;
	typedef std::vector<SliceArray*> TextureSlices;
	typedef std::map<Image*, SliceArray*> ImageSlices;

	Compositor();
	~Compositor();

	void reset();
	bool addImage(Image* image);
	bool composit();

	inline TextureArray& getTextures() { return m_textures; }
	inline TextureSlices& getTextureSlices() { return m_texture_slices; }
	float getUsageRatio();
	float getUsageRatioForTexture(int idx);

	inline CompositorOptions& getOptions() { return m_options; }
	inline std::string& getFileNamePrefix() { return m_file_prefix; }

	bool saveTextures(const char* path = NULL);
	bool saveToXML(const char* path = NULL);
	bool saveToBin(const char* path = NULL);

private:
	void _clearTextures();
	void _clearImages();
	void _clearSlices();
	int _getMostSuitableWidth(RectArray::iterator begin, RectArray::iterator end); // calculate most suitable width

	bool _insertRect(ImageRect* rect);
	void _createTexture(int texture_width);
	Slice* _findFreeSlice(Size rect_size);

	bool _printToTextures();

	std::string m_file_prefix;

	RectArray m_rects;

	SliceArray m_used_slices;
	SliceArray m_free_slices;
	
	TextureArray m_textures;
	TextureSlices m_texture_slices;
	ImageSlices m_image_slices;

	CompositorOptions m_options;
};

int test_cropper();

} // icropper

#endif
