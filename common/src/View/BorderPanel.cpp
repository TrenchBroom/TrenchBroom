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
        IMPLEMENT_DYNAMIC_CLASS(BorderPanel, wxPanel)

        BorderPanel::BorderPanel() :
        wxPanel(),
        m_borders(0),
        m_thickness(1) {}
        
        BorderPanel::BorderPanel(wxWindow* parent, const int borders, const int thickness) :
        wxPanel(),
        m_borders(0),
        m_thickness(1) {
            Create(parent, borders, thickness);
        }
        
        BorderPanel::~BorderPanel() {}

        void BorderPanel::Create(wxWindow* parent, int borders, int thickness) {
            wxPanel::Create(parent);
            m_borders = borders;
            m_thickness = thickness;
            Bind(wxEVT_PAINT, &BorderPanel::OnPaint, this);
        }

        void BorderPanel::OnPaint(wxPaintEvent& event) {
            if (IsBeingDeleted()) return;

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
                size.x += m_thickness;
            if ((m_borders & wxTOP) != 0)
                size.y += m_thickness;
            if ((m_borders & wxRIGHT) != 0)
                size.x += m_thickness;
            if ((m_borders & wxBOTTOM) != 0)
                size.y += m_thickness;
            return size;
        }
    }
}
