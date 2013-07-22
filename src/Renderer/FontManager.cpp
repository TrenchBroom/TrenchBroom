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

#include "FontManager.h"
#include "Exceptions.h"
#include "CollectionUtils.h"
#include "IO/FileSystem.h"
#include "IO/Path.h"
#include "Renderer/TextureFont.h"

namespace TrenchBroom {
    namespace Renderer {
        FontManager::FontManager() :
        m_library(NULL) {
            FT_Error error = FT_Init_FreeType(&m_library);
            if (error != 0) {
                m_library = NULL;
                
                RenderException e;
                e << "Error initializing FreeType: " << error;
                throw e;
            }
        }
        
        FontManager::~FontManager() {
            MapUtils::clearAndDelete(m_cache);
            if (m_library != NULL) {
                FT_Done_FreeType(m_library);
                m_library = NULL;
            }
        }
        
        TextureFont& FontManager::font(const FontDescriptor& fontDescriptor) {
            FontCache::iterator it = m_cache.lower_bound(fontDescriptor);
            if (it != m_cache.end() && it->first.compare(fontDescriptor) == 0)
                return *it->second;
            
            IO::FileSystem fs;
            const IO::Path fontPath = fs.findFontFile(fontDescriptor.name());
            
            FT_Face face;
            FT_Error error = FT_New_Face(m_library, fontPath.asString().c_str(), 0, &face);
            if (error != 0) {
                RenderException e;
                e << "Error loading font '" << fontDescriptor.name() << "': " << error;
                throw e;
            }
            
            FT_Set_Pixel_Sizes(face, 0, fontDescriptor.size());
            // FT_Set_Char_Size(face, 0, fontDescriptor.size() * 64, 72, 72);
            
            TextureFont* font = new TextureFont(face);
            m_cache.insert(it, std::make_pair(fontDescriptor, font));
            FT_Done_Face(face);
            return *font;
        }
        
        FontDescriptor FontManager::selectFontSize(const FontDescriptor& fontDescriptor, const String& string, const float maxWidth, const size_t minFontSize) {
            FontDescriptor actualDescriptor = fontDescriptor;
            Vec2f actualBounds = font(actualDescriptor).measure(string);
            while (actualBounds.x() > maxWidth && actualDescriptor.size() > minFontSize) {
                actualDescriptor = FontDescriptor(actualDescriptor.name(), actualDescriptor.size() - 1);
                actualBounds = font(actualDescriptor).measure(string);
            }
            return actualDescriptor;
        }
    }
}
