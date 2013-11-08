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
#include "sprite_nodes/CCMeshSpriteFrame.h"
#include "sprite_nodes/CCMeshImage.h"

NS_CC_BEGIN

CCMeshSpriteFrame::CCMeshSpriteFrame()
: m_mesh(NULL)
{

}

CCMeshSpriteFrame::~CCMeshSpriteFrame()
{
	CC_SAFE_RELEASE(m_mesh);
}

CCMeshSpriteFrame* CCMeshSpriteFrame::create(CCMeshImage* mesh)
{
	CCAssert(mesh, "Must specify CCMeshImage!");
	CCMeshSpriteFrame *pSpriteFrame = new CCMeshSpriteFrame();
	pSpriteFrame->initWithMesh(mesh);
	pSpriteFrame->autorelease();
	return pSpriteFrame;
}

bool CCMeshSpriteFrame::initWithMesh(CCMeshImage* mesh)
{
	CCSpriteFrame::initWithTexture(NULL, 
		CCRect(0, 0, mesh->getContentSize().width, mesh->getContentSize().height)); 

	CC_SAFE_RELEASE(m_mesh);
	m_mesh = mesh;
	mesh->retain();

	return true;
}

NS_CC_END

