/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TrenchBroom_BitmapButton
#define TrenchBroom_BitmapButton

#include <wx/bitmap.h>
#include <wx/control.h>

namespace TrenchBroom {
    namespace View {
        class BitmapButton : public wxControl {
        private:
            wxBitmap m_bitmap;
            wxBitmap m_disabledBitmap;
        public:
            BitmapButton(wxWindow* parent, wxWindowID windowId, const wxBitmap& bitmap);
            
            void OnPaint(wxPaintEvent& event);
            void OnMouseDown(wxMouseEvent& event);
            void DoUpdateWindowUI(wxUpdateUIEvent& event);
        private:
            wxSize bitmapSize() const;
            wxBitmap currentBitmap() const;
        };
    }
}

#endif /* defined(TrenchBroom_BitmapButton) */
