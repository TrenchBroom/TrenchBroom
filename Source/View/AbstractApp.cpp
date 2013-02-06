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

wxMenu* AbstractApp::CreateFileMenu(wxEvtHandler* eventHandler, bool mapViewFocused) {
    using namespace TrenchBroom::View::CommandIds::Menu;

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
    fileMenu->Append(FileLoadPointFile, wxT("Load Point File"));
    fileMenu->Append(FileUnloadPointFile, wxT("Unload Point File"));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_CLOSE, wxT("Close\tCtrl-W"));
    fileMenu->SetEventHandler(eventHandler);
    return fileMenu;
}

wxMenu* AbstractApp::CreateEditMenu(wxEvtHandler* eventHandler, wxMenu* actionMenu, bool mapViewFocused) {
    using namespace TrenchBroom::View::CommandIds::Menu;
    
    wxMenu* editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, wxT("Undo\tCtrl-Z"));
    editMenu->Append(wxID_REDO, wxT("Redo\tCtrl-Shift-Z"));
    editMenu->AppendSeparator();
    editMenu->Append(wxID_CUT, wxT("Cut\tCtrl+X"));
    editMenu->Append(wxID_COPY, wxT("Copy\tCtrl+C"));
    editMenu->Append(wxID_PASTE, wxT("Paste\tCtrl+V"));
    if (mapViewFocused) editMenu->Append(wxID_DELETE, wxT("Delete\tBack"));
    else editMenu->Append(wxID_DELETE, wxT("Delete"));
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
    if (mapViewFocused) {
        toolsMenu->AppendCheckItem(EditToggleClipTool, wxT("Clip Tool\tC"));
        toolsMenu->Append(EditToggleClipSide, wxT("Toggle Clip Side\tTAB"));
        toolsMenu->Append(EditPerformClip, wxT("Perform Clip\tENTER"));
        toolsMenu->AppendSeparator();
        toolsMenu->AppendCheckItem(EditToggleVertexTool, wxT("Vertex Tool\tV"));
        toolsMenu->AppendCheckItem(EditToggleRotateObjectsTool, wxT("Rotate Objects Tool\tR"));
    } else {
        toolsMenu->AppendCheckItem(EditToggleClipTool, wxT("Clip Tool"));
        toolsMenu->Append(EditToggleClipSide, wxT("Toggle Clip Side"));
        toolsMenu->Append(EditPerformClip, wxT("Perform Clip"));
        toolsMenu->AppendSeparator();
        toolsMenu->AppendCheckItem(EditToggleVertexTool, wxT("Vertex Tool"));
        toolsMenu->AppendCheckItem(EditToggleRotateObjectsTool, wxT("Rotate Objects Tool"));
    }
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
    editMenu->AppendCheckItem(EditToggleTextureLock, wxT("Toggle Texture Lock"));
    editMenu->Append(EditShowMapProperties, wxT("Edit Map Properties..."));
    editMenu->SetEventHandler(eventHandler);
    
    return editMenu;
}

