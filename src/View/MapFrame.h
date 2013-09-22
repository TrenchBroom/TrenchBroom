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

#ifndef __TrenchBroom__MapFrame__
#define __TrenchBroom__MapFrame__

#include "Controller/Command.h"
#include "Controller/CommandListener.h"
#include "IO/Path.h"
#include "Model/ModelTypes.h"
#include "View/ControllerFacade.h"
#include "View/ViewTypes.h"

#include <wx/frame.h>

namespace TrenchBroom {
    class Logger;

    namespace View {
        class Console;
        class FrameManager;
        class Inspector;
        class KeyboardShortcut;
        class MapView;
        class NavBar;
        
        class MapFrame : public wxFrame, public Controller::CommandListener {
        public:
            static const wxEventType EVT_REBUILD_MENU;
        private:
            FrameManager* m_frameManager;
            ControllerFacade m_controller;
            MapDocumentPtr m_document;

            Console* m_console;
            NavBar* m_navBar;
            MapView* m_mapView;
            Inspector* m_inspector;
            bool m_menuNeedsRebuilding;
        public:
            MapFrame();
            MapFrame(FrameManager* frameManager, MapDocumentPtr document);
            void Create(FrameManager* frameManager, MapDocumentPtr document);
            ~MapFrame();
            
            Logger* logger() const;
            void positionOnScreen(wxFrame* reference);
            
            bool newDocument(Model::GamePtr game);
            bool openDocument(Model::GamePtr game, const IO::Path& path);
            
            void OnClose(wxCloseEvent& event);
            
            void OnFileClose(wxCommandEvent& event);
            
            void OnEditUndo(wxCommandEvent& event);
            void OnEditRedo(wxCommandEvent& event);
            void OnEditToggleClipTool(wxCommandEvent& event);
            
            void OnUpdateUI(wxUpdateUIEvent& event);
            
            void OnMapViewSetFocus(wxFocusEvent& event);
            void OnMapViewKillFocus(wxFocusEvent& event);
            
            void OnRebuildMenu(wxEvent& event);
            
            void commandDo(Controller::Command::Ptr command);
            void commandDone(Controller::Command::Ptr command);
            void commandDoFailed(Controller::Command::Ptr command);
            void commandUndo(Controller::Command::Ptr command);
            void commandUndone(Controller::Command::Ptr command);
            void commandUndoFailed(Controller::Command::Ptr command);

            DECLARE_DYNAMIC_CLASS(MapFrame)
        private:
            void createGui();
            void createMenuBar(const bool showModifiers);
            void updateMenuBar(const bool showModifiers);
            void updateTitle();

            bool confirmOrDiscardChanges();
            
            bool saveDocument();
            bool saveDocumentAs(const IO::Path& path);
        };
    }
}

#endif /* defined(__TrenchBroom__MapFrame__) */
