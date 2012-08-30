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

#include "AbstractApp.h"

#include "wx/config.h"
#include <wx/docview.h>
#include "wx/event.h"

#include "Model/MapDocument.h"
#include "View/EditorView.h"

BEGIN_EVENT_TABLE(AbstractApp, wxApp)
EVT_MENU    (wxID_EXIT, AbstractApp::OnFileExit)
END_EVENT_TABLE()

wxMenu* AbstractApp::CreateFileMenu() {
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_NEW, wxT("New\tCtrl-N"));
    fileMenu->Append(wxID_OPEN, wxT("Open...\tCtrl-O"));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_CLOSE, wxT("Close\tCtrl-W"));
    fileMenu->Append(wxID_SAVE, wxT("Save\tCtrl-S"));
    fileMenu->Append(wxID_SAVEAS, wxT("Save as...\tCtrl-Shift-S"));
    return fileMenu;
}

bool AbstractApp::OnInit() {
	m_docManager = new wxDocManager();
    new wxDocTemplate(m_docManager, wxT("Quake map document"), wxT("*.map"), wxEmptyString, wxT("map"), wxT("Quake map document"), wxT("TrenchBroom editor view"), CLASSINFO(TrenchBroom::Model::MapDocument), CLASSINFO(TrenchBroom::View::EditorView));
    wxMenuBar* menuBar = new wxMenuBar();

    wxMenu* fileMenu = CreateFileMenu();
    m_docManager->FileHistoryUseMenu(fileMenu);
    m_docManager->FileHistoryLoad(*wxConfig::Get());
    fileMenu->SetEventHandler(m_docManager);
    menuBar->Append(fileMenu, wxT("File"));

    PostInit();
#ifdef __APPLE__
    
    // file history menu
    
    // won't show up in the app's menu if we don't add them here
    
    
    
    SetExitOnFrameDelete(false);
    wxMenuBar::MacSetCommonMenuBar(menuBar);
#endif
    
    return true;
}

int AbstractApp::OnExit() {
    wxDELETE(m_docManager);
    return wxApp::OnExit();
}

void AbstractApp::OnUnhandledException() {
    try {
        throw;
    } catch (std::exception& e) {
        wxLogWarning(e.what());
    }
}

void AbstractApp::OnFileExit(wxCommandEvent& event) {
    Exit();
}
