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
        m_frameManager(NULL) {}
        
        FrameManager* TrenchBroomApp::frameManager() {
            return m_frameManager;
        }

        bool TrenchBroomApp::OnInit() {
            if (!wxApp::OnInit())
                return false;
            
            std::setlocale(LC_NUMERIC, "C");
            
            assert(m_frameManager == NULL);
            m_frameManager = new FrameManager(useSDI());
            
#ifdef __APPLE__
            SetExitOnFrameDelete(false);
            wxMenuBar* menuBar = Menu::createMenuBar(TrenchBroom::View::NullMenuSelector(), false);
            wxMenuBar::MacSetCommonMenuBar(menuBar);
            
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
                    if (m_frameManager->allFramesClosed())
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
            MapFrame* frame = m_frameManager->newFrame();
            return frame != NULL && frame->newDocument(game);
        }
        
        bool TrenchBroomApp::openDocument(const String& pathStr) {
            Model::Game::Ptr game = Model::QuakeGame::newGame();
            const IO::Path path(pathStr);
            MapFrame* frame = m_frameManager->newFrame();
            return frame != NULL && frame->openDocument(game, path);
        }
    }
}
