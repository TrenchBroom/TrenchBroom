/*
 Copyright (C) 2010 Kristian Duske

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

#include <QApplication>
#include <QChildEvent>
#include <QClipboard>
#include <QComboBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QStatusBar>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QtGlobal>

#include "Console.h"
#include "Exceptions.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "TrenchBroomApp.h"
#include "io/ExportOptions.h"
#include "io/PathQt.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/Game.h"
#include "mdl/GameFactory.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/MapFormat.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"
#include "mdl/Resource.h"
#include "mdl/WorldNode.h"
#include "ui/ActionBuilder.h"
#include "ui/Actions.h"
#include "ui/Autosaver.h"
#include "ui/ChoosePathTypeDialog.h"
#include "ui/ClipTool.h"
#include "ui/ColorButton.h"
#include "ui/CompilationDialog.h"
#include "ui/EdgeTool.h"
#include "ui/FaceInspector.h"
#include "ui/FaceTool.h"
#include "ui/FrameManager.h"
#include "ui/GLContextManager.h"
#include "ui/Grid.h"
#include "ui/InfoPanel.h"
#include "ui/Inspector.h"
#include "ui/LaunchGameEngineDialog.h"
#include "ui/MapDocument.h"
#include "ui/MapView2D.h"
#include "ui/MapViewBase.h"
#include "ui/MapViewToolBox.h"
#include "ui/ObjExportDialog.h"
#include "ui/PasteType.h"
#include "ui/QtUtils.h"
#include "ui/RenderView.h"
#include "ui/ReplaceMaterialDialog.h"
#include "ui/SignalDelayer.h"
#include "ui/Splitter.h"
#include "ui/SwitchableMapViewContainer.h"
#include "ui/VertexTool.h"
#include "ui/ViewUtils.h"

#include "kdl/overload.h"
#include "kdl/range_to_vector.h"
#include "kdl/string_format.h"
#include "kdl/string_utils.h"

#include "vm/vec.h"
#include "vm/vec_io.h"

#include <fmt/format.h>

#include <cassert>
#include <chrono>
#include <iterator>
#include <string>
#include <variant>
#include <vector>

namespace tb::ui
{

MapFrame::MapFrame(FrameManager& frameManager, std::shared_ptr<MapDocument> document)
  : m_frameManager{frameManager}
  , m_document{std::move(document)}
  , m_lastInputTime{std::chrono::system_clock::now()}
  , m_autosaver{std::make_unique<Autosaver>(m_document)}
  , m_autosaveTimer{new QTimer{this}}
  , m_processResourcesTimer{new QTimer{this}}
  , m_contextManager{std::make_unique<GLContextManager>()}
  , m_updateTitleSignalDelayer{new SignalDelayer{this}}
  , m_updateActionStateSignalDelayer{new SignalDelayer{this}}
  , m_updateStatusBarSignalDelayer{new SignalDelayer{this}}
{
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

  m_autosaveTimer->start(1000);
  m_processResourcesTimer->start(20);

  connectObservers();
  bindEvents();

  restoreWindowGeometry(this);
  restoreWindowState(this);

  setAcceptDrops(true);
}

MapFrame::~MapFrame()
{
  // Search for a RenderView (QOpenGLWindow subclass) and make it current in order to
  // allow for calling OpenGL methods in destructors.
  auto* renderView = findChild<RenderView*>();
  if (renderView)
  {
    renderView->makeCurrent();
  }

  // The MapDocument's CachingLogger has a pointer to m_console, which
  // is about to be destroyed (DestroyChildren()). Clear the pointer
  // so we don't try to log to a dangling pointer (#1885).
  m_document->setParentLogger(nullptr);

  m_mapView->deactivateTool();

  m_notifierConnection.disconnect();
  removeRecentDocumentsMenu();

  // The order of deletion here is important because both the document and the children
  // need the context manager (and its embedded VBO) to clean up their resources.

  // Destroy the children first because they might still access document resources.
  // The children must be deleted in reverse order!
  const auto children = this->children();
  qDeleteAll(std::rbegin(children), std::rend(children));

  // let's trigger a final autosave before releasing the document
  auto logger = NullLogger{};
  m_autosaver->triggerAutosave(logger);

  m_document->setViewEffectsService(nullptr);
  m_document.reset();

  // FIXME: m_contextManager is deleted via smart pointer; it may release openGL resources
  // in its destructor
}

void MapFrame::positionOnScreen(QWidget* reference)
{
  restoreWindowGeometry(this);
  restoreWindowState(this);
  if (reference)
  {
    const auto offset = QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
    move(reference->pos() + QPoint(offset, offset));
  }
}

std::shared_ptr<MapDocument> MapFrame::document() const
{
  return m_document;
}

Logger& MapFrame::logger() const
{
  return *m_console;
}

QAction* MapFrame::findAction(const std::filesystem::path& path)
{
  const auto& actionManager = ActionManager::instance();
  auto& actionsMap = actionManager.actionsMap();
  if (const auto iAction = actionsMap.find(path); iAction != std::end(actionsMap))
  {
    const auto& action = iAction->second;
    if (const auto iQAction = m_actionMap.find(&action);
        iQAction != std::end(m_actionMap))
    {
      return iQAction->second;
    }
  }
  return nullptr;
}

void MapFrame::updateTitle()
{
  setWindowModified(m_document->modified());
  setWindowTitle(
    QString::fromStdString(m_document->filename()) + QString("[*] - TrenchBroom"));
  setWindowFilePath(io::pathAsQString(m_document->path()));
}

void MapFrame::updateTitleDelayed()
{
  m_updateTitleSignalDelayer->queueSignal();
}

void MapFrame::createMenus()
{
  auto createMenuResult =
    populateMenuBar(*menuBar(), m_actionMap, [&](const Action& action) {
      auto context = ActionExecutionContext{this, currentMapViewBase()};
      action.execute(context);
    });

  m_recentDocumentsMenu = createMenuResult.recentDocumentsMenu;
  m_undoAction = createMenuResult.undoAction;
  m_redoAction = createMenuResult.redoAction;

  addRecentDocumentsMenu();
}

void MapFrame::updateShortcuts()
{
  for (auto [tbAction, qtAction] : m_actionMap)
  {
    updateActionKeySequence(*qtAction, *tbAction);
  }
}

void MapFrame::updateActionState()
{
  auto context = ActionExecutionContext{this, currentMapViewBase()};
  for (auto [tAction, qAction] : m_actionMap)
  {
    if (qAction == m_undoAction || qAction == m_redoAction)
    {
      // These are handled specially for performance reasons.
      continue;
    }
    qAction->setEnabled(tAction->enabled(context));
    if (qAction->isCheckable())
    {
      qAction->setChecked(tAction->checked(context));
    }
  }
}

void MapFrame::updateActionStateDelayed()
{
  m_updateActionStateSignalDelayer->queueSignal();
}

void MapFrame::updateUndoRedoActions()
{
  const auto document = kdl::mem_lock(m_document);
  if (m_undoAction)
  {
    if (document->canUndoCommand())
    {
      const auto text = "Undo " + document->undoCommandName();
      m_undoAction->setText(QString::fromStdString(text));
      m_undoAction->setEnabled(true);
    }
    else
    {
      m_undoAction->setText("Undo");
      m_undoAction->setEnabled(false);
    }
  }
  if (m_redoAction)
  {
    if (document->canRedoCommand())
    {
      const auto text = "Redo " + document->redoCommandName();
      m_redoAction->setText(QString::fromStdString(text));
      m_redoAction->setEnabled(true);
    }
    else
    {
      m_redoAction->setText("Redo");
      m_redoAction->setEnabled(false);
    }
  }
}

void MapFrame::addRecentDocumentsMenu()
{
  auto& app = TrenchBroomApp::instance();
  app.addRecentDocumentMenu(*m_recentDocumentsMenu);
}

void MapFrame::removeRecentDocumentsMenu()
{
  auto& app = TrenchBroomApp::instance();
  app.removeRecentDocumentMenu(*m_recentDocumentsMenu);
}

void MapFrame::updateRecentDocumentsMenu()
{
  if (m_document->path().is_absolute())
  {
    auto& app = TrenchBroomApp::instance();
    app.updateRecentDocument(m_document->path());
  }
}

void MapFrame::createGui()
{
  setWindowIconTB(this);
  setWindowTitle("TrenchBroom");

  m_hSplitter = new Splitter{Qt::Horizontal, DrawKnob::No};
  m_hSplitter->setChildrenCollapsible(false);
  m_hSplitter->setObjectName("MapFrame_HorizontalSplitter");

  m_vSplitter = new Splitter{Qt::Vertical, DrawKnob::No};
  m_vSplitter->setChildrenCollapsible(false);
  m_vSplitter->setObjectName("MapFrame_VerticalSplitterSplitter");

  m_infoPanel = new InfoPanel{m_document};
  m_console = m_infoPanel->console();

  m_mapView = new SwitchableMapViewContainer{m_document, *m_contextManager};
  m_currentMapView = m_mapView->firstMapViewBase();
  ensure(
    m_currentMapView, "SwitchableMapViewContainer should have constructed a MapViewBase");

  m_inspector = new Inspector{m_document, *m_contextManager};

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

  auto* frameLayout = new QVBoxLayout{};
  frameLayout->setContentsMargins(0, 0, 0, 0);
  frameLayout->addWidget(m_hSplitter);

  // NOTE: you can't set the layout of a QMainWindow, so make another widget to wrap this
  // layout in
  auto* layoutWrapper = new QWidget{};
  layoutWrapper->setLayout(frameLayout);

  setCentralWidget(layoutWrapper);

  restoreWindowState(m_hSplitter);
  restoreWindowState(m_vSplitter);
}

void MapFrame::createToolBar()
{
  m_toolBar = addToolBar("Toolbar");
  m_toolBar->setObjectName("MapFrameToolBar");
  m_toolBar->setFloatable(false);
  m_toolBar->setMovable(false);
  // macOS Qt bug: with the 32x32 default icon size, 24x24 highdpi icons get scaled up to
  // 32x32. We expect them to be drawn at 24x24 logical pixels centered in a 32x32 box, as
  // is the case with non-highdpi icons. As a workaround, just lower the toolbar size to
  // 24x24 (we could alternatively render the icons at 32x32).
  m_toolBar->setIconSize(QSize(24, 24));

  populateToolBar(*m_toolBar, m_actionMap, [&](const auto& tbAction) {
    auto context = ActionExecutionContext{this, currentMapViewBase()};
    tbAction.execute(context);
  });

  m_gridChoice = new QComboBox{};
  for (int i = Grid::MinSize; i <= Grid::MaxSize; ++i)
  {
    const auto gridSize = Grid::actualSize(i);
    const auto gridSizeStr = tr("Grid %1").arg(QString::number(gridSize, 'g'));
    m_gridChoice->addItem(gridSizeStr, QVariant(i));
  }

  m_toolBar->addWidget(m_gridChoice);
}

void MapFrame::updateToolBarWidgets()
{
  const auto& grid = m_document->grid();
  const auto sizeIndex = grid.size() - Grid::MinSize;
  m_gridChoice->setCurrentIndex(sizeIndex);
}

void MapFrame::createStatusBar()
{
  m_statusBarLabel = new QLabel{};
  statusBar()->addWidget(m_statusBarLabel);
}

namespace
{
template <typename T>
const mdl::EntityNodeBase* commonEntityForNodeList(const std::vector<T*>& nodes)
{
  return !nodes.empty()
             && kdl::all_of(
               nodes,
               [&](const auto* node) {
                 return node->entity() == nodes.front()->entity();
               })
           ? nodes.front()->entity()
           : nullptr;
}

std::optional<std::string> commonClassnameForEntityList(
  const std::vector<mdl::EntityNode*>& nodes)
{
  return !nodes.empty()
             && kdl::all_of(
               nodes,
               [&](const auto* entityNode) {
                 return entityNode->entity().classname()
                        == nodes.front()->entity().classname();
               })
           ? std::optional{nodes.front()->entity().classname()}
           : std::nullopt;
}

std::string numberWithSuffix(
  size_t count, const std::string& singular, const std::string& plural)
{
  return std::to_string(count) + " " + kdl::str_plural(count, singular, plural);
}

QString describeSelection(const MapDocument& document)
{
  const auto Arrow = QString(" ") + QString(QChar(0x203A)) + QString(" ");

  auto pipeSeparatedSections = QStringList{};

  pipeSeparatedSections << QString::fromStdString(document.game()->config().name)
                        << QString::fromStdString(
                             mdl::formatName(document.world()->mapFormat()))
                        << QString::fromStdString(document.currentLayer()->name());

  // open groups
  auto groups = std::vector<mdl::GroupNode*>{};
  for (auto* group = document.currentGroup(); group != nullptr;
       group = group->containingGroup())
  {
    groups.push_back(group);
  }

  if (!groups.empty())
  {
    auto openGroups = QStringList{};

    // groups vector is sorted from innermost to outermost, so visit it in reverse order
    for (auto it = groups.rbegin(); it != groups.rend(); ++it)
    {
      const auto* group = *it;
      openGroups << QString::fromStdString(group->name());
    }

    const auto openGroupsString =
      QObject::tr("Open groups: %1").arg(openGroups.join(Arrow));
    pipeSeparatedSections << openGroupsString;
  }

  // build a vector of strings describing the things that are selected
  auto tokens = std::vector<std::string>{};

  const auto& selectedNodes = document.selectedNodes();

  // selected brushes
  if (!selectedNodes.brushes().empty())
  {
    const auto* commonEntityNode = commonEntityForNodeList(selectedNodes.brushes());

    // if all selected brushes are from the same entity, print the entity name
    auto token = numberWithSuffix(selectedNodes.brushes().size(), "brush", "brushes");
    token += commonEntityNode ? " (" + commonEntityNode->entity().classname() + ")"
                              : " (multiple entities)";
    tokens.push_back(token);
  }

  // selected patches
  if (!selectedNodes.patches().empty())
  {
    const auto* commonEntityNode = commonEntityForNodeList(selectedNodes.patches());

    // if all selected patches are from the same entity, print the entity name
    auto token = numberWithSuffix(selectedNodes.patches().size(), "patch", "patches");
    token += commonEntityNode ? " (" + commonEntityNode->entity().classname() + ")"
                              : " (multiple entities)";
    tokens.push_back(token);
  }

  // selected brush faces
  if (document.hasSelectedBrushFaces())
  {
    const auto token =
      numberWithSuffix(document.selectedBrushFaces().size(), "face", "faces");
    tokens.push_back(token);
  }

  // entities
  if (!selectedNodes.entities().empty())
  {
    const auto commonClassname = commonClassnameForEntityList(selectedNodes.entities());

    auto token = numberWithSuffix(selectedNodes.entities().size(), "entity", "entities");
    token += " (" + commonClassname.value_or("multiple classnames") + ") ";
    tokens.push_back(token);
  }

  // groups
  if (!selectedNodes.groups().empty())
  {
    tokens.push_back(numberWithSuffix(selectedNodes.groups().size(), "group", "groups"));
  }

  // get the layers of the selected nodes
  const auto selectedObjectLayers =
    mdl::collectContainingLayersUserSorted(selectedNodes.nodes());
  auto layersDescription = QString{};
  if (selectedObjectLayers.size() == 1)
  {
    auto* layer = selectedObjectLayers.front();
    layersDescription =
      QObject::tr(" in layer \"%1\"").arg(QString::fromStdString(layer->name()));
  }
  else if (selectedObjectLayers.size() > 1)
  {
    layersDescription = QObject::tr(" in %1 layers").arg(selectedObjectLayers.size());
  }

  // now, turn `tokens` into a comma-separated string
  if (!tokens.empty())
  {
    pipeSeparatedSections << QObject::tr("%1%2 selected")
                               .arg(QString::fromStdString(
                                 kdl::str_join(tokens, ", ", ", and ", " and ")))
                               .arg(layersDescription);
  }

  // count hidden objects
  size_t hiddenGroups = 0u;
  size_t hiddenEntities = 0u;
  size_t hiddenBrushes = 0u;
  size_t hiddenPatches = 0u;

  const auto& editorContext = document.editorContext();
  document.world()->accept(kdl::overload(
    [](auto&& thisLambda, const mdl::WorldNode* world) {
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const mdl::LayerNode* layer) {
      layer->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, const mdl::GroupNode* group) {
      if (!editorContext.visible(group))
      {
        ++hiddenGroups;
      }
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, const mdl::EntityNode* entity) {
      if (!editorContext.visible(entity))
      {
        ++hiddenEntities;
      }
      entity->visitChildren(thisLambda);
    },
    [&](const mdl::BrushNode* brush) {
      if (!editorContext.visible(brush))
      {
        ++hiddenBrushes;
      }
    },
    [&](const mdl::PatchNode* patch) {
      if (!editorContext.visible(patch))
      {
        ++hiddenPatches;
      }
    }));

  // print hidden objects
  if (hiddenGroups > 0 || hiddenEntities > 0 || hiddenBrushes > 0)
  {
    auto hiddenDescriptors = std::vector<std::string>{};

    if (hiddenGroups > 0)
    {
      hiddenDescriptors.push_back(numberWithSuffix(hiddenGroups, "group", "groups"));
    }
    if (hiddenEntities > 0)
    {
      hiddenDescriptors.push_back(numberWithSuffix(hiddenEntities, "entity", "entities"));
    }
    if (hiddenBrushes > 0)
    {
      hiddenDescriptors.push_back(numberWithSuffix(hiddenBrushes, "brush", "brushes"));
    }
    if (hiddenPatches > 0)
    {
      hiddenDescriptors.push_back(numberWithSuffix(hiddenPatches, "patch", "patches"));
    }

    pipeSeparatedSections << QObject::tr("%1 hidden")
                               .arg(QString::fromStdString(kdl::str_join(
                                 hiddenDescriptors, ", ", ", and ", " and ")));
  }

  return QString::fromLatin1("   ")
         + pipeSeparatedSections.join(QLatin1String("   |   "));
}

} // namespace

void MapFrame::updateStatusBar()
{
  m_statusBarLabel->setText(QString{describeSelection(*m_document)});
}

void MapFrame::updateStatusBarDelayed()
{
  m_updateStatusBarSignalDelayer->queueSignal();
}

void MapFrame::connectObservers()
{
  auto& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &MapFrame::preferenceDidChange);

  m_notifierConnection +=
    m_document->documentWasClearedNotifier.connect(this, &MapFrame::documentWasCleared);
  m_notifierConnection +=
    m_document->documentWasNewedNotifier.connect(this, &MapFrame::documentDidChange);
  m_notifierConnection +=
    m_document->documentWasLoadedNotifier.connect(this, &MapFrame::documentDidChange);
  m_notifierConnection +=
    m_document->documentWasSavedNotifier.connect(this, &MapFrame::documentDidChange);
  m_notifierConnection += m_document->documentModificationStateDidChangeNotifier.connect(
    this, &MapFrame::documentModificationStateDidChange);
  m_notifierConnection +=
    m_document->transactionDoneNotifier.connect(this, &MapFrame::transactionDone);
  m_notifierConnection +=
    m_document->transactionUndoneNotifier.connect(this, &MapFrame::transactionUndone);
  m_notifierConnection +=
    m_document->selectionDidChangeNotifier.connect(this, &MapFrame::selectionDidChange);
  m_notifierConnection += m_document->currentLayerDidChangeNotifier.connect(
    this, &MapFrame::currentLayerDidChange);
  m_notifierConnection +=
    m_document->groupWasOpenedNotifier.connect(this, &MapFrame::groupWasOpened);
  m_notifierConnection +=
    m_document->groupWasClosedNotifier.connect(this, &MapFrame::groupWasClosed);
  m_notifierConnection += m_document->nodeVisibilityDidChangeNotifier.connect(
    this, &MapFrame::nodeVisibilityDidChange);
  m_notifierConnection += m_document->editorContextDidChangeNotifier.connect(
    this, &MapFrame::editorContextDidChange);
  m_notifierConnection +=
    m_document->pointFileWasLoadedNotifier.connect(this, &MapFrame::pointFileDidChange);
  m_notifierConnection +=
    m_document->pointFileWasUnloadedNotifier.connect(this, &MapFrame::pointFileDidChange);
  m_notifierConnection +=
    m_document->portalFileWasLoadedNotifier.connect(this, &MapFrame::portalFileDidChange);
  m_notifierConnection += m_document->portalFileWasUnloadedNotifier.connect(
    this, &MapFrame::portalFileDidChange);

  Grid& grid = m_document->grid();
  m_notifierConnection +=
    grid.gridDidChangeNotifier.connect(this, &MapFrame::gridDidChange);

  m_notifierConnection += m_mapView->mapViewToolBox().toolActivatedNotifier.connect(
    this, &MapFrame::toolActivated);
  m_notifierConnection += m_mapView->mapViewToolBox().toolDeactivatedNotifier.connect(
    this, &MapFrame::toolDeactivated);
  m_notifierConnection +=
    m_mapView->mapViewToolBox().toolHandleSelectionChangedNotifier.connect(
      this, &MapFrame::toolHandleSelectionChanged);
}

void MapFrame::documentWasCleared(ui::MapDocument*)
{
  updateTitle();
  updateActionState();
  updateUndoRedoActions();
}

void MapFrame::documentDidChange(ui::MapDocument*)
{
  updateTitle();
  updateActionState();
  updateUndoRedoActions();
  updateRecentDocumentsMenu();
}

void MapFrame::documentModificationStateDidChange()
{
  updateTitleDelayed();
}

void MapFrame::transactionDone(const std::string& /* name */)
{
  QTimer::singleShot(0, this, [this]() {
    // FIXME: Delaying this with QTimer::singleShot is a hack to work around the lack of
    // a notification that's called _after_ the CommandProcessor undo/redo stacks are
    // modified.
    //
    // The current transactionDoneNotifier is called after the transaction executes, but
    // before it's pushed onto the undo stack, but we need to read the undo stack in
    // updateUndoRedoActions(), so this QTimer::singleShot is needed for now.
    updateUndoRedoActions();
  });
}

