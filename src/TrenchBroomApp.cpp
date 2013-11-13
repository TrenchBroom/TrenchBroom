/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "TrenchBroomApp.h"

#include <clocale>

#include "IO/Path.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "View/CommandIds.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "View/MapView.h"
#include "View/Menu.h"
#include "View/NewDocumentDialog.h"
#include "View/PreferenceDialog.h"
#include "View/WelcomeFrame.h"

#include <wx/choicdlg.h>
#include <wx/filedlg.h>

#ifndef TESTING
IMPLEMENT_APP(TrenchBroom::View::TrenchBroomApp)
#endif

namespace TrenchBroom {
    namespace View {
        TrenchBroomApp& TrenchBroomApp::instance() {
            TrenchBroomApp* app = static_cast<TrenchBroomApp*>(wxTheApp);
            return *app;
        }
        
        TrenchBroomApp::TrenchBroomApp() :
        wxApp(),
        m_frameManager(NULL),
        m_recentDocuments(CommandIds::Menu::FileRecentDocuments, 10),
        m_lastFocusedWindow(NULL),
        m_lastFocusedWindowIsMapView(false) {}
        
        FrameManager* TrenchBroomApp::frameManager() {
            return m_frameManager;
        }
        
        const IO::Path::List& TrenchBroomApp::recentDocuments() const {
            return m_recentDocuments.recentDocuments();
        }
        
        void TrenchBroomApp::addRecentDocumentMenu(wxMenu* menu) {
            m_recentDocuments.addMenu(menu);
        }
        
        void TrenchBroomApp::removeRecentDocumentMenu(wxMenu* menu) {
            m_recentDocuments.removeMenu(menu);
        }
        
        void TrenchBroomApp::updateRecentDocument(const IO::Path& path) {
            m_recentDocuments.updatePath(path);
        }
        
        bool TrenchBroomApp::newDocument() {
            // Todo: Query the game
            
            Model::GamePtr game = detectGame(NULL);
            if (game == NULL)
                return false;

            MapFrame* frame = m_frameManager->newFrame();
            return frame != NULL && frame->newDocument(game);
        }
        
        bool TrenchBroomApp::openDocument(const String& pathStr) {
            MapFrame* frame = NULL;
            try {
                const IO::Path path(pathStr);
                // TODO: use a separate function with a dedicated game selection dialog here
                Model::GamePtr game = detectGame(NULL, path);
                if (game == NULL)
                    return false;
                MapFrame* frame = m_frameManager->newFrame();
                return frame != NULL && frame->openDocument(game, path);
            } catch (...) {
                if (frame != NULL)
                    frame->Close();
                return false;
            }
        }
        
        bool TrenchBroomApp::OnInit() {
            if (!wxApp::OnInit())
                return false;
            
            std::setlocale(LC_NUMERIC, "C");
            
            // load image handlers
            wxImage::AddHandler(new wxPNGHandler());
            
            assert(m_frameManager == NULL);
            m_frameManager = new FrameManager(useSDI());
            
            m_recentDocuments.setHandler(this, &TrenchBroomApp::OnFileOpenRecent);
            
#ifdef __APPLE__
            SetExitOnFrameDelete(false);
            wxMenuBar* menuBar = Menu::createMenuBar(TrenchBroom::View::NullMenuSelector(), false);
            wxMenuBar::MacSetCommonMenuBar(menuBar);
            
            wxMenu* recentDocumentsMenu = Menu::findRecentDocumentsMenu(menuBar);
            assert(recentDocumentsMenu != NULL);
            m_recentDocuments.addMenu(recentDocumentsMenu);
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &TrenchBroomApp::OnFileExit, this, wxID_EXIT);
            
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_NEW);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_OPEN);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_SAVE);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_SAVEAS);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_CLOSE);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_UNDO);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_REDO);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_CUT);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_COPY);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_PASTE);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, wxID_DELETE);
            Bind(wxEVT_UPDATE_UI, &TrenchBroomApp::OnUpdateUI, this, CommandIds::Menu::Lowest, CommandIds::Menu::Highest);
#endif
            
            Bind(wxEVT_COMMAND_MENU_SELECTED, &TrenchBroomApp::OnFileNew, this, wxID_NEW);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &TrenchBroomApp::OnFileOpen, this, wxID_OPEN);
            Bind(wxEVT_COMMAND_MENU_SELECTED, &TrenchBroomApp::OnOpenPreferences, this, wxID_PREFERENCES);
            
#ifndef __APPLE__
            if (wxApp::argc > 1) {
                const wxString filename = wxApp::argv[1];
                if (!openDocument(filename.ToStdString()))
                    return false;
            } else {
                showWelcomeFrame();
            }
