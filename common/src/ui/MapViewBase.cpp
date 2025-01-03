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

#include "MapViewBase.h"

#include <QDebug>
#include <QMenu>
#include <QMimeData>
#include <QShortcut>
#include <QString>
#include <QtGlobal>

#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/ChangeBrushFaceAttributesRequest.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionGroup.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/GroupNode.h"
#include "mdl/HitAdapter.h"
#include "mdl/HitFilter.h"
#include "mdl/LayerNode.h"
#include "mdl/ModelUtils.h"
#include "mdl/PatchNode.h"
#include "mdl/PointTrace.h"
#include "mdl/PortalFile.h"
#include "mdl/WorldNode.h"
#include "render/Camera.h"
#include "render/Compass.h"
#include "render/FontDescriptor.h"
#include "render/FontManager.h"
#include "render/MapRenderer.h"
#include "render/PrimitiveRenderer.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/RenderService.h"
#include "ui/Actions.h"
#include "ui/Animation.h"
#include "ui/EnableDisableTagCallback.h"
#include "ui/FlashSelectionAnimation.h"
#include "ui/GLContextManager.h"
#include "ui/Grid.h"
#include "ui/MapDocument.h"
#include "ui/MapFrame.h"
#include "ui/MapViewActivationTracker.h"
#include "ui/MapViewToolBox.h"
#include "ui/QtUtils.h"
#include "ui/SelectionTool.h"
#include "ui/SignalDelayer.h"
#include "ui/Transaction.h"

#include "kdl/memory_utils.h"
#include "kdl/string_compare.h"
#include "kdl/string_format.h"
#include "kdl/vector_utils.h"

#include "vm/polygon.h"
#include "vm/util.h"

#include <vector>

namespace tb::ui
{
const int MapViewBase::DefaultCameraAnimationDuration = 250;

MapViewBase::MapViewBase(
  std::weak_ptr<MapDocument> document,
  MapViewToolBox& toolBox,
  render::MapRenderer& renderer,
  GLContextManager& contextManager)
  : RenderView{contextManager}
  , m_document{std::move(document)}
  , m_toolBox{toolBox}
  , m_renderer{renderer}
  , m_animationManager{std::make_unique<AnimationManager>(this)}
  , m_updateActionStatesSignalDelayer{new SignalDelayer{this}}
{
  setToolBox(toolBox);
  bindEvents();
  connectObservers();

  setAcceptDrops(true);
}

void MapViewBase::setCompass(std::unique_ptr<render::Compass> compass)
{
  m_compass = std::move(compass);
}

void MapViewBase::mapViewBaseVirtualInit()
{
  createActionsAndUpdatePicking();
}

MapViewBase::~MapViewBase()
{
  // Deleting m_compass will access the VBO so we need to be current
  // see: http://doc.qt.io/qt-5/qopenglwidget.html#resource-initialization-and-cleanup
  makeCurrent();
}

void MapViewBase::setIsCurrent(const bool isCurrent)
{
  m_isCurrent = isCurrent;
}

void MapViewBase::bindEvents()
{
  connect(
    m_updateActionStatesSignalDelayer,
    &SignalDelayer::processSignal,
    this,
    &MapViewBase::updateActionStates);
}

void MapViewBase::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection +=
    document->nodesWereAddedNotifier.connect(this, &MapViewBase::nodesDidChange);
  m_notifierConnection +=
    document->nodesWereRemovedNotifier.connect(this, &MapViewBase::nodesDidChange);
  m_notifierConnection +=
    document->nodesDidChangeNotifier.connect(this, &MapViewBase::nodesDidChange);
  m_notifierConnection +=
    document->nodeVisibilityDidChangeNotifier.connect(this, &MapViewBase::nodesDidChange);
  m_notifierConnection +=
    document->nodeLockingDidChangeNotifier.connect(this, &MapViewBase::nodesDidChange);
  m_notifierConnection +=
    document->commandDoneNotifier.connect(this, &MapViewBase::commandDone);
  m_notifierConnection +=
    document->commandUndoneNotifier.connect(this, &MapViewBase::commandUndone);
  m_notifierConnection +=
    document->selectionDidChangeNotifier.connect(this, &MapViewBase::selectionDidChange);
  m_notifierConnection += document->materialCollectionsDidChangeNotifier.connect(
    this, &MapViewBase::materialCollectionsDidChange);
  m_notifierConnection += document->entityDefinitionsDidChangeNotifier.connect(
    this, &MapViewBase::entityDefinitionsDidChange);
  m_notifierConnection +=
    document->modsDidChangeNotifier.connect(this, &MapViewBase::modsDidChange);
  m_notifierConnection += document->editorContextDidChangeNotifier.connect(
    this, &MapViewBase::editorContextDidChange);
  m_notifierConnection +=
    document->documentWasNewedNotifier.connect(this, &MapViewBase::documentDidChange);
  m_notifierConnection +=
    document->documentWasClearedNotifier.connect(this, &MapViewBase::documentDidChange);
  m_notifierConnection +=
    document->documentWasLoadedNotifier.connect(this, &MapViewBase::documentDidChange);
  m_notifierConnection +=
    document->pointFileWasLoadedNotifier.connect(this, &MapViewBase::pointFileDidChange);
  m_notifierConnection += document->pointFileWasUnloadedNotifier.connect(
    this, &MapViewBase::pointFileDidChange);
  m_notifierConnection += document->portalFileWasLoadedNotifier.connect(
    this, &MapViewBase::portalFileDidChange);
  m_notifierConnection += document->portalFileWasUnloadedNotifier.connect(
    this, &MapViewBase::portalFileDidChange);

  auto& grid = document->grid();
  m_notifierConnection +=
    grid.gridDidChangeNotifier.connect(this, &MapViewBase::gridDidChange);

  m_notifierConnection +=
    m_toolBox.toolActivatedNotifier.connect(this, &MapViewBase::toolChanged);
  m_notifierConnection +=
    m_toolBox.toolDeactivatedNotifier.connect(this, &MapViewBase::toolChanged);

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &MapViewBase::preferenceDidChange);
}

/**
 * Full re-initialization of QActions and picking state.
 */
void MapViewBase::createActionsAndUpdatePicking()
{
  createActions();
  updateActionStates();
  updatePickResult();
}

void MapViewBase::nodesDidChange(const std::vector<mdl::Node*>&)
{
  updatePickResult();
  update();
}

void MapViewBase::toolChanged(Tool&)
{
  updatePickResult();
  updateActionStates();
  update();
}

