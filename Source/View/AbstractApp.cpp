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
EVT_UPDATE_UI(TrenchBroom::View::CommandIds::Menu::FileOpenRecent, AbstractApp::OnUpdateMenuItem)
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

EVT_EXECUTABLE(AbstractApp::OnExecutableEvent)
END_EVENT_TABLE()

wxMenu* AbstractApp::buildMenu(const TrenchBroom::Preferences::Menu& menu, const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused) {
    using TrenchBroom::View::KeyboardShortcut;
    using TrenchBroom::Preferences::Menu;
    using TrenchBroom::Preferences::MenuItem;
    using TrenchBroom::Preferences::MultiMenu;
    using TrenchBroom::Preferences::ShortcutMenuItem;

    wxMenu* result = new wxMenu();

    const Menu::List& items = menu.items();
    Menu::List::const_iterator it, end;
    for (it = items.begin(), end = items.end(); it != end; ++it) {
        const MenuItem& item = **it;
        switch (item.type()) {
            case MenuItem::MITAction: {
                const ShortcutMenuItem& shortcutItem = static_cast<const ShortcutMenuItem&>(item);
                const KeyboardShortcut& shortcut = shortcutItem.shortcut();
                if (mapViewFocused || shortcut.alwaysShowModifier())
                    result->Append(shortcut.commandId(), shortcut.menuText());
                else
                    result->Append(shortcut.commandId(), shortcut.text());
                break;
            }
            case MenuItem::MITCheck: {
                const ShortcutMenuItem& shortcutItem = static_cast<const ShortcutMenuItem&>(item);
                const KeyboardShortcut& shortcut = shortcutItem.shortcut();
                if (mapViewFocused || shortcut.alwaysShowModifier())
                    result->AppendCheckItem(shortcut.commandId(), shortcut.menuText());
                else
                    result->AppendCheckItem(shortcut.commandId(), shortcut.text());
                break;
            }
            case MenuItem::MITMenu: {
                const Menu& subMenu = static_cast<const Menu&>(item);
                wxMenuItem* wxSubMenuItem = new wxMenuItem(result, subMenu.menuId(), subMenu.text());
                wxSubMenuItem->SetSubMenu(buildMenu(subMenu, selector, eventHandler, mapViewFocused));
                result->Append(wxSubMenuItem);
                break;
            }
            case MenuItem::MITMultiMenu: {
                const MultiMenu& multiMenu = static_cast<const MultiMenu&>(item);
                const Menu* multiMenuItem = multiMenu.selectMenu(selector);
                if (multiMenuItem != NULL) {
                    wxMenuItem* wxSubMenuItem = new wxMenuItem(result, multiMenu.menuId(), multiMenu.text());
                    wxSubMenuItem->SetSubMenu(buildMenu(*multiMenuItem, selector, eventHandler, mapViewFocused));
                    result->Append(wxSubMenuItem);
                } else {
                    result->Append(multiMenu.menuId(), multiMenu.text());
                }
                break;
            }
            case MenuItem::MITSeparator: {
                result->AppendSeparator();
                break;
            }
        }
    }

    result->SetEventHandler(eventHandler);
    return result;
}

wxMenu* AbstractApp::CreateFileMenu(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused) {
    using TrenchBroom::Preferences::PreferenceManager;
    using TrenchBroom::Preferences::Menu;

    PreferenceManager& prefs = PreferenceManager::preferences();
    const Menu& menu = prefs.getMenu(TrenchBroom::Preferences::FileMenu);
    wxMenu* fileMenu = buildMenu(menu, selector, eventHandler, mapViewFocused);

    wxMenuItem* openRecentItem = fileMenu->FindItem(TrenchBroom::View::CommandIds::Menu::FileOpenRecent);
    assert(openRecentItem != NULL);
    wxMenu* openRecentMenu = openRecentItem->GetSubMenu();
    assert(openRecentMenu != NULL);
    m_docManager->FileHistoryUseMenu(openRecentMenu);
    m_docManager->FileHistoryAddFilesToMenu(openRecentMenu);
    openRecentMenu->SetEventHandler(m_docManager);

    return fileMenu;
}

