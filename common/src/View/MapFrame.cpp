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

#include "Console.h"
#include "Exceptions.h"
#include "FileLogger.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "TrenchBroomApp.h"
#include "IO/PathQt.h"
#include "Model/AttributableNode.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/ExportFormat.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/ModelUtils.h"
#include "Model/Node.h"
#include "Model/WorldNode.h"
#include "View/Actions.h"
#include "View/Autosaver.h"
#if !defined __APPLE__
#include "View/BorderLine.h"
#endif
#include "View/MapViewBase.h"
#include "View/ClipTool.h"
#include "View/ColorButton.h"
#include "View/CompilationDialog.h"
#include "View/EdgeTool.h"
#include "View/FaceInspector.h"
#include "View/FaceTool.h"
#include "View/FrameManager.h"
#include "View/GLContextManager.h"
#include "View/Grid.h"
#include "View/InfoPanel.h"
#include "View/Inspector.h"
#include "View/LaunchGameEngineDialog.h"
#include "View/MainMenuBuilder.h"
#include "View/MapDocument.h"
#include "View/PasteType.h"
#include "View/RenderView.h"
#include "View/ReplaceTextureDialog.h"
#include "View/Splitter.h"
#include "View/SwitchableMapViewContainer.h"
#include "View/VertexTool.h"
#include "View/ViewUtils.h"
#include "View/QtUtils.h"
#include "View/MapViewToolBox.h"

#include <kdl/overload.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>

#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <cassert>
#include <chrono>
#include <iterator>
#include <string>
#include <vector>

#include <QtGlobal>
#include <QTimer>
#include <QLabel>
#include <QString>
#include <QApplication>
#include <QChildEvent>
#include <QClipboard>
#include <QInputDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QFileDialog>
#include <QPushButton>
#include <QStatusBar>
#include <QStringList>
#include <QToolBar>
#include <QComboBox>
#include <QVBoxLayout>
#include <QTableWidget>

namespace TrenchBroom {
    namespace View {
        MapFrame::MapFrame(FrameManager* frameManager, std::shared_ptr<MapDocument> document) :
        QMainWindow(),
        m_frameManager(frameManager),
        m_document(std::move(document)),
        m_lastInputTime(std::chrono::system_clock::now()),
        m_autosaver(std::make_unique<Autosaver>(m_document)),
        m_autosaveTimer(nullptr),
        m_toolBar(nullptr),
        m_hSplitter(nullptr),
        m_vSplitter(nullptr),
        m_contextManager(std::make_unique<GLContextManager>()),
        m_mapView(nullptr),
        m_currentMapView(nullptr),
        m_infoPanel(nullptr),
        m_console(nullptr),
        m_inspector(nullptr),
        m_gridChoice(nullptr),
        m_statusBarLabel(nullptr),
        m_compilationDialog(nullptr),
        m_recentDocumentsMenu(nullptr),
        m_undoAction(nullptr),
        m_redoAction(nullptr) {
            ensure(m_frameManager != nullptr, "frameManager is null");
            ensure(m_document != nullptr, "document is null");

            setAttribute(Qt::WA_DeleteOnClose);
            setObjectName("MapFrame");

            installEventFilter(this);

            createGui();
            createMenus();
            createToolBar();
            createStatusBar();

            updateShortcuts();
            updateActionState();
            updateUndoRedoActions();
            updateToolBarWidgets();

            m_document->setParentLogger(m_console);
            m_document->setViewEffectsService(m_mapView);

            m_autosaveTimer = new QTimer(this);
            m_autosaveTimer->start(1000);

            bindObservers();
            bindEvents();

            restoreWindowGeometry(this);
            restoreWindowState(this);
        }

        MapFrame::~MapFrame() {
            // Search for a RenderView (QOpenGLWindow subclass) and make it current in order to allow for calling
            // OpenGL methods in destructors.
            auto* renderView = findChild<RenderView*>();
            if (renderView != nullptr) {
                renderView->makeCurrent();
            }

            // The MapDocument's CachingLogger has a pointer to m_console, which
            // is about to be destroyed (DestroyChildren()). Clear the pointer
            // so we don't try to log to a dangling pointer (#1885).
            m_document->setParentLogger(nullptr);

            m_mapView->deactivateTool();

            unbindObservers();
            removeRecentDocumentsMenu();

            // The order of deletion here is important because both the document and the children
            // need the context manager (and its embedded VBO) to clean up their resources.

            // Destroy the children first because they might still access document resources.
            // The children must be deleted in reverse order!
            const auto children = this->children();
            qDeleteAll(std::rbegin(children), std::rend(children));

            // let's trigger a final autosave before releasing the document
            NullLogger logger;
            m_autosaver->triggerAutosave(logger);

            m_document->setViewEffectsService(nullptr);
            m_document.reset();

            // FIXME: m_contextManager is deleted via smart pointer; it may release openGL resources in its destructor
        }

        void MapFrame::positionOnScreen(QWidget* reference) {
            restoreWindowGeometry(this);
            restoreWindowState(this);
            if (reference) {
                const auto offset =  QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
                move(reference->pos() + QPoint(offset, offset));
            }
        }

        std::shared_ptr<MapDocument> MapFrame::document() const {
            return m_document;
        }

        Logger& MapFrame::logger() const {
            return *m_console;
        }

        void MapFrame::updateTitle() {
            setWindowModified(m_document->modified());
            setWindowTitle(QString::fromStdString(m_document->filename()) + QString("[*] - TrenchBroom"));
            setWindowFilePath(IO::pathAsQString(m_document->path()));
        }

