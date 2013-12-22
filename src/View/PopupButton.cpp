/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "PopupButton.h"
#include "View/LayoutConstants.h"

#include <wx/tglbtn.h>
#include <wx/popupwin.h>
#include <wx/settings.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        class PopupWindow : public wxPopupTransientWindow {
        public:
            PopupWindow(wxWindow* parent) :
            wxPopupTransientWindow(parent) {}
            
            /*
            void Position(const wxPoint& desiredOrigin, const wxSize& buttonSize) {
                const int width = wxSystemSettings::GetMetric(wxSYS_SCREEN_X, this);
                const int height = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y, this);
                
                wxPoint origin = desiredOrigin;
                const wxSize size = GetSize();
                
                if (origin.x + size.x > width)
                    origin.x = width - size.x;
                if (origin.y + size.y > height) {
                    origin.y = desiredOrigin.y - buttonSize.y - size.y;
                    if (origin.y < 0) {
                        origin.x = desiredOrigin.x - size.x;
                        origin.y = height - size.y;
                    }
                }
                
                SetPosition(origin);
            }
             */
            
            bool ProcessLeftDown(wxMouseEvent& event) {
                const wxPoint mousePos = event.GetPosition();
                const wxPoint screenPos = ClientToScreen(event.GetPosition());
                wxWindow* window = wxFindWindowAtPoint(screenPos);
                wxLogDebug(window->GetClassInfo()->GetClassName());
                return wxPopupTransientWindow::ProcessLeftDown(event);
            }
        };
        
        PopupButton::PopupButton(wxWindow* parent, const wxString& caption, const Align popupAlign) :
        wxPanel(parent),
        m_popupAlign(popupAlign) {
            m_button = new wxToggleButton(this, wxID_ANY, caption, wxDefaultPosition, wxDefaultSize, LayoutConstants::ToggleButtonStyle | wxBU_EXACTFIT);
            m_window = new PopupWindow(this);

            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_button);
#ifdef __APPLE__
            sizer->SetItemMinSize(m_button, m_button->GetSize().x, m_button->GetSize().y + 1);
#endif
            SetSizerAndFit(sizer);
            
            m_button->Bind(wxEVT_TOGGLEBUTTON, &PopupButton::OnButtonToggled, this);
            m_window->Bind(wxEVT_SHOW, &PopupButton::OnPopupShow, this);
        }

        wxWindow* PopupButton::GetPopupWindow() const {
            return m_window;
        }

        void PopupButton::OnButtonToggled(wxCommandEvent& event) {
            if (m_button->GetValue()) {
                wxPoint position = GetScreenRect().GetRightBottom();
                position.x -= 2*m_window->GetSize().x;
                position.y -= m_window->GetSize().y;
                /*
                position.x = m_popupAlign == Left ? GetScreenRect().GetLeft() : GetScreenRect().GetRight() - m_window->GetSize().x;
                position.y = GetScreenRect().GetBottom();
#ifdef __APPLE__
                position.y += 1;
#endif
                 */
                m_window->Position(position, m_window->GetSize());
                m_window->Popup();
            } else {
                m_window->Dismiss();
            }
        }

        void PopupButton::OnPopupShow(wxShowEvent& event) {
            if (m_button->GetValue() != event.IsShown())
                m_button->SetValue(event.IsShown());
        }
    }
}