void MapViewBase::commandDone(Command&)
{
  updateActionStatesDelayed();
  updatePickResult();
  update();
}

void MapViewBase::commandUndone(UndoableCommand&)
{
  updateActionStatesDelayed();
  updatePickResult();
  update();
}

void MapViewBase::selectionDidChange(const Selection&)
{
  updateActionStatesDelayed();
}

void MapViewBase::materialCollectionsDidChange()
{
  update();
}

void MapViewBase::entityDefinitionsDidChange()
{
  createActions();
  updateActionStates();
  update();
}

void MapViewBase::modsDidChange()
{
  update();
}

void MapViewBase::editorContextDidChange()
{
  update();
}

void MapViewBase::gridDidChange()
{
  update();
}

void MapViewBase::pointFileDidChange()
{
  update();
}

void MapViewBase::portalFileDidChange()
{
  invalidatePortalFileRenderer();
  update();
}

void MapViewBase::preferenceDidChange(const std::filesystem::path& path)
{
  if (path == Preferences::RendererFontSize.path())
  {
    fontManager().clearCache();
  }

  updateActionBindings();
  update();
}

void MapViewBase::documentDidChange(MapDocument*)
{
  createActionsAndUpdatePicking();
  update();
}

void MapViewBase::createActions()
{
  // Destroy existing QShortcuts via the weak references in m_shortcuts
  for (auto& [shortcut, action] : m_shortcuts)
  {
    unused(action);
    delete shortcut;
  }
  m_shortcuts.clear();

  auto visitor = [this](const Action& action) {
    const auto keySequence = action.keySequence();

    auto* shortcut = new QShortcut{this};
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    shortcut->setKey(keySequence);
    connect(
      shortcut, &QShortcut::activated, this, [this, &action] { triggerAction(action); });
    connect(shortcut, &QShortcut::activatedAmbiguously, this, [this, &action] {
      triggerAmbiguousAction(action.label());
    });
    m_shortcuts.emplace_back(shortcut, &action);
  };

  auto& actionManager = ActionManager::instance();
  // We don't create a QShortcut for actions whose key binding is handled
  // by the menu or toolbar since they would conflict.
  actionManager.visitMapViewActions(visitor);

  auto document = kdl::mem_lock(m_document);
  document->visitTagActions(visitor);
  document->visitEntityDefinitionActions(visitor);
}

void MapViewBase::updateActionBindings()
{
  for (auto& [shortcut, action] : m_shortcuts)
  {
    shortcut->setKey(action->keySequence());
  }
}

void MapViewBase::updateActionStates()
{
  auto context = ActionExecutionContext{findMapFrame(this), this};
  for (auto& [shortcut, action] : m_shortcuts)
  {
    shortcut->setEnabled(hasFocus() && action->enabled(context));
  }
}

void MapViewBase::updateActionStatesDelayed()
{
  m_updateActionStatesSignalDelayer->queueSignal();
}

void MapViewBase::triggerAction(const Action& action)
{
  auto context = ActionExecutionContext{findMapFrame(this), this};
  action.execute(context);
}

void MapViewBase::triggerAmbiguousAction(const QString& label)
{
  qDebug() << "Ambiguous action triggered: " << label;
}

void MapViewBase::move(const vm::direction direction)
{
  if ((actionContext() & ActionContext::RotateTool) != 0)
  {
    moveRotationCenter(direction);
  }
  else if ((actionContext() & ActionContext::AnyVertexTool) != 0)
  {
    moveVertices(direction);
  }
  else if ((actionContext() & ActionContext::NodeSelection) != 0)
  {
    moveObjects(direction);
  }
}

void MapViewBase::moveRotationCenter(const vm::direction direction)
{
  auto document = kdl::mem_lock(m_document);
  const auto& grid = document->grid();
  const auto delta = moveDirection(direction) * double(grid.actualSize());
  m_toolBox.moveRotationCenter(delta);
  update();
}

void MapViewBase::moveVertices(const vm::direction direction)
{
  auto document = kdl::mem_lock(m_document);
  const auto& grid = document->grid();
  const auto delta = moveDirection(direction) * double(grid.actualSize());
  m_toolBox.moveVertices(delta);
}

void MapViewBase::moveObjects(const vm::direction direction)
{
  auto document = kdl::mem_lock(m_document);
  const auto& grid = document->grid();
  const auto delta = moveDirection(direction) * double(grid.actualSize());
  document->translateObjects(delta);
}

void MapViewBase::duplicateObjects()
{
  auto document = kdl::mem_lock(m_document);
  if (document->hasSelectedNodes())
  {
    document->duplicateObjects();
  }
}

void MapViewBase::duplicateAndMoveObjects(const vm::direction direction)
{
  auto transaction = Transaction{m_document};
  duplicateObjects();
  moveObjects(direction);
  transaction.commit();
}

void MapViewBase::rotateObjects(const vm::rotation_axis axisSpec, const bool clockwise)
{
  auto document = kdl::mem_lock(m_document);
  if (document->hasSelectedNodes())
  {
    const auto axis = rotationAxis(axisSpec, clockwise);
    const auto angle = m_toolBox.rotateObjectsToolActive()
                         ? vm::abs(m_toolBox.rotateToolAngle())
                         : vm::Cd::half_pi();

    const auto& grid = document->grid();
    const auto center = m_toolBox.rotateObjectsToolActive()
                          ? m_toolBox.rotateToolCenter()
                          : grid.referencePoint(document->selectionBounds());

    document->rotateObjects(center, axis, angle);
  }
}

vm::vec3d MapViewBase::rotationAxis(
  const vm::rotation_axis axisSpec, const bool clockwise) const
{
  vm::vec3d axis;
  switch (axisSpec)
  {
  case vm::rotation_axis::roll:
    axis = -moveDirection(vm::direction::forward);
    break;
  case vm::rotation_axis::pitch:
    axis = moveDirection(vm::direction::right);
    break;
  case vm::rotation_axis::yaw:
    axis = moveDirection(vm::direction::up);
    break;
    switchDefault();
  }

  return clockwise ? -axis : axis;
}

void MapViewBase::flipObjects(const vm::direction direction)
{
  if (canFlipObjects())
  {
    auto document = kdl::mem_lock(m_document);

    // If we snap the selection bounds' center to the grid size, then
    // selections that are an odd number of grid units wide get translated.
    // Instead, snap to 1/2 the grid size.
    // (see: https://github.com/TrenchBroom/TrenchBroom/issues/1495 )
    auto halfGrid = Grid{document->grid().size()};
    halfGrid.decSize();

    const auto center = halfGrid.referencePoint(document->selectionBounds());
    const auto axis = flipAxis(direction);

    document->flipObjects(center, axis);
  }
}

