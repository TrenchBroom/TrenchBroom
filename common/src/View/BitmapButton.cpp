/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "BitmapButton.h"

#include <wx/dcclient.h>
#include <wx/log.h>

#include <algorithm>

namespace TrenchBroom {
    namespace View {
        BitmapButton::BitmapButton(wxWindow* parent, wxWindowID windowId, const wxBitmap& bitmap) :
        wxControl(parent, windowId, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
        m_bitmap(bitmap),
        m_disabledBitmap(bitmap.ConvertToDisabled()) {
            assert(m_bitmap.IsOk());

            SetBackgroundColour(parent->GetBackgroundColour());
            SetBackgroundStyle(wxBG_STYLE_PAINT);
            SetMinClientSize(bitmapSize());
            
            Bind(wxEVT_PAINT, &BitmapButton::OnPaint, this);
            Bind(wxEVT_LEFT_DOWN, &BitmapButton::OnMouseDown, this);
        }
        
        void BitmapButton::OnPaint(wxPaintEvent& event) {
            if (IsBeingDeleted()) return;

            const wxSize size = GetClientSize();
            const wxSize bmpSize = bitmapSize();
            const wxSize delta = size - bmpSize;
            const wxPoint offset(delta.x / 2, delta.y / 2);
            
            wxPaintDC dc(this);
            dc.SetPen(wxPen(GetBackgroundColour()));
            dc.SetBrush(wxBrush(GetBackgroundColour()));
            dc.DrawRectangle(GetClientRect());
            dc.DrawBitmap(currentBitmap(), offset);
        }
        
        void BitmapButton::OnMouseDown(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            if (!IsEnabled())
                return;
            
            wxCommandEvent buttonEvent(wxEVT_BUTTON, GetId());
            buttonEvent.SetEventObject(this);
            buttonEvent.SetInt(1);
            
            ProcessEvent(buttonEvent);
        }

        void BitmapButton::DoUpdateWindowUI(wxUpdateUIEvent& event) {
            if (event.GetSetEnabled() && IsEnabled() != event.GetEnabled()) {
                Enable(event.GetEnabled());
                Refresh();
            }
        }

        wxSize BitmapButton::bitmapSize() const {
            return wxSize(m_bitmap.GetWidth(), m_bitmap.GetHeight());
        }

        wxBitmap BitmapButton::currentBitmap() const {
            return IsEnabled() ? m_bitmap : m_disabledBitmap;
        }
    }
}
