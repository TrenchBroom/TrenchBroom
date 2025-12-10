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
#include "mdl/EditorContext.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionGroup.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityDefinitionUtils.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/Game.h"
#include "mdl/GameConfig.h"
#include "mdl/Grid.h"
#include "mdl/GroupNode.h"
#include "mdl/HitAdapter.h"
#include "mdl/HitFilter.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Layers.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Map_World.h"
#include "mdl/ModelUtils.h"
#include "mdl/PatchNode.h"
#include "mdl/PointTrace.h"
#include "mdl/Transaction.h"
#include "mdl/UpdateBrushFaceAttributes.h"
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
#include "ui/MapDocument.h"
#include "ui/MapFrame.h"
#include "ui/MapViewActivationTracker.h"
#include "ui/MapViewToolBox.h"
#include "ui/QtUtils.h"
#include "ui/SelectionTool.h"
#include "ui/SignalDelayer.h"

#include "kd/contracts.h"
#include "kd/ranges/to.h"
#include "kd/string_compare.h"
#include "kd/string_format.h"
#include "kd/vector_utils.h"

#include "vm/polygon.h"
#include "vm/util.h"

#include <algorithm>
#include <ranges>
#include <vector>

namespace tb::ui
{
const int MapViewBase::DefaultCameraAnimationDuration = 250;

MapViewBase::MapViewBase(
  MapDocument& document, MapViewToolBox& toolBox, GLContextManager& contextManager)
  : RenderView{contextManager}
  , m_document{document}
  , m_toolBox{toolBox}
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
  m_notifierConnection +=
    m_document.documentWasLoadedNotifier.connect(this, &MapViewBase::documentWasLoaded);
  m_notifierConnection +=
    m_document.documentDidChangeNotifier.connect(this, &MapViewBase::documentDidChange);

  m_notifierConnection += m_document.materialCollectionsDidChangeNotifier.connect(
    this, &MapViewBase::materialCollectionsDidChange);
  m_notifierConnection += m_document.entityDefinitionsDidChangeNotifier.connect(
    this, &MapViewBase::entityDefinitionsDidChange);
  m_notifierConnection +=
    m_document.modsDidChangeNotifier.connect(this, &MapViewBase::modsDidChange);
  m_notifierConnection += m_document.editorContextDidChangeNotifier.connect(
    this, &MapViewBase::editorContextDidChange);
  m_notifierConnection +=
    m_document.pointFileWasLoadedNotifier.connect(this, &MapViewBase::pointFileDidChange);
  m_notifierConnection += m_document.pointFileWasUnloadedNotifier.connect(
    this, &MapViewBase::pointFileDidChange);
  m_notifierConnection += m_document.portalFileWasLoadedNotifier.connect(
    this, &MapViewBase::portalFileDidChange);
  m_notifierConnection += m_document.portalFileWasUnloadedNotifier.connect(
    this, &MapViewBase::portalFileDidChange);

