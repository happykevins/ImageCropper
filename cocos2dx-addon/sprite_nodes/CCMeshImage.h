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

#ifndef __SPRITE_CCMESH_IMAGE_H__
#define __SPRITE_CCMESH_IMAGE_H__

#include "base_nodes/CCNode.h"
#include "textures/CCTextureAtlas.h"

NS_CC_BEGIN

struct CCMeshImageInfo;

typedef std::map<int, CCTextureAtlas*> CCTextureAtlasMap;

class CC_DLL CCMeshImage : public CCNode
{
public:
	CCMeshImage();
	virtual ~CCMeshImage();

	static CCMeshImage* create(CCMeshImageInfo* info);

	bool initWithImageInfo(CCMeshImageInfo* info);

	inline CCString* getName() { return m_name; }

	// @see CCNode
	virtual void draw();

private:
	CCString* m_name;
	CCTextureAtlasMap m_atlas_map;
};

NS_CC_END

#endif //__SPRITE_CCMESH_IMAGE_H__
