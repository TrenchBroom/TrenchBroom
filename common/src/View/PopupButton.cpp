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

#include "PopupButton.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/frame.h>
#include <wx/tglbtn.h>
#include <wx/popupwin.h>
#include <wx/settings.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        PopupButton::PopupButton(wxWindow* parent, const wxString& caption) :
        wxPanel(parent) {
            m_button = new wxToggleButton(this, wxID_ANY, caption, wxDefaultPosition, wxDefaultSize, LayoutConstants::ToggleButtonStyle | wxBU_EXACTFIT);
            
            wxFrame* frame = findFrame(this);
            m_window = new wxPopupTransientWindow(frame);

#if defined __APPLE__
            m_window->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif

            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_button);
#ifdef __APPLE__
            sizer->SetItemMinSize(m_button, m_button->GetSize().x, m_button->GetSize().y + 1);
#endif
#ifdef __linux__
            sizer->SetItemMinSize(m_button, m_button->GetSize().x + 3, m_button->GetSize().y);
#endif
            SetSizerAndFit(sizer);
            
            m_button->Bind(wxEVT_TOGGLEBUTTON, &PopupButton::OnButtonToggled, this);
            m_window->Bind(wxEVT_SHOW, &PopupButton::OnPopupShow, this);
        }

        wxWindow* PopupButton::GetPopupWindow() const {
            return m_window;
        }

        void PopupButton::OnButtonToggled(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_button->GetValue()) {
                wxPoint position = GetScreenRect().GetRightBottom();
                position.x -= 2*m_window->GetSize().x;
                position.y -= m_window->GetSize().y;
                m_window->Position(position, m_window->GetSize());
                m_window->Popup();
            } else {
                m_window->Dismiss();
            }
        }

        void PopupButton::OnPopupShow(wxShowEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_button->GetValue() != event.IsShown())
                m_button->SetValue(event.IsShown());
            event.Skip();
        }

        bool PopupButton::Enable(bool enable) {
            if (wxPanel::Enable(enable)) {
                m_button->Enable(enable);
                return true;
            }
            return false;
        }
    }
}
