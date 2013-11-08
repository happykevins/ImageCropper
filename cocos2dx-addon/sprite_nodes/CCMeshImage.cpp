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
#include "sprite_nodes/CCMeshImage.h"
#include "sprite_nodes/CCMeshFileInfo.h"
#include "sprite_nodes/CCSprite.h"
#include "sprite_nodes/CCSpriteBatchNode.h"
#include "textures/CCTextureCache.h"
#include "support/CCPointExtension.h"
#include "shaders/CCShaderCache.h"

NS_CC_BEGIN

CCMeshImage::CCMeshImage()
	: m_name(NULL)
{

}

CCMeshImage::~CCMeshImage()
{
	CC_SAFE_RELEASE(m_name);
	for (CCTextureAtlasMap::iterator it = m_atlas_map.begin();
		it != m_atlas_map.end(); it++)
	{
		CC_SAFE_RELEASE(it->second);
	}
	m_atlas_map.clear();
}

CCMeshImage* CCMeshImage::create(CCMeshImageInfo* info)
{
	CCMeshImage * pRet = new CCMeshImage();
	if (info && pRet && pRet->initWithImageInfo(info))
	{
		pRet->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(pRet);
	}
	return pRet;
}

bool CCMeshImage::initWithImageInfo(CCMeshImageInfo* info)
{
	bool error_found = false;

	for (std::vector<CCMeshSliceInfo>::iterator slice = info->slices.begin(); 
		slice != info->slices.end(); slice++)
	{
		CCTextureAtlas* atlas = NULL;
		CCTextureAtlasMap::iterator atlas_it = m_atlas_map.find(slice->texture_id);
		if (atlas_it == m_atlas_map.end())
		{
			std::string tex_filename = info->id2tex[slice->texture_id];
			CCTexture2D* texture = CCTextureCache::sharedTextureCache()->addImage(tex_filename.c_str());
			if (!texture)
			{
				error_found = true;
				break;
			}
			texture->setAliasTexParameters();
			atlas = new CCTextureAtlas();
			atlas->initWithTexture(texture, 15);
			m_atlas_map[slice->texture_id] = atlas;
		}
		else
		{
			atlas = atlas_it->second;
		}

		if (atlas->getTotalQuads() == atlas->getCapacity())
		{
			atlas->resizeCapacity(atlas->getCapacity() * 1.5f + 1);
		}

		CCSize tex_size = atlas->getTexture()->getContentSize();

		float left   = slice->texture_pos.x / tex_size.width;
		float right  = left + slice->size.width / tex_size.width;
		float top    = slice->texture_pos.y / tex_size.height;
		float bottom = top + slice->size.height / tex_size.height;

		float ele_pos_left = slice->image_pos.x / info->scale_ratio;
		float ele_pos_top = (slice->image_pos.y + slice->size.height) / info->scale_ratio;
		float ele_width = slice->size.width / info->scale_ratio;
		float ele_height = slice->size.height / info->scale_ratio;

		ccV3F_C4B_T2F_Quad quad;

		if (slice->rotated)
		{
			quad.bl.texCoords.u = left;
			quad.bl.texCoords.v = top;
			quad.br.texCoords.u = left;
			quad.br.texCoords.v = bottom;
			quad.tl.texCoords.u = right;
			quad.tl.texCoords.v = top;
			quad.tr.texCoords.u = right;
			quad.tr.texCoords.v = bottom;
		}
		else
		{
			quad.bl.texCoords.u = left;
			quad.bl.texCoords.v = bottom;
			quad.br.texCoords.u = right;
			quad.br.texCoords.v = bottom;
			quad.tl.texCoords.u = left;
			quad.tl.texCoords.v = top;
			quad.tr.texCoords.u = right;
			quad.tr.texCoords.v = top;
		}

		quad.bl.vertices.x = (float) (ele_pos_left);
		quad.bl.vertices.y = ele_pos_top - ele_height;
		quad.bl.vertices.z = 0.0f;
		quad.br.vertices.x = (float)(ele_pos_left + ele_width);
		quad.br.vertices.y = ele_pos_top - ele_height;
		quad.br.vertices.z = 0.0f;
		quad.tl.vertices.x = (float)(ele_pos_left);
		quad.tl.vertices.y = ele_pos_top;
		quad.tl.vertices.z = 0.0f;
		quad.tr.vertices.x = (float)(ele_pos_left + ele_width);
		quad.tr.vertices.y = ele_pos_top;
		quad.tr.vertices.z = 0.0f;

		atlas->updateQuad(&quad, atlas->getTotalQuads());
	}

	if (!error_found)
	{
		m_name = new CCString(info->name);
		setAnchorPoint(ccp(0, 0));
		setContentSize(CCSize(info->size.width / info->scale_ratio, info->size.height / info->scale_ratio));

		// shader stuff
		setShaderProgram(CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTexture_uColor));
		//m_nUniformColor = glGetUniformLocation( getShaderProgram()->getProgram(), "u_color");
	}

	return !error_found;
}

void CCMeshImage::draw()
{
	CC_NODE_DRAW_SETUP();
	
	ccGLBlendFunc( CC_BLEND_SRC, CC_BLEND_DST );

	//GLfloat colors[4] = {_displayedColor.r / 255.0f, _displayedColor.g / 255.0f, _displayedColor.b / 255.0f, _displayedOpacity / 255.0f};
	//getShaderProgram()->setUniformLocationWith4fv(m_nUniformColor, colors, 1);

	for (CCTextureAtlasMap::iterator it = m_atlas_map.begin();
		it != m_atlas_map.end(); it++)
	{
		it->second->drawQuads();
	}
}

NS_CC_END

