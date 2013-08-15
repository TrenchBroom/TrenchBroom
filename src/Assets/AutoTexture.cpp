/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "AutoTexture.h"

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        AutoTexture::AutoTexture(const size_t width, const size_t height, const Color& averageColor, const Buffer<unsigned char>& buffer) :
        m_textureId(0),
        m_width(width),
        m_height(height),
        m_averageColor(averageColor) {
            assert(m_width > 0);
            assert(m_height > 0);
            m_buffers.push_back(buffer);
        }
        
        AutoTexture::AutoTexture(const size_t width, const size_t height, const Color& averageColor, const Buffer<unsigned char>::List& buffers) :
        m_textureId(0),
        m_width(width),
        m_height(height),
        m_averageColor(averageColor),
        m_buffers(buffers) {
            assert(m_width > 0);
            assert(m_height > 0);
        }

        AutoTexture::~AutoTexture() {
            deleteBuffers();
            if (m_textureId != 0) {
                glDeleteTextures(1, &m_textureId);
                m_textureId = 0;
            }
        }
        
        size_t AutoTexture::width() const {
            return m_width;
        }
        
        size_t AutoTexture::height() const {
            return m_height;
        }

        const Color& AutoTexture::averageColor() const {
            return m_averageColor;
        }

        void AutoTexture::activate() const {
            if (m_textureId == 0) {
                glGenTextures(1, &m_textureId);
                glBindTexture(GL_TEXTURE_2D, m_textureId);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(m_buffers.size() - 1));
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                size_t mipWidth = m_width;
                size_t mipHeight = m_height;
                size_t offset = 0;
                for (size_t j = 0; j < m_buffers.size(); ++j) {
                    const GLvoid* data = static_cast<GLvoid*>(m_buffers[j].ptr());
                    glTexImage2D(GL_TEXTURE_2D, j, GL_RGBA,
                                 static_cast<GLsizei>(mipWidth),
                                 static_cast<GLsizei>(mipHeight),
                                 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                    offset += mipWidth * mipHeight;
                    mipWidth /= 2;
                    mipHeight /= 2;
                }
                deleteBuffers();
            } else {
                glBindTexture(GL_TEXTURE_2D, m_textureId);
            }
        }
        
        void AutoTexture::deactivate() const {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        void AutoTexture::deleteBuffers() const {
            m_buffers = Buffer<unsigned char>::List(0);
        }
    }
}
