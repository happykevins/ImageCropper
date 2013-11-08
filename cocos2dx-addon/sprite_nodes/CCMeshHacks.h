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

#ifndef __SPRITE_CCMESH_HACKS_H__
#define __SPRITE_CCMESH_HACKS_H__

#include "sprite_nodes/CCMeshHeaders.h"
#include "sprite_nodes/CCSpriteFrameCache.h"

NS_CC_BEGIN

//
// How to integration the CCMeshSprite to cocos2d-x
//
//	1. class CCSpriteFrameCache: add "friend class CCSpriteFrameCacheHacker;" 
//	2. CCSpriteFrameCache.cpp: add "#include "sprite_nodes/CCMeshHacks.h;" 
//	3. in function "CCSpriteFrameCache::addSpriteFramesWithFile" after the code get "fullPath":
//		add "if (CCSpriteFrameCacheHacker::checkNLoadMeshFile(fullPath)) { m_pLoadedFileNames->insert(pszPlist); return; }"  
//	4*. CCBReader.cpp & CCNodeLoader.cpp: add "#include "sprite_nodes/CCMeshHacks.h;" 
//	5*. in function "CCBReader::readKeyframe" at section "type == kCCBPropTypeSpriteFrame":
//		add "if (spriteFrame = CCSpriteFrameCacheHacker::getMeshSpriteFrame(spriteFile, spriteSheet)) {} else"
//	6*. in function "CCNodeLoader::parsePropTypeSpriteFrame" before "if (spriteSheet.length() == 0)":
//		add "if (spriteFrame = CCSpriteFrameCacheHacker::getMeshSpriteFrame(spriteFile, spriteSheet)) {} else"	
//

class CC_DLL CCSpriteFrameCacheHacker
{
public:
	static bool checkNLoadMeshFile(const std::string& fileName);

	static CCSpriteFrame* getMeshSpriteFrame(const std::string& spriteName, const std::string& sheetName);
};


NS_CC_END

#endif //__SPRITE_CCMESH_HACKS_H__