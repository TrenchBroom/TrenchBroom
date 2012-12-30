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
#include "View/CommandIds.h"
#include "View/EditorView.h"
#include "View/PreferencesDialog.h"

BEGIN_EVENT_TABLE(AbstractApp, wxApp)
EVT_MENU(wxID_PREFERENCES, AbstractApp::OnOpenPreferences)
END_EVENT_TABLE()

wxMenu* AbstractApp::CreateFileMenu(wxEvtHandler* eventHandler) {
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
    fileMenu->SetEventHandler(eventHandler);
    return fileMenu;
}

wxMenu* AbstractApp::CreateEditMenu(wxEvtHandler* eventHandler, wxMenu* actionMenu) {
    using namespace TrenchBroom::View::CommandIds::Menu;
    
    wxMenu* editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, wxT("Undo\tCtrl-Z"));
    editMenu->Append(wxID_REDO, wxT("Redo\tCtrl-Shift-Z"));
    editMenu->AppendSeparator();
    editMenu->Append(wxID_CUT, wxT("Cut\tCtrl+X"));
    editMenu->Append(wxID_COPY, wxT("Copy\tCtrl+C"));
    editMenu->Append(wxID_PASTE, wxT("Paste\tCtrl+V"));
    editMenu->Append(wxID_DELETE, wxT("Delete\tBack"));
    editMenu->AppendSeparator();
    editMenu->Append(EditSelectAll, wxT("Select All\tCtrl+A"));
    editMenu->Append(EditSelectSiblings, wxT("Select Siblings\tCtrl+Alt+A"));
    editMenu->Append(EditSelectTouching, wxT("Select Touching\tCtrl+T"));
    editMenu->Append(EditSelectNone, wxT("Select None\tCtrl+Shift+A"));
    editMenu->AppendSeparator();
    editMenu->Append(EditHideSelected, wxT("Hide Selected\tCtrl+H"));
    editMenu->Append(EditHideUnselected, wxT("Hide Unselected\tCtrl+Alt+H"));
    editMenu->Append(EditUnhideAll, wxT("Unhide All\tCtrl+Shift+H"));
    editMenu->AppendSeparator();
    editMenu->Append(EditLockSelected, wxT("Lock Selected\tCtrl+L"));
    editMenu->Append(EditLockUnselected, wxT("Lock Unselected\tCtrl+Alt+L"));
    editMenu->Append(EditUnlockAll, wxT("Unlock All\tCtrl+Shift+L"));

    wxMenu* toolsMenu = new wxMenu();
    toolsMenu->AppendCheckItem(EditToggleClipTool, wxT("Clip Tool\tC"));
    toolsMenu->Append(EditToggleClipSide, wxT("Toggle Clip Side\tTAB"));
    toolsMenu->Append(EditPerformClip, wxT("Perform Clip\tENTER"));
    toolsMenu->AppendSeparator();
    toolsMenu->AppendCheckItem(EditToggleVertexTool, wxT("Vertex Tool\tV"));
    toolsMenu->SetEventHandler(eventHandler);

    editMenu->AppendSeparator();
    editMenu->AppendSubMenu(toolsMenu, wxT("Tools"));
    if (actionMenu != NULL) {
        editMenu->AppendSubMenu(actionMenu, wxT("Actions"));
        actionMenu->SetEventHandler(eventHandler);
    } else {
        editMenu->Append(EditActions, wxT("Actions"));
    }
    editMenu->AppendSeparator();
    editMenu->Append(EditCreatePointEntity, wxT("Create Point Entity"));
    editMenu->Append(EditCreateBrushEntity, wxT("Create Brush Entity"));
    editMenu->AppendSeparator();
    editMenu->AppendCheckItem(EditToggleTextureLock, wxT("Toggle Texture Lock"));
    editMenu->SetEventHandler(eventHandler);
    
    return editMenu;
}

