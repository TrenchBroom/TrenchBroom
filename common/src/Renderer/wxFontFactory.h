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

#ifndef __TrenchBroom__wxFontFactory__
#define __TrenchBroom__wxFontFactory__

#include "Renderer/FontFactory.h"

class wxBitmap;
class wxDC;

namespace TrenchBroom {
    namespace Renderer {
        class FontDescriptor;
        class TextureFont;
        
        class wxFontFactory : public FontFactory {
        private:
            TextureFont* doCreateFont(const FontDescriptor& fontDescriptor);

            TextureFont* buildFont(wxDC& dc, wxBitmap& buffer, unsigned char firstChar, unsigned char charCount);
            Metrics computeMetrics(wxDC& dc, unsigned char firstChar, unsigned char charCount) const;
        };
    }
}

#endif /* defined(__TrenchBroom__wxFontFactory__) */
