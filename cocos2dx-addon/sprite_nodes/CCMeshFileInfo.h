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

#ifndef __SPRITE_CCMESH_FILEINFO_H__
#define __SPRITE_CCMESH_FILEINFO_H__

#include "platform/CCSAXParser.h"
#include "support/CCPointExtension.h"

#include <vector>
#include <map>
#include <string>

NS_CC_BEGIN

typedef std::map<int, std::string> iCropperID2TexMap;

struct CC_DLL CCMeshSliceInfo
{
	CCMeshSliceInfo() : texture_id(0), rotated(false) {}

	int texture_id;
	CCPoint texture_pos;
	CCPoint image_pos;
	CCSize size;
	bool rotated;
};

struct CC_DLL CCMeshImageInfo
{
	CCMeshImageInfo() : scale_ratio(1.0f) {}

	std::string name;
	CCSize size;
	float scale_ratio;
	iCropperID2TexMap id2tex;
	std::vector<CCMeshSliceInfo> slices;
};

struct CC_DLL CCMeshFileInfo
{
	~CCMeshFileInfo();
	CCMeshImageInfo* getImage(const char* image_name);

	iCropperID2TexMap id2tex;
	std::map<std::string, CCMeshImageInfo*> images;
};

class CC_DLL CCMeshXMLParser : public CCSAXDelegator
{
public:
	typedef std::map<std::string, std::string> Properties;

	CCMeshXMLParser();
	CCMeshFileInfo* parse(const char* filename);

	virtual void startElement(void *ctx, const char *name, const char **atts);
	virtual void endElement(void *ctx, const char *name);
	virtual void textHandler(void *ctx, const char *s, int len);

private:
	bool _propertiesExist(Properties& props, const char* name);
	int _propertiesInt(Properties& props, const char* name);
	bool _propertiesBool(Properties& props, const char* name);
	float _propertiesFloat(Properties& props, const char* name);

	CCMeshFileInfo* m_images;
	CCMeshImageInfo* m_processing_image;
};

class CC_DLL CCMeshBINParser
{
public:
	CCMeshFileInfo* parse(const char* filename) { return NULL; }
};

class CC_DLL CCMeshFileConfig
{
public:
	enum FileMode
	{
		kBIN = 0,		// "icb"
		kXML = 1,		// "xml"
		kFileModeNum
	};

	static CCMeshFileConfig* sharedMeshFileConfig();

	inline void setMeshFileMode(FileMode mode) { m_FileMode = mode; }
	inline FileMode getMeshFileMode() { return m_FileMode; }

	inline const char* getMeshFileSuffix() { return m_FileSuffix[m_FileMode]; }
	inline void setMeshFileSuffix(FileMode mode, const char* suffix) { m_FileSuffix[mode] = suffix; }

	bool isMeshFile(const char* filename);
	CCMeshFileInfo* parseMeshFile(const char* filename);

	bool convertImageName2MeshFileName(const char* image_name, std::string& mesh_name);

private:
	CCMeshFileConfig();

	const char* m_FileSuffix[kFileModeNum];
	FileMode m_FileMode;
};

NS_CC_END

#endif //__SPRITE_CCMESH_FILEINFO_H__