bool MapViewBase::canFlipObjects() const
{
  auto document = kdl::mem_lock(m_document);
  return !m_toolBox.anyToolActive() && document->hasSelectedNodes();
}

void MapViewBase::moveUV(const vm::direction direction, const UVActionMode mode)
{
  auto document = kdl::mem_lock(m_document);
  if (document->hasSelectedBrushFaces())
  {
    const auto offset = moveUVOffset(direction, mode);
    document->translateUV(camera().up(), camera().right(), offset);
  }
}

vm::vec2f MapViewBase::moveUVOffset(
  const vm::direction direction, const UVActionMode mode) const
{
  switch (direction)
  {
  case vm::direction::up:
    return vm::vec2f{0.0f, moveUVDistance(mode)};
  case vm::direction::down:
    return vm::vec2f{0.0f, -moveUVDistance(mode)};
  case vm::direction::left:
    return vm::vec2f{-moveUVDistance(mode), 0.0f};
  case vm::direction::right:
    return vm::vec2f{moveUVDistance(mode), 0.0f};
  case vm::direction::forward:
  case vm::direction::backward:
    return vm::vec2f{};
    switchDefault();
  }
}

float MapViewBase::moveUVDistance(const UVActionMode mode) const
{
  const auto& grid = kdl::mem_lock(m_document)->grid();
  const auto gridSize = static_cast<float>(grid.actualSize());

  switch (mode)
  {
  case UVActionMode::Fine:
    return 1.0f;
  case UVActionMode::Coarse:
    return 2.0f * gridSize;
  case UVActionMode::Normal:
    return gridSize;
    switchDefault();
  }
}

void MapViewBase::rotateUV(const bool clockwise, const UVActionMode mode)
{
  auto document = kdl::mem_lock(m_document);
  if (document->hasSelectedBrushFaces())
  {
    const auto angle = rotateUVAngle(clockwise, mode);
    document->rotateUV(angle);
  }
}

float MapViewBase::rotateUVAngle(const bool clockwise, const UVActionMode mode) const
{
  const auto& grid = kdl::mem_lock(m_document)->grid();
  const auto gridAngle = static_cast<float>(vm::to_degrees(grid.angle()));
  auto angle = 0.0f;

  switch (mode)
  {
  case UVActionMode::Fine:
    angle = 1.0f;
    break;
  case UVActionMode::Coarse:
    angle = 90.0f;
    break;
  case UVActionMode::Normal:
    angle = gridAngle;
    break;
  }
  return clockwise ? angle : -angle;
}

void MapViewBase::flipUV(const vm::direction direction)
{
  auto document = kdl::mem_lock(m_document);
  if (document->hasSelectedBrushFaces())
  {
    document->flipUV(camera().up(), camera().right(), direction);
  }
}

void MapViewBase::resetUV()
{
  auto request = mdl::ChangeBrushFaceAttributesRequest{};

  auto document = kdl::mem_lock(m_document);
  request.resetAll(document->game()->config().faceAttribsConfig.defaults);
  document->setFaceAttributes(request);
}

void MapViewBase::resetUVToWorld()
{
  auto request = mdl::ChangeBrushFaceAttributesRequest{};

  auto document = kdl::mem_lock(m_document);
  request.resetAllToParaxial(document->game()->config().faceAttribsConfig.defaults);
  document->setFaceAttributes(request);
}

void MapViewBase::assembleBrush()
{
  if (m_toolBox.assembleBrushToolActive())
  {
    m_toolBox.performAssembleBrush();
  }
}

void MapViewBase::toggleClipSide()
{
  m_toolBox.toggleClipSide();
}

void MapViewBase::performClip()
{
  m_toolBox.performClip();
}

void MapViewBase::resetCameraZoom()
{
  camera().setZoom(1.0f);
}

void MapViewBase::cancel()
{
  if (!ToolBoxConnector::cancel())
  {
    auto document = kdl::mem_lock(m_document);
    if (document->hasSelection())
    {
      document->deselectAll();
    }
    else if (document->currentGroup())
    {
      document->closeGroup();
    }
  }
}

void MapViewBase::deactivateTool()
{
  m_toolBox.deactivateAllTools();
}

void MapViewBase::createPointEntity()
{
  auto* action = qobject_cast<const QAction*>(sender());
  auto document = kdl::mem_lock(m_document);
  const auto index = action->data().toUInt();
  const auto* definition =
    findEntityDefinition(mdl::EntityDefinitionType::PointEntity, index);
  ensure(definition != nullptr, "definition is null");
  assert(definition->type() == mdl::EntityDefinitionType::PointEntity);
  createPointEntity(static_cast<const mdl::PointEntityDefinition*>(definition));
}

void MapViewBase::createBrushEntity()
{
  auto* action = qobject_cast<const QAction*>(sender());
  auto document = kdl::mem_lock(m_document);
  const auto index = action->data().toUInt();
  const auto* definition =
    findEntityDefinition(mdl::EntityDefinitionType::BrushEntity, index);
  ensure(definition != nullptr, "definition is null");
  assert(definition->type() == mdl::EntityDefinitionType::BrushEntity);
  createBrushEntity(static_cast<const mdl::BrushEntityDefinition*>(definition));
}

mdl::EntityDefinition* MapViewBase::findEntityDefinition(
  const mdl::EntityDefinitionType type, const size_t index) const
{
  size_t count = 0;
  for (const auto& group : kdl::mem_lock(m_document)->entityDefinitionManager().groups())
  {
    const auto definitions =
      group.definitions(type, mdl::EntityDefinitionSortOrder::Name);
    if (index < count + definitions.size())
    {
      return definitions[index - count];
    }
    count += definitions.size();
  }
  return nullptr;
}

void MapViewBase::createPointEntity(const mdl::PointEntityDefinition* definition)
{
  ensure(definition != nullptr, "definition is null");

  auto document = kdl::mem_lock(m_document);
  const auto delta = computePointEntityPosition(definition->bounds());
  document->createPointEntity(definition, delta);
}

void MapViewBase::createBrushEntity(const mdl::BrushEntityDefinition* definition)
{
  ensure(definition != nullptr, "definition is null");

  auto document = kdl::mem_lock(m_document);
  document->createBrushEntity(definition);
}

bool MapViewBase::canCreateBrushEntity()
{
  auto document = kdl::mem_lock(m_document);
  return document->selectedNodes().hasOnlyBrushes();
}

