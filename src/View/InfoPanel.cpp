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
#include "View/IssueBrowser.h"

#include <wx/button.h>
#include <wx/simplebook.h>
#include <wx/sizer.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        InfoPanel::InfoPanel(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_console(NULL),
        m_issueBrowser(NULL) {
            m_mainBook = new wxSimplebook(this);
            m_extraBook = new wxSimplebook(this);
            
            m_consoleActiveBitmap = IO::loadImageResource(IO::Path("images/ConsolePressed.png"));
            m_consoleInactiveBitmap = IO::loadImageResource(IO::Path("images/Console.png"));
            m_issueBrowserActiveBitmap = IO::loadImageResource(IO::Path("images/IssueBrowserPressed.png"));
            m_issueBrowserInactiveBitmap = IO::loadImageResource(IO::Path("images/IssueBrowser.png"));
            
            assert(m_consoleActiveBitmap.IsOk());
            assert(m_consoleInactiveBitmap.IsOk());
            assert(m_issueBrowserActiveBitmap.IsOk());
            assert(m_issueBrowserInactiveBitmap.IsOk());
            
            m_console = new Console(m_mainBook, m_extraBook);
            m_issueBrowser = new IssueBrowser(m_mainBook, m_extraBook, document, controller);
            
            m_mainBook->AddPage(m_console, "Console");
            m_mainBook->AddPage(m_issueBrowser, "Issues");
            
            m_consoleButton = new wxButton(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBU_LEFT | wxBU_EXACTFIT | wxBORDER_NONE);
            m_consoleButton->SetBitmap(m_consoleActiveBitmap);
            m_issueBrowserButton = new wxButton(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBU_LEFT | wxBU_EXACTFIT | wxBORDER_NONE);
            m_issueBrowserButton->SetBitmap(m_issueBrowserInactiveBitmap);
            
            m_consoleButton->Bind(wxEVT_BUTTON, &InfoPanel::OnConsoleButtonPressed, this);
            m_issueBrowserButton->Bind(wxEVT_BUTTON, &InfoPanel::OnIssueBrowserButtonPressed, this);
            m_mainBook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &InfoPanel::OnNotebookPageChanged, this);
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->AddSpacer(8);
            buttonSizer->Add(m_consoleButton, 0, wxALIGN_CENTER_VERTICAL);
            buttonSizer->AddSpacer(8);
            buttonSizer->Add(m_issueBrowserButton, 0, wxALIGN_CENTER_VERTICAL);
            buttonSizer->AddSpacer(8);
            buttonSizer->Add(m_extraBook, 1, wxEXPAND);
            
            wxSizer* notebookSizer = new wxBoxSizer(wxVERTICAL);
            notebookSizer->Add(buttonSizer, 0, wxEXPAND);
            notebookSizer->AddSpacer(3);
            notebookSizer->Add(m_mainBook, 1, wxEXPAND);
            SetSizer(notebookSizer);
        }

        Logger* InfoPanel::logger() {
            return m_console;
        }

        void InfoPanel::OnConsoleButtonPressed(wxCommandEvent& event) {
            m_mainBook->SetSelection(0);
            m_extraBook->SetSelection(0);
        }
        
        void InfoPanel::OnIssueBrowserButtonPressed(wxCommandEvent& event) {
            m_mainBook->SetSelection(1);
            m_extraBook->SetSelection(1);
        }

        void InfoPanel::OnNotebookPageChanged(wxBookCtrlEvent& event) {
            if (event.GetSelection() == 0) {
                m_consoleButton->SetBitmap(m_consoleActiveBitmap);
                m_issueBrowserButton->SetBitmap(m_issueBrowserInactiveBitmap);
            } else {
                m_consoleButton->SetBitmap(m_consoleInactiveBitmap);
                m_issueBrowserButton->SetBitmap(m_issueBrowserActiveBitmap);
            }
        }
    }
}
