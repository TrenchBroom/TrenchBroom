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

#include "TabBook.h"

#include "View/TabBar.h"

#include <wx/simplebook.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        TabBookPage::TabBookPage(wxWindow* parent) :
        wxPanel(parent) {}
        TabBookPage::~TabBookPage() {}
        
        wxWindow* TabBookPage::createTabBarPage(wxWindow* parent) {
            return new wxPanel(parent);
        }

        TabBook::TabBook(wxWindow* parent) :
        wxPanel(parent),
        m_tabBar(new TabBar(this)),
        m_tabBook(new wxSimplebook(this)) {
            m_tabBook->Bind(wxEVT_COMMAND_BOOKCTRL_PAGE_CHANGED, &TabBook::OnTabBookPageChanged, this);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_tabBar, 0, wxEXPAND);
            sizer->Add(m_tabBook, 1, wxEXPAND);
            SetSizer(sizer);
        }
        
        void TabBook::addPage(TabBookPage* page, const wxString& title) {
            assert(page != NULL);
            assert(page->GetParent() == this);
            
            RemoveChild(page);
            page->Reparent(m_tabBook);
            m_tabBook->AddPage(page, title);
            m_tabBar->addTab(page, title);
        }

        void TabBook::switchToPage(const size_t index) {
            assert(index < m_tabBook->GetPageCount());
            m_tabBook->SetSelection(index);
        }

        void TabBook::setTabBarHeight(const int height) {
            GetSizer()->SetItemMinSize(m_tabBar, wxDefaultCoord, height);
            Layout();
        }

        void TabBook::OnTabBookPageChanged(wxBookCtrlEvent& event) {
            if (IsBeingDeleted()) return;

            ProcessEvent(event);
            event.Skip();
        }
    }
}
