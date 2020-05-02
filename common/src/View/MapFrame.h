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
#include "View/Selection.h"

#include <QMainWindow>
#include <QPointer>
#include <QDialog>

#include <map>
#include <memory>
#include <string>

class QAction;
class QComboBox;
class QDialog;
class QDropEvent;
class QMenuBar;
class QLabel;
class QSplitter;
class QTimer;
class QToolBar;

namespace TrenchBroom {
    class Logger;

    namespace IO {
        class Path;
    }

    namespace Model {
        enum class ExportFormat;
        class Game;
        class Group;
        class Layer;
    }

    namespace View {
        class Action;
        class Autosaver;
        class Console;
        class FrameManager;
        class GLContextManager;
        class InfoPanel;
        class Inspector;
        enum class InspectorPage;
        class MapDocument;
        class MapViewBase;
        enum class PasteType;
        class SwitchableMapViewContainer;
        class Tool;

        class MapFrame : public QMainWindow {
            Q_OBJECT
        private:
            FrameManager* m_frameManager;
            std::shared_ptr<MapDocument> m_document;

            std::unique_ptr<Autosaver> m_autosaver;
            QTimer* m_autosaveTimer;

            QToolBar* m_toolBar;

            QSplitter* m_hSplitter;
            QSplitter* m_vSplitter;

            std::unique_ptr<GLContextManager> m_contextManager;
            SwitchableMapViewContainer* m_mapView;
            /**
             * Last focused MapViewBase. It's a QPointer to handle changing from e.g. a 2-pane map view to 1-pane.
             */
            QPointer<MapViewBase> m_currentMapView;
            InfoPanel* m_infoPanel;
            Console* m_console;
            Inspector* m_inspector;

            QComboBox* m_gridChoice;
            QLabel* m_statusBarLabel;

            QPointer<QDialog> m_compilationDialog;
        private: // shortcuts
            using ActionMap = std::map<const Action*, QAction*>;
            ActionMap m_actionMap;
        private: // special menu entries
            QMenu* m_recentDocumentsMenu;
            QAction* m_undoAction;
            QAction* m_redoAction;
        public:
            MapFrame(FrameManager* frameManager, std::shared_ptr<MapDocument> document);
            ~MapFrame() override;

            void positionOnScreen(QWidget* reference);
            std::shared_ptr<MapDocument> document() const;
        public: // getters and such
            Logger& logger() const;
        private: // title bar contents
            void updateTitle();
        private: // menu bar
            void createMenus();
            void updateShortcuts();
            void updateActionState();
            void updateUndoRedoActions();

            void addRecentDocumentsMenu();
            void removeRecentDocumentsMenu();
            void updateRecentDocumentsMenu();
        private: // tool bar
            class ToolBarBuilder;
            void createToolBar();
            void updateToolBarWidgets();
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

            void transactionDone(const std::string&);
            void transactionUndone(const std::string&);

            void preferenceDidChange(const IO::Path& path);
            void gridDidChange();
            void toolActivated(Tool* tool);
            void toolDeactivated(Tool* tool);
            void toolHandleSelectionChanged(Tool* tool);
            void selectionDidChange(const Selection& selection);
            void currentLayerDidChange(const TrenchBroom::Model::Layer* layer);
            void groupWasOpened(Model::Group* group);
            void groupWasClosed(Model::Group* group);
        private: // menu event handlers
            void bindEvents();
        public:
            bool newDocument(std::shared_ptr<Model::Game> game, Model::MapFormat mapFormat);
            bool openDocument(std::shared_ptr<Model::Game> game, Model::MapFormat mapFormat, const IO::Path& path);
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

            void cutSelection();
            void copySelection();
            void copyToClipboard();
            bool canCutSelection() const;
            bool canCopySelection() const;

            void pasteAtCursorPosition();
            void pasteAtOriginalPosition();
            PasteType paste();
            bool canPaste() const;

            void duplicateSelection();
            bool canDuplicateSelectino() const;

            void deleteSelection();
            bool canDeleteSelection() const;

