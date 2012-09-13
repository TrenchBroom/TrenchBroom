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

#include <wx/config.h>
#include <wx/docview.h>

#include "IO/Pak.h"
#include "Model/Alias.h"
#include "Model/Bsp.h"
#include "Model/MapDocument.h"
#include "Utility/DocManager.h"
#include "View/EditorView.h"
#include "View/CommandIds.h"

BEGIN_EVENT_TABLE(AbstractApp, wxApp)
END_EVENT_TABLE()

wxMenu* AbstractApp::CreateFileMenu() {
    wxMenu* fileHistoryMenu = new wxMenu();
    fileHistoryMenu->SetEventHandler(m_docManager);
    m_docManager->FileHistoryUseMenu(fileHistoryMenu);
    m_docManager->FileHistoryAddFilesToMenu(fileHistoryMenu);
    
    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(wxID_NEW, wxT("New\tCtrl-N"));
    fileMenu->Append(wxID_OPEN, wxT("Open...\tCtrl-O"));
    fileMenu->AppendSubMenu(fileHistoryMenu, "Open Recent");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_SAVE, wxT("Save\tCtrl-S"));
    fileMenu->Append(wxID_SAVEAS, wxT("Save as...\tCtrl-Shift-S"));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_CLOSE, wxT("Close\tCtrl-W"));
    return fileMenu;
}

wxMenu* AbstractApp::CreateEditMenu() {
    using namespace TrenchBroom::View;
    
    wxMenu* editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, wxT("Undo\tCtrl-Z"));
    editMenu->Append(wxID_REDO, wxT("Redo\tCtrl-Shift-Z"));
    editMenu->AppendSeparator();
    editMenu->Append(wxID_CUT, wxT("Cut\tCtrl+X"));
    editMenu->Append(wxID_COPY, wxT("Copy\tCtrl+C"));
    editMenu->Append(wxID_PASTE, wxT("Paste\tCtrl+V"));
    editMenu->AppendSeparator();
    editMenu->Append(CommandIds::Menu::EditSelectAll, wxT("Select All\tCtrl+A"));
    editMenu->Append(CommandIds::Menu::EditSelectSiblings, wxT("Select Siblings\tCtrl+Alt+A"));
    editMenu->Append(CommandIds::Menu::EditSelectTouching, wxT("Select Touching\tCtrl+T"));
    editMenu->Append(CommandIds::Menu::EditSelectNone, wxT("Select None\tCtrl+Shift+A"));
    editMenu->AppendSeparator();
    editMenu->Append(CommandIds::Menu::EditHideSelected, wxT("Hide Selected\tCtrl+H"));
    editMenu->Append(CommandIds::Menu::EditHideUnselected, wxT("Hide Unselected\tCtrl+Alt+H"));
    editMenu->Append(CommandIds::Menu::EditUnhideAll, wxT("Unhide All\tCtrl+Shift+H"));
    editMenu->AppendSeparator();
    editMenu->Append(CommandIds::Menu::EditLockSelected, wxT("Lock Selected\tCtrl+L"));
    editMenu->Append(CommandIds::Menu::EditLockUnselected, wxT("Lock Unselected\tCtrl+Alt+L"));
    editMenu->Append(CommandIds::Menu::EditUnlockAll, wxT("Unlock All\tCtrl+Shift+L"));
    
    return editMenu;
}

wxMenu* AbstractApp::CreateViewMenu() {
    wxMenu* viewMenu = new wxMenu();
    return viewMenu;
}

wxMenu* AbstractApp::CreateHelpMenu() {
    wxMenu* helpMenu = new wxMenu();
    return helpMenu;
}

wxMenuBar* AbstractApp::CreateMenuBar(wxEvtHandler* eventHandler) {
    wxMenu* fileMenu = CreateFileMenu();
    fileMenu->SetEventHandler(m_docManager);
    
    wxMenu* editMenu = CreateEditMenu();
    editMenu->SetEventHandler(eventHandler);
    
    wxMenu* viewMenu = CreateViewMenu();
    viewMenu->SetEventHandler(eventHandler);
    
    wxMenu* helpMenu = CreateHelpMenu();
    helpMenu->SetEventHandler(eventHandler);
    
    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, wxT("File"));
    menuBar->Append(editMenu, wxT("Edit"));
    menuBar->Append(viewMenu, wxT("View"));
    menuBar->Append(helpMenu, wxT("Help"));
    
    return menuBar;
}

bool AbstractApp::OnInit() {
    // initialize globals
    TrenchBroom::IO::PakManager::sharedManager = new TrenchBroom::IO::PakManager();
    TrenchBroom::Model::AliasManager::sharedManager = new TrenchBroom::Model::AliasManager();
    TrenchBroom::Model::BspManager::sharedManager = new TrenchBroom::Model::BspManager();
    
	m_docManager = new DocManager();
    new wxDocTemplate(m_docManager, wxT("Quake map document"), wxT("*.map"), wxEmptyString, wxT("map"), wxT("Quake map document"), wxT("TrenchBroom editor view"), CLASSINFO(TrenchBroom::Model::MapDocument), CLASSINFO(TrenchBroom::View::EditorView));
    
    return true;
}

int AbstractApp::OnExit() {
    m_docManager->FileHistorySave(*wxConfig::Get());
    wxDELETE(m_docManager);
    
    delete TrenchBroom::IO::PakManager::sharedManager;
    TrenchBroom::IO::PakManager::sharedManager = NULL;
    delete TrenchBroom::Model::AliasManager::sharedManager;
    TrenchBroom::Model::AliasManager::sharedManager = NULL;
    delete TrenchBroom::Model::BspManager::sharedManager;
    TrenchBroom::Model::BspManager::sharedManager = NULL;
    
    return wxApp::OnExit();
}

void AbstractApp::OnUnhandledException() {
    try {
        throw;
    } catch (std::exception& e) {
        wxLogError(e.what());
    }
}
