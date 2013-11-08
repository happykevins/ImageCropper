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
#include "sprite_nodes/CCMeshHacks.h"
#include "sprite_nodes/CCMeshImage.h"
#include "sprite_nodes/CCMeshSpriteFrame.h"
#include "textures/CCTextureCache.h"

NS_CC_BEGIN

bool CCSpriteFrameCacheHacker::checkNLoadMeshFile(const std::string& fileName)
{
	if (!CCMeshFileConfig::sharedMeshFileConfig()->isMeshFile(fileName.c_str()))
		return false;

	// parse file
	CCMeshFileInfo* images = CCMeshFileConfig::sharedMeshFileConfig()->parseMeshFile(fileName.c_str());
	if (!images)
	{
		CCLog("CCMeshFileInfo Load Error: %s", fileName.c_str());
		return false;
	}

	// load textures
	//for (auto item: images->id2tex)
	for (iCropperID2TexMap::iterator it = images->id2tex.begin();
		it != images->id2tex.end(); it++)
	{
		if (!CCTextureCache::sharedTextureCache()->addImage(it->second.c_str()))
			CCLog("CCMeshFileInfo Load Texture Error: %s", it->second.c_str());
	}

	// create sprite frames
	//for (auto item: images->images)
	for (std::map<std::string, CCMeshImageInfo*>::iterator it = images->images.begin();
		it != images->images.end(); it++)
	{
		CCMeshImage* mesh = CCMeshImage::create(it->second);
		if (!mesh)
		{
			CCLog("CCMeshImage Create Failed: %s", it->first);
			continue;
		}
		CCMeshSpriteFrame* frame = CCMeshSpriteFrame::create(mesh);
		CCAssert(frame, "Create mesh frame failed!");

		CCSpriteFrameCache::sharedSpriteFrameCache()->m_pSpriteFrames->setObject(frame, it->first);
	}
	
	CC_SAFE_DELETE(images);

	return true;
}

CCSpriteFrame* CCSpriteFrameCacheHacker::getMeshSpriteFrame(const std::string& spriteName, const std::string& sheetName)
{
	if (spriteName.empty())
		return NULL;
	
	std::string meshFile = sheetName;
	if (meshFile.empty())
	{
		if (!CCMeshFileConfig::sharedMeshFileConfig()->convertImageName2MeshFileName(spriteName.c_str(), meshFile))
			return NULL;
	}
	else if(!CCMeshFileConfig::sharedMeshFileConfig()->isMeshFile(meshFile.c_str()))
	{
		return NULL;
	}

	CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(meshFile.c_str());
	return CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(spriteName.c_str());
}

NS_CC_END
