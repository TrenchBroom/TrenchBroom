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
#include "View/Actions.h"
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
        m_gridChoice(nullptr),
        m_statusBarLabel(nullptr),
        m_recentDocumentsMenu(nullptr),
        m_undoAction(nullptr),
        m_redoAction(nullptr),
        m_pasteAction(nullptr),
        m_pasteAtOriginalPositionAction(nullptr) {}

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
        m_gridChoice(nullptr),
        m_statusBarLabel(nullptr),
        m_recentDocumentsMenu(nullptr),
        m_undoAction(nullptr),
        m_redoAction(nullptr),
        m_pasteAction(nullptr),
        m_pasteAtOriginalPositionAction(nullptr) {
            setAttribute(Qt::WA_DeleteOnClose);

            ensure(frameManager != nullptr, "frameManager is null");
            ensure(document.get() != nullptr, "document is null");

            m_frameManager = frameManager;
            m_document = document;
            m_autosaver = new Autosaver(m_document);

            m_contextManager = new GLContextManager();

            createGui();
            createMenus();
            createToolBar();
            createStatusBar();

            updateShortcuts();
            updateActionState();
            updateUndoRedoActions();

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

        void MapFrame::updateTitle() {
            setWindowModified(m_document->modified());
            setWindowTitle(QString::fromStdString(m_document->filename()) + QString("[*] - TrenchBroom"));
            setWindowFilePath(QString::fromStdString(m_document->path().asString()));
        }

        class MapFrame::MenuBuilder : public MenuVisitor {
        private:
            MapFrame* m_frame;
            ActionMap& m_actionMap;
            QMenuBar* m_menuBar;
            QMenu* m_currentMenu;
        public:
            explicit MenuBuilder(MapFrame* frame, ActionMap& actionMap) :
            m_frame(frame),
            m_actionMap(actionMap),
            m_menuBar(m_frame->menuBar()),
            m_currentMenu(nullptr) {
                assert(m_frame != nullptr);
                assert(m_menuBar != nullptr);
            }

            void visit(const Menu& menu) override {
                auto* parentMenu = m_currentMenu;
                if (m_currentMenu == nullptr) {
                    // top level menu
                    m_currentMenu = m_menuBar->addMenu(QString::fromStdString(menu.name()));
                } else {
                    m_currentMenu = m_currentMenu->addMenu(QString::fromStdString(menu.name()));
                }

                if (menu.entryType() == MenuEntryType::Menu_RecentDocuments) {
                    m_frame->m_recentDocumentsMenu = m_currentMenu;
                }

                menu.visitEntries(*this);
                m_currentMenu = parentMenu;
            }

            void visit(const MenuSeparatorItem& item) override {
                assert(m_currentMenu != nullptr);
                m_currentMenu->addSeparator();
            }

            void visit(const MenuActionItem& item) override {
                assert(m_currentMenu != nullptr);
                const auto& tAction = item.action();
                auto* qAction = m_currentMenu->addAction(QString::fromStdString(tAction.name()));
                qAction->setCheckable(tAction.checkable());

                auto* frame = m_frame;
                connect(qAction, &QAction::triggered, m_frame, [frame, &tAction]() { frame->triggerAction(tAction); });
                m_actionMap.emplace_back(qAction, &tAction);

                if (item.entryType() == MenuEntryType::Menu_Undo) {
                    m_frame->m_undoAction = qAction;
                } else if (item.entryType() == MenuEntryType::Menu_Redo) {
                    m_frame->m_redoAction = qAction;
                } else if (item.entryType() == MenuEntryType::Menu_Paste) {
                    m_frame->m_pasteAction = qAction;
                } else if (item.entryType() == MenuEntryType::Menu_PasteAtOriginalPosition) {
                    m_frame->m_pasteAtOriginalPositionAction = qAction;
                }
            }
        };

        void MapFrame::createMenus() {
            MenuBuilder menuBuilder(this, m_actionMap);
            const auto& actionManager = ActionManager::instance();
            actionManager.visitMainMenu(menuBuilder);
        }

        void MapFrame::updateShortcuts() {
            for (auto [qAction, tAction] : m_actionMap) {
                qAction->setShortcut(tAction->keySequence());
            }
        }

        void MapFrame::updateActionState() {
            // FIXME: Do we need to do this more fine grained? Right now we just update all actions whenever anything
            // changes.
            ActionExecutionContext context(this, nullptr);
            for (auto [qAction, tAction] : m_actionMap) {
                if (qAction == m_undoAction || qAction == m_redoAction ||
                    qAction == m_pasteAction || qAction == m_pasteAtOriginalPositionAction) {
                    // These are handled specially for performance reasons.
                    continue;
                }
                qAction->setEnabled(tAction->enabled(context));
                if (qAction->isCheckable()) {
                    qAction->setChecked(tAction->checked(context));
                }
            }
        }

        void MapFrame::updateUndoRedoActions() {
            const auto document = lock(m_document);
            if (m_undoAction != nullptr) {
                if (document->canUndoLastCommand()) {
                    const auto text = "Undo " + document->lastCommandName();
                    m_undoAction->setText(QString::fromStdString(text));
                    m_undoAction->setEnabled(true);
                } else {
                    m_undoAction->setText("Undo");
                    m_undoAction->setEnabled(false);
                }
            }
            if (m_redoAction != nullptr) {
                if (document->canRedoNextCommand()) {
                    const auto text = "Redo " + document->nextCommandName();
                    m_redoAction->setText(QString::fromStdString(text));
                    m_redoAction->setEnabled(true);
                } else {
                    m_redoAction->setText("Redo");
                    m_redoAction->setEnabled(false);
                }
            }
        }

        void MapFrame::updatePasteActions() {
            const auto enable = canPaste();
            if (m_pasteAction != nullptr) {
                m_pasteAction->setEnabled(enable);
            }
            if (m_pasteAtOriginalPositionAction != nullptr) {
                m_pasteAtOriginalPositionAction->setEnabled(enable);
            }
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

        class MapFrame::ToolBarBuilder : public MenuVisitor {
        private:
            MapFrame* m_frame;
            ActionMap& m_actionMap;
            QToolBar* m_toolBar;
        public:
            explicit ToolBarBuilder(MapFrame* frame, ActionMap& actionMap, QToolBar* toolBar) :
                m_frame(frame),
                m_actionMap(actionMap),
                m_toolBar(toolBar) {
                assert(m_frame != nullptr);
                assert(m_toolBar != nullptr);
            }

            void visit(const Menu& menu) override {
                menu.visitEntries(*this);
            }

            void visit(const MenuSeparatorItem& item) override {
                m_toolBar->addSeparator();
            }

            void visit(const MenuActionItem& item) override {
                const auto& tAction = item.action();
                auto* qAction = m_toolBar->addAction(QString::fromStdString(tAction.name()));
                qAction->setChecked(tAction.checkable());
                if (tAction.hasIcon()) {
                    qAction->setIcon(IO::loadIconResourceQt(tAction.iconPath()));
                }

                auto* frame = m_frame;
                connect(qAction, &QAction::triggered, m_frame, [frame, &tAction]() { frame->triggerAction(tAction); });
                m_actionMap.emplace_back(qAction, &tAction);
            }
        };

        void MapFrame::createToolBar() {
            QToolBar* toolBar = addToolBar("Toolbar");
            toolBar->setFloatable(false);
            toolBar->setMovable(false);

            ToolBarBuilder builder(this, m_actionMap, toolBar);
            auto& actionManager = ActionManager::instance();
            actionManager.visitToolBarActions(builder);

            m_gridChoice = new QComboBox();
            for (int i = Grid::MinSize; i < Grid::MaxSize; ++i) {
                const auto gridSize = Grid::actualSize(i);
                const auto gridSizeStr = QString::number(gridSize, 'f', 3);
                m_gridChoice->addItem(gridSizeStr, QVariant(i));
            }

            toolBar->addSeparator();
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
            m_document->transactionDoneNotifier.addObserver(this, &MapFrame::transactionDone);
            m_document->transactionUndoneNotifier.addObserver(this, &MapFrame::transactionUndone);
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
            m_document->transactionDoneNotifier.removeObserver(this, &MapFrame::transactionDone);
            m_document->transactionUndoneNotifier.removeObserver(this, &MapFrame::transactionUndone);
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
            updateActionState();
        }

        void MapFrame::documentDidChange(View::MapDocument* document) {
            updateTitle();
            updateActionState();
            updateRecentDocumentsMenu();
        }

        void MapFrame::documentModificationStateDidChange() {
            updateTitle();
        }

        void MapFrame::transactionDone(const String& name) {
            updateUndoRedoActions();
        }

        void MapFrame::transactionUndone(const String& name) {
            updateUndoRedoActions();
        }

        void MapFrame::preferenceDidChange(const IO::Path& path) {
            if (path == Preferences::MapViewLayout.path()) {
                m_mapView->switchToMapView(static_cast<MapViewLayout>(pref(Preferences::MapViewLayout)));
            }
        }

        void MapFrame::gridDidChange() {
            updateActionState();
        }

        void MapFrame::toolActivated(Tool* tool) {
            updateActionState();
        }

        void MapFrame::toolDeactivated(Tool* tool) {
            updateActionState();
        }

        void MapFrame::selectionDidChange(const Selection& selection) {
            updateActionState();
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

            connect(qApp, &QApplication::focusChanged, this, &MapFrame::onFocusChange);
            connect(m_gridChoice, QOverload<int>::of(&QComboBox::activated), this, &MapFrame::OnToolBarSetGridSize);

            connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &MapFrame::updatePasteActions);
        }

        void MapFrame::triggerAction(const Action& action) {
            ActionExecutionContext context(this, nullptr);
            action.execute(context);
        }

        bool MapFrame::newDocument(Model::GameSPtr game, const Model::MapFormat mapFormat) {
            if (!confirmOrDiscardChanges()) {
                return false;
            }
            m_document->newDocument(mapFormat, MapDocument::DefaultWorldBounds, game);
            return true;
        }

        bool MapFrame::openDocument(Model::GameSPtr game, const Model::MapFormat mapFormat, const IO::Path& path) {
            if (!confirmOrDiscardChanges()) {
                return false;
            }
            m_document->loadDocument(mapFormat, MapDocument::DefaultWorldBounds, game, path);
            return true;
        }

        bool MapFrame::saveDocument() {
            try {
                if (m_document->persistent()) {
                    m_document->saveDocument();
                    logger().info() << "Saved " << m_document->path();
                    return true;
                } else {
                    return saveDocumentAs();
                }
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
                if (newFileName.isEmpty()) {
                    return false;
                }

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

        void MapFrame::loadPointFile() {
            QString defaultDir;
            if (!m_document->path().isEmpty()) {
                defaultDir = QString::fromStdString(m_document->path().deleteLastComponent().asString());
            }

            const QString fileName = QFileDialog::getOpenFileName(this, "Load Point File", defaultDir, "Point files (*.pts);;Any files (*.*)");

            if (!fileName.isEmpty()) {
                m_document->loadPointFile(IO::Path(fileName.toStdString()));
            }
        }

        void MapFrame::reloadPointFile() {
            if (canReloadPointFile()) {
                m_document->reloadPointFile();
            }
        }

        void MapFrame::unloadPointFile() {
            if (canUnloadPointFile())
                m_document->unloadPointFile();
        }


        bool MapFrame::canUnloadPointFile() const {
            return m_document->isPointFileLoaded();
        }

        bool MapFrame::canReloadPointFile() const {
            return m_document->canReloadPointFile();
        }

        void MapFrame::loadPortalFile() {
            QString defaultDir;
            if (!m_document->path().isEmpty()) {
                defaultDir = QString::fromStdString(m_document->path().deleteLastComponent().asString());
            }

            const QString fileName = QFileDialog::getOpenFileName(this, "Load Portal File", defaultDir, "Portal files (*.prt);;Any files (*.*)");

            if (!fileName.isEmpty()) {
                m_document->loadPortalFile(IO::Path(fileName.toStdString()));
            }
        }

        void MapFrame::reloadPortalFile() {
            if (canReloadPortalFile()) {
                m_document->reloadPortalFile();
            }
        }

        void MapFrame::unloadPortalFile() {
            if (canUnloadPortalFile()) {
                m_document->unloadPortalFile();
            }
        }

        bool MapFrame::canUnloadPortalFile() const {
            return m_document->isPortalFileLoaded();
        }

        bool MapFrame::canReloadPortalFile() const {
            return m_document->canReloadPortalFile();
        }

        void MapFrame::reloadTextureCollections() {
            m_document->reloadTextureCollections();
        }

        void MapFrame::reloadEntityDefinitions() {
            m_document->reloadEntityDefinitions();
        }

        void MapFrame::closeDocument() {
            close();
        }

        void MapFrame::undo() {
            if (canUndo()) {
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
        }

        void MapFrame::redo() {
            if (canRedo()) {
                // FIXME:
                auto textCtrl = nullptr; //findFocusedTextCtrl();
                if (textCtrl != nullptr) {
                    //textCtrl->Redo();
                } else {
                    m_document->redoNextCommand();
                }
            }
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

        bool MapFrame::canRedo() const {
            // FIXME:
            auto textCtrl = nullptr; //findFocusedTextCtrl();
            if (textCtrl != nullptr) {
                return true; // textCtrl->CanRedo();
            } else {
                return m_document->canRedoNextCommand();
            }
        }

#if 0
        wxTextCtrl* MapFrame::findFocusedTextCtrl() const {
            return nullptr;
            // FIXME:
//            return dynamic_cast<wxTextCtrl*>(FindFocus());
        }
#endif

        void MapFrame::repeatLastCommands() {
            m_document->repeatLastCommands();
        }

        void MapFrame::clearRepeatableCommands() {
            if (hasRepeatableCommands()) {
                m_document->clearRepeatableCommands();
            }
        }

        bool MapFrame::hasRepeatableCommands() const {
            return m_document->hasRepeatableCommands();
        }

        void MapFrame::cutSelection() {
            if (canCutSelection()) {
                copyToClipboard();
                Transaction transaction(m_document, "Cut");
                m_document->deleteObjects();
            }
        }

        void MapFrame::copySelection() {
            if (canCopySelection()) {
                copyToClipboard();
            }
        }

        void MapFrame::copyToClipboard() {
            QClipboard *clipboard = QApplication::clipboard();

            String str;
            if (m_document->hasSelectedNodes()) {
                str = m_document->serializeSelectedNodes();
            } else if (m_document->hasSelectedBrushFaces()) {
                str = m_document->serializeSelectedBrushFaces();
            }

            clipboard->setText(QString::fromStdString(str));
        }

        bool MapFrame::canCutSelection() const {
            return m_document->hasSelectedNodes() && !m_mapView->anyToolActive();
        }

        bool MapFrame::canCopySelection() const {
            return m_document->hasSelectedNodes() || m_document->hasSelectedBrushFaces();
        }

        void MapFrame::pasteAtCursorPosition() {
            if (canPaste()) {
                const vm::bbox3 referenceBounds = m_document->referenceBounds();
                Transaction transaction(m_document);
                if (paste() == PT_Node && m_document->hasSelectedNodes()) {
                    const vm::bbox3 bounds = m_document->selectionBounds();
                    const vm::vec3 delta = m_mapView->pasteObjectsDelta(bounds, referenceBounds);
                    m_document->translateObjects(delta);
                }
            }
        }

        void MapFrame::pasteAtOriginalPosition() {
            if (canPaste()) {
                paste();
            }
        }

        PasteType MapFrame::paste() {
            auto *clipboard = QApplication::clipboard();
            const auto qtext = clipboard->text();

            if (qtext.isEmpty()) {
                logger().error("Clipboard is empty");
                return PT_Failed;
            }

            return m_document->paste(qtext.toStdString());
        }

        /**
         * This is relatively expensive so only call it when the clipboard changes or e.g. the user tries to paste.
         */
        bool MapFrame::canPaste() const {
            if (!m_mapView->isCurrent()) {
                return false;
            }

            auto* clipboard = QApplication::clipboard();
            return !clipboard->text().isEmpty();
        }

        void MapFrame::duplicateSelection() {
            if (canDuplicateSelectino()) {
                m_document->duplicateObjects();
            }
        }

        bool MapFrame::canDuplicateSelectino() const {
            return m_document->hasSelectedNodes();
        }

        void MapFrame::deleteSelection() {
            if (canDeleteSelection()) {
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

        bool MapFrame::canDeleteSelection() const {
            if (m_mapView->clipToolActive()) {
                return m_mapView->clipTool()->canRemoveLastPoint();
            } else if (m_mapView->vertexToolActive()) {
                return m_mapView->vertexTool()->canRemoveSelection();
            } else if (m_mapView->edgeToolActive()) {
                return m_mapView->edgeTool()->canRemoveSelection();
            } else if (m_mapView->faceToolActive()) {
                return m_mapView->faceTool()->canRemoveSelection();
            } else {
                return canCutSelection();
            }
        }

        void MapFrame::selectAll() {
            if (canSelect()) {
                m_document->selectAllNodes();
            }
        }

        void MapFrame::selectSiblings() {
            if (canSelectSiblings()) {
                m_document->selectSiblings();
            }
        }

        void MapFrame::selectTouching() {
            if (canSelectByBrush()) {
                m_document->selectTouching(true);
            }
        }

        void MapFrame::selectInside() {
            if (canSelectByBrush()) {
                m_document->selectInside(true);
            }
        }

        void MapFrame::selectTall() {
            if (canSelectTall()) {
                m_mapView->selectTall();
            }
        }

        void MapFrame::selectByLineNumber() {
            if (canSelect()) {
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

        void MapFrame::selectNone() {
            if (canDeselect()) {
                m_document->deselectAll();
            }
        }

        bool MapFrame::canSelect() const {
            return canChangeSelection();
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

        bool MapFrame::canDeselect() const {
            return canChangeSelection() && m_document->hasSelectedNodes();
        }

        bool MapFrame::canChangeSelection() const {
            return m_document->editorContext().canChangeSelection();
        }

        void MapFrame::groupSelectedObjects() {
            if (canGroup()) {
                const String name = queryGroupName(this);
                if (!name.empty())
                    m_document->groupSelection(name);
            }
        }

        bool MapFrame::canGroup() const {
            return m_document->hasSelectedNodes() && !m_mapView->anyToolActive();
        }

        void MapFrame::ungroupSelectedObjects() {
            if (canUngroup()) {
                m_document->ungroupSelection();
            }
        }

        bool MapFrame::canUngroup() const {
            return m_document->selectedNodes().hasOnlyGroups() && !m_mapView->anyToolActive();
        }

        void MapFrame::replaceTexture() {
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

        void MapFrame::toggleCreateComplexBrushTool() {
            if (canToggleCreateComplexBrushTool()) {
                m_mapView->toggleCreateComplexBrushTool();
            }
        }

        bool MapFrame::canToggleCreateComplexBrushTool() const {
            return m_mapView->canToggleCreateComplexBrushTool();
        }

        bool MapFrame::createComplexBrushToolActive() const {
            return m_mapView->createComplexBrushToolActive();
        }

        void MapFrame::toggleClipTool() {
            if (canToggleClipTool()) {
                m_mapView->toggleClipTool();
            }
        }

        bool MapFrame::canToggleClipTool() const {
            return m_mapView->canToggleClipTool();
        }

        bool MapFrame::clipToolActive() const {
            return m_mapView->clipToolActive();
        }

        void MapFrame::toggleRotateObjectsTool() {
            if (canToggleRotateObjectsTool()) {
                m_mapView->toggleRotateObjectsTool();
            }
        }

        bool MapFrame::canToggleRotateObjectsTool() const {
            return m_mapView->canToggleRotateObjectsTool();
        }

        bool MapFrame::rotateObjectsToolActive() const {
            return m_mapView->rotateObjectsToolActive();
        }

        void MapFrame::toggleScaleObjectsTool() {
            if (canToggleScaleObjectsTool()) {
                m_mapView->toggleScaleObjectsTool();
            }
        }

        bool MapFrame::canToggleScaleObjectsTool() const {
            return m_mapView->canToggleScaleObjectsTool();
        }

        bool MapFrame::scaleObjectsToolActive() const {
            return m_mapView->scaleObjectsToolActive();
        }

        void MapFrame::toggleShearObjectsTool() {
            if (canToggleShearObjectsTool()) {
                m_mapView->toggleShearObjectsTool();
            }
        }

        bool MapFrame::canToggleShearObjectsTool() const {
            return m_mapView->canToggleShearObjectsTool();
        }

        bool MapFrame::shearObjectsToolActive() const {
            return m_mapView->shearObjectsToolActive();
        }

        void MapFrame::toggleVertexTool() {
            if (canToggleVertexTool()) {
                m_mapView->toggleVertexTool();
            }
        }

        bool MapFrame::canToggleVertexTool() const {
            return m_mapView->canToggleVertexTools();
        }

        bool MapFrame::vertexToolActive() const {
            return m_mapView->vertexToolActive();
        }

        void MapFrame::toggleEdgeTool() {
            if (canToggleEdgeTool()) {
                m_mapView->toggleEdgeTool();
            }
        }

        bool MapFrame::canToggleEdgeTool() const {
            return m_mapView->canToggleVertexTools();
        }

        bool MapFrame::edgeToolActive() const {
            return m_mapView->edgeToolActive();
        }

        void MapFrame::toggleFaceTool() {
            if (canToggleFaceTool()) {
                m_mapView->toggleFaceTool();
            }
        }

        bool MapFrame::canToggleFaceTool() const {
            return m_mapView->canToggleVertexTools();
        }

        bool MapFrame::faceToolActive() const {
            return m_mapView->faceToolActive();
        }

        void MapFrame::csgConvexMerge() {
            if (canDoCsgConvexMerge()) {
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

        bool MapFrame::canDoCsgConvexMerge() const {
            return (m_document->hasSelectedBrushFaces() && m_document->selectedBrushFaces().size() > 1) ||
                   (m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() > 1) ||
                   (m_mapView->vertexToolActive() && m_mapView->vertexTool()->canDoCsgConvexMerge()) ||
                   (m_mapView->edgeToolActive() && m_mapView->edgeTool()->canDoCsgConvexMerge()) ||
                   (m_mapView->faceToolActive() && m_mapView->faceTool()->canDoCsgConvexMerge());
        }

        void MapFrame::csgSubtract() {
            if (canDoCsgSubtract()) {
                m_document->csgSubtract();
            }
        }

        bool MapFrame::canDoCsgSubtract() const {
            return m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() >= 1;
        }

        void MapFrame::csgHollow() {
            if (canDoCsgHollow()) {
                m_document->csgHollow();
            }
        }

        bool MapFrame::canDoCsgHollow() const {
            return m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() >= 1;
        }

        void MapFrame::csgIntersect() {
            if (canDoCsgIntersect()) {
                m_document->csgIntersect();
            }
        }

        bool MapFrame::canDoCsgIntersect() const {
            return m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() > 1;
        }

        void MapFrame::snapVerticesToInteger() {
            if (canSnapVertices()) {
                m_document->snapVertices(1u);
            }
        }

        void MapFrame::snapVerticesToGrid() {
            if (canSnapVertices()) {
                m_document->snapVertices(m_document->grid().actualSize());
            }
        }

        bool MapFrame::canSnapVertices() const {
            return m_document->selectedNodes().hasOnlyBrushes();
        }

        void MapFrame::toggleTextureLock() {
            PreferenceManager::instance().set(Preferences::TextureLock, !pref(Preferences::TextureLock));
            PreferenceManager::instance().saveChanges();
        }

        void MapFrame::toggleUVLock() {
            PreferenceManager::instance().set(Preferences::UVLock, !pref(Preferences::UVLock));
            PreferenceManager::instance().saveChanges();
        }

        void MapFrame::toggleShowGrid() {
            m_document->grid().toggleVisible();
        }

        void MapFrame::toggleSnapToGrid() {
            m_document->grid().toggleSnap();
        }

        void MapFrame::incGridSize() {
            if (canIncGridSize()) {
                m_document->grid().incSize();
            }
        }

        bool MapFrame::canIncGridSize() const {
            return m_document->grid().size() < Grid::MaxSize;
        }

        void MapFrame::decGridSize() {
            if (canDecGridSize()) {
                m_document->grid().decSize();
            }
        }

        bool MapFrame::canDecGridSize() const {
            return m_document->grid().size() > Grid::MinSize;
        }

        void MapFrame::setGridSize(const int size) {
            m_document->grid().setSize(size);
        }

        void MapFrame::moveCameraToNextPoint() {
            if (canMoveCameraToNextPoint()) {
                m_mapView->moveCameraToNextTracePoint();
            }
        }

        bool MapFrame::canMoveCameraToNextPoint() const {
            return m_mapView->canMoveCameraToNextTracePoint();
        }

        void MapFrame::moveCameraToPreviousPoint() {
            if (canMoveCameraToPreviousPoint()) {
                m_mapView->moveCameraToPreviousTracePoint();
            }
        }

        bool MapFrame::canMoveCameraToPreviousPoint() const {
            return m_mapView->canMoveCameraToPreviousTracePoint();
        }

        void MapFrame::focusCameraOnSelection() {
            if (canFocusCamera()) {
                m_mapView->focusCameraOnSelection(true);
            }
        }

        bool MapFrame::canFocusCamera() const {
            return m_document->hasSelectedNodes();
        }

        void MapFrame::moveCameraToPosition() {
            bool ok = false;
            const QString str = QInputDialog::getText(this, "Move Camera", "Enter a position (x y z) for the camera.", QLineEdit::Normal, "0.0 0.0 0.0", &ok);
            if (ok) {
                const vm::vec3 position = vm::vec3::parse(str.toStdString());
                m_mapView->moveCameraToPosition(position, true);
            }
        }

        void MapFrame::isolateSelection() {
            if (canIsolateSelection()) {
                m_document->isolate(m_document->selectedNodes().nodes());
            }
        }

        bool MapFrame::canIsolateSelection() const {
            return m_document->hasSelectedNodes();
        }

        void MapFrame::hideSelection() {
            if (canHideSelection()) {
                m_document->hideSelection();
            }
        }

        bool MapFrame::canHideSelection() const {
            return m_document->hasSelectedNodes();
        }

        void MapFrame::showAll() {
            m_document->showAll();
        }

        void MapFrame::switchToInspectorPage(const Inspector::InspectorPage page) {
            m_inspector->show();
            m_inspector->switchToPage(page);
        }

        void MapFrame::toggleInfoPanel() {
            m_infoPanel->setHidden(!m_infoPanel->isHidden());
        }

        bool MapFrame::infoPanelVisible() const {
            return m_infoPanel->isVisible();
        }

        void MapFrame::toggleInspector() {
            m_inspector->setHidden(!m_inspector->isHidden());
        }

        bool MapFrame::inspectorVisible() const {
            return m_inspector->isVisible();
        }

        void MapFrame::toggleMaximizeCurrentView() {
            m_mapView->toggleMaximizeCurrentView();
        }

        bool MapFrame::currentViewMaximized() {
            return m_mapView->currentViewMaximized();
        }

        void MapFrame::showCompileDialog() {
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

        void MapFrame::showLaunchEngineDialog() {
            // FIXME:
//            LaunchGameEngineDialog dialog(this, m_document);
//            dialog.ShowModal();
        }

        void MapFrame::debugPrintVertices() {
            m_document->printVertices();
        }

        void MapFrame::debugCreateBrush() {
            bool ok = false;
            const QString str = QInputDialog::getText(this, "Create Brush", "Enter a list of at least 4 points (x y z) (x y z) ...", QLineEdit::Normal, "", &ok);
            if (ok) {
                std::vector<vm::vec3> positions;
                vm::vec3::parseAll(str.toStdString(), std::back_inserter(positions));
                m_document->createBrush(positions);
            }
        }

        void MapFrame::debugCreateCube() {
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

        void MapFrame::debugClipBrush() {
            bool ok = false;
            const QString str = QInputDialog::getText(this, "Clip Brush", "Enter face points ( x y z ) ( x y z ) ( x y z )", QLineEdit::Normal, "", &ok);
            if (ok) {
                std::vector<vm::vec3> points;
                vm::vec3::parseAll(str.toStdString(), std::back_inserter(points));
                assert(points.size() == 3);
                m_document->clipBrushes(points[0], points[1], points[2]);
            }
        }

        void MapFrame::debugCopyJSShortcutMap() {
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

        void MapFrame::debugCrash() {
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

        void MapFrame::debugThrowExceptionDuringCommand() {
            m_document->throwExceptionDuringCommand();
        }

        void MapFrame::debugSetWindowSize() {
            bool ok = false;
            const QString str = QInputDialog::getText(this, "Window Size", "Enter Size (W H)", QLineEdit::Normal, "1920 1080", &ok);
            if (ok) {
                const auto size = vm::vec2i::parse(str.toStdString());
                resize(size.x(), size.y());
            }
        }

        void MapFrame::OnFlipObjectsHorizontally() {
            if (m_mapView->canFlipObjects()) {
                m_mapView->flipObjects(vm::direction::left);
            }
        }

        void MapFrame::OnFlipObjectsVertically() {
            if (m_mapView->canFlipObjects()) {
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
            updateActionState();
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
