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

#include "MapFrame.h"

#include <QTimer>
#include <QLabel>
#include <QString>
#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QMenuBar>
#include <QShortcut>

#include "TrenchBroomApp.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "IO/DiskFileSystem.h"
#include "IO/ResourceUtils.h"
#include "Model/AttributableNode.h"
#include "Model/Brush.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/Node.h"
#include "Model/NodeCollection.h"
#include "Model/PointFile.h"
#include "Model/World.h"
#include "View/ActionManager.h"
#include "View/Autosaver.h"
#include "View/BorderLine.h"
#include "View/CachingLogger.h"
#include "FileLogger.h"
#include "View/ClipTool.h"
#include "View/CommandIds.h"
#include "View/CommandWindowUpdateLocker.h"
#include "View/CompilationDialog.h"
#include "View/Console.h"
#include "View/EdgeTool.h"
#include "View/FaceTool.h"
#include "View/GLContextManager.h"
#include "View/Grid.h"
#include "View/InfoPanel.h"
#include "View/Inspector.h"
#include "View/LaunchGameEngineDialog.h"
#include "View/MapDocument.h"
#include "View/MapFrameDropTarget.h"
#include "View/Menu.h"
#include "View/OpenClipboard.h"
#include "View/RenderView.h"
#include "View/ReplaceTextureDialog.h"
#include "View/SplitterWindow2.h"
#include "View/SwitchableMapViewContainer.h"
#include "View/VertexTool.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"
#include "View/ModifiableMenuItem.h"

#include <vecmath/util.h>

#include <cassert>
#include <iterator>

namespace TrenchBroom {
    namespace View {
        MapFrame::MapFrame() :
        QMainWindow(),
        m_frameManager(nullptr),
        m_autosaver(nullptr),
        m_autosaveTimer(nullptr),
        m_contextManager(nullptr),
        m_mapView(nullptr),
//        m_console(nullptr),
//        m_inspector(nullptr),
//        m_lastFocus(nullptr),
//        m_gridChoice(nullptr),
//        m_compilationDialog(nullptr),
        m_updateLocker(nullptr) {}

        MapFrame::MapFrame(FrameManager* frameManager, MapDocumentSPtr document) :
        QMainWindow(),
        m_frameManager(frameManager),
        m_autosaver(nullptr),
        m_autosaveTimer(nullptr),
        m_contextManager(nullptr),
        m_mapView(nullptr),
//        m_console(nullptr),
//        m_inspector(nullptr),
//        m_lastFocus(nullptr),
//        m_gridChoice(nullptr),
//        m_compilationDialog(nullptr),
        m_updateLocker(nullptr) {
            Create(frameManager, document);
        }

        void MapFrame::Create(FrameManager* frameManager, MapDocumentSPtr document) {
            setAttribute(Qt::WA_DeleteOnClose);

            ensure(frameManager != nullptr, "frameManager is null");
            ensure(document.get() != nullptr, "document is null");

            m_frameManager = frameManager;
            m_document = document;
            m_autosaver = new Autosaver(m_document);

            m_contextManager = new GLContextManager();

            createGui();
            createToolBar();
            createMenuBar();
            createActions();

            updateGridActions();

            createMenus();
            createStatusBar();

            m_document->setParentLogger(logger());
            m_document->setViewEffectsService(m_mapView);

            m_autosaveTimer = new QTimer(this);
            m_autosaveTimer->start(1000);

            bindObservers();
            bindEvents();

            clearDropTarget();
            
            m_updateLocker = new CommandWindowUpdateLocker(this, m_document);
#ifdef __APPLE__
            m_updateLocker->Start();
#endif
        }
        
        static RenderView* FindChildRenderView(wxWindow *current) {
            // FIXME: necessary in Qt?
#if 0
            for (wxWindow *child : current->GetChildren()) {
                RenderView *canvas = wxDynamicCast(child, RenderView);
                if (canvas != nullptr)
                    return canvas;
                
                canvas = FindChildRenderView(child);
                if (canvas != nullptr)
                    return canvas;
            }
#endif
            return nullptr;
        }

        MapFrame::~MapFrame() {
            // FIXME: necessary in Qt?
#if 0
            // Search for a RenderView (wxGLCanvas subclass) and make it current.
            RenderView* canvas = FindChildRenderView(this);
            if (canvas != nullptr && m_contextManager != nullptr) {
                wxGLContext* mainContext = m_contextManager->mainContext();
                if (mainContext != nullptr)
                    mainContext->SetCurrent(*canvas);
            }
#endif

            // The MapDocument's CachingLogger has a pointer to m_console, which
            // is about to be destroyed (DestroyChildren()). Clear the pointer
            // so we don't try to log to a dangling pointer (#1885).
            m_document->setParentLogger(nullptr);

            // Makes IsBeingDeleted() return true
            // FIXME: necessary in Qt?
//            SendDestroyEvent();

            m_mapView->deactivateTool();
            
            unbindObservers();
            // FIXME: necessary in Qt?
//            removeRecentDocumentsMenu(GetMenuBar());

            delete m_updateLocker;
            m_updateLocker = nullptr;

            // FIXME: I think this is deleted by the parent widget in Qt
//            delete m_autosaveTimer;
//            m_autosaveTimer = nullptr;

            delete m_autosaver;
            m_autosaver = nullptr;

            // The order of deletion here is important because both the document and the children
            // need the context manager (and its embedded VBO) to clean up their resources.

            destroy(false, true); // Destroy the children first because they might still access document resources.
            
            m_document->setViewEffectsService(nullptr);
            m_document.reset();

            delete m_contextManager;
            m_contextManager = nullptr;
        }

#if 0
        void MapFrame::positionOnScreen(wxFrame* reference) {
            const wxDisplay display;
            const wxRect displaySize = display.GetClientArea();
            if (reference == nullptr) {
                SetSize(std::min(displaySize.width, 1024), std::min(displaySize.height, 768));
                CenterOnScreen();
            } else {
                wxPoint position = reference->GetPosition();
                position.x += 23;
                position.y += 23;

                if (displaySize.GetBottom() - position.x < 100 ||
                    displaySize.GetRight() - position.y < 70)
                    position = displaySize.GetTopLeft();

                SetPosition(position);
                SetSize(std::min(displaySize.GetRight() - position.x, reference->GetSize().x), std::min(displaySize.GetBottom() - position.y, reference->GetSize().y));
            }
        }
#endif

        MapDocumentSPtr MapFrame::document() const {
            return m_document;
        }

        Logger* MapFrame::logger() const {
            // FIXME: return m_console
            return &FileLogger::instance();
        }

        void MapFrame::setToolBoxDropTarget() {
            // FIXME:
            //SetDropTarget(nullptr);
            m_mapView->setToolBoxDropTarget();
        }

        void MapFrame::clearDropTarget() {
            m_mapView->clearDropTarget();
            // FIXME:
            //SetDropTarget(new MapFrameDropTarget(m_document, this));
        }

        bool MapFrame::newDocument(Model::GameSPtr game, const Model::MapFormat::Type mapFormat) {
            if (!confirmOrDiscardChanges())
                return false;
            m_document->newDocument(mapFormat, MapDocument::DefaultWorldBounds, game);
            return true;
        }

        bool MapFrame::openDocument(Model::GameSPtr game, const Model::MapFormat::Type mapFormat, const IO::Path& path) {
            if (!confirmOrDiscardChanges())
                return false;
            m_document->loadDocument(mapFormat, MapDocument::DefaultWorldBounds, game, path);
            return true;
        }

        bool MapFrame::saveDocument() {
            try {
                if (m_document->persistent()) {
                    m_document->saveDocument();
                    logger()->info("Saved " + m_document->path().asString());
                    return true;
                }
                return saveDocumentAs();
            } catch (const FileSystemException& e) {
                QMessageBox::critical(this, "", e.what(), QMessageBox::Ok);
                return false;
            } catch (...) {
                QMessageBox::critical(this, "", QString::fromStdString("Unknown error while saving " + m_document->path().asString()), QMessageBox::Ok);
                return false;
            }
        }

        bool MapFrame::saveDocumentAs() {
            try {
                const IO::Path& originalPath = m_document->path();
                const IO::Path directory = originalPath.deleteLastComponent();
                const IO::Path fileName = originalPath.lastComponent();

                const QString newFileName = QFileDialog::getSaveFileName(this, "Save map file", QString::fromStdString(originalPath.asString()), "Map files (*.map)");
                if (newFileName.isEmpty())
                    return false;

                const IO::Path path(newFileName.toStdString());
                m_document->saveDocumentAs(path);
                logger()->info("Saved " + m_document->path().asString());
                return true;
            } catch (const FileSystemException& e) {
                QMessageBox::critical(this, "", e.what(), QMessageBox::Ok);
                return false;
            } catch (...) {
                QMessageBox::critical(this, "", QString::fromStdString("Unknown error while saving " + m_document->filename()), QMessageBox::Ok);
                return false;
            }
        }

        bool MapFrame::exportDocumentAsObj() {
            const IO::Path& originalPath = m_document->path();
            const IO::Path objPath = originalPath.replaceExtension("obj");
            wxString wildcard;

            const QString newFileName = QFileDialog::getSaveFileName(this, "Export Wavefront OBJ file", QString::fromStdString(objPath.asString()), "Wavefront OBJ files (*.obj)");
            if (newFileName.isEmpty())
                return false;

            return exportDocument(Model::WavefrontObj, IO::Path(newFileName.toStdString()));
        }

        bool MapFrame::exportDocument(const Model::ExportFormat format, const IO::Path& path) {
            try {
                m_document->exportDocumentAs(format, path);
                logger()->info("Exported " + path.asString());
                return true;
            } catch (const FileSystemException& e) {
                QMessageBox::critical(this, "", e.what(), QMessageBox::Ok);
                return false;
            } catch (...) {
                QMessageBox::critical(this, "", QString::fromStdString("Unknown error while exporting " + path.asString()), QMessageBox::Ok);
                return false;
            }
        }

