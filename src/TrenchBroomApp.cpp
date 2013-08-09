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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TrenchBroomApp.h"

#include <clocale>

#include "IO/Path.h"
#include "Model/QuakeGame.h"
#include "View/CommandIds.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "View/Menu.h"

#include <wx/filedlg.h>

#ifndef TESTING
IMPLEMENT_APP(TrenchBroom::View::TrenchBroomApp)
#endif

namespace TrenchBroom {
    namespace View {
        TrenchBroomApp::TrenchBroomApp() :
        wxApp(),
        m_frameManager(NULL),
        m_recentDocuments(CommandIds::Menu::FileRecentDocuments, 10) {}
        
        FrameManager* TrenchBroomApp::frameManager() {
            return m_frameManager;
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

        bool TrenchBroomApp::OnInit() {
            if (!wxApp::OnInit())
                return false;
            
            std::setlocale(LC_NUMERIC, "C");
            
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
            Bind(wxEVT_COMMAND_MENU_SELECTED, &TrenchBroomApp::OnOpenPreferences, this, wxID_PREFERENCES);
            
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

#ifndef __APPLE__
            if (wxApp::argc > 1) {
                const wxString filename = wxApp::argv[1];
                return openDocument(filename.ToStdString());
            } else {
                return newDocument();
            }
#endif
            return true;
        }
        
        int TrenchBroomApp::OnExit() {
            delete m_frameManager;
            m_frameManager = NULL;
            return wxApp::OnExit();
        }
        
        void TrenchBroomApp::OnUnhandledException() {
            try {
                throw;
            } catch (std::exception& e) {
                wxLogError(e.what());
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
        
        int TrenchBroomApp::FilterEvent(wxEvent& event) {
            /*
             Because the Ubuntu window manager will unfocus the map view when a menu is opened, we track all SET_FOCUS
             events here and send a separate event if any control other than the map view receives the focus. This event
             will be added to the event queue here and then dispatched directly to the map frame containing the focused
             control once it is filtered here, too.
             */
            
            if (event.GetEventObject() != NULL) {
                if (event.GetEventType() == wxEVT_SET_FOCUS) {
                    wxObject* object = event.GetEventObject();
                    wxWindow* window = wxDynamicCast(object, wxWindow);
                    if (window != NULL) {
                        // find the frame containing the focused control
                        wxFrame* frame = wxDynamicCast(window, wxFrame);
                        wxWindow* parent = window->GetParent();
                        while (frame == NULL && parent != NULL) {
                            frame = wxDynamicCast(parent, wxFrame);
                            parent = parent->GetParent();
                        }

                        /*
                         If we found a frame, then send a command event to the frame that will cause it to rebuild its
                         menu. The frame must keep track of whether the menu actually needs to be rebuilt (only if the
                         map view previously had focus and just lost it or vice versa).
                         Make sure the command is sent via AddPendingEvent to give wxWidgets a chance to update the 
                         focus states!
                         */
                        if (frame != NULL) {
                            wxCommandEvent buildMenuEvent(MapFrame::EVT_REBUILD_MENU);
                            buildMenuEvent.SetClientData(event.GetEventObject());
                            buildMenuEvent.SetEventObject(frame);
                            buildMenuEvent.SetId(event.GetId());
                            AddPendingEvent(buildMenuEvent);
                        }
                    }
                } else if (event.GetEventType() == MapFrame::EVT_REBUILD_MENU) {
                    wxFrame* frame = wxStaticCast(event.GetEventObject(), wxFrame);
                    frame->ProcessWindowEventLocally(event);
                    return 1;
                }
            }
            return wxApp::FilterEvent(event);
        }

#ifdef __APPLE__
        void TrenchBroomApp::OnOpenPreferences(wxCommandEvent& event) {
        }

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
            newDocument();
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
        
        bool TrenchBroomApp::newDocument() {
            MapFrame* frame = m_frameManager->newFrame();
            const IO::Path quakePath("/Applications/Quake");
            Model::GamePtr game = Model::QuakeGame::newGame(quakePath, Color(1.0f, 1.0f, 1.0f, 1.0f), frame->logger());
            return frame != NULL && frame->newDocument(game);
        }
        
        bool TrenchBroomApp::openDocument(const String& pathStr) {
            MapFrame* frame = m_frameManager->newFrame();
            try {
                const IO::Path quakePath("/Applications/Quake");
                Model::GamePtr game = Model::QuakeGame::newGame(quakePath, Color(1.0f, 1.0f, 1.0f, 1.0f), frame->logger());
                const IO::Path path(pathStr);
                return frame != NULL && frame->openDocument(game, path);
            } catch (...) {
                if (frame != NULL)
                    frame->Close();
                return false;
            }
        }
    }
}