wxMenu* AbstractApp::CreateEditMenu(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused) {
    using TrenchBroom::Preferences::PreferenceManager;
    using TrenchBroom::Preferences::Menu;

    PreferenceManager& prefs = PreferenceManager::preferences();
    const Menu& menu = prefs.getMenu(TrenchBroom::Preferences::EditMenu);
    return buildMenu(menu, selector, eventHandler, mapViewFocused);
}

wxMenu* AbstractApp::CreateViewMenu(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused) {
    using TrenchBroom::Preferences::PreferenceManager;
    using TrenchBroom::Preferences::Menu;

    PreferenceManager& prefs = PreferenceManager::preferences();
    const Menu& menu = prefs.getMenu(TrenchBroom::Preferences::ViewMenu);
    return buildMenu(menu, selector, eventHandler, mapViewFocused);
}

wxMenu* AbstractApp::CreateHelpMenu(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused) {
    using namespace TrenchBroom::View::CommandIds::Menu;

    wxMenu* helpMenu = new wxMenu();
    helpMenu->Append(HelpShowHelp, wxT("TrenchBroom Help"));
    helpMenu->SetEventHandler(eventHandler);
    return helpMenu;
}

wxMenuBar* AbstractApp::CreateMenuBar(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused) {
    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(CreateFileMenu(selector, eventHandler, mapViewFocused), wxT("File"));
    menuBar->Append(CreateEditMenu(selector, eventHandler, mapViewFocused), wxT("Edit"));
    menuBar->Append(CreateViewMenu(selector, eventHandler, mapViewFocused), wxT("View"));
    menuBar->Append(CreateHelpMenu(selector, this, mapViewFocused), wxT("Help"));

    return menuBar;
}

void AbstractApp::DetachFileHistoryMenu(wxMenuBar* menuBar) {
    if (menuBar != NULL) {
        int fileMenuIndex = menuBar->FindMenu(wxT("File"));
        assert(fileMenuIndex != wxNOT_FOUND);

        wxMenu* fileMenu = menuBar->GetMenu(static_cast<size_t>(fileMenuIndex));
        wxMenuItem* openRecentItem = fileMenu->FindItem(TrenchBroom::View::CommandIds::Menu::FileOpenRecent);
        assert(openRecentItem != NULL);
        wxMenu* openRecentMenu = openRecentItem->GetSubMenu();
        assert(openRecentMenu != NULL);

        m_docManager->FileHistoryRemoveMenu(openRecentMenu);
    }
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

    m_lastActivationEvent = 0;
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
    if (m_preferencesFrame == NULL) {
        TrenchBroom::View::PreferencesFrame* frame = new TrenchBroom::View::PreferencesFrame();
        frame->CenterOnScreen();
        frame->Show();
    } else {
        m_preferencesFrame->Raise();
    }
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
             event.GetId() == wxID_OPEN ||
             event.GetId() == TrenchBroom::View::CommandIds::Menu::FileOpenRecent)
        event.Enable(m_preferencesFrame == NULL);
    else
        event.Enable(false);

    if (GetTopWindow() != NULL && m_preferencesFrame == NULL)
        event.Skip();
}

void AbstractApp::OnExecutableEvent(TrenchBroom::ExecutableEvent& event) {
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
        } else if (event.GetEventType() == wxEVT_ACTIVATE) {
            m_lastActivationEvent = wxGetLocalTimeMillis();
        } else if (event.GetEventType() == wxEVT_LEFT_DOWN ||
                   event.GetEventType() == wxEVT_MIDDLE_DOWN ||
                   event.GetEventType() == wxEVT_RIGHT_DOWN ||
                   event.GetEventType() == wxEVT_LEFT_UP ||
                   event.GetEventType() == wxEVT_MIDDLE_UP ||
                   event.GetEventType() == wxEVT_RIGHT_UP) {
            if (wxGetLocalTimeMillis() - m_lastActivationEvent <= 10)
                return 1;
        }
    }

    return wxApp::FilterEvent(event);
}
