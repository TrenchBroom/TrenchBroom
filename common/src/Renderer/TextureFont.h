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

#ifndef __TrenchBroom__Font__
#define __TrenchBroom__Font__

#include "VecMath.h"
#include "FreeType.h"
#include "Renderer/GL.h"

#include <vector>


namespace TrenchBroom {
    namespace Renderer {
        class FontTexture;
        
        class TextureFont {
        private:
            static const int Border = 3;
            
            struct Char {
                int x, y, w, h, a;
                Char(const int i_x, const int i_y, const int i_w, const int i_h, const int i_a);
                
                void append(Vec2f::List& vertices, const int xOffset, const int yOffset, const int textureLength, const bool clockwise) const;
                float sMin(const size_t textureWidth) const;
                float sMax(const size_t textureWidth) const;
                float tMin(const size_t textureHeight) const;
                float tMax(const size_t textureHeight) const;
            };
            
            typedef std::vector<Char> CharList;
            CharList m_chars;
            
            unsigned char m_minChar;
            unsigned char m_maxChar;
            int m_lineHeight;
            
            GLuint m_textureId;
            int m_textureLength;
            FontTexture* m_texture;
        public:
            TextureFont(FT_Face face, const unsigned char minChar = ' ', const unsigned char maxChar = '~');
            ~TextureFont();
            
            Vec2f::List quads(const String& string, const bool clockwise, const Vec2f& offset = Vec2f());
            Vec2f measure(const String& string);
            
            void activate();
            void deactivate();
        };
    }
}


#endif /* defined(__TrenchBroom__Font__) */
