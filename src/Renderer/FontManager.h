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

#ifndef __TrenchBroom__FontManager__
#define __TrenchBroom__FontManager__

#include "FreeType.h"
#include "Renderer/FontDescriptor.h"

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class TextureFont;
        
        class FontManager {
        private:
            typedef std::map<FontDescriptor, TextureFont*> FontCache;
            
            FT_Library m_library;
            FontCache m_cache;
        public:
            FontManager();
            ~FontManager();
            
            TextureFont& font(const FontDescriptor& fontDescriptor);
            FontDescriptor selectFontSize(const FontDescriptor& fontDescriptor, const String& string, const float maxWidth, const size_t minFontSize);
        };
    }
}

#endif /* defined(__TrenchBroom__FontManager__) */
