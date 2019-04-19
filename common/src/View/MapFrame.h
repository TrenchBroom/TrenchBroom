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
#include "View/Inspector.h"
#include "View/Selection.h"
#include "View/ViewTypes.h"

#include <QMainWindow>

#include <utility>

class QAction;
class QComboBox;
class QDropEvent;
class QMenuBar;
class QLabel;
class QSplitter;
class QTimer;

namespace TrenchBroom {
    class Logger;

    namespace IO {
        class Path;
    }

    namespace View {
        class Action;
        class Autosaver;
        class Console;
        class InfoPanel;
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

            QSplitter* m_hSplitter;
            QSplitter* m_vSplitter;

            GLContextManager* m_contextManager;
            SwitchableMapViewContainer* m_mapView;
            InfoPanel* m_infoPanel;
            Console* m_console;
            Inspector* m_inspector;

            QComboBox* m_gridChoice;
            QLabel* m_statusBarLabel;
        private: // shortcuts
            using ActionMap = std::vector<std::pair<QAction*, const Action*>>;
            ActionMap m_actionMap;
        private: // special menu entries
            QMenu* m_recentDocumentsMenu;
            QAction* m_undoAction;
            QAction* m_redoAction;
            QAction* m_pasteAction;
            QAction* m_pasteAtOriginalPositionAction;
#if 0
            wxDialog* m_compilationDialog;
#endif
        public:
            MapFrame();
            MapFrame(FrameManager* frameManager, MapDocumentSPtr document);
            ~MapFrame() override;

            void positionOnScreen(QWidget* reference);
            MapDocumentSPtr document() const;
        public: // getters and such
            Logger& logger() const;
        public: // drop targets
            void setToolBoxDropTarget();
            void clearDropTarget();
        protected:
            void dragEnterEvent(QDragEnterEvent* event) override;
            void dropEvent(QDropEvent* event) override;
        private: // title bar contents
            void updateTitle();
        private: // menu bar
            class MenuBuilder;
            void createMenus();
            void updateShortcuts();
            void updateActionState();
            void updateUndoRedoActions();
            void updatePasteActions();

            void addRecentDocumentsMenu(QMenuBar* menuBar);
            void removeRecentDocumentsMenu(QMenuBar* menuBar);
            void updateRecentDocumentsMenu();
        private: // tool bar
            class ToolBarBuilder;
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

            void transactionDone(const String&);
            void transactionUndone(const String&);

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
        private slots:
            void triggerAction(const Action& action);
        public:
            bool newDocument(Model::GameSPtr game, Model::MapFormat mapFormat);
            bool openDocument(Model::GameSPtr game, Model::MapFormat mapFormat, const IO::Path& path);
            bool saveDocument();
            bool saveDocumentAs();
            bool exportDocumentAsObj();
            bool exportDocument(Model::ExportFormat format, const IO::Path& path);
        private:
            bool confirmOrDiscardChanges();
        public:
            void loadPointFile();
            void reloadPointFile();
            void unloadPointFile();
            bool canReloadPointFile() const;
            bool canUnloadPortalFile() const;

            void loadPortalFile();
            void reloadPortalFile();
            void unloadPortalFile();
            bool canReloadPortalFile() const;
            bool canUnloadPointFile() const;

            void reloadTextureCollections();
            void reloadEntityDefinitions();
            void closeDocument();

            void undo();
            void redo();
            bool canUndo() const;
            bool canRedo() const;

            void repeatLastCommands();
            void clearRepeatableCommands();
            bool hasRepeatableCommands() const;

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
            void OnEditToggleUVLock();

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

            void switchToInspectorPage(Inspector::InspectorPage page);

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

            void OnToolBarSetGridSize(int index);
            void onFocusChange(QWidget* old, QWidget* now);
        private:

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