void MapFrame::transactionUndone(const std::string& /* name */)
{
  QTimer::singleShot(0, this, [this]() {
    // FIXME: see MapFrame::transactionDone
    updateUndoRedoActions();
  });
}

void MapFrame::preferenceDidChange(const std::filesystem::path& path)
{
  if (path == Preferences::MapViewLayout.path())
  {
    m_mapView->switchToMapView(
      static_cast<MapViewLayout>(pref(Preferences::MapViewLayout)));
  }

  updateShortcuts();
}

void MapFrame::gridDidChange()
{
  updateActionStateDelayed();
  updateToolBarWidgets();
}

void MapFrame::toolActivated(Tool&)
{
  updateActionStateDelayed();
}

void MapFrame::toolDeactivated(Tool&)
{
  updateActionStateDelayed();
}

void MapFrame::toolHandleSelectionChanged(Tool&)
{
  updateActionStateDelayed();
}

void MapFrame::selectionDidChange(const Selection&)
{
  updateActionStateDelayed();
  updateStatusBarDelayed();
}

void MapFrame::currentLayerDidChange(const tb::mdl::LayerNode*)
{
  updateStatusBarDelayed();
}

void MapFrame::groupWasOpened(mdl::GroupNode*)
{
  updateStatusBarDelayed();
}

