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

#ifndef TrenchBroom_FontGlyphBuilder
#define TrenchBroom_FontGlyphBuilder

#include "Renderer/FontGlyph.h"

namespace TrenchBroom {
    namespace Renderer {
        class FontTexture;
        
        class FontGlyphBuilder {
        private:
            size_t m_maxAscend;
            size_t m_cellSize;
            size_t m_margin;
            size_t m_textureSize;
            char* m_textureBuffer;
            
            size_t m_x;
            size_t m_y;
        public:
            FontGlyphBuilder(size_t maxAscend, size_t cellSize, size_t margin, FontTexture& texture);
            
            FontGlyph createGlyph(size_t left, size_t top, size_t width, size_t height, size_t advance, const char* glyphBuffer, size_t pitch);
        private:
            void drawGlyph(size_t left, size_t top, size_t width, size_t height, const char* glyphBuffer, size_t pitch);
        };
    }
}

#endif /* defined(TrenchBroom_FontGlyphBuilder) */