void MapViewBase::toggleTagVisible(const mdl::SmartTag& tag)
{
  const auto tagIndex = tag.index();

  auto document = kdl::mem_lock(m_document);
  auto& editorContext = document->editorContext();
  auto hiddenTags = editorContext.hiddenTags();
  hiddenTags ^= mdl::TagType::Type{1} << tagIndex;
  editorContext.setHiddenTags(hiddenTags);
}

void MapViewBase::enableTag(const mdl::SmartTag& tag)
{
  assert(tag.canEnable());
  auto document = kdl::mem_lock(m_document);

  auto transaction = Transaction{document, "Turn Selection into " + tag.name()};
  auto callback = EnableDisableTagCallback{};
  tag.enable(callback, *document);
  transaction.commit();
}

void MapViewBase::disableTag(const mdl::SmartTag& tag)
{
  assert(tag.canDisable());
  auto document = kdl::mem_lock(m_document);
  auto transaction = Transaction{document, "Turn Selection into non-" + tag.name()};
  auto callback = EnableDisableTagCallback{};
  tag.disable(callback, *document);
  transaction.commit();
}

void MapViewBase::makeStructural()
{
  auto document = kdl::mem_lock(m_document);
  if (!document->selectedNodes().hasBrushes())
  {
    return;
  }

  auto toReparent = std::vector<mdl::Node*>{};
  const auto& selectedBrushes = document->selectedNodes().brushes();
  std::copy_if(
    selectedBrushes.begin(),
    selectedBrushes.end(),
    std::back_inserter(toReparent),
    [&](const auto* brushNode) { return brushNode->entity() != document->world(); });

  auto transaction = Transaction{document, "Make Structural"};

  if (!toReparent.empty())
  {
    reparentNodes(toReparent, document->parentForNodes(toReparent), false);
  }

  auto anyTagDisabled = false;
  auto callback = EnableDisableTagCallback{};
  for (auto* brush : document->selectedNodes().brushes())
  {
    for (const auto& tag : document->smartTags())
    {
      if (brush->hasTag(tag) || brush->anyFacesHaveAnyTagInMask(tag.type()))
      {
        anyTagDisabled = true;
        tag.disable(callback, *document);
      }
    }
  }

  if (!anyTagDisabled && toReparent.empty())
  {
    transaction.cancel();
  }
  else
  {
    transaction.commit();
  }
}

void MapViewBase::toggleEntityDefinitionVisible(const mdl::EntityDefinition* definition)
{
  auto document = kdl::mem_lock(m_document);

  auto& editorContext = document->editorContext();
  editorContext.setEntityDefinitionHidden(
    definition, !editorContext.entityDefinitionHidden(definition));
}

void MapViewBase::createEntity(const mdl::EntityDefinition* definition)
{
  auto document = kdl::mem_lock(m_document);
  if (definition->type() == mdl::EntityDefinitionType::PointEntity)
  {
    createPointEntity(static_cast<const mdl::PointEntityDefinition*>(definition));
  }
  else if (canCreateBrushEntity())
  {
    createBrushEntity(static_cast<const mdl::BrushEntityDefinition*>(definition));
  }
}

void MapViewBase::toggleShowEntityClassnames()
{
  togglePref(Preferences::ShowEntityClassnames);
}

void MapViewBase::toggleShowGroupBounds()
{
  togglePref(Preferences::ShowGroupBounds);
}

void MapViewBase::toggleShowBrushEntityBounds()
{
  togglePref(Preferences::ShowBrushEntityBounds);
}

void MapViewBase::toggleShowPointEntityBounds()
{
  togglePref(Preferences::ShowPointEntityBounds);
}

void MapViewBase::toggleShowPointEntities()
{
  togglePref(Preferences::ShowPointEntities);
}

void MapViewBase::toggleShowPointEntityModels()
{
  togglePref(Preferences::ShowPointEntityModels);
}

void MapViewBase::toggleShowBrushes()
{
  togglePref(Preferences::ShowBrushes);
}

void MapViewBase::showMaterials()
{
  setPref(Preferences::FaceRenderMode, Preferences::faceRenderModeTextured());
}

void MapViewBase::hideMaterials()
{
  setPref(Preferences::FaceRenderMode, Preferences::faceRenderModeFlat());
}

void MapViewBase::hideFaces()
{
  setPref(Preferences::FaceRenderMode, Preferences::faceRenderModeSkip());
}

void MapViewBase::toggleShadeFaces()
{
  togglePref(Preferences::ShadeFaces);
}

void MapViewBase::toggleShowFog()
{
  togglePref(Preferences::ShowFog);
}

void MapViewBase::toggleShowEdges()
{
  togglePref(Preferences::ShowEdges);
}

void MapViewBase::showAllEntityLinks()
{
  setPref(Preferences::FaceRenderMode, Preferences::entityLinkModeAll());
}

void MapViewBase::showTransitivelySelectedEntityLinks()
{
  setPref(Preferences::FaceRenderMode, Preferences::entityLinkModeTransitive());
}

void MapViewBase::showDirectlySelectedEntityLinks()
{
  setPref(Preferences::FaceRenderMode, Preferences::entityLinkModeDirect());
}

void MapViewBase::hideAllEntityLinks()
{
  setPref(Preferences::FaceRenderMode, Preferences::entityLinkModeNone());
}

bool MapViewBase::event(QEvent* event)
{
  if (event->type() == QEvent::WindowDeactivate)
  {
    cancelMouseDrag();
  }

  return RenderView::event(event);
}

void MapViewBase::focusInEvent(QFocusEvent* event)
{
  updateActionStates(); // enable/disable QShortcut's to reflect whether we have focus
                        // (needed because of QOpenGLWindow; see comment in
                        // createAndRegisterShortcut)
  updateModifierKeys();
  update();
  RenderView::focusInEvent(event);
}

void MapViewBase::focusOutEvent(QFocusEvent* event)
{
  clearModifierKeys();
  update();
  RenderView::focusOutEvent(event);
}

ActionContext::Type MapViewBase::actionContext() const
{
  auto document = kdl::mem_lock(m_document);

  const auto viewContext = viewActionContext();
  const auto toolContext =
    m_toolBox.assembleBrushToolActive()   ? ActionContext::AssembleBrushTool
    : m_toolBox.clipToolActive()          ? ActionContext::ClipTool
    : m_toolBox.anyVertexToolActive()     ? ActionContext::AnyVertexTool
    : m_toolBox.rotateObjectsToolActive() ? ActionContext::RotateTool
    : m_toolBox.scaleObjectsToolActive()  ? ActionContext::ScaleTool
    : m_toolBox.shearObjectsToolActive()  ? ActionContext::ShearTool
                                          : ActionContext::NoTool;
  const auto selectionContext =
    document->hasSelectedNodes()        ? ActionContext::NodeSelection
    : document->hasSelectedBrushFaces() ? ActionContext::FaceSelection
                                        : ActionContext::NoSelection;
  return viewContext | toolContext | selectionContext;
}

