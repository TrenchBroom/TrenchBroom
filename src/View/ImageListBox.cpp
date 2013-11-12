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

#include "ImageListBox.h"

#include <wx/control.h>
#include <wx/dc.h>
#include <wx/settings.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ImageListBox::ImageListBox(wxWindow* parent) :
        wxVListBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE) {
        }

        void ImageListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const {
            const wxBitmap& img = image(n);
            const wxString ttl = title(n);
            const wxString sub = subtitle(n);
            const wxString shortSub = wxControl::Ellipsize(sub, dc, wxELLIPSIZE_MIDDLE, rect.GetWidth() - 40 - 4);
            
            dc.DrawBitmap(img, rect.GetLeft() + 2, rect.GetTop() + 4, true);

            if (IsSelected(n))
                dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT));
            else
                dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
            
            dc.SetFont(wxNORMAL_FONT->Bold());
            dc.DrawText(ttl, rect.GetLeft() + 40, rect.GetTop() + 2);
            dc.SetFont(*wxSMALL_FONT);
            dc.DrawText(shortSub, rect.GetLeft() + 40, rect.GetTop() + 20);
        }
        
        void ImageListBox::OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const {
            assert(n < GetItemCount());
            dc.SetPen(*wxTRANSPARENT_PEN);
            if (IsSelected(n))
                dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
            else
                dc.SetBrush(*wxWHITE_BRUSH);
            dc.DrawRectangle(rect);
        }
        
        void ImageListBox::OnDrawSeparator(wxDC& dc, wxRect& rect, size_t n) const {
            assert(n < GetItemCount());
            dc.SetPen(*wxLIGHT_GREY_PEN);
            dc.DrawLine(rect.GetLeft(), rect.GetBottom(), rect.GetRight(), rect.GetBottom());
            rect.Deflate(0, 1);
        }
        
        wxCoord ImageListBox::OnMeasureItem(size_t n) const {
            assert(n < GetItemCount());
            return 33 + 8;
        }
    }
}
