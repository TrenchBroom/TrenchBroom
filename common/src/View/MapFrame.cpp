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
#include <QToolBar>
#include <QComboBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

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
#include "View/Autosaver.h"
#include "View/BorderLine.h"
#include "View/CachingLogger.h"
#include "FileLogger.h"
#include "View/ActionList.h"
#include "View/ClipTool.h"
// FIXME:
//#include "View/CompilationDialog.h"
#include "View/Console.h"
#include "View/EdgeTool.h"
#include "View/FaceTool.h"
#include "View/GLContextManager.h"
#include "View/Grid.h"
#include "View/InfoPanel.h"
#include "View/Inspector.h"
//#include "View/LaunchGameEngineDialog.h"
#include "View/MapDocument.h"
//#include "View/MapFrameDropTarget.h"
#include "View/RenderView.h"
//#include "View/ReplaceTextureDialog.h"
#include "View/SwitchableMapViewContainer.h"
#include "View/VertexTool.h"
#include "View/ViewUtils.h"
#include "View/wxUtils.h"
#include "View/MapViewToolBox.h"

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
        m_hSplitter(nullptr),
        m_vSplitter(nullptr),
        m_contextManager(nullptr),
        m_mapView(nullptr),
        m_infoPanel(nullptr),
        m_console(nullptr),
        m_inspector(nullptr),
        m_gridChoice(nullptr) {}

        MapFrame::MapFrame(FrameManager* frameManager, MapDocumentSPtr document) :
        QMainWindow(),
        m_frameManager(frameManager),
        m_autosaver(nullptr),
        m_autosaveTimer(nullptr),
        m_hSplitter(nullptr),
        m_vSplitter(nullptr),
        m_contextManager(nullptr),
        m_mapView(nullptr),
        m_infoPanel(nullptr),
        m_console(nullptr),
        m_inspector(nullptr),
        m_gridChoice(nullptr) {
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
            createActions();
            createToolBar();

            updateBindings();
            updateGridActions();
            updateToolActions();
            updateOtherActions();
            updateUndoRedoActions();
            updateClipboardActions();

            createMenus();
            createStatusBar();

            m_document->setParentLogger(m_console);
            m_document->setViewEffectsService(m_mapView);

            m_autosaveTimer = new QTimer(this);
            m_autosaveTimer->start(1000);

            bindObservers();
            bindEvents();

            clearDropTarget();
            setAcceptDrops(true);
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

        void MapFrame::positionOnScreen(QWidget* reference) {
            // FIXME: Restore saved size from preferences here?

            resize(1024, 768);
            if (reference) {
                move(reference->pos() + QPoint(23, 23));
            } else {
                // FIXME: Should we bother centering it on screen like the wx version did?
            }
        }

        MapDocumentSPtr MapFrame::document() const {
            return m_document;
        }

        Logger& MapFrame::logger() const {
            return *m_console;
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

        void MapFrame::dragEnterEvent(QDragEnterEvent* event) {
            // TODO: Also need this for MapViewBase, and the texture / entity browser too maybe.
            // See if it can be factored out into an event filter object?
            if (event->mimeData()->hasUrls()) {
                event->acceptProposedAction();
            }
        }

        void MapFrame::dropEvent(QDropEvent* event) {
            const QMimeData* mimeData = event->mimeData();

            for (const QUrl& url : mimeData->urls()) {
               const QString qString = url.toLocalFile();

               loadTextureCollection(m_document, this, qString);
               loadEntityDefinitionFile(m_document, this, qString);
            }
            event->acceptProposedAction();
        }

        bool MapFrame::newDocument(Model::GameSPtr game, const Model::MapFormat mapFormat) {
            if (!confirmOrDiscardChanges())
                return false;
            m_document->newDocument(mapFormat, MapDocument::DefaultWorldBounds, game);
            return true;
        }

        bool MapFrame::openDocument(Model::GameSPtr game, const Model::MapFormat mapFormat, const IO::Path& path) {
            if (!confirmOrDiscardChanges())
                return false;
            m_document->loadDocument(mapFormat, MapDocument::DefaultWorldBounds, game, path);
            return true;
        }

        bool MapFrame::saveDocument() {
            try {
                if (m_document->persistent()) {
                    m_document->saveDocument();
                    logger().info() << "Saved " << m_document->path();
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
                logger().info() << "Saved " << m_document->path();
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

            const QString newFileName = QFileDialog::getSaveFileName(this, "Export Wavefront OBJ file", QString::fromStdString(objPath.asString()), "Wavefront OBJ files (*.obj)");
            if (newFileName.isEmpty())
                return false;

            return exportDocument(Model::WavefrontObj, IO::Path(newFileName.toStdString()));
        }

        bool MapFrame::exportDocument(const Model::ExportFormat format, const IO::Path& path) {
            try {
                m_document->exportDocumentAs(format, path);
                logger().info() << "Exported " << path;
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

        void MapFrame::createActions() {
		    // File

            fileNewAction = new QAction("New", this);
            registerBinding(fileNewAction, ActionList::instance().menuFileNewInfo);
            connect(fileNewAction, &QAction::triggered, &TrenchBroomApp::instance(), &TrenchBroomApp::OnFileNew);

            fileOpenAction = new QAction("Open", this);
            registerBinding(fileOpenAction, ActionList::instance().menuFileOpenInfo);
            connect(fileOpenAction, &QAction::triggered, &TrenchBroomApp::instance(), &TrenchBroomApp::OnFileOpen);

            fileSaveAction = new QAction("Save", this);
            registerBinding(fileSaveAction, ActionList::instance().menuFileSaveInfo);
            connect(fileSaveAction, &QAction::triggered, this, &MapFrame::OnFileSave);

            fileSaveAsAction = new QAction("Save as...", this);
            registerBinding(fileSaveAsAction, ActionList::instance().menuFileSaveasInfo);
            connect(fileSaveAsAction, &QAction::triggered, this, &MapFrame::OnFileSaveAs);

            fileExportObjAction = new QAction("Wavefront OBJ...", this);
            registerBinding(fileExportObjAction, ActionList::instance().menuFileExportWavefrontOBJInfo);
            connect(fileExportObjAction, &QAction::triggered, this, &MapFrame::OnFileExportObj);

            fileLoadPointFileAction = new QAction("Load Point File...", this);
            registerBinding(fileLoadPointFileAction, ActionList::instance().menuFileLoadPointFileInfo);
            connect(fileLoadPointFileAction, &QAction::triggered, this, &MapFrame::OnFileLoadPointFile);

            fileReloadPointFileAction = new QAction("Reload Point File", this);
            registerBinding(fileReloadPointFileAction, ActionList::instance().menuFileReloadPointFileInfo);
            connect(fileReloadPointFileAction, &QAction::triggered, this, &MapFrame::OnFileReloadPointFile);

            fileUnloadPointFileAction = new QAction("Unload Point File", this);
            registerBinding(fileUnloadPointFileAction, ActionList::instance().menuFileUnloadPointFileInfo);
            connect(fileUnloadPointFileAction, &QAction::triggered, this, &MapFrame::OnFileUnloadPointFile);

            fileLoadPortalFileAction = new QAction("Load Portal File...", this);
            registerBinding(fileLoadPortalFileAction, ActionList::instance().menuFileLoadPortalFileInfo);
            connect(fileLoadPortalFileAction, &QAction::triggered, this, &MapFrame::OnFileLoadPortalFile);

            fileReloadPortalFileAction = new QAction("Reload Portal File", this);
            registerBinding(fileReloadPortalFileAction, ActionList::instance().menuFileReloadPortalFileInfo);
            connect(fileReloadPortalFileAction, &QAction::triggered, this, &MapFrame::OnFileReloadPortalFile);

            fileUnloadPortalFileAction = new QAction("Unload Portal File", this);
            registerBinding(fileUnloadPortalFileAction, ActionList::instance().menuFileUnloadPortalFileInfo);
            connect(fileUnloadPortalFileAction, &QAction::triggered, this, &MapFrame::OnFileUnloadPortalFile);

            fileReloadTextureCollectionsAction = new QAction("Reload Texture Collections", this);
            registerBinding(fileReloadTextureCollectionsAction,
                            ActionList::instance().menuFileReloadTextureCollectionsInfo);
            connect(fileReloadTextureCollectionsAction, &QAction::triggered, this, &MapFrame::OnFileReloadTextureCollections);

            fileReloadEntityDefinitionsAction = new QAction("Reload Entity Definitions", this);
            registerBinding(fileReloadEntityDefinitionsAction,
                            ActionList::instance().menuFileReloadEntityDefinitionsInfo);
            connect(fileReloadEntityDefinitionsAction, &QAction::triggered, this, &MapFrame::OnFileReloadEntityDefinitions);

            fileCloseAction = new QAction("Close", this);
            registerBinding(fileCloseAction, ActionList::instance().menuFileCloseInfo);
            connect(fileCloseAction, &QAction::triggered, this, &MapFrame::OnFileClose);

            // Edit

            editUndoAction = new QAction("Undo", this);
            registerBinding(editUndoAction, ActionList::instance().menuEditUndoInfo);
            connect(editUndoAction, &QAction::triggered, this, &MapFrame::OnEditUndo); //, this, wxID_UNDO);

            editRedoAction = new QAction("Redo", this);
            registerBinding(editRedoAction, ActionList::instance().menuEditRedoInfo);
            connect(editRedoAction, &QAction::triggered, this, &MapFrame::OnEditRedo); //, this, wxID_REDO);

            editRepeatAction = new QAction("Repeat", this);
            registerBinding(editRepeatAction, ActionList::instance().menuEditRepeatInfo);
            connect(editRepeatAction, &QAction::triggered, this, &MapFrame::OnEditRepeat); //, this, CommandIds::Menu::EditRepeat);

            editClearRepeatAction = new QAction("Clear Repeatable Commands", this);
            registerBinding(editClearRepeatAction, ActionList::instance().menuEditClearRepeatableCommandsInfo);
            connect(editClearRepeatAction, &QAction::triggered, this, &MapFrame::OnEditClearRepeat); //, this, CommandIds::Menu::EditClearRepeat);


            editCutAction = new QAction("Cut", this);
            registerBinding(editCutAction, ActionList::instance().menuEditCutInfo);
            connect(editCutAction, &QAction::triggered, this, &MapFrame::OnEditCut); //, this, wxID_CUT);

            editCopyAction = new QAction("Copy", this);
            registerBinding(editCopyAction, ActionList::instance().menuEditCopyInfo);
            connect(editCopyAction, &QAction::triggered, this, &MapFrame::OnEditCopy); //, this, wxID_COPY);

            editPasteAction = new QAction("Paste", this);
            registerBinding(editPasteAction, ActionList::instance().menuEditPasteInfo);
            connect(editPasteAction, &QAction::triggered, this, &MapFrame::OnEditPaste);

            editPasteAtOriginalPositionAction = new QAction("Paste at Original Position", this);
            registerBinding(editPasteAtOriginalPositionAction,
                            ActionList::instance().menuEditPasteatOriginalPositionInfo);
            connect(editPasteAtOriginalPositionAction, &QAction::triggered, this, &MapFrame::OnEditPasteAtOriginalPosition);

            editDuplicateAction = new QAction("Duplicate", this);
            registerBinding(editDuplicateAction, ActionList::instance().menuEditDuplicateInfo);
            editDuplicateAction->setIcon(IO::loadIconResourceQt(IO::Path("DuplicateObjects.png")));
            connect(editDuplicateAction, &QAction::triggered, this, &MapFrame::OnEditDuplicate);

            editDeleteAction = new QAction("Delete", this);
            registerBinding(editDeleteAction, ActionList::instance().menuEditDeleteInfo);
            connect(editDeleteAction, &QAction::triggered, this, &MapFrame::OnEditDelete);


            editSelectAllAction = new QAction("Select All", this);
            registerBinding(editSelectAllAction, ActionList::instance().menuEditSelectAllInfo);
            connect(editSelectAllAction, &QAction::triggered, this, &MapFrame::OnEditSelectAll); //, this, CommandIds::Menu::EditSelectAll);

            editSelectSiblingsAction = new QAction("Select Siblings", this);
            registerBinding(editSelectSiblingsAction, ActionList::instance().menuEditSelectSiblingsInfo);
            connect(editSelectSiblingsAction, &QAction::triggered, this, &MapFrame::OnEditSelectSiblings); //, this, CommandIds::Menu::EditSelectSiblings);

            editSelectTouchingAction = new QAction("Select Touching", this);
            registerBinding(editSelectTouchingAction, ActionList::instance().menuEditSelectTouchingInfo);
            connect(editSelectTouchingAction, &QAction::triggered, this, &MapFrame::OnEditSelectTouching); //, this, CommandIds::Menu::EditSelectTouching);

            editSelectInsideAction = new QAction("Select Inside", this);
            registerBinding(editSelectInsideAction, ActionList::instance().menuEditSelectInsideInfo);
            connect(editSelectInsideAction, &QAction::triggered, this, &MapFrame::OnEditSelectInside); //, this, CommandIds::Menu::EditSelectInside);

            editSelectTallAction = new QAction("Select Tall", this);
            registerBinding(editSelectTallAction, ActionList::instance().menuEditSelectTallInfo);
            connect(editSelectTallAction, &QAction::triggered, this, &MapFrame::OnEditSelectTall); //, this, CommandIds::Menu::EditSelectTall);

            editSelectByLineNumberAction = new QAction("Select by Line Number", this);
            registerBinding(editSelectByLineNumberAction, ActionList::instance().menuEditSelectbyLineNumberInfo);
            connect(editSelectByLineNumberAction, &QAction::triggered, this, &MapFrame::OnEditSelectByLineNumber); //, this, CommandIds::Menu::EditSelectByFilePosition);

            editSelectNoneAction = new QAction("Select None", this);
            registerBinding(editSelectNoneAction, ActionList::instance().menuEditSelectNoneInfo);
            connect(editSelectNoneAction, &QAction::triggered, this, &MapFrame::OnEditSelectNone); //, this, CommandIds::Menu::EditSelectNone);


            editGroupSelectedObjectsAction = new QAction("Group", this);
            registerBinding(editGroupSelectedObjectsAction, ActionList::instance().menuEditGroupInfo);
            connect(editGroupSelectedObjectsAction, &QAction::triggered, this, &MapFrame::OnEditGroupSelectedObjects); //, this, CommandIds::Menu::EditGroupSelection);

            editUngroupSelectedObjectsAction = new QAction("Ungroup", this);
            registerBinding(editUngroupSelectedObjectsAction, ActionList::instance().menuEditUngroupInfo);
            connect(editUngroupSelectedObjectsAction, &QAction::triggered, this, &MapFrame::OnEditUngroupSelectedObjects); //, this, CommandIds::Menu::EditUngroupSelection);


            editToolActionGroup = new QActionGroup(this);

            editDeactivateToolAction = new QAction("Deactivate Tool", editToolActionGroup);
            editDeactivateToolAction->setIcon(IO::loadIconResourceQt(IO::Path("NoTool.png")));
            connect(editDeactivateToolAction, &QAction::triggered, this, &MapFrame::OnEditDeactivateTool); //, this, CommandIds::Menu::EditDeactivateTool);

            editToggleCreateComplexBrushToolAction = new QAction("Brush Tool", editToolActionGroup);
            editToggleCreateComplexBrushToolAction->setIcon(IO::loadIconResourceQt(IO::Path("BrushTool.png")));
            editToggleCreateComplexBrushToolAction->setCheckable(true);
            registerBinding(editToggleCreateComplexBrushToolAction, ActionList::instance().menuEditToolsBrushToolInfo);
            connect(editToggleCreateComplexBrushToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleCreateComplexBrushTool); //, this, CommandIds::Menu::EditToggleCreateComplexBrushTool);

            editToggleClipToolAction = new QAction("Clip Tool", editToolActionGroup);
            editToggleClipToolAction->setIcon(IO::loadIconResourceQt(IO::Path("ClipTool.png")));
            editToggleClipToolAction->setCheckable(true);
            registerBinding(editToggleClipToolAction, ActionList::instance().menuEditToolsClipToolInfo);
            connect(editToggleClipToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleClipTool); //, this, CommandIds::Menu::EditToggleClipTool);

            editToggleRotateObjectsToolAction = new QAction("Rotate Tool", editToolActionGroup);
            editToggleRotateObjectsToolAction->setIcon(IO::loadIconResourceQt(IO::Path("RotateTool.png")));
            editToggleRotateObjectsToolAction->setCheckable(true);
            registerBinding(editToggleRotateObjectsToolAction, ActionList::instance().menuEditToolsRotateToolInfo);
            connect(editToggleRotateObjectsToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleRotateObjectsTool); //, this, CommandIds::Menu::EditToggleRotateObjectsTool);

            editToggleScaleObjectsToolAction = new QAction("Scale Tool", editToolActionGroup);
            editToggleScaleObjectsToolAction->setIcon(IO::loadIconResourceQt(IO::Path("ScaleTool.png")));
            editToggleScaleObjectsToolAction->setCheckable(true);
            registerBinding(editToggleScaleObjectsToolAction, ActionList::instance().menuEditToolsScaleToolInfo);
            connect(editToggleScaleObjectsToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleScaleObjectsTool); //, this, CommandIds::Menu::EditToggleScaleObjectsTool);

            editToggleShearObjectsToolAction = new QAction("Shear Tool", editToolActionGroup);
            editToggleShearObjectsToolAction->setIcon(IO::loadIconResourceQt(IO::Path("ShearTool.png")));
            editToggleShearObjectsToolAction->setCheckable(true);
            registerBinding(editToggleShearObjectsToolAction, ActionList::instance().menuEditToolsShearToolInfo);
            connect(editToggleShearObjectsToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleShearObjectsTool); //, this, CommandIds::Menu::EditToggleShearObjectsTool);

            editToggleVertexToolAction = new QAction("Vertex Tool", editToolActionGroup);
            editToggleVertexToolAction->setIcon(IO::loadIconResourceQt(IO::Path("VertexTool.png")));
            editToggleVertexToolAction->setCheckable(true);
            registerBinding(editToggleVertexToolAction, ActionList::instance().menuEditToolsVertexToolInfo);
            connect(editToggleVertexToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleVertexTool); //, this, CommandIds::Menu::EditToggleVertexTool);

            editToggleEdgeToolAction = new QAction("Edge Tool", editToolActionGroup);
            editToggleEdgeToolAction->setIcon(IO::loadIconResourceQt(IO::Path("EdgeTool.png")));
            editToggleEdgeToolAction->setCheckable(true);
            registerBinding(editToggleEdgeToolAction, ActionList::instance().menuEditToolsEdgeToolInfo);
            connect(editToggleEdgeToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleEdgeTool); //, this, CommandIds::Menu::EditToggleEdgeTool);

            editToggleFaceToolAction = new QAction("Face Tool", editToolActionGroup);
            editToggleFaceToolAction->setIcon(IO::loadIconResourceQt(IO::Path("FaceTool.png")));
            editToggleFaceToolAction->setCheckable(true);
            registerBinding(editToggleFaceToolAction, ActionList::instance().menuEditToolsFaceToolInfo);
            connect(editToggleFaceToolAction, &QAction::triggered, this, &MapFrame::OnEditToggleFaceTool); //, this, CommandIds::Menu::EditToggleFaceTool);


            editCsgConvexMergeAction = new QAction("Convex Merge", this);
            registerBinding(editCsgConvexMergeAction, ActionList::instance().menuEditCSGConvexMergeInfo);
            connect(editCsgConvexMergeAction, &QAction::triggered, this, &MapFrame::OnEditCsgConvexMerge); //, this, CommandIds::Menu::EditCsgConvexMerge);

            editCsgSubtractAction = new QAction("Subtract", this);
            registerBinding(editCsgSubtractAction, ActionList::instance().menuEditCSGSubtractInfo);
            connect(editCsgSubtractAction, &QAction::triggered, this, &MapFrame::OnEditCsgSubtract); //, this, CommandIds::Menu::EditCsgSubtract);

            editCsgIntersectAction = new QAction("Intersect", this);
            registerBinding(editCsgIntersectAction, ActionList::instance().menuEditCSGIntersectInfo);
            connect(editCsgIntersectAction, &QAction::triggered, this, &MapFrame::OnEditCsgIntersect); //, this, CommandIds::Menu::EditCsgIntersect);

            editCsgHollowAction = new QAction("Hollow", this);
            registerBinding(editCsgHollowAction, ActionList::instance().menuEditCSGHollowInfo);
            connect(editCsgHollowAction, &QAction::triggered, this, &MapFrame::OnEditCsgHollow); //, this, CommandIds::Menu::EditCsgHollow);


            editReplaceTextureAction = new QAction("Replace Texture...", this);
            registerBinding(editReplaceTextureAction, ActionList::instance().menuEditReplaceTextureInfo);
            connect(editReplaceTextureAction, &QAction::triggered, this, &MapFrame::OnEditReplaceTexture); //, this, CommandIds::Menu::EditReplaceTexture);

            editToggleTextureLockAction = new QAction("Texture Lock", this);
            registerBinding(editToggleTextureLockAction, ActionList::instance().menuEditTextureLockInfo);
            editToggleTextureLockAction->setCheckable(true);
            editToggleTextureLockAction->setIcon(IO::loadIconResourceOffOnQt(IO::Path("TextureLockOff.png"), IO::Path("TextureLockOn.png")));
            connect(editToggleTextureLockAction, &QAction::triggered, this, &MapFrame::OnEditToggleTextureLock); //, this, CommandIds::Menu::EditToggleTextureLock);

            editToggleUVLockAction = new QAction("UV Lock", this);
            registerBinding(editToggleUVLockAction, ActionList::instance().menuEditUVLockInfo);
            editToggleUVLockAction->setCheckable(true);
            editToggleUVLockAction->setIcon(IO::loadIconResourceOffOnQt(IO::Path("UVLockOff.png"), IO::Path("UVLockOn.png")));
            connect(editToggleUVLockAction, &QAction::triggered, this, &MapFrame::OnEditToggleUVLock); //, this, CommandIds::Menu::EditToggleUVLock);

            editSnapVerticesToIntegerAction = new QAction("Snap Vertices to Integer", this);
            registerBinding(editSnapVerticesToIntegerAction, ActionList::instance().menuEditSnapVerticestoIntegerInfo);
            connect(editSnapVerticesToIntegerAction, &QAction::triggered, this, &MapFrame::OnEditSnapVerticesToInteger); //, this, CommandIds::Menu::EditSnapVerticesToInteger);

            editSnapVerticesToGridAction = new QAction("Snap Vertices to Grid", this);
            registerBinding(editSnapVerticesToGridAction, ActionList::instance().menuEditSnapVerticestoGridInfo);
            connect(editSnapVerticesToGridAction, &QAction::triggered, this, &MapFrame::OnEditSnapVerticesToGrid); //, this, CommandIds::Menu::EditSnapVerticesToGrid);

            // View

            viewToggleShowGridAction = new QAction("Show Grid", this);
            viewToggleShowGridAction->setCheckable(true);
            registerBinding(viewToggleShowGridAction, ActionList::instance().menuViewGridShowGridInfo);
            connect(viewToggleShowGridAction, &QAction::triggered, this, &MapFrame::OnViewToggleShowGrid);

            viewToggleSnapToGridAction = new QAction("Snap to Grid", this);
            viewToggleSnapToGridAction->setCheckable(true);
            registerBinding(viewToggleSnapToGridAction, ActionList::instance().menuViewGridSnaptoGridInfo);
            connect(viewToggleSnapToGridAction, &QAction::triggered, this, &MapFrame::OnViewToggleSnapToGrid);

            viewIncGridSizeAction = new QAction("Increase Grid Size", this);
            registerBinding(viewIncGridSizeAction, ActionList::instance().menuViewGridIncreaseGridSizeInfo);
            connect(viewIncGridSizeAction, &QAction::triggered, this, &MapFrame::OnViewIncGridSize);

            viewDecGridSizeAction = new QAction("Decrease Grid Size", this);
            registerBinding(viewDecGridSizeAction, ActionList::instance().menuViewGridDecreaseGridSizeInfo);
            connect(viewDecGridSizeAction, &QAction::triggered, this, &MapFrame::OnViewDecGridSize);

            viewSetGridSizeActionGroup = new QActionGroup(this);

            viewSetGridSize0Point125Action = new QAction("Set Grid Size 0.125", viewSetGridSizeActionGroup);
            viewSetGridSize0Point125Action->setData(QVariant(-3));
            viewSetGridSize0Point125Action->setCheckable(true);
            registerBinding(viewSetGridSize0Point125Action, ActionList::instance().menuViewGridSetGridSize0125Info);
            connect(viewSetGridSize0Point125Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize0Point25Action = new QAction("Set Grid Size 0.25", viewSetGridSizeActionGroup);
            viewSetGridSize0Point25Action->setData(QVariant(-2));
            viewSetGridSize0Point25Action->setCheckable(true);
            registerBinding(viewSetGridSize0Point25Action, ActionList::instance().menuViewGridSetGridSize025Info);
            connect(viewSetGridSize0Point25Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize0Point5Action = new QAction("Set Grid Size 0.5", viewSetGridSizeActionGroup);
            viewSetGridSize0Point5Action->setData(QVariant(-1));
            viewSetGridSize0Point5Action->setCheckable(true);
            registerBinding(viewSetGridSize0Point5Action, ActionList::instance().menuViewGridSetGridSize05Info);
            connect(viewSetGridSize0Point5Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize1Action = new QAction("Set Grid Size 1", viewSetGridSizeActionGroup);
            viewSetGridSize1Action->setData(QVariant(0));
            viewSetGridSize1Action->setCheckable(true);
            registerBinding(viewSetGridSize1Action, ActionList::instance().menuViewGridSetGridSize1Info);
            connect(viewSetGridSize1Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize2Action = new QAction("Set Grid Size 2", viewSetGridSizeActionGroup);
            viewSetGridSize2Action->setData(QVariant(1));
            viewSetGridSize2Action->setCheckable(true);
            registerBinding(viewSetGridSize2Action, ActionList::instance().menuViewGridSetGridSize2Info);
            connect(viewSetGridSize2Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize4Action = new QAction("Set Grid Size 4", viewSetGridSizeActionGroup);
            viewSetGridSize4Action->setData(QVariant(2));
            viewSetGridSize4Action->setCheckable(true);
            registerBinding(viewSetGridSize4Action, ActionList::instance().menuViewGridSetGridSize4Info);
            connect(viewSetGridSize4Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize8Action = new QAction("Set Grid Size 8", viewSetGridSizeActionGroup);
            viewSetGridSize8Action->setData(QVariant(3));
            viewSetGridSize8Action->setCheckable(true);
            registerBinding(viewSetGridSize8Action, ActionList::instance().menuViewGridSetGridSize8Info);
            connect(viewSetGridSize8Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize16Action = new QAction("Set Grid Size 16", viewSetGridSizeActionGroup);
            viewSetGridSize16Action->setData(QVariant(4));
            viewSetGridSize16Action->setCheckable(true);
            registerBinding(viewSetGridSize16Action, ActionList::instance().menuViewGridSetGridSize16Info);
            connect(viewSetGridSize16Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize32Action = new QAction("Set Grid Size 32", viewSetGridSizeActionGroup);
            viewSetGridSize32Action->setData(QVariant(5));
            viewSetGridSize32Action->setCheckable(true);
            registerBinding(viewSetGridSize32Action, ActionList::instance().menuViewGridSetGridSize32Info);
            connect(viewSetGridSize32Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize64Action = new QAction("Set Grid Size 64", viewSetGridSizeActionGroup);
            viewSetGridSize64Action->setData(QVariant(6));
            viewSetGridSize64Action->setCheckable(true);
            registerBinding(viewSetGridSize64Action, ActionList::instance().menuViewGridSetGridSize64Info);
            connect(viewSetGridSize64Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize128Action = new QAction("Set Grid Size 128", viewSetGridSizeActionGroup);
            viewSetGridSize128Action->setData(QVariant(7));
            viewSetGridSize128Action->setCheckable(true);
            registerBinding(viewSetGridSize128Action, ActionList::instance().menuViewGridSetGridSize128Info);
            connect(viewSetGridSize128Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewSetGridSize256Action = new QAction("Set Grid Size 256", viewSetGridSizeActionGroup);
            viewSetGridSize256Action->setData(QVariant(8));
            viewSetGridSize256Action->setCheckable(true);
            registerBinding(viewSetGridSize256Action, ActionList::instance().menuViewGridSetGridSize256Info);
            connect(viewSetGridSize256Action, &QAction::triggered, this, &MapFrame::OnViewSetGridSize);

            viewMoveCameraToNextPointAction = new QAction("Move to Next Point", this);
            registerBinding(viewMoveCameraToNextPointAction, ActionList::instance().menuViewCameraMovetoNextPointInfo);
            connect(viewMoveCameraToNextPointAction, &QAction::triggered, this, &MapFrame::OnViewMoveCameraToNextPoint);

            viewMoveCameraToPreviousPointAction = new QAction("Move to Previous Point", this);
            registerBinding(viewMoveCameraToPreviousPointAction, ActionList::instance().menuViewCameraMovetoPreviousPointInfo);
            connect(viewMoveCameraToPreviousPointAction, &QAction::triggered, this, &MapFrame::OnViewMoveCameraToPreviousPoint);

            viewFocusCameraOnSelectionAction = new QAction("Focus on Selection", this);
            registerBinding(viewFocusCameraOnSelectionAction, ActionList::instance().menuViewCameraFocusonSelectionInfo);
            connect(viewFocusCameraOnSelectionAction, &QAction::triggered, this, &MapFrame::OnViewFocusCameraOnSelection);

            viewMoveCameraToPositionAction = new QAction("Move Camera to...", this);
            registerBinding(viewMoveCameraToPositionAction, ActionList::instance().menuViewCameraMoveCameratoInfo);
            connect(viewMoveCameraToPositionAction, &QAction::triggered, this, &MapFrame::OnViewMoveCameraToPosition);

            viewHideSelectionAction = new QAction("Hide", this);
            registerBinding(viewHideSelectionAction, ActionList::instance().menuViewHideInfo);
            connect(viewHideSelectionAction, &QAction::triggered, this, &MapFrame::OnViewHideSelectedObjects);

            viewIsolateSelectionAction = new QAction("Isolate", this);
            registerBinding(viewIsolateSelectionAction, ActionList::instance().menuViewIsolateInfo);
            connect(viewIsolateSelectionAction, &QAction::triggered, this, &MapFrame::OnViewIsolateSelectedObjects);

            viewUnhideAllAction = new QAction("Show All", this);
            registerBinding(viewUnhideAllAction, ActionList::instance().menuViewShowAllInfo);
            connect(viewUnhideAllAction, &QAction::triggered, this, &MapFrame::OnViewShowHiddenObjects);

            viewSwitchToMapInspectorAction = new QAction("Switch to Map Inspector", this);
            registerBinding(viewSwitchToMapInspectorAction, ActionList::instance().menuViewSwitchtoMapInspectorInfo);
            connect(viewSwitchToMapInspectorAction, &QAction::triggered, this, &MapFrame::OnViewSwitchToMapInspector);

            viewSwitchToEntityInspectorAction = new QAction("Switch to Entity Inspector", this);
            registerBinding(viewSwitchToEntityInspectorAction, ActionList::instance().menuViewSwitchtoEntityInspectorInfo);
            connect(viewSwitchToEntityInspectorAction, &QAction::triggered, this, &MapFrame::OnViewSwitchToEntityInspector);

            viewSwitchToFaceInspectorAction = new QAction("Switch to Face Inspector", this);
            registerBinding(viewSwitchToFaceInspectorAction, ActionList::instance().menuViewSwitchtoFaceInspectorInfo);
            connect(viewSwitchToFaceInspectorAction, &QAction::triggered, this, &MapFrame::OnViewSwitchToFaceInspector);

            viewToggleMaximizeCurrentViewAction = new QAction("Maximize Current View", this);
            registerBinding(viewToggleMaximizeCurrentViewAction, ActionList::instance().menuViewMaximizeCurrentViewInfo);
            connect(viewToggleMaximizeCurrentViewAction, &QAction::triggered, this, &MapFrame::OnViewToggleMaximizeCurrentView);

            viewPreferencesAction = new QAction("Preferences...", this);
            registerBinding(viewPreferencesAction, ActionList::instance().menuViewPreferencesInfo);
            connect(viewPreferencesAction, &QAction::triggered, &TrenchBroomApp::instance(), &TrenchBroomApp::OnOpenPreferences);

            viewToggleInfoPanelAction = new QAction("Toggle Info Panel", this);
            registerBinding(viewToggleInfoPanelAction, ActionList::instance().menuViewToggleInfoPanelInfo);
            viewToggleInfoPanelAction->setCheckable(true);
            connect(viewToggleInfoPanelAction, &QAction::triggered, this, &MapFrame::OnViewToggleInfoPanel);

            viewToggleInspectorAction = new QAction("Toggle Inspector", this);
            registerBinding(viewToggleInspectorAction, ActionList::instance().menuViewToggleInspectorInfo);
            viewToggleInspectorAction->setCheckable(true);
            connect(viewToggleInspectorAction, &QAction::triggered, this, &MapFrame::OnViewToggleInspector);

            runCompileAction = new QAction("Compile...", this);
            registerBinding(runCompileAction, ActionList::instance().menuRunCompileInfo);
            connect(runCompileAction, &QAction::triggered, this, &MapFrame::OnRunCompile);

            runLaunchAction = new QAction("Launch...", this);
            registerBinding(runLaunchAction, ActionList::instance().menuRunLaunchInfo);
            connect(runLaunchAction, &QAction::triggered, this, &MapFrame::OnRunLaunch);

            debugPrintVerticesAction = new QAction("Print Vertices", this);
            registerBinding(debugPrintVerticesAction, ActionList::instance().menuDebugPrintVerticesInfo);
            connect(debugPrintVerticesAction, &QAction::triggered, this, &MapFrame::OnDebugPrintVertices);

            debugCreateBrushAction = new QAction("Create Brush...", this);
            registerBinding(debugCreateBrushAction, ActionList::instance().menuDebugCreateBrushInfo);
            connect(debugCreateBrushAction, &QAction::triggered, this, &MapFrame::OnDebugCreateBrush);

            debugCreateCubeAction = new QAction("Create Cube...", this);
            registerBinding(debugCreateCubeAction, ActionList::instance().menuDebugCreateCubeInfo);
            connect(debugCreateCubeAction, &QAction::triggered, this, &MapFrame::OnDebugCreateCube);

            debugClipWithFaceAction = new QAction("Clip Brush...", this);
            registerBinding(debugClipWithFaceAction, ActionList::instance().menuDebugClipBrushInfo);
            connect(debugClipWithFaceAction, &QAction::triggered, this, &MapFrame::OnDebugClipBrush);

            debugCopyJSShortcutsAction = new QAction("Copy Javascript Shortcut Map", this);
            registerBinding(debugCopyJSShortcutsAction, ActionList::instance().menuDebugCopyJavascriptShortcutMapInfo);
            connect(debugCopyJSShortcutsAction, &QAction::triggered, this, &MapFrame::OnDebugCopyJSShortcutMap);

            debugCrashAction = new QAction("Crash...", this);
            registerBinding(debugCrashAction, ActionList::instance().menuDebugCrashInfo);
            connect(debugCrashAction, &QAction::triggered, this, &MapFrame::OnDebugCrash);

            debugThrowExceptionDuringCommandAction = new QAction("Throw Exception During Command", this);
            registerBinding(debugThrowExceptionDuringCommandAction, ActionList::instance().menuDebugThrowExceptionDuringCommandInfo);
            connect(debugThrowExceptionDuringCommandAction, &QAction::triggered, this, &MapFrame::OnDebugThrowExceptionDuringCommand);

            debugCrashReportDialogAction = new QAction("Show Crash Report Dialog", this);
            registerBinding(debugCrashReportDialogAction, ActionList::instance().menuDebugShowCrashReportDialogInfo);
            connect(viewPreferencesAction, &QAction::triggered, &TrenchBroomApp::instance(), &TrenchBroomApp::OnDebugShowCrashReportDialog);

            debugSetWindowSizeAction = new QAction("Set Window Size...", this);
            registerBinding(debugSetWindowSizeAction, ActionList::instance().menuDebugSetWindowSizeInfo);
            connect(debugSetWindowSizeAction, &QAction::triggered, this, &MapFrame::OnDebugSetWindowSize);

            helpManualAction = new QAction("TrenchBroom Manual", this);
            registerBinding(helpManualAction, ActionList::instance().menuHelpTrenchBroomManualInfo);
            connect(helpManualAction, &QAction::triggered, &TrenchBroomApp::instance(), &TrenchBroomApp::OnHelpShowManual);

            helpAboutAction = new QAction("About TrenchBroom", this);
            registerBinding(helpAboutAction, ActionList::instance().menuHelpAboutTrenchBroomInfo);
            connect(helpAboutAction, &QAction::triggered, &TrenchBroomApp::instance(), &TrenchBroomApp::OnOpenAbout);

            flipObjectsHorizontallyAction = new QAction("Flip Horizontally", this);
            flipObjectsHorizontallyAction->setIcon(IO::loadIconResourceQt(IO::Path("FlipHorizontally.png")));
            connect(flipObjectsHorizontallyAction, &QAction::triggered, this, &MapFrame::OnFlipObjectsHorizontally);

            flipObjectsVerticallyAction = new QAction("Flip Vertically", this);
            flipObjectsVerticallyAction->setIcon(IO::loadIconResourceQt(IO::Path("FlipVertically.png")));
            connect(flipObjectsVerticallyAction, &QAction::triggered, this, &MapFrame::OnFlipObjectsVertically);
        }

        void MapFrame::registerBinding(QAction *action, const ActionInfo &info) {
            m_actionInfoList.emplace_back(std::make_pair(action, &info));
		}

		void MapFrame::updateBindings() {
            // set up bindings
            for (auto [action, menuInfo] : m_actionInfoList) {
                qDebug("found path %s, binding: %s",
                       menuInfo->preferencePath.asString().c_str(),
                       menuInfo->key().toString(QKeySequence::NativeText).toStdString().c_str());

                action->setShortcut(menuInfo->key());
            }
		}

        void MapFrame::createMenus() {
            QMenu* fileMenu = menuBar()->addMenu("File");
            fileMenu->addAction(fileNewAction);// addUnmodifiableActionItem(wxID_NEW, "New", KeyboardShortcut('N', WXK_CONTROL));
            fileMenu->addSeparator();
            fileMenu->addAction(fileOpenAction);// UnmodifiableActionItem(wxID_OPEN, "Open...", KeyboardShortcut('O', WXK_CONTROL));
            QMenu* openRecentMenu = fileMenu->addMenu("Open Recent");
            // FIXME: implement recents
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
            const int gridSizeIndex = indexForGridSize(m_document->grid().size());

            for (int i = 0; i < numActions; ++i) {
                actions[i]->setChecked(i == gridSizeIndex);
            }

            viewIncGridSizeAction->setEnabled(canIncGridSize());
            viewDecGridSizeAction->setEnabled(canDecGridSize());

            // Update toolbar
            m_gridChoice->setCurrentIndex(indexForGridSize(m_document->grid().size()));
        }

        void MapFrame::updateToolActions() {
            editDeactivateToolAction->setEnabled(true);
            editDeactivateToolAction->setChecked(!m_mapView->anyToolActive());

            editToggleCreateComplexBrushToolAction->setEnabled(m_mapView->canToggleCreateComplexBrushTool());
            editToggleCreateComplexBrushToolAction->setChecked(m_mapView->createComplexBrushToolActive());

            editToggleClipToolAction->setEnabled(m_mapView->canToggleClipTool());
            editToggleClipToolAction->setChecked(m_mapView->clipToolActive());

            editToggleRotateObjectsToolAction->setEnabled(m_mapView->canToggleRotateObjectsTool());
            editToggleRotateObjectsToolAction->setChecked(m_mapView->rotateObjectsToolActive());

            editToggleScaleObjectsToolAction->setEnabled(m_mapView->canToggleScaleObjectsTool());
            editToggleScaleObjectsToolAction->setChecked(m_mapView->scaleObjectsToolActive());

            editToggleShearObjectsToolAction->setEnabled(m_mapView->canToggleShearObjectsTool());
            editToggleShearObjectsToolAction->setChecked(m_mapView->shearObjectsToolActive());

            editToggleVertexToolAction->setEnabled(m_mapView->canToggleVertexTools());
            editToggleVertexToolAction->setChecked(m_mapView->vertexToolActive());

            editToggleEdgeToolAction->setEnabled(m_mapView->canToggleVertexTools());
            editToggleEdgeToolAction->setChecked(m_mapView->edgeToolActive());

            editToggleFaceToolAction->setEnabled(m_mapView->canToggleVertexTools());
            editToggleFaceToolAction->setChecked(m_mapView->faceToolActive());
		}

		void MapFrame::updateOtherActions() {
		    // FIXME: MapDocument::persistent() does disk IO - don't do any IO in here

		    fileReloadPointFileAction->setEnabled(canReloadPointFile());
            fileUnloadPointFileAction->setEnabled(canUnloadPointFile());
            fileReloadPortalFileAction->setEnabled(canReloadPortalFile());
            fileUnloadPortalFileAction->setEnabled(canUnloadPortalFile());

            editCutAction->setEnabled(canCut());
            editCopyAction->setEnabled(canCopy());
            // For paste actions, see updateClipboardActions()
            editDuplicateAction->setEnabled(canDuplicate());
            editDeleteAction->setEnabled(canDelete());
            editSelectAllAction->setEnabled(canSelect());
            editSelectSiblingsAction->setEnabled(canSelectSiblings());
            editSelectTouchingAction->setEnabled(canSelectByBrush());
            editSelectInsideAction->setEnabled(canSelectByBrush());
            editSelectTallAction->setEnabled(canSelectTall());
            editSelectByLineNumberAction->setEnabled(canSelect());
            editSelectNoneAction->setEnabled(canDeselect());
            editGroupSelectedObjectsAction->setEnabled(canGroup());
            editUngroupSelectedObjectsAction->setEnabled(canUngroup());
            editCsgConvexMergeAction->setEnabled(canDoCsgConvexMerge());
            editCsgSubtractAction->setEnabled(canDoCsgSubtract());
            editCsgIntersectAction->setEnabled(canDoCsgIntersect());
            editCsgHollowAction->setEnabled(canDoCsgHollow());
            editReplaceTextureAction->setEnabled(true);
            editToggleTextureLockAction->setEnabled(true);
            editToggleTextureLockAction->setChecked(pref(Preferences::TextureLock));
            editToggleUVLockAction->setEnabled(true);
            editToggleUVLockAction->setChecked(pref(Preferences::UVLock));
            editSnapVerticesToIntegerAction->setEnabled(canSnapVertices());
            editSnapVerticesToGridAction->setEnabled(canSnapVertices());

            viewMoveCameraToNextPointAction->setEnabled(canMoveCameraToNextPoint());
            viewMoveCameraToPreviousPointAction->setEnabled(canMoveCameraToPreviousPoint());
            viewFocusCameraOnSelectionAction->setEnabled(canFocusCamera());
            viewMoveCameraToPositionAction->setEnabled(true);
            viewIsolateSelectionAction->setEnabled(canIsolate());
            viewHideSelectionAction->setEnabled(canHide());
            viewUnhideAllAction->setEnabled(true);
            viewSwitchToMapInspectorAction->setEnabled(true);
            viewSwitchToEntityInspectorAction->setEnabled(true);
            viewSwitchToFaceInspectorAction->setEnabled(true);
            viewToggleInfoPanelAction->setEnabled(true);
            viewToggleInfoPanelAction->setChecked(m_infoPanel->isVisible());
            viewToggleInspectorAction->setEnabled(true);
            viewToggleInspectorAction->setChecked(m_inspector->isVisible());
            viewToggleMaximizeCurrentViewAction->setEnabled(m_mapView->canMaximizeCurrentView());
            viewToggleMaximizeCurrentViewAction->setChecked(m_mapView->currentViewMaximized());
            viewPreferencesAction->setEnabled(true);
            runCompileAction->setEnabled(canCompile());
            runLaunchAction->setEnabled(canLaunch());
            debugPrintVerticesAction->setEnabled(true);
            debugCreateBrushAction->setEnabled(true);
            debugCreateCubeAction->setEnabled(true);
            debugClipWithFaceAction->setEnabled(m_document->selectedNodes().hasOnlyBrushes());
            debugCopyJSShortcutsAction->setEnabled(true);
            debugCrashAction->setEnabled(true);
            debugThrowExceptionDuringCommandAction->setEnabled(true);
            debugCrashReportDialogAction->setEnabled(true);
            debugSetWindowSizeAction->setEnabled(true);
            helpManualAction->setEnabled(true);
            helpAboutAction->setEnabled(true);
            flipObjectsHorizontallyAction->setEnabled(m_mapView->canFlipObjects());
            flipObjectsVerticallyAction->setEnabled(m_mapView->canFlipObjects());
		}

		void MapFrame::updateUndoRedoActions() {
            // FIXME:
		}

        void MapFrame::updateClipboardActions() {
            const bool paste = canPaste();
            editPasteAction->setEnabled(paste);
            editPasteAtOriginalPositionAction->setEnabled(paste);
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
            if (m_document->path().isAbsolute()) {
                View::TrenchBroomApp::instance().updateRecentDocument(m_document->path());
            }
        }

        void MapFrame::createGui() {
            setWindowIconTB(this);
            setWindowTitle("TrenchBroom");

            // FIXME: handle sash gravity, persistence
            m_hSplitter = new QSplitter(Qt::Horizontal);
            m_hSplitter->setChildrenCollapsible(false);
            //m_hSplitter->SetName("MapFrameHSplitter");

            m_vSplitter = new QSplitter(Qt::Vertical);
            m_vSplitter->setChildrenCollapsible(false);
            //m_vSplitter->SetName("MapFrameVSplitter");

            m_infoPanel = new InfoPanel(nullptr, m_document);
            m_console = m_infoPanel->console();

            m_mapView = new SwitchableMapViewContainer(nullptr, m_console, m_document, *m_contextManager);

            m_inspector = new Inspector(nullptr, m_document, *m_contextManager);

            m_mapView->connectTopWidgets(m_inspector);

            // Add widgets to splitters
            m_vSplitter->addWidget(m_mapView);
            m_vSplitter->addWidget(m_infoPanel);

            m_hSplitter->addWidget(m_vSplitter);
            m_hSplitter->addWidget(m_inspector);

            // Configure minimum sizes
            m_mapView->setMinimumSize(100, 100);
            m_infoPanel->setMinimumSize(100, 100);

            m_vSplitter->setMinimumSize(100, 100);
            m_inspector->setMinimumSize(350, 100);

            // Configure the sash gravity so the first widget gets most of the space
            m_hSplitter->setSizes(QList<int>{1'000'000, 1});
            m_vSplitter->setSizes(QList<int>{1'000'000, 1});

            QVBoxLayout* frameSizer = new QVBoxLayout();
            frameSizer->setContentsMargins(0, 0, 0, 0);
            frameSizer->setSpacing(0); // no space between BorderLine and m_hSplitter
#if !defined __APPLE__
            frameSizer->addWidget(new BorderLine());
#endif
            frameSizer->addWidget(m_hSplitter);

            // FIXME:
//            wxPersistenceManager::Get().RegisterAndRestore(m_hSplitter);
//            wxPersistenceManager::Get().RegisterAndRestore(m_vSplitter);

            // NOTE: you can't set the layout of a QMainWindow, so make another widget to wrap this layout in
            QWidget* layoutWrapper = new QWidget();
            layoutWrapper->setLayout(frameSizer);

            setCentralWidget(layoutWrapper);
        }

        void MapFrame::createToolBar() {
		    QToolBar* toolBar = addToolBar("Toolbar");
		    toolBar->setFloatable(false);
		    toolBar->setMovable(false);
		    toolBar->addAction(editDeactivateToolAction);
            toolBar->addAction(editToggleCreateComplexBrushToolAction);
            toolBar->addAction(editToggleClipToolAction);
            toolBar->addAction(editToggleVertexToolAction);
            toolBar->addAction(editToggleEdgeToolAction);
            toolBar->addAction(editToggleFaceToolAction);
            toolBar->addAction(editToggleRotateObjectsToolAction);
            toolBar->addAction(editToggleScaleObjectsToolAction);
            toolBar->addAction(editToggleShearObjectsToolAction);
            toolBar->addSeparator();
            toolBar->addAction(editDuplicateAction);
            toolBar->addAction(flipObjectsHorizontallyAction);
            toolBar->addAction(flipObjectsVerticallyAction);
            toolBar->addSeparator();
            toolBar->addAction(editToggleTextureLockAction);
            toolBar->addAction(editToggleUVLockAction);
            toolBar->addSeparator();

            const QString gridSizes[12] = { "Grid 0.125", "Grid 0.25", "Grid 0.5", "Grid 1", "Grid 2", "Grid 4", "Grid 8", "Grid 16", "Grid 32", "Grid 64", "Grid 128", "Grid 256" };
            m_gridChoice = new QComboBox();
            for (int i = 0; i < 12; ++i) {
                m_gridChoice->addItem(gridSizes[i], QVariant(gridSizeForIndex(i)));
            }
            toolBar->addWidget(m_gridChoice);
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

        static QString describeSelection(const MapDocument* document) {
            const QString DblArrow = QString(" ") + QString(QChar(0x00BB)) + QString(" ");
            const QString Arrow = QString(" ") + QString(QChar(0x203A)) + QString(" ");

            QString result;

            // current layer
            result += QString::fromStdString(document->currentLayer()->name()) + DblArrow;

            // open groups
            std::list<Model::Group*> groups;
            for (Model::Group* group = document->currentGroup(); group != nullptr; group = group->group()) {
                groups.push_front(group);
            }
            for (Model::Group* group : groups) {
                result += QString::fromStdString(group->name()) + Arrow;
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
            result += QString::fromStdString(StringUtils::join(tokens, ", ", ", and ", " and ")) + " selected";

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

            m_mapView->mapViewToolBox()->toolActivatedNotifier.addObserver(this, &MapFrame::toolActivated);
            m_mapView->mapViewToolBox()->toolDeactivatedNotifier.addObserver(this, &MapFrame::toolDeactivated);
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

            m_mapView->mapViewToolBox()->toolActivatedNotifier.removeObserver(this, &MapFrame::toolActivated);
            m_mapView->mapViewToolBox()->toolDeactivatedNotifier.removeObserver(this, &MapFrame::toolDeactivated);
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
            if (path == Preferences::MapViewLayout.path()) {
                m_mapView->switchToMapView(static_cast<MapViewLayout>(pref(Preferences::MapViewLayout)));
            }
        }

        void MapFrame::gridDidChange() {
            const Grid& grid = m_document->grid();
            updateGridActions();
        }

        void MapFrame::toolActivated(Tool* tool) {
		    updateToolActions();
		    updateOtherActions();
		}

        void MapFrame::toolDeactivated(Tool* tool) {
            updateToolActions();
            updateOtherActions();
        }

        void MapFrame::selectionDidChange(const Selection& selection) {
            updateStatusBar();
            updateToolActions();
            updateOtherActions();
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

            connect(qApp, &QApplication::focusChanged, this, &MapFrame::onFocusChange);
            connect(m_gridChoice, QOverload<int>::of(&QComboBox::activated), this, &MapFrame::OnToolBarSetGridSize);

            connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &MapFrame::updateClipboardActions);
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
                logger().error("Clipboard is empty");
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
                const auto string = QInputDialog::getText(this, "Select by Line Numbers", "Enter a comma- or space separated list of line numbers.");
                if (string.isEmpty())
                    return;

                std::vector<size_t> positions;
                for (const QString& token : string.split(", ")) {
                    bool ok;
                    long position = token.toLong(&ok);
                    if (ok && position > 0) {
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
                if (m_mapView->vertexToolActive() && m_mapView->vertexTool()->canDoCsgConvexMerge()) {
                    m_mapView->vertexTool()->csgConvexMerge();
                } else if (m_mapView->edgeToolActive() && m_mapView->edgeTool()->canDoCsgConvexMerge()) {
                    m_mapView->edgeTool()->csgConvexMerge();
                } else if (m_mapView->faceToolActive() && m_mapView->faceTool()->canDoCsgConvexMerge()) {
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
        }

        void MapFrame::OnEditToggleUVLock() {
            PreferenceManager::instance().set(Preferences::UVLock, !pref(Preferences::UVLock));
            PreferenceManager::instance().saveChanges();
        }

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
            switchToInspectorPage(Inspector::InspectorPage_Map);
        }

        void MapFrame::OnViewSwitchToEntityInspector() {
            switchToInspectorPage(Inspector::InspectorPage_Entity);
        }

        void MapFrame::OnViewSwitchToFaceInspector() {
            switchToInspectorPage(Inspector::InspectorPage_Face);
        }

        void MapFrame::switchToInspectorPage(const Inspector::InspectorPage page) {
            m_inspector->show();
            m_inspector->switchToPage(page);
        }

        void MapFrame::OnViewToggleMaximizeCurrentView() {
            m_mapView->toggleMaximizeCurrentView();
        }

        void MapFrame::OnViewToggleInfoPanel() {
            m_infoPanel->setHidden(!m_infoPanel->isHidden());
        }

        void MapFrame::OnViewToggleInspector() {
            m_inspector->setHidden(!m_inspector->isHidden());
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

            // FIXME: reimplement
//            const String str = ActionManager::instance().getJSTable();
//            clipboard->setText(QString::fromStdString(str));
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
        void MapFrame::OnUpdateUI() {
		    // FIXME: implement

            const auto& actionManager = ActionManager::instance();
            const auto& grid = m_document->grid();

            switch (event.GetId()) {

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
                case CommandIds::Menu::ViewToggleShowGrid:
                    event.Enable(true);
                    event.Check(grid.visible());
                    break;
                case CommandIds::Menu::ViewToggleSnapToGrid:
                    event.Enable(true);
                    event.Check(grid.snap());
                    break;
                case CommandIds::Menu::ViewIncGridSize:
                    event.Enable(canIncGridSize());
                    break;
                case CommandIds::Menu::ViewDecGridSize:
                    event.Enable(canDecGridSize());
                    break;
                case CommandIds::Menu::ViewSetGridSize0Point125:
                    event.Enable(true);
                    event.Check(grid.size() == -3);
                    break;
                case CommandIds::Menu::ViewSetGridSize0Point25:
                    event.Enable(true);
                    event.Check(grid.size() == -2);
                    break;
                case CommandIds::Menu::ViewSetGridSize0Point5:
                    event.Enable(true);
                    event.Check(grid.size() == -1);
                    break;
                case CommandIds::Menu::ViewSetGridSize1:
                    event.Enable(true);
                    event.Check(grid.size() == 0);
                    break;
                case CommandIds::Menu::ViewSetGridSize2:
                    event.Enable(true);
                    event.Check(grid.size() == 1);
                    break;
                case CommandIds::Menu::ViewSetGridSize4:
                    event.Enable(true);
                    event.Check(grid.size() == 2);
                    break;
                case CommandIds::Menu::ViewSetGridSize8:
                    event.Enable(true);
                    event.Check(grid.size() == 3);
                    break;
                case CommandIds::Menu::ViewSetGridSize16:
                    event.Enable(true);
                    event.Check(grid.size() == 4);
                    break;
                case CommandIds::Menu::ViewSetGridSize32:
                    event.Enable(true);
                    event.Check(grid.size() == 5);
                    break;
                case CommandIds::Menu::ViewSetGridSize64:
                    event.Enable(true);
                    event.Check(grid.size() == 6);
                    break;
                case CommandIds::Menu::ViewSetGridSize128:
                    event.Enable(true);
                    event.Check(grid.size() == 7);
                    break;
                case CommandIds::Menu::ViewSetGridSize256:
                    event.Enable(true);
                    event.Check(grid.size() == 8);
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

        void MapFrame::OnToolBarSetGridSize(const int index) {
            m_document->grid().setSize(gridSizeForIndex(index));
        }

        void MapFrame::onFocusChange(QWidget* old, QWidget* now) {
            updateOtherActions();
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

        /**
         * This is relatively expensive so only call it when the clipboard changes or e.g. the user tries to paste.
         */
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
    }
}
