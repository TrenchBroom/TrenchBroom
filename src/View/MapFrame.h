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
        public:
            static const wxEventType EVT_REBUILD_MENUBAR;
        private:
            FrameManager* m_frameManager;
            MapDocumentSPtr m_document;
            ControllerSPtr m_controller;
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
            
            bool newDocument(Model::GamePtr game);
            bool openDocument(Model::GamePtr game, const IO::Path& path);
            
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
            void OnEditDeleteObjects(wxCommandEvent& event);
            
            void OnEditSelectAll(wxCommandEvent& event);
            void OnEditSelectSiblings(wxCommandEvent& event);
            void OnEditSelectTouching(wxCommandEvent& event);
            void OnEditSelectContained(wxCommandEvent& event);
            void OnEditSelectByLineNumber(wxCommandEvent& event);
            void OnEditSelectNone(wxCommandEvent& event);
            
            void OnEditMoveObjectsForward(wxCommandEvent& event);
            void OnEditMoveObjectsBackward(wxCommandEvent& event);
            void OnEditMoveObjectsLeft(wxCommandEvent& event);
            void OnEditMoveObjectsRight(wxCommandEvent& event);
            void OnEditMoveObjectsUp(wxCommandEvent& event);
            void OnEditMoveObjectsDown(wxCommandEvent& event);
            
            void OnEditDuplicateObjectsForward(wxCommandEvent& event);
            void OnEditDuplicateObjectsBackward(wxCommandEvent& event);
            void OnEditDuplicateObjectsLeft(wxCommandEvent& event);
            void OnEditDuplicateObjectsRight(wxCommandEvent& event);
            void OnEditDuplicateObjectsUp(wxCommandEvent& event);
            void OnEditDuplicateObjectsDown(wxCommandEvent& event);
            
            void OnEditRollObjectsCW(wxCommandEvent& event);
            void OnEditRollObjectsCCW(wxCommandEvent& event);
            void OnEditPitchObjectsCW(wxCommandEvent& event);
            void OnEditPitchObjectsCCW(wxCommandEvent& event);
            void OnEditYawObjectsCW(wxCommandEvent& event);
            void OnEditYawObjectsCCW(wxCommandEvent& event);
            
            void OnEditFlipObjectsH(wxCommandEvent& event);
            void OnEditFlipObjectsV(wxCommandEvent& event);
            
            void OnEditMoveTexturesUp(wxCommandEvent& event);
            void OnEditMoveTexturesDown(wxCommandEvent& event);
            void OnEditMoveTexturesLeft(wxCommandEvent& event);
            void OnEditMoveTexturesRight(wxCommandEvent& event);
            void OnEditRotateTexturesCW(wxCommandEvent& event);
            void OnEditRotateTexturesCCW(wxCommandEvent& event);
            
            void OnEditMoveTexturesUpFine(wxCommandEvent& event);
            void OnEditMoveTexturesDownFine(wxCommandEvent& event);
            void OnEditMoveTexturesLeftFine(wxCommandEvent& event);
            void OnEditMoveTexturesRightFine(wxCommandEvent& event);
            void OnEditRotateTexturesCWFine(wxCommandEvent& event);
            void OnEditRotateTexturesCCWFine(wxCommandEvent& event);

            void OnEditToggleClipTool(wxCommandEvent& event);
            void OnEditToggleClipSide(wxCommandEvent& event);
            void OnEditPerformClip(wxCommandEvent& event);
            void OnEditDeleteLastClipPoint(wxCommandEvent& event);
            
            void OnEditToggleRotateObjectsTool(wxCommandEvent& event);

            void OnEditToggleVertexTool(wxCommandEvent& event);
            void OnEditMoveVerticesForward(wxCommandEvent& event);
            void OnEditMoveVerticesBackward(wxCommandEvent& event);
            void OnEditMoveVerticesLeft(wxCommandEvent& event);
            void OnEditMoveVerticesRight(wxCommandEvent& event);
            void OnEditMoveVerticesUp(wxCommandEvent& event);
            void OnEditMoveVerticesDown(wxCommandEvent& event);
            void OnEditSnapVertices(wxCommandEvent& event);

            void OnEditToggleTextureTool(wxCommandEvent& event);
            
            void OnEditToggleMovementRestriction(wxCommandEvent& event);
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
            
            void OnRebuildMenuBar(wxEvent& event);
            void OnAutosaveTimer(wxTimerEvent& event);
            void OnIdleSetFocusToMapView(wxIdleEvent& event);

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
            void collectPastedObjects(const Model::ObjectList& objects, Model::ObjectParentList& pastedObjects, Model::ObjectList& selectableObjects);
            
            void duplicateObjects();
            void rotateTextures(bool clockwise, bool snapAngle);
        };
    }
}

#endif /* defined(__TrenchBroom__MapFrame__) */