wxMenu* AbstractApp::CreateViewMenu(wxEvtHandler* eventHandler, bool mapViewFocused) {
    using namespace TrenchBroom::View::CommandIds::Menu;

    wxMenu* viewMenu = new wxMenu();
    wxMenu* gridMenu = new wxMenu();
    gridMenu->AppendCheckItem(ViewToggleShowGrid, wxT("Show Grid\tCtrl+G"));
    gridMenu->AppendCheckItem(ViewToggleSnapToGrid, wxT("Snap to Grid Grid\tCtrl+Shift+G"));
    gridMenu->AppendSeparator();
    gridMenu->Append(ViewIncGridSize, wxT("Increase Grid Size\tCtrl++"));
    gridMenu->Append(ViewDecGridSize, wxT("Decrease Grid Size\tCtrl+-"));
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
    if (mapViewFocused) {
        cameraMenu->Append(ViewMoveCameraForward, wxT("Move Forward\tAlt+UP"));
        cameraMenu->Append(ViewMoveCameraBackward, wxT("Move Backward\tAlt+DOWN"));
        cameraMenu->Append(ViewMoveCameraLeft, wxT("Move Left\tAlt+LEFT"));
        cameraMenu->Append(ViewMoveCameraRight, wxT("Move Right\tAlt+RIGHT"));
        cameraMenu->Append(ViewMoveCameraUp, wxT("Move Up\tAlt+PGUP"));
        cameraMenu->Append(ViewMoveCameraDown, wxT("Move Down\tAlt+PGDN"));
        cameraMenu->AppendSeparator();
        cameraMenu->Append(ViewMoveCameraToNextPoint, wxT("Move Camera to Next Point\tAlt++"));
        cameraMenu->Append(ViewMoveCameraToPreviousPoint, wxT("Move Camera to Previous Point\tAlt+-"));
    } else {
        cameraMenu->Append(ViewMoveCameraForward, wxT("Move Forward"));
        cameraMenu->Append(ViewMoveCameraBackward, wxT("Move Backward"));
        cameraMenu->Append(ViewMoveCameraLeft, wxT("Move Left"));
        cameraMenu->Append(ViewMoveCameraRight, wxT("Move Right"));
        cameraMenu->Append(ViewMoveCameraUp, wxT("Move Up"));
        cameraMenu->Append(ViewMoveCameraDown, wxT("Move Down"));
        cameraMenu->AppendSeparator();
        cameraMenu->Append(ViewMoveCameraToNextPoint, wxT("Move Camera to Next Point"));
        cameraMenu->Append(ViewMoveCameraToPreviousPoint, wxT("Move Camera to Previous Point"));
    }
    cameraMenu->Append(ViewCenterCameraOnSelection, wxT("Center On Selection\tALT+C"));
    cameraMenu->SetEventHandler(eventHandler);
    viewMenu->AppendSubMenu(cameraMenu, wxT("Camera"));
    
    viewMenu->SetEventHandler(eventHandler);
    return viewMenu;
}

wxMenu* AbstractApp::CreateHelpMenu(wxEvtHandler* eventHandler, bool mapViewFocused) {
    wxMenu* helpMenu = new wxMenu();
    helpMenu->SetEventHandler(eventHandler);
    return helpMenu;
}

wxMenuBar* AbstractApp::CreateMenuBar(wxEvtHandler* eventHandler, wxMenu* actionMenu, bool mapViewFocused) {
    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(CreateFileMenu(eventHandler, mapViewFocused), wxT("File"));
    menuBar->Append(CreateEditMenu(eventHandler, actionMenu, mapViewFocused), wxT("Edit"));
    menuBar->Append(CreateViewMenu(eventHandler, mapViewFocused), wxT("View"));
    menuBar->Append(CreateHelpMenu(eventHandler, mapViewFocused), wxT("Help"));
    
    return menuBar;
}

void AbstractApp::DetachFileHistoryMenu(wxMenuBar* menuBar) {
    if (menuBar != NULL) {
        int fileMenuIndex = menuBar->FindMenu(wxT("File"));
        assert(fileMenuIndex != wxNOT_FOUND);
        
        wxMenu* fileMenu = menuBar->GetMenu(static_cast<size_t>(fileMenuIndex));
        int fileHistoryMenuIndex = fileMenu->FindItem(wxT("Open Recent"));
        assert(fileHistoryMenuIndex != wxNOT_FOUND);
        
        wxMenuItem* fileHistoryMenuItem = fileMenu->FindItem(fileHistoryMenuIndex);
        assert(fileHistoryMenuItem != NULL);
        
        wxMenu* fileHistoryMenu = fileHistoryMenuItem->GetSubMenu();
        assert(fileHistoryMenu != NULL);
        
        m_docManager->FileHistoryRemoveMenu(fileHistoryMenu);
    }
}