void MapFrame::groupWasClosed(mdl::GroupNode*)
{
  updateStatusBarDelayed();
}

void MapFrame::nodeVisibilityDidChange(const std::vector<mdl::Node*>&)
{
  updateStatusBarDelayed();
}

void MapFrame::editorContextDidChange()
{
  // e.g. changing the view filters may cause the number of hidden brushes/entities to
  // change
  updateStatusBarDelayed();
}

void MapFrame::pointFileDidChange()
{
  updateActionStateDelayed();
}

void MapFrame::portalFileDidChange()
{
  updateActionStateDelayed();
}

void MapFrame::bindEvents()
{
  connect(m_autosaveTimer, &QTimer::timeout, this, &MapFrame::triggerAutosave);
  connect(
    m_processResourcesTimer, &QTimer::timeout, this, &MapFrame::triggerProcessResources);
  connect(qApp, &QApplication::focusChanged, this, &MapFrame::focusChange);
  connect(
    m_gridChoice,
    QOverload<int>::of(&QComboBox::activated),
    this,
    [this](const int index) { setGridSize(index + Grid::MinSize); });
  connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [this]() {
    // update the "Paste" menu items
    this->updateActionState();
  });
  connect(
    m_toolBar, &QToolBar::visibilityChanged, this, [this](const bool /* visible */) {
      // update the "Toggle Toolbar" menu item
      this->updateActionState();
    });

  connect(
    m_updateTitleSignalDelayer,
    &SignalDelayer::processSignal,
    this,
    &MapFrame::updateTitle);
  connect(
    m_updateActionStateSignalDelayer,
    &SignalDelayer::processSignal,
    this,
    &MapFrame::updateActionState);
  connect(
    m_updateStatusBarSignalDelayer,
    &SignalDelayer::processSignal,
    this,
    &MapFrame::updateStatusBar);
}

