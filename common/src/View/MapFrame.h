/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
#include "Renderer/PerspectiveCamera.h"
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
        class InfoPanel;
        class FrameManager;
        class Inspector;
        class KeyboardShortcut;
        class MapView;
        class NavBar;
        class StatusBar;
        
        class MapFrame : public wxFrame {
        private:
            FrameManager* m_frameManager;
            MapDocumentSPtr m_document;
            ControllerSPtr m_controller;
            Renderer::PerspectiveCamera m_camera3D;
            
            Autosaver* m_autosaver;
            wxTimer* m_autosaveTimer;

            InfoPanel* m_infoPanel;
            NavBar* m_navBar;
            MapView* m_mapView;
            Inspector* m_inspector;
            StatusBar* m_statusBar;
        public:
            MapFrame();
            MapFrame(FrameManager* frameManager, MapDocumentSPtr document);
            void Create(FrameManager* frameManager, MapDocumentSPtr document);
            ~MapFrame();
            
            Logger* logger() const;
            void positionOnScreen(wxFrame* reference);
            
            bool newDocument(Model::GamePtr game, Model::MapFormat::Type mapFormat);
            bool openDocument(Model::GamePtr game, const IO::Path& path);
            
            void setMapViewDropTarget();
            void clearMapViewDropTarget();
            
            void OnNavBarSize(wxSizeEvent& event);
            
            void OnClose(wxCloseEvent& event);
            
            void OnFileSave(wxCommandEvent& event);
            void OnFileSaveAs(wxCommandEvent& event);
            void OnFileLoadPointFile(wxCommandEvent& event);
            void OnFileUnloadPointFile(wxCommandEvent& event);
            void OnFileClose(wxCommandEvent& event);
            
            void OnEditUndo(wxCommandEvent& event);
            void OnEditRedo(wxCommandEvent& event);
            void OnEditCut(wxCommandEvent& event);
            void OnEditCopy(wxCommandEvent& event);
            void OnEditPaste(wxCommandEvent& event);
            void OnEditPasteAtOriginalPosition(wxCommandEvent& event);
            
            void OnEditSelectAll(wxCommandEvent& event);
            void OnEditSelectSiblings(wxCommandEvent& event);
            void OnEditSelectTouching(wxCommandEvent& event);
            void OnEditSelectContained(wxCommandEvent& event);
            void OnEditSelectByLineNumber(wxCommandEvent& event);
            void OnEditSelectNone(wxCommandEvent& event);
            
            void OnEditSnapVertices(wxCommandEvent& event);

            void OnEditToggleTextureTool(wxCommandEvent& event);
            
            void OnEditToggleTextureLock(wxCommandEvent& event);
            
            void OnViewToggleShowGrid(wxCommandEvent& event);
            void OnViewToggleSnapToGrid(wxCommandEvent& event);
            void OnViewIncGridSize(wxCommandEvent& event);
            void OnViewDecGridSize(wxCommandEvent& event);
            void OnViewSetGridSize(wxCommandEvent& event);
            
            void OnViewMoveCameraToNextPoint(wxCommandEvent& event);
            void OnViewMoveCameraToPreviousPoint(wxCommandEvent& event);
            void OnViewCenterCameraOnSelection(wxCommandEvent& event);
            
            void OnViewSwitchToMapInspector(wxCommandEvent& event);
            void OnViewSwitchToEntityInspector(wxCommandEvent& event);
            void OnViewSwitchToFaceInspector(wxCommandEvent& event);
            
            void OnUpdateUI(wxUpdateUIEvent& event);
            
            void OnAutosaveTimer(wxTimerEvent& event);
            void OnIdleSetFocusToMapView(wxIdleEvent& event);

            DECLARE_DYNAMIC_CLASS(MapFrame)
        private:
            void bindObservers();
            void unbindObservers();

            void preferenceDidChange(const IO::Path& path);
            void commandDone(Controller::Command::Ptr command);
            void commandUndone(Controller::Command::Ptr command);

            void createGui();
            void bindEvents();

            void rebuildMenuBar();
            void createMenuBar();
            void updateTitle();

            bool confirmOrDiscardChanges();
            
            bool saveDocument();
            bool saveDocumentAs();
            
            void pasteObjects(const Model::ObjectList& objects, const Vec3& delta);
            void collectPastedObjects(const Model::ObjectList& objects, Model::ObjectParentList& pastedObjects, Model::ObjectList& selectableObjects);
        };
    }
}

#endif /* defined(__TrenchBroom__MapFrame__) */
