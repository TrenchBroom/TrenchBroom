/*
 Copyright (C) 2010-2012 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TextureRenderer.h"

#include "IO/Wad.h"
#include "Model/Bsp.h"
#include "Model/Alias.h"
#include "Renderer/Palette.h"

namespace TrenchBroom {
    namespace Renderer {
        void TextureRenderer::init(unsigned int width, unsigned int height) {
            m_width = width;
            m_height = height;
            m_textureBuffer = NULL;
			m_textureId = 0;
        }
        
        void TextureRenderer::init(unsigned char* rgbImage, unsigned int width, unsigned int height) {
            init(width, height);
            m_textureBuffer = rgbImage;
        }
        
        TextureRenderer::TextureRenderer(unsigned char* rgbImage, const Color& averageColor, unsigned int width, unsigned int height) :
        m_averageColor(averageColor) {
            init(rgbImage, width, height);
        }
        
        TextureRenderer::TextureRenderer(const Model::AliasSkin& skin, unsigned int skinIndex, const Palette& palette) {
            init(skin.width(), skin.height());
            m_textureBuffer = new unsigned char[m_width * m_height * 3];
            palette.indexedToRgb(skin.pictures()[skinIndex], m_textureBuffer, m_width * m_height, m_averageColor);
        }
        
        TextureRenderer::TextureRenderer(const Model::BspTexture& texture, const Palette& palette) {
            init(texture.width(), texture.height());
            m_textureBuffer = new unsigned char[m_width * m_height * 3];
            palette.indexedToRgb(texture.image(), m_textureBuffer, m_width * m_height, m_averageColor);
        }
        
        TextureRenderer::TextureRenderer() {
            init(1, 1);
            m_textureBuffer = new unsigned char[4];
            for (int i = 0; i < 4; i++)
                m_textureBuffer[i] = 0;
        }
        
        TextureRenderer::~TextureRenderer() {
            if (m_textureId > 0)
                glDeleteTextures(1, &m_textureId);
            if (m_textureBuffer != NULL)
                delete [] m_textureBuffer;
        }

        void TextureRenderer::activate() {
            if (m_textureId == 0) {
                if (m_textureBuffer != NULL) {
                    glGenTextures(1, &m_textureId);
                    glBindTexture(GL_TEXTURE_2D, m_textureId);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height), 0, GL_RGB, GL_UNSIGNED_BYTE, m_textureBuffer);
                    delete [] m_textureBuffer;
                    m_textureBuffer = NULL;
                }
            }
            
            glBindTexture(GL_TEXTURE_2D, m_textureId);
        }
        
        void TextureRenderer::deactivate() {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}
