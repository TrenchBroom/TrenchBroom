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

#include "IO/FileManager.h"
#include "Renderer/Text/TexturedFont.h"
#include "Utility/Console.h"

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            FontManager::FontManager(Utility::Console& console) :
            m_library(NULL),
            m_console(console) {
                FT_Error error = FT_Init_FreeType(&m_library);
                if (error != 0) {
                    m_console.error("Error initializing FreeType (FT error: %i)", error);
                    m_library = NULL;
                }
            }
            
            FontManager::~FontManager() {
                if (m_library != NULL) {
                    FT_Done_FreeType(m_library);
                    m_library = NULL;
                }
            }

            TexturedFont* FontManager::font(const FontDescriptor& fontDescriptor) {
                FontCache::iterator it = m_cache.lower_bound(fontDescriptor);
                if (it != m_cache.end() && it->first.compare(fontDescriptor) == 0)
                    return it->second;
                
                IO::FileManager fileManager;
                String fontPath = fileManager.resolveFontPath(fontDescriptor.name());
                
                FT_Face face;
                FT_Error error = FT_New_Face(m_library, fontPath.c_str(), 0, &face);
                if (error != 0) {
                    m_console.error("Error loading font '%s', size %i (FT error: %i)", fontDescriptor.name().c_str(), fontDescriptor.size(), error);
                    return NULL;
                }
                
                FT_Set_Pixel_Sizes(face, 0, fontDescriptor.size());
                // FT_Set_Char_Size(face, 0, fontDescriptor.size() * 64, 72, 72);
                
                TexturedFont* font = new TexturedFont(face);
                m_cache.insert(it, std::pair<FontDescriptor, TexturedFont*>(fontDescriptor, font));
                FT_Done_Face(face);
                
                return font;
            }

            FontDescriptor FontManager::selectFontSize(const FontDescriptor& fontDescriptor, const String& string, const float maxWidth, unsigned int minFontSize) {
                FontDescriptor actualDescriptor = fontDescriptor;
                TexturedFont* actualFont = font(actualDescriptor);
                Vec2f actualBounds = actualFont->measure(string);
                while (actualBounds.x > maxWidth && actualDescriptor.size() > minFontSize) {
                    actualDescriptor = FontDescriptor(actualDescriptor.name(), actualDescriptor.size() - 1);
                    actualFont = font(actualDescriptor);
                    actualBounds = actualFont->measure(string);
                }
                return actualDescriptor;
            }
        }
    }
}