void MapViewBase::flashSelection()
{
  auto animation = std::make_unique<FlashSelectionAnimation>(m_renderer, this, 180);
  m_animationManager->runAnimation(std::move(animation), true);
}

void MapViewBase::installActivationTracker(MapViewActivationTracker& activationTracker)
{
  activationTracker.addWindow(this);
}

bool MapViewBase::isCurrent() const
{
  return m_isCurrent;
}

MapViewBase* MapViewBase::firstMapViewBase()
{
  return this;
}

bool MapViewBase::cancelMouseDrag()
{
  return ToolBoxConnector::cancelDrag();
}

void MapViewBase::refreshViews()
{
  update();
}

void MapViewBase::initializeGL()
{
  if (doInitializeGL())
  {
    auto& logger = kdl::mem_lock(m_document)->logger();
    logger.info() << "Renderer info: " << GLContextManager::GLRenderer << " version "
                  << GLContextManager::GLVersion << " from "
                  << GLContextManager::GLVendor;
    logger.info() << "Depth buffer bits: " << depthBits();
    logger.info() << "Multisampling "
                  << kdl::str_select(multisample(), "enabled", "disabled");
  }
}

bool MapViewBase::shouldRenderFocusIndicator() const
{
  return true;
}

void MapViewBase::renderContents()
{
  preRender();

  const auto& fontPath = pref(Preferences::RendererFontPath());
  const auto fontSize = static_cast<size_t>(pref(Preferences::RendererFontSize));
  const auto fontDescriptor = render::FontDescriptor{fontPath, fontSize};

  auto document = kdl::mem_lock(m_document);
  const auto& grid = document->grid();

  auto renderContext =
    render::RenderContext{renderMode(), camera(), fontManager(), shaderManager()};
  renderContext.setFilterMode(
    pref(Preferences::TextureMinFilter), pref(Preferences::TextureMagFilter));
  renderContext.setShowMaterials(
    pref(Preferences::FaceRenderMode) == Preferences::faceRenderModeTextured());
  renderContext.setShowFaces(
    pref(Preferences::FaceRenderMode) != Preferences::faceRenderModeSkip());
  renderContext.setShowEdges(pref(Preferences::ShowEdges));
  renderContext.setShadeFaces(pref(Preferences::ShadeFaces));
  renderContext.setShowPointEntities(pref(Preferences::ShowPointEntities));
  renderContext.setShowPointEntityModels(pref(Preferences::ShowPointEntityModels));
  renderContext.setShowEntityClassnames(pref(Preferences::ShowEntityClassnames));
  renderContext.setShowGroupBounds(pref(Preferences::ShowGroupBounds));
  renderContext.setShowBrushEntityBounds(pref(Preferences::ShowBrushEntityBounds));
  renderContext.setShowPointEntityBounds(pref(Preferences::ShowPointEntityBounds));
  renderContext.setShowFog(pref(Preferences::ShowFog));
  renderContext.setShowGrid(grid.visible());
  renderContext.setGridSize(grid.actualSize());
  renderContext.setDpiScale(static_cast<float>(window()->devicePixelRatioF()));
  renderContext.setSoftMapBounds(
    pref(Preferences::ShowSoftMapBounds)
      ? vm::bbox3f{document->softMapBounds().bounds.value_or(vm::bbox3d{})}
      : vm::bbox3f{});

  setupGL(renderContext);
  setRenderOptions(renderContext);

  auto renderBatch = render::RenderBatch{vboManager()};

  renderGrid(renderContext, renderBatch);
  renderMap(m_renderer, renderContext, renderBatch);
  renderTools(m_toolBox, renderContext, renderBatch);

  renderCoordinateSystem(renderContext, renderBatch);
  renderSoftWorldBounds(renderContext, renderBatch);
  renderPointFile(renderContext, renderBatch);
  renderPortalFile(renderContext, renderBatch);
  renderCompass(renderBatch);
  renderFPS(renderContext, renderBatch);

  renderBatch.render(renderContext);

  if (document->needsResourceProcessing())
  {
    update();
  }
}

void MapViewBase::preRender() {}

void MapViewBase::renderGrid(render::RenderContext&, render::RenderBatch&) {}

void MapViewBase::setupGL(render::RenderContext& context)
{
  const auto& viewport = context.camera().viewport();
  const auto r = devicePixelRatioF();
  const auto x = static_cast<int>(viewport.x * r);
  const auto y = static_cast<int>(viewport.y * r);
  const auto width = static_cast<int>(viewport.width * r);
  const auto height = static_cast<int>(viewport.height * r);
  glAssert(glViewport(x, y, width, height));

  if (pref(Preferences::EnableMSAA))
  {
    glAssert(glEnable(GL_MULTISAMPLE));
  }
  else
  {
    glAssert(glDisable(GL_MULTISAMPLE));
  }
  glAssert(glEnable(GL_BLEND));
  glAssert(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  glAssert(glShadeModel(GL_SMOOTH));
}

void MapViewBase::renderCoordinateSystem(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  if (pref(Preferences::ShowAxes))
  {
    auto document = kdl::mem_lock(m_document);
    const auto& worldBounds = document->worldBounds();

    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.renderCoordinateSystem(vm::bbox3f{worldBounds});
  }
}

void MapViewBase::renderSoftWorldBounds(render::RenderContext&, render::RenderBatch&) {}

void MapViewBase::renderPointFile(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  auto document = kdl::mem_lock(m_document);
  if (const auto* pointFile = document->pointFile())
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(pref(Preferences::PointFileColor));
    renderService.renderLineStrip(pointFile->points());
  }
}

void MapViewBase::renderPortalFile(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  if (!m_portalFileRenderer)
  {
    validatePortalFileRenderer(renderContext);
    assert(m_portalFileRenderer != nullptr);
  }
  renderBatch.add(m_portalFileRenderer.get());
}

void MapViewBase::invalidatePortalFileRenderer()
{
  m_portalFileRenderer = nullptr;
}

