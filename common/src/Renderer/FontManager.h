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

#ifndef TrenchBroom_FontManager
#define TrenchBroom_FontManager

#include "Renderer/FontDescriptor.h"

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        class FontFactory;
        class TextureFont;
        
        class FontManager {
        private:
            typedef std::map<FontDescriptor, TextureFont*> FontCache;
            
            FontFactory* m_factory;
            FontCache m_cache;
        public:
            FontManager();
            ~FontManager();
            
            TextureFont& font(const FontDescriptor& fontDescriptor);
            FontDescriptor selectFontSize(const FontDescriptor& fontDescriptor, const String& string, const float maxWidth, const size_t minFontSize);
        };
    }
}

#endif /* defined(TrenchBroom_FontManager) */
