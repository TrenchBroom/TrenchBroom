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

#ifndef TrenchBroom_ImageListBox
#define TrenchBroom_ImageListBox

#include <wx/vlbox.h>

namespace TrenchBroom {
    namespace View {
        class ImageListBox : public wxVListBox {
        private:
            wxSize m_imageSize;
            bool m_empty;
            wxString m_emptyText;
        public:
            ImageListBox(wxWindow* parent, const wxSize& imageSize, const wxString& emptyText, long style = wxBORDER_NONE);
            
            size_t selection() const;
            bool hasSelection() const;
            wxCoord itemHeight() const;
            
            void SetItemCount(size_t itemCount);
        private:
            void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;
            void drawItem(wxDC& dc, const wxRect& rect, size_t n) const;
            void drawEmptyItem(wxDC& dc, const wxRect& rect) const;
            
            void OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const;
            void OnDrawSeparator(wxDC& dc, wxRect& rect, size_t n) const;
            wxCoord OnMeasureItem(size_t n) const;
            
            virtual const wxBitmap& image(const size_t n) const = 0;
            virtual wxString title(const size_t n) const = 0;
            virtual wxString subtitle(const size_t n) const = 0;
        };
    }
}


#endif /* defined(TrenchBroom_ImageListBox) */
