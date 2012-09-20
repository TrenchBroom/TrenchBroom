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

#ifndef __TrenchBroom__StringVectorizer__
#define __TrenchBroom__StringVectorizer__

#include "Renderer/Text/FontDescriptor.h"
#include "Renderer/Text/Path.h"
#include "Utility/VecMath.h"

#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Utility {
        class Console;
    }
    
    namespace Renderer {
        namespace Text {
            class StringVectorizer {
            private:
                typedef std::map<FontDescriptor, FT_Face> FontCache;
                
                Utility::Console& m_console;
                FT_Library m_library;
                FontCache m_fontCache;
                
                FT_Face makeFont(const FontDescriptor& fontDescriptor);
                
                inline bool linearPoint(char tag) {
                    return (tag & 0x1) == 0x1;
                }
                
                inline bool quadraticBezierPoint(char tag) {
                    return (tag & 0x3) == 0x0;
                }
                
                inline bool cubicBezierPoint(char tag) {
                    return (tag & 0x3) == 0x2;
                }
                
                inline void setPoint(const FT_Vector* points, size_t index, Vec2f& result) {
                    result.x = points[index].x / 64.0f;
                    result.y = points[index].y / 64.0f;
                }
            public:
                StringVectorizer(Utility::Console& console);
                ~StringVectorizer();
                
                PathPtr makePath(const FontDescriptor& fontDescriptor, const String& string);
                Vec2f measureString(const FontDescriptor& fontDescriptor, const String& string);
            };
        }
    }
}

#endif /* defined(__TrenchBroom__StringVectorizer__) */
