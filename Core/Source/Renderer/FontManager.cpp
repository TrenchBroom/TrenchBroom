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

#include "FontManager.h"
#include <cassert>
#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {
        bool operator<(FontDescriptor const& left, FontDescriptor const& right) {
            int cmp = left.name.compare(right.name);
            if (cmp < 0) return true;
            if (cmp > 0) return false;
            return left.size < right.size;
        }

        FontManager::FontManager() {}

        FontManager::~FontManager() {
            clear();
        }

        FontPtr FontManager::font(const FontDescriptor& fontDescriptor) {
            FontCache::iterator it = m_fontCache.find(fontDescriptor);
            if (it == m_fontCache.end()) {
                std::string fontPath = resolveFont(fontDescriptor.name);
                FTFont* font = new FTTextureFont(fontPath.c_str());
                font->FaceSize(fontDescriptor.size);
                font->UseDisplayList(true);
                FontPtr fontPtr(font);
                m_fontCache[fontDescriptor] = fontPtr;
                return fontPtr;
            } else {
                return it->second;
            }
            
        }

        void FontManager::clear() {
            m_fontCache.clear();
        }
    }
}
