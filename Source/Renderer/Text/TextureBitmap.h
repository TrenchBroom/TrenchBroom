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

#ifndef TrenchBroom_TextureBitmap_h
#define TrenchBroom_TextureBitmap_h

#include <cassert>
#include <cstring>
#include <memory>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            class TextureBitmap {
            public:
                typedef std::auto_ptr<TextureBitmap> Ptr;
            private:
                size_t m_width;
                size_t m_height;
                char* m_buffer;
            public:
                TextureBitmap(size_t width, size_t height) :
                m_width(width),
                m_height(height),
                m_buffer(NULL) {
                    assert(m_width > 0);
                    assert(m_height > 0);
                    m_buffer = new char[m_width * m_height];
                    std::memset(m_buffer, 0, m_width * m_height);
                }
                
                TextureBitmap(const TextureBitmap& other) :
                m_height(other.m_height),
                m_width(other.m_width),
                m_buffer(NULL) {
                    m_buffer = new char[m_width * m_height];
                    std::memcpy(m_buffer, other.m_buffer, m_width * m_height);
                }
                
                TextureBitmap& operator=(TextureBitmap other) {
                    m_width = other.m_width;
                    m_height = other.m_height;
                    std::swap(m_buffer, other.m_buffer);
                    return *this;
                }
                
                ~TextureBitmap() {
                    m_width = 0;
                    m_height = 0;
                    delete [] m_buffer;
                    m_buffer = NULL;
                }
                
                inline size_t width() const {
                    return m_width;
                }
                
                inline size_t height() const {
                    return m_height;
                }
                
                inline const char* buffer() const {
                    return m_buffer;
                }
                
                inline void drawGlyph(const int x, const int y, const int rowHeight, const FT_GlyphSlot glyph) {
                    const size_t left = static_cast<size_t>(x + glyph->bitmap_left);
                    const size_t top = static_cast<size_t>(y + rowHeight - glyph->bitmap_top);
                    const size_t rows = static_cast<size_t>(glyph->bitmap.rows);
                    const size_t width = static_cast<size_t>(glyph->bitmap.width);
                    const size_t pitch = static_cast<size_t>(glyph->bitmap.pitch);
                    
                    for (size_t r = 0; r < rows; r++) {
                        const size_t index = (r + top) * m_width + left;
                        std::memcpy(m_buffer + index, glyph->bitmap.buffer + r * pitch, width);
                    }
                }
            };
        }
    }
}

#endif
