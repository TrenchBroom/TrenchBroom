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

#include "BorderPanel.h"

#include "View/ViewConstants.h"

#include <wx/dcclient.h>

namespace TrenchBroom {
    namespace View {
        BorderPanel::BorderPanel(wxWindow* parent, const int borders) :
        wxPanel(parent, wxID_ANY),
        m_borders(borders) {
            Bind(wxEVT_PAINT, &BorderPanel::OnPaint, this);
        }
        
        BorderPanel::~BorderPanel() {}

        void BorderPanel::OnPaint(wxPaintEvent& event) {
            wxPaintDC dc(this);
            dc.SetPen(wxPen(Colors::borderColor()));
            
            wxRect rect = GetClientRect();
            if ((m_borders & wxLEFT) != 0)
                dc.DrawLine(rect.GetLeft(), rect.GetTop(), rect.GetLeft(), rect.GetBottom());
            if ((m_borders & wxTOP) != 0)
                dc.DrawLine(rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetTop());
            if ((m_borders & wxRIGHT) != 0)
                dc.DrawLine(rect.GetRight(), rect.GetTop(), rect.GetRight(), rect.GetBottom());
            if ((m_borders & wxBOTTOM) != 0)
                dc.DrawLine(rect.GetLeft(), rect.GetBottom(), rect.GetRight(), rect.GetBottom());
            event.Skip();
        }

        wxSize BorderPanel::DoGetBestSize() const {
            wxSize size = wxPanel::DoGetBestSize();
            if ((m_borders & wxLEFT) != 0)
                ++size.x;
            if ((m_borders & wxTOP) != 0)
                ++size.y;
            if ((m_borders & wxRIGHT) != 0)
                ++size.x;
            if ((m_borders & wxBOTTOM) != 0)
                ++size.y;
            return size;
        }
    }
}