Result<bool> MapFrame::newDocument(
  std::shared_ptr<mdl::Game> game, const mdl::MapFormat mapFormat)
{
  if (!confirmOrDiscardChanges() || !closeCompileDialog())
  {
    return false;
  }

  return m_document->newDocument(mapFormat, MapDocument::DefaultWorldBounds, game)
         | kdl::transform([]() { return true; });
}

Result<bool> MapFrame::openDocument(
  std::shared_ptr<mdl::Game> game,
  const mdl::MapFormat mapFormat,
  const std::filesystem::path& path)
{
  if (!confirmOrDiscardChanges() || !closeCompileDialog())
  {
    return false;
  }

  const auto startTime = std::chrono::high_resolution_clock::now();
  return m_document->loadDocument(mapFormat, MapDocument::DefaultWorldBounds, game, path)
         | kdl::transform([&]() {
             const auto endTime = std::chrono::high_resolution_clock::now();

             logger().info() << "Loaded " << m_document->path() << " in "
                             << std::chrono::duration_cast<std::chrono::milliseconds>(
                                  endTime - startTime)
                                  .count()
                             << "ms";

             return true;
           });
}

bool MapFrame::saveDocument()
{
  try
  {
    if (m_document->persistent())
    {
      const auto startTime = std::chrono::high_resolution_clock::now();
      m_document->saveDocument();
      const auto endTime = std::chrono::high_resolution_clock::now();

      logger().info() << "Saved " << m_document->path() << " in "
                      << std::chrono::duration_cast<std::chrono::milliseconds>(
                           endTime - startTime)
                           .count()
                      << "ms";
      return true;
    }
    return saveDocumentAs();
  }
  catch (...)
  {
    QMessageBox::critical(
      this,
      "",
      QString::fromStdString("Unknown error while saving " + m_document->path().string()),
      QMessageBox::Ok);
    return false;
  }
}

bool MapFrame::saveDocumentAs()
{
  try
  {
    const auto& originalPath = m_document->path();
    const auto directory = originalPath.parent_path();
    const auto fileName = originalPath.filename();

    const auto newFileName = QFileDialog::getSaveFileName(
      this, tr("Save map file"), io::pathAsQString(originalPath), "Map files (*.map)");
    if (newFileName.isEmpty())
    {
      return false;
    }

    const auto path = io::pathFromQString(newFileName);

    const auto startTime = std::chrono::high_resolution_clock::now();
    m_document->saveDocumentAs(path);
    const auto endTime = std::chrono::high_resolution_clock::now();

    logger().info() << "Saved " << m_document->path() << " in "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(
                         endTime - startTime)
                         .count()
                    << "ms";
    return true;
  }
  catch (...)
  {
    QMessageBox::critical(
      this,
      "",
      QString::fromStdString("Unknown error while saving " + m_document->filename()),
      QMessageBox::Ok);
    return false;
  }
}

void MapFrame::revertDocument()
{
  if (m_document->persistent() && confirmRevertDocument())
  {
    const auto mapFormat = m_document->world()->mapFormat();
    const auto game = m_document->game();
    const auto path = m_document->path();
    m_document->loadDocument(mapFormat, MapDocument::DefaultWorldBounds, game, path)
      | kdl::transform_error(
        [&](auto e) { m_document->error() << "Failed to rever document: " << e.msg; });
  }
}

bool MapFrame::exportDocumentAsObj()
{
  if (!m_objExportDialog)
  {
    m_objExportDialog = new ObjExportDialog{this};
  }

  m_objExportDialog->updateExportPath();
  showModelessDialog(m_objExportDialog);
  return true;
}

bool MapFrame::exportDocumentAsMap()
{
  const auto& originalPath = m_document->path();

  const auto newFileName = QFileDialog::getSaveFileName(
    this, tr("Export Map file"), io::pathAsQString(originalPath), "Map files (*.map)");
  if (newFileName.isEmpty())
  {
    return false;
  }

  const auto options = io::MapExportOptions{io::pathFromQString(newFileName)};
  return exportDocument(options);
}

bool MapFrame::exportDocument(const io::ExportOptions& options)
{
  const auto exportPath = std::visit([](const auto& o) { return o.exportPath; }, options);

  if (exportPath == m_document->path())
  {
    QMessageBox::critical(
      this,
      "",
      tr("You can't overwrite the current document.\nPlease choose a different file name "
         "to export "
         "to."));
    return false;
  }

  return m_document->exportDocumentAs(options) | kdl::transform([&]() {
           logger().info() << "Exported " << exportPath;
           return true;
         })
         | kdl::transform_error([&](auto e) {
             logger().error() << "Could not export '" << exportPath << "': " + e.msg;
             QMessageBox::critical(this, "", QString::fromStdString(e.msg));
             return false;
           })
         | kdl::value();
}

/**
 * Returns whether the window should close.
 */
