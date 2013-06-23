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
#include "View/Menu.h"

#include <wx/filedlg.h>

#ifndef TESTING
IMPLEMENT_APP(TrenchBroom::View::TrenchBroomApp)
#endif

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(TrenchBroomApp, wxApp)
        EVT_MENU(wxID_NEW, TrenchBroomApp::OnFileNew)
        EVT_MENU(wxID_OPEN, TrenchBroomApp::OnFileOpen)

#ifdef __APPLE__
        EVT_MENU(wxID_PREFERENCES, TrenchBroomApp::OnOpenPreferences)
        EVT_MENU(wxID_EXIT, TrenchBroomApp::OnFileExit)

        EVT_UPDATE_UI(wxID_PREFERENCES, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI(wxID_NEW, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI(wxID_OPEN, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI(wxID_EXIT, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI(wxID_SAVE, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI(wxID_SAVEAS, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI(wxID_CLOSE, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI(wxID_UNDO, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI(wxID_REDO, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI(wxID_CUT, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI(wxID_COPY, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI(wxID_PASTE, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI(wxID_DELETE, TrenchBroomApp::OnUpdateUI)
        EVT_UPDATE_UI_RANGE(CommandIds::Menu::Lowest, CommandIds::Menu::Highest, TrenchBroomApp::OnUpdateUI)
#endif
        END_EVENT_TABLE()
        
        TrenchBroomApp::TrenchBroomApp() :
        wxApp(),
        m_documentManager(NULL) {}
        
        bool TrenchBroomApp::OnInit() {
            if (!wxApp::OnInit())
                return false;
            
            std::setlocale(LC_NUMERIC, "C");
            
            assert(m_documentManager == NULL);
            m_documentManager = new TrenchBroom::View::DocumentManager(useSDI());
            
#ifdef __APPLE__
            SetExitOnFrameDelete(false);
            wxMenuBar* menuBar = Menu::createMenuBar(TrenchBroom::View::NullMenuSelector(), false);
            wxMenuBar::MacSetCommonMenuBar(menuBar);
#endif
            
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
            delete m_documentManager;
            m_documentManager = NULL;
            
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
                    event.Enable(true);
                    break;
                default:
                    if (m_documentManager->documents().empty())
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
            Model::Game::Ptr game = Model::QuakeGame::newGame();
            MapDocument::Ptr document = m_documentManager->newDocument(game);
            if (document == NULL)
                return false;
            document->createOrRaiseFrame();
            return true;
        }
        
        bool TrenchBroomApp::openDocument(const String& pathStr) {
            Model::Game::Ptr game = Model::QuakeGame::newGame();
            const IO::Path path(pathStr);
            MapDocument::Ptr document = m_documentManager->openDocument(game, path);
            if (document == NULL)
                return false;
            document->createOrRaiseFrame();
            return true;
        }
    }
}
