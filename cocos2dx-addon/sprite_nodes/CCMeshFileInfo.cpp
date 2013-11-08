/****************************************************************************
Copyright (c) 2013-2014 Kevin Sun

https://github.com/happykevins
email:happykevins@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "sprite_nodes/CCMeshFileInfo.h"
#include "ccMacros.h"
#include "platform/CCFileUtils.h"
#include <algorithm>

NS_CC_BEGIN

size_t _str_replace_ch(std::string& str, char which, char to)
{
	size_t num = 0;
	for ( size_t i = 0; i < str.size(); i++ )
	{
		if ( str[i] == which )
		{
			str[i] = to;
			num++;
		}
	}
	return num;
}

CCMeshFileInfo::~CCMeshFileInfo()
{
	//for (auto image: images)
	for (std::map<std::string, CCMeshImageInfo*>::iterator it = images.begin();
		it != images.end(); it++)
	{
		delete it->second;
	}
	images.clear();
}

CCMeshImageInfo* CCMeshFileInfo::getImage(const char* image_name)
{
	std::string s(image_name);
	_str_replace_ch(s, '\\', '/');
	auto result = images.find(s);
	if (result != images.end())
		return (*result).second;
	return NULL;
}

CCMeshXMLParser::CCMeshXMLParser()
	: m_images(NULL)
	, m_processing_image(NULL)
{

}

CCMeshFileInfo* CCMeshXMLParser::parse(const char* filename)
{
	CCSAXParser parser;
	if ( !parser.init("UTF-8") )
	{
		return NULL;
	}
	parser.setDelegator(this);

	CCMeshFileInfo* images = new CCMeshFileInfo;
	m_images = images;

	if ( parser.parse(filename) )
	{
		// parse success
	}
	else
	{
		delete images;
		images = NULL;
	}
	m_images = NULL;
	m_processing_image = NULL;

	return images;
}

void CCMeshXMLParser::startElement(void *ctx, const char *name, const char **atts)
{
	Properties props;
	if (atts)
	{
		for ( size_t i = 0; atts[i] != NULL && atts[i+1] != NULL; i+=2 )
		{
			props[atts[i]] = atts[i+1];
		}
	}

	if (strcmp(name, "rect") == 0)
	{
		CCAssert(m_processing_image, "Invalid Processing Image!");
		CCMeshSliceInfo slice;

		slice.texture_id = _propertiesInt(props, "id");
		slice.texture_pos.x = _propertiesInt(props, "texture_x");
		slice.texture_pos.y = _propertiesInt(props, "texture_y");
		slice.image_pos.x = _propertiesInt(props, "image_x");
		slice.image_pos.y = _propertiesInt(props, "image_y");
		slice.size.width = _propertiesInt(props, "width");
		slice.size.height = _propertiesInt(props, "height");
		slice.rotated = _propertiesBool(props, "rotate");

		if (m_processing_image->id2tex.find(slice.texture_id) == m_processing_image->id2tex.end())
		{
			m_processing_image->id2tex[slice.texture_id] = m_images->id2tex[slice.texture_id];
		}

		m_processing_image->slices.push_back(slice);
	}
	else if (strcmp(name, "image") == 0)
	{
		CCAssert(props.find("name") != props.end(), "No Image Name!");
		m_processing_image = new CCMeshImageInfo;
		m_processing_image->size.width = _propertiesInt(props, "width");
		m_processing_image->size.height = _propertiesInt(props, "height");
		if (_propertiesExist(props, "scale"))
		{
			m_processing_image->scale_ratio = _propertiesFloat(props, "scale");
		}
		std::string image_name = props["name"];
		_str_replace_ch(image_name, '\\', '/');
		CCAssert(m_images->images.find(image_name) == m_images->images.end(), "Conflict! Image With the Same Name!");
		m_processing_image->name = image_name;
		m_images->images[image_name] = m_processing_image;
	}
	else if (strcmp(name, "texture") == 0)
	{
		CCAssert(props.find("file") != props.end(), "No Texture File!");
		std::string filename = props["file"];
		int id = _propertiesInt(props, "id");
		m_images->id2tex[id] = filename;
	}
	else if (strcmp(name, "textures") == 0)
	{

	}
}

void CCMeshXMLParser::endElement(void *ctx, const char *name)
{
	if (strcmp(name, "image") == 0)
	{
		m_processing_image = NULL;
	}
	else if (strcmp(name, "texture") == 0)
	{

	}
}

void CCMeshXMLParser::textHandler(void *ctx, const char *s, int len)
{

}

bool CCMeshXMLParser::_propertiesExist(Properties& props, const char* name)
{
	auto pit = props.find(name);
	if ( pit != props.end() )
	{
		return true;
	}
	return false;
}

int CCMeshXMLParser::_propertiesInt(Properties& props, const char* name)
{
	auto pit = props.find(name);
	if ( pit != props.end() )
	{
		return atoi(pit->second.c_str());
	}
	return 0;
}

bool CCMeshXMLParser::_propertiesBool(Properties& props, const char* name)
{
	return _propertiesInt(props, name) != 0;
}

float CCMeshXMLParser::_propertiesFloat(Properties& props, const char* name)
{
	auto pit = props.find(name);
	if ( pit != props.end() )
	{
		return atof(pit->second.c_str());
	}
	return 0.0f;
}

CCMeshFileConfig::CCMeshFileConfig()
: m_FileMode(kBIN)
{
	m_FileSuffix[kBIN] = ".icb";
	m_FileSuffix[kXML] = ".xml";
}

CCMeshFileConfig* CCMeshFileConfig::sharedMeshFileConfig()
{
	static CCMeshFileConfig* instance = NULL;
	if (!instance)
	{
		instance = new CCMeshFileConfig;
	}
	return instance;
}

bool CCMeshFileConfig::isMeshFile(const char* filename)
{
	std::string suffix(filename);
	suffix = suffix.substr(suffix.rfind('.'), suffix.size());
	transform(suffix.begin(), suffix.end(), suffix.begin(), tolower);

	return strcmp(suffix.c_str(), m_FileSuffix[m_FileMode]) == 0;
}

CCMeshFileInfo* CCMeshFileConfig::parseMeshFile(const char* filename)
{
	if (m_FileMode == kBIN)
	{
		CCMeshBINParser parser;
		return parser.parse(filename);
	}
	else if (m_FileMode == kXML)
	{
		CCMeshXMLParser parser;
		return parser.parse(filename);
	}

	return NULL;
}

bool CCMeshFileConfig::convertImageName2MeshFileName(const char* image_name, std::string& mesh_name)
{
	CCAssert(image_name, "Image Name is NULL!");
	std::string temp(image_name);
	mesh_name = temp.substr(0, temp.rfind('.')) + m_FileSuffix[m_FileMode];
	std::string fullpath = CCFileUtils::sharedFileUtils()->fullPathForFilename(mesh_name.c_str());
	return CCFileUtils::sharedFileUtils()->isFileExist(fullpath);
}

NS_CC_END

