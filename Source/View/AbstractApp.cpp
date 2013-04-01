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

#include <wx/aboutdlg.h>
#include <wx/config.h>
#include <wx/docview.h>
#include <wx/generic/helpext.h>
#include <wx/fs_mem.h>

#include "IO/FileManager.h"
#include "IO/Pak.h"
#include "Model/Alias.h"
#include "Model/Bsp.h"
#include "Model/MapDocument.h"
#include "Utility/DocManager.h"
#include "View/AboutDialog.h"
#include "View/CommandIds.h"
#include "View/EditorFrame.h"
#include "View/EditorView.h"
#include "View/KeyboardShortcut.h"
#include "View/PreferencesFrame.h"

BEGIN_EVENT_TABLE(AbstractApp, wxApp)
EVT_MENU(wxID_NEW, AbstractApp::OnFileNew)
EVT_MENU(wxID_OPEN, AbstractApp::OnFileOpen)
EVT_MENU(wxID_SAVE, AbstractApp::OnFileSave)
EVT_MENU(wxID_SAVEAS, AbstractApp::OnFileSaveAs)
EVT_MENU(wxID_CLOSE, AbstractApp::OnFileClose)
EVT_MENU(wxID_PREFERENCES, AbstractApp::OnOpenPreferences)
EVT_MENU(wxID_ABOUT, AbstractApp::OnOpenAbout)
EVT_MENU(TrenchBroom::View::CommandIds::Menu::HelpShowHelp, AbstractApp::OnHelpShowHelp)

