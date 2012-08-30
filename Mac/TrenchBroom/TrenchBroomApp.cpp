/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TrenchBroomApp.h"

#include <wx/config.h>
#include <wx/docview.h>

IMPLEMENT_APP(TrenchBroomApp)

BEGIN_EVENT_TABLE(TrenchBroomApp, AbstractApp)
EVT_MENU    (wxID_EXIT, TrenchBroomApp::OnFileExit)
END_EVENT_TABLE()

bool TrenchBroomApp::OnInit() {
    if (AbstractApp::OnInit()) {
        wxMenuBar* menuBar = new wxMenuBar();
        wxMenu* fileMenu = new wxMenu();
        fileMenu->Append(wxID_NEW, wxT("New\tCtrl-N"));
        fileMenu->Append(wxID_OPEN, wxT("Open...\tCtrl-O"));
        fileMenu->AppendSeparator();
        fileMenu->Append(wxID_CLOSE, wxT("Close\tCtrl-W"));
        fileMenu->Append(wxID_SAVE, wxT("Save\tCtrl-S"));
        fileMenu->Append(wxID_SAVEAS, wxT("Save as...\tCtrl-Shift-S"));
        
        m_docManager->FileHistoryUseMenu(fileMenu);
        m_docManager->FileHistoryLoad(*wxConfig::Get());

        // these won't show up in the app menu if we don't add them here
        fileMenu->Append(wxID_ABOUT, wxT("About"));
        fileMenu->Append(wxID_PREFERENCES, wxT("Preferences...\tCtrl-,"));
        fileMenu->Append(wxID_EXIT, wxT("Exit"));

        fileMenu->SetEventHandler(m_docManager);
        menuBar->Append(fileMenu, wxT("File"));

        wxMenuBar::MacSetCommonMenuBar(menuBar);
        SetExitOnFrameDelete(false);
        
        return true;
    }
    
    return false;
}


void TrenchBroomApp::OnFileExit(wxCommandEvent& event) {
    Exit();
}