  m_notifierConnection +=
    m_document.gridDidChangeNotifier.connect(this, &MapViewBase::gridDidChange);

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

void MapViewBase::documentWasLoaded()
{
  createActionsAndUpdatePicking();
  update();
}

void MapViewBase::documentDidChange()
{
  updatePickResult();
  updateActionStates();
  update();
}

void MapViewBase::toolChanged(Tool&)
{
  updatePickResult();
  updateActionStates();
  update();
}

void MapViewBase::materialCollectionsDidChange()
{
  update();
}

void MapViewBase::entityDefinitionsDidChange()
{
  createActions();
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

  m_document.visitTagActions(visitor);
  m_document.visitEntityDefinitionActions(visitor);
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
  const auto& map = m_document.map();
  const auto& grid = map.grid();
  const auto delta = moveDirection(direction) * double(grid.actualSize());
  m_toolBox.moveRotationCenter(delta);
  update();
}

void MapViewBase::moveVertices(const vm::direction direction)
{
  const auto& map = m_document.map();
  const auto& grid = map.grid();
  const auto delta = moveDirection(direction) * double(grid.actualSize());
  m_toolBox.moveVertices(delta);
}

void MapViewBase::moveObjects(const vm::direction direction)
{
  auto& map = m_document.map();
  const auto& grid = map.grid();
  const auto delta = moveDirection(direction) * double(grid.actualSize());
  translateSelection(map, delta);
}

void MapViewBase::duplicateObjects()
{
  auto& map = m_document.map();
  if (map.selection().hasNodes())
  {
    duplicateSelectedNodes(map);
  }
}

void MapViewBase::duplicateAndMoveObjects(const vm::direction direction)
{
  auto& map = m_document.map();
  auto transaction = mdl::Transaction{map};
  duplicateObjects();
  moveObjects(direction);
  transaction.commit();
}

void MapViewBase::rotate(const vm::rotation_axis axisSpec, const bool clockwise)
{
  auto& map = m_document.map();
  if (const auto& bounds = map.selectionBounds())
  {
    const auto axis = rotationAxis(axisSpec, clockwise);
    const auto angle = m_toolBox.rotateToolActive() ? vm::abs(m_toolBox.rotateToolAngle())
                                                    : vm::Cd::half_pi();

    const auto& grid = map.grid();
    const auto center = m_toolBox.rotateToolActive() ? m_toolBox.rotateToolCenter()
                                                     : grid.referencePoint(*bounds);

    rotateSelection(map, center, axis, angle);
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

void MapViewBase::flip(const vm::direction direction)
{
  if (canFlip())
  {
    auto& map = m_document.map();

    // If we snap the selection bounds' center to the grid size, then
    // selections that are an odd number of grid units wide get translated.
    // Instead, snap to 1/2 the grid size.
    // (see: https://github.com/TrenchBroom/TrenchBroom/issues/1495 )
    auto halfGrid = mdl::Grid{map.grid().size()};
    halfGrid.decSize();

    const auto center = halfGrid.referencePoint(*map.selectionBounds());
    const auto axis = flipAxis(direction);

    flipSelection(map, center, axis);
  }
}

bool MapViewBase::canFlip() const
{
  const auto& map = m_document.map();
  return !m_toolBox.anyModalToolActive() && map.selection().hasNodes();
}

void MapViewBase::moveUV(const vm::direction direction, const UVActionMode mode)
{
  auto& map = m_document.map();
  if (map.selection().hasBrushFaces())
  {
    const auto offset = moveUVOffset(direction, mode);
    translateUV(map, camera().up(), camera().right(), offset);
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
  const auto& map = m_document.map();
  const auto& grid = map.grid();
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
  auto& map = m_document.map();
  if (map.selection().hasBrushFaces())
  {
    const auto angle = rotateUVAngle(clockwise, mode);
    mdl::rotateUV(map, angle);
  }
}

float MapViewBase::rotateUVAngle(const bool clockwise, const UVActionMode mode) const
{
  const auto& map = m_document.map();
  const auto& grid = map.grid();
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
  auto& map = m_document.map();
  if (map.selection().hasBrushFaces())
  {
    mdl::flipUV(map, camera().up(), camera().right(), direction);
  }
}

void MapViewBase::resetUV()
{
  auto& map = m_document.map();
  setBrushFaceAttributes(
    map, mdl::resetAll(map.game().config().faceAttribsConfig.defaults));
}

void MapViewBase::resetUVToWorld()
{
  auto& map = m_document.map();
  setBrushFaceAttributes(
    map, mdl::resetAllToParaxial(map.game().config().faceAttribsConfig.defaults));
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
    auto& map = m_document.map();
    if (map.selection().hasAny())
    {
      deselectAll(map);
    }
    else if (map.editorContext().currentGroup())
    {
      closeGroup(map);
    }
  }
}

void MapViewBase::deactivateCurrentTool()
{
  m_toolBox.deactivateCurrentTool();
}

void MapViewBase::createPointEntity()
{
  auto* action = qobject_cast<const QAction*>(sender());
  auto& map = m_document.map();

  const auto classname = action->data().toString().toStdString();
  if (const auto* definition = map.entityDefinitionManager().definition(classname))
  {
    contract_assert(getType(*definition) == mdl::EntityDefinitionType::Point);
    createPointEntity(*definition);
  }
  else
  {
    map.logger().error() << "Unknown entity classname: " << classname;
  }
}

void MapViewBase::createBrushEntity()
{
  auto* action = qobject_cast<const QAction*>(sender());
  auto& map = m_document.map();

  const auto classname = action->data().toString().toStdString();
  if (const auto* definition = map.entityDefinitionManager().definition(classname))
  {
    createBrushEntity(*definition);
  }
  else
  {
    map.logger().error() << "Unknown entity classname: " << classname;
  }
}

void MapViewBase::createPointEntity(const mdl::EntityDefinition& definition)
{
  contract_pre(definition.pointEntityDefinition);

  auto& map = m_document.map();
  const auto delta = computePointEntityPosition(definition.pointEntityDefinition->bounds);
  mdl::createPointEntity(map, definition, delta);
}

void MapViewBase::createBrushEntity(const mdl::EntityDefinition& definition)
{
  auto& map = m_document.map();
  mdl::createBrushEntity(map, definition);
}

bool MapViewBase::canCreateBrushEntity()
{
  const auto& map = m_document.map();
  return map.selection().hasOnlyBrushes();
}

void MapViewBase::toggleTagVisible(const mdl::SmartTag& tag)
{
  const auto tagIndex = tag.index();

  auto& map = m_document.map();
  auto& editorContext = map.editorContext();
  auto hiddenTags = editorContext.hiddenTags();
  hiddenTags ^= mdl::TagType::Type{1} << tagIndex;
  editorContext.setHiddenTags(hiddenTags);
}

void MapViewBase::enableTag(const mdl::SmartTag& tag)
{
  contract_pre(tag.canEnable());

  auto& map = m_document.map();
  auto transaction = mdl::Transaction{map, "Turn Selection into " + tag.name()};
  auto callback = EnableDisableTagCallback{};
  tag.enable(callback, map);
  transaction.commit();
}

void MapViewBase::disableTag(const mdl::SmartTag& tag)
{
  contract_pre(tag.canDisable());

  auto& map = m_document.map();
  auto transaction = mdl::Transaction{map, "Turn Selection into non-" + tag.name()};
  auto callback = EnableDisableTagCallback{};
  tag.disable(callback, map);
  transaction.commit();
}

void MapViewBase::makeStructural()
{
  auto& map = m_document.map();
  if (!map.selection().hasBrushes())
  {
    return;
  }

  auto toReparent = std::vector<mdl::Node*>{};
  const auto& selectedBrushes = map.selection().brushes;
  std::ranges::copy_if(
    selectedBrushes, std::back_inserter(toReparent), [&](const auto* brushNode) {
      return brushNode->entity() != &map.worldNode();
    });

  auto transaction = mdl::Transaction{map, "Make Structural"};

  if (!toReparent.empty())
  {
    reparentNodes(toReparent, parentForNodes(map, toReparent), false);
  }

  auto anyTagDisabled = false;
  auto callback = EnableDisableTagCallback{};
  for (auto* brush : map.selection().brushes)
  {
    for (const auto& tag : map.smartTags())
    {
      if (brush->hasTag(tag) || brush->anyFacesHaveAnyTagInMask(tag.type()))
      {
        anyTagDisabled = true;
        tag.disable(callback, map);
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

void MapViewBase::toggleEntityDefinitionVisible(const mdl::EntityDefinition& definition)
{
  auto& map = m_document.map();

  auto& editorContext = map.editorContext();
  editorContext.setEntityDefinitionHidden(
    definition, !editorContext.entityDefinitionHidden(definition));
}

void MapViewBase::createEntity(const mdl::EntityDefinition& definition)
{
  switch (getType(definition))
  {
  case mdl::EntityDefinitionType::Point:
    createPointEntity(definition);
    break;
  case mdl::EntityDefinitionType::Brush:
    if (canCreateBrushEntity())
    {
      createBrushEntity(definition);
    }
    break;
    switchDefault();
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
  const auto& map = m_document.map();

  const auto viewContext = viewActionContext();
  const auto toolContext =
    m_toolBox.assembleBrushToolActive() ? ActionContext::AssembleBrushTool
    : m_toolBox.clipToolActive()        ? ActionContext::ClipTool
    : m_toolBox.anyVertexToolActive()   ? ActionContext::AnyVertexTool
    : m_toolBox.rotateToolActive()      ? ActionContext::RotateTool
    : m_toolBox.scaleToolActive()       ? ActionContext::ScaleTool
    : m_toolBox.shearToolActive()       ? ActionContext::ShearTool
                                        : ActionContext::NoTool;
  const auto selectionContext = map.selection().hasNodes() ? ActionContext::NodeSelection
                                : map.selection().hasBrushFaces()
                                  ? ActionContext::FaceSelection
                                  : ActionContext::NoSelection;
  return viewContext | toolContext | selectionContext;
}

void MapViewBase::flashSelection()
{
  auto animation =
    std::make_unique<FlashSelectionAnimation>(m_document.mapRenderer(), this, 180);
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
    auto& logger = m_document.logger();
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

  const auto& map = m_document.map();
  const auto& grid = map.grid();

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
      ? vm::bbox3f{softMapBounds(map).bounds.value_or(vm::bbox3d{})}
      : vm::bbox3f{});

  setupGL(renderContext);
  setRenderOptions(renderContext);

  auto renderBatch = render::RenderBatch{vboManager()};

  renderGrid(renderContext, renderBatch);
  renderMap(m_document.mapRenderer(), renderContext, renderBatch);
  renderTools(m_toolBox, renderContext, renderBatch);

  renderCoordinateSystem(renderContext, renderBatch);
  renderSoftWorldBounds(renderContext, renderBatch);
  renderPointFile(renderContext, renderBatch);
  renderPortalFile(renderContext, renderBatch);
  renderCompass(renderBatch);
  renderFPS(renderContext, renderBatch);

  renderBatch.render(renderContext);

  if (map.needsResourceProcessing())
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
    const auto& map = m_document.map();
    const auto& worldBounds = map.worldBounds();

    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.renderCoordinateSystem(vm::bbox3f{worldBounds});
  }
}

void MapViewBase::renderSoftWorldBounds(render::RenderContext&, render::RenderBatch&) {}

void MapViewBase::renderPointFile(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  if (const auto* pointFile = m_document.pointTrace())
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
    contract_assert(m_portalFileRenderer);
  }
  renderBatch.add(m_portalFileRenderer.get());
}

void MapViewBase::invalidatePortalFileRenderer()
{
  m_portalFileRenderer = nullptr;
}

void MapViewBase::validatePortalFileRenderer(render::RenderContext&)
{
  contract_pre(m_portalFileRenderer == nullptr);

  m_portalFileRenderer = std::make_unique<render::PrimitiveRenderer>();

  if (const auto* portals = m_document.portals())
  {
    for (const auto& portal : *portals)
    {
      m_portalFileRenderer->renderFilledPolygon(
        pref(Preferences::PortalFileFillColor),
        render::PrimitiveRendererOcclusionPolicy::Hide,
        render::PrimitiveRendererCullingPolicy::ShowBackfaces,
        portal.vertices());

      const auto lineWidth = 4.0f;
      m_portalFileRenderer->renderPolygon(
        pref(Preferences::PortalFileBorderColor),
        lineWidth,
        render::PrimitiveRendererOcclusionPolicy::Hide,
        portal.vertices());
    }
  }
}

void MapViewBase::renderCompass(render::RenderBatch& renderBatch)
{
  if (m_compass)
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

  auto& map = m_document.map();
  const auto& nodes = map.selection().nodes;
  auto* newBrushParent = findNewParentEntityForBrushes(nodes);
  auto* currentGroup = map.editorContext().currentGroup();
  auto* newGroup = findNewGroupForObjects(nodes);
  auto* mergeGroup = findGroupToMergeGroupsInto(map.selection());

  auto* mapFrame = findMapFrame(this);

  auto menu = QMenu{};
  const auto addMainMenuAction = [&](const auto& path) -> QAction* {
    auto* groupAction = mapFrame->findAction(path);
    contract_assert(groupAction);

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

  addMainMenuAction("Menu/Edit/Rename Groups");

  if (newGroup && canReparentNodes(nodes, newGroup))
  {
    menu.addAction(
      tr("Add Objects to Group %1").arg(QString::fromStdString(newGroup->name())),
      this,
      &MapViewBase::addSelectedObjectsToGroup);
  }
  if (currentGroup && map.selection().hasNodes())
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
  for (auto* layerNode : map.worldNode().allLayersUserSorted())
  {
    auto* action = moveSelectionTo->addAction(
      QString::fromStdString(layerNode->name()), this, [&map, layerNode] {
        moveSelectedNodesToLayer(map, layerNode);
      });
    action->setEnabled(canMoveSelectedNodesToLayer(map, layerNode));
  }

  const auto moveSelectionToItems = moveSelectionTo->actions();
  moveSelectionTo->setEnabled(std::ranges::any_of(
    moveSelectionToItems, [](QAction* action) { return action->isEnabled(); }));

  if (selectedObjectLayers.size() == 1u)
  {
    auto* layerNode = selectedObjectLayers[0];
    auto* action = menu.addAction(
      tr("Make Layer %1 Active").arg(QString::fromStdString(layerNode->name())),
      this,
      [&map, layerNode]() { setCurrentLayer(map, layerNode); });
    action->setEnabled(canSetCurrentLayer(map, layerNode));
  }
  else
  {
    auto* makeLayerActive = menu.addMenu(tr("Make Layer Active"));
    for (auto* layerNode : selectedObjectLayers)
    {
      auto* action = makeLayerActive->addAction(
        QString::fromStdString(layerNode->name()), this, [&map, layerNode]() {
          setCurrentLayer(map, layerNode);
        });
      action->setEnabled(canSetCurrentLayer(map, layerNode));
    }
    if (makeLayerActive->isEmpty())
    {
      makeLayerActive->setDisabled(true);
    }
  }

  auto* hideLayersAction =
    menu.addAction(tr("Hide Layers"), this, [&map, selectedObjectLayers]() {
      hideLayers(map, selectedObjectLayers);
    });
  hideLayersAction->setEnabled(canHideLayers(selectedObjectLayers));
  auto* isolateLayersAction =
    menu.addAction(tr("Isolate Layers"), this, [&map, selectedObjectLayers]() {
      isolateLayers(map, selectedObjectLayers);
    });
  isolateLayersAction->setEnabled(canIsolateLayers(map, selectedObjectLayers));
  auto* selectAllInLayersAction =
    menu.addAction(tr("Select All in Layers"), this, [&map, selectedObjectLayers]() {
      selectAllInLayers(map, selectedObjectLayers);
    });
  selectAllInLayersAction->setEnabled(canSelectAllInLayers(map, selectedObjectLayers));

  menu.addSeparator();

  if (map.selection().hasOnlyBrushes())
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

  menu.addMenu(makeEntityGroupsMenu(mdl::EntityDefinitionType::Point));
  menu.addMenu(makeEntityGroupsMenu(mdl::EntityDefinitionType::Brush));

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
  case mdl::EntityDefinitionType::Point:
    menu->setTitle(tr("Create Point Entity"));
    break;
  case mdl::EntityDefinitionType::Brush:
    menu->setTitle(tr("Create Brush Entity"));
    break;
  }

  const auto enableMakeBrushEntity = canCreateBrushEntity();

  const auto& map = m_document.map();
  for (const auto& group : map.entityDefinitionManager().groups())
  {
    auto creatableDefinitions =
      filterAndSort(group.definitions, type, mdl::EntityDefinitionSortOrder::Name)
      | std::views::filter([](const auto* d) {
          return !kdl::cs::str_is_equal(
            d->name, mdl::EntityPropertyValues::WorldspawnClassname);
        });

    if (!std::ranges::empty(creatableDefinitions))
    {
      const auto groupName = QString::fromStdString(displayName(group));
      auto* groupMenu = new QMenu{groupName};

      for (const auto* definition : creatableDefinitions)
      {
        const auto label = fromStdStringView(mdl::getShortName(*definition));
        QAction* action = nullptr;

        switch (type)
        {
        case mdl::EntityDefinitionType::Point:
          action = groupMenu->addAction(
            label, this, qOverload<>(&MapViewBase::createPointEntity));
          break;
        case mdl::EntityDefinitionType::Brush:
          action = groupMenu->addAction(
            label, this, qOverload<>(&MapViewBase::createBrushEntity));
          action->setEnabled(enableMakeBrushEntity);
          break;
        }

        action->setData(QVariant::fromValue(QString::fromStdString(definition->name)));
      }

      menu->addMenu(groupMenu);
    }
  }

  return menu;
}

void MapViewBase::addSelectedObjectsToGroup()
{
  auto& map = m_document.map();
  const auto nodes = map.selection().nodes;

  auto* newGroup = findNewGroupForObjects(nodes);
  contract_assert(newGroup != nullptr);

  auto transaction = mdl::Transaction{map, "Add Objects to Group"};
  reparentNodes(nodes, newGroup, true);
  deselectAll(map);
  selectNodes(map, {newGroup});
  transaction.commit();
}

void MapViewBase::removeSelectedObjectsFromGroup()
{
  auto& map = m_document.map();
  const auto& editorContext = map.editorContext();

  const auto nodes = map.selection().nodes;
  auto* currentGroup = editorContext.currentGroup();
  contract_assert(currentGroup);

  auto transaction = mdl::Transaction{map, "Remove Objects from Group"};
  reparentNodes(nodes, editorContext.currentLayer(), true);

  while (editorContext.currentGroup())
  {
    closeGroup(map);
  }
  selectNodes(map, nodes);
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
  auto& map = m_document.map();
  auto* newGroup = findGroupToMergeGroupsInto(map.selection());
  contract_assert(newGroup != nullptr);

  auto transaction = mdl::Transaction{map, "Merge Groups"};
  mergeSelectedGroupsWithGroup(map, newGroup);
  transaction.commit();
}

mdl::GroupNode* MapViewBase::findGroupToMergeGroupsInto(
  const mdl::Selection& selection) const
{
  using namespace mdl::HitFilters;

  if (!(selection.hasOnlyGroups() && selection.groups.size() >= 2))
  {
    return nullptr;
  }

  const auto hits = pickResult().all(type(mdl::nodeHitType()));
  if (!hits.empty())
  {
    if (auto* mergeTarget = findOutermostClosedGroup(mdl::hitToNode(hits.front())))
    {
      if (std::ranges::all_of(selection.nodes, [&](const auto* node) {
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
  auto& map = m_document.map();
  const auto nodes = map.selection().nodes;
  auto* newParent = findNewParentEntityForBrushes(nodes);
  contract_assert(newParent);

  auto transaction =
    mdl::Transaction{map, "Move " + kdl::str_plural(nodes.size(), "Brush", "Brushes")};
  reparentNodes(nodes, newParent, false);

  deselectAll(map);
  selectNodes(map, nodes);
  transaction.commit();
}

mdl::Node* MapViewBase::findNewParentEntityForBrushes(
  const std::vector<mdl::Node*>& nodes) const
{
  using namespace mdl::HitFilters;

  const auto& map = m_document.map();
  const auto& hit = pickResult().first(type(mdl::BrushNode::BrushHitType));
  if (const auto faceHandle = mdl::hitToFaceHandle(hit))
  {
    auto* brush = faceHandle->node();
    auto* newParent = brush->entity();

    if (newParent && newParent != &map.worldNode() && canReparentNodes(nodes, newParent))
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

  return map.editorContext().currentLayer();
}

bool MapViewBase::canReparentNodes(
  const std::vector<mdl::Node*>& nodes, const mdl::Node* newParent) const
{
  return std::ranges::any_of(
    nodes, [&](const auto* node) { return canReparentNode(node, newParent); });
}

/**
 * Return the given nodes, but replace all entity brushes with the parent entity (with
 * duplicates removed).
 */
static std::vector<mdl::Node*> collectEntitiesForNodes(
  const std::vector<mdl::Node*>& selectedNodes, const mdl::WorldNode& worldNode)
{
  auto result = std::vector<mdl::Node*>{};
  const auto addNode = [&](auto&& thisLambda, auto* node) {
    if (node->entity() == &worldNode)
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
  contract_pre(newParent != nullptr);

  auto& map = m_document.map();
  const auto inputNodes =
    preserveEntities ? collectEntitiesForNodes(nodes, map.worldNode()) : nodes;

  const auto reparentableNodes = collectReparentableNodes(inputNodes, newParent);
  contract_assert(!reparentableNodes.empty());

  const auto name = "Move "
                    + kdl::str_plural(reparentableNodes.size(), "Object", "Objects")
                    + " to " + newParent->name();

  auto transaction = mdl::Transaction{map, name};
  deselectAll(map);
  if (!mdl::reparentNodes(map, {{newParent, reparentableNodes}}))
  {
    transaction.cancel();
    return;
  }
  selectNodes(map, reparentableNodes);
  transaction.commit();
}

std::vector<mdl::Node*> MapViewBase::collectReparentableNodes(
  const std::vector<mdl::Node*>& nodes, const mdl::Node* newParent) const
{
  return nodes | std::views::filter([&](const auto* node) {
           return newParent != node && newParent != node->parent()
                  && !newParent->isDescendantOf(node);
         })
         | kdl::ranges::to<std::vector>();
}

bool MapViewBase::canMergeGroups() const
{
  const auto& map = m_document.map();
  auto* mergeGroup = findGroupToMergeGroupsInto(map.selection());
  return mergeGroup;
}

bool MapViewBase::canMakeStructural() const
{
  const auto& map = m_document.map();
  if (map.selection().hasOnlyBrushes())
  {
    const auto& brushes = map.selection().brushes;
    return std::ranges::any_of(brushes, [&](const auto* brush) {
      return brush->hasAnyTag() || brush->entity() != &map.worldNode()
             || brush->anyFaceHasAnyTag();
    });
  }
  return false;
}

} // namespace tb::ui
