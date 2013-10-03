#include <iostream>
#include <vector>
#include <string>

#define GFLAGS_DLL_DECL
#include <gflags/gflags.h>

#include "icropper.h"

//////////////////////////////////////////////////////////////////////////

DEFINE_string(src_path, "", "Source image path.");
DEFINE_string(src_files, "", "Image files to processing, seperated with blank for multi-files.");
DEFINE_string(out_path, "", "Output files(texture & description files) path.");
DEFINE_string(out_file, "", "Output icropper file, do not specify suffix name; if leave empty, using the first image name.");

DEFINE_bool(xml_only, false, "If only output xml file.");
DEFINE_bool(icb_only, false, "If only output icb file.");

DEFINE_double(scale, 1.0f, "Scale raw image.");
DEFINE_int32(block_size, 100, "Base cropping unit in pixels.");
DEFINE_int32(crop_min_area, 1000, "Cropping minmum area.");
DEFINE_double(crop_max_ratio, 0.6f, "Cropping maxmum area usage.");
DEFINE_int32(crop_max_depth, 4, "Cropping maxmum times.");

DEFINE_bool(force_single, false, "If must pack into 1 texture.");
DEFINE_int32(max_texture_size, 2048, "Maxmum size of texture.");
DEFINE_int32(fixed_texture_size, 0, "Use fixed texture size, default is 0, reps not using fixed size.");
DEFINE_int32(texture_padding, 1, "Padding size for rects in texture.");
DEFINE_bool(y_axis_up, true, "If direction of axis-y is down to top.");
DEFINE_bool(enable_rotate, true, "If rects placed into texture enable rotate.");

DEFINE_string(texture_suffix, "png", "Texture file suffix.");
DEFINE_string(xmlfile_suffix, "xml", "ICropper xml description file suffix.");
DEFINE_string(icbfile_suffix, "icb", "ICropper binary description file suffix.");

//////////////////////////////////////////////////////////////////////////
using namespace icropper;

typedef std::vector<Image*> ImageArray;

CropOptions			s_crop_options;
CompositorOptions	s_comp_options;

ImageArray			s_images;
Compositor			s_compositor;

// trim string
std::string trim_str(std::string s)
{
	if (s.length() == 0) return s;
	size_t beg = s.find_first_not_of(" \a\b\f\n\r\t\v");
	size_t end = s.find_last_not_of(" \a\b\f\n\r\t\v");
	if (beg == std::string::npos) return "";
	return std::string(s, beg, end - beg + 1);
}

// split string
std::vector<std::string> split_str(std::string str,std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str+=pattern;
	size_t size=str.size();

	for(size_t i=0; i<size; i++)
	{
		pos=str.find(pattern,i);
		if(pos<size)
		{
			std::string s=str.substr(i,pos-i);
			std::string ts = trim_str(s);
			if (!ts.empty())
				result.push_back(ts);
			i=pos+pattern.size()-1;
		}
	}
	return result;
}

int init_options()
{
	//
	// cropping options
	//
	s_crop_options.block_size.width		= FLAGS_block_size;
	s_crop_options.block_size.height	= FLAGS_block_size;
	s_crop_options.min_area				= FLAGS_crop_min_area;
	s_crop_options.crop_depth			= FLAGS_crop_max_depth;
	s_crop_options.crop_usage_ratio		= (float)FLAGS_crop_max_ratio;
	//s_crop_options.rotate_degress;
	s_crop_options.scale_ratio			= (float)FLAGS_scale;

	//
	// composit options
	//
	s_comp_options.max_texture_size		= FLAGS_max_texture_size;
	s_comp_options.fixed_texture_size	= FLAGS_fixed_texture_size;
	s_comp_options.texture_padding		= FLAGS_texture_padding;
	s_comp_options.texture_file_suffix	= FLAGS_texture_suffix;
	s_comp_options.xml_file_suffix		= FLAGS_xmlfile_suffix;
	s_comp_options.icb_file_suffix		= FLAGS_icbfile_suffix;
	s_comp_options.force_single_texture	= FLAGS_force_single;
	s_comp_options.flip_axis_y			= FLAGS_y_axis_up;
	s_comp_options.enable_rotate		= FLAGS_enable_rotate;
	
	return 0;
}


int crop_images()
{
	std::vector<std::string> files = split_str(FLAGS_src_files, " ");
	//for (auto s: files) std::cout << "[" << s << "]" << std::endl;
	
	for (auto f: files)
	{
		Image* image = Image::createWithFileName(f.c_str(), FLAGS_src_path.c_str());
		if (!image)
		{
			std::cout << "[ERR]" << "Can't open file: " << FLAGS_src_path << f << std::endl;
			return -1;
		}

		image->getOptions() = s_crop_options;
		
		if (image->crop())
		{
			s_images.push_back(image);
		}
		else
		{
			std::cout << "[ERR]" << "Cropping file failed: " << f << std::endl;
			return -1;
		}
	}

	return 0;
}

int composit_images()
{
	s_compositor.getOptions() = s_comp_options;
	s_compositor.getFileNamePrefix() = FLAGS_out_file;

	for (auto image: s_images)
	{
		if (!s_compositor.addImage(image))
		{
			std::cout << "[ERR]" << "Compositing image failed: " << image->getFileName() << std::endl;
			return -1;
		}
	}

	if (!s_compositor.composit())
	{
		std::cout << "[ERR]" << "Compositing failed!" << std::endl;
		return -1;
	}

	return 0;
}

int save_files()
{
	if (!s_compositor.saveTextures(FLAGS_out_path.c_str()))
	{
		std::cout << "[ERR]" << "Save textures failed: " << FLAGS_out_file << std::endl;
		return -1;
	}

	if ((!FLAGS_icb_only) && (!s_compositor.saveToXML(FLAGS_out_path.c_str())))
	{
		std::cout << "[ERR]" << "Save XML file failed: " << FLAGS_out_file << std::endl;
		return -1;
	}

	if ((!FLAGS_xml_only) && (!s_compositor.saveToBin(FLAGS_out_path.c_str())))
	{
		std::cout << "[ERR]" << "Save ICB file failed: " << FLAGS_out_file << std::endl;
		return -1;
	}

	return 0;
}

int main(int argc, char** argv)
{
	google::ParseCommandLineFlags(&argc, &argv, true); 
	
	if (init_options())
		return -1;

	if (crop_images())
		return -1;

	if (composit_images())
		return -1;

	if (save_files())
		return -1;

	return 0;
}