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

#include "TabBar.h"

#include "View/TabBook.h"
#include "View/ViewConstants.h"

#include <wx/button.h>
#include <wx/settings.h>
#include <wx/simplebook.h>
#include <wx/sizer.h>
#include <wx/statline.h>

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace View {
        TabBarButton::TabBarButton(wxWindow* parent, const wxString& label) :
        wxStaticText(parent, wxID_ANY, label),
        m_pressed(false) {
            SetFont(GetFont().Bold());
            Bind(wxEVT_LEFT_DOWN, &TabBarButton::OnClick, this);
        }
        
        void TabBarButton::setPressed(const bool pressed) {
            m_pressed = pressed;
            updateLabel();
        }

        void TabBarButton::OnClick(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            wxCommandEvent commandEvent(wxEVT_BUTTON, GetId());
            commandEvent.SetEventObject(this);
            ProcessEvent(commandEvent);
        }

        void TabBarButton::updateLabel() {
            if (m_pressed)
                SetForegroundColour(Colors::highlightText());
            else
                SetForegroundColour(Colors::defaultText());
            Refresh();
        }

        TabBar::TabBar(TabBook* tabBook) :
        ContainerBar(tabBook, wxBOTTOM),
        m_tabBook(tabBook),
        m_barBook(new wxSimplebook(this)),
        m_controlSizer(new wxBoxSizer(wxHORIZONTAL)) {
            assert(m_tabBook != NULL);
            m_tabBook->Bind(wxEVT_COMMAND_BOOKCTRL_PAGE_CHANGED, &TabBar::OnTabBookPageChanged, this);

            m_controlSizer->AddSpacer(LayoutConstants::TabBarBarLeftMargin);
            m_controlSizer->AddStretchSpacer();
            m_controlSizer->Add(m_barBook, 0, wxALIGN_CENTER_VERTICAL);
            m_controlSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            outerSizer->Add(m_controlSizer, 1, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::NarrowHMargin);
            
            SetSizer(outerSizer);
        }
        
        void TabBar::addTab(TabBookPage* bookPage, const wxString& title) {
            assert(bookPage != NULL);
            
            TabBarButton* button = new TabBarButton(this, title);
            button->Bind(wxEVT_BUTTON, &TabBar::OnButtonClicked, this);
            button->setPressed(m_buttons.empty());
            m_buttons.push_back(button);
            
            const size_t sizerIndex = 2 * (m_buttons.size() - 1) + 1;
            m_controlSizer->Insert(sizerIndex, button, 0, wxALIGN_CENTER_VERTICAL);
            m_controlSizer->InsertSpacer(sizerIndex + 1, LayoutConstants::WideHMargin);
            
            wxWindow* barPage = bookPage->createTabBarPage(m_barBook);
            m_barBook->AddPage(barPage, title);
            
            Layout();
        }
        
        void TabBar::OnButtonClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            wxWindow* button = static_cast<wxWindow*>(event.GetEventObject());
            const size_t index = findButtonIndex(button);
            assert(index < m_buttons.size());
            m_tabBook->switchToPage(index);
        }

        void TabBar::OnTabBookPageChanged(wxBookCtrlEvent& event) {
            if (IsBeingDeleted()) return;

            const int oldIndex = event.GetOldSelection();
            const int newIndex = event.GetSelection();
            
            setButtonInactive(oldIndex);
            setButtonActive(newIndex);
            m_barBook->SetSelection(static_cast<size_t>(newIndex));
        }

        size_t TabBar::findButtonIndex(wxWindow* button) const {
            for (size_t i = 0; i < m_buttons.size(); ++i) {
                if (m_buttons[i] == button)
                    return i;
            }
            return m_buttons.size();
        }

        void TabBar::setButtonActive(const int index) {
            m_buttons[static_cast<size_t>(index)]->setPressed(true);
        }
        
        void TabBar::setButtonInactive(const int index) {
            m_buttons[static_cast<size_t>(index)]->setPressed(false);
        }
    }
}
