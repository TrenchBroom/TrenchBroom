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

#ifndef __TrenchBroom__TexturedFont__
#define __TrenchBroom__TexturedFont__

#include "GL/glew.h"
#include "Renderer/Text/FontDescriptor.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            class TextureBitmap;
            
            class TexturedFont {
            private:
                static const int Border = 3;
                
                struct Char {
                    int x, y, w, h, a;
                    Char(int i_x, int i_y, int i_w, int i_h, int i_a) :
                    x(i_x),
                    y(i_y),
                    w(i_w),
                    h(i_h),
                    a(i_a) {}
                    
                    inline void append(Vec2f::List& vertices, int xOffset, int yOffset, int textureLength, bool clockwise) const {
                        if (clockwise) {
                            vertices.push_back(Vec2f(xOffset, yOffset));
                            vertices.push_back(Vec2f(x, y + h) / static_cast<float>(textureLength));
                            vertices.push_back(Vec2f(xOffset, yOffset + h));
                            vertices.push_back(Vec2f(x, y) / static_cast<float>(textureLength));
                            vertices.push_back(Vec2f(xOffset + w, yOffset + h));
                            vertices.push_back(Vec2f(x + w, y) / static_cast<float>(textureLength));
                            vertices.push_back(Vec2f(xOffset + w, yOffset));
                            vertices.push_back(Vec2f(x + w, y + h) / static_cast<float>(textureLength));
                        } else {
                            vertices.push_back(Vec2f(xOffset, yOffset));
                            vertices.push_back(Vec2f(x, y + h) / static_cast<float>(textureLength));
                            vertices.push_back(Vec2f(xOffset + w, yOffset));
                            vertices.push_back(Vec2f(x + w, y + h) / static_cast<float>(textureLength));
                            vertices.push_back(Vec2f(xOffset + w, yOffset + h));
                            vertices.push_back(Vec2f(x + w, y) / static_cast<float>(textureLength));
                            vertices.push_back(Vec2f(xOffset, yOffset + h));
                            vertices.push_back(Vec2f(x, y) / static_cast<float>(textureLength));
                        }
                    }
                    
                    inline float sMin(size_t textureWidth) const {
                        return static_cast<float>(x) / static_cast<float>(textureWidth);
                    }

                    inline float sMax(size_t textureWidth) const {
                        return static_cast<float>(x + w) / static_cast<float>(textureWidth);
                    }

                    inline float tMin(size_t textureHeight) const {
                        return static_cast<float>(y) / static_cast<float>(textureHeight);
                    }
                    
                    inline float tMax(size_t textureHeight) const {
                        return static_cast<float>(y + h) / static_cast<float>(textureHeight);
                    }
                };
                
                typedef std::vector<Char> CharList;
                CharList m_chars;
                
                unsigned char m_minChar;
                unsigned char m_maxChar;
                int m_lineHeight;
                
                GLuint m_textureId;
                int m_textureLength;
                TextureBitmap* m_bitmap;
            public:
                TexturedFont(FT_Face face, const unsigned char minChar = ' ', const unsigned char maxChar = '~');
                ~TexturedFont();
                
                Vec2f::List quads(const String& string, bool clockwise, const Vec2f& offset = Vec2f());
                Vec2f measure(const String& string);
                
                void activate();
                void deactivate();
            };
        }
    }
}

#endif /* defined(__TrenchBroom__TexturedFont__) */
