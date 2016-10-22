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

#include "PopupWindow.h"

#include <wx/display.h>
#include <wx/window.h>
#include <wx/app.h>

namespace TrenchBroom {
    namespace View {
        PopupWindow::PopupWindow() {}
        PopupWindow::PopupWindow(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, const long style, const wxString& name) {
            Create(parent, id, caption, pos, size, style, name);
        }

        bool PopupWindow::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, const long style, const wxString& name) {
            if (!wxFrame::Create(parent, id, caption, pos, size, style, name))
                return false;

            Bind(wxEVT_ACTIVATE, &PopupWindow::OnActivate, this);
            Bind(wxEVT_ACTIVATE_APP, &PopupWindow::OnActivate, this);
            return true;
        }

        void PopupWindow::Position(const wxPoint& origin, const wxSize& size) {
            // determine the position and size of the screen we clamp the popup to
            wxPoint posScreen;
            wxSize sizeScreen;

            const int displayNum = wxDisplay::GetFromPoint(origin);
            if ( displayNum != wxNOT_FOUND )
            {
                const wxRect rectScreen = wxDisplay(static_cast<unsigned int>(displayNum)).GetGeometry();
                posScreen = rectScreen.GetPosition();
                sizeScreen = rectScreen.GetSize();
            }
            else // outside of any display?
            {
                // just use the primary one then
                posScreen = wxPoint(0, 0);
                sizeScreen = wxGetDisplaySize();
            }


            const wxSize sizeSelf = GetSize();

            // is there enough space to put the popup below the window (where we put it
            // by default)?
            wxCoord y = origin.y + size.y;
            if ( y + sizeSelf.y > posScreen.y + sizeScreen.y )
            {
                // check if there is enough space above
                if ( origin.y > sizeSelf.y )
                {
                    // do position the control above the window
                    y -= size.y + sizeSelf.y;
                }
                //else: not enough space below nor above, leave below
            }

            // now check left/right too
            wxCoord x = origin.x;

            if ( wxTheApp->GetLayoutDirection() == wxLayout_RightToLeft )
            {
                // shift the window to the left instead of the right.
                x -= size.x;
                x += sizeSelf.x;        // also shift it by window width.
            }
            else
            {
                x += size.x;
                x -= sizeSelf.x;
            }


            if ( x + sizeSelf.x > posScreen.x + sizeScreen.x )
            {
                // check if there is enough space to the left
                if ( origin.x > sizeSelf.x )
                {
                    // do position the control to the left
                    x -= size.x + sizeSelf.x;
                }
                //else: not enough space there neither, leave in default position
            }

            Move(x, y, wxSIZE_NO_ADJUSTMENTS);
        }

        void PopupWindow::Popup() {
            Show();
        }

        void PopupWindow::Dismiss() {
            Hide();
        }

        void PopupWindow::OnActivate(wxActivateEvent& event) {
            if (!event.GetActive()) {
#ifndef _WIN32
                if (!IsDescendant(FindFocus()))
#endif
                    Dismiss();
            }
        }
    }
}
