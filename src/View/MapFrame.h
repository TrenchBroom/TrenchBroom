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

#include "IO/Path.h"
#include "Controller/Command.h"
#include "Model/ModelTypes.h"
#include "View/ControllerFacade.h"
#include "View/ViewTypes.h"

#include <wx/frame.h>

class wxTimer;
class wxTimerEvent;

namespace TrenchBroom {
    class Logger;

    namespace Model {
        class BrushFace;
        class Model;
        class SelectionResult;
    }
    
    namespace View {
        class Autosaver;
        class Console;
        class FrameManager;
        class Inspector;
        class KeyboardShortcut;
        class MapView;
        class NavBar;
        
        class MapFrame : public wxFrame {
        public:
            static const wxEventType EVT_REBUILD_MENUBAR;
        private:
            FrameManager* m_frameManager;
            MapDocumentPtr m_document;
            ControllerPtr m_controller;
            Autosaver* m_autosaver;
            wxTimer* m_autosaveTimer;

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
            
            void OnFileSave(wxCommandEvent& event);
            void OnFileSaveAs(wxCommandEvent& event);
            void OnFileClose(wxCommandEvent& event);
            
            void OnEditUndo(wxCommandEvent& event);
            void OnEditRedo(wxCommandEvent& event);
            void OnEditCut(wxCommandEvent& event);
            void OnEditCopy(wxCommandEvent& event);
            void OnEditPaste(wxCommandEvent& event);
            void OnEditPasteAtOriginalPosition(wxCommandEvent& event);
            void OnEditDeleteObjects(wxCommandEvent& event);
            
            void OnEditSelectAll(wxCommandEvent& event);
            void OnEditSelectSiblings(wxCommandEvent& event);
            void OnEditSelectTouching(wxCommandEvent& event);
            void OnEditSelectContained(wxCommandEvent& event);
            void OnEditSelectByLineNumber(wxCommandEvent& event);
            void OnEditSelectNone(wxCommandEvent& event);
            
            void OnEditToggleClipTool(wxCommandEvent& event);
            void OnEditToggleClipSide(wxCommandEvent& event);
            void OnEditPerformClip(wxCommandEvent& event);
            void OnEditDeleteLastClipPoint(wxCommandEvent& event);
            
            void OnEditToggleRotateObjectsTool(wxCommandEvent& event);
            void OnEditToggleMovementRestriction(wxCommandEvent& event);
            
            void OnUpdateUI(wxUpdateUIEvent& event);
            
            void OnMapViewSetFocus(wxFocusEvent& event);
            void OnMapViewKillFocus(wxFocusEvent& event);
            
            void OnRebuildMenuBar(wxEvent& event);
            void OnAutosaveTimer(wxTimerEvent& event);

            DECLARE_DYNAMIC_CLASS(MapFrame)
        private:
            void bindObservers();
            void unbindObservers();
            
            void selectionDidChange(const Model::SelectionResult& result);
            void commandDone(Controller::Command::Ptr command);
            void commandUndone(Controller::Command::Ptr command);

            void createGui();
            void bindEvents();
            void rebuildMenuBar();
            void createMenuBar(const bool showModifiers);
            void updateMenuBar(const bool showModifiers);
            void updateTitle();

            bool confirmOrDiscardChanges();
            
            bool saveDocument();
            bool saveDocumentAs();
            void pasteObjects(const Model::ObjectList& objects, const Vec3& delta);
            Model::ObjectList collectPastedObjects(const Model::ObjectList& objects);
        };
    }
}

#endif /* defined(__TrenchBroom__MapFrame__) */
