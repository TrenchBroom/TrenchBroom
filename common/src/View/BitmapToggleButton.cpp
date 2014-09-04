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

#include "BitmapToggleButton.h"

#include <wx/dcclient.h>
#include <wx/log.h> 

#include <algorithm>

namespace TrenchBroom {
    namespace View {
        BitmapToggleButton::BitmapToggleButton(wxWindow* parent, wxWindowID windowId, const wxBitmap& upBitmap, const wxBitmap& downBitmap) :
        m_state(false) {
            assert(upBitmap.IsOk());
            assert(downBitmap.IsOk());
            
            Create(parent, windowId);
            
            SetBitmap(upBitmap);
            SetBitmapPressed(downBitmap);
            SetMinClientSize(bitmapSize());
            
            Bind(wxEVT_PAINT, &BitmapToggleButton::OnPaint, this);
            Bind(wxEVT_LEFT_DOWN, &BitmapToggleButton::OnMouseDown, this);
        }
        
        void BitmapToggleButton::OnPaint(wxPaintEvent& event) {
            const wxSize size = GetClientSize();
            const wxSize bmpSize = bitmapSize();
            const wxSize delta = size - bmpSize;
            const wxPoint offset(delta.x / 2, delta.y / 2);
            
            wxPaintDC dc(this);
            dc.DrawBitmap(currentBitmap(), offset);
        }
        
        void BitmapToggleButton::OnMouseDown(wxMouseEvent& event) {
            if (!IsEnabled())
                return;
            
            m_state = !m_state;
            Refresh();
            
            wxCommandEvent buttonEvent(wxEVT_BUTTON, GetId());
            buttonEvent.SetEventObject(this);
            buttonEvent.SetInt(static_cast<int>(m_state));
            
            ProcessEvent(buttonEvent);
        }

        void BitmapToggleButton::DoUpdateWindowUI(wxUpdateUIEvent& event) {
            if (event.GetSetEnabled())
                Enable(event.GetEnabled());
            if (event.GetSetChecked()) {
                m_state = event.GetChecked();
                Refresh();
            }
        }

        wxSize BitmapToggleButton::bitmapSize() const {
            const wxBitmap upBitmap = GetBitmap();
            const wxBitmap downBitmap = GetBitmapPressed();
            return wxSize(std::max(upBitmap.GetWidth(), downBitmap.GetWidth()),
                          std::max(upBitmap.GetHeight(), downBitmap.GetHeight()));
        }

        wxBitmap BitmapToggleButton::currentBitmap() const {
            return m_state ? GetBitmapPressed() : GetBitmap();
        }
    }
}
