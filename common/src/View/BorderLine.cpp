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

#include "BorderLine.h"

#include "View/ViewConstants.h"

#include <wx/dcclient.h>

namespace TrenchBroom {
    namespace View {
        BorderLine::BorderLine(wxWindow* parent, Direction direction, const int thickness) :
        wxWindow(parent, wxID_ANY) {
            SetForegroundColour(Colors::borderColor());
            SetBackgroundStyle(wxBG_STYLE_PAINT);
            Bind(wxEVT_PAINT, &BorderLine::OnPaint, this);
            if (direction == Direction_Horizontal) {
                const wxSize size(wxSize(wxDefaultSize.x, thickness));
                SetMinSize(size);
                SetMaxSize(size);
            } else {
                const wxSize size(wxSize(thickness, wxDefaultSize.y));
                SetMinSize(size);
                SetMaxSize(size);
            }
        }
        
        void BorderLine::OnPaint(wxPaintEvent& event) {
            if (IsBeingDeleted()) return;

            wxPaintDC dc(this);
            dc.SetPen(wxPen(GetForegroundColour()));
            dc.SetBrush(wxBrush(GetForegroundColour()));

            dc.DrawRectangle(GetClientRect());
            event.Skip();
        }
    }
}