wxMenu* AbstractApp::CreateViewMenu(wxEvtHandler* eventHandler) {
    using namespace TrenchBroom::View::CommandIds::Menu;

    wxMenu* viewMenu = new wxMenu();
    wxMenu* gridMenu = new wxMenu();
    gridMenu->AppendCheckItem(ViewToggleShowGrid, wxT("Show Grid\tCtrl+G"));
    gridMenu->AppendCheckItem(ViewToggleSnapToGrid, wxT("Snap to Grid Grid\tCtrl+Shift+G"));
    gridMenu->AppendSeparator();
    gridMenu->AppendCheckItem(ViewSetGridSize1, wxT("Set Grid Size 1\tCtrl+1"));
    gridMenu->AppendCheckItem(ViewSetGridSize2, wxT("Set Grid Size 2\tCtrl+2"));
    gridMenu->AppendCheckItem(ViewSetGridSize4, wxT("Set Grid Size 4\tCtrl+3"));
    gridMenu->AppendCheckItem(ViewSetGridSize8, wxT("Set Grid Size 8\tCtrl+4"));
    gridMenu->AppendCheckItem(ViewSetGridSize16, wxT("Set Grid Size 16\tCtrl+5"));
    gridMenu->AppendCheckItem(ViewSetGridSize32, wxT("Set Grid Size 32\tCtrl+6"));
    gridMenu->AppendCheckItem(ViewSetGridSize64, wxT("Set Grid Size 64\tCtrl+7"));
    gridMenu->AppendCheckItem(ViewSetGridSize128, wxT("Set Grid Size 128\tCtrl+8"));
    gridMenu->AppendCheckItem(ViewSetGridSize256, wxT("Set Grid Size 256\tCtrl+9"));
    gridMenu->SetEventHandler(eventHandler);
    viewMenu->AppendSubMenu(gridMenu, wxT("Grid"));
    
    wxMenu* cameraMenu = new wxMenu();
    cameraMenu->Append(ViewMoveCameraForward, wxT("Move Forward\tAlt+UP"));
    cameraMenu->Append(ViewMoveCameraBackward, wxT("Move Backward\tAlt+DOWN"));
    cameraMenu->Append(ViewMoveCameraLeft, wxT("Move Left\tAlt+LEFT"));
    cameraMenu->Append(ViewMoveCameraRight, wxT("Move Right\tAlt+RIGHT"));
    cameraMenu->Append(ViewMoveCameraUp, wxT("Move Up\tAlt+PGUP"));
    cameraMenu->Append(ViewMoveCameraDown, wxT("Move Down\tAlt+PGDN"));
    cameraMenu->SetEventHandler(eventHandler);
    viewMenu->AppendSubMenu(cameraMenu, wxT("Camera"));
    
    viewMenu->SetEventHandler(eventHandler);
    return viewMenu;
}

wxMenu* AbstractApp::CreateHelpMenu(wxEvtHandler* eventHandler) {
    wxMenu* helpMenu = new wxMenu();
    helpMenu->SetEventHandler(eventHandler);
    return helpMenu;
}

wxMenuBar* AbstractApp::CreateMenuBar(wxEvtHandler* eventHandler, wxMenu* actionMenu) {
    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(CreateFileMenu(eventHandler), wxT("File"));
    menuBar->Append(CreateEditMenu(eventHandler, actionMenu), wxT("Edit"));
    menuBar->Append(CreateViewMenu(eventHandler), wxT("View"));
    menuBar->Append(CreateHelpMenu(eventHandler), wxT("Help"));
    
    return menuBar;
}

wxMenu* AbstractApp::CreateTextureActionMenu() {
    using namespace TrenchBroom::View::CommandIds::Menu;
    
    wxMenu* textureActionMenu = new wxMenu();
    textureActionMenu->Append(EditMoveTexturesUp, wxT("Move Up\tUP"));
    textureActionMenu->Append(EditMoveTexturesRight, wxT("Move Right\tRIGHT"));
    textureActionMenu->Append(EditMoveTexturesDown, wxT("Move Down\tDOWN"));
    textureActionMenu->Append(EditMoveTexturesLeft, wxT("Move Left\tLEFT"));
    textureActionMenu->Append(EditRotateTexturesCW, wxT("Rotate CW\tPGUP"));
    textureActionMenu->Append(EditRotateTexturesCCW, wxT("Rotate CCW\tPGDN"));
    return textureActionMenu;
}

