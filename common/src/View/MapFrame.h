/*
 Copyright (C) 2010-2017 Kristian Duske

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

#ifndef TrenchBroom_MapFrame
#define TrenchBroom_MapFrame

#include "Model/MapFormat.h"
#include "Model/ModelTypes.h"
//#include "View/Inspector.h"
#include "View/Selection.h"
#include "View/ViewTypes.h"
//#include "SplitterWindow2.h"

#include <QMainWindow>

#include <utility>

class QTimer;
class QLabel;
class QMenuBar;
class QAction;
class QActionGroup;
class QComboBox;

namespace TrenchBroom {
    class Logger;

    namespace IO {
        class Path;
    }

    namespace View {
        class Autosaver;
        class CommandWindowUpdateLocker;
        class Console;
        class FrameManager;
        class GLContextManager;
        class Inspector;
        class SwitchableMapViewContainer;
        class Tool;
        class ActionInfo;

        class MapFrame : public QMainWindow {
            Q_OBJECT
        private:
            FrameManager* m_frameManager;
            MapDocumentSPtr m_document;

            Autosaver* m_autosaver;
            QTimer* m_autosaveTimer;

            // FIXME: re-add all of these
#if 0
            SplitterWindow2* m_hSplitter;
            SplitterWindow2* m_vSplitter;
#endif

            GLContextManager* m_contextManager;
            SwitchableMapViewContainer* m_mapView;
#if 0
            Console* m_console;
            Inspector* m_inspector;
#endif

            // FIXME: only used for rebuildMenuBar(). drop?
            //wxWindow* m_lastFocus;

            QComboBox* m_gridChoice;
            QLabel* m_statusBarLabel;

#if 0
            wxDialog* m_compilationDialog;
#endif
            CommandWindowUpdateLocker* m_updateLocker;
        private: // actions
            QAction* fileNewAction;
            QAction* fileOpenAction;
            QAction* fileSaveAction;
            QAction* fileSaveAsAction;
            QAction* fileExportObjAction;
            QAction* fileLoadPointFileAction;
            QAction* fileReloadPointFileAction;
            QAction* fileUnloadPointFileAction;
            QAction* fileLoadPortalFileAction;
            QAction* fileReloadPortalFileAction;
            QAction* fileUnloadPortalFileAction;
            QAction* fileReloadTextureCollectionsAction;
            QAction* fileReloadEntityDefinitionsAction;
            QAction* fileCloseAction;

            QAction* editUndoAction;
            QAction* editRedoAction;
            QAction* editRepeatAction;
            QAction* editClearRepeatAction;
            QAction* editCutAction;
            QAction* editCopyAction;
            QAction* editPasteAction;
            QAction* editPasteAtOriginalPositionAction;
            QAction* editDuplicateAction;
            QAction* editDeleteAction;
            QAction* editSelectAllAction;
            QAction* editSelectSiblingsAction;
            QAction* editSelectTouchingAction;
            QAction* editSelectInsideAction;
            QAction* editSelectTallAction;
            QAction* editSelectByLineNumberAction;
            QAction* editSelectNoneAction;
            QAction* editGroupSelectedObjectsAction;
            QAction* editUngroupSelectedObjectsAction;
            QActionGroup* editToolActionGroup;
            QAction* editDeactivateToolAction;
            QAction* editToggleCreateComplexBrushToolAction;
            QAction* editToggleClipToolAction;
            QAction* editToggleRotateObjectsToolAction;
            QAction* editToggleScaleObjectsToolAction;
            QAction* editToggleShearObjectsToolAction;
            QAction* editToggleVertexToolAction;
            QAction* editToggleEdgeToolAction;
            QAction* editToggleFaceToolAction;
            QAction* editCsgConvexMergeAction;
            QAction* editCsgSubtractAction;
            QAction* editCsgIntersectAction;
            QAction* editCsgHollowAction;
            QAction* editReplaceTextureAction;
            QAction* editToggleTextureLockAction;
            QAction* editToggleUVLockAction;
            QAction* editSnapVerticesToIntegerAction;
            QAction* editSnapVerticesToGridAction;

            QAction* viewToggleShowGridAction;
            QAction* viewToggleSnapToGridAction;
            QAction* viewIncGridSizeAction;
            QAction* viewDecGridSizeAction;
            QActionGroup* viewSetGridSizeActionGroup;
            QAction* viewSetGridSize0Point125Action;
            QAction* viewSetGridSize0Point25Action;
            QAction* viewSetGridSize0Point5Action;
            QAction* viewSetGridSize1Action;
            QAction* viewSetGridSize2Action;
            QAction* viewSetGridSize4Action;
            QAction* viewSetGridSize8Action;
            QAction* viewSetGridSize16Action;
            QAction* viewSetGridSize32Action;
            QAction* viewSetGridSize64Action;
            QAction* viewSetGridSize128Action;
            QAction* viewSetGridSize256Action;

            QAction* viewMoveCameraToNextPointAction;
            QAction* viewMoveCameraToPreviousPointAction;
            QAction* viewFocusCameraOnSelectionAction;
            QAction* viewMoveCameraToPositionAction;

            QAction* viewIsolateSelectionAction;
            QAction* viewHideSelectionAction;
            QAction* viewUnhideAllAction;
            QAction* viewSwitchToMapInspectorAction;
            QAction* viewSwitchToEntityInspectorAction;
            QAction* viewSwitchToFaceInspectorAction;
            QAction* viewToggleInfoPanelAction;
            QAction* viewToggleInspectorAction;
            QAction* viewToggleMaximizeCurrentViewAction;
            QAction* viewPreferencesAction;

            QAction* runCompileAction;
            QAction* runLaunchAction;

            QAction* debugPrintVerticesAction;
            QAction* debugCreateBrushAction;
            QAction* debugCreateCubeAction;
            QAction* debugClipWithFaceAction;
            QAction* debugCopyJSShortcutsAction;
            QAction* debugCrashAction;
            QAction* debugThrowExceptionDuringCommandAction;
            QAction* debugCrashReportDialogAction;
            QAction* debugSetWindowSizeAction;

            QAction* helpManualAction;
            QAction* helpAboutAction;

            QAction* flipObjectsHorizontallyAction;
            QAction* flipObjectsVerticallyAction;

            std::vector<std::pair<QAction*, const ActionInfo*>> m_actionInfoList;
        public:
            MapFrame();
            MapFrame(FrameManager* frameManager, MapDocumentSPtr document);
            void Create(FrameManager* frameManager, MapDocumentSPtr document);
            virtual ~MapFrame();

            //void positionOnScreen(wxFrame* reference);
            MapDocumentSPtr document() const;
        public: // getters and such
            Logger* logger() const;
        public: // drop targets
            void setToolBoxDropTarget();
            void clearDropTarget();
        public: // document management
            bool newDocument(Model::GameSPtr game, Model::MapFormat mapFormat);
            bool openDocument(Model::GameSPtr game, Model::MapFormat mapFormat, const IO::Path& path);
        private:
            bool saveDocument();
            bool saveDocumentAs();
            bool exportDocumentAsObj();
            bool exportDocument(Model::ExportFormat format, const IO::Path& path);

            bool confirmOrDiscardChanges();
        private: // title bar contents
            void updateTitle();
        private: // menu bar
// FIXME: was WIN32 only, needed for Qt?
#if 0
			void OnActivate(wxActivateEvent& event);
			void OnDelayedActivate(wxIdleEvent& event);
#endif
            // FIXME: only used for rebuildMenuBar(). drop?
			//void OnChildFocus(wxChildFocusEvent& event);
            void rebuildMenuBar();
            void createMenuBar();
            void createActions();
            void registerBinding(QAction* action, const ActionInfo& info);
            void updateBindings();
            void createMenus();
            void updateGridActions();
            void updateToolActions();
            void updateOtherActions();
            void updateUndoRedoActions();

            void addRecentDocumentsMenu(QMenuBar* menuBar);
            void removeRecentDocumentsMenu(QMenuBar* menuBar);
            void updateRecentDocumentsMenu();
        private: // tool bar
            void createToolBar();
        private: // status bar
            void createStatusBar();
            void updateStatusBar();
        private: // gui creation
            void createGui();
        private: // notification handlers
            void bindObservers();
            void unbindObservers();

            void documentWasCleared(View::MapDocument* document);
            void documentDidChange(View::MapDocument* document);
            void documentModificationStateDidChange();
            void preferenceDidChange(const IO::Path& path);
            void gridDidChange();
            void toolActivated(Tool* tool);
            void toolDeactivated(Tool* tool);
            void selectionDidChange(const Selection& selection);
            void currentLayerDidChange(const TrenchBroom::Model::Layer* layer);
            void groupWasOpened(Model::Group* group);
            void groupWasClosed(Model::Group* group);
        private: // menu event handlers
            void bindEvents();

            void OnFileSave();
            void OnFileSaveAs();
            void OnFileExportObj();
            void OnFileLoadPointFile();
            void OnFileReloadPointFile();
            void OnFileUnloadPointFile();
            void OnFileLoadPortalFile();
            void OnFileReloadPortalFile();
            void OnFileUnloadPortalFile();
            void OnFileReloadTextureCollections();
            void OnFileReloadEntityDefinitions();
            void OnFileClose();

            void OnEditUndo();
            void OnEditRedo();
            void OnEditRepeat();
            void OnEditClearRepeat();

            void OnEditCut();
            void OnEditCopy();
            void copyToClipboard();

            void OnEditPaste();
            void OnEditPasteAtOriginalPosition();
            
            PasteType paste();

            void OnEditDelete();
            void OnEditDuplicate();

            void OnEditSelectAll();
            void OnEditSelectSiblings();
            void OnEditSelectTouching();
            void OnEditSelectInside();
            void OnEditSelectTall();
            void OnEditSelectByLineNumber();
            void OnEditSelectNone();

            void OnEditGroupSelectedObjects();
            void OnEditUngroupSelectedObjects();

            void OnEditDeactivateTool();
            void OnEditToggleCreateComplexBrushTool();
            void OnEditToggleClipTool();
            void OnEditToggleRotateObjectsTool();
            void OnEditToggleScaleObjectsTool();
            void OnEditToggleShearObjectsTool();
            void OnEditToggleVertexTool();
            void OnEditToggleEdgeTool();
            void OnEditToggleFaceTool();

            void OnEditCsgConvexMerge();
            void OnEditCsgSubtract();
            void OnEditCsgIntersect();
            void OnEditCsgHollow();

            void OnEditReplaceTexture();

            void OnEditToggleTextureLock();
            //wxBitmap textureLockBitmap();
            void OnEditToggleUVLock();
            //wxBitmap UVLockBitmap();

            void OnEditSnapVerticesToInteger();
            void OnEditSnapVerticesToGrid();

            void OnViewToggleShowGrid();
            void OnViewToggleSnapToGrid();
            void OnViewIncGridSize();
            void OnViewDecGridSize();
            void OnViewSetGridSize();

            void OnViewMoveCameraToNextPoint();
            void OnViewMoveCameraToPreviousPoint();
            void OnViewFocusCameraOnSelection();
            void OnViewMoveCameraToPosition();

            void OnViewHideSelectedObjects();
            void OnViewIsolateSelectedObjects();
            void OnViewShowHiddenObjects();
            
            void OnViewSwitchToMapInspector();
            void OnViewSwitchToEntityInspector();
            void OnViewSwitchToFaceInspector();

            // FIXME:
            //void switchToInspectorPage(Inspector::InspectorPage page);
            void ensureInspectorVisible();
            
            void OnViewToggleMaximizeCurrentView();
            void OnViewToggleInfoPanel();
            void OnViewToggleInspector();

            void OnRunCompile();
        public:
            void compilationDialogWillClose();
        private:
            void OnRunLaunch();

            void OnDebugPrintVertices();
            void OnDebugCreateBrush();
            void OnDebugCreateCube();
            void OnDebugClipBrush();
            void OnDebugCopyJSShortcutMap();
            void OnDebugCrash();
            void OnDebugThrowExceptionDuringCommand();
            void OnDebugSetWindowSize();
            
            void OnFlipObjectsHorizontally();
            void OnFlipObjectsVertically();

//            void OnUpdateUI(wxUpdateUIEvent& event);

            void OnToolBarSetGridSize(int index);
            void onFocusChange(QWidget* old, QWidget* now);
        private:
            bool canUnloadPointFile() const;
            bool canReloadPointFile() const;
            bool canUnloadPortalFile() const;
            bool canReloadPortalFile() const;

            bool canUndo() const;
            void undo();
            bool canRedo() const;
            void redo();
            //wxTextCtrl* findFocusedTextCtrl() const;

            bool canCut() const;
            bool canCopy() const;
            bool canPaste() const;
            bool canDelete() const;
            bool canDuplicate() const;
            bool canSelectSiblings() const;
            bool canSelectByBrush() const;
            bool canSelectTall() const;
            bool canSelect() const;
            bool canDeselect() const;
            bool canChangeSelection() const;
            bool canGroup() const;
            bool canUngroup() const;
            bool canHide() const;
            bool canIsolate() const;
            bool canDoCsgConvexMerge() const;
            bool canDoCsgSubtract() const;
            bool canDoCsgIntersect() const;
            bool canDoCsgHollow() const;
            bool canSnapVertices() const;
            bool canDecGridSize() const;
            bool canIncGridSize() const;
            bool canMoveCameraToNextPoint() const;
            bool canMoveCameraToPreviousPoint() const;
            bool canFocusCamera() const;
            bool canCompile() const;
            bool canLaunch() const;
        private: // other event handlers
  //          void OnClose(wxCloseEvent& event);
            void OnAutosaveTimer();
        private: // grid helpers
            static int indexForGridSize(const int gridSize);
            static int gridSizeForIndex(const int index);
        };
    }
}

#endif /* defined(TrenchBroom_MapFrame) */
