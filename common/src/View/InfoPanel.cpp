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

#include "InfoPanel.h"

#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/Console.h"
#include "View/ContainerBar.h"
#include "View/IssueBrowser.h"
#include "View/TabBar.h"
#include "View/TabBook.h"

#include <wx/sizer.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        InfoPanel::InfoPanel(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_tabBook(NULL),
        m_tabBar(NULL),
        m_console(NULL),
        m_issueBrowser(NULL) {
            m_tabBar = new ContainerBar(parent, wxTOP | wxBOTTOM);
            m_tabBook = new TabBook(this, m_tabBar);

            wxSizer* tabBarSizer = new wxBoxSizer(wxVERTICAL);
            tabBarSizer->Add(m_tabBook->tabBar(), 1, wxEXPAND);
            m_tabBar->SetSizer(tabBarSizer);
            
            m_console = new Console(m_tabBook);
            m_issueBrowser = new IssueBrowser(m_tabBook, document, controller);
            
            m_tabBook->addPage(m_console, "Console");
            m_tabBook->addPage(m_issueBrowser, "Issues");

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_tabBook, 1, wxEXPAND);
            SetSizer(sizer);
        }

        wxWindow* InfoPanel::tabBar() const {
            return m_tabBar;
        }

        Console* InfoPanel::console() const {
            return m_console;
        }
    }
}
