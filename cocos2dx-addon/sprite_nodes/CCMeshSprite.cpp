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
#include "sprite_nodes/CCMeshSprite.h"
#include "sprite_nodes/CCMeshSpriteFrame.h"
#include "sprite_nodes/CCMeshImage.h"
#include "sprite_nodes/CCMeshFileInfo.h"
#include "sprite_nodes/CCSpriteFrame.h"
#include "sprite_nodes/CCSpriteFrameCache.h"
#include "draw_nodes/CCDrawingPrimitives.h"

NS_CC_BEGIN

CCMeshSprite::CCMeshSprite()
	: m_frame_image(NULL)
{
}

CCMeshSprite::~CCMeshSprite()
{
	CC_SAFE_RELEASE(m_frame_image);
}

CCMeshSprite* CCMeshSprite::create()
{
	CCMeshSprite* spr = new CCMeshSprite;
	if (spr->init())
	{
		spr->autorelease();
		return spr;
	}
	CC_SAFE_DELETE(spr);
	return NULL;
}

CCMeshSprite* CCMeshSprite::create(const char* image_name)
{
	std::string mesh_file;
	if (!CCMeshFileConfig::sharedMeshFileConfig()->convertImageName2MeshFileName(image_name, mesh_file))
		return NULL;

	return create(mesh_file.c_str(), image_name);
}

CCMeshSprite* CCMeshSprite::create(const char* mesh_file, const char* image_name)
{
	CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile(mesh_file);
	CCSpriteFrame* frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(image_name);

	CCMeshSprite* spr = create();
	spr->setDisplayFrame(frame);

	return spr;
}

void CCMeshSprite::setMeshImage(CCMeshImage* image)
{
	if (m_frame_image == image)
		return;

	if (m_frame_image)
		CC_SAFE_RELEASE_NULL(m_frame_image);

	m_frame_image = image;
	m_frame_image->retain();
	setContentSize(image->getContentSize());
}

void CCMeshSprite::setDisplayFrame(CCSpriteFrame *pNewFrame)
{
	CCMeshSpriteFrame* iFrame = dynamic_cast<CCMeshSpriteFrame*>(pNewFrame);
	if (!iFrame)
	{
		CCSprite::setDisplayFrame(pNewFrame);
		return;
	}
	setMeshImage(iFrame->getMesh());
}

bool CCMeshSprite::isFrameDisplayed(CCSpriteFrame *pFrame)
{
	CCMeshSpriteFrame* iFrame = dynamic_cast<CCMeshSpriteFrame*>(pFrame);
	if (!iFrame)
		return CCSprite::isFrameDisplayed(pFrame);

	// TODO:
	return false;
}

CCSpriteFrame* CCMeshSprite::displayFrame(void)
{
	if (!m_frame_image)
		return CCSprite::displayFrame();

	return NULL; //TODO: acquire iMeshSpriteFrame;
}

void CCMeshSprite::draw(void)
{
	if (m_frame_image)
	{
		m_frame_image->draw();

		// bounding box
		const CCSize& s = this->getContentSize();
		CCPoint vertices[4]={
			ccp(0,0),ccp(s.width,0),
			ccp(s.width,s.height),ccp(0,s.height),
		};
		ccDrawColor4B(0xff, 0xff, 0xff, 0xff);
		ccDrawPoly(vertices, 4, true);
	}
	else
	{
		CCSprite::draw();
	}
}

NS_CC_END