        bool MapFrame::confirmOrDiscardChanges() {
            if (!m_document->modified())
                return true;
            const QMessageBox::StandardButton result = QMessageBox::question(this, "TrenchBroom", QString::fromStdString(m_document->filename() + " has been modified. Do you want to save the changes?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            switch (result) {
                case QMessageBox::Yes:
                    return saveDocument();
                case QMessageBox::No:
                    return true;
                default:
                    return false;
            }
        }

        void MapFrame::updateTitle() {
            setWindowModified(m_document->modified());
            setWindowTitle(QString::fromStdString(m_document->filename()) + QString("[*] - TrenchBroom"));
            setWindowFilePath(QString::fromStdString(m_document->path().asString()));
        }

#if defined(_WIN32)
		/*
		This and the following method were added to reset the menu bar correctly when the map frame
		regains focus after the preference dialog is closed. Since the map view reports not having focus
		when the activation event is processed, we set up a delayed processing in the next idle event.

		See also issue #1762, which only affects Windows.
		*/
		void MapFrame::OnActivate(wxActivateEvent& event) {
            if (IsBeingDeleted()) return;

			Bind(wxEVT_IDLE, &MapFrame::OnDelayedActivate, this);
			event.Skip();
        }

		void MapFrame::OnDelayedActivate(wxIdleEvent& event) {
			if (IsBeingDeleted()) return;

			Unbind(wxEVT_IDLE, &MapFrame::OnDelayedActivate, this);
			rebuildMenuBar();
			event.Skip();
		}
#endif

// FIXME: only used for rebuildMenuBar(). drop?
#if 0
		void MapFrame::OnChildFocus(wxChildFocusEvent& event) {
            wxWindow* focus = FindFocus();
            if (focus == nullptr)
                focus = event.GetWindow();
            if (focus != m_lastFocus && focus != this) {
                rebuildMenuBar();
                m_lastFocus = focus;
            }

			event.Skip();
        }
#endif

        void MapFrame::rebuildMenuBar() {
//            wxMenuBar* oldMenuBar = GetMenuBar();
//            removeRecentDocumentsMenu(oldMenuBar);
            createMenuBar();
//            oldMenuBar->Destroy();
        }

        void MapFrame::createMenuBar() {
//			const ActionManager& actionManager = ActionManager::instance();
//            QMenuBar* menuBar = actionManager.createMenuBarQt(m_mapView->viewportHasFocus());
//            setMenuBar(menuBar);

            // FIXME: recents
            //addRecentDocumentsMenu(menuBar);
        }

        void MapFrame::createActions() {
		    // File

            fileNewAction = new QAction("New", this);
            fileNewAction->setShortcuts(QKeySequence::New);
            connect(fileNewAction, &QAction::triggered, &TrenchBroomApp::instance(), &TrenchBroomApp::OnFileNew);

            fileOpenAction = new QAction("Open", this);
            fileOpenAction->setShortcuts(QKeySequence::Open);
            connect(fileOpenAction, &QAction::triggered, &TrenchBroomApp::instance(), &TrenchBroomApp::OnFileOpen);

            fileSaveAction = new QAction("Save", this);
            fileSaveAction->setShortcuts(QKeySequence::Save);
            connect(fileSaveAction, &QAction::triggered, this, &MapFrame::OnFileSave);

            fileSaveAsAction = new QAction("Save as...", this);
            fileSaveAsAction->setShortcuts(QKeySequence::SaveAs);
            connect(fileSaveAsAction, &QAction::triggered, this, &MapFrame::OnFileSaveAs);

            fileExportObjAction = new QAction("Wavefront OBJ...", this);
            connect(fileExportObjAction, &QAction::triggered, this, &MapFrame::OnFileExportObj);

            fileLoadPointFileAction = new QAction("Load Point File...", this);
            connect(fileLoadPointFileAction, &QAction::triggered, this, &MapFrame::OnFileLoadPointFile);

            fileReloadPointFileAction = new QAction("Reload Point File", this);
            connect(fileReloadPointFileAction, &QAction::triggered, this, &MapFrame::OnFileReloadPointFile);

            fileUnloadPointFileAction = new QAction("Unload Point File", this);
            connect(fileUnloadPointFileAction, &QAction::triggered, this, &MapFrame::OnFileUnloadPointFile);

            fileLoadPortalFileAction = new QAction("Load Portal File...", this);
            connect(fileLoadPortalFileAction, &QAction::triggered, this, &MapFrame::OnFileLoadPortalFile);

            fileReloadPortalFileAction = new QAction("Reload Portal File", this);
            connect(fileReloadPortalFileAction, &QAction::triggered, this, &MapFrame::OnFileReloadPortalFile);

            fileUnloadPortalFileAction = new QAction("Unload Portal File", this);
            connect(fileUnloadPortalFileAction, &QAction::triggered, this, &MapFrame::OnFileUnloadPortalFile);

            fileReloadTextureCollectionsAction = new QAction("Reload Texture Collections", this);
            connect(fileReloadTextureCollectionsAction, &QAction::triggered, this, &MapFrame::OnFileReloadTextureCollections);

            fileReloadEntityDefinitionsAction = new QAction("Reload Entity Definitions", this);
            connect(fileReloadEntityDefinitionsAction, &QAction::triggered, this, &MapFrame::OnFileReloadEntityDefinitions);

            fileCloseAction = new QAction("Close", this);
            fileCloseAction->setShortcuts(QKeySequence::Close);
            connect(fileCloseAction, &QAction::triggered, this, &MapFrame::OnFileClose);

            // Edit

            editUndoAction = new QAction("Undo", this);
            editUndoAction->setShortcuts(QKeySequence::Undo);
            connect(editUndoAction, &QAction::triggered, this, &MapFrame::OnEditUndo); //, this, wxID_UNDO);

            editRedoAction = new QAction("Redo", this);
            editRedoAction->setShortcuts(QKeySequence::Redo);
            connect(editRedoAction, &QAction::triggered, this, &MapFrame::OnEditRedo); //, this, wxID_REDO);

            editRepeatAction = new QAction("Repeat", this);
            connect(editRepeatAction, &QAction::triggered, this, &MapFrame::OnEditRepeat); //, this, CommandIds::Menu::EditRepeat);

            editClearRepeatAction = new QAction("Clear Repeatable Commands", this);
            connect(editClearRepeatAction, &QAction::triggered, this, &MapFrame::OnEditClearRepeat); //, this, CommandIds::Menu::EditClearRepeat);


            editCutAction = new QAction("Cut", this);
            editCutAction->setShortcuts(QKeySequence::Cut);
            connect(editCutAction, &QAction::triggered, this, &MapFrame::OnEditCut); //, this, wxID_CUT);

            editCopyAction = new QAction("Copy", this);
            editCopyAction->setShortcuts(QKeySequence::Copy);
            connect(editCopyAction, &QAction::triggered, this, &MapFrame::OnEditCopy); //, this, wxID_COPY);

            editPasteAction = new QAction("Paste", this);
            editPasteAction->setShortcuts(QKeySequence::Paste);
            connect(editPasteAction, &QAction::triggered, this, &MapFrame::OnEditPaste); //, this, wxID_PASTE);

            editPasteAtOriginalPositionAction = new QAction("Paste at Original Position", this);
            connect(editPasteAtOriginalPositionAction, &QAction::triggered, this, &MapFrame::OnEditPasteAtOriginalPosition); //, this, CommandIds::Menu::EditPasteAtOriginalPosition);

            editDuplicateAction = new QAction("Duplicate", this);
            connect(editDuplicateAction, &QAction::triggered, this, &MapFrame::OnEditDuplicate); //, this, wxID_DUPLICATE);

            editDeleteAction = new QAction("Delete", this);
            connect(editDeleteAction, &QAction::triggered, this, &MapFrame::OnEditDelete); //, this, wxID_DELETE);


            editSelectAllAction = new QAction("Select All", this);
            connect(editSelectAllAction, &QAction::triggered, this, &MapFrame::OnEditSelectAll); //, this, CommandIds::Menu::EditSelectAll);

            editSelectSiblingsAction = new QAction("Select Siblings", this);
            connect(editSelectSiblingsAction, &QAction::triggered, this, &MapFrame::OnEditSelectSiblings); //, this, CommandIds::Menu::EditSelectSiblings);

            editSelectTouchingAction = new QAction("Select Touching", this);
            connect(editSelectTouchingAction, &QAction::triggered, this, &MapFrame::OnEditSelectTouching); //, this, CommandIds::Menu::EditSelectTouching);

            editSelectInsideAction = new QAction("Select Inside", this);
            connect(editSelectInsideAction, &QAction::triggered, this, &MapFrame::OnEditSelectInside); //, this, CommandIds::Menu::EditSelectInside);

            editSelectTallAction = new QAction("Select Tall", this);
            connect(editSelectTallAction, &QAction::triggered, this, &MapFrame::OnEditSelectTall); //, this, CommandIds::Menu::EditSelectTall);

            editSelectByLineNumberAction = new QAction("Select by Line Number", this);
            connect(editSelectByLineNumberAction, &QAction::triggered, this, &MapFrame::OnEditSelectByLineNumber); //, this, CommandIds::Menu::EditSelectByFilePosition);

            editSelectNoneAction = new QAction("Select None", this);
            connect(editSelectNoneAction, &QAction::triggered, this, &MapFrame::OnEditSelectNone); //, this, CommandIds::Menu::EditSelectNone);


            editGroupSelectedObjectsAction = new QAction("Group", this);
            connect(editGroupSelectedObjectsAction, &QAction::triggered, this, &MapFrame::OnEditGroupSelectedObjects); //, this, CommandIds::Menu::EditGroupSelection);

            editUngroupSelectedObjectsAction = new QAction("Ungroup", this);
            connect(editUngroupSelectedObjectsAction, &QAction::triggered, this, &MapFrame::OnEditUngroupSelectedObjects); //, this, CommandIds::Menu::EditUngroupSelection);


            editDeactivateToolAction = new QAction("Deactivate Tool", this);
            connect(editDeactivateToolAction, &QAction::triggered, this, &MapFrame::OnEditDeactivateTool); //, this, CommandIds::Menu::EditDeactivateTool);

            editToggleCreateComplexBrushToolAction = new QAction("Brush Tool", this);
            editToggleCreateComplexBrushToolAction->setCheckable(true);
            editToggleCreateComplexBrushToolAction->setData(QVariant::fromValue(ModifiableMenuItem(IO::Path("Brush Tool"), "B")));
            connect(editToggleCreateComplexBrushToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleCreateComplexBrushTool); //, this, CommandIds::Menu::EditToggleCreateComplexBrushTool);

            editToggleClipToolAction = new QAction("Clip Tool", this);
            editToggleClipToolAction->setCheckable(true);
            editToggleClipToolAction->setData(QVariant::fromValue(ModifiableMenuItem(IO::Path("Clip Tool"), "C")));
            connect(editToggleClipToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleClipTool); //, this, CommandIds::Menu::EditToggleClipTool);

            editToggleRotateObjectsToolAction = new QAction("Rotate Tool", this);
            editToggleRotateObjectsToolAction->setCheckable(true);
            editToggleRotateObjectsToolAction->setData(QVariant::fromValue(ModifiableMenuItem(IO::Path("Rotate Tool"), "R")));
            connect(editToggleRotateObjectsToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleRotateObjectsTool); //, this, CommandIds::Menu::EditToggleRotateObjectsTool);

            editToggleScaleObjectsToolAction = new QAction("Scale Tool", this);
            editToggleScaleObjectsToolAction->setCheckable(true);
            editToggleScaleObjectsToolAction->setData(QVariant::fromValue(ModifiableMenuItem(IO::Path("Scale Tool"), "T")));
            connect(editToggleScaleObjectsToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleScaleObjectsTool); //, this, CommandIds::Menu::EditToggleScaleObjectsTool);

            editToggleShearObjectsToolAction = new QAction("Shear Tool", this);
            editToggleShearObjectsToolAction->setCheckable(true);
            editToggleShearObjectsToolAction->setData(QVariant::fromValue(ModifiableMenuItem(IO::Path("Shear Tool"), "G")));
            connect(editToggleShearObjectsToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleShearObjectsTool); //, this, CommandIds::Menu::EditToggleShearObjectsTool);

            editToggleVertexToolAction = new QAction("Vertex Tool", this);
            editToggleVertexToolAction->setCheckable(true);
            editToggleVertexToolAction->setData(QVariant::fromValue(ModifiableMenuItem(IO::Path("Vertex Tool"), "V")));
            connect(editToggleVertexToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleVertexTool); //, this, CommandIds::Menu::EditToggleVertexTool);

            editToggleEdgeToolAction = new QAction("Edge Tool", this);
            editToggleEdgeToolAction->setCheckable(true);
            editToggleEdgeToolAction->setData(QVariant::fromValue(ModifiableMenuItem(IO::Path("Edge Tool"), "E")));
            connect(editToggleEdgeToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleEdgeTool); //, this, CommandIds::Menu::EditToggleEdgeTool);

            editToggleFaceToolAction = new QAction("Face Tool", this);
            editToggleFaceToolAction->setCheckable(true);
            editToggleFaceToolAction->setData(QVariant::fromValue(ModifiableMenuItem(IO::Path("Face Tool"), "F")));
            connect(editToggleFaceToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleFaceTool); //, this, CommandIds::Menu::EditToggleFaceTool);


            editCsgConvexMergeAction = new QAction("Convex Merge", this);
            connect(editCsgConvexMergeAction, &QAction::triggered, this, &MapFrame::OnEditCsgConvexMerge); //, this, CommandIds::Menu::EditCsgConvexMerge);

            editCsgSubtractAction = new QAction("Subtract", this);
            connect(editCsgSubtractAction, &QAction::triggered, this, &MapFrame::OnEditCsgSubtract); //, this, CommandIds::Menu::EditCsgSubtract);

            editCsgIntersectAction = new QAction("Intersect", this);
            connect(editCsgIntersectAction, &QAction::triggered, this, &MapFrame::OnEditCsgIntersect); //, this, CommandIds::Menu::EditCsgIntersect);

            editCsgHollowAction = new QAction("Hollow", this);
            connect(editCsgHollowAction, &QAction::triggered, this, &MapFrame::OnEditCsgHollow); //, this, CommandIds::Menu::EditCsgHollow);


            editReplaceTextureAction = new QAction("Replace Texture...", this);
            connect(editReplaceTextureAction, &QAction::triggered, this, &MapFrame::OnEditReplaceTexture); //, this, CommandIds::Menu::EditReplaceTexture);

            editToggleTextureLockAction = new QAction("Texture Lock", this);
            connect(editToggleTextureLockAction, &QAction::triggered, this, &MapFrame::OnEditToggleTextureLock); //, this, CommandIds::Menu::EditToggleTextureLock);

            editToggleUVLockAction = new QAction("UV Lock", this);
            connect(editToggleUVLockAction, &QAction::triggered, this, &MapFrame::OnEditToggleUVLock); //, this, CommandIds::Menu::EditToggleUVLock);

            editSnapVerticesToIntegerAction = new QAction("Snap Vertices to Integer", this);
            connect(editSnapVerticesToIntegerAction, &QAction::triggered, this, &MapFrame::OnEditSnapVerticesToInteger); //, this, CommandIds::Menu::EditSnapVerticesToInteger);

            editSnapVerticesToGridAction = new QAction("Snap Vertices to Grid", this);
            connect(editSnapVerticesToGridAction, &QAction::triggered, this, &MapFrame::OnEditSnapVerticesToGrid); //, this, CommandIds::Menu::EditSnapVerticesToGrid);

            // View

            viewToggleShowGridAction = new QAction("Show Grid", this);
            viewToggleShowGridAction->setCheckable(true);
            connect(viewToggleShowGridAction, &QAction::triggered, this, &MapFrame::OnViewToggleShowGrid);

            viewToggleSnapToGridAction = new QAction("Snap to Grid", this);
            viewToggleSnapToGridAction->setCheckable(true);
            connect(viewToggleSnapToGridAction, &QAction::triggered, this, &MapFrame::OnViewToggleSnapToGrid);

            viewIncGridSizeAction = new QAction("Increase Grid Size", this);
            viewIncGridSizeAction->setShortcut(QKeySequence("+")); // FIXME:
            connect(viewIncGridSizeAction, &QAction::triggered, this, &MapFrame::OnViewIncGridSize);

            viewDecGridSizeAction = new QAction("Decrease Grid Size", this);
            viewDecGridSizeAction->setShortcut(QKeySequence("-")); // FIXME:
            connect(viewDecGridSizeAction, &QAction::triggered, this, &MapFrame::OnViewDecGridSize);

            viewSetGridSizeActionGroup = new QActionGroup(this);

            viewSetGridSize0Point125Action = new QAction("Set Grid Size 0.125", viewSetGridSizeActionGroup);
            viewSetGridSize0Point125Action->setData(QVariant(-3));
            viewSetGridSize0Point125Action->setCheckable(true);
            connect(viewSetGridSize0Point125Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize0Point25Action = new QAction("Set Grid Size 0.25", viewSetGridSizeActionGroup);
            viewSetGridSize0Point25Action->setData(QVariant(-2));
            viewSetGridSize0Point25Action->setCheckable(true);
            connect(viewSetGridSize0Point25Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize0Point5Action = new QAction("Set Grid Size 0.5", viewSetGridSizeActionGroup);
            viewSetGridSize0Point5Action->setData(QVariant(-1));
            viewSetGridSize0Point5Action->setCheckable(true);
            connect(viewSetGridSize0Point5Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize1Action = new QAction("Set Grid Size 1", viewSetGridSizeActionGroup);
            viewSetGridSize1Action->setData(QVariant(0));
            viewSetGridSize1Action->setCheckable(true);
            connect(viewSetGridSize1Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize2Action = new QAction("Set Grid Size 2", viewSetGridSizeActionGroup);
            viewSetGridSize2Action->setData(QVariant(1));
            viewSetGridSize2Action->setCheckable(true);
            connect(viewSetGridSize2Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize4Action = new QAction("Set Grid Size 4", viewSetGridSizeActionGroup);
            viewSetGridSize4Action->setData(QVariant(2));
            viewSetGridSize4Action->setCheckable(true);
            connect(viewSetGridSize4Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize8Action = new QAction("Set Grid Size 8", viewSetGridSizeActionGroup);
            viewSetGridSize8Action->setData(QVariant(3));
            viewSetGridSize8Action->setCheckable(true);
            connect(viewSetGridSize8Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize16Action = new QAction("Set Grid Size 16", viewSetGridSizeActionGroup);
            viewSetGridSize16Action->setData(QVariant(4));
            viewSetGridSize16Action->setCheckable(true);
            connect(viewSetGridSize16Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize32Action = new QAction("Set Grid Size 32", viewSetGridSizeActionGroup);
            viewSetGridSize32Action->setData(QVariant(5));
            viewSetGridSize32Action->setCheckable(true);
            connect(viewSetGridSize32Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize64Action = new QAction("Set Grid Size 64", viewSetGridSizeActionGroup);
            viewSetGridSize64Action->setData(QVariant(6));
            viewSetGridSize64Action->setCheckable(true);
            connect(viewSetGridSize64Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize128Action = new QAction("Set Grid Size 128", viewSetGridSizeActionGroup);
            viewSetGridSize128Action->setData(QVariant(7));
            viewSetGridSize128Action->setCheckable(true);
            connect(viewSetGridSize128Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize256Action = new QAction("Set Grid Size 256", viewSetGridSizeActionGroup);
            viewSetGridSize256Action->setData(QVariant(8));
            viewSetGridSize256Action->setCheckable(true);
            connect(viewSetGridSize256Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewMoveCameraToNextPointAction = new QAction("Move to Next Point", this);
            connect(viewMoveCameraToNextPointAction, &QAction::triggered, this, &MapFrame::OnViewMoveCameraToNextPoint);

            viewMoveCameraToPreviousPointAction = new QAction("Move to Previous Point", this);
            connect(viewMoveCameraToPreviousPointAction, &QAction::triggered, this, &MapFrame::OnViewMoveCameraToPreviousPoint);

            viewFocusCameraOnSelectionAction = new QAction("Focus on Selection", this);
            connect(viewFocusCameraOnSelectionAction, &QAction::triggered, this, &MapFrame::OnViewFocusCameraOnSelection);

            viewMoveCameraToPositionAction = new QAction("Move Camera to...", this);
            connect(viewMoveCameraToPositionAction, &QAction::triggered, this, &MapFrame::OnViewMoveCameraToPosition);

            viewHideSelectionAction = new QAction("Hide", this);
            connect(viewHideSelectionAction, &QAction::triggered, this, &MapFrame::OnViewHideSelectedObjects);

            viewIsolateSelectionAction = new QAction("Isolate", this);
            connect(viewIsolateSelectionAction, &QAction::triggered, this, &MapFrame::OnViewIsolateSelectedObjects);

            viewUnhideAllAction = new QAction("Show All", this);
            connect(viewUnhideAllAction, &QAction::triggered, this, &MapFrame::OnViewShowHiddenObjects);

            viewSwitchToMapInspectorAction = new QAction("Switch to Map Inspector", this);
            connect(viewSwitchToMapInspectorAction, &QAction::triggered, this, &MapFrame::OnViewSwitchToMapInspector);

            viewSwitchToEntityInspectorAction = new QAction("Switch to Entity Inspector", this);
            connect(viewSwitchToEntityInspectorAction, &QAction::triggered, this, &MapFrame::OnViewSwitchToEntityInspector);

            viewSwitchToFaceInspectorAction = new QAction("Switch to Face Inspector", this);
            connect(viewSwitchToFaceInspectorAction, &QAction::triggered, this, &MapFrame::OnViewSwitchToFaceInspector);

            viewToggleMaximizeCurrentViewAction = new QAction("Maximize Current View", this);
            connect(viewToggleMaximizeCurrentViewAction, &QAction::triggered, this, &MapFrame::OnViewToggleMaximizeCurrentView);

            viewPreferencesAction = new QAction("Preferences...", this);
            connect(viewPreferencesAction, &QAction::triggered, &TrenchBroomApp::instance(), &TrenchBroomApp::OnOpenPreferences);

            viewToggleInfoPanelAction = new QAction("Toggle Info Panel", this);
            viewToggleInfoPanelAction->setCheckable(true);
            connect(viewToggleInfoPanelAction, &QAction::triggered, this, &MapFrame::OnViewToggleInfoPanel);

            viewToggleInspectorAction = new QAction("Toggle Inspector", this);
            viewToggleInspectorAction->setCheckable(true);
            connect(viewToggleInspectorAction, &QAction::triggered, this, &MapFrame::OnViewToggleInspector);

            runCompileAction = new QAction("Compile...", this);
            connect(runCompileAction, &QAction::triggered, this, &MapFrame::OnRunCompile);

            runLaunchAction = new QAction("Launch...", this);
            connect(runLaunchAction, &QAction::triggered, this, &MapFrame::OnRunLaunch);

            debugPrintVerticesAction = new QAction("Print Vertices", this);
            connect(debugPrintVerticesAction, &QAction::triggered, this, &MapFrame::OnDebugPrintVertices);

            debugCreateBrushAction = new QAction("Create Brush...", this);
            connect(debugCreateBrushAction, &QAction::triggered, this, &MapFrame::OnDebugCreateBrush);

            debugCreateCubeAction = new QAction("Create Cube...", this);
            connect(debugCreateCubeAction, &QAction::triggered, this, &MapFrame::OnDebugCreateCube);

            debugClipWithFaceAction = new QAction("Clip Brush...", this);
            connect(debugClipWithFaceAction, &QAction::triggered, this, &MapFrame::OnDebugClipBrush);

            debugCopyJSShortcutsAction = new QAction("Copy Javascript Shortcut Map", this);
            connect(debugCopyJSShortcutsAction, &QAction::triggered, this, &MapFrame::OnDebugCopyJSShortcutMap);

            debugCrashAction = new QAction("Crash...", this);
            connect(debugCrashAction, &QAction::triggered, this, &MapFrame::OnDebugCrash);

            debugThrowExceptionDuringCommandAction = new QAction("Throw Exception During Command", this);
            connect(debugThrowExceptionDuringCommandAction, &QAction::triggered, this, &MapFrame::OnDebugThrowExceptionDuringCommand);

            debugCrashReportDialogAction = new QAction("Show Crash Report Dialog", this);
            connect(viewPreferencesAction, &QAction::triggered, &TrenchBroomApp::instance(), &TrenchBroomApp::OnDebugShowCrashReportDialog);

            debugSetWindowSizeAction = new QAction("Set Window Size...", this);
            connect(debugSetWindowSizeAction, &QAction::triggered, this, &MapFrame::OnDebugSetWindowSize);

            helpManualAction = new QAction("TrenchBroom Manual", this);
            connect(helpManualAction, &QAction::triggered, &TrenchBroomApp::instance(), &TrenchBroomApp::OnHelpShowManual);

            helpAboutAction = new QAction("About TrenchBroom", this);
            connect(helpAboutAction, &QAction::triggered, &TrenchBroomApp::instance(), &TrenchBroomApp::OnOpenAbout);

            // set up bindings
            for (QObject* child : children()) {
                if (QAction* action = dynamic_cast<QAction*>(child); action != nullptr) {
                    qDebug("found action %s", action->text().toStdString().c_str());
                    if (action->data().canConvert<ModifiableMenuItem>()) {
                        const auto menuInfo = action->data().value<ModifiableMenuItem>();
                        qDebug("found path %s, binding: %s",
                                menuInfo.configPath().asString().c_str(),
                               menuInfo.defaultKey().toStdString().c_str());

                        action->setShortcut(QKeySequence(menuInfo.defaultKey()));
                    }
                }
            }
        }


        void MapFrame::createMenus() {
            QMenu* fileMenu = menuBar()->addMenu("File");
            fileMenu->addAction(fileNewAction);// addUnmodifiableActionItem(wxID_NEW, "New", KeyboardShortcut('N', WXK_CONTROL));
            fileMenu->addSeparator();
            fileMenu->addAction(fileOpenAction);// UnmodifiableActionItem(wxID_OPEN, "Open...", KeyboardShortcut('O', WXK_CONTROL));
            QMenu* openRecentMenu = fileMenu->addMenu("Open Recent");
            fileMenu->addSeparator();
            fileMenu->addAction(fileSaveAction);// addUnmodifiableActionItem(wxID_SAVE, "Save", KeyboardShortcut('S', WXK_CONTROL));
            fileMenu->addAction(fileSaveAsAction); // addUnmodifiableActionItem(wxID_SAVEAS, "Save as...", KeyboardShortcut('S', WXK_SHIFT, WXK_CONTROL));

            QMenu* exportMenu = fileMenu->addMenu("Export");
            exportMenu->addAction(fileExportObjAction); // addModifiableActionItem(CommandIds::Menu::FileExportObj, "Wavefront OBJ...");

            fileMenu->addSeparator();
            fileMenu->addAction(fileLoadPointFileAction); //addModifiableActionItem(CommandIds::Menu::FileLoadPointFile, );
            fileMenu->addAction(fileReloadPointFileAction); //addModifiableActionItem(CommandIds::Menu::FileReloadPointFile, "Reload Point File");
            fileMenu->addAction(fileUnloadPointFileAction); //addModifiableActionItem(CommandIds::Menu::FileUnloadPointFile, "Unload Point File");
            fileMenu->addSeparator();
            fileMenu->addAction(fileLoadPortalFileAction); //addModifiableActionItem(CommandIds::Menu::FileLoadPortalFile, "Load Portal File...");
            fileMenu->addAction(fileReloadPortalFileAction); //addModifiableActionItem(CommandIds::Menu::FileReloadPortalFile, "Reload Portal File");
            fileMenu->addAction(fileUnloadPortalFileAction); //addModifiableActionItem(CommandIds::Menu::FileUnloadPortalFile, "Unload Portal File");
            fileMenu->addSeparator();
            fileMenu->addAction(fileReloadTextureCollectionsAction); //addModifiableActionItem(CommandIds::Menu::FileReloadTextureCollections, "Reload Texture Collections", KeyboardShortcut(WXK_F5));
            fileMenu->addAction(fileReloadEntityDefinitionsAction); //addModifiableActionItem(CommandIds::Menu::FileReloadEntityDefinitions, "Reload Entity Definitions", KeyboardShortcut(WXK_F6));
            fileMenu->addSeparator();
            fileMenu->addAction(fileCloseAction);// UnmodifiableActionItem(wxID_CLOSE, "Close", KeyboardShortcut('W', WXK_CONTROL));

            QMenu *editMenu = menuBar()->addMenu("Edit");
            editMenu->addAction(editUndoAction); //addUnmodifiableActionItem(wxID_UNDO, "Undo", KeyboardShortcut('Z', WXK_CONTROL));
            editMenu->addAction(editRedoAction); //addUnmodifiableActionItem(wxID_REDO, "Redo", KeyboardShortcut('Z', WXK_CONTROL, WXK_SHIFT));
            editMenu->addSeparator();
            editMenu->addAction(editRepeatAction); //addModifiableActionItem(CommandIds::Menu::EditRepeat, "Repeat", KeyboardShortcut('R', WXK_CONTROL));
            editMenu->addAction(editClearRepeatAction); //addModifiableActionItem(CommandIds::Menu::EditClearRepeat, "Clear Repeatable Commands", KeyboardShortcut('R', WXK_CONTROL, WXK_SHIFT));
            editMenu->addSeparator();
            editMenu->addAction(editCutAction); //addUnmodifiableActionItem(wxID_CUT, "Cut", KeyboardShortcut('X', WXK_CONTROL));
            editMenu->addAction(editCopyAction); //addUnmodifiableActionItem(wxID_COPY, "Copy", KeyboardShortcut('C', WXK_CONTROL));
            editMenu->addAction(editPasteAction); //addUnmodifiableActionItem(wxID_PASTE, "Paste", KeyboardShortcut('V', WXK_CONTROL));
            editMenu->addAction(editPasteAtOriginalPositionAction); //addModifiableActionItem(CommandIds::Menu::EditPasteAtOriginalPosition, "Paste at Original Position", KeyboardShortcut('V', WXK_CONTROL, WXK_ALT));
            editMenu->addAction(editDuplicateAction); //addModifiableActionItem(wxID_DUPLICATE, "Duplicate", KeyboardShortcut('D', WXK_CONTROL));
            editMenu->addAction(editDeleteAction); //addModifiableActionItem(wxID_DELETE, "Delete", KeyboardShortcut(WXK_DELETE));

            editMenu->addSeparator();
            editMenu->addAction(editSelectAllAction); //addModifiableActionItem(CommandIds::Menu::EditSelectAll, "Select All", KeyboardShortcut('A', WXK_CONTROL));
            editMenu->addAction(editSelectSiblingsAction); //addModifiableActionItem(CommandIds::Menu::EditSelectSiblings, "Select Siblings", KeyboardShortcut('B', WXK_CONTROL));
            editMenu->addAction(editSelectTouchingAction); //addModifiableActionItem(CommandIds::Menu::EditSelectTouching, "Select Touching", KeyboardShortcut('T', WXK_CONTROL));
            editMenu->addAction(editSelectInsideAction); //addModifiableActionItem(CommandIds::Menu::EditSelectInside, "Select Inside", KeyboardShortcut('E', WXK_CONTROL));
            editMenu->addAction(editSelectTallAction); //addModifiableActionItem(CommandIds::Menu::EditSelectTall, "Select Tall", KeyboardShortcut('E', WXK_CONTROL, WXK_SHIFT));
            editMenu->addAction(editSelectByLineNumberAction); //addModifiableActionItem(CommandIds::Menu::EditSelectByFilePosition, "Select by Line Number");
            editMenu->addAction(editSelectNoneAction); //addModifiableActionItem(CommandIds::Menu::EditSelectNone, "Select None", KeyboardShortcut('A', WXK_CONTROL, WXK_SHIFT));
            editMenu->addSeparator();

            editMenu->addAction(editGroupSelectedObjectsAction); //addModifiableActionItem(CommandIds::Menu::EditGroupSelection, "Group", KeyboardShortcut('G', WXK_CONTROL));
            editMenu->addAction(editUngroupSelectedObjectsAction); //addModifiableActionItem(CommandIds::Menu::EditUngroupSelection, "Ungroup", KeyboardShortcut('G', WXK_CONTROL, WXK_SHIFT));
            editMenu->addSeparator();

            QMenu* toolMenu = editMenu->addMenu("Tools");
            toolMenu->addAction(editToggleCreateComplexBrushToolAction); //addModifiableCheckItem(CommandIds::Menu::EditToggleCreateComplexBrushTool, "Brush Tool", KeyboardShortcut('B'));
            toolMenu->addAction(editToggleClipToolAction); //addModifiableCheckItem(CommandIds::Menu::EditToggleClipTool, "Clip Tool", KeyboardShortcut('C'));
            toolMenu->addAction(editToggleRotateObjectsToolAction); //addModifiableCheckItem(CommandIds::Menu::EditToggleRotateObjectsTool, "Rotate Tool", KeyboardShortcut('R'));
            toolMenu->addAction(editToggleScaleObjectsToolAction); //addModifiableCheckItem(CommandIds::Menu::EditToggleScaleObjectsTool, "Scale Tool", KeyboardShortcut('T'));
            toolMenu->addAction(editToggleShearObjectsToolAction); //addModifiableCheckItem(CommandIds::Menu::EditToggleShearObjectsTool, "Shear Tool", KeyboardShortcut('G'));
            toolMenu->addAction(editToggleVertexToolAction); //addModifiableCheckItem(CommandIds::Menu::EditToggleVertexTool, "Vertex Tool", KeyboardShortcut('V'));
            toolMenu->addAction(editToggleEdgeToolAction); //addModifiableCheckItem(CommandIds::Menu::EditToggleEdgeTool, "Edge Tool", KeyboardShortcut('E'));
            toolMenu->addAction(editToggleFaceToolAction); //addModifiableCheckItem(CommandIds::Menu::EditToggleFaceTool, "Face Tool", KeyboardShortcut('F'));

            QMenu* csgMenu = editMenu->addMenu("CSG");
            csgMenu->addAction(editCsgConvexMergeAction); // addModifiableActionItem(CommandIds::Menu::EditCsgConvexMerge, "Convex Merge", KeyboardShortcut('J', WXK_CONTROL));
            csgMenu->addAction(editCsgSubtractAction); // addModifiableActionItem(CommandIds::Menu::EditCsgSubtract, "Subtract", KeyboardShortcut('K', WXK_CONTROL));
            csgMenu->addAction(editCsgHollowAction); // addModifiableActionItem(CommandIds::Menu::EditCsgHollow, "Hollow", KeyboardShortcut('K', WXK_CONTROL, WXK_ALT));
            csgMenu->addAction(editCsgIntersectAction); // addModifiableActionItem(CommandIds::Menu::EditCsgIntersect, "Intersect", KeyboardShortcut('L', WXK_CONTROL));

            editMenu->addSeparator();
            editMenu->addAction(editSnapVerticesToIntegerAction); //addModifiableActionItem(CommandIds::Menu::EditSnapVerticesToInteger, "Snap Vertices to Integer", KeyboardShortcut('V', WXK_SHIFT, WXK_CONTROL));
            editMenu->addAction(editSnapVerticesToGridAction); //addModifiableActionItem(CommandIds::Menu::EditSnapVerticesToGrid, "Snap Vertices to Grid", KeyboardShortcut('V', WXK_SHIFT, WXK_CONTROL, WXK_ALT));
            editMenu->addSeparator();
            editMenu->addAction(editToggleTextureLockAction); //addModifiableCheckItem(CommandIds::Menu::EditToggleTextureLock, "Texture Lock");
            editMenu->addAction(editToggleUVLockAction); //addModifiableCheckItem(CommandIds::Menu::EditToggleUVLock, "UV Lock", KeyboardShortcut('U'));
            editMenu->addAction(editReplaceTextureAction); //addModifiableActionItem(CommandIds::Menu::EditReplaceTexture, "Replace Texture...");

            QMenu* viewMenu = menuBar()->addMenu("View");
            QMenu* gridMenu = viewMenu->addMenu("Grid");
            gridMenu->addAction(viewToggleShowGridAction); //, "Show Grid", KeyboardShortcut('0'));
            gridMenu->addAction(viewToggleSnapToGridAction); //, "Snap to Grid", KeyboardShortcut('0', WXK_ALT));
            gridMenu->addAction(viewIncGridSizeAction); //, "Increase Grid Size", KeyboardShortcut('+'));
            gridMenu->addAction(viewDecGridSizeAction); //, "Decrease Grid Size", KeyboardShortcut('-'));
            gridMenu->addSeparator();
            gridMenu->addAction(viewSetGridSize0Point125Action); //, "Set Grid Size 0.125");
            gridMenu->addAction(viewSetGridSize0Point25Action); //, "Set Grid Size 0.25");
            gridMenu->addAction(viewSetGridSize0Point5Action); //, "Set Grid Size 0.5");
            gridMenu->addAction(viewSetGridSize1Action); //, "Set Grid Size 1", KeyboardShortcut('1'));
            gridMenu->addAction(viewSetGridSize2Action); //, "Set Grid Size 2", KeyboardShortcut('2'));
            gridMenu->addAction(viewSetGridSize4Action); //, "Set Grid Size 4", KeyboardShortcut('3'));
            gridMenu->addAction(viewSetGridSize8Action); //, "Set Grid Size 8", KeyboardShortcut('4'));
            gridMenu->addAction(viewSetGridSize16Action); //, "Set Grid Size 16", KeyboardShortcut('5'));
            gridMenu->addAction(viewSetGridSize32Action); //, "Set Grid Size 32", KeyboardShortcut('6'));
            gridMenu->addAction(viewSetGridSize64Action); //, "Set Grid Size 64", KeyboardShortcut('7'));
            gridMenu->addAction(viewSetGridSize128Action); //, "Set Grid Size 128", KeyboardShortcut('8'));
            gridMenu->addAction(viewSetGridSize256Action); //, "Set Grid Size 256", KeyboardShortcut('9'));

            QMenu* cameraMenu = viewMenu->addMenu("Camera");
            cameraMenu->addAction(viewMoveCameraToNextPointAction); //addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToNextPoint, "Move to Next Point", KeyboardShortcut('.'));
            cameraMenu->addAction(viewMoveCameraToPreviousPointAction); //addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToPreviousPoint, "Move to Previous Point", KeyboardShortcut(','));
            cameraMenu->addAction(viewFocusCameraOnSelectionAction); //addModifiableActionItem(CommandIds::Menu::ViewFocusCameraOnSelection, "Focus on Selection", KeyboardShortcut('U', WXK_CONTROL));
            cameraMenu->addAction(viewMoveCameraToPositionAction); //addModifiableActionItem(CommandIds::Menu::ViewMoveCameraToPosition, "Move Camera to...");
            cameraMenu->addSeparator();

            viewMenu->addSeparator();
            viewMenu->addAction(viewIsolateSelectionAction); //addModifiableActionItem(CommandIds::Menu::ViewIsolateSelection, "Isolate", KeyboardShortcut('I', WXK_CONTROL));
            viewMenu->addAction(viewHideSelectionAction); //addModifiableActionItem(CommandIds::Menu::ViewHideSelection, "Hide", KeyboardShortcut('I', WXK_CONTROL, WXK_ALT));
            viewMenu->addAction(viewUnhideAllAction); //addModifiableActionItem(CommandIds::Menu::ViewUnhideAll, "Show All", KeyboardShortcut('I', WXK_CONTROL, WXK_SHIFT));

            viewMenu->addSeparator();
            viewMenu->addAction(viewSwitchToMapInspectorAction); //, "Switch to Map Inspector", KeyboardShortcut('1', WXK_CONTROL));
            viewMenu->addAction(viewSwitchToEntityInspectorAction); //, "Switch to Entity Inspector", KeyboardShortcut('2', WXK_CONTROL));
            viewMenu->addAction(viewSwitchToFaceInspectorAction); //, "Switch to Face Inspector", KeyboardShortcut('3', WXK_CONTROL));
            viewMenu->addSeparator();
            viewMenu->addAction(viewToggleInfoPanelAction); //, "Toggle Info Panel", KeyboardShortcut('4', WXK_CONTROL));
            viewMenu->addAction(viewToggleInspectorAction); //, "Toggle Inspector", KeyboardShortcut('5', WXK_CONTROL));
            viewMenu->addSeparator();
            viewMenu->addAction(viewToggleMaximizeCurrentViewAction); //, "Maximize Current View", KeyboardShortcut(WXK_SPACE, WXK_CONTROL));
            viewMenu->addSeparator();
            viewMenu->addAction(viewPreferencesAction); // wxID_PREFERENCES, "Preferences...");

            QMenu* runMenu = menuBar()->addMenu("Run");
            runMenu->addAction(runCompileAction); //CommandIds::Menu::RunCompile, "Compile...");
            runMenu->addAction(runLaunchAction); //CommandIds::Menu::RunLaunch, "Launch...");

#ifndef NDEBUG
            QMenu* debugMenu = menuBar()->addMenu("Debug");
            debugMenu->addAction(debugPrintVerticesAction); //(CommandIds::Menu::DebugPrintVertices, "Print Vertices");
            debugMenu->addAction(debugCreateBrushAction); //(CommandIds::Menu::DebugCreateBrush, "Create Brush...");
            debugMenu->addAction(debugCreateCubeAction); //(CommandIds::Menu::DebugCreateCube, "Create Cube...");
            debugMenu->addAction(debugClipWithFaceAction); //(CommandIds::Menu::DebugClipWithFace, "Clip Brush...");
            debugMenu->addAction(debugCopyJSShortcutsAction); //(CommandIds::Menu::DebugCopyJSShortcuts, "Copy Javascript Shortcut Map");
            debugMenu->addAction(debugCrashAction); //(CommandIds::Menu::DebugCrash, "Crash...");
            debugMenu->addAction(debugThrowExceptionDuringCommandAction); //(CommandIds::Menu::DebugThrowExceptionDuringCommand, "Throw Exception During Command");
            debugMenu->addAction(debugCrashReportDialogAction); //(CommandIds::Menu::DebugCrashReportDialog, "Show Crash Report Dialog");
            debugMenu->addAction(debugSetWindowSizeAction); //(CommandIds::Menu::DebugSetWindowSize, "Set Window Size...");
#endif

            QMenu* helpMenu = menuBar()->addMenu("Help");
            helpMenu->addAction(helpManualAction); //(wxID_HELP, "TrenchBroom Manual");
            helpMenu->addSeparator();
            helpMenu->addAction(helpAboutAction); //(wxID_ABOUT, "About TrenchBroom");
        }

        void MapFrame::updateGridActions() {
            viewToggleShowGridAction->setChecked(m_document->grid().visible());
            viewToggleSnapToGridAction->setChecked(m_document->grid().snap());

            QAction* actions[] = {
                viewSetGridSize0Point125Action,
                viewSetGridSize0Point25Action,
                viewSetGridSize0Point5Action,
                viewSetGridSize1Action,
                viewSetGridSize2Action,
                viewSetGridSize4Action,
                viewSetGridSize8Action,
                viewSetGridSize16Action,
                viewSetGridSize32Action,
                viewSetGridSize64Action,
                viewSetGridSize128Action,
                viewSetGridSize256Action
            };
            constexpr int numActions = static_cast<int>(sizeof(actions) / sizeof(actions[0]));
            const int gridSizeIndex = m_document->grid().size() - Grid::MinSize;

            for (int i = 0; i < numActions; ++i) {
                actions[i]->setChecked(i == gridSizeIndex);
            }

            viewIncGridSizeAction->setEnabled(canIncGridSize());
            viewDecGridSizeAction->setEnabled(canDecGridSize());
        }

#if 0
        void MapFrame::addRecentDocumentsMenu(wxMenuBar* menuBar) {
            const ActionManager& actionManager = ActionManager::instance();
            wxMenu* recentDocumentsMenu = actionManager.findRecentDocumentsMenu(menuBar);
            ensure(recentDocumentsMenu != nullptr, "recentDocumentsMenu is null");

            TrenchBroomApp& app = TrenchBroomApp::instance();
            app.addRecentDocumentMenu(recentDocumentsMenu);
        }

        void MapFrame::removeRecentDocumentsMenu(wxMenuBar* menuBar) {
            const ActionManager& actionManager = ActionManager::instance();
            wxMenu* recentDocumentsMenu = actionManager.findRecentDocumentsMenu(menuBar);
            ensure(recentDocumentsMenu != nullptr, "recentDocumentsMenu is null");

            TrenchBroomApp& app = TrenchBroomApp::instance();
            app.removeRecentDocumentMenu(recentDocumentsMenu);
        }
#endif

        void MapFrame::updateRecentDocumentsMenu() {
		    // FIXME: recents
#if 0
            if (m_document->path().isAbsolute())
                View::TrenchBroomApp::instance().updateRecentDocument(m_document->path());
#endif
        }

        void MapFrame::createGui() {
            TrenchBroom::View::setWindowIcon(this);
            setWindowTitle("TrenchBroom");

#if 0
            m_hSplitter = new SplitterWindow2(this);
            m_hSplitter->setSashGravity(1.0);
            m_hSplitter->SetName("MapFrameHSplitter");

            m_vSplitter = new SplitterWindow2(m_hSplitter);
            m_vSplitter->setSashGravity(1.0);
            m_vSplitter->SetName("MapFrameVSplitter");

            InfoPanel* infoPanel = new InfoPanel(m_vSplitter, m_document);
            m_console = infoPanel->console();
#endif
            m_mapView = new SwitchableMapViewContainer(nullptr, /* was m_console */ logger(), m_document, *m_contextManager);

#if 0
            m_inspector = new Inspector(m_hSplitter, m_document, *m_contextManager);

            m_mapView->connectTopWidgets(m_inspector);

            m_vSplitter->splitHorizontally(m_mapView, infoPanel, wxSize(100, 100), wxSize(100, 100));
            m_hSplitter->splitVertically(m_vSplitter, m_inspector, wxSize(100, 100), wxSize(350, 100));

            wxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
#if !defined __APPLE__
            frameSizer->Add(new BorderLine(this), 1, wxEXPAND);
#endif
            frameSizer->Add(m_hSplitter, 1, wxEXPAND);

            SetSizer(frameSizer);

            wxPersistenceManager::Get().RegisterAndRestore(m_hSplitter);
            wxPersistenceManager::Get().RegisterAndRestore(m_vSplitter);
#endif

            setCentralWidget(m_mapView);
        }

        void MapFrame::createToolBar() {
		    // FIXME: implement
#if 0
            wxToolBar* toolBar = CreateToolBar(wxTB_DEFAULT_STYLE | 
#if !defined _WIN32
				wxTB_NODIVIDER | 
#endif
				wxTB_FLAT);
            toolBar->SetMargins(2, 2);
            toolBar->AddRadioTool(CommandIds::Menu::EditDeactivateTool, "Default Tool", IO::loadImageResource("NoTool.png"), wxNullBitmap, "Disable Current Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleCreateComplexBrushTool, "Brush Tool", IO::loadImageResource("BrushTool.png"), wxNullBitmap, "Brush Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleClipTool, "Clip Tool", IO::loadImageResource("ClipTool.png"), wxNullBitmap, "Clip Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleVertexTool, "Vertex Tool", IO::loadImageResource("VertexTool.png"), wxNullBitmap, "Vertex Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleEdgeTool, "Edge Tool", IO::loadImageResource("EdgeTool.png"), wxNullBitmap, "Edge Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleFaceTool, "Face Tool", IO::loadImageResource("FaceTool.png"), wxNullBitmap, "Face Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleRotateObjectsTool, "Rotate Tool", IO::loadImageResource("RotateTool.png"), wxNullBitmap, "Rotate Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleScaleObjectsTool, "Scale Tool", IO::loadImageResource("ScaleTool.png"), wxNullBitmap, "Scale Tool");
            toolBar->AddRadioTool(CommandIds::Menu::EditToggleShearObjectsTool, "Shear Tool", IO::loadImageResource("ShearTool.png"), wxNullBitmap, "Shear Tool");
            toolBar->AddSeparator();
            toolBar->AddTool(wxID_DUPLICATE, "Duplicate Objects", IO::loadImageResource("DuplicateObjects.png"), wxNullBitmap, wxITEM_NORMAL, "Duplicate Objects");
            toolBar->AddTool(CommandIds::Actions::FlipObjectsHorizontally, "Flip Horizontally", IO::loadImageResource("FlipHorizontally.png"), wxNullBitmap, wxITEM_NORMAL, "Flip Horizontally");
            toolBar->AddTool(CommandIds::Actions::FlipObjectsVertically, "Flip Vertically", IO::loadImageResource("FlipVertically.png"), wxNullBitmap, wxITEM_NORMAL, "Flip Vertically");
            toolBar->AddSeparator();
            toolBar->AddCheckTool(CommandIds::Menu::EditToggleTextureLock, "Texture Lock", textureLockBitmap(), wxNullBitmap, "Toggle Texture Lock");
            toolBar->AddCheckTool(CommandIds::Menu::EditToggleUVLock, "UV Lock", UVLockBitmap(), wxNullBitmap, "Toggle UV Lock");
            toolBar->AddSeparator();

            const wxString gridSizes[12] = { "Grid 0.125", "Grid 0.25", "Grid 0.5", "Grid 1", "Grid 2", "Grid 4", "Grid 8", "Grid 16", "Grid 32", "Grid 64", "Grid 128", "Grid 256" };
            m_gridChoice = new wxChoice(toolBar, wxID_ANY, wxDefaultPosition, wxDefaultSize, 12, gridSizes);
            m_gridChoice->SetSelection(indexForGridSize(m_document->grid().size()));
            toolBar->AddControl(m_gridChoice);
            
            toolBar->Realize();
#endif
        }

        void MapFrame::createStatusBar() {
            m_statusBarLabel = new QLabel();
            statusBar()->addWidget(m_statusBarLabel);
        }
        
        static Model::AttributableNode* commonEntityForBrushList(const Model::BrushList& list) {
            if (list.empty())
                return nullptr;
            
            Model::AttributableNode* firstEntity = list.front()->entity();
            bool multipleEntities = false;
            
            for (const Model::Brush* brush : list) {
                if (brush->entity() != firstEntity) {
                    multipleEntities = true;
                }
            }

            if (multipleEntities) {
                return nullptr;
            } else {
                return firstEntity;
            }
        }
        
        static String commonClassnameForEntityList(const Model::EntityList& list) {
            if (list.empty())
                return "";
            
            const String firstClassname = list.front()->classname();
            bool multipleClassnames = false;
            
            for (const Model::Entity* entity : list) {
                if (entity->classname() != firstClassname) {
                    multipleClassnames = true;
                }
            }
            
            if (multipleClassnames) {
                return "";
            } else {
                return firstClassname;
            }
        }
        
        static String numberWithSuffix(size_t count, const String &singular, const String &plural) {
            return std::to_string(count) + " " + StringUtils::safePlural(count, singular, plural);
        }
        
        static wxString describeSelection(const MapDocument* document) {
            const wxString DblArrow = wxString(" ") + wxString(wxUniChar(0x00BB)) + wxString(" ");
            const wxString Arrow = wxString(" ") + wxString(wxUniChar(0x203A)) + wxString(" ");
            
            wxString result;
            
            // current layer
            result << document->currentLayer()->name() << DblArrow;
            
            // open groups
            std::list<Model::Group*> groups;
            for (Model::Group* group = document->currentGroup(); group != nullptr; group = group->group()) {
                groups.push_front(group);
            }
            for (Model::Group* group : groups) {
                result << group->name() << Arrow;
            }
            
            // build a vector of strings describing the things that are selected
            StringList tokens;
            
            const auto &selectedNodes = document->selectedNodes();
            
            // selected brushes
            if (!selectedNodes.brushes().empty()) {
                Model::AttributableNode *commonEntity = commonEntityForBrushList(selectedNodes.brushes());
                
                // if all selected brushes are from the same entity, print the entity name
                String token = numberWithSuffix(selectedNodes.brushes().size(), "brush", "brushes");
                if (commonEntity) {
                    token += " (" + commonEntity->classname() + ")";
                } else {
                    token += " (multiple entities)";
                }
                tokens.push_back(token);
            }

            // selected brush faces
            if (document->hasSelectedBrushFaces()) {
                const auto token = numberWithSuffix(document->selectedBrushFaces().size(), "face", "faces");
                tokens.push_back(token);
            }

            // entities
            if (!selectedNodes.entities().empty()) {
                String commonClassname = commonClassnameForEntityList(selectedNodes.entities());
                
                String token = numberWithSuffix(selectedNodes.entities().size(), "entity", "entities");
                if (commonClassname != "") {
                    token += " (" + commonClassname + ")";
                } else {
                    token += " (multiple classnames)";
                }
                tokens.push_back(token);
            }
            
            // groups
            if (!selectedNodes.groups().empty()) {
                tokens.push_back(numberWithSuffix(selectedNodes.groups().size(), "group", "groups"));
            }
            
            // layers
            if (!selectedNodes.layers().empty()) {
                tokens.push_back(numberWithSuffix(selectedNodes.layers().size(), "layer", "layers"));
            }
            
            if (tokens.empty()) {
                tokens.push_back("nothing");
            }
            
            // now, turn `tokens` into a comma-separated string
            result << StringUtils::join(tokens, ", ", ", and ", " and ") << " selected";
            
            return result;
        }
        
        void MapFrame::updateStatusBar() {
            m_statusBarLabel->setText(QString(describeSelection(m_document.get())));
        }
        
        void MapFrame::bindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &MapFrame::preferenceDidChange);

            m_document->documentWasClearedNotifier.addObserver(this, &MapFrame::documentWasCleared);
            m_document->documentWasNewedNotifier.addObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasLoadedNotifier.addObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasSavedNotifier.addObserver(this, &MapFrame::documentDidChange);
            m_document->documentModificationStateDidChangeNotifier.addObserver(this, &MapFrame::documentModificationStateDidChange);
            m_document->selectionDidChangeNotifier.addObserver(this, &MapFrame::selectionDidChange);
            m_document->currentLayerDidChangeNotifier.addObserver(this, &MapFrame::currentLayerDidChange);
            m_document->groupWasOpenedNotifier.addObserver(this, &MapFrame::groupWasOpened);
            m_document->groupWasClosedNotifier.addObserver(this, &MapFrame::groupWasClosed);
            
            Grid& grid = m_document->grid();
            grid.gridDidChangeNotifier.addObserver(this, &MapFrame::gridDidChange);
        }

        void MapFrame::unbindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            assertResult(prefs.preferenceDidChangeNotifier.removeObserver(this, &MapFrame::preferenceDidChange));

            m_document->documentWasClearedNotifier.removeObserver(this, &MapFrame::documentWasCleared);
            m_document->documentWasNewedNotifier.removeObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasLoadedNotifier.removeObserver(this, &MapFrame::documentDidChange);
            m_document->documentWasSavedNotifier.removeObserver(this, &MapFrame::documentDidChange);
            m_document->documentModificationStateDidChangeNotifier.removeObserver(this, &MapFrame::documentModificationStateDidChange);
            m_document->selectionDidChangeNotifier.removeObserver(this, &MapFrame::selectionDidChange);
            m_document->currentLayerDidChangeNotifier.removeObserver(this, &MapFrame::currentLayerDidChange);
            m_document->groupWasOpenedNotifier.removeObserver(this, &MapFrame::groupWasOpened);
            m_document->groupWasClosedNotifier.removeObserver(this, &MapFrame::groupWasClosed);
            
            Grid& grid = m_document->grid();
            grid.gridDidChangeNotifier.removeObserver(this, &MapFrame::gridDidChange);
        }

        void MapFrame::documentWasCleared(View::MapDocument* document) {
            updateTitle();
        }

        void MapFrame::documentDidChange(View::MapDocument* document) {
            updateTitle();
            updateRecentDocumentsMenu();
        }

        void MapFrame::documentModificationStateDidChange() {
            updateTitle();
        }

        void MapFrame::preferenceDidChange(const IO::Path& path) {
            const ActionManager& actionManager = ActionManager::instance();
            if (actionManager.isMenuShortcutPreference(path)) {
                rebuildMenuBar();
            } else if (path == Preferences::MapViewLayout.path())
                m_mapView->switchToMapView(static_cast<MapViewLayout>(pref(Preferences::MapViewLayout)));
        }

        void MapFrame::gridDidChange() {
            const Grid& grid = m_document->grid();
            // FIXME: toolbar
//            m_gridChoice->SetSelection(indexForGridSize(grid.size()));

            updateGridActions();
        }
        
        void MapFrame::selectionDidChange(const Selection& selection) {
            updateStatusBar();
        }
        
        void MapFrame::currentLayerDidChange(const TrenchBroom::Model::Layer* layer) {
            updateStatusBar();
        }
        
        void MapFrame::groupWasOpened(Model::Group* group) {
            updateStatusBar();
        }
        
        void MapFrame::groupWasClosed(Model::Group* group) {
            updateStatusBar();
        }

        void MapFrame::bindEvents() {

            // FIXME:
#if 0

            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_SAVE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_SAVEAS);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_CLOSE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_UNDO);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_REDO);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_CUT);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_COPY);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_PASTE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, wxID_DUPLICATE);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, CommandIds::Menu::Lowest, CommandIds::Menu::Highest);

            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, CommandIds::Actions::FlipObjectsHorizontally);
            Bind(wxEVT_UPDATE_UI, &MapFrame::OnUpdateUI, this, CommandIds::Actions::FlipObjectsVertically);

            Bind(wxEVT_CLOSE_WINDOW, &MapFrame::OnClose, this);
            connect(m_autosaveTimer, &QTimer::timeout, this, &MapFrame::OnAutosaveTimer);
			Bind(wxEVT_CHILD_FOCUS, &MapFrame::OnChildFocus, this);

#if defined(_WIN32)
            Bind(wxEVT_ACTIVATE, &MapFrame::OnActivate, this);
#endif

            m_gridChoice->Bind(wxEVT_CHOICE, &MapFrame::OnToolBarSetGridSize, this);
#endif
        }

        void MapFrame::OnFileSave() {
            saveDocument();
        }

        void MapFrame::OnFileSaveAs() {
            saveDocumentAs();
        }

        void MapFrame::OnFileExportObj() {
            exportDocumentAsObj();
        }

        void MapFrame::OnFileLoadPointFile() {
            QString defaultDir;
            if (!m_document->path().isEmpty())
                defaultDir = QString::fromStdString(m_document->path().deleteLastComponent().asString());

            const QString fileName = QFileDialog::getOpenFileName(this, "Load Point File", defaultDir, "Point files (*.pts);;Any files (*.*)");

            if (!fileName.isEmpty())
                m_document->loadPointFile(IO::Path(fileName.toStdString()));
        }

        void MapFrame::OnFileReloadPointFile() {
            if (canReloadPointFile()) {
                m_document->reloadPointFile();
            }
        }

        void MapFrame::OnFileUnloadPointFile() {
            if (canUnloadPointFile())
                m_document->unloadPointFile();
        }
        
        void MapFrame::OnFileLoadPortalFile() {
            QString defaultDir;
            if (!m_document->path().isEmpty()) {
                defaultDir = QString::fromStdString(m_document->path().deleteLastComponent().asString());
            }

            const QString fileName = QFileDialog::getOpenFileName(this, "Load Portal File", defaultDir, "Portal files (*.prt);;Any files (*.*)");

            if (!fileName.isEmpty()) {
                m_document->loadPortalFile(IO::Path(fileName.toStdString()));
            }
        }

        void MapFrame::OnFileReloadPortalFile() {
            if (canReloadPortalFile()) {
                m_document->reloadPortalFile();
            }
        }

        void MapFrame::OnFileUnloadPortalFile() {
            if (canUnloadPortalFile()) {
                m_document->unloadPortalFile();
            }
        }

        void MapFrame::OnFileReloadTextureCollections() {
            m_document->reloadTextureCollections();
        }

        void MapFrame::OnFileReloadEntityDefinitions() {
            m_document->reloadEntityDefinitions();
        }

        void MapFrame::OnFileClose() {
            close();
        }

        void MapFrame::OnEditUndo() {
            if (canUndo()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                undo();
            }
        }

        void MapFrame::OnEditRedo() {
            if (canRedo()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                redo();
            }
        }

        void MapFrame::OnEditRepeat() {
            m_document->repeatLastCommands();
        }

        void MapFrame::OnEditClearRepeat() {
            m_document->clearRepeatableCommands();
        }

        void MapFrame::OnEditCut() {
            if (canCut()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                copyToClipboard();
                Transaction transaction(m_document, "Cut");
                m_document->deleteObjects();
            }
        }

        void MapFrame::OnEditCopy() {
            if (canCopy()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                copyToClipboard();
            }
        }

        void MapFrame::copyToClipboard() {
            QClipboard *clipboard = QApplication::clipboard();

            String str;
            if (m_document->hasSelectedNodes())
                str = m_document->serializeSelectedNodes();
            else if (m_document->hasSelectedBrushFaces())
                str = m_document->serializeSelectedBrushFaces();

            clipboard->setText(QString::fromStdString(str));
        }

        void MapFrame::OnEditPaste() {
            if (canPaste()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                const vm::bbox3 referenceBounds = m_document->referenceBounds();
                Transaction transaction(m_document);
                if (paste() == PT_Node && m_document->hasSelectedNodes()) {
                    const vm::bbox3 bounds = m_document->selectionBounds();
                    const vm::vec3 delta = m_mapView->pasteObjectsDelta(bounds, referenceBounds);
                    m_document->translateObjects(delta);
                }
            }
        }

        void MapFrame::OnEditPasteAtOriginalPosition() {
            if (canPaste()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                paste();
            }
        }

        PasteType MapFrame::paste() {
            QClipboard *clipboard = QApplication::clipboard();
            const QString qtext = clipboard->text();

            if (qtext.isEmpty()) {
                logger()->error("Clipboard is empty");
                return PT_Failed;
            }

            const String text = qtext.toStdString();
            return m_document->paste(text);
        }

        void MapFrame::OnEditDelete() {
            if (canDelete()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                if (m_mapView->clipToolActive())
                    m_mapView->clipTool()->removeLastPoint();
                else if (m_mapView->vertexToolActive())
                    m_mapView->vertexTool()->removeSelection();
                else if (m_mapView->edgeToolActive())
                    m_mapView->edgeTool()->removeSelection();
                else if (m_mapView->faceToolActive())
                    m_mapView->faceTool()->removeSelection();
                else if (!m_mapView->anyToolActive())
                    m_document->deleteObjects();
            }
        }

        void MapFrame::OnEditDuplicate() {
            if (canDuplicate()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->duplicateObjects();
            }
        }

        void MapFrame::OnEditSelectAll() {
            if (canSelect()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->selectAllNodes();
            }
        }

        void MapFrame::OnEditSelectSiblings() {
            if (canSelectSiblings()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->selectSiblings();
            }
        }

        void MapFrame::OnEditSelectTouching() {
            if (canSelectByBrush()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->selectTouching(true);
            }
        }

        void MapFrame::OnEditSelectInside() {
            if (canSelectByBrush()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->selectInside(true);
            }
        }

        void MapFrame::OnEditSelectTall() {
            if (canSelectTall()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->selectTall();
            }
        }

        void MapFrame::OnEditSelectByLineNumber() {
            if (canSelect()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                const auto qstring = QInputDialog::getText(this, "Select by Line Numbers", "Enter a comma- or space separated list of line numbers.");
                const auto string = qstring.toStdString();
                if (string.empty())
                    return;

                std::vector<size_t> positions;
                wxStringTokenizer tokenizer(string, ", ");
                while (tokenizer.HasMoreTokens()) {
                    const wxString token = tokenizer.NextToken();
                    long position;
                    if (token.ToLong(&position) && position > 0) {
                        positions.push_back(static_cast<size_t>(position));
                    }
                }

                m_document->selectNodesWithFilePosition(positions);
            }
        }

        void MapFrame::OnEditSelectNone() {
            if (canDeselect()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->deselectAll();
            }
        }

        void MapFrame::OnEditGroupSelectedObjects() {
            if (canGroup()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                const String name = queryGroupName(this);
                if (!name.empty())
                    m_document->groupSelection(name);
            }
        }

        void MapFrame::OnEditUngroupSelectedObjects() {
            if (canUngroup()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->ungroupSelection();
            }
        }

        void MapFrame::OnEditReplaceTexture() {
		    // FIXME:
#if 0
            ReplaceTextureDialog dialog(this, m_document, *m_contextManager);
            dialog.CenterOnParent();
            dialog.ShowModal();
#endif
        }

        void MapFrame::OnEditDeactivateTool() {
            m_mapView->deactivateTool();
        }

        void MapFrame::OnEditToggleCreateComplexBrushTool() {
            if (m_mapView->canToggleCreateComplexBrushTool()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleCreateComplexBrushTool();
            }
        }

        void MapFrame::OnEditToggleClipTool() {
            if (m_mapView->canToggleClipTool()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleClipTool();
            }
        }

        void MapFrame::OnEditToggleRotateObjectsTool() {
            if (m_mapView->canToggleRotateObjectsTool()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleRotateObjectsTool();
            }
        }

        void MapFrame::OnEditToggleScaleObjectsTool() {
            if (m_mapView->canToggleScaleObjectsTool()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleScaleObjectsTool();
            }
        }

        void MapFrame::OnEditToggleShearObjectsTool() {
            if (m_mapView->canToggleShearObjectsTool()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleShearObjectsTool();
            }
        }
        
        void MapFrame::OnEditToggleVertexTool() {
            if (m_mapView->canToggleVertexTools()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleVertexTool();
            }
        }
        
        void MapFrame::OnEditToggleEdgeTool() {
            if (m_mapView->canToggleVertexTools()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleEdgeTool();
            }
        }
        
        void MapFrame::OnEditToggleFaceTool() {
            if (m_mapView->canToggleVertexTools()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->toggleFaceTool();
            }
        }

        void MapFrame::OnEditCsgConvexMerge() {
            if (canDoCsgConvexMerge()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                if (m_mapView->vertexToolActive()) {
                    m_mapView->vertexTool()->csgConvexMerge();
                } else if (m_mapView->edgeToolActive()) {
                    m_mapView->edgeTool()->csgConvexMerge();
                } else if (m_mapView->faceToolActive()) {
                    m_mapView->faceTool()->csgConvexMerge();
                } else {
                    m_document->csgConvexMerge();
                }
            }
        }

        void MapFrame::OnEditCsgSubtract() {
            if (canDoCsgSubtract()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->csgSubtract();
            }
        }

        void MapFrame::OnEditCsgIntersect() {
            if (canDoCsgIntersect()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->csgIntersect();
            }
        }

        void MapFrame::OnEditCsgHollow() {
            if (canDoCsgHollow()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->csgHollow();
            }
        }

        void MapFrame::OnEditToggleTextureLock() {
            PreferenceManager::instance().set(Preferences::TextureLock, !pref(Preferences::TextureLock));
            PreferenceManager::instance().saveChanges();
            
            //FIXME: Toolbar
//             GetToolBar()->SetToolNormalBitmap(CommandIds::Menu::EditToggleTextureLock, textureLockBitmap());
        }
//FIXME: Toolbar
#if 0
        wxBitmap MapFrame::textureLockBitmap() {
            if (pref(Preferences::TextureLock)) {
                return IO::loadImageResource("TextureLockOn.png");
            } else {
                return IO::loadImageResource("TextureLockOff.png");
            }
        }
#endif

        void MapFrame::OnEditToggleUVLock() {
            PreferenceManager::instance().set(Preferences::UVLock, !pref(Preferences::UVLock));
            PreferenceManager::instance().saveChanges();

            //FIXME: Toolbar
//            GetToolBar()->SetToolNormalBitmap(CommandIds::Menu::EditToggleUVLock, UVLockBitmap());
        }

        //FIXME: Toolbar
#if 0
        wxBitmap MapFrame::UVLockBitmap() {
            if (pref(Preferences::UVLock)) {
                return IO::loadImageResource("UVLockOn.png");
            } else {
                return IO::loadImageResource("UVLockOff.png");
            }
        }
#endif

        void MapFrame::OnEditSnapVerticesToInteger() {
            if (canSnapVertices()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->snapVertices(1u);
            }
        }
        
        void MapFrame::OnEditSnapVerticesToGrid() {
            if (canSnapVertices()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->snapVertices(m_document->grid().actualSize());
            }
        }

        void MapFrame::OnViewToggleShowGrid() {
            m_document->grid().toggleVisible();
        }

        void MapFrame::OnViewToggleSnapToGrid() {
            m_document->grid().toggleSnap();
        }

        void MapFrame::OnViewIncGridSize() {
            if (canIncGridSize()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->grid().incSize();
            }
        }

        void MapFrame::OnViewDecGridSize() {
            if (canDecGridSize()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->grid().decSize();
            }
        }

        void MapFrame::OnViewSetGridSize() {
		    QAction* caller = dynamic_cast<QAction*>(sender());
            m_document->grid().setSize(caller->data().toInt());
        }

        void MapFrame::OnViewMoveCameraToNextPoint() {
            if (canMoveCameraToNextPoint()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->moveCameraToNextTracePoint();
            }
        }

        void MapFrame::OnViewMoveCameraToPreviousPoint() {
            if (canMoveCameraToPreviousPoint()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->moveCameraToPreviousTracePoint();
            }
        }

        void MapFrame::OnViewFocusCameraOnSelection() {
            if (canFocusCamera()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->focusCameraOnSelection(true);
            }
        }

        void MapFrame::OnViewMoveCameraToPosition() {
            bool ok = false;
            const QString str = QInputDialog::getText(this, "Move Camera", "Enter a position (x y z) for the camera.", QLineEdit::Normal, "0.0 0.0 0.0", &ok);
            if (ok) {
                const vm::vec3 position = vm::vec3::parse(str.toStdString());
                m_mapView->moveCameraToPosition(position, true);
            }
        }
        
        void MapFrame::OnViewHideSelectedObjects() {
            if (canHide()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->hideSelection();
            }
        }
        
        void MapFrame::OnViewIsolateSelectedObjects() {
            if (canIsolate()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_document->isolate(m_document->selectedNodes().nodes());
            }
        }
        
        void MapFrame::OnViewShowHiddenObjects() {
            m_document->showAll();
        }

        void MapFrame::OnViewSwitchToMapInspector() {
            // FIXME:
//            switchToInspectorPage(Inspector::InspectorPage_Map);
        }

        void MapFrame::OnViewSwitchToEntityInspector() {
            // FIXME:
//            switchToInspectorPage(Inspector::InspectorPage_Entity);
        }

        void MapFrame::OnViewSwitchToFaceInspector() {
            // FIXME:
//            switchToInspectorPage(Inspector::InspectorPage_Face);
        }

        // FIXME:
#if 0
        void MapFrame::switchToInspectorPage(const Inspector::InspectorPage page) {
            ensureInspectorVisible();
            m_inspector->switchToPage(page);
        }
#endif
        void MapFrame::ensureInspectorVisible() {
		    // FIXME:
//            if (m_hSplitter->isMaximized(m_vSplitter))
//                m_hSplitter->restore();
        }

        void MapFrame::OnViewToggleMaximizeCurrentView() {
            m_mapView->toggleMaximizeCurrentView();
        }

        void MapFrame::OnViewToggleInfoPanel() {
            // FIXME:
//            if (m_vSplitter->isMaximized(m_mapView))
//                m_vSplitter->restore();
//            else
//                m_vSplitter->maximize(m_mapView);
        }

        void MapFrame::OnViewToggleInspector() {
            // FIXME:
//            if (m_hSplitter->isMaximized(m_vSplitter))
//                m_hSplitter->restore();
//            else
//                m_hSplitter->maximize(m_vSplitter);
        }

        void MapFrame::OnRunCompile() {
            // FIXME:
//            if (m_compilationDialog == nullptr) {
//                m_compilationDialog = new CompilationDialog(this);
//                m_compilationDialog->Show();
//            } else {
//                m_compilationDialog->Raise();
//            }
        }

        void MapFrame::compilationDialogWillClose() {
            // FIXME:
//            m_compilationDialog = nullptr;
        }

        void MapFrame::OnRunLaunch() {
            // FIXME:
//            LaunchGameEngineDialog dialog(this, m_document);
//            dialog.ShowModal();
        }
        
        void MapFrame::OnDebugPrintVertices() {
            m_document->printVertices();
        }

        void MapFrame::OnDebugCreateBrush() {
            bool ok = false;
            const QString str = QInputDialog::getText(this, "Create Brush", "Enter a list of at least 4 points (x y z) (x y z) ...", QLineEdit::Normal, "", &ok);
            if (ok) {
                std::vector<vm::vec3> positions;
                vm::vec3::parseAll(str.toStdString(), std::back_inserter(positions));
                m_document->createBrush(positions);
            }
        }

        void MapFrame::OnDebugCreateCube() {
            bool ok = false;
            const QString str = QInputDialog::getText(this, "Create Cube", "Enter bounding box size", QLineEdit::Normal, "", &ok);
            if (ok) {
                const double size = str.toDouble();
                const vm::bbox3 bounds(size / 2.0);
                const auto posArray = bounds.vertices();
                const auto posList = std::vector<vm::vec3>(std::begin(posArray), std::end(posArray));
                m_document->createBrush(posList);
            }
        }
        
        void MapFrame::OnDebugClipBrush() {
            bool ok = false;
            const QString str = QInputDialog::getText(this, "Clip Brush", "Enter face points ( x y z ) ( x y z ) ( x y z )", QLineEdit::Normal, "", &ok);
            if (ok) {
                std::vector<vm::vec3> points;
                vm::vec3::parseAll(str.toStdString(), std::back_inserter(points));
                assert(points.size() == 3);
                m_document->clipBrushes(points[0], points[1], points[2]);
            }
        }

        void MapFrame::OnDebugCopyJSShortcutMap() {
            QClipboard *clipboard = QApplication::clipboard();

            const String str = ActionManager::instance().getJSTable();
            clipboard->setText(QString::fromStdString(str));
        }

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wcast-qual"
#endif
        static void debugSegfault() {
            volatile void *test = nullptr;
            printf("%p\n", *((void **)test));
        }
#ifdef __clang__
#pragma clang diagnostic pop
#endif

        static void debugException() {
            Exception e;
            throw e;
        }

        void MapFrame::OnDebugCrash() {
            QStringList items;
            items << "Null pointer dereference" << "Unhandled exception";

            bool ok;
            const QString item = QInputDialog::getItem(this, "Crash", "Choose a crash type", items, 0, false, &ok);
            if (ok) {
                const int idx = items.indexOf(item);
                if (idx == 0) {
                    debugSegfault();
                } else if (idx == 1) {
                    debugException();
                }
            }
        }

        void MapFrame::OnDebugThrowExceptionDuringCommand() {
            m_document->throwExceptionDuringCommand();
        }

        void MapFrame::OnDebugSetWindowSize() {
            bool ok = false;
            const QString str = QInputDialog::getText(this, "Window Size", "Enter Size (W H)", QLineEdit::Normal, "1920 1080", &ok);
            if (ok) {
                const auto size = vm::vec2i::parse(str.toStdString());
                resize(size.x(), size.y());
            }
        }

        void MapFrame::OnFlipObjectsHorizontally() {
            if (m_mapView->canFlipObjects()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->flipObjects(vm::direction::left);
            }
        }

        void MapFrame::OnFlipObjectsVertically() {
            if (m_mapView->canFlipObjects()) { // on gtk, menu shortcuts remain enabled even if the menu item is disabled
                m_mapView->flipObjects(vm::direction::up);
            }
        }

#if 0
        void MapFrame::OnUpdateUI(wxUpdateUIEvent& event) {
		    // FIXME: implement

            const ActionManager& actionManager = ActionManager::instance();

            switch (event.GetId()) {
                case CommandIds::Menu::FileReloadPointFile:
                    event.Enable(canReloadPointFile());
                    break;
                case CommandIds::Menu::FileUnloadPointFile:
                    event.Enable(canUnloadPointFile());
                    break;
                case CommandIds::Menu::FileReloadPortalFile:
                    event.Enable(canReloadPortalFile());
                    break;
                case CommandIds::Menu::FileUnloadPortalFile:
                    event.Enable(canUnloadPortalFile());
                    break;
                case wxID_UNDO: {
                    const ActionMenuItem* item = actionManager.findMenuItem(wxID_UNDO);
                    ensure(item != nullptr, "item is null");
                    if (canUndo()) {
                        event.Enable(true);
                        const auto textEdit = findFocusedTextCtrl() != nullptr;
                        const auto name = textEdit ? "" : m_document->lastCommandName();
                        event.SetText(item->menuString(name, m_mapView->viewportHasFocus()));
                    } else {
                        event.Enable(false);
                        event.SetText(item->menuString("", m_mapView->viewportHasFocus()));
                    }
                    break;
                }
                case wxID_REDO: {
                    const ActionMenuItem* item = actionManager.findMenuItem(wxID_REDO);
                    if (canRedo()) {
                        event.Enable(true);
                        const auto textEdit = findFocusedTextCtrl() != nullptr;
                        const auto name = textEdit ? "" : m_document->nextCommandName();
                        event.SetText(item->menuString(name, m_mapView->viewportHasFocus()));
                    } else {
                        event.Enable(false);
                        event.SetText(item->menuString("", m_mapView->viewportHasFocus()));
                    }
                    break;
                }
                case CommandIds::Menu::EditRepeat:
                case CommandIds::Menu::EditClearRepeat:
                    event.Enable(true);
                    break;
                case wxID_CUT:
                    event.Enable(canCut());
                    break;
                case wxID_COPY:
                    event.Enable(canCopy());
                    break;
                case wxID_PASTE:
                case CommandIds::Menu::EditPasteAtOriginalPosition:
                    event.Enable(canPaste());
                    break;
                case wxID_DUPLICATE:
                    event.Enable(canDuplicate());
                    break;
                case wxID_DELETE:
                    event.Enable(canDelete());
                    break;
                case CommandIds::Menu::EditSelectAll:
                    event.Enable(canSelect());
                    break;
                case CommandIds::Menu::EditSelectSiblings:
                    event.Enable(canSelectSiblings());
                    break;
                case CommandIds::Menu::EditSelectTouching:
                case CommandIds::Menu::EditSelectInside:
                    event.Enable(canSelectByBrush());
                    break;
                case CommandIds::Menu::EditSelectTall:
                    event.Enable(canSelectTall());
                    break;
                case CommandIds::Menu::EditSelectByFilePosition:
                    event.Enable(canSelect());
                    break;
                case CommandIds::Menu::EditSelectNone:
                    event.Enable(canDeselect());
                    break;
                case CommandIds::Menu::EditGroupSelection:
                    event.Enable(canGroup());
                    break;
                case CommandIds::Menu::EditUngroupSelection:
                    event.Enable(canUngroup());
                    break;
                case CommandIds::Menu::EditDeactivateTool:
                    event.Check(!m_mapView->anyToolActive());
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditToggleCreateComplexBrushTool:
                    event.Check(m_mapView->createComplexBrushToolActive());
                    event.Enable(m_mapView->canToggleCreateComplexBrushTool());
                    break;
                case CommandIds::Menu::EditToggleClipTool:
                    event.Check(m_mapView->clipToolActive());
                    event.Enable(m_mapView->canToggleClipTool());
                    break;
                case CommandIds::Menu::EditToggleRotateObjectsTool:
                    event.Check(m_mapView->rotateObjectsToolActive());
                    event.Enable(m_mapView->canToggleRotateObjectsTool());
                    break;
                case CommandIds::Menu::EditToggleScaleObjectsTool:
                    event.Check(m_mapView->scaleObjectsToolActive());
                    event.Enable(m_mapView->canToggleScaleObjectsTool());
                    break;
                case CommandIds::Menu::EditToggleShearObjectsTool:
                    event.Check(m_mapView->shearObjectsToolActive());
                    event.Enable(m_mapView->canToggleShearObjectsTool());
                    break;
                case CommandIds::Menu::EditToggleVertexTool:
                    event.Check(m_mapView->vertexToolActive());
                    event.Enable(m_mapView->canToggleVertexTools());
                    break;
                case CommandIds::Menu::EditToggleEdgeTool:
                    event.Check(m_mapView->edgeToolActive());
                    event.Enable(m_mapView->canToggleVertexTools());
                    break;
                case CommandIds::Menu::EditToggleFaceTool:
                    event.Check(m_mapView->faceToolActive());
                    event.Enable(m_mapView->canToggleVertexTools());
                    break;
                case CommandIds::Menu::EditCsgConvexMerge:
                    event.Enable(canDoCsgConvexMerge());
                    break;
                case CommandIds::Menu::EditCsgSubtract:
                    event.Enable(canDoCsgSubtract());
                    break;
                case CommandIds::Menu::EditCsgIntersect:
                    event.Enable(canDoCsgIntersect());
                    break;
                case CommandIds::Menu::EditCsgHollow:
                    event.Enable(canDoCsgHollow());
                    break;
                case CommandIds::Menu::EditSnapVerticesToInteger:
                case CommandIds::Menu::EditSnapVerticesToGrid:
                    event.Enable(canSnapVertices());
                    break;
                case CommandIds::Menu::EditReplaceTexture:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::EditToggleTextureLock:
                    event.Enable(true);
                    event.Check(pref(Preferences::TextureLock));
                    break;
                case CommandIds::Menu::EditToggleUVLock:
                    event.Enable(true);
                    event.Check(pref(Preferences::UVLock));
                    break;
                case CommandIds::Menu::ViewMoveCameraToNextPoint:
                    event.Enable(canMoveCameraToNextPoint());
                    break;
                case CommandIds::Menu::ViewMoveCameraToPreviousPoint:
                    event.Enable(canMoveCameraToPreviousPoint());
                    break;
                case CommandIds::Menu::ViewFocusCameraOnSelection:
                    event.Enable(canFocusCamera());
                    break;
                case CommandIds::Menu::ViewMoveCameraToPosition:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::ViewHideSelection:
                    event.Enable(canHide());
                    break;
                case CommandIds::Menu::ViewIsolateSelection:
                    event.Enable(canIsolate());
                    break;
                case CommandIds::Menu::ViewUnhideAll:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::ViewSwitchToMapInspector:
                case CommandIds::Menu::ViewSwitchToEntityInspector:
                case CommandIds::Menu::ViewSwitchToFaceInspector:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::ViewToggleMaximizeCurrentView:
                    event.Enable(m_mapView->canMaximizeCurrentView());
                    event.Check(m_mapView->currentViewMaximized());
                    break;
                case CommandIds::Menu::ViewToggleInfoPanel:
                    event.Enable(true);
                    event.Check(!m_vSplitter->isMaximized(m_mapView));
                    break;
                case CommandIds::Menu::ViewToggleInspector:
                    event.Enable(true);
                    event.Check(!m_hSplitter->isMaximized(m_vSplitter));
                    break;
                case CommandIds::Menu::RunCompile:
                    event.Enable(canCompile());
                    break;
                case CommandIds::Menu::RunLaunch:
                    event.Enable(canLaunch());
                    break;
                case CommandIds::Menu::DebugPrintVertices:
                case CommandIds::Menu::DebugCreateBrush:
                case CommandIds::Menu::DebugCreateCube:
                case CommandIds::Menu::DebugCopyJSShortcuts:
                case CommandIds::Menu::DebugCrash:
                case CommandIds::Menu::DebugThrowExceptionDuringCommand:
                case CommandIds::Menu::DebugSetWindowSize:
                    event.Enable(true);
                    break;
                case CommandIds::Menu::DebugClipWithFace:
                    event.Enable(m_document->selectedNodes().hasOnlyBrushes());
                    break;
                case CommandIds::Actions::FlipObjectsHorizontally:
                case CommandIds::Actions::FlipObjectsVertically:
                    event.Enable(m_mapView->canFlipObjects());
                    break;
                default:
                    event.Enable(event.GetId() >= CommandIds::Menu::FileRecentDocuments && event.GetId() < CommandIds::Menu::FileRecentDocuments + 10);
                    break;
            }
        }
#endif

        void MapFrame::OnToolBarSetGridSize() {
		    // FIXME: implement
            //m_document->grid().setSize(gridSizeForIndex(event.GetSelection()));
        }

        bool MapFrame::canUnloadPointFile() const {
            return m_document->isPointFileLoaded();
        }

        bool MapFrame::canReloadPointFile() const {
            return m_document->canReloadPointFile();
        }

        bool MapFrame::canUnloadPortalFile() const {
            return m_document->isPortalFileLoaded();
        }

        bool MapFrame::canReloadPortalFile() const {
            return m_document->canReloadPortalFile();
        }

        bool MapFrame::canUndo() const {
            // FIXME:
            auto textCtrl = nullptr;//findFocusedTextCtrl();
            if (textCtrl != nullptr) {
                return true; // textCtrl->CanUndo();
            } else {
                return m_document->canUndoLastCommand();
            }
        }

        void MapFrame::undo() {
            assert(canUndo());

            // FIXME:
            auto textCtrl = nullptr;//findFocusedTextCtrl();
            if (textCtrl != nullptr) {
                //textCtrl->Undo();
            } else {
                // FIXME:
                if (!m_mapView->cancelMouseDrag() /* && !m_inspector->cancelMouseDrag()*/) {
                    m_document->undoLastCommand();
                }
            }
        }

        bool MapFrame::canRedo() const {
            // FIXME:
            auto textCtrl = nullptr; //findFocusedTextCtrl();
            if (textCtrl != nullptr) {
                return true; // textCtrl->CanRedo();
            } else {
                return m_document->canRedoNextCommand();
            }
        }

        void MapFrame::redo() {
            assert(canRedo());

            // FIXME:
            auto textCtrl = nullptr; //findFocusedTextCtrl();
            if (textCtrl != nullptr) {
                //textCtrl->Redo();
            } else {
                m_document->redoNextCommand();
            }
        }

#if 0
        wxTextCtrl* MapFrame::findFocusedTextCtrl() const {
		    return nullptr;
		    // FIXME:
//            return dynamic_cast<wxTextCtrl*>(FindFocus());
        }
#endif

        bool MapFrame::canCut() const {
            return m_document->hasSelectedNodes() && !m_mapView->anyToolActive();
        }

        bool MapFrame::canCopy() const {
            return m_document->hasSelectedNodes() || m_document->hasSelectedBrushFaces();
        }

        bool MapFrame::canPaste() const {
            if (!m_mapView->isCurrent())
                return false;

            QClipboard *clipboard = QApplication::clipboard();
            return !clipboard->text().isEmpty();

        }

        bool MapFrame::canDelete() const {
            if (m_mapView->clipToolActive())
                return m_mapView->clipTool()->canRemoveLastPoint();
            else if (m_mapView->vertexToolActive())
                return m_mapView->vertexTool()->canRemoveSelection();
            else if (m_mapView->edgeToolActive())
                return m_mapView->edgeTool()->canRemoveSelection();
            else if (m_mapView->faceToolActive())
                return m_mapView->faceTool()->canRemoveSelection();
            else
                return canCut();
        }

        bool MapFrame::canDuplicate() const {
            return m_document->hasSelectedNodes();
        }

        bool MapFrame::canSelectSiblings() const {
            return canChangeSelection() && m_document->hasSelectedNodes();
        }

        bool MapFrame::canSelectByBrush() const {
            return canChangeSelection() && m_document->selectedNodes().hasOnlyBrushes();
        }

        bool MapFrame::canSelectTall() const {
            return canChangeSelection() && m_document->selectedNodes().hasOnlyBrushes() && m_mapView->canSelectTall();
        }

        bool MapFrame::canSelect() const {
            return canChangeSelection();
        }

        bool MapFrame::canDeselect() const {
            return canChangeSelection() && m_document->hasSelectedNodes();
        }

        bool MapFrame::canChangeSelection() const {
            return m_document->editorContext().canChangeSelection();
        }

        bool MapFrame::canGroup() const {
            return m_document->hasSelectedNodes() && !m_mapView->anyToolActive();
        }

        bool MapFrame::canUngroup() const {
            return m_document->selectedNodes().hasOnlyGroups() && !m_mapView->anyToolActive();
        }

        bool MapFrame::canHide() const {
            return m_document->hasSelectedNodes();
        }

        bool MapFrame::canIsolate() const {
            return m_document->hasSelectedNodes();
        }

        bool MapFrame::canDoCsgConvexMerge() const {
            return (m_document->hasSelectedBrushFaces() && m_document->selectedBrushFaces().size() > 1) ||
                   (m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() > 1) ||
                   (m_mapView->vertexToolActive() && m_mapView->vertexTool()->canDoCsgConvexMerge()) ||
                   (m_mapView->edgeToolActive() && m_mapView->edgeTool()->canDoCsgConvexMerge()) ||
                   (m_mapView->faceToolActive() && m_mapView->faceTool()->canDoCsgConvexMerge());
        }

        bool MapFrame::canDoCsgSubtract() const {
            return m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() >= 1;
        }

        bool MapFrame::canDoCsgIntersect() const {
            return m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() > 1;
        }

        bool MapFrame::canDoCsgHollow() const {
            return m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() >= 1;
        }

        bool MapFrame::canSnapVertices() const {
            return m_document->selectedNodes().hasOnlyBrushes();
        }

        bool MapFrame::canDecGridSize() const {
            return m_document->grid().size() > Grid::MinSize;
        }

        bool MapFrame::canIncGridSize() const {
            return m_document->grid().size() < Grid::MaxSize;
        }

        bool MapFrame::canMoveCameraToNextPoint() const {
            return m_mapView->canMoveCameraToNextTracePoint();
        }

        bool MapFrame::canMoveCameraToPreviousPoint() const {
            return m_mapView->canMoveCameraToPreviousTracePoint();
        }

        bool MapFrame::canFocusCamera() const {
            return m_document->hasSelectedNodes();
        }

        bool MapFrame::canCompile() const {
            return m_document->persistent();
        }

        bool MapFrame::canLaunch() const {
            return m_document->persistent();
        }

#if 0
        void MapFrame::OnClose(wxCloseEvent& event) {
		    // FIXME: implement

            if (!IsBeingDeleted()) {
                if (m_compilationDialog != nullptr && !m_compilationDialog->Close()) {
                    event.Veto();
                } else {
                    ensure(m_frameManager != nullptr, "frameManager is null");
                    if (event.CanVeto() && !confirmOrDiscardChanges())
                        event.Veto();
                    else
                        m_frameManager->removeAndDestroyFrame(this);
                }
            }
        }
#endif
        void MapFrame::OnAutosaveTimer() {
            m_autosaver->triggerAutosave(logger());
        }
        
        int MapFrame::indexForGridSize(const int gridSize) {
            return gridSize - Grid::MinSize;
        }
        
        int MapFrame::gridSizeForIndex(const int index) {
            const int size = index + Grid::MinSize;
            assert(size <= Grid::MaxSize);
            assert(size >= Grid::MinSize);
            return size;
        }
        
        int MapFrame::gridSizeForMenuId(const int menuId) {
            const int size = menuId - CommandIds::Menu::ViewSetGridSize1;
            assert(size <= Grid::MaxSize);
            assert(size >= Grid::MinSize);
            return size;
        }
    }
}