wxMenu* AbstractApp::CreateTextureActionMenu(bool mapViewFocused) {
    using namespace TrenchBroom::View::CommandIds::Menu;
    
    wxMenu* textureActionMenu = new wxMenu();
    if (mapViewFocused) {
        
        textureActionMenu->Append(EditMoveTexturesUp, wxT("Move Up\tUP"));
        textureActionMenu->Append(EditMoveTexturesDown, wxT("Move Down\tDOWN"));
        textureActionMenu->Append(EditMoveTexturesLeft, wxT("Move Left\tLEFT"));
        textureActionMenu->Append(EditMoveTexturesRight, wxT("Move Right\tRIGHT"));
        textureActionMenu->Append(EditRotateTexturesCW, wxT("Rotate 15 CW\tPGUP"));
        textureActionMenu->Append(EditRotateTexturesCCW, wxT("Rotate 15 CCW\tPGDN"));
        textureActionMenu->AppendSeparator();
        textureActionMenu->Append(EditMoveTexturesUpFine, wxT("Move Up by 1\tCtrl+UP"));
        textureActionMenu->Append(EditMoveTexturesDownFine, wxT("Move Down by 1\tCtrl+DOWN"));
        textureActionMenu->Append(EditMoveTexturesLeftFine, wxT("Move Left by 1\tCtrl+LEFT"));
        textureActionMenu->Append(EditMoveTexturesRightFine, wxT("Move Right by 1\tCtrl+RIGHT"));
        textureActionMenu->Append(EditRotateTexturesCWFine, wxT("Rotate 1 CW\tCtrl+PGUP"));
        textureActionMenu->Append(EditRotateTexturesCCWFine, wxT("Rotate 1 CCW\tCtrl+PGDN"));
    } else {
        textureActionMenu->Append(EditMoveTexturesUp, wxT("Move Up"));
        textureActionMenu->Append(EditMoveTexturesDown, wxT("Move Down"));
        textureActionMenu->Append(EditMoveTexturesLeft, wxT("Move Left"));
        textureActionMenu->Append(EditMoveTexturesRight, wxT("Move Right"));
        textureActionMenu->Append(EditRotateTexturesCW, wxT("Rotate 15 CW"));
        textureActionMenu->Append(EditRotateTexturesCCW, wxT("Rotate 15 CCW"));
        textureActionMenu->AppendSeparator();
        textureActionMenu->Append(EditMoveTexturesUpFine, wxT("Move Up by 1"));
        textureActionMenu->Append(EditMoveTexturesDownFine, wxT("Move Down by 1"));
        textureActionMenu->Append(EditMoveTexturesLeftFine, wxT("Move Left by 1"));
        textureActionMenu->Append(EditMoveTexturesRightFine, wxT("Move Right by 1"));
        textureActionMenu->Append(EditRotateTexturesCWFine, wxT("Rotate 1 CW"));
        textureActionMenu->Append(EditRotateTexturesCCWFine, wxT("Rotate 1 CCW"));
    }
    return textureActionMenu;
}

