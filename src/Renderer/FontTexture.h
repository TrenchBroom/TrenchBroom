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

#ifndef __TrenchBroom__FontTexture__
#define __TrenchBroom__FontTexture__

#include "FreeType.h"

#include <memory>

namespace TrenchBroom {
    namespace Renderer {
        class FontTexture {
        public:
            typedef std::auto_ptr<FontTexture> Ptr;
        private:
            size_t m_width;
            size_t m_height;
            char* m_buffer;
        public:
            FontTexture();
            FontTexture(const size_t width, const size_t height);
            FontTexture(const FontTexture& other);
            FontTexture& operator=(FontTexture other);
            ~FontTexture();
            
            size_t width() const;
            size_t height() const;
            const char* buffer() const;
            
            void drawGlyph(const int x, const int y, const int maxAscend, const FT_GlyphSlot glyph);
        };
    }
}

#endif /* defined(__TrenchBroom__FontTexture__) */