wxMenu* AbstractApp::CreateObjectActionMenu() {
    using namespace TrenchBroom::View::CommandIds::Menu;
    
    wxMenu* objectActionMenu = new wxMenu();
    objectActionMenu->Append(EditMoveObjectsForward, wxT("Move Forward\tUP"));
    objectActionMenu->Append(EditMoveObjectsRight, wxT("Move Right\tRIGHT"));
    objectActionMenu->Append(EditMoveObjectsBackward, wxT("Move Backward\tDOWN"));
    objectActionMenu->Append(EditMoveObjectsLeft, wxT("Move Left\tLEFT"));
    objectActionMenu->Append(EditMoveObjectsUp, wxT("Move Up\tPGUP"));
    objectActionMenu->Append(EditMoveObjectsDown, wxT("Move Down\tPGDN"));
    objectActionMenu->AppendSeparator();
    objectActionMenu->Append(EditRollObjectsCW, wxT("Rotate Clockwise CW\tCtrl+UP"));
    objectActionMenu->Append(EditRollObjectsCCW, wxT("Rotate Counterclockwise CCW\tCtrl+DOWN"));
    objectActionMenu->Append(EditYawObjectsCW, wxT("Rotate Left CW\tCtrl+LEFT"));
    objectActionMenu->Append(EditYawObjectsCCW, wxT("Rotate Right CCW\tCtrl+RIGHT"));
    objectActionMenu->Append(EditPitchObjectsCW, wxT("Rotate Up CW\tCtrl+PGUP"));
    objectActionMenu->Append(EditPitchObjectsCCW, wxT("Rotate Down CCW\tCtrl+PGDN"));
    objectActionMenu->AppendSeparator();
    objectActionMenu->Append(EditFlipObjectsHorizontally, wxT("Flip Horizontally\tCtrl+F"));
    objectActionMenu->Append(EditFlipObjectsVertically, wxT("Flip Vertically\tCtrl+Alt+F"));
    objectActionMenu->AppendSeparator();
    objectActionMenu->Append(EditDuplicateObjects, wxT("Duplicate\tCtrl+D"));
    return objectActionMenu;
}

wxMenu* AbstractApp::CreateVertexActionMenu() {
    using namespace TrenchBroom::View::CommandIds::Menu;
    
    wxMenu* vertexActionMenu = new wxMenu();
    vertexActionMenu->Append(EditMoveVerticesForward, wxT("Move Forward\tUP"));
    vertexActionMenu->Append(EditMoveVerticesBackward, wxT("Move Backward\tDOWN"));
    vertexActionMenu->Append(EditMoveVerticesLeft, wxT("Move Left\tLEFT"));
    vertexActionMenu->Append(EditMoveVerticesRight, wxT("Move Right\tRIGHT"));
    vertexActionMenu->Append(EditMoveVerticesUp, wxT("Move Up\tPGUP"));
    vertexActionMenu->Append(EditMoveVerticesDown, wxT("Move Down\tPGDN"));
    return vertexActionMenu;
}

void AbstractApp::UpdateAllViews(wxView* sender, wxObject* hint) {
    const wxList& documents = m_docManager->GetDocuments();
    
    wxList::const_iterator it, end;
    for (it = documents.begin(), end = documents.end(); it != end; ++it) {
        wxDocument* document = static_cast<wxDocument*>(*it);
        document->UpdateAllViews(sender, hint);
    }
}

bool AbstractApp::OnInit() {
    // initialize globals
    TrenchBroom::IO::PakManager::sharedManager = new TrenchBroom::IO::PakManager();
    TrenchBroom::Model::AliasManager::sharedManager = new TrenchBroom::Model::AliasManager();
    TrenchBroom::Model::BspManager::sharedManager = new TrenchBroom::Model::BspManager();
    
	m_docManager = new DocManager();
    m_docManager->FileHistoryLoad(*wxConfig::Get());

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

void AbstractApp::OnOpenPreferences(wxCommandEvent& event) {
    TrenchBroom::View::PreferencesDialog dialog;
    dialog.ShowModal();
}


