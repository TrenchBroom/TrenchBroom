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

#include "CollapsibleTitledPanel.h"

#include "View/BorderLine.h"
#include "View/ViewConstants.h"

#include <wx/sizer.h>
#include <wx/stattext.h>

wxDEFINE_EVENT(TITLE_BAR_CLICK, wxCommandEvent);

namespace TrenchBroom {
    namespace View {
        CollapsibleTitleBar::CollapsibleTitleBar(wxWindow* parent, const wxString& title, const wxString& stateText) :
        TitleBar(parent, title, LayoutConstants::NarrowHMargin, LayoutConstants::NarrowVMargin),
        m_stateText(new wxStaticText(this, wxID_ANY, stateText)) {
            m_stateText->SetFont(m_titleText->GetFont());
            m_stateText->SetForegroundColour(*wxLIGHT_GREY);
            
            GetSizer()->Add(m_stateText, 0, wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            GetSizer()->AddSpacer(LayoutConstants::NarrowHMargin);
            Layout();
            
            Bind(wxEVT_LEFT_DOWN, &CollapsibleTitleBar::OnClick, this);
            m_titleText->Bind(wxEVT_LEFT_DOWN, &CollapsibleTitleBar::OnClick, this);
            m_stateText->Bind(wxEVT_LEFT_DOWN, &CollapsibleTitleBar::OnClick, this);
        }
        
        void CollapsibleTitleBar::setStateText(const wxString& stateText) {
            m_stateText->SetLabel(stateText);
            Layout();
        }
        
        void CollapsibleTitleBar::OnClick(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            wxCommandEvent newEvent(TITLE_BAR_CLICK, GetId());
            newEvent.SetEventObject(this);
            wxPostEvent(this, newEvent);
        }

        CollapsibleTitledPanel::CollapsibleTitledPanel(wxWindow* parent, const wxString& title, const bool initiallyExpanded) :
        wxPanel(parent),
        m_titleBar(new CollapsibleTitleBar(this, title, "hide")),
        m_divider(new BorderLine(this, BorderLine::Direction_Horizontal)),
        m_panel(new wxPanel(this)),
        m_expanded(initiallyExpanded) {
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_titleBar, 0, wxEXPAND);
            sizer->Add(m_divider, 0, wxEXPAND);
            sizer->Add(m_panel, 1, wxEXPAND);
            SetSizer(sizer);
            
            m_titleBar->Bind(TITLE_BAR_CLICK, &CollapsibleTitledPanel::OnTitleBarClick, this);
            
            update();
        }
        
        wxWindow* CollapsibleTitledPanel::getPanel() const {
            return m_panel;
        }

        void CollapsibleTitledPanel::expand() {
            setExpanded(true);
        }
        
        void CollapsibleTitledPanel::collapse() {
            setExpanded(false);
        }
        
        bool CollapsibleTitledPanel::expanded() const {
            return m_expanded;
        }
        
        void CollapsibleTitledPanel::setExpanded(const bool expanded) {
            if (expanded == m_expanded)
                return;
            
            m_expanded = expanded;
            update();
        }
        
        void CollapsibleTitledPanel::OnTitleBarClick(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            setExpanded(!m_expanded);
        }

        void CollapsibleTitledPanel::update() {
            if (m_expanded) {
                m_divider->Show();
                m_panel->wxWindowBase::ShowWithEffect(wxSHOW_EFFECT_ROLL_TO_BOTTOM);
                m_titleBar->setStateText("hide");
            } else {
                m_divider->Hide();
                m_panel->wxWindowBase::HideWithEffect(wxSHOW_EFFECT_ROLL_TO_TOP);
                m_titleBar->setStateText("show");
            }

            wxWindow* window = this;
            while (window != NULL) {
                window->Layout();
                window = window->GetParent();
            }
            
        }
    }
}
