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

#include "ImageListBox.h"

#include "View/ViewConstants.h"

#include <wx/control.h>
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/settings.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ImageListBox::ImageListBox(wxWindow* parent, const wxSize& imageSize, const wxString& emptyText, const long style) :
        wxVListBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE | style),
        m_imageSize(imageSize),
        m_empty(true),
        m_emptyText(emptyText),
		m_border(2, 4) {}

		wxCoord ImageListBox::itemWidth(const wxString& subtitle) const {
			wxMemoryDC dc;
			dc.SetFont(*wxNORMAL_FONT);
			return dc.GetTextExtent(subtitle).x + 2 * m_border.x + m_imageSize.x + 8;
		}

		wxCoord ImageListBox::itemHeight() const {
			const int imageHeight = m_imageSize.y + 2 * m_border.y;
			int textHeight = 2 * m_border.y + 4;

			wxMemoryDC dc;
			dc.SetFont(wxNORMAL_FONT->Bold());
			textHeight += dc.GetTextExtent("Wgy").y;
			
			dc.SetFont(*wxNORMAL_FONT);
			textHeight += dc.GetTextExtent("Wgy").y;

            return std::max(imageHeight, textHeight);
        }

        void ImageListBox::SetItemCount(const size_t itemCount) {
            if (itemCount == 0) {
                m_empty = true;
                wxVListBox::SetItemCount(1);
            } else {
                m_empty = false;
                wxVListBox::SetItemCount(itemCount);
            }
        }

        void ImageListBox::OnDrawItem(wxDC& dc, const wxRect& rect, const size_t n) const {
            if (m_empty)
                drawEmptyItem(dc, rect);
            else
                drawItem(dc, rect, n);
        }
        
        void ImageListBox::drawItem(wxDC& dc, const wxRect& rect, const size_t n) const {
            const wxBitmap& img = image(n);
            const wxString ttl = title(n);
            const wxString sub = subtitle(n);
            
            dc.DrawBitmap(img, rect.GetLeft() + m_border.x, rect.GetTop() + m_border.y, true);
            
			int yOff = rect.GetTop() + m_border.y;

			if (IsSelected(n))
                dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT));
            else
                dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
            
            dc.SetFont(wxNORMAL_FONT->Bold());
            const wxString shortTtl = wxControl::Ellipsize(ttl, dc, wxELLIPSIZE_MIDDLE, rect.GetWidth() - (rect.GetLeft() + m_imageSize.x + 8 + 6));
            dc.DrawText(shortTtl, rect.GetLeft() + m_imageSize.x + 8, yOff);

			yOff += dc.GetTextExtent("Wgy").y + 4;

            dc.SetFont(*wxSMALL_FONT);
            const wxString shortSub = wxControl::Ellipsize(sub, dc, wxELLIPSIZE_MIDDLE, rect.GetWidth() - (rect.GetLeft() + m_imageSize.x + 8 + 6));
            dc.DrawText(shortSub, rect.GetLeft() + m_imageSize.x + 8, yOff);
        }
        
        void ImageListBox::drawEmptyItem(wxDC& dc, const wxRect& rect) const {
            dc.SetFont(wxNORMAL_FONT->Larger().Larger().Bold());
            const wxSize textSize = dc.GetTextExtent(m_emptyText);
            
            const int x = (rect.GetWidth() - textSize.x) / 2;
            const int y = (rect.GetHeight() - textSize.y) / 2;
            dc.SetTextForeground(*wxLIGHT_GREY);
            dc.DrawText(m_emptyText, x, y);
        }

        void ImageListBox::OnDrawBackground(wxDC& dc, const wxRect& rect, const size_t n) const {
            assert(n < GetItemCount());
            dc.SetPen(*wxTRANSPARENT_PEN);
            if (IsSelected(n))
                dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
            else
                dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX)));
            dc.DrawRectangle(rect);
        }
        
        void ImageListBox::OnDrawSeparator(wxDC& dc, wxRect& rect, const size_t n) const {
            if (m_empty)
                return;
            
            assert(n < GetItemCount());
            dc.SetPen(Colors::borderColor());
            dc.DrawLine(rect.GetLeft(), rect.GetBottom(), rect.GetRight(), rect.GetBottom());
            rect.Deflate(0, 1);
        }
        
        wxCoord ImageListBox::OnMeasureItem(const size_t n) const {
            return itemHeight();
        }
    }
}
