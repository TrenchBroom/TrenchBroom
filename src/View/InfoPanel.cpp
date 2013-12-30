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

#include "InfoPanel.h"

#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/Console.h"
#include "View/IssueBrowser.h"

#include <wx/toolbar.h>
#include <wx/toolbook.h>
#include <wx/sizer.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        InfoPanel::InfoPanel(wxWindow* parent, MapDocumentWPtr document) :
        wxPanel(parent),
        m_console(NULL),
        m_issueBrowser(NULL) {
            m_notebook = new wxToolbook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBK_LEFT);
            m_notebook->GetToolBar()->ToggleWindowStyle(wxTB_TEXT);
            
            const wxBitmap consoleIcon = IO::loadImageResource(IO::Path("images/Console.png"));
            const wxBitmap issueBrowserIcon = IO::loadImageResource(IO::Path("images/IssueBrowser.png"));
            
            assert(consoleIcon.IsOk());
            assert(issueBrowserIcon.IsOk());
            
            wxImageList* images = new wxImageList(16, 16);
            const int consoleIconIndex = images->Add(consoleIcon);
            const int issueBrowserIconIndex = images->Add(issueBrowserIcon);
            m_notebook->AssignImageList(images);
            
            assert(consoleIconIndex != -1);
            assert(issueBrowserIconIndex != -1);

            m_console = new Console(m_notebook);
            m_issueBrowser = new IssueBrowser(m_notebook, document);
            
            m_notebook->AddPage(m_console, _("Console"), true, consoleIconIndex);
            m_notebook->AddPage(m_issueBrowser, _("Issues"), false, issueBrowserIconIndex);
            
            wxSizer* notebookSizer = new wxBoxSizer(wxVERTICAL);
            notebookSizer->Add(m_notebook, 1, wxEXPAND);
            SetSizer(notebookSizer);
        }

        Logger* InfoPanel::logger() {
            return m_console;
        }
    }
}