wxMenu* AbstractApp::CreateObjectActionMenu(bool mapViewFocused) {
    using namespace TrenchBroom::View::CommandIds::Menu;
    
    wxMenu* objectActionMenu = new wxMenu();
    if (mapViewFocused) {
        objectActionMenu->Append(EditMoveObjectsForward, wxT("Move Forward\tUP"));
        objectActionMenu->Append(EditMoveObjectsBackward, wxT("Move Backward\tDOWN"));
        objectActionMenu->Append(EditMoveObjectsLeft, wxT("Move Left\tLEFT"));
        objectActionMenu->Append(EditMoveObjectsRight, wxT("Move Right\tRIGHT"));
        objectActionMenu->Append(EditMoveObjectsUp, wxT("Move Up\tPGUP"));
        objectActionMenu->Append(EditMoveObjectsDown, wxT("Move Down\tPGDN"));
        objectActionMenu->AppendSeparator();
        objectActionMenu->Append(EditRollObjectsCW, wxT("Rotate 90 Clockwise\tCtrl+UP"));
        objectActionMenu->Append(EditRollObjectsCCW, wxT("Rotate 90 Counterclockwise\tCtrl+DOWN"));
        objectActionMenu->Append(EditYawObjectsCW, wxT("Rotate 90 Left\tCtrl+LEFT"));
        objectActionMenu->Append(EditYawObjectsCCW, wxT("Rotate 90 Right\tCtrl+RIGHT"));
        objectActionMenu->Append(EditPitchObjectsCW, wxT("Rotate 90 Up\tCtrl+PGUP"));
        objectActionMenu->Append(EditPitchObjectsCCW, wxT("Rotate 90 Down\tCtrl+PGDN"));
    } else {
        objectActionMenu->Append(EditMoveObjectsForward, wxT("Move Forward"));
        objectActionMenu->Append(EditMoveObjectsBackward, wxT("Move Backward"));
        objectActionMenu->Append(EditMoveObjectsLeft, wxT("Move Left"));
        objectActionMenu->Append(EditMoveObjectsRight, wxT("Move Right"));
        objectActionMenu->Append(EditMoveObjectsUp, wxT("Move Up"));
        objectActionMenu->Append(EditMoveObjectsDown, wxT("Move Down"));
        objectActionMenu->AppendSeparator();
        objectActionMenu->Append(EditRollObjectsCW, wxT("Rotate 90 Clockwise"));
        objectActionMenu->Append(EditRollObjectsCCW, wxT("Rotate 90 Counterclockwise"));
        objectActionMenu->Append(EditYawObjectsCW, wxT("Rotate 90 Left"));
        objectActionMenu->Append(EditYawObjectsCCW, wxT("Rotate 90 Right"));
        objectActionMenu->Append(EditPitchObjectsCW, wxT("Rotate 90 Up"));
        objectActionMenu->Append(EditPitchObjectsCCW, wxT("Rotate 90 Down"));
    }
    objectActionMenu->AppendSeparator();
    objectActionMenu->Append(EditFlipObjectsHorizontally, wxT("Flip Horizontally\tCtrl+F"));
    objectActionMenu->Append(EditFlipObjectsVertically, wxT("Flip Vertically\tCtrl+Alt+F"));
    objectActionMenu->AppendSeparator();
    objectActionMenu->Append(EditDuplicateObjects, wxT("Duplicate\tCtrl+D"));
    return objectActionMenu;
}

wxMenu* AbstractApp::CreateVertexActionMenu(bool mapViewFocused) {
    using namespace TrenchBroom::View::CommandIds::Menu;
    
    wxMenu* vertexActionMenu = new wxMenu();
    if (mapViewFocused) {
        vertexActionMenu->Append(EditMoveVerticesForward, wxT("Move Forward\tUP"));
        vertexActionMenu->Append(EditMoveVerticesBackward, wxT("Move Backward\tDOWN"));
        vertexActionMenu->Append(EditMoveVerticesLeft, wxT("Move Left\tLEFT"));
        vertexActionMenu->Append(EditMoveVerticesRight, wxT("Move Right\tRIGHT"));
        vertexActionMenu->Append(EditMoveVerticesUp, wxT("Move Up\tPGUP"));
        vertexActionMenu->Append(EditMoveVerticesDown, wxT("Move Down\tPGDN"));
    } else {
        vertexActionMenu->Append(EditMoveVerticesForward, wxT("Move Forward"));
        vertexActionMenu->Append(EditMoveVerticesBackward, wxT("Move Backward"));
        vertexActionMenu->Append(EditMoveVerticesLeft, wxT("Move Left"));
        vertexActionMenu->Append(EditMoveVerticesRight, wxT("Move Right"));
        vertexActionMenu->Append(EditMoveVerticesUp, wxT("Move Up"));
        vertexActionMenu->Append(EditMoveVerticesDown, wxT("Move Down"));
    }
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

    new wxDocTemplate(m_docManager,
                      wxT("Quake map document"),
                      wxT("*.map"),
                      wxEmptyString,
                      wxT("map"),
                      wxT("Quake map document"),
                      wxT("TrenchBroom editor view"),
                      CLASSINFO(TrenchBroom::Model::MapDocument),
                      CLASSINFO(TrenchBroom::View::EditorView)
                      );
    
    // load image handles
    wxImage::AddHandler(new wxPNGHandler());
    
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
    
    int width = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
    int height = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
    wxPoint pos((width - dialog.GetSize().x) / 2, (height - dialog.GetSize().y) / 2);
    dialog.SetPosition(pos);
    
    dialog.ShowModal();
}