#endif
            
            m_lastActivation = 0;
            return true;
        }
        
        int TrenchBroomApp::OnExit() {
            delete m_frameManager;
            m_frameManager = NULL;
            
            wxImage::CleanUpHandlers();
            return wxApp::OnExit();
        }
        
        void TrenchBroomApp::OnUnhandledException() {
            try {
                throw;
            } catch (std::exception& e) {
                wxLogError(e.what());
            } catch (...) {
                wxLogError("Unhandled exception");
            }
        }
        
        void TrenchBroomApp::OnFileNew(wxCommandEvent& event) {
            newDocument();
        }
        
        void TrenchBroomApp::OnFileOpen(wxCommandEvent& event) {
            const wxString pathStr = ::wxLoadFileSelector("", "map", "", NULL);
            if (!pathStr.empty())
                openDocument(pathStr.ToStdString());
        }
        
        void TrenchBroomApp::OnFileOpenRecent(wxCommandEvent& event) {
            const wxVariant* object = static_cast<wxVariant*>(event.m_callbackUserData); // this must be changed in 2.9.5 to event.GetEventUserData()
            assert(object != NULL);
            const wxString data = object->GetString();
            if (!openDocument(data.ToStdString())) {
                m_recentDocuments.removePath(IO::Path(data.ToStdString()));
                ::wxMessageBox(data.ToStdString() + " could not be opened.", "TrenchBroom", wxOK, NULL);
            }
        }
        
        void TrenchBroomApp::OnOpenPreferences(wxCommandEvent& event) {
            PreferenceDialog dialog;
            dialog.ShowModal();
        }
        
        int TrenchBroomApp::FilterEvent(wxEvent& event) {
            /*
             Because the Ubuntu window manager will unfocus the map view when a menu is opened, we track all SET_FOCUS
             events here and send a separate event either
             - the map view itself receives a focus event and it was not the last control to receive one, or
             - another control receives a focus event and the last control to do so was the map view.
             An event will be added to the event queue here and then dispatched directly to the map frame containing the
             focused control once it is filtered here, too.
             */
            
            if (event.GetEventObject() != NULL) {
                if (event.GetEventType() == wxEVT_SET_FOCUS) {
                    // find the frame containing the focused control
                    wxWindow* window = wxDynamicCast(event.GetEventObject(), wxWindow);
                    wxFrame* frame = wxDynamicCast(window, wxFrame);
                    wxWindow* parent = window->GetParent();
                    while (frame == NULL && parent != NULL) {
                        frame = wxDynamicCast(parent, wxFrame);
                        parent = parent->GetParent();
                    }
                    
                    if (frame != NULL) {
                        if (window != m_lastFocusedWindow) {
                            const bool windowIsMapView = wxDynamicCast(window, MapView) != NULL;
                            if (windowIsMapView || m_lastFocusedWindowIsMapView) {
                                /*
                                 If we found a frame, then send a command event to the frame that will cause it to rebuild its
                                 menu.
                                 Make sure the command is sent via AddPendingEvent to give wxWidgets a chance to update the
                                 focus states!
                                 */
                                if (frame != NULL) {
                                    wxCommandEvent buildMenuEvent(MapFrame::EVT_REBUILD_MENUBAR);
                                    buildMenuEvent.SetClientData(event.GetEventObject());
                                    buildMenuEvent.SetEventObject(frame);
                                    buildMenuEvent.SetId(event.GetId());
                                    AddPendingEvent(buildMenuEvent);
                                }
                            }
                            m_lastFocusedWindow = window;
                            m_lastFocusedWindowIsMapView = windowIsMapView;
                        }
                    }
                } else if (event.GetEventType() == MapFrame::EVT_REBUILD_MENUBAR) {
                    wxFrame* frame = wxStaticCast(event.GetEventObject(), wxFrame);
                    frame->ProcessWindowEventLocally(event);
                    return 1;
                } else if (event.GetEventType() == wxEVT_ACTIVATE) {
                    m_lastActivation = wxGetLocalTimeMillis();
                } else if (event.GetEventType() == wxEVT_LEFT_DOWN ||
                           event.GetEventType() == wxEVT_MIDDLE_DOWN ||
                           event.GetEventType() == wxEVT_RIGHT_DOWN ||
                           event.GetEventType() == wxEVT_LEFT_UP ||
                           event.GetEventType() == wxEVT_MIDDLE_UP ||
                           event.GetEventType() == wxEVT_RIGHT_UP) {
                    if (wxGetLocalTimeMillis() - m_lastActivation <= 10)
                        return 1;
                }
            }
            return wxApp::FilterEvent(event);
        }
        
#ifdef __APPLE__
        void TrenchBroomApp::OnFileExit(wxCommandEvent& event) {
            Exit();
        }
        
        void TrenchBroomApp::OnUpdateUI(wxUpdateUIEvent& event) {
            switch (event.GetId()) {
                case wxID_PREFERENCES:
                case wxID_NEW:
                case wxID_OPEN:
                case wxID_EXIT:
                case CommandIds::Menu::FileOpenRecent:
                    event.Enable(true);
                    break;
                default:
                    if (event.GetId() >= CommandIds::Menu::FileRecentDocuments &&
                        event.GetId() < CommandIds::Menu::FileRecentDocuments + 10)
                        event.Enable(true);
                    else if (m_frameManager->allFramesClosed())
                        event.Enable(false);
                    break;
            }
        }
        
        void TrenchBroomApp::MacNewFile() {
            showWelcomeFrame();
        }
        
        void TrenchBroomApp::MacOpenFiles(const wxArrayString& filenames) {
            wxArrayString::const_iterator it, end;
            for (it = filenames.begin(), end = filenames.end(); it != end; ++it) {
                const wxString& filename = *it;
                openDocument(filename.ToStdString());
            }
        }
#endif
        
        bool TrenchBroomApp::useSDI() {
#ifdef _WIN32
            return true;
#else
            return false;
#endif
        }
        
        void TrenchBroomApp::showWelcomeFrame() {
            WelcomeFrame* welcomeFrame = new WelcomeFrame();
            welcomeFrame->Show();
        }
        
        Model::GamePtr TrenchBroomApp::detectGame(wxWindow* parent, const IO::Path& path) {
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();
            Model::GamePtr game = gameFactory.detectGame(path);
            if (game != NULL)
                return game;

            NewDocumentDialog dialog(parent);
            if (dialog.ShowModal() != wxID_OK)
                return Model::GamePtr();

            const String gameName = dialog.selectedGameName();
            return gameFactory.createGame(gameName);
        }
    }
}
