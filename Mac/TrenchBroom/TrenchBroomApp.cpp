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

#include "wx/config.h"

#include "Model/MapDocument.h"
#include "View/EditorView.h"

IMPLEMENT_APP(TrenchBroomApp)

BEGIN_EVENT_TABLE(TrenchBroomApp, wxApp)
EVT_MENU    (wxID_NEW, TrenchBroomApp::OnFileNew)
EVT_MENU    (wxID_OPEN, TrenchBroomApp::OnFileOpen)
END_EVENT_TABLE()

bool TrenchBroomApp::OnInit() {
	m_docManager = new wxDocManager;

    new wxDocTemplate(m_docManager, wxT("Quake map document"), wxT("*.map"), wxEmptyString, wxT("map"), wxT("Quake map document"), wxT("TrenchBroom editor view"), CLASSINFO(TrenchBroom::Model::MapDocument), CLASSINFO(TrenchBroom::View::EditorView));

#ifdef __APPLE__
    // don't close app when the last frame closes
    SetExitOnFrameDelete(false);
    
    // show menu bar even when no frame is open
    wxMenuBar* menuBar = new wxMenuBar();
    
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_NEW, wxT("New\tCtrl-N"));
    fileMenu->Append(wxID_OPEN, wxT("Open...\tCtrl-O"));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_CLOSE, wxT("Close\tCtrl-W"));
    fileMenu->Append(wxID_SAVE, wxT("Save\tCtrl-S"));
    fileMenu->Append(wxID_SAVEAS, wxT("Save as...\tCtrl-Shift-S"));
    
    // file history menu
    m_docManager->FileHistoryUseMenu(fileMenu);
    m_docManager->FileHistoryLoad(*wxConfig::Get());

    // won't show up in the app's menu if we don't add them here
    fileMenu->Append(wxID_ABOUT, wxT("About"));
    fileMenu->Append(wxID_PREFERENCES, wxT("Preferences...\tCtrl-,"));
    fileMenu->Append(wxID_EXIT, wxT("Exit"));

    menuBar->Append(fileMenu, wxT("File"));
    wxMenuBar::MacSetCommonMenuBar(menuBar);
#endif
    
    return true;
}

int TrenchBroomApp::OnExit() {
    wxDELETE(m_docManager);
    return wxApp::OnExit();
}

void TrenchBroomApp::OnFileNew(wxCommandEvent& event) {
    m_docManager->OnFileNew(event);
}

void TrenchBroomApp::OnFileOpen(wxCommandEvent& event) {
    m_docManager->OnFileOpen(event);
}

void TrenchBroomApp::OnUnhandledException() {
    try {
        throw;
    } catch (std::exception& e) {
        wxLogWarning(e.what());
    }
}
