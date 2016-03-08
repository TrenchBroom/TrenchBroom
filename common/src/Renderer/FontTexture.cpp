/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FontTexture.h"

#include <cassert>
#include <cstring>
#include <memory>

namespace TrenchBroom {
    namespace Renderer {
        FontTexture::FontTexture() :
        m_size(0),
        m_buffer(NULL),
        m_textureId(0) {}
        
        FontTexture::FontTexture(const size_t cellCount, const size_t cellSize, const size_t margin) :
        m_size(computeTextureSize(cellCount, cellSize, margin)),
        m_buffer(NULL),
        m_textureId(0) {
            m_buffer = new char[m_size * m_size];
            std::memset(m_buffer, 0, m_size * m_size);
        }
        
        FontTexture::FontTexture(const FontTexture& other) :
        m_size(other.m_size),
        m_buffer(NULL),
        m_textureId(0) {
            m_buffer = new char[m_size * m_size];
            std::memcpy(m_buffer, other.m_buffer, m_size * m_size);
        }
        
        FontTexture& FontTexture::operator=(FontTexture other) {
            using std::swap;
            swap(m_size, other.m_size);
            swap(m_buffer, other.m_buffer);
            swap(m_textureId, other.m_textureId);
            return *this;
        }
        
        FontTexture::~FontTexture() {
            m_size = 0;
            if (m_textureId != 0) {
                glAssert(glDeleteTextures(1, &m_textureId));
                m_textureId = 0;
            }
            delete [] m_buffer;
            m_buffer = NULL;
        }
        
        size_t FontTexture::size() const {
            return m_size;
        }
 
        void FontTexture::activate() {
            if (m_textureId == 0) {
                assert(m_buffer != NULL);
                glAssert(glGenTextures(1, &m_textureId));
                glAssert(glBindTexture(GL_TEXTURE_2D, m_textureId));
                glAssert(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
                glAssert(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
                glAssert(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                glAssert(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
                glAssert(glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, static_cast<GLsizei>(m_size), static_cast<GLsizei>(m_size), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_buffer));
                delete [] m_buffer;
                m_buffer = NULL;
            }
            
            assert(m_textureId > 0);
            glAssert(glBindTexture(GL_TEXTURE_2D, m_textureId));
        }
        
        void FontTexture::deactivate() {
            glAssert(glBindTexture(GL_TEXTURE_2D, 0));
        }

        size_t FontTexture::computeTextureSize(const size_t cellCount, const size_t cellSize, const size_t margin) const {
            const size_t minTextureSize = margin + cellCount * (cellSize + margin);
            size_t textureSize = 1;
            while (textureSize < minTextureSize)
                textureSize = textureSize << 1;
            return textureSize;
        }
    }
}
