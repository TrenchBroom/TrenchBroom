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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__ImageListBox__
#define __TrenchBroom__ImageListBox__

#include <wx/vlbox.h>

namespace TrenchBroom {
    namespace View {
        class ImageListBox : public wxVListBox {
        public:
            ImageListBox(wxWindow* parent);
            
            size_t selection() const;
            bool hasSelection() const;
        private:
            void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;
            void OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const;
            void OnDrawSeparator(wxDC& dc, wxRect& rect, size_t n) const;
            wxCoord OnMeasureItem(size_t n) const;
            
            virtual const wxBitmap& image(const size_t n) const = 0;
            virtual wxString title(const size_t n) const = 0;
            virtual wxString subtitle(const size_t n) const = 0;
        };
    }
}


#endif /* defined(__TrenchBroom__ImageListBox__) */
