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

#include "wxFontFactory.h"

#include "Renderer/FontDescriptor.h"
#include "Renderer/FontTexture.h"
#include "Renderer/TextureFont.h"

#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/font.h>

namespace TrenchBroom {
    namespace Renderer {
        TextureFont* wxFontFactory::doCreateFont(const FontDescriptor& fontDescriptor) {
            const wxFontInfo info = wxFontInfo(static_cast<int>(fontDescriptor.size())).FaceName(fontDescriptor.name());
            const wxFont wxf(info);
            
            const size_t bufferSize = 3 * fontDescriptor.size();
            wxBitmap buffer(static_cast<int>(bufferSize), static_cast<int>(bufferSize), 8);
            wxMemoryDC dc(buffer);
            dc.SetFont(wxf);

            return buildFont(dc, buffer, fontDescriptor.minChar(), fontDescriptor.charCount());
        }

        TextureFont* wxFontFactory::buildFont(wxDC& dc, wxBitmap& buffer, unsigned char firstChar, unsigned char charCount) {
            const Metrics metrics = computeMetrics(dc, firstChar, charCount);
            
            dc.SetBackground(*wxBLACK_BRUSH);
            dc.SetTextForeground(*wxWHITE);
            dc.SetTextBackground(*wxBLACK);
            
            FontTexture* texture = new FontTexture(charCount, metrics.cellSize, metrics.lineHeight);
            FontGlyphBuilder glyphBuilder(metrics.maxAscend, metrics.cellSize, 3, *texture);
            
            FontGlyph::List glyphs;
            wxCoord width, height, descend, externalLeading;

            for (unsigned char c = firstChar; c < firstChar + charCount; ++c) {
                wxString str;
                str << c;
                
                dc.Clear();
                dc.DrawText(str, 5, 5);
                dc.GetTextExtent(str, &width, &height, &descend, &externalLeading);
                
                glyphs.push_back(glyphBuilder.createGlyph(5, 5,
                                                          width, height,
                                                          width,
                                                          ,
                                                          width));
            }

            return new TextureFont(texture, glyphs, metrics.lineHeight, firstChar, charCount);
        }

        wxFontFactory::Metrics wxFontFactory::computeMetrics(wxDC& dc, unsigned char firstChar, unsigned char charCount) const {
            
            int maxWidth = 0;
            int maxAscend = 0;
            int maxDescend = 0;
            int lineHeight = 0;
            wxCoord width, height, descend, externalLeading;
            
            for (unsigned char c = firstChar; c < firstChar + charCount; ++c) {
                wxString str;
                str << c;
                dc.GetTextExtent(str, &width, &height, &descend, &externalLeading);
                
                maxWidth = std::max(maxWidth, width);
                maxAscend = std::max(maxAscend, height - descend);
                maxDescend = std::max(maxDescend, descend);
                lineHeight = std::max(lineHeight, height);
            }

            const int cellSize = std::max(maxWidth, maxAscend + maxDescend);
            
            Metrics metrics;
            metrics.cellSize = static_cast<size_t>(cellSize);
            metrics.maxAscend = static_cast<size_t>(maxAscend);
            metrics.lineHeight = static_cast<size_t>(lineHeight);
            return metrics;
        }
    }
}
