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

#ifndef TrenchBroom_Font
#define TrenchBroom_Font

#include "VecMath.h"
#include "AttrString.h"
#include "FreeType.h"
#include "Renderer/FontGlyph.h"
#include "Renderer/FontGlyphBuilder.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class FontTexture;
        
        class TextureFont {
        public:
        private:
            FontTexture* m_texture;
            FontGlyph::List m_glyphs;
            size_t m_lineHeight;
            
            unsigned char m_firstChar;
            unsigned char m_charCount;
        public:
            TextureFont(FontTexture* texture, const FontGlyph::List& glyphs, size_t lineHeight, unsigned char firstChar, unsigned char charCount);
            ~TextureFont();
            
            Vec2f::List quads(const AttrString& string, bool clockwise, const Vec2f& offset = Vec2f::Null);
            Vec2f measure(const AttrString& string);

            Vec2f::List quads(const String& string, bool clockwise, const Vec2f& offset = Vec2f::Null);
            Vec2f measure(const String& string);
            
            void activate();
            void deactivate();
        };
    }
}


#endif /* defined(TrenchBroom_Font) */