EVT_UPDATE_UI(wxID_NEW, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI(wxID_OPEN, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI(wxID_SAVE, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI(wxID_SAVEAS, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI(wxID_CLOSE, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI(wxID_UNDO, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI(wxID_REDO, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI(wxID_CUT, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI(wxID_COPY, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI(wxID_PASTE, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI(wxID_DELETE, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI_RANGE(TrenchBroom::View::CommandIds::Menu::Lowest, TrenchBroom::View::CommandIds::Menu::Highest, AbstractApp::OnUpdateMenuItem)

EVT_ANIMATION(AbstractApp::OnAnimation)
END_EVENT_TABLE()

void AbstractApp::appendItem(wxMenu* menu, const TrenchBroom::Preferences::Preference<TrenchBroom::View::KeyboardShortcut>& pref, bool mapViewFocused) {
    using TrenchBroom::View::KeyboardShortcut;
    using namespace TrenchBroom::Preferences;
    
    PreferenceManager& prefs = PreferenceManager::preferences();
    const KeyboardShortcut& shortcut = prefs.getKeyboardShortcut(pref);
    if (mapViewFocused || shortcut.alwaysShowModifier())
        menu->Append(shortcut.commandId(), shortcut.menuText());
    else
        menu->Append(shortcut.commandId(), shortcut.text());
}

void AbstractApp::appendCheckItem(wxMenu* menu, const TrenchBroom::Preferences::Preference<TrenchBroom::View::KeyboardShortcut>& pref, bool mapViewFocused) {
    using TrenchBroom::View::KeyboardShortcut;
    using namespace TrenchBroom::Preferences;
    
    PreferenceManager& prefs = PreferenceManager::preferences();
    const KeyboardShortcut& shortcut = prefs.getKeyboardShortcut(pref);
    if (mapViewFocused || shortcut.alwaysShowModifier())
        menu->AppendCheckItem(shortcut.commandId(), shortcut.menuText());
    else
        menu->AppendCheckItem(shortcut.commandId(), shortcut.text());
}

wxMenu* AbstractApp::CreateFileMenu(wxEvtHandler* eventHandler, bool mapViewFocused) {
    using namespace TrenchBroom::Preferences;
    
    wxMenu* fileHistoryMenu = new wxMenu();
    fileHistoryMenu->SetEventHandler(m_docManager);
    m_docManager->FileHistoryUseMenu(fileHistoryMenu);
    m_docManager->FileHistoryAddFilesToMenu(fileHistoryMenu);
    
    wxMenu* fileMenu = new wxMenu();
    appendItem(fileMenu, FileNew, mapViewFocused);
    appendItem(fileMenu, FileOpen, mapViewFocused);
    fileMenu->AppendSubMenu(fileHistoryMenu, "Open Recent");
    fileMenu->AppendSeparator();
    appendItem(fileMenu, FileSave, mapViewFocused);
    appendItem(fileMenu, FileSaveAs, mapViewFocused);
    fileMenu->AppendSeparator();
    appendItem(fileMenu, FileLoadPointFile, mapViewFocused);
    appendItem(fileMenu, FileUnloadPointFile, mapViewFocused);
    fileMenu->AppendSeparator();
    appendItem(fileMenu, FileClose, mapViewFocused);
    fileMenu->SetEventHandler(eventHandler);
    return fileMenu;
}

wxMenu* AbstractApp::CreateEditMenu(wxEvtHandler* eventHandler, wxMenu* actionMenu, bool mapViewFocused) {
    using namespace TrenchBroom::Preferences;
    
    wxMenu* editMenu = new wxMenu();
    wxMenu* toolsMenu = new wxMenu();
    
    appendItem(editMenu, EditUndo, mapViewFocused);
    appendItem(editMenu, EditRedo, mapViewFocused);
    editMenu->AppendSeparator();
    appendItem(editMenu, EditCut, mapViewFocused);
    appendItem(editMenu, EditCopy, mapViewFocused);
    appendItem(editMenu, EditPaste, mapViewFocused);
    appendItem(editMenu, EditPasteAtOriginalPosition, mapViewFocused);
    appendItem(editMenu, EditDelete, mapViewFocused);
    editMenu->AppendSeparator();
    appendItem(editMenu, EditSelectAll, mapViewFocused);
    appendItem(editMenu, EditSelectSiblings, mapViewFocused);
    appendItem(editMenu, EditSelectTouching, mapViewFocused);
    appendItem(editMenu, EditSelectByFilePosition, mapViewFocused);
    appendItem(editMenu, EditSelectNone, mapViewFocused);
    editMenu->AppendSeparator();
    appendItem(editMenu, EditHideSelected, mapViewFocused);
    appendItem(editMenu, EditHideUnselected, mapViewFocused);
    appendItem(editMenu, EditUnhideAll, mapViewFocused);
    editMenu->AppendSeparator();
    appendItem(editMenu, EditLockSelected, mapViewFocused);
    appendItem(editMenu, EditLockUnselected, mapViewFocused);
    appendItem(editMenu, EditUnlockAll, mapViewFocused);
    
    appendCheckItem(toolsMenu, EditToolsToggleClipTool, mapViewFocused);
    appendItem(toolsMenu, EditToolsToggleClipSide, mapViewFocused);
    appendItem(toolsMenu, EditToolsPerformClip, mapViewFocused);
    toolsMenu->AppendSeparator();
    appendCheckItem(toolsMenu, EditToolsToggleVertexTool, mapViewFocused);
    appendCheckItem(toolsMenu, EditToolsToggleRotateTool, mapViewFocused);
    
    editMenu->AppendSeparator();
    editMenu->AppendSubMenu(toolsMenu, wxT("Tools"));
    
    if (actionMenu != NULL) {
        editMenu->AppendSubMenu(actionMenu, wxT("Actions"));
        actionMenu->SetEventHandler(eventHandler);
    } else {
        editMenu->Append(wxID_ANY, wxT("Actions"));
    }
    
    editMenu->AppendSeparator();
    appendCheckItem(editMenu, EditToggleTextureLock, mapViewFocused);
    appendItem(editMenu, EditShowMapProperties, mapViewFocused);
    
    toolsMenu->SetEventHandler(eventHandler);
    editMenu->SetEventHandler(eventHandler);
    
    return editMenu;
}

wxMenu* AbstractApp::CreateViewMenu(wxEvtHandler* eventHandler, bool mapViewFocused) {
    using namespace TrenchBroom::Preferences;
    
    wxMenu* viewMenu = new wxMenu();
    wxMenu* gridMenu = new wxMenu();
    wxMenu* cameraMenu = new wxMenu();
    
    appendCheckItem(gridMenu, ViewGridToggleShowGrid, mapViewFocused);
    appendCheckItem(gridMenu, ViewGridToggleSnapToGrid, mapViewFocused);
    gridMenu->AppendSeparator();
    appendItem(gridMenu, ViewGridIncGridSize, mapViewFocused);
    appendItem(gridMenu, ViewGridDecGridSize, mapViewFocused);
    gridMenu->AppendSeparator();
    appendCheckItem(gridMenu, ViewGridSetSize1, mapViewFocused);
    appendCheckItem(gridMenu, ViewGridSetSize2, mapViewFocused);
    appendCheckItem(gridMenu, ViewGridSetSize4, mapViewFocused);
    appendCheckItem(gridMenu, ViewGridSetSize8, mapViewFocused);
    appendCheckItem(gridMenu, ViewGridSetSize16, mapViewFocused);
    appendCheckItem(gridMenu, ViewGridSetSize32, mapViewFocused);
    appendCheckItem(gridMenu, ViewGridSetSize64, mapViewFocused);
    appendCheckItem(gridMenu, ViewGridSetSize128, mapViewFocused);
    appendCheckItem(gridMenu, ViewGridSetSize256, mapViewFocused);
    
    gridMenu->SetEventHandler(eventHandler);
    viewMenu->AppendSubMenu(gridMenu, wxT("Grid"));
    
    appendItem(cameraMenu, ViewCameraMoveForward, mapViewFocused);
    appendItem(cameraMenu, ViewCameraMoveBackward, mapViewFocused);
    appendItem(cameraMenu, ViewCameraMoveLeft, mapViewFocused);
    appendItem(cameraMenu, ViewCameraMoveRight, mapViewFocused);
    appendItem(cameraMenu, ViewCameraMoveUp, mapViewFocused);
    appendItem(cameraMenu, ViewCameraMoveDown, mapViewFocused);
    cameraMenu->AppendSeparator();
    appendItem(cameraMenu, ViewCameraMoveToNextPoint, mapViewFocused);
    appendItem(cameraMenu, ViewCameraMoveToPreviousPoint, mapViewFocused);
    appendItem(cameraMenu, ViewCameraCenterCameraOnSelection, mapViewFocused);
    
    cameraMenu->SetEventHandler(eventHandler);
    viewMenu->AppendSubMenu(cameraMenu, wxT("Camera"));
    
    viewMenu->AppendSeparator();
    appendItem(viewMenu, ViewSwitchToEntityTab, mapViewFocused);
    appendItem(viewMenu, ViewSwitchToFaceTab, mapViewFocused);
    appendItem(viewMenu, ViewSwitchToViewTab, mapViewFocused);
    
    viewMenu->SetEventHandler(eventHandler);
    return viewMenu;
}

wxMenu* AbstractApp::CreateHelpMenu(wxEvtHandler* eventHandler, bool mapViewFocused) {
    using namespace TrenchBroom::View::CommandIds::Menu;
    
    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(HelpShowHelp, wxT("TrenchBroom Help"));
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
    using namespace TrenchBroom::Preferences;
    
    wxMenu* textureActionMenu = new wxMenu();
    
    appendItem(textureActionMenu, EditActionsMoveTexturesUp, mapViewFocused);
    appendItem(textureActionMenu, EditActionsMoveTexturesDown, mapViewFocused);
    appendItem(textureActionMenu, EditActionsMoveTexturesLeft, mapViewFocused);
    appendItem(textureActionMenu, EditActionsMoveTexturesRight, mapViewFocused);
    appendItem(textureActionMenu, EditActionsRotateTexturesCW, mapViewFocused);
    appendItem(textureActionMenu, EditActionsRotateTexturesCCW, mapViewFocused);
    textureActionMenu->AppendSeparator();
    appendItem(textureActionMenu, EditActionsMoveTexturesUpFine, mapViewFocused);
    appendItem(textureActionMenu, EditActionsMoveTexturesDownFine, mapViewFocused);
    appendItem(textureActionMenu, EditActionsMoveTexturesLeftFine, mapViewFocused);
    appendItem(textureActionMenu, EditActionsMoveTexturesRightFine, mapViewFocused);
    appendItem(textureActionMenu, EditActionsRotateTexturesCWFine, mapViewFocused);
    appendItem(textureActionMenu, EditActionsRotateTexturesCCWFine, mapViewFocused);
    
    return textureActionMenu;
}

wxMenu* AbstractApp::CreateObjectActionMenu(bool mapViewFocused) {
    using namespace TrenchBroom::Preferences;
    
    wxMenu* objectActionMenu = new wxMenu();
    
    appendItem(objectActionMenu, EditActionsMoveObjectsForward, mapViewFocused);
    appendItem(objectActionMenu, EditActionsMoveObjectsBackward, mapViewFocused);
    appendItem(objectActionMenu, EditActionsMoveObjectsLeft, mapViewFocused);
    appendItem(objectActionMenu, EditActionsMoveObjectsRight, mapViewFocused);
    appendItem(objectActionMenu, EditActionsMoveObjectsUp, mapViewFocused);
    appendItem(objectActionMenu, EditActionsMoveObjectsDown, mapViewFocused);
    objectActionMenu->AppendSeparator();
    appendItem(objectActionMenu, EditActionsDuplicateObjectsForward, mapViewFocused);
    appendItem(objectActionMenu, EditActionsDuplicateObjectsBackward, mapViewFocused);
    appendItem(objectActionMenu, EditActionsDuplicateObjectsLeft, mapViewFocused);
    appendItem(objectActionMenu, EditActionsDuplicateObjectsRight, mapViewFocused);
    appendItem(objectActionMenu, EditActionsDuplicateObjectsUp, mapViewFocused);
    appendItem(objectActionMenu, EditActionsDuplicateObjectsDown, mapViewFocused);
    objectActionMenu->AppendSeparator();
    appendItem(objectActionMenu, EditActionsRollObjectsCW, mapViewFocused);
    appendItem(objectActionMenu, EditActionsRollObjectsCCW, mapViewFocused);
    appendItem(objectActionMenu, EditActionsYawObjectsCW, mapViewFocused);
    appendItem(objectActionMenu, EditActionsYawObjectsCCW, mapViewFocused);
    appendItem(objectActionMenu, EditActionsPitchObjectsCW, mapViewFocused);
    appendItem(objectActionMenu, EditActionsPitchObjectsCCW, mapViewFocused);
    objectActionMenu->AppendSeparator();
    appendItem(objectActionMenu, EditActionsFlipObjectsHorizontally, mapViewFocused);
    appendItem(objectActionMenu, EditActionsFlipObjectsVertically, mapViewFocused);
    objectActionMenu->AppendSeparator();
    appendItem(objectActionMenu, EditActionsDuplicateObjects, mapViewFocused);
    objectActionMenu->AppendSeparator();
    appendItem(objectActionMenu, EditActionsCorrectVertices, mapViewFocused);
    appendItem(objectActionMenu, EditActionsSnapVertices, mapViewFocused);
    
    return objectActionMenu;
}

wxMenu* AbstractApp::CreateVertexActionMenu(bool mapViewFocused) {
    using namespace TrenchBroom::Preferences;
    
    wxMenu* vertexActionMenu = new wxMenu();
    
    appendItem(vertexActionMenu, EditActionsMoveVerticesForward, mapViewFocused);
    appendItem(vertexActionMenu, EditActionsMoveVerticesBackward, mapViewFocused);
    appendItem(vertexActionMenu, EditActionsMoveVerticesLeft, mapViewFocused);
    appendItem(vertexActionMenu, EditActionsMoveVerticesRight, mapViewFocused);
    appendItem(vertexActionMenu, EditActionsMoveVerticesUp, mapViewFocused);
    appendItem(vertexActionMenu, EditActionsMoveVerticesDown, mapViewFocused);
    vertexActionMenu->AppendSeparator();
    appendItem(vertexActionMenu, EditActionsCorrectVertices, mapViewFocused);
    appendItem(vertexActionMenu, EditActionsSnapVertices, mapViewFocused);
    
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
    m_preferencesFrame = NULL;
    
    // initialize globals
    TrenchBroom::IO::PakManager::sharedManager = new TrenchBroom::IO::PakManager();
    TrenchBroom::Model::AliasManager::sharedManager = new TrenchBroom::Model::AliasManager();
    TrenchBroom::Model::BspManager::sharedManager = new TrenchBroom::Model::BspManager();
    
	m_docManager = new DocManager();
    m_docManager->FileHistoryLoad(*wxConfig::Get());
    
    new wxDocTemplate(m_docManager,
                      wxT("Quake map document"),
#if defined __linux__ // appears to be a bug in wxWidgets' file dialog, on Linux it will only allow lowercase extensions
                      wxT("*.*"),
#else
                      wxT("*.map"),
#endif
                      wxEmptyString,
                      wxT("map"),
                      wxT("Quake map document"),
                      wxT("TrenchBroom editor view"),
                      CLASSINFO(TrenchBroom::Model::MapDocument),
                      CLASSINFO(TrenchBroom::View::EditorView)
                      );
    
    // load file system handlers
    wxFileSystem::AddHandler(new wxMemoryFSHandler());
    
    // load image handles
    wxImage::AddHandler(new wxGIFHandler());
    wxImage::AddHandler(new wxPNGHandler());
    
    TrenchBroom::IO::FileManager fileManager;
    String helpPath = fileManager.appendPath(fileManager.resourceDirectory(), "Documentation");
    
    m_helpController = new wxExtHelpController();
    m_helpController->Initialize(helpPath);
    
    return true;
}

int AbstractApp::OnExit() {
    m_docManager->FileHistorySave(*wxConfig::Get());
    wxDELETE(m_docManager);
    wxDELETE(m_helpController);
    
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

void AbstractApp::OnOpenAbout(wxCommandEvent& event) {
    TrenchBroom::View::AboutDialog aboutDialog(NULL);
    aboutDialog.ShowModal();
}

void AbstractApp::OnOpenPreferences(wxCommandEvent& event) {
    if (m_preferencesFrame != NULL)
        return;
    TrenchBroom::View::PreferencesFrame* frame = new TrenchBroom::View::PreferencesFrame();
    frame->CenterOnScreen();
    frame->Show();
}

void AbstractApp::OnFileNew(wxCommandEvent& event) {
    if (m_docManager != NULL)
        m_docManager->OnFileNew(event);
}

void AbstractApp::OnFileOpen(wxCommandEvent& event) {
    if (m_docManager != NULL)
        m_docManager->OnFileOpen(event);
}

void AbstractApp::OnFileSave(wxCommandEvent& event) {
    if (m_docManager != NULL)
        m_docManager->OnFileSave(event);
}

void AbstractApp::OnFileSaveAs(wxCommandEvent& event) {
    if (m_docManager != NULL)
        m_docManager->OnFileSaveAs(event);
}

void AbstractApp::OnFileClose(wxCommandEvent& event) {
    if (m_docManager != NULL)
        m_docManager->OnFileClose(event);
}

void AbstractApp::OnHelpShowHelp(wxCommandEvent& event) {
    assert(m_helpController != NULL);
    m_helpController->DisplaySection(01);
}

void AbstractApp::OnUpdateMenuItem(wxUpdateUIEvent& event) {
    if (event.GetId() == wxID_ABOUT ||
        event.GetId() == TrenchBroom::View::CommandIds::Menu::HelpShowHelp)
        event.Enable(true);
    else if (event.GetId() == wxID_PREFERENCES ||
             event.GetId() == wxID_NEW ||
             event.GetId() == wxID_OPEN)
        event.Enable(m_preferencesFrame == NULL);
    else
        event.Enable(false);
    
    if (GetTopWindow() != NULL && m_preferencesFrame == NULL)
        event.Skip();
}

void AbstractApp::OnAnimation(TrenchBroom::View::AnimationEvent& event) {
    event.execute();
}

int AbstractApp::FilterEvent(wxEvent& event) {
    if (event.GetEventObject() != NULL) {
        if (event.GetEventType() == wxEVT_SET_FOCUS) {
            wxObject* object = event.GetEventObject();
            wxWindow* window = wxDynamicCast(object, wxWindow);
            if (window != NULL) {
                wxFrame* frame = wxDynamicCast(window, wxFrame);
                wxWindow* parent = window->GetParent();
                while (frame == NULL && parent != NULL) {
                    frame = wxDynamicCast(parent, wxFrame);
                    parent = parent->GetParent();
                }
                
                // If we found a frame, and window is not a menu, then send a command event to the frame
                // that will cause it to rebuild its menu. The frame must keep track of whether the menu actually needs
                // to be rebuilt (only if MapGLCanvas previously had focus and just lost it or vice versa).
                // make sure the command is sent via QueueEvent to give wxWidgets a chance to update the focus states!
                if (frame != NULL) {
                    //bool isMenu = wxDynamicCast(window, wxMenu) || wxDynamicCast(window, wxMenuItem);
                    //if (!isMenu) {
                    wxCommandEvent focusEvent(TrenchBroom::View::EditorFrame::EVT_SET_FOCUS);
                    focusEvent.SetClientData(event.GetEventObject());
                    focusEvent.SetEventObject(frame);
                    focusEvent.SetId(event.GetId());
                    AddPendingEvent(focusEvent);
                    //}
                }
            }
        } else if (event.GetEventType() == TrenchBroom::View::EditorFrame::EVT_SET_FOCUS) {
            wxFrame* frame = wxStaticCast(event.GetEventObject(), wxFrame);
            frame->ProcessWindowEventLocally(event);
            return 1;
        }
    }
    
    return wxApp::FilterEvent(event);
}