            void selectAll();
            void selectSiblings();
            void selectTouching();
            void selectInside();
            void selectTall();
            void selectByLineNumber();
            void selectInverse();
            void selectNone();

            bool canSelect() const;
            bool canSelectSiblings() const;
            bool canSelectByBrush() const;
            bool canSelectTall() const;
            bool canDeselect() const;
            bool canChangeSelection() const;
            bool canSelectInverse() const;

            void groupSelectedObjects();
            bool canGroupSelectedObjects() const;

            void ungroupSelectedObjects();
            bool canUngroupSelectedObjects() const;

            void renameSelectedGroups();
            bool canRenameSelectedGroups() const;

            void toggleCreateComplexBrushTool();
            bool canToggleCreateComplexBrushTool() const;
            bool createComplexBrushToolActive() const;

            void toggleClipTool();
            bool canToggleClipTool() const;
            bool clipToolActive() const;

            void toggleRotateObjectsTool();
            bool canToggleRotateObjectsTool() const;
            bool rotateObjectsToolActive() const;

            void toggleScaleObjectsTool();
            bool canToggleScaleObjectsTool() const;
            bool scaleObjectsToolActive() const;

            void toggleShearObjectsTool();
            bool canToggleShearObjectsTool() const;
            bool shearObjectsToolActive() const;

            bool anyVertexToolActive() const;

            void toggleVertexTool();
            bool canToggleVertexTool() const;
            bool vertexToolActive() const;

            void toggleEdgeTool();
            bool canToggleEdgeTool() const;
            bool edgeToolActive() const;

            void toggleFaceTool();
            bool canToggleFaceTool() const;
            bool faceToolActive() const;

            void csgConvexMerge();
            bool canDoCsgConvexMerge() const;

            void csgSubtract();
            bool canDoCsgSubtract() const;

            void csgHollow();
            bool canDoCsgHollow() const;

            void csgIntersect();
            bool canDoCsgIntersect() const;

            void snapVerticesToInteger();
            void snapVerticesToGrid();
            bool canSnapVertices() const;

            void replaceTexture();

            void toggleTextureLock();
            void toggleUVLock();

            void toggleShowGrid();
            void toggleSnapToGrid();

            void incGridSize();
            bool canIncGridSize() const;

            void decGridSize();
            bool canDecGridSize() const;

            void setGridSize(int size);

            void moveCameraToNextPoint();
            bool canMoveCameraToNextPoint() const;

            void moveCameraToPreviousPoint();
            bool canMoveCameraToPreviousPoint() const;

            void focusCameraOnSelection();
            bool canFocusCamera() const;

            void moveCameraToPosition();

            void isolateSelection();
            bool canIsolateSelection() const;

            void hideSelection();
            bool canHideSelection() const;

            void showAll();

            void switchToInspectorPage(InspectorPage page);

            void toggleToolbar();
            bool toolbarVisible() const;

            void toggleInfoPanel();
            bool infoPanelVisible() const;

            void toggleInspector();
            bool inspectorVisible() const;

            void toggleMaximizeCurrentView();
            bool currentViewMaximized();

            void showCompileDialog();
            void compilationDialogWillClose();

            void showLaunchEngineDialog();

            void debugPrintVertices();
            void debugCreateBrush();
            void debugCreateCube();
            void debugClipBrush();
            void debugCrash();
            void debugThrowExceptionDuringCommand();
            void debugSetWindowSize();
            void debugShowPalette();

            void focusChange(QWidget* oldFocus, QWidget* newFocus);

            MapViewBase* currentMapViewBase();
        private:
            bool canCompile() const;
            bool canLaunch() const;
        protected: // other event handlers
            void changeEvent(QEvent* event) override;
            void closeEvent(QCloseEvent* event) override;
        private:
            void triggerAutosave();
        };

        class DebugPaletteWindow : public QDialog {
            Q_OBJECT
        public:
            DebugPaletteWindow(QWidget *parent = nullptr);
            virtual ~DebugPaletteWindow();
        };
    }
}

#endif /* defined(TrenchBroom_MapFrame) */
