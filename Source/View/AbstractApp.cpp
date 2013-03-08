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
#include "View/PreferencesDialog.h"

BEGIN_EVENT_TABLE(AbstractApp, wxApp)
EVT_MENU(wxID_PREFERENCES, AbstractApp::OnOpenPreferences)
EVT_MENU(wxID_ABOUT, AbstractApp::OnOpenAbout)
EVT_MENU(TrenchBroom::View::CommandIds::Menu::HelpShowHelp, AbstractApp::OnHelpShowHelp)

EVT_UPDATE_UI(wxID_UNDO, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI(wxID_REDO, AbstractApp::OnUpdateMenuItem)
EVT_UPDATE_UI_RANGE(TrenchBroom::View::CommandIds::Menu::Lowest, TrenchBroom::View::CommandIds::Menu::Highest, AbstractApp::OnUpdateMenuItem)

EVT_ANIMATION(AbstractApp::OnAnimation)
END_EVENT_TABLE()

void AbstractApp::appendItem(wxMenu* menu, const TrenchBroom::Preferences::Preference<TrenchBroom::View::KeyboardShortcut>& pref, bool withAccelerator) {
    using TrenchBroom::View::KeyboardShortcut;
    using namespace TrenchBroom::Preferences;
    
    PreferenceManager& prefs = PreferenceManager::preferences();
    const KeyboardShortcut& shortcut = prefs.getKeyboardShortcut(pref);
    if (withAccelerator)
        menu->Append(shortcut.commandId(), shortcut.menuText());
    else
        menu->Append(shortcut.commandId(), shortcut.text());
}

void AbstractApp::appendCheckItem(wxMenu* menu, const TrenchBroom::Preferences::Preference<TrenchBroom::View::KeyboardShortcut>& pref, bool withAccelerator) {
    using TrenchBroom::View::KeyboardShortcut;
    using namespace TrenchBroom::Preferences;
    
    PreferenceManager& prefs = PreferenceManager::preferences();
    const KeyboardShortcut& shortcut = prefs.getKeyboardShortcut(pref);
    if (withAccelerator)
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
    appendItem(fileMenu, FileNew);
    appendItem(fileMenu, FileOpen);
    fileMenu->AppendSubMenu(fileHistoryMenu, "Open Recent");
    fileMenu->AppendSeparator();
    appendItem(fileMenu, FileSave);
    appendItem(fileMenu, FileSaveAs);
    fileMenu->AppendSeparator();
    appendItem(fileMenu, FileLoadPointFile);
    appendItem(fileMenu, FileUnloadPointFile);
    fileMenu->AppendSeparator();
    appendItem(fileMenu, FileClose);
    fileMenu->SetEventHandler(eventHandler);
    return fileMenu;
}

wxMenu* AbstractApp::CreateEditMenu(wxEvtHandler* eventHandler, wxMenu* actionMenu, bool mapViewFocused) {
    using namespace TrenchBroom::Preferences;
    
    wxMenu* editMenu = new wxMenu();
    wxMenu* toolsMenu = new wxMenu();

    appendItem(editMenu, EditUndo);
    appendItem(editMenu, EditRedo);
    editMenu->AppendSeparator();
    appendItem(editMenu, EditCut);
    appendItem(editMenu, EditCopy);
    appendItem(editMenu, EditPaste);
    appendItem(editMenu, EditPasteAtOriginalPosition);
    appendItem(editMenu, EditDelete, mapViewFocused);
    editMenu->AppendSeparator();
    appendItem(editMenu, EditSelectAll);
    appendItem(editMenu, EditSelectTouching);
    appendItem(editMenu, EditSelectNone);
    editMenu->AppendSeparator();
    appendItem(editMenu, EditHideSelected);
    appendItem(editMenu, EditHideUnselected);
    appendItem(editMenu, EditUnhideAll);
    editMenu->AppendSeparator();
    appendItem(editMenu, EditLockSelected);
    appendItem(editMenu, EditLockUnselected);
    appendItem(editMenu, EditUnlockAll);
    
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
    appendCheckItem(editMenu, EditToggleTextureLock);
    appendItem(editMenu, EditShowMapProperties);

    toolsMenu->SetEventHandler(eventHandler);
    editMenu->SetEventHandler(eventHandler);

    return editMenu;
}

wxMenu* AbstractApp::CreateViewMenu(wxEvtHandler* eventHandler, bool mapViewFocused) {
    using namespace TrenchBroom::Preferences;

    wxMenu* viewMenu = new wxMenu();
    wxMenu* gridMenu = new wxMenu();
    wxMenu* cameraMenu = new wxMenu();
    
    appendCheckItem(gridMenu, ViewGridToggleShowGrid);
    appendCheckItem(gridMenu, ViewGridToggleSnapToGrid);
    gridMenu->AppendSeparator();
    appendItem(gridMenu, ViewGridIncGridSize);
    appendItem(gridMenu, ViewGridDecGridSize);
    gridMenu->AppendSeparator();
    appendCheckItem(gridMenu, ViewGridSetSize1);
    appendCheckItem(gridMenu, ViewGridSetSize2);
    appendCheckItem(gridMenu, ViewGridSetSize4);
    appendCheckItem(gridMenu, ViewGridSetSize8);
    appendCheckItem(gridMenu, ViewGridSetSize16);
    appendCheckItem(gridMenu, ViewGridSetSize32);
    appendCheckItem(gridMenu, ViewGridSetSize64);
    appendCheckItem(gridMenu, ViewGridSetSize128);
    appendCheckItem(gridMenu, ViewGridSetSize256);

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
    appendItem(cameraMenu, ViewCameraCenterCameraOnSelection);

    cameraMenu->SetEventHandler(eventHandler);
    viewMenu->AppendSubMenu(cameraMenu, wxT("Camera"));

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
    appendItem(objectActionMenu, EditActionsRollObjectsCW, mapViewFocused);
    appendItem(objectActionMenu, EditActionsRollObjectsCCW, mapViewFocused);
    appendItem(objectActionMenu, EditActionsYawObjectsCW, mapViewFocused);
    appendItem(objectActionMenu, EditActionsYawObjectsCCW, mapViewFocused);
    appendItem(objectActionMenu, EditActionsPitchObjectsCW, mapViewFocused);
    appendItem(objectActionMenu, EditActionsPitchObjectsCCW, mapViewFocused);
    objectActionMenu->AppendSeparator();
    appendItem(objectActionMenu, EditActionsFlipObjectsHorizontally);
    appendItem(objectActionMenu, EditActionsFlipObjectsVertically);
    objectActionMenu->AppendSeparator();
    appendItem(objectActionMenu, EditActionsDuplicateObjects);
    objectActionMenu->AppendSeparator();
    appendItem(objectActionMenu, EditActionsCorrectVertices);
    appendItem(objectActionMenu, EditActionsSnapVertices);

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
    appendItem(vertexActionMenu, EditActionsCorrectVertices);
    appendItem(vertexActionMenu, EditActionsSnapVertices);

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
    TrenchBroom::View::PreferencesDialog dialog;

    int width = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
    int height = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
    wxPoint pos((width - dialog.GetSize().x) / 2, (height - dialog.GetSize().y) / 2);
    dialog.SetPosition(pos);

    dialog.ShowModal();
}

void AbstractApp::OnHelpShowHelp(wxCommandEvent& event) {
    assert(m_helpController != NULL);
    m_helpController->DisplaySection(01);
}


void AbstractApp::OnUpdateMenuItem(wxUpdateUIEvent& event) {
    if (event.GetId() == wxID_ABOUT ||
        event.GetId() == wxID_PREFERENCES ||
        event.GetId() == TrenchBroom::View::CommandIds::Menu::HelpShowHelp)
        event.Enable(true);
    else
        event.Enable(false);
    if (GetTopWindow() != NULL)
        event.Skip();
}

void AbstractApp::OnAnimation(TrenchBroom::View::AnimationEvent& event) {
    event.execute();
}

int AbstractApp::FilterEvent(wxEvent& event) {
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

    return wxApp::FilterEvent(event);
}
