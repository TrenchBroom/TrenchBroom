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

#include "ImagePanel.h"

#include <wx/dcclient.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ImagePanel::ImagePanel(wxWindow* parent) :
        wxPanel(parent) {
            SetBackgroundStyle(wxBG_STYLE_PAINT);
            Bind(wxEVT_PAINT, &ImagePanel::OnPaint, this);
        }
        
        void ImagePanel::SetImage(const wxBitmap& bitmap) {
            assert(bitmap.IsOk());
            m_bitmap = bitmap;
            SetMinSize(m_bitmap.GetSize());
            Refresh();
        }
        
        void ImagePanel::OnPaint(wxPaintEvent& event) {
            wxClientDC dc(this);
            dc.SetPen(wxPen(GetBackgroundColour()));
            dc.SetBrush(wxBrush(GetBackgroundColour()));
            dc.DrawRectangle(GetRect());
            dc.DrawBitmap(m_bitmap, 0, 0, true);
        }
    }
}