void MapViewBase::validatePortalFileRenderer(render::RenderContext&)
{
  assert(m_portalFileRenderer == nullptr);
  m_portalFileRenderer = std::make_unique<render::PrimitiveRenderer>();

  auto document = kdl::mem_lock(m_document);
  auto* portalFile = document->portalFile();
  if (portalFile)
  {
    for (const auto& poly : portalFile->portals())
    {
      m_portalFileRenderer->renderFilledPolygon(
        pref(Preferences::PortalFileFillColor),
        render::PrimitiveRendererOcclusionPolicy::Hide,
        render::PrimitiveRendererCullingPolicy::ShowBackfaces,
        poly.vertices());

      const auto lineWidth = 4.0f;
      m_portalFileRenderer->renderPolygon(
        pref(Preferences::PortalFileBorderColor),
        lineWidth,
        render::PrimitiveRendererOcclusionPolicy::Hide,
        poly.vertices());
    }
  }
}

void MapViewBase::renderCompass(render::RenderBatch& renderBatch)
{
  if (m_compass != nullptr)
  {
    m_compass->render(renderBatch);
  }
}

void MapViewBase::renderFPS(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  if (pref(Preferences::ShowFPS))
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.renderHeadsUp(m_currentFPS);
  }
}

void MapViewBase::processEvent(const KeyEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void MapViewBase::processEvent(const MouseEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void MapViewBase::processEvent(const ScrollEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void MapViewBase::processEvent(const GestureEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void MapViewBase::processEvent(const CancelEvent& event)
{
  ToolBoxConnector::processEvent(event);
}

void MapViewBase::doShowPopupMenu()
{
  // We process input events during paint event processing, but we cannot show a popup
  // menu during paint processing, so we enqueue an event for later.
  QMetaObject::invokeMethod(this, "showPopupMenuLater", Qt::QueuedConnection);
}

void MapViewBase::showPopupMenuLater()
{
  beforePopupMenu();

  auto document = kdl::mem_lock(m_document);
  const auto& nodes = document->selectedNodes().nodes();
  auto* newBrushParent = findNewParentEntityForBrushes(nodes);
  auto* currentGroup = document->editorContext().currentGroup();
  auto* newGroup = findNewGroupForObjects(nodes);
  auto* mergeGroup = findGroupToMergeGroupsInto(document->selectedNodes());

  auto* mapFrame = findMapFrame(this);

  auto menu = QMenu{};
  const auto addMainMenuAction = [&](const auto& path) -> QAction* {
    auto* groupAction = mapFrame->findAction(path);
    assert(groupAction != nullptr);
    menu.addAction(groupAction);
    return groupAction;
  };

  addMainMenuAction("Menu/Edit/Group");
  addMainMenuAction("Menu/Edit/Ungroup");

  auto* mergeGroupAction = menu.addAction(
    mergeGroup
      ? tr("Merge Groups into %1").arg(QString::fromStdString(mergeGroup->name()))
      : tr("Merge Groups"),
    this,
    &MapViewBase::mergeSelectedGroups);
  mergeGroupAction->setEnabled(canMergeGroups());

  auto* renameAction =
    menu.addAction(tr("Rename Groups"), mapFrame, &MapFrame::renameSelectedGroups);
  renameAction->setEnabled(mapFrame->canRenameSelectedGroups());

  if (newGroup && canReparentNodes(nodes, newGroup))
  {
    menu.addAction(
      tr("Add Objects to Group %1").arg(QString::fromStdString(newGroup->name())),
      this,
      &MapViewBase::addSelectedObjectsToGroup);
  }
  if (currentGroup && !document->selectedNodes().empty())
  {
    menu.addAction(
      tr("Remove Objects from Group %1")
        .arg(QString::fromStdString(currentGroup->name())),
      this,
      &MapViewBase::removeSelectedObjectsFromGroup);
  }
  menu.addSeparator();

  // Linked group operations

  addMainMenuAction("Menu/Edit/Create Linked Duplicate");
  addMainMenuAction("Menu/Edit/Select Linked Groups");
  addMainMenuAction("Menu/Edit/Separate Linked Groups");
  menu.addSeparator();

  // Layer operations

  const auto selectedObjectLayers = mdl::collectContainingLayersUserSorted(nodes);

  auto* moveSelectionTo = menu.addMenu(tr("Move to Layer"));
  for (auto* layerNode : document->world()->allLayersUserSorted())
  {
    auto* action =
      moveSelectionTo->addAction(QString::fromStdString(layerNode->name()), this, [=] {
        document->moveSelectionToLayer(layerNode);
      });
    action->setEnabled(document->canMoveSelectionToLayer(layerNode));
  }

  const auto moveSelectionToItems = moveSelectionTo->actions();
  moveSelectionTo->setEnabled(std::any_of(
    std::begin(moveSelectionToItems),
    std::end(moveSelectionToItems),
    [](QAction* action) { return action->isEnabled(); }));

  if (selectedObjectLayers.size() == 1u)
  {
    auto* layerNode = selectedObjectLayers[0];
    auto* action = menu.addAction(
      tr("Make Layer %1 Active").arg(QString::fromStdString(layerNode->name())),
      this,
      [=]() { document->setCurrentLayer(layerNode); });
    action->setEnabled(document->canSetCurrentLayer(layerNode));
  }
  else
  {
    auto* makeLayerActive = menu.addMenu(tr("Make Layer Active"));
    for (auto* layerNode : selectedObjectLayers)
    {
      auto* action = makeLayerActive->addAction(
        QString::fromStdString(layerNode->name()), this, [=]() {
          document->setCurrentLayer(layerNode);
        });
      action->setEnabled(document->canSetCurrentLayer(layerNode));
    }
    if (makeLayerActive->isEmpty())
    {
      makeLayerActive->setDisabled(true);
    }
  }

  auto* hideLayersAction = menu.addAction(
    tr("Hide Layers"), this, [=]() { document->hideLayers(selectedObjectLayers); });
  hideLayersAction->setEnabled(document->canHideLayers(selectedObjectLayers));
  auto* isolateLayersAction = menu.addAction(
    tr("Isolate Layers"), this, [=]() { document->isolateLayers(selectedObjectLayers); });
  isolateLayersAction->setEnabled(document->canIsolateLayers(selectedObjectLayers));
  auto* selectAllInLayersAction = menu.addAction(tr("Select All in Layers"), this, [=]() {
    document->selectAllInLayers(selectedObjectLayers);
  });
  selectAllInLayersAction->setEnabled(
    document->canSelectAllInLayers(selectedObjectLayers));

  menu.addSeparator();

  if (document->selectedNodes().hasOnlyBrushes())
  {
    auto* moveToWorldAction =
      menu.addAction(tr("Make Structural"), this, &MapViewBase::makeStructural);
    moveToWorldAction->setEnabled(canMakeStructural());

    const auto isEntity = newBrushParent->accept(kdl::overload(
      [](const mdl::WorldNode*) { return false; },
      [](const mdl::LayerNode*) { return false; },
      [](const mdl::GroupNode*) { return false; },
      [](const mdl::EntityNode*) { return true; },
      [](const mdl::BrushNode*) { return false; },
      [](const mdl::PatchNode*) { return false; }));

    if (isEntity)
    {
      menu.addAction(
        tr("Move Brushes to Entity %1")
          .arg(QString::fromStdString(newBrushParent->name())),
        this,
        &MapViewBase::moveSelectedBrushesToEntity);
    }
  }

  menu.addSeparator();

  using namespace mdl::HitFilters;
  const auto& hit = pickResult().first(type(mdl::BrushNode::BrushHitType));
  const auto faceHandle = mdl::hitToFaceHandle(hit);
  if (faceHandle)
  {
    const auto* material = faceHandle->face().material();
    menu.addAction(
      tr("Reveal %1 in Material Browser")
        .arg(QString::fromStdString(faceHandle->face().attributes().materialName())),
      mapFrame,
      [=] { mapFrame->revealMaterial(material); });

    menu.addSeparator();
  }

  menu.addMenu(makeEntityGroupsMenu(mdl::EntityDefinitionType::PointEntity));
  menu.addMenu(makeEntityGroupsMenu(mdl::EntityDefinitionType::BrushEntity));

  menu.exec(QCursor::pos());

  // Generate a synthetic mouse move event to update the mouse position after the popup
  // menu closes.
  const auto screenPos = QCursor::pos();
  const auto windowPos = window()->mapFromGlobal(screenPos);
  const auto localPos = mapFromGlobal(screenPos);
  auto mouseEvent = QMouseEvent(
    QEvent::MouseMove,
    localPos,
    windowPos,
    screenPos,
    Qt::NoButton,
    Qt::NoButton,
    Qt::NoModifier,
    Qt::MouseEventSynthesizedByApplication);
  mouseMoveEvent(&mouseEvent);
}

void MapViewBase::beforePopupMenu() {}

/**
 * Forward drag and drop events from QWidget to ToolBoxConnector
 */
void MapViewBase::dragEnterEvent(QDragEnterEvent* dragEnterEvent)
{
  if (dragEnter(
        static_cast<float>(dragEnterEvent->position().x()),
        static_cast<float>(dragEnterEvent->position().y()),
        dragEnterEvent->mimeData()->text().toStdString()))
  {
    dragEnterEvent->acceptProposedAction();
  }
}

void MapViewBase::dragLeaveEvent(QDragLeaveEvent*)
{
  dragLeave();
}

void MapViewBase::dragMoveEvent(QDragMoveEvent* dragMoveEvent)
{
  dragMove(
    static_cast<float>(dragMoveEvent->position().x()),
    static_cast<float>(dragMoveEvent->position().y()),
    dragMoveEvent->mimeData()->text().toStdString());
  dragMoveEvent->acceptProposedAction();
}

void MapViewBase::dropEvent(QDropEvent* dropEvent)
{
  dragDrop(
    static_cast<float>(dropEvent->position().x()),
    static_cast<float>(dropEvent->position().y()),
    dropEvent->mimeData()->text().toStdString());
  dropEvent->acceptProposedAction();
}

QMenu* MapViewBase::makeEntityGroupsMenu(const mdl::EntityDefinitionType type)
{
  auto* menu = new QMenu{};

  switch (type)
  {
  case mdl::EntityDefinitionType::PointEntity:
    menu->setTitle(tr("Create Point Entity"));
    break;
  case mdl::EntityDefinitionType::BrushEntity:
    menu->setTitle(tr("Create Brush Entity"));
    break;
  }

  const bool enableMakeBrushEntity = canCreateBrushEntity();
  size_t id = 0;

  auto document = kdl::mem_lock(m_document);
  for (const auto& group : document->entityDefinitionManager().groups())
  {
    const auto definitions =
      group.definitions(type, mdl::EntityDefinitionSortOrder::Name);

    const auto filteredDefinitions = kdl::vec_filter(definitions, [](auto* definition) {
      return !kdl::cs::str_is_equal(
        definition->name(), mdl::EntityPropertyValues::WorldspawnClassname);
    });

    if (!filteredDefinitions.empty())
    {
      const auto groupName = QString::fromStdString(group.displayName());
      auto* groupMenu = new QMenu{groupName};

      for (auto* definition : filteredDefinitions)
      {
        const auto label = QString::fromStdString(definition->shortName());
        QAction* action = nullptr;

        switch (type)
        {
        case mdl::EntityDefinitionType::PointEntity: {
          action = groupMenu->addAction(
            label, this, qOverload<>(&MapViewBase::createPointEntity));
          break;
        }
        case mdl::EntityDefinitionType::BrushEntity: {
          action = groupMenu->addAction(
            label, this, qOverload<>(&MapViewBase::createBrushEntity));
          action->setEnabled(enableMakeBrushEntity);
          break;
        }
        }

        // TODO: Would be cleaner to pass this as the string entity name
        action->setData(QVariant::fromValue<size_t>(id++));
      }

      menu->addMenu(groupMenu);
    }
  }

  return menu;
}

void MapViewBase::addSelectedObjectsToGroup()
{
  auto document = kdl::mem_lock(m_document);
  const auto nodes = document->selectedNodes().nodes();
  auto* newGroup = findNewGroupForObjects(nodes);
  ensure(newGroup != nullptr, "newGroup is null");

  auto transaction = Transaction{document, "Add Objects to Group"};
  reparentNodes(nodes, newGroup, true);
  document->deselectAll();
  document->selectNodes({newGroup});
  transaction.commit();
}

void MapViewBase::removeSelectedObjectsFromGroup()
{
  auto document = kdl::mem_lock(m_document);
  const auto nodes = document->selectedNodes().nodes();
  auto* currentGroup = document->editorContext().currentGroup();
  ensure(currentGroup != nullptr, "currentGroup is null");

  auto transaction = Transaction{document, "Remove Objects from Group"};
  reparentNodes(nodes, document->currentLayer(), true);

  while (document->currentGroup() != nullptr)
  {
    document->closeGroup();
  }
  document->selectNodes(nodes);
  transaction.commit();
}

mdl::Node* MapViewBase::findNewGroupForObjects(const std::vector<mdl::Node*>& nodes) const
{
  using namespace mdl::HitFilters;

  const auto hits = pickResult().all(type(mdl::nodeHitType()));
  if (!hits.empty())
  {
    auto* newGroup = mdl::findOutermostClosedGroup(mdl::hitToNode(hits.front()));
    if (newGroup && canReparentNodes(nodes, newGroup))
    {
      return newGroup;
    }
  }
  return nullptr;
}

void MapViewBase::mergeSelectedGroups()
{
  auto document = kdl::mem_lock(m_document);
  auto* newGroup = findGroupToMergeGroupsInto(document->selectedNodes());
  ensure(newGroup != nullptr, "newGroup is null");

  auto transaction = Transaction{document, "Merge Groups"};
  document->mergeSelectedGroupsWithGroup(newGroup);
  transaction.commit();
}

mdl::GroupNode* MapViewBase::findGroupToMergeGroupsInto(
  const mdl::NodeCollection& selectedNodes) const
{
  using namespace mdl::HitFilters;

  if (!(selectedNodes.hasOnlyGroups() && selectedNodes.groupCount() >= 2))
  {
    return nullptr;
  }

  auto document = kdl::mem_lock(m_document);
  const auto hits = pickResult().all(type(mdl::nodeHitType()));
  if (!hits.empty())
  {
    if (auto* mergeTarget = findOutermostClosedGroup(mdl::hitToNode(hits.front())))
    {
      if (kdl::all_of(selectedNodes.nodes(), [&](const auto* node) {
            return node == mergeTarget || canReparentNode(node, mergeTarget);
          }))
      {
        return mergeTarget;
      }
    }
  }

  return nullptr;
}

bool MapViewBase::canReparentNode(const mdl::Node* node, const mdl::Node* newParent) const
{
  return newParent != node && newParent != node->parent() && newParent->canAddChild(node);
}

void MapViewBase::moveSelectedBrushesToEntity()
{
  auto document = kdl::mem_lock(m_document);
  const auto nodes = document->selectedNodes().nodes();
  auto* newParent = findNewParentEntityForBrushes(nodes);
  ensure(newParent != nullptr, "newParent is null");

  auto transaction =
    Transaction{document, "Move " + kdl::str_plural(nodes.size(), "Brush", "Brushes")};
  reparentNodes(nodes, newParent, false);

  document->deselectAll();
  document->selectNodes(nodes);
  transaction.commit();
}

mdl::Node* MapViewBase::findNewParentEntityForBrushes(
  const std::vector<mdl::Node*>& nodes) const
{
  using namespace mdl::HitFilters;

  auto document = kdl::mem_lock(m_document);
  const auto& hit = pickResult().first(type(mdl::BrushNode::BrushHitType));
  if (const auto faceHandle = mdl::hitToFaceHandle(hit))
  {
    auto* brush = faceHandle->node();
    auto* newParent = brush->entity();

    if (newParent && newParent != document->world() && canReparentNodes(nodes, newParent))
    {
      return newParent;
    }
  }

  if (!nodes.empty())
  {
    auto* lastNode = nodes.back();

    if (auto* group = mdl::findContainingGroup(lastNode))
    {
      return group;
    }

    if (auto* layer = mdl::findContainingLayer(lastNode))
    {
      return layer;
    }
  }

  return document->currentLayer();
}

bool MapViewBase::canReparentNodes(
  const std::vector<mdl::Node*>& nodes, const mdl::Node* newParent) const
{
  return std::any_of(nodes.begin(), nodes.end(), [&](const auto* node) {
    return canReparentNode(node, newParent);
  });
}

/**
 * Return the given nodes, but replace all entity brushes with the parent entity (with
 * duplicates removed).
 */
static std::vector<mdl::Node*> collectEntitiesForNodes(
  const std::vector<mdl::Node*>& selectedNodes, const mdl::WorldNode* world)
{
  auto result = std::vector<mdl::Node*>{};
  const auto addNode = [&](auto&& thisLambda, auto* node) {
    if (node->entity() == world)
    {
      result.push_back(node);
    }
    else
    {
      node->visitParent(thisLambda);
    }
  };

  mdl::Node::visitAll(
    selectedNodes,
    kdl::overload(
      [](mdl::WorldNode*) {},
      [](mdl::LayerNode*) {},
      [&](mdl::GroupNode* group) { result.push_back(group); },
      [&](mdl::EntityNode* entity) { result.push_back(entity); },
      [&](auto&& thisLambda, mdl::BrushNode* brush) { addNode(thisLambda, brush); },
      [&](auto&& thisLambda, mdl::PatchNode* patch) { addNode(thisLambda, patch); }));
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

void MapViewBase::reparentNodes(
  const std::vector<mdl::Node*>& nodes, mdl::Node* newParent, const bool preserveEntities)
{
  ensure(newParent != nullptr, "newParent is null");

  auto document = kdl::mem_lock(m_document);
  const auto inputNodes =
    preserveEntities ? collectEntitiesForNodes(nodes, document->world()) : nodes;

  const auto reparentableNodes = collectReparentableNodes(inputNodes, newParent);
  assert(!reparentableNodes.empty());

  const auto name = "Move "
                    + kdl::str_plural(reparentableNodes.size(), "Object", "Objects")
                    + " to " + newParent->name();

  auto transaction = Transaction{document, name};
  document->deselectAll();
  if (!document->reparentNodes({{newParent, reparentableNodes}}))
  {
    transaction.cancel();
    return;
  }
  document->selectNodes(reparentableNodes);
  transaction.commit();
}

std::vector<mdl::Node*> MapViewBase::collectReparentableNodes(
  const std::vector<mdl::Node*>& nodes, const mdl::Node* newParent) const
{
  return kdl::vec_filter(nodes, [&](const auto* node) {
    return newParent != node && newParent != node->parent()
           && !newParent->isDescendantOf(node);
  });
}

bool MapViewBase::canMergeGroups() const
{
  auto document = kdl::mem_lock(m_document);
  auto* mergeGroup = findGroupToMergeGroupsInto(document->selectedNodes());
  return mergeGroup != nullptr;
}

bool MapViewBase::canMakeStructural() const
{
  auto document = kdl::mem_lock(m_document);
  if (document->selectedNodes().hasOnlyBrushes())
  {
    const auto& brushes = document->selectedNodes().brushes();
    return std::any_of(brushes.begin(), brushes.end(), [&](const auto* brush) {
      return brush->hasAnyTag() || brush->entity() != document->world()
             || brush->anyFaceHasAnyTag();
    });
  }
  return false;
}

} // namespace tb::ui
