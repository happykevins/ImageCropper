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

#ifndef __SPRITE_CCMESH_SPRITE_H__
#define __SPRITE_CCMESH_SPRITE_H__

#include "sprite_nodes/CCSprite.h"

NS_CC_BEGIN

class CCMeshImage;
class CCSpriteFrame;


class CC_DLL CCMeshSprite : public CCSprite
{
public:
	CCMeshSprite();
	virtual ~CCMeshSprite();

	static CCMeshSprite* create();

	static CCMeshSprite* create(const char* image_name);

	static CCMeshSprite* create(const char* mesh_file, const char* image_name);
	
	virtual void setMeshImage(CCMeshImage* image);

	// @see CCSprite
	virtual void setDisplayFrame(CCSpriteFrame *pNewFrame);
	// @see CCSprite
	virtual bool isFrameDisplayed(CCSpriteFrame *pFrame);
	// @see CCSprite
	virtual CCSpriteFrame* displayFrame(void);
	// @see CCSprite
	virtual void draw(void);

private:
	CCMeshImage* m_frame_image;
};

NS_CC_END

#endif // __SPRITE_CCMESH_SPRITE_H__