bool MapFrame::confirmOrDiscardChanges()
{
  if (!m_document->modified())
  {
    return true;
  }

  const auto result = QMessageBox::question(
    this,
    "TrenchBroom",
    QString::fromStdString(
      m_document->filename() + " has been modified. Do you want to save the changes?"),
    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

  if (result == QMessageBox::Yes)
  {
    return saveDocument();
  }
  return result == QMessageBox::No;
}

/**
 * Returns whether the document should be reverted.
 */
bool MapFrame::confirmRevertDocument()
{
  if (!m_document->modified())
  {
    return true;
  }

  auto messageBox = QMessageBox{this};
  messageBox.setWindowTitle("TrenchBroom");
  messageBox.setIcon(QMessageBox::Question);
  messageBox.setText(tr("Revert %1 to %2?")
                       .arg(QString::fromStdString(m_document->filename()))
                       .arg(io::pathAsQString(m_document->path())));
  messageBox.setInformativeText(
    tr("This will discard all unsaved changes and reload the document from disk."));

  auto* revertButton = messageBox.addButton(tr("Revert"), QMessageBox::DestructiveRole);
  auto* cancelButton = messageBox.addButton(QMessageBox::Cancel);
  messageBox.setDefaultButton(cancelButton);

  messageBox.exec();

  return messageBox.clickedButton() == revertButton;
}

void MapFrame::loadPointFile()
{
  const auto defaultDir = !m_document->path().empty()
                            ? io::pathAsQString(m_document->path().parent_path())
                            : QString{};

  const auto fileName = QFileDialog::getOpenFileName(
    this,
    tr("Load Point File"),
    defaultDir,
    "Point files (*.pts *.lin);;Any files (*.*)");

  if (!fileName.isEmpty())
  {
    m_document->loadPointFile(io::pathFromQString(fileName));
  }
}

void MapFrame::reloadPointFile()
{
  if (canReloadPointFile())
  {
    m_document->reloadPointFile();
  }
}

void MapFrame::unloadPointFile()
{
  if (canUnloadPointFile())
  {
    m_document->unloadPointFile();
  }
}

bool MapFrame::canUnloadPointFile() const
{
  return m_document->isPointFileLoaded();
}

bool MapFrame::canReloadPointFile() const
{
  return m_document->canReloadPointFile();
}

void MapFrame::loadPortalFile()
{
  const auto defaultDir = !m_document->path().empty()
                            ? io::pathAsQString(m_document->path().parent_path())
                            : QString{};

  const auto fileName = QFileDialog::getOpenFileName(
    this, tr("Load Portal File"), defaultDir, "Portal files (*.prt);;Any files (*.*)");

  if (!fileName.isEmpty())
  {
    m_document->loadPortalFile(io::pathFromQString(fileName));
  }
}

void MapFrame::reloadPortalFile()
{
  if (canReloadPortalFile())
  {
    m_document->reloadPortalFile();
  }
}

void MapFrame::unloadPortalFile()
{
  if (canUnloadPortalFile())
  {
    m_document->unloadPortalFile();
  }
}

bool MapFrame::canUnloadPortalFile() const
{
  return m_document->isPortalFileLoaded();
}

bool MapFrame::canReloadPortalFile() const
{
  return m_document->canReloadPortalFile();
}

void MapFrame::reloadMaterialCollections()
{
  m_document->reloadMaterialCollections();
}

void MapFrame::reloadEntityDefinitions()
{
  m_document->reloadEntityDefinitions();
}

void MapFrame::closeDocument()
{
  close();
}

void MapFrame::undo()
{
  if (canUndo() && !m_mapView->cancelMouseDrag() && !m_inspector->cancelMouseDrag())
  {
    m_document->undoCommand();
  }
}

void MapFrame::redo()
{
  if (canRedo())
  {
    m_document->redoCommand();
  }
}

bool MapFrame::canUndo() const
{
  return m_document->canUndoCommand();
}

bool MapFrame::canRedo() const
{
  return m_document->canRedoCommand();
}

void MapFrame::repeatLastCommands()
{
  m_document->repeatCommands();
}

void MapFrame::clearRepeatableCommands()
{
  if (hasRepeatableCommands())
  {
    m_document->clearRepeatableCommands();
  }
}

bool MapFrame::hasRepeatableCommands() const
{
  return m_document->canRepeatCommands();
}

void MapFrame::cutSelection()
{
  if (canCutSelection())
  {
    copyToClipboard();
    auto transaction = Transaction{m_document, "Cut"};
    m_document->deleteObjects();
    transaction.commit();
  }
}

void MapFrame::copySelection()
{
  if (canCopySelection())
  {
    copyToClipboard();
  }
}

void MapFrame::copyToClipboard()
{
  const auto str = m_document->hasSelectedNodes() ? m_document->serializeSelectedNodes()
                   : m_document->hasSelectedBrushFaces()
                     ? m_document->serializeSelectedBrushFaces()
                     : std::string{};

  auto* clipboard = QApplication::clipboard();
  clipboard->setText(mapStringToUnicode(m_document->encoding(), str));
}

bool MapFrame::canCutSelection() const
{
  return widgetOrChildHasFocus(m_mapView) && m_document->hasSelectedNodes()
         && !m_mapView->anyToolActive();
}

bool MapFrame::canCopySelection() const
{
  return widgetOrChildHasFocus(m_mapView)
         && (m_document->hasSelectedNodes() || m_document->hasSelectedBrushFaces());
}

void MapFrame::pasteAtCursorPosition()
{
  if (canPaste())
  {
    const auto referenceBounds = m_document->referenceBounds();

    auto transaction = Transaction{m_document, "Paste"};
    switch (paste())
    {
    case PasteType::Node:
      if (m_document->hasSelectedNodes())
      {
        const auto bounds = m_document->selectionBounds();

        // The pasted objects must be hidden to prevent the picking done in
        // pasteObjectsDelta from hitting them
        // (https://github.com/TrenchBroom/TrenchBroom/issues/2755)
        const auto nodes = m_document->selectedNodes().nodes();

        m_document->hide(nodes);
        const auto delta = m_mapView->pasteObjectsDelta(bounds, referenceBounds);
        m_document->show(nodes);
        m_document->selectNodes(nodes); // Hiding deselected the nodes, so reselect them
        if (!m_document->translateObjects(delta))
        {
          transaction.cancel();
          break;
        }
      }
      transaction.commit();
      break;
    case PasteType::BrushFace:
      transaction.commit();
      break;
    case PasteType::Failed:
      transaction.cancel();
      break;
    }
  }
}

void MapFrame::pasteAtOriginalPosition()
{
  if (canPaste())
  {
    paste();
  }
}

PasteType MapFrame::paste()
{
  auto* clipboard = QApplication::clipboard();
  const auto qtext = clipboard->text();

  if (qtext.isEmpty())
  {
    logger().error("Clipboard is empty");
    return PasteType::Failed;
  }

  return m_document->paste(mapStringFromUnicode(m_document->encoding(), qtext));
}

/**
 * This is relatively expensive so only call it when the clipboard changes or e.g. the
 * user tries to paste.
 */
bool MapFrame::canPaste() const
{
  if (!widgetOrChildHasFocus(m_mapView) || !m_mapView->isCurrent())
  {
    return false;
  }

  const auto* clipboard = QApplication::clipboard();
  const auto* mimeData = clipboard->mimeData();
  return mimeData && mimeData->hasText();
}

void MapFrame::duplicateSelection()
{
  if (canDuplicateSelectino())
  {
    m_document->duplicateObjects();
  }
}

bool MapFrame::canDuplicateSelectino() const
{
  return m_document->hasSelectedNodes();
}

void MapFrame::deleteSelection()
{
  if (canDeleteSelection())
  {
    if (m_mapView->clipToolActive())
    {
      m_mapView->clipTool().removeLastPoint();
    }
    else if (m_mapView->vertexToolActive())
    {
      m_mapView->vertexTool().removeSelection();
    }
    else if (m_mapView->edgeToolActive())
    {
      m_mapView->edgeTool().removeSelection();
    }
    else if (m_mapView->faceToolActive())
    {
      m_mapView->faceTool().removeSelection();
    }
    else if (!m_mapView->anyToolActive())
    {
      m_document->deleteObjects();
    }
  }
}

bool MapFrame::canDeleteSelection() const
{
  if (m_mapView->clipToolActive())
  {
    return m_mapView->clipTool().canRemoveLastPoint();
  }
  if (m_mapView->vertexToolActive())
  {
    return m_mapView->vertexTool().canRemoveSelection();
  }
  if (m_mapView->edgeToolActive())
  {
    return m_mapView->edgeTool().canRemoveSelection();
  }
  if (m_mapView->faceToolActive())
  {
    return m_mapView->faceTool().canRemoveSelection();
  }
  return canCutSelection();
}

void MapFrame::selectAll()
{
  if (canSelect())
  {
    m_document->selectAllNodes();
  }
}

void MapFrame::selectSiblings()
{
  if (canSelectSiblings())
  {
    m_document->selectSiblings();
  }
}

void MapFrame::selectTouching()
{
  if (canSelectByBrush())
  {
    m_document->selectTouching(true);
  }
}

void MapFrame::selectInside()
{
  if (canSelectByBrush())
  {
    m_document->selectInside(true);
  }
}

void MapFrame::selectTall()
{
  if (canSelectTall())
  {
    m_mapView->selectTall();
  }
}

void MapFrame::selectByLineNumber()
{
  if (canSelect())
  {
    const auto string = QInputDialog::getText(
      this,
      "Select by Line Numbers",
      "Enter a comma- or space separated list of line numbers.");
    if (!string.isEmpty())
    {
      auto positions = std::vector<size_t>{};
      for (const auto& token : string.split(QRegularExpression{"[, ]"}))
      {
        bool ok;
        const auto position = token.toLong(&ok);
        if (ok && position > 0)
        {
          positions.push_back(static_cast<size_t>(position));
        }
      }

      m_document->selectNodesWithFilePosition(positions);
    }
  }
}

void MapFrame::selectInverse()
{
  if (canSelectInverse())
  {
    m_document->selectInverse();
  }
}

void MapFrame::selectNone()
{
  if (canDeselect())
  {
    m_document->deselectAll();
  }
}

bool MapFrame::canSelect() const
{
  return canChangeSelection();
}

bool MapFrame::canSelectSiblings() const
{
  return canChangeSelection() && m_document->hasSelectedNodes();
}

bool MapFrame::canSelectByBrush() const
{
  return canChangeSelection() && m_document->selectedNodes().hasOnlyBrushes();
}

bool MapFrame::canSelectTall() const
{
  return canChangeSelection() && m_document->selectedNodes().hasOnlyBrushes()
         && m_mapView->canSelectTall();
}

bool MapFrame::canDeselect() const
{
  return canChangeSelection() && m_document->hasSelectedNodes();
}

bool MapFrame::canChangeSelection() const
{
  return m_document->editorContext().canChangeSelection();
}

bool MapFrame::canSelectInverse() const
{
  return m_document->editorContext().canChangeSelection();
}

void MapFrame::groupSelectedObjects()
{
  if (canGroupSelectedObjects())
  {
    const auto name = queryGroupName(this, "Unnamed");
    if (!name.empty())
    {
      m_document->groupSelection(name);
    }
  }
}

bool MapFrame::canGroupSelectedObjects() const
{
  return m_document->hasSelectedNodes() && !m_mapView->anyToolActive();
}

void MapFrame::ungroupSelectedObjects()
{
  if (canUngroupSelectedObjects())
  {
    m_document->ungroupSelection();
  }
}

bool MapFrame::canUngroupSelectedObjects() const
{
  return m_document->selectedNodes().hasGroups() && !m_mapView->anyToolActive();
}

void MapFrame::renameSelectedGroups()
{
  if (canRenameSelectedGroups())
  {
    auto document = kdl::mem_lock(m_document);
    assert(document->selectedNodes().hasOnlyGroups());

    const auto suggestion = document->selectedNodes().groups().front()->name();
    const auto name = queryGroupName(this, suggestion);
    if (!name.empty())
    {
      document->renameGroups(name);
    }
  }
}

bool MapFrame::canRenameSelectedGroups() const
{
  auto document = kdl::mem_lock(m_document);
  return document->selectedNodes().hasOnlyGroups();
}

void MapFrame::replaceMaterial()
{
  auto dialog = ReplaceMaterialDialog{m_document, *m_contextManager, this};
  dialog.exec();
}

void MapFrame::moveSelectedObjects()
{
  auto ok = false;
  const auto str = QInputDialog::getText(
    this,
    "Move Objects",
    "Enter coordinates: X Y Z",
    QLineEdit::Normal,
    "0.0 0.0 0.0",
    &ok);
  if (ok)
  {
    if (const auto offset = vm::parse<double, 3>(str.toStdString()))
    {
      m_document->translateObjects(*offset);
    }
    else
    {
      QMessageBox::warning(this, "Error", tr("Invalid coordinates: '%1'").arg(str));
    }
  }
}

bool MapFrame::canMoveSelectedObjects() const
{
  return m_document->hasSelectedNodes() && !m_mapView->anyToolActive();
}

bool MapFrame::anyToolActive() const
{
  return m_mapView->anyToolActive();
}

void MapFrame::toggleAssembleBrushTool()
{
  if (canToggleAssembleBrushTool())
  {
    m_mapView->toggleAssembleBrushTool();
  }
}

bool MapFrame::canToggleAssembleBrushTool() const
{
  return m_mapView->canToggleAssembleBrushTool();
}

bool MapFrame::assembleBrushToolActive() const
{
  return m_mapView->assembleBrushToolActive();
}

void MapFrame::toggleClipTool()
{
  if (canToggleClipTool())
  {
    m_mapView->toggleClipTool();
  }
}

bool MapFrame::canToggleClipTool() const
{
  return m_mapView->canToggleClipTool();
}

bool MapFrame::clipToolActive() const
{
  return m_mapView->clipToolActive();
}

void MapFrame::toggleRotateObjectsTool()
{
  if (canToggleRotateObjectsTool())
  {
    m_mapView->toggleRotateObjectsTool();
  }
}

bool MapFrame::canToggleRotateObjectsTool() const
{
  return m_mapView->canToggleRotateObjectsTool();
}

bool MapFrame::rotateObjectsToolActive() const
{
  return m_mapView->rotateObjectsToolActive();
}

void MapFrame::toggleScaleObjectsTool()
{
  if (canToggleScaleObjectsTool())
  {
    m_mapView->toggleScaleObjectsTool();
  }
}

bool MapFrame::canToggleScaleObjectsTool() const
{
  return m_mapView->canToggleScaleObjectsTool();
}

bool MapFrame::scaleObjectsToolActive() const
{
  return m_mapView->scaleObjectsToolActive();
}

void MapFrame::toggleShearObjectsTool()
{
  if (canToggleShearObjectsTool())
  {
    m_mapView->toggleShearObjectsTool();
  }
}

bool MapFrame::canToggleShearObjectsTool() const
{
  return m_mapView->canToggleShearObjectsTool();
}

bool MapFrame::shearObjectsToolActive() const
{
  return m_mapView->shearObjectsToolActive();
}

bool MapFrame::anyVertexToolActive() const
{
  return vertexToolActive() || edgeToolActive() || faceToolActive();
}

void MapFrame::toggleVertexTool()
{
  if (canToggleVertexTool())
  {
    m_mapView->toggleVertexTool();
  }
}

bool MapFrame::canToggleVertexTool() const
{
  return m_mapView->canToggleVertexTools();
}

bool MapFrame::vertexToolActive() const
{
  return m_mapView->vertexToolActive();
}

void MapFrame::toggleEdgeTool()
{
  if (canToggleEdgeTool())
  {
    m_mapView->toggleEdgeTool();
  }
}

bool MapFrame::canToggleEdgeTool() const
{
  return m_mapView->canToggleVertexTools();
}

bool MapFrame::edgeToolActive() const
{
  return m_mapView->edgeToolActive();
}

void MapFrame::toggleFaceTool()
{
  if (canToggleFaceTool())
  {
    m_mapView->toggleFaceTool();
  }
}

bool MapFrame::canToggleFaceTool() const
{
  return m_mapView->canToggleVertexTools();
}

bool MapFrame::faceToolActive() const
{
  return m_mapView->faceToolActive();
}

void MapFrame::csgConvexMerge()
{
  if (canDoCsgConvexMerge())
  {
    if (m_mapView->vertexToolActive() && m_mapView->vertexTool().canDoCsgConvexMerge())
    {
      m_mapView->vertexTool().csgConvexMerge();
    }
    else if (m_mapView->edgeToolActive() && m_mapView->edgeTool().canDoCsgConvexMerge())
    {
      m_mapView->edgeTool().csgConvexMerge();
    }
    else if (m_mapView->faceToolActive() && m_mapView->faceTool().canDoCsgConvexMerge())
    {
      m_mapView->faceTool().csgConvexMerge();
    }
    else
    {
      m_document->csgConvexMerge();
    }
  }
}

bool MapFrame::canDoCsgConvexMerge() const
{
  return (m_document->hasSelectedBrushFaces()
          && m_document->selectedBrushFaces().size() > 1)
         || (m_document->selectedNodes().hasOnlyBrushes() && m_document->selectedNodes().brushCount() > 1)
         || (m_mapView->vertexToolActive() && m_mapView->vertexTool().canDoCsgConvexMerge())
         || (m_mapView->edgeToolActive() && m_mapView->edgeTool().canDoCsgConvexMerge())
         || (m_mapView->faceToolActive() && m_mapView->faceTool().canDoCsgConvexMerge());
}

void MapFrame::csgSubtract()
{
  if (canDoCsgSubtract())
  {
    m_document->csgSubtract();
  }
}

bool MapFrame::canDoCsgSubtract() const
{
  return m_document->selectedNodes().hasOnlyBrushes()
         && m_document->selectedNodes().brushCount() >= 1;
}

void MapFrame::csgHollow()
{
  if (canDoCsgHollow())
  {
    m_document->csgHollow();
  }
}

bool MapFrame::canDoCsgHollow() const
{
  return m_document->selectedNodes().hasOnlyBrushes()
         && m_document->selectedNodes().brushCount() >= 1;
}

void MapFrame::csgIntersect()
{
  if (canDoCsgIntersect())
  {
    m_document->csgIntersect();
  }
}

bool MapFrame::canDoCsgIntersect() const
{
  return m_document->selectedNodes().hasOnlyBrushes()
         && m_document->selectedNodes().brushCount() > 1;
}

void MapFrame::snapVerticesToInteger()
{
  if (canSnapVertices())
  {
    m_document->snapVertices(1u);
  }
}

void MapFrame::snapVerticesToGrid()
{
  if (canSnapVertices())
  {
    m_document->snapVertices(m_document->grid().actualSize());
  }
}

bool MapFrame::canSnapVertices() const
{
  return m_document->hasAnySelectedBrushNodes();
}

void MapFrame::toggleAlignmentLock()
{
  togglePref(Preferences::AlignmentLock);
}

void MapFrame::toggleUVLock()
{
  togglePref(Preferences::UVLock);
}

void MapFrame::toggleShowGrid()
{
  m_document->grid().toggleVisible();
}

void MapFrame::toggleSnapToGrid()
{
  m_document->grid().toggleSnap();
}

void MapFrame::incGridSize()
{
  if (canIncGridSize())
  {
    m_document->grid().incSize();
  }
}

bool MapFrame::canIncGridSize() const
{
  return m_document->grid().size() < Grid::MaxSize;
}

void MapFrame::decGridSize()
{
  if (canDecGridSize())
  {
    m_document->grid().decSize();
  }
}

bool MapFrame::canDecGridSize() const
{
  return m_document->grid().size() > Grid::MinSize;
}

void MapFrame::setGridSize(const int size)
{
  m_document->grid().setSize(size);
}

void MapFrame::moveCameraToNextPoint()
{
  if (canMoveCameraToNextPoint())
  {
    m_mapView->moveCameraToNextTracePoint();
  }
}

bool MapFrame::canMoveCameraToNextPoint() const
{
  return m_mapView->canMoveCameraToNextTracePoint();
}

void MapFrame::moveCameraToPreviousPoint()
{
  if (canMoveCameraToPreviousPoint())
  {
    m_mapView->moveCameraToPreviousTracePoint();
  }
}

bool MapFrame::canMoveCameraToPreviousPoint() const
{
  return m_mapView->canMoveCameraToPreviousTracePoint();
}

void MapFrame::reset2dCameras()
{
  if (auto* mapView2d = dynamic_cast<ui::MapView2D*>(currentMapViewBase()))
  {
    m_mapView->reset2dCameras(mapView2d->camera(), true);
  }
}

void MapFrame::focusCameraOnSelection()
{
  if (canFocusCamera())
  {
    m_mapView->focusCameraOnSelection(true);
  }
}

bool MapFrame::canFocusCamera() const
{
  return m_document->hasSelectedNodes();
}

void MapFrame::moveCameraToPosition()
{
  auto ok = false;
  const auto str = QInputDialog::getText(
    this,
    "Move Camera",
    "Enter a position (x y z) for the camera.",
    QLineEdit::Normal,
    "0.0 0.0 0.0",
    &ok);
  if (ok)
  {
    if (const auto position = vm::parse<float, 3>(str.toStdString()))
    {
      m_mapView->moveCameraToPosition(*position, true);
    }
  }
}

void MapFrame::isolateSelection()
{
  if (canIsolateSelection())
  {
    m_document->isolate();
  }
}

bool MapFrame::canIsolateSelection() const
{
  return m_document->hasSelectedNodes();
}

void MapFrame::hideSelection()
{
  if (canHideSelection())
  {
    m_document->hideSelection();
  }
}

bool MapFrame::canHideSelection() const
{
  return m_document->hasSelectedNodes();
}

void MapFrame::showAll()
{
  m_document->showAll();
}

void MapFrame::switchToInspectorPage(const InspectorPage page)
{
  m_inspector->show();
  m_inspector->switchToPage(page);
}

void MapFrame::toggleToolbar()
{
  m_toolBar->setVisible(!m_toolBar->isVisible());
}

bool MapFrame::toolbarVisible() const
{
  return m_toolBar->isVisible();
}

void MapFrame::toggleInfoPanel()
{
  m_infoPanel->setHidden(!m_infoPanel->isHidden());
}

bool MapFrame::infoPanelVisible() const
{
  return m_infoPanel->isVisible();
}

void MapFrame::toggleInspector()
{
  m_inspector->setHidden(!m_inspector->isHidden());
}

bool MapFrame::inspectorVisible() const
{
  return m_inspector->isVisible();
}

void MapFrame::toggleMaximizeCurrentView()
{
  m_mapView->toggleMaximizeCurrentView();
}

bool MapFrame::currentViewMaximized() const
{
  return m_mapView->currentViewMaximized();
}

void MapFrame::showCompileDialog()
{
  if (!m_compilationDialog)
  {
    m_compilationDialog = new CompilationDialog{this};
  }
  showModelessDialog(m_compilationDialog);
}

bool MapFrame::closeCompileDialog()
{
  if (!m_compilationDialog)
  {
    return true;
  }

  if (m_compilationDialog->close())
  {
    m_compilationDialog = nullptr;
    return true;
  }

  return false;
}

void MapFrame::showLaunchEngineDialog()
{
  auto dialog = LaunchGameEngineDialog{m_document, this};
  dialog.exec();
}

namespace
{

const mdl::Material* materialToReveal(std::shared_ptr<MapDocument> document)
{
  const auto* firstMaterial = document->allSelectedBrushFaces().front().face().material();
  const auto allFacesHaveIdenticalMaterial = kdl::all_of(
    document->allSelectedBrushFaces(),
    [&](const auto& face) { return face.face().material() == firstMaterial; });

  return allFacesHaveIdenticalMaterial ? firstMaterial : nullptr;
}

} // namespace

bool MapFrame::canRevealMaterial() const
{
  return materialToReveal(m_document) != nullptr;
}

void MapFrame::revealMaterial()
{
  if (const auto material = materialToReveal(m_document))
  {
    revealMaterial(material);
  }
}

void MapFrame::revealMaterial(const mdl::Material* material)
{
  m_inspector->switchToPage(InspectorPage::Face);
  m_inspector->faceInspector()->revealMaterial(material);
}

void MapFrame::debugPrintVertices()
{
  m_document->printVertices();
}

void MapFrame::debugCreateBrush()
{
  auto ok = false;
  const auto str = QInputDialog::getText(
    this,
    "Create Brush",
    "Enter a list of at least 4 points (x y z) (x y z) ...",
    QLineEdit::Normal,
    "",
    &ok);
  if (ok)
  {
    auto positions = std::vector<vm::vec3d>{};
    vm::parse_all<double, 3>(str.toStdString(), std::back_inserter(positions));
    m_document->createBrush(positions);
  }
}

void MapFrame::debugCreateCube()
{
  auto ok = false;
  const auto str = QInputDialog::getText(
    this, "Create Cube", "Enter bounding box size", QLineEdit::Normal, "", &ok);
  if (ok)
  {
    const auto size = str.toDouble();
    const auto bounds = vm::bbox3d{size / 2.0};
    const auto positions = bounds.vertices() | kdl::to_vector;
    m_document->createBrush(positions);
  }
}

void MapFrame::debugClipBrush()
{
  auto ok = false;
  const auto str = QInputDialog::getText(
    this,
    "Clip Brush",
    "Enter face points ( x y z ) ( x y z ) ( x y z )",
    QLineEdit::Normal,
    "",
    &ok);
  if (ok)
  {
    auto points = std::vector<vm::vec3d>{};
    vm::parse_all<double, 3>(str.toStdString(), std::back_inserter(points));
    if (points.size() == 3)
    {
      m_document->clipBrushes(points[0], points[1], points[2]);
    }
  }
}

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wcast-qual"
#endif
static void debugSegfault()
{
  volatile void* test = nullptr;
  printf("%p\n", *((void**)test));
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

[[noreturn]] static void debugException()
{
  const auto e = Exception{};
  throw e;
}

void MapFrame::debugCrash()
{
  auto items = QStringList{};
  items << "Null pointer dereference"
        << "Unhandled exception";

  bool ok;
  const auto item =
    QInputDialog::getItem(this, "Crash", "Choose a crash type", items, 0, false, &ok);
  if (ok)
  {
    const auto idx = items.indexOf(item);
    if (idx == 0)
    {
      debugSegfault();
    }
    else if (idx == 1)
    {
      debugException();
    }
  }
}

void MapFrame::debugThrowExceptionDuringCommand()
{
  m_document->throwExceptionDuringCommand();
}

void MapFrame::debugSetWindowSize()
{
  auto ok = false;
  const auto str = QInputDialog::getText(
    this, "Window Size", "Enter Size (W H)", QLineEdit::Normal, "1920 1080", &ok);
  if (ok)
  {
    if (const auto size = vm::parse<int, 2>(str.toStdString()))
    {
      resize(size->x(), size->y());
    }
  }
}

void MapFrame::debugShowPalette()
{
  auto* window = new DebugPaletteWindow{this};
  showModelessDialog(window);
}

void MapFrame::focusChange(QWidget* /* oldFocus */, QWidget* newFocus)
{
  if (auto* newMapView = dynamic_cast<MapViewBase*>(newFocus))
  {
    m_currentMapView = newMapView;
  }

  updateActionState();
  updateUndoRedoActions();
}

MapViewBase* MapFrame::currentMapViewBase()
{
  if (!m_currentMapView)
  {
    // This happens when the current map view is deleted (e.g. 4-pane to 1-pane layout)
    m_currentMapView = m_mapView->firstMapViewBase();
    ensure(
      m_currentMapView != nullptr,
      "SwitchableMapViewContainer should have constructed a MapViewBase");
  }
  return m_currentMapView;
}

bool MapFrame::canCompile() const
{
  return m_document->persistent();
}

bool MapFrame::canLaunch() const
{
  return m_document->persistent();
}

void MapFrame::dragEnterEvent(QDragEnterEvent* event)
{
  if (
    m_document->game()->config().materialConfig.property && event->mimeData()->hasUrls()
    && kdl::all_of(event->mimeData()->urls(), [](const auto& url) {
         if (!url.isLocalFile())
         {
           return false;
         }

         const auto fileInfo = QFileInfo{url.toLocalFile()};
         return fileInfo.isFile() && fileInfo.fileName().toLower().endsWith(".wad");
       }))
  {
    event->accept();
  }
}

void MapFrame::dropEvent(QDropEvent* event)
{
  const auto urls = event->mimeData()->urls();
  if (urls.empty())
  {
    return;
  }

  const auto& wadPropertyKey = m_document->game()->config().materialConfig.property;
  if (!wadPropertyKey)
  {
    return;
  }

  const auto* wadPathsStr = m_document->world()->entity().property(*wadPropertyKey);
  auto wadPaths = wadPathsStr ? kdl::vec_transform(
                                  kdl::str_split(*wadPathsStr, ";"),
                                  [](const auto& s) { return std::filesystem::path{s}; })
                              : std::vector<std::filesystem::path>{};

  auto pathDialog = ChoosePathTypeDialog{
    window(),
    io::pathFromQString(urls.front().toLocalFile()),
    document()->path(),
    document()->game()->gamePath()};

  const auto result = pathDialog.exec();
  if (result != QDialog::Accepted)
  {
    return;
  }

  auto wadPathsToAdd = kdl::vec_transform(urls, [&](const auto& url) {
    return convertToPathType(
      pathDialog.pathType(),
      io::pathFromQString(url.toLocalFile()),
      document()->path(),
      document()->game()->gamePath());
  });

  const auto newWadPathsStr = kdl::str_join(
    kdl::vec_transform(
      kdl::vec_concat(std::move(wadPaths), std::move(wadPathsToAdd)),
      [](const auto& path) { return path.string(); }),
    ";");
  document()->setProperty(*wadPropertyKey, newWadPathsStr);

  event->acceptProposedAction();
}

void MapFrame::changeEvent(QEvent*)
{
  if (m_mapView)
  {
    m_mapView->windowActivationStateChanged(isActiveWindow());
  }
}

void MapFrame::closeEvent(QCloseEvent* event)
{
  if (!closeCompileDialog())
  {
    event->ignore();
  }
  else
  {
    if (!confirmOrDiscardChanges())
    {
      event->ignore();
    }
    else
    {
      saveWindowGeometry(this);
      saveWindowState(this);
      saveWindowState(m_hSplitter);
      saveWindowState(m_vSplitter);

      m_frameManager.removeFrame(this);
      event->accept();
    }
  }
  // Don't call superclass implementation
}

template <typename F>
static void applyRecursively(QObject* object, const F& f)
{
  f(object);
  for (auto* child : object->children())
  {
    applyRecursively(child, f);
  }
}

bool MapFrame::eventFilter(QObject* target, QEvent* event)
{
  if (
    event->type() == QEvent::MouseButtonPress
    || event->type() == QEvent::MouseButtonRelease
    || event->type() == QEvent::MouseButtonDblClick || event->type() == QEvent::MouseMove
    || event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
  {
    m_lastInputTime = std::chrono::system_clock::now();
  }
  else if (event->type() == QEvent::ChildAdded)
  {
    auto* childEvent = static_cast<QChildEvent*>(event);
    applyRecursively(
      childEvent->child(), [&](auto* object) { object->installEventFilter(this); });
  }
  else if (event->type() == QEvent::ChildRemoved)
  {
    auto* childEvent = static_cast<QChildEvent*>(event);
    applyRecursively(
      childEvent->child(), [&](auto* object) { object->removeEventFilter(this); });
  }
  return QMainWindow::eventFilter(target, event);
}

void MapFrame::triggerAutosave()
{
  using namespace std::chrono_literals;
  if (
    QGuiApplication::mouseButtons() == Qt::NoButton
    && std::chrono::system_clock::now() - m_lastInputTime > 2s)
  {
    m_autosaver->triggerAutosave(logger());
  }
}

void MapFrame::triggerProcessResources()
{
  auto document = kdl::mem_lock(m_document);
  document->processResourcesAsync(mdl::ProcessContext{
    true, [&](const auto&, const auto& error) { logger().error() << error; }});
}

// DebugPaletteWindow

DebugPaletteWindow::DebugPaletteWindow(QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle(tr("Palette"));

  const auto roles = std::vector<std::pair<QPalette::ColorRole, QString>>{
    {QPalette::Window, "Window"},
    {QPalette::WindowText, "WindowText"},
    {QPalette::Base, "Base"},
    {QPalette::AlternateBase, "AlternateBase"},
    {QPalette::ToolTipBase, "ToolTipBase"},
    {QPalette::ToolTipText, "ToolTipText"},
    {QPalette::PlaceholderText, "PlaceholderText"},
    {QPalette::Text, "Text"},
    {QPalette::Button, "Button"},
    {QPalette::ButtonText, "ButtonText"},
    {QPalette::BrightText, "BrightText"},
    {QPalette::Light, "Light"},
    {QPalette::Midlight, "Midlight"},
    {QPalette::Dark, "Dark"},
    {QPalette::Mid, "Mid"},
    {QPalette::Shadow, "Shadow"},
    {QPalette::Highlight, "Highlight"},
    {QPalette::HighlightedText, "HighlightedText"}};

  const auto groups = std::vector<std::pair<QPalette::ColorGroup, QString>>{
    {QPalette::Disabled, "Disabled"},
    {QPalette::Active, "Active"},
    {QPalette::Inactive, "Inactive"}};

  auto verticalHeaderLabels = QStringList{};
  for (const auto& [role, roleLabel] : roles)
  {
    verticalHeaderLabels.append(roleLabel);
  }

  auto horizontalHeaderLabels = QStringList{};
  for (const auto& [group, groupLabel] : groups)
  {
    horizontalHeaderLabels.append(groupLabel);
  }

  auto* table = new QTableWidget{int(roles.size()), int(groups.size())};
  table->setHorizontalHeaderLabels(horizontalHeaderLabels);
  table->setVerticalHeaderLabels(verticalHeaderLabels);

  for (int x = 0; x < table->columnCount(); ++x)
  {
    for (int y = 0; y < table->rowCount(); ++y)
    {
      const auto role = roles.at(size_t(y)).first;
      const auto group = groups.at(size_t(x)).first;

      auto* button = new ColorButton{};
      table->setCellWidget(y, x, button);

      button->setColor(qApp->palette().color(group, role));

      connect(button, &ColorButton::colorChangedByUser, this, [=](const QColor& color) {
        auto palette = qApp->palette();
        palette.setColor(group, role, color);
        qApp->setPalette(palette);
      });
    }
  }

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(table);
  setLayout(layout);
}

DebugPaletteWindow::~DebugPaletteWindow() = default;

} // namespace tb::ui
