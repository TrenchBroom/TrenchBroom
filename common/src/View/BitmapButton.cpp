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

namespace TrenchBroom {
    namespace View {
        BitmapButton::BitmapButton(wxWindow* parent, const wxWindowID windowId) :
        wxWindow(parent, windowId, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE) {
            InheritAttributes();

            Bind(wxEVT_PAINT, &BitmapButton::OnPaint, this);
            Bind(wxEVT_LEFT_DOWN, &BitmapButton::OnMouseDown, this);
        }

        BitmapButton::~BitmapButton() = default;

        bool BitmapButton::HasTransparentBackground() {
            return true;
        }

        bool BitmapButton::ShouldInheritColours() const {
            return true;
        }

        wxSize BitmapButton::DoGetBestClientSize() const {
            return bitmapSize();
        }

        void BitmapButton::OnPaint(wxPaintEvent& event) {
            if (IsBeingDeleted()) return;

            const wxSize size = GetClientSize();
            const wxSize bmpSize = bitmapSize();
            const wxSize delta = size - bmpSize;
            const wxPoint offset(delta.x / 2, delta.y / 2);

            wxPaintDC dc(this);
            dc.DrawBitmap(currentBitmap(), offset);
        }

        void BitmapButton::OnMouseDown(wxMouseEvent& event) {
            if (!IsBeingDeleted() && IsEnabled()) {
                processClick();
            }
        }

        void BitmapButton::DoUpdateWindowUI(wxUpdateUIEvent& event) {
            if (event.GetSetEnabled() && IsThisEnabled() != event.GetEnabled()) {
                Enable(event.GetEnabled());
                Refresh();
            }
        }

        wxSize BitmapButton::bitmapSize() const {
            return wxSize(currentBitmap().GetWidth(), currentBitmap().GetHeight());
        }
    }
}
