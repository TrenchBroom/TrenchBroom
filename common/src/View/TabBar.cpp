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

#include <cassert>
#include <iostream>

namespace TrenchBroom {
    namespace View {
        TabBar::TabBar(wxWindow* parent, TabBook* tabBook) :
        ContainerBar(parent, wxTOP | wxBOTTOM),
        m_tabBook(tabBook),
        m_barBook(new wxSimplebook(this)),
        m_controlSizer(new wxBoxSizer(wxHORIZONTAL)) {
            assert(m_tabBook != NULL);
            m_tabBook->Bind(wxEVT_COMMAND_BOOKCTRL_PAGE_CHANGED, &TabBar::OnTabBookPageChanged, this);

            m_controlSizer->AddSpacer(LayoutConstants::BarHorizontalMargin);
            m_controlSizer->AddStretchSpacer();
            m_controlSizer->Add(m_barBook, 0, wxALIGN_CENTER_VERTICAL);
            m_controlSizer->AddSpacer(LayoutConstants::BarHorizontalMargin);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::BarVerticalMargin);
            outerSizer->Add(m_controlSizer, 1, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::BarVerticalMargin);
            
            SetSizer(outerSizer);
        }
        
        void TabBar::addTab(const wxString& title, TabBookPage* bookPage) {
            assert(bookPage != NULL);
            
            wxButton* button = new wxButton(this, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_EXACTFIT);
            button->Bind(wxEVT_BUTTON, &TabBar::OnButtonClicked, this);
            m_buttons.push_back(button);
            
            m_controlSizer->Insert(m_buttons.size(), button, 0, wxALIGN_CENTER_VERTICAL);
            m_controlSizer->InsertSpacer(m_buttons.size() + 1, LayoutConstants::ControlHorizontalMargin);
            
            wxWindow* barPage = bookPage->createTabBarPage(m_barBook);
            m_barBook->AddPage(barPage, title);
            
            Layout();
        }
        
        void TabBar::OnButtonClicked(wxCommandEvent& event) {
            wxWindow* button = static_cast<wxWindow*>(event.GetEventObject());
            const size_t index = findButtonIndex(button);
            assert(index < m_buttons.size());
            m_tabBook->switchToPage(index);
        }

        void TabBar::OnTabBookPageChanged(wxBookCtrlEvent& event) {
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
            m_buttons[static_cast<size_t>(index)]->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
        }
        
        void TabBar::setButtonInactive(const int index) {
            m_buttons[static_cast<size_t>(index)]->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
        }
    }
}
