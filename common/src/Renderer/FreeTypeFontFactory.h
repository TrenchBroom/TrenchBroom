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

#ifndef TrenchBroom_FreeTypeFontFactory
#define TrenchBroom_FreeTypeFontFactory

#include "FreeType.h"
#include "Renderer/FontFactory.h"

namespace TrenchBroom {
    namespace Renderer {
        class FontDescriptor;
        class TextureFont;
        
        class FreeTypeFontFactory : public FontFactory {
        private:
            FT_Library m_library;
        public:
            FreeTypeFontFactory();
            ~FreeTypeFontFactory();
        private:
            TextureFont* doCreateFont(const FontDescriptor& fontDescriptor);
            
            FT_Face loadFont(const FontDescriptor& fontDescriptor);
            TextureFont* buildFont(FT_Face face, unsigned char firstChar, unsigned char charCount);

            Metrics computeMetrics(FT_Face face, unsigned char firstChar, unsigned char charCount) const;
        };
    }
}

#endif /* defined(TrenchBroom_FreeTypeFontFactory) */
