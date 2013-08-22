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

#ifndef __TrenchBroom__FontManager__
#define __TrenchBroom__FontManager__

#include "Renderer/Text/FontDescriptor.h"
#include "Utility/FreeType.h"
#include "Utility/VecMath.h"

#include <map>

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Utility {
        class Console;
    }

    namespace Renderer {
        namespace Text {
            class TexturedFont;

            class FontManager {
            private:
                typedef std::map<FontDescriptor, TexturedFont*> FontCache;

                Utility::Console& m_console;
                FT_Library m_library;
                FontCache m_cache;
            public:
                FontManager(Utility::Console& console);
                ~FontManager();

                TexturedFont* font(const FontDescriptor& fontDescriptor);

                FontDescriptor selectFontSize(const FontDescriptor& fontDescriptor, const String& string, const float maxWidth, unsigned int minFontSize);
            };
        }
    }
}

#endif /* defined(__TrenchBroom__FontManager__) */