        void MapFrame::createMenus() {
            MainMenuBuilder menuBuilder(*menuBar(), m_actionMap, [this](const Action& action) {
                ActionExecutionContext context(this, currentMapViewBase());
                action.execute(context);
            });

            const auto& actionManager = ActionManager::instance();
            actionManager.visitMainMenu(menuBuilder);

            m_recentDocumentsMenu = menuBuilder.recentDocumentsMenu;
            m_undoAction = menuBuilder.undoAction;
            m_redoAction = menuBuilder.redoAction;

            addRecentDocumentsMenu();
        }

        void MapFrame::updateShortcuts() {
            for (auto [tAction, qAction] : m_actionMap) {
                MenuBuilderBase::updateActionKeySeqeunce(qAction, tAction);
            }
        }

        void MapFrame::updateActionState() {
            ActionExecutionContext context(this, currentMapViewBase());
            for (auto [tAction, qAction] : m_actionMap) {
                if (qAction == m_undoAction || qAction == m_redoAction) {
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
            const auto document = kdl::mem_lock(m_document);
            if (m_undoAction != nullptr) {
                if (document->canUndoCommand()) {
                    const auto text = "Undo " + document->undoCommandName();
                    m_undoAction->setText(QString::fromStdString(text));
                    m_undoAction->setEnabled(true);
                } else {
                    m_undoAction->setText("Undo");
                    m_undoAction->setEnabled(false);
                }
            }
            if (m_redoAction != nullptr) {
                if (document->canRedoCommand()) {
                    const auto text = "Redo " + document->redoCommandName();
                    m_redoAction->setText(QString::fromStdString(text));
                    m_redoAction->setEnabled(true);
                } else {
                    m_redoAction->setText("Redo");
                    m_redoAction->setEnabled(false);
                }
            }
        }

        void MapFrame::addRecentDocumentsMenu() {
            TrenchBroomApp& app = TrenchBroomApp::instance();
            app.addRecentDocumentMenu(m_recentDocumentsMenu);
        }

        void MapFrame::removeRecentDocumentsMenu() {
            auto& app = TrenchBroomApp::instance();
            app.removeRecentDocumentMenu(m_recentDocumentsMenu);
        }

        void MapFrame::updateRecentDocumentsMenu() {
            if (m_document->path().isAbsolute()) {
                auto& app = TrenchBroomApp::instance();
                app.updateRecentDocument(m_document->path());
            }
        }

        void MapFrame::createGui() {
            setWindowIconTB(this);
            setWindowTitle("TrenchBroom");

            m_hSplitter = new Splitter(Qt::Horizontal);
            m_hSplitter->setChildrenCollapsible(false);
            m_hSplitter->setObjectName("MapFrame_HorizontalSplitter");

            m_vSplitter = new Splitter(Qt::Vertical);
            m_vSplitter->setChildrenCollapsible(false);
            m_vSplitter->setObjectName("MapFrame_VerticalSplitterSplitter");

            m_infoPanel = new InfoPanel(m_document);
            m_console = m_infoPanel->console();

            m_mapView = new SwitchableMapViewContainer(m_console, m_document, *m_contextManager);
            m_currentMapView = m_mapView->firstMapViewBase();
            ensure(m_currentMapView != nullptr, "SwitchableMapViewContainer should have constructed a MapViewBase");

            m_inspector = new Inspector(m_document, *m_contextManager);

            m_mapView->connectTopWidgets(m_inspector);

            // Add widgets to splitters
            m_vSplitter->addWidget(m_mapView);
            m_vSplitter->addWidget(m_infoPanel);

            m_hSplitter->addWidget(m_vSplitter);
            m_hSplitter->addWidget(m_inspector);

            // configure minimum sizes
            m_mapView->setMinimumSize(100, 100);
            m_infoPanel->setMinimumSize(100, 100);

            m_vSplitter->setMinimumSize(100, 100);
            m_inspector->setMinimumSize(350, 100);

            // resize only the map view when the window resizes
            m_vSplitter->setStretchFactor(0, 1);
            m_vSplitter->setStretchFactor(1, 0);
            m_hSplitter->setStretchFactor(0, 1);
            m_hSplitter->setStretchFactor(1, 0);

            // give most of the space to the map view
            m_hSplitter->setSizes(QList<int>{1'000'000, 1});
            m_vSplitter->setSizes(QList<int>{1'000'000, 1});

            auto* frameLayout = new QVBoxLayout();
            frameLayout->setContentsMargins(0, 0, 0, 0);
            frameLayout->setSpacing(0); // no space between BorderLine and m_hSplitter
            frameLayout->addWidget(m_hSplitter);

            // NOTE: you can't set the layout of a QMainWindow, so make another widget to wrap this layout in
            auto* layoutWrapper = new QWidget();
            layoutWrapper->setLayout(frameLayout);

            setCentralWidget(layoutWrapper);

            restoreWindowState(m_hSplitter);
            restoreWindowState(m_vSplitter);
        }

        class MapFrame::ToolBarBuilder : public MenuBuilderBase, public MenuVisitor {
        private:
            QToolBar& m_toolBar;
        public:
            explicit ToolBarBuilder(QToolBar& toolBar, ActionMap& actions, const TriggerFn& triggerFn) :
            MenuBuilderBase(actions, triggerFn),
            m_toolBar(toolBar) {}

            void visit(const Menu& menu) override {
                menu.visitEntries(*this);
            }

            void visit(const MenuSeparatorItem&) override {
                m_toolBar.addSeparator();
            }

            void visit(const MenuActionItem& item) override {
                const auto& tAction = item.action();
                QAction* qAction = findOrCreateQAction(&tAction);
                m_toolBar.addAction(qAction);
            }
        };

        void MapFrame::createToolBar() {
            m_toolBar = addToolBar("Toolbar");
            m_toolBar->setObjectName("MapFrameToolBar");
            m_toolBar->setFloatable(false);
            m_toolBar->setMovable(false);
            // macOS Qt bug: with the 32x32 default icon size, 24x24 highdpi icons get scaled up to 32x32.
            // We expect them to be drawn at 24x24 logical pixels centered in a 32x32 box, as is the case with non-highdpi icons.
            // As a workaround, just lower the toolbar size to 24x24 (we could alternatively render the icons at 32x32).
            m_toolBar->setIconSize(QSize(24, 24));

            ToolBarBuilder builder(*m_toolBar, m_actionMap, [this](const Action& action) {
                ActionExecutionContext context(this, currentMapViewBase());
                action.execute(context);
            });

            auto& actionManager = ActionManager::instance();
            actionManager.visitToolBarActions(builder);

            m_gridChoice = new QComboBox();
            for (int i = Grid::MinSize; i <= Grid::MaxSize; ++i) {
                const FloatType gridSize = Grid::actualSize(i);
                const QString gridSizeStr = tr("Grid %1").arg(QString::number(gridSize, 'g'));
                m_gridChoice->addItem(gridSizeStr, QVariant(i));
            }

            m_toolBar->addWidget(m_gridChoice);
        }

        void MapFrame::updateToolBarWidgets() {
            const Grid& grid = m_document->grid();
            const int sizeIndex = grid.size() - Grid::MinSize;
            m_gridChoice->setCurrentIndex(sizeIndex);
        }

        void MapFrame::createStatusBar() {
            m_statusBarLabel = new QLabel();
            statusBar()->addWidget(m_statusBarLabel);
        }

        static Model::AttributableNode* commonEntityForBrushList(const std::vector<Model::BrushNode*>& list) {
            if (list.empty())
                return nullptr;

            Model::AttributableNode* firstEntity = list.front()->entity();
            bool multipleEntities = false;

            for (const Model::BrushNode* brush : list) {
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

        static std::string commonClassnameForEntityList(const std::vector<Model::EntityNode*>& list) {
            if (list.empty())
                return "";

            const std::string firstClassname = list.front()->entity().classname();
            bool multipleClassnames = false;

            for (const Model::EntityNode* entityNode : list) {
                if (entityNode->entity().classname() != firstClassname) {
                    multipleClassnames = true;
                }
            }

            if (multipleClassnames) {
                return "";
            } else {
                return firstClassname;
            }
        }

        static std::string numberWithSuffix(size_t count, const std::string& singular, const std::string& plural) {
            return std::to_string(count) + " " + kdl::str_plural(count, singular, plural);
        }

        static QString describeSelection(const MapDocument* document) {
            const QString Arrow = QString(" ") + QString(QChar(0x203A)) + QString(" ");

            QStringList pipeSeparatedSections;

            pipeSeparatedSections << QString::fromStdString(document->game()->gameName())
                                  << QString::fromStdString(Model::formatName(document->world()->format()))
                                  << QString::fromStdString(document->currentLayer()->name());

            // open groups
            std::vector<Model::GroupNode*> groups;
            for (Model::GroupNode* group = document->currentGroup(); group != nullptr; group = group->group()) {
                groups.push_back(group);
            }
            if (!groups.empty()) {
                QStringList openGroups;

                // groups vector is sorted from innermost to outermost, so visit it in reverse order
                for (auto it = groups.rbegin(); it != groups.rend(); ++it) {
                    const Model::GroupNode* group = *it;
                    openGroups << QString::fromStdString(group->name());
                }

                const QString openGroupsString = QObject::tr("Open groups: %1").arg(openGroups.join(Arrow));
                pipeSeparatedSections << openGroupsString;
            }

            // build a vector of strings describing the things that are selected
            std::vector<std::string> tokens;

            const auto &selectedNodes = document->selectedNodes();

            // selected brushes
            if (!selectedNodes.brushes().empty()) {
                Model::AttributableNode* commonEntityNode = commonEntityForBrushList(selectedNodes.brushes());

                // if all selected brushes are from the same entity, print the entity name
                std::string token = numberWithSuffix(selectedNodes.brushes().size(), "brush", "brushes");
                if (commonEntityNode) {
                    token += " (" + commonEntityNode->entity().classname() + ")";
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
                std::string commonClassname = commonClassnameForEntityList(selectedNodes.entities());

                std::string token = numberWithSuffix(selectedNodes.entities().size(), "entity", "entities");
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

            // get the layers of the selected nodes
            const std::vector<Model::LayerNode*> selectedObjectLayers = Model::findContainingLayersUserSorted(selectedNodes.nodes());
            QString layersDescription;
            if (selectedObjectLayers.size() == 1) {
                Model::LayerNode* layer = selectedObjectLayers[0];
                layersDescription = QObject::tr(" in layer \"%1\"").arg(QString::fromStdString(layer->name()));
            } else if (selectedObjectLayers.size() > 1) {
                layersDescription = QObject::tr(" in %1 layers").arg(selectedObjectLayers.size());
            }

            // now, turn `tokens` into a comma-separated string
            if (!tokens.empty()) {
                pipeSeparatedSections << QObject::tr("%1%2 selected")
                        .arg(QString::fromStdString(kdl::str_join(tokens, ", ", ", and ", " and ")))
                        .arg(layersDescription);
            }

            // count hidden objects
            size_t hiddenGroups = 0u;
            size_t hiddenEntities = 0u;
            size_t hiddenBrushes = 0u;

            const auto& editorContext = document->editorContext();
            document->world()->accept(kdl::overload(
                [](auto&& thisLambda, const Model::WorldNode* world) { world->visitChildren(thisLambda); },
                [](auto&& thisLambda, const Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
                [&](auto&& thisLambda, const Model::GroupNode* group) { 
                    if (!editorContext.visible(group)) {
                        ++hiddenGroups;
                    }
                    group->visitChildren(thisLambda); 
                },
                [&](auto&& thisLambda, const Model::EntityNode* entity) { 
                    if (!editorContext.visible(entity)) {
                        ++hiddenEntities;
                    }
                    entity->visitChildren(thisLambda); 
                },
                [&](const Model::BrushNode* brush) {
                    if (!editorContext.visible(brush)) {
                        ++hiddenBrushes;
                    }
                 }
            ));

            // print hidden objects
            if (hiddenGroups > 0 || hiddenEntities > 0 || hiddenBrushes > 0) {
                std::vector<std::string> hiddenDescriptors;

                if (hiddenGroups > 0) {
                    hiddenDescriptors.push_back(numberWithSuffix(hiddenGroups, "group", "groups"));
                }
                if (hiddenEntities > 0) {
                    hiddenDescriptors.push_back(numberWithSuffix(hiddenEntities, "entity", "entities"));
                }
                if (hiddenBrushes > 0) {
                    hiddenDescriptors.push_back(numberWithSuffix(hiddenBrushes, "brush", "brushes"));
                }

                pipeSeparatedSections << QObject::tr("%1 hidden")
                        .arg(QString::fromStdString(kdl::str_join(hiddenDescriptors, ", ", ", and ", " and ")));
            }

            return QString::fromLatin1("   ") + pipeSeparatedSections.join(QLatin1String("   |   "));
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
            m_document->nodeVisibilityDidChangeNotifier.addObserver(this, &MapFrame::nodeVisibilityDidChange);
            m_document->editorContextDidChangeNotifier.addObserver(this, &MapFrame::editorContextDidChange);

            Grid& grid = m_document->grid();
            grid.gridDidChangeNotifier.addObserver(this, &MapFrame::gridDidChange);

            m_mapView->mapViewToolBox()->toolActivatedNotifier.addObserver(this, &MapFrame::toolActivated);
            m_mapView->mapViewToolBox()->toolDeactivatedNotifier.addObserver(this, &MapFrame::toolDeactivated);
            m_mapView->mapViewToolBox()->toolHandleSelectionChangedNotifier.addObserver(this, &MapFrame::toolHandleSelectionChanged);
        }

        void MapFrame::unbindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            assertResult(prefs.preferenceDidChangeNotifier.removeObserver(this, &MapFrame::preferenceDidChange))

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
            m_document->nodeVisibilityDidChangeNotifier.removeObserver(this, &MapFrame::nodeVisibilityDidChange);
            m_document->editorContextDidChangeNotifier.removeObserver(this, &MapFrame::editorContextDidChange);

            Grid& grid = m_document->grid();
            grid.gridDidChangeNotifier.removeObserver(this, &MapFrame::gridDidChange);

            m_mapView->mapViewToolBox()->toolActivatedNotifier.removeObserver(this, &MapFrame::toolActivated);
            m_mapView->mapViewToolBox()->toolDeactivatedNotifier.removeObserver(this, &MapFrame::toolDeactivated);
            m_mapView->mapViewToolBox()->toolHandleSelectionChangedNotifier.removeObserver(this, &MapFrame::toolHandleSelectionChanged);
        }

        void MapFrame::documentWasCleared(View::MapDocument*) {
            updateTitle();
            updateActionState();
        }

        void MapFrame::documentDidChange(View::MapDocument*) {
            updateTitle();
            updateActionState();
            updateRecentDocumentsMenu();
        }

        void MapFrame::documentModificationStateDidChange() {
            updateTitle();
        }

        void MapFrame::transactionDone(const std::string& /* name */) {
            QTimer::singleShot(0, this, [this]() {
                // FIXME: Delaying this with QTimer::singleShot is a hack to work around the lack of
                // a notification that's called _after_ the CommandProcessor undo/redo stacks are modified.
                //
                // The current transactionDoneNotifier is called after the transaction executes, but before it's
                // pushed onto the undo stack, but we need to read the undo stack in updateUndoRedoActions(),
                // so this QTimer::singleShot is needed for now.
                updateUndoRedoActions();
            });
        }

        void MapFrame::transactionUndone(const std::string& /* name */) {
            QTimer::singleShot(0, this, [this]() {
                // FIXME: see MapFrame::transactionDone
                updateUndoRedoActions();
            });
        }

        void MapFrame::preferenceDidChange(const IO::Path& path) {
            if (path == Preferences::MapViewLayout.path()) {
                m_mapView->switchToMapView(static_cast<MapViewLayout>(pref(Preferences::MapViewLayout)));
            }

            updateShortcuts();
        }

        void MapFrame::gridDidChange() {
            updateActionState();
            updateToolBarWidgets();
        }

        void MapFrame::toolActivated(Tool*) {
            updateActionState();
        }

        void MapFrame::toolDeactivated(Tool*) {
            updateActionState();
        }

        void MapFrame::toolHandleSelectionChanged(Tool*) {
            updateActionState();
        }

        void MapFrame::selectionDidChange(const Selection&) {
            updateActionState();
            updateStatusBar();
        }

        void MapFrame::currentLayerDidChange(const TrenchBroom::Model::LayerNode*) {
            updateStatusBar();
        }

        void MapFrame::groupWasOpened(Model::GroupNode*) {
            updateStatusBar();
        }

        void MapFrame::groupWasClosed(Model::GroupNode*) {
            updateStatusBar();
        }

        void MapFrame::nodeVisibilityDidChange(const std::vector<Model::Node*>&) {
            updateStatusBar();
        }

        void MapFrame::editorContextDidChange() {
            // e.g. changing the view filters may cause the number of hidden brushes/entities to change
            updateStatusBar();
        }

        void MapFrame::bindEvents() {
            connect(m_autosaveTimer, &QTimer::timeout, this, &MapFrame::triggerAutosave);
            connect(qApp, &QApplication::focusChanged, this, &MapFrame::focusChange);
            connect(m_gridChoice, QOverload<int>::of(&QComboBox::activated), this, [this](const int index) { setGridSize(index + Grid::MinSize); });
            connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [this]() {
                // update the "Paste" menu items
                this->updateActionState();
            });
            connect(m_toolBar, &QToolBar::visibilityChanged, this, [this](const bool /* visible */) {
                // update the "Toggle Toolbar" menu item
                this->updateActionState();
            });
        }

        bool MapFrame::newDocument(std::shared_ptr<Model::Game> game, const Model::MapFormat mapFormat) {
            if (!confirmOrDiscardChanges()) {
                return false;
            }
            m_document->newDocument(mapFormat, MapDocument::DefaultWorldBounds, game);
            return true;
        }

        bool MapFrame::openDocument(std::shared_ptr<Model::Game> game, const Model::MapFormat mapFormat, const IO::Path& path) {
            if (!confirmOrDiscardChanges()) {
                return false;
            }
            m_document->loadDocument(mapFormat, MapDocument::DefaultWorldBounds, game, path);
            return true;
        }

        bool MapFrame::saveDocument() {
            try {
                if (m_document->persistent()) {
                    const auto startTime = std::chrono::high_resolution_clock::now();
                    m_document->saveDocument();
                    const auto endTime = std::chrono::high_resolution_clock::now();

                    logger().info() << "Saved " << m_document->path() << " in "
                                    << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << "ms";
                    return true;
                } else {
                    return saveDocumentAs();
                }
            } catch (const FileSystemException& e) {
                QMessageBox::critical(this, "", e.what());
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

                const QString newFileName = QFileDialog::getSaveFileName(this, tr("Save map file"), IO::pathAsQString(originalPath), "Map files (*.map)");
                if (newFileName.isEmpty()) {
                    return false;
                }

                const IO::Path path = IO::pathFromQString(newFileName);

                const auto startTime = std::chrono::high_resolution_clock::now();
                m_document->saveDocumentAs(path);
                const auto endTime = std::chrono::high_resolution_clock::now();

                logger().info() << "Saved " << m_document->path() << " in "
                                << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << "ms";
                return true;
            } catch (const FileSystemException& e) {
                QMessageBox::critical(this, "", e.what());
                return false;
            } catch (...) {
                QMessageBox::critical(this, "", QString::fromStdString("Unknown error while saving " + m_document->filename()), QMessageBox::Ok);
                return false;
            }
        }

        bool MapFrame::revertDocument() {
            if (!m_document->persistent()) {
                return false;
            }
            if (!confirmRevertDocument()) {
                return false;
            }
            const auto mapFormat = m_document->world()->format();
            const auto game = m_document->game();
            const auto path = m_document->path();
            m_document->loadDocument(mapFormat, MapDocument::DefaultWorldBounds, game, path);
            return true;
        }

        bool MapFrame::exportDocumentAsObj() {
            const IO::Path& originalPath = m_document->path();
            const IO::Path objPath = originalPath.replaceExtension("obj");

            const QString newFileName = QFileDialog::getSaveFileName(this, tr("Export Wavefront OBJ file"), IO::pathAsQString(objPath), "Wavefront OBJ files (*.obj)");
            if (newFileName.isEmpty())
                return false;

            return exportDocument(Model::ExportFormat::WavefrontObj, IO::pathFromQString(newFileName));
        }

        bool MapFrame::exportDocumentAsMap() {
            const IO::Path& originalPath = m_document->path();

            const QString newFileName = QFileDialog::getSaveFileName(this, tr("Export Map file"), IO::pathAsQString(originalPath), "Map files (*.map)");
            if (newFileName.isEmpty()) {
                return false;
            }

            return exportDocument(Model::ExportFormat::Map, IO::pathFromQString(newFileName));
        }

        bool MapFrame::exportDocument(const Model::ExportFormat format, const IO::Path& path) {
            if (path == m_document->path()) {
                QMessageBox::critical(this, "", tr("You can't overwrite the current document.\nPlease choose a different file name to export to."));
                return false;
            }
            try {
                m_document->exportDocumentAs(format, path);
                logger().info() << "Exported " << path;
                return true;
            } catch (const FileSystemException& e) {
                QMessageBox::critical(this, "", e.what());
                return false;
            } catch (...) {
                QMessageBox::critical(this, "", QString::fromStdString("Unknown error while exporting " + path.asString()), QMessageBox::Ok);
                return false;
            }
        }

        /**
         * Returns whether the window should close.
         */
        bool MapFrame::confirmOrDiscardChanges() {
            if (!m_document->modified())
                return true;
            const QMessageBox::StandardButton result = QMessageBox::question(this, "TrenchBroom", QString::fromStdString(m_document->filename() + " has been modified. Do you want to save the changes?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif
            switch (result) {
                case QMessageBox::Yes:
                    return saveDocument();
                case QMessageBox::No:
                    return true;
                default:
                    return false;
            }
#ifdef __clang__
#pragma clang diagnostic pop
#endif
        }

        /**
         * Returns whether the document should be reverted.
         */
        bool MapFrame::confirmRevertDocument() {
            if (!m_document->modified()) {
                return true;
            }

            QMessageBox messageBox(this);
            messageBox.setWindowTitle("TrenchBroom");
            messageBox.setIcon(QMessageBox::Question);
            messageBox.setText(tr("Revert %1 to %2?")
                .arg(QString::fromStdString(m_document->filename()))
                .arg(IO::pathAsQString(m_document->path())));
            messageBox.setInformativeText(tr("This will discard all unsaved changes and reload the document from disk."));

            auto* revertButton = messageBox.addButton(tr("Revert"), QMessageBox::DestructiveRole);
            messageBox.addButton(QMessageBox::Cancel);

            messageBox.exec();

            return (messageBox.clickedButton() == revertButton);
        }

        void MapFrame::loadPointFile() {
            QString defaultDir;
            if (!m_document->path().isEmpty()) {
                defaultDir = IO::pathAsQString(m_document->path().deleteLastComponent());
            }

            const QString fileName = QFileDialog::getOpenFileName(this, tr("Load Point File"), defaultDir, "Point files (*.pts);;Any files (*.*)");

            if (!fileName.isEmpty()) {
                m_document->loadPointFile(IO::pathFromQString(fileName));
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
                defaultDir = IO::pathAsQString(m_document->path().deleteLastComponent());
            }

            const QString fileName = QFileDialog::getOpenFileName(this, tr("Load Portal File"), defaultDir, "Portal files (*.prt);;Any files (*.*)");

            if (!fileName.isEmpty()) {
                m_document->loadPortalFile(IO::pathFromQString(fileName));
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
                if (!m_mapView->cancelMouseDrag() && !m_inspector->cancelMouseDrag()) {
                    m_document->undoCommand();
                }
            }
        }

        void MapFrame::redo() {
            if (canRedo()) {
                m_document->redoCommand();
            }
        }

        bool MapFrame::canUndo() const {
            return m_document->canUndoCommand();
        }

        bool MapFrame::canRedo() const {
            return m_document->canRedoCommand();
        }

        void MapFrame::repeatLastCommands() {
            m_document->repeatCommands();
        }

        void MapFrame::clearRepeatableCommands() {
            if (hasRepeatableCommands()) {
                m_document->clearRepeatableCommands();
            }
        }

        bool MapFrame::hasRepeatableCommands() const {
            return m_document->canRepeatCommands();
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

            std::string str;
            if (m_document->hasSelectedNodes()) {
                str = m_document->serializeSelectedNodes();
            } else if (m_document->hasSelectedBrushFaces()) {
                str = m_document->serializeSelectedBrushFaces();
            }

            clipboard->setText(mapStringToUnicode(m_document->encoding(), str));
        }

        bool MapFrame::canCutSelection() const {
            return widgetOrChildHasFocus(m_mapView) && m_document->hasSelectedNodes() && !m_mapView->anyToolActive();
        }

        bool MapFrame::canCopySelection() const {
            return widgetOrChildHasFocus(m_mapView) && (m_document->hasSelectedNodes() || m_document->hasSelectedBrushFaces());
        }

        void MapFrame::pasteAtCursorPosition() {
            if (canPaste()) {
                const vm::bbox3 referenceBounds = m_document->referenceBounds();
                Transaction transaction(m_document);
                if (paste() == PasteType::Node && m_document->hasSelectedNodes()) {
                    const vm::bbox3 bounds = m_document->selectionBounds();

                    // The pasted objects must be hidden to prevent the picking done in pasteObjectsDelta
                    // from hitting them (https://github.com/TrenchBroom/TrenchBroom/issues/2755)
                    const std::vector<Model::Node*> nodes = m_document->selectedNodes().nodes();
                    m_document->hide(nodes);
                    const vm::vec3 delta = m_mapView->pasteObjectsDelta(bounds, referenceBounds);
                    m_document->show(nodes);
                    m_document->select(nodes); // Hiding deselected the nodes, so reselect them
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
                return PasteType::Failed;
            }

            return m_document->paste(mapStringFromUnicode(m_document->encoding(), qtext));
        }

        /**
         * This is relatively expensive so only call it when the clipboard changes or e.g. the user tries to paste.
         */
        bool MapFrame::canPaste() const {
            if (!widgetOrChildHasFocus(m_mapView)) {
                return false;
            }

            if (!m_mapView->isCurrent()) {
                return false;
            }

            const auto* clipboard = QApplication::clipboard();
            const auto* mimeData = clipboard->mimeData();
            return mimeData != nullptr && mimeData->hasText();
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

        void MapFrame::selectInverse() {
            if (canSelectInverse()) {
               m_document->selectInverse();
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

        bool MapFrame::canSelectInverse() const {
            return m_document->editorContext().canChangeSelection();
        }

        void MapFrame::groupSelectedObjects() {
            if (canGroupSelectedObjects()) {
                const std::string name = queryGroupName(this);
                if (!name.empty()) {
                    m_document->groupSelection(name);
                }
            }
        }

        bool MapFrame::canGroupSelectedObjects() const {
            return m_document->hasSelectedNodes() && !m_mapView->anyToolActive();
        }

        void MapFrame::ungroupSelectedObjects() {
            if (canUngroupSelectedObjects()) {
                m_document->ungroupSelection();
            }
        }

        bool MapFrame::canUngroupSelectedObjects() const {
            return m_document->selectedNodes().hasOnlyGroups() && !m_mapView->anyToolActive();
        }

        void MapFrame::renameSelectedGroups() {
            if (canRenameSelectedGroups()) {
                auto document = kdl::mem_lock(m_document);
                assert(document->selectedNodes().hasOnlyGroups());
                const std::string name = queryGroupName(this);
                if (!name.empty()) {
                    document->renameGroups(name);
                }
            }
        }

        bool MapFrame::canRenameSelectedGroups() const {
            auto document = kdl::mem_lock(m_document);
            return document->selectedNodes().hasOnlyGroups();
        }

        void MapFrame::replaceTexture() {
            ReplaceTextureDialog dialog(m_document, *m_contextManager, this);
            dialog.exec();
        }

        bool MapFrame::anyToolActive() const {
            return m_mapView->anyToolActive();
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

        bool MapFrame::anyVertexToolActive() const {
            return vertexToolActive() || edgeToolActive() || faceToolActive();
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
            return m_document->selectedNodes().hasBrushesRecursively();
        }

        void MapFrame::toggleTextureLock() {
            togglePref(Preferences::TextureLock);
        }

        void MapFrame::toggleUVLock() {
            togglePref(Preferences::UVLock);
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
                const vm::vec3 position = vm::parse<FloatType, 3>(str.toStdString());
                m_mapView->moveCameraToPosition(position, true);
            }
        }

        void MapFrame::isolateSelection() {
            if (canIsolateSelection()) {
                m_document->isolate();
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

        void MapFrame::switchToInspectorPage(const InspectorPage page) {
            m_inspector->show();
            m_inspector->switchToPage(page);
        }

        void MapFrame::toggleToolbar() {
            m_toolBar->setVisible(!m_toolBar->isVisible());
        }

        bool MapFrame::toolbarVisible() const {
            return m_toolBar->isVisible();
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
            if (m_compilationDialog == nullptr) {
                m_compilationDialog = new CompilationDialog(this);
            }
            showModelessDialog(m_compilationDialog);
        }

        void MapFrame::showLaunchEngineDialog() {
            LaunchGameEngineDialog dialog(m_document, this);
            dialog.exec();
        }

        static const Assets::Texture* textureToReveal(std::shared_ptr<MapDocument> document) {
            kdl::vector_set<const Assets::Texture*> selectedTextures;
            for (const Model::BrushFaceHandle& face : document->allSelectedBrushFaces()) {
                selectedTextures.insert(face.face().texture());
            }
            if (selectedTextures.size() == 1) {
                return *selectedTextures.begin();
            }
            return nullptr;
        }

        bool MapFrame::canRevealTexture() const {
            return textureToReveal(m_document) != nullptr;
        }

        void MapFrame::revealTexture() {
            if (const auto texture = textureToReveal(m_document)) {
                revealTexture(texture);
            }
        }

        void MapFrame::revealTexture(const Assets::Texture* texture) {
            m_inspector->switchToPage(InspectorPage::Face);
            m_inspector->faceInspector()->revealTexture(texture);
        }

        void MapFrame::debugPrintVertices() {
            m_document->printVertices();
        }

        void MapFrame::debugCreateBrush() {
            bool ok = false;
            const QString str = QInputDialog::getText(this, "Create Brush", "Enter a list of at least 4 points (x y z) (x y z) ...", QLineEdit::Normal, "", &ok);
            if (ok) {
                std::vector<vm::vec3> positions;
                vm::parse_all<FloatType, 3>(str.toStdString(), std::back_inserter(positions));
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
                vm::parse_all<FloatType, 3>(str.toStdString(), std::back_inserter(points));
                assert(points.size() == 3);
                m_document->clipBrushes(points[0], points[1], points[2]);
            }
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

        [[noreturn]] static void debugException() {
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
                const auto size = vm::parse<int, 2>(str.toStdString());
                resize(size.x(), size.y());
            }
        }

        void MapFrame::debugShowPalette() {
            DebugPaletteWindow* window = new DebugPaletteWindow(this);
            showModelessDialog(window);
        }

        void MapFrame::focusChange(QWidget* /* oldFocus */, QWidget* newFocus) {
            auto newMapView = dynamic_cast<MapViewBase*>(newFocus);
            if (newMapView != nullptr) {
                m_currentMapView = newMapView;
            }

            updateActionState();
        }

        MapViewBase* MapFrame::currentMapViewBase() {
            if (!m_currentMapView) {
                // This happens when the current map view is deleted (e.g. 4-pane to 1-pane layout)
                m_currentMapView = m_mapView->firstMapViewBase();
                ensure(m_currentMapView != nullptr, "SwitchableMapViewContainer should have constructed a MapViewBase");
            }
            return m_currentMapView;
        }

        bool MapFrame::canCompile() const {
            return m_document->persistent();
        }

        bool MapFrame::canLaunch() const {
            return m_document->persistent();
        }

        void MapFrame::changeEvent(QEvent*) {
            if (m_mapView != nullptr) {
                m_mapView->windowActivationStateChanged(isActiveWindow());
            }
        }

        void MapFrame::closeEvent(QCloseEvent* event) {
            if (m_compilationDialog != nullptr && !m_compilationDialog->close()) {
                event->ignore();
            } else {
                ensure(m_frameManager != nullptr, "frameManager is null");
                if (!confirmOrDiscardChanges()) {
                    event->ignore();
                } else {
                    saveWindowGeometry(this);
                    saveWindowState(this);
                    saveWindowState(m_hSplitter);
                    saveWindowState(m_vSplitter);

                    m_frameManager->removeFrame(this);
                    event->accept();
                }
            }
            // Don't call superclass implementation
        }

        template <typename F>
        static void applyRecursively(QObject* object, const F& f) {
            f(object);
            for (auto* child : object->children()) {
                applyRecursively(child, f);
            }
        }

        bool MapFrame::eventFilter(QObject* target, QEvent* event) {
            if (event->type() == QEvent::MouseButtonPress ||
                event->type() == QEvent::MouseButtonRelease ||
                event->type() == QEvent::MouseButtonDblClick ||
                event->type() == QEvent::MouseMove ||
                event->type() == QEvent::KeyPress ||
                event->type() == QEvent::KeyRelease) {
                m_lastInputTime = std::chrono::system_clock::now();
            } else if (event->type() == QEvent::ChildAdded) {
                auto* childEvent = static_cast<QChildEvent*>(event);
                applyRecursively(childEvent->child(), [&](auto* object) { object->installEventFilter(this); });
            } else if (event->type() == QEvent::ChildRemoved) {
                auto* childEvent = static_cast<QChildEvent*>(event);
                applyRecursively(childEvent->child(), [&](auto* object) { object->removeEventFilter(this); });
            }
            return QMainWindow::eventFilter(target, event);
        }

        void MapFrame::triggerAutosave() {
            using namespace std::chrono_literals;
            if (QGuiApplication::mouseButtons() == Qt::NoButton && std::chrono::system_clock::now() - m_lastInputTime > 2s) {
                m_autosaver->triggerAutosave(logger());
            }
        }

        // DebugPaletteWindow

        DebugPaletteWindow::DebugPaletteWindow(QWidget *parent)
        : QDialog(parent) {
            setWindowTitle(tr("Palette"));

            const auto roles = std::vector<std::pair<QPalette::ColorRole, QString>> {
                { QPalette::Window,          "Window" },
                { QPalette::WindowText,      "WindowText" },
                { QPalette::Base,            "Base" },
                { QPalette::AlternateBase,   "AlternateBase" },
                { QPalette::ToolTipBase,     "ToolTipBase" },
                { QPalette::ToolTipText,     "ToolTipText" },
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
                { QPalette::PlaceholderText, "PlaceholderText" },
#endif
                { QPalette::Text,            "Text" },
                { QPalette::Button,          "Button" },
                { QPalette::ButtonText,      "ButtonText" },
                { QPalette::BrightText,      "BrightText" },
                { QPalette::Light,           "Light" },
                { QPalette::Midlight,        "Midlight" },
                { QPalette::Dark,            "Dark" },
                { QPalette::Mid,             "Mid" },
                { QPalette::Shadow,          "Shadow" },
                { QPalette::Highlight,       "Highlight" },
                { QPalette::HighlightedText, "HighlightedText" }
            };

            const auto groups = std::vector<std::pair<QPalette::ColorGroup, QString>> {
                { QPalette::Disabled,  "Disabled" },
                { QPalette::Active,    "Active" },
                { QPalette::Inactive,  "Inactive" }
            };

            QStringList verticalHeaderLabels;
            for (const auto& role : roles) {
                verticalHeaderLabels.append(role.second);
            }

            QStringList horizontalHeaderLabels;
            for (const auto& group : groups) {
                horizontalHeaderLabels.append(group.second);
            }

            auto* table = new QTableWidget(static_cast<int>(roles.size()),
                                           static_cast<int>(groups.size()));
            table->setHorizontalHeaderLabels(horizontalHeaderLabels);
            table->setVerticalHeaderLabels(verticalHeaderLabels);

            for (int x = 0; x < table->columnCount(); ++x) {
                for (int y = 0; y < table->rowCount(); ++y) {
                    const QPalette::ColorRole role = roles.at(static_cast<size_t>(y)).first;
                    const QPalette::ColorGroup group = groups.at(static_cast<size_t>(x)).first;

                    ColorButton* button = new ColorButton();
                    table->setCellWidget(y, x, button);

                    button->setColor(qApp->palette().color(group, role));

                    connect(button, &ColorButton::colorChangedByUser, this, [=](const QColor& color){
                        QPalette palette = qApp->palette();
                        palette.setColor(group, role, color);
                        qApp->setPalette(palette);
                    });
                }
            }

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(table);
            setLayout(layout);
        }

        DebugPaletteWindow::~DebugPaletteWindow() = default;
    }
}
