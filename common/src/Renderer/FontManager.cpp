/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "FontManager.h"
#include "CollectionUtils.h"
#include "Renderer/FreeTypeFontFactory.h"
#include "Renderer/TextureFont.h"

namespace TrenchBroom {
    namespace Renderer {
        FontManager::FontManager() :
        m_factory(new FreeTypeFontFactory()) {}
        
        FontManager::~FontManager() {
            MapUtils::clearAndDelete(m_cache);
            delete m_factory;
            m_factory = nullptr;
        }
        
        TextureFont& FontManager::font(const FontDescriptor& fontDescriptor) {
            FontCache::iterator it = m_cache.lower_bound(fontDescriptor);
            if (it != std::end(m_cache) && it->first.compare(fontDescriptor) == 0)
                return *it->second;

            TextureFont* font = m_factory->createFont(fontDescriptor);;
            m_cache.insert(it, std::make_pair(fontDescriptor, font));
            return *font;
        }
        
        FontDescriptor FontManager::selectFontSize(const FontDescriptor& fontDescriptor, const String& string, const float maxWidth, const size_t minFontSize) {
            FontDescriptor actualDescriptor = fontDescriptor;
            Vec2f actualBounds = font(actualDescriptor).measure(string);
            while (actualBounds.x() > maxWidth && actualDescriptor.size() > minFontSize) {
                actualDescriptor = FontDescriptor(actualDescriptor.path(), actualDescriptor.size() - 1);
                actualBounds = font(actualDescriptor).measure(string);
            }
            return actualDescriptor;
        }
    }
}
