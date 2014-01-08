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
        m_width(0),
        m_height(0),
        m_buffer(NULL) {}
        
        FontTexture::FontTexture(const size_t width, const size_t height) :
        m_width(width),
        m_height(height),
        m_buffer(NULL) {
            assert(m_width > 0);
            assert(m_height > 0);
            m_buffer = new char[m_width * m_height];
            std::memset(m_buffer, 0, m_width * m_height);
        }
        
        FontTexture::FontTexture(const FontTexture& other) :
        m_width(other.m_width),
        m_height(other.m_height),
        m_buffer(NULL) {
            m_buffer = new char[m_width * m_height];
            std::memcpy(m_buffer, other.m_buffer, m_width * m_height);
        }
        
        FontTexture& FontTexture::operator=(FontTexture other) {
            using std::swap;
            m_width = other.m_width;
            m_height = other.m_height;
            swap(m_buffer, other.m_buffer);
            return *this;
        }
        
        FontTexture::~FontTexture() {
            m_width = 0;
            m_height = 0;
            delete [] m_buffer;
            m_buffer = NULL;
        }
        
        size_t FontTexture::width() const {
            return m_width;
        }
        
        size_t FontTexture::height() const {
            return m_height;
        }
        
        const char* FontTexture::buffer() const {
            return m_buffer;
        }
        
        void FontTexture::drawGlyph(const int x, const int y, const int maxAscend, const FT_GlyphSlot glyph) {
            const size_t left = static_cast<size_t>(x + glyph->bitmap_left);
            const size_t top = static_cast<size_t>(y + maxAscend - glyph->bitmap_top);
            const size_t rows = static_cast<size_t>(glyph->bitmap.rows);
            const size_t width = static_cast<size_t>(glyph->bitmap.width);
            const size_t pitch = static_cast<size_t>(glyph->bitmap.pitch);
            
            for (size_t r = 0; r < rows; r++) {
                const size_t index = (r + top) * m_width + left;
                assert(index + width < m_width * m_height);
                std::memcpy(m_buffer + index, glyph->bitmap.buffer + r * pitch, width);
            }
        }
    }
}
