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

#include "MapView2D.h"

#include "Macros.h"
#include "gl/ContextManager.h"
#include "gl/OrthographicCamera.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/Grid.h"
#include "mdl/HitAdapter.h"
#include "mdl/HitFilter.h"
#include "mdl/Map_Picking.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/PickResult.h"
#include "mdl/PointTrace.h"
#include "render/Compass2D.h"
#include "render/GridRenderer.h"
#include "render/MapRenderer.h"
#include "render/RenderContext.h"
#include "render/SelectionBoundsRenderer.h"
#include "ui/CameraAnimation.h"
#include "ui/CameraLinkHelper.h"
#include "ui/CameraTool2D.h"
#include "ui/ClipToolController.h"
#include "ui/CreateEntityToolController.h"
#include "ui/DrawShapeToolController2D.h"
#include "ui/EdgeTool.h"
#include "ui/EdgeToolController.h"
#include "ui/ExtrudeToolController.h"
#include "ui/FaceTool.h" // IWYU pragma: keep
#include "ui/FaceToolController.h"
#include "ui/MapDocument.h"
#include "ui/MapViewToolBox.h"
#include "ui/MoveObjectsToolController.h"
#include "ui/RotateToolController.h"
#include "ui/ScaleToolController.h"
#include "ui/SelectionTool.h"
#include "ui/ShearToolController.h"
#include "ui/VertexTool.h"
#include "ui/VertexToolController.h"

#include "kd/contracts.h"

#include "vm/util.h"

namespace tb::ui
{

MapView2D::MapView2D(
  MapDocument& document,
  MapViewToolBox& toolBox,
  gl::ContextManager& contextManager,
  ViewPlane viewPlane)
  : MapViewBase{document, toolBox, contextManager}
  , m_camera{std::make_unique<gl::OrthographicCamera>()}
{
  connectObservers();
  initializeCamera(viewPlane);
  initializeToolChain(toolBox);

  switch (viewPlane)
  {
  case ViewPlane::XY:
    setObjectName("XY View");
    break;
  case ViewPlane::YZ:
    setObjectName("YZ View");
    break;
  case ViewPlane::XZ:
    setObjectName("XZ View");
    break;
    switchDefault();
  }

  mapViewBaseVirtualInit();
}

void MapView2D::initializeCamera(const ViewPlane viewPlane)
{
  const auto& map = m_document.map();
  const auto worldBounds = vm::bbox3f{map.worldBounds()};

  switch (viewPlane)
  {
  case MapView2D::ViewPlane::XY:
    m_camera->setDirection(vm::vec3f{0, 0, -1}, vm::vec3f{0, 1, 0});
    m_camera->moveTo(vm::vec3f{0.0f, 0.0f, worldBounds.max.z()});
    break;
  case MapView2D::ViewPlane::XZ:
    m_camera->setDirection(vm::vec3f{0, 1, 0}, vm::vec3f{0, 0, 1});
    m_camera->moveTo(vm::vec3f{0.0f, worldBounds.min.y(), 0.0f});
    break;
  case MapView2D::ViewPlane::YZ:
    m_camera->setDirection(vm::vec3f{-1, 0, 0}, vm::vec3f{0, 0, 1});
    m_camera->moveTo(vm::vec3f{worldBounds.max.x(), 0.0f, 0.0f});
    break;
  }
  m_camera->setNearPlane(1.0f);
  // NOTE: GridRenderer draws at the far side of the map bounds, so add some extra
  // margin so it's not fighting the far plane.
  m_camera->setFarPlane(worldBounds.size().x() + 16.0f);
}

void MapView2D::initializeToolChain(MapViewToolBox& toolBox)
{
  addToolController(std::make_unique<CameraTool2D>(*m_camera));
  addToolController(
    std::make_unique<MoveObjectsToolController>(toolBox.moveObjectsTool()));
  addToolController(std::make_unique<RotateToolController2D>(toolBox.rotateTool()));
  addToolController(std::make_unique<ScaleToolController2D>(toolBox.scaleTool()));
  addToolController(std::make_unique<ShearToolController2D>(toolBox.shearTool()));
  addToolController(std::make_unique<ExtrudeToolController2D>(toolBox.extrudeTool()));
  addToolController(std::make_unique<ClipToolController2D>(toolBox.clipTool()));
  addToolController(std::make_unique<VertexToolController>(toolBox.vertexTool()));
  addToolController(std::make_unique<EdgeToolController>(toolBox.edgeTool()));
  addToolController(std::make_unique<FaceToolController>(toolBox.faceTool()));
  addToolController(
    std::make_unique<CreateEntityToolController2D>(toolBox.createEntityTool()));
  addToolController(std::make_unique<SelectionTool>(m_document));
  addToolController(
    std::make_unique<DrawShapeToolController2D>(toolBox.drawShapeTool(), m_document));
}

void MapView2D::connectObservers()
{
  m_notifierConnection +=
    m_camera->cameraDidChangeNotifier.connect(this, &MapView2D::cameraDidChange);
}

void MapView2D::cameraDidChange(const gl::Camera*)
{
  update();
}

PickRequest MapView2D::pickRequest(const float x, const float y) const
{
  return {vm::ray3d{m_camera->pickRay(x, y)}, *m_camera};
}

mdl::PickResult MapView2D::pick(const vm::ray3d& pickRay) const
{
  auto& map = m_document.map();
  const auto axis = vm::find_abs_max_component(pickRay.direction);

  auto pickResult = mdl::PickResult::bySize(axis);
  mdl::pick(map, pickRay, pickResult);

  return pickResult;
}

void MapView2D::initializeGL()
{
  MapViewBase::initializeGL();
  setCompass(std::make_unique<render::Compass2D>());
}

void MapView2D::updateViewport(
  const int x, const int y, const int width, const int height)
{
  m_camera->setViewport({x, y, width, height});
}

vm::vec3d MapView2D::pasteObjectsDelta(
  const vm::bbox3d& bounds, const vm::bbox3d& referenceBounds) const
{
  const auto& map = m_document.map();
  const auto& grid = map.grid();
  const auto& worldBounds = map.worldBounds();

  const auto& pickRay = MapView2D::pickRay();

  const auto toMin = referenceBounds.min - pickRay.origin;
  const auto toMax = referenceBounds.max - pickRay.origin;
  const auto anchor = dot(toMin, pickRay.direction) > dot(toMax, pickRay.direction)
                        ? referenceBounds.min
                        : referenceBounds.max;
  const auto dragPlane = vm::plane3d(anchor, -pickRay.direction);

  return grid.moveDeltaForBounds(dragPlane, bounds, worldBounds, pickRay);
}

bool MapView2D::canSelectTall()
{
  return true;
}

void MapView2D::selectTall()
{
  const auto cameraAxis = vm::find_abs_max_component(m_camera->direction());
  selectTouchingNodes(m_document.map(), cameraAxis, true);
}

void MapView2D::reset2dCameras(const gl::Camera& masterCamera, const bool animate)
{
  const auto oldPosition = m_camera->position();
  const auto factors = vm::vec3f{1, 1, 1} - vm::abs(masterCamera.direction())
                       - vm::abs(m_camera->direction());
  const auto newPosition =
    (vm::vec3f{1, 1, 1} - factors) * oldPosition + factors * masterCamera.position();
  m_camera->moveTo(newPosition);

  if (animate)
  {
    animateCamera(
      newPosition, m_camera->direction(), m_camera->up(), masterCamera.zoom());
  }
  else
  {
    m_camera->moveTo(newPosition);
    m_camera->setZoom(masterCamera.zoom());
  }
}

void MapView2D::focusCameraOnSelection(const bool animate)
{
  const auto& map = m_document.map();
  const auto bounds = vm::bbox3f{map.referenceBounds()};
  const auto diff = bounds.center() - m_camera->position();
  const auto delta = vm::dot(diff, m_camera->up()) * m_camera->up()
                     + vm::dot(diff, m_camera->right()) * m_camera->right();
  moveCameraToPosition(m_camera->position() + delta, animate);
}

void MapView2D::moveCameraToPosition(const vm::vec3f& position, const bool animate)
{
  if (animate)
  {
    animateCamera(position, m_camera->direction(), m_camera->up(), m_camera->zoom());
  }
  else
  {
    m_camera->moveTo(position);
  }
}

void MapView2D::animateCamera(
  const vm::vec3f& position,
  const vm::vec3f& /* direction */,
  const vm::vec3f& /* up */,
  const float zoom,
  const int duration)
{
  const auto actualPosition =
    vm::dot(position, m_camera->up()) * m_camera->up()
    + vm::dot(position, m_camera->right()) * m_camera->right()
    + vm::dot(m_camera->position(), m_camera->direction()) * m_camera->direction();

  auto animation = std::make_unique<CameraAnimation>(
    *m_camera, actualPosition, m_camera->direction(), m_camera->up(), zoom, duration);
  m_animationManager->runAnimation(std::move(animation), true);
}

void MapView2D::moveCameraToCurrentTracePoint()
{
  contract_pre(m_document.isPointFileLoaded());

  if (const auto* pointTrace = m_document.pointTrace())
  {
    moveCameraToPosition(pointTrace->currentPoint(), true);
  }
}

gl::Camera& MapView2D::camera()
{
  return *m_camera;
}

vm::vec3d MapView2D::moveDirection(const vm::direction direction) const
{
  // The mapping is a bit counter intuitive, but it makes sense considering that the
  // cursor up key is usually bounds to the forward action (which makes sense in 3D),
  // but should move objects "up" in 2D.
  switch (direction)
  {
  case vm::direction::forward:
    return vm::vec3d{vm::get_abs_max_component_axis(m_camera->up())};
  case vm::direction::backward:
    return vm::vec3d{-vm::get_abs_max_component_axis(m_camera->up())};
  case vm::direction::left:
    return vm::vec3d{-vm::get_abs_max_component_axis(m_camera->right())};
  case vm::direction::right:
    return vm::vec3d{vm::get_abs_max_component_axis(m_camera->right())};
  case vm::direction::up:
    return vm::vec3d{-vm::get_abs_max_component_axis(m_camera->direction())};
  case vm::direction::down:
    return vm::vec3d{vm::get_abs_max_component_axis(m_camera->direction())};
    switchDefault();
  }
}

size_t MapView2D::flipAxis(const vm::direction direction) const
{
  switch (direction)
  {
  case vm::direction::forward:
  case vm::direction::backward:
    // These are not currently used, but it would be a "forward flip"
    return vm::find_abs_max_component(m_camera->direction());
  case vm::direction::left:
  case vm::direction::right:
    // Horizontal flip
    return vm::find_abs_max_component(m_camera->right());
  case vm::direction::up:
  case vm::direction::down:
    // Vertical flip. In 2D views, this corresponds to the vertical axis of the
    // viewport.
    return vm::find_abs_max_component(m_camera->up());
    switchDefault();
  }
}

vm::vec3d MapView2D::computePointEntityPosition(const vm::bbox3d& bounds) const
{
  using namespace mdl::HitFilters;

  const auto& map = m_document.map();

  const auto& grid = map.grid();
  const auto& worldBounds = map.worldBounds();

  const auto& hit = pickResult().first(type(mdl::BrushNode::BrushHitType) && selected());
  if (const auto faceHandle = mdl::hitToFaceHandle(hit))
  {
    const auto& face = faceHandle->face();
    return grid.moveDeltaForBounds(face.boundary(), bounds, worldBounds, pickRay());
  }
  else
  {
    const auto referenceBounds = map.referenceBounds();
    const auto& pickRay = MapView2D::pickRay();

    const auto toMin = referenceBounds.min - pickRay.origin;
    const auto toMax = referenceBounds.max - pickRay.origin;
    const auto anchor =
      vm::dot(toMin, pickRay.direction) > vm::dot(toMax, pickRay.direction)
        ? referenceBounds.min
        : referenceBounds.max;
    const auto dragPlane = vm::plane3d{anchor, -pickRay.direction};

    return grid.moveDeltaForBounds(dragPlane, bounds, worldBounds, pickRay);
  }
}

ActionContext::Type MapView2D::viewActionContext() const
{
  return ActionContext::View2D;
}

render::RenderMode MapView2D::renderMode()
{
  return render::RenderMode::Render2D;
}

void MapView2D::renderGrid(render::RenderContext&, render::RenderBatch& renderBatch)
{
  const auto& map = m_document.map();
  renderBatch.addOneShot(new render::GridRenderer(*m_camera, map.worldBounds()));
}

void MapView2D::renderMap(
  render::MapRenderer& renderer,
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch)
{
  renderer.render(renderContext, renderBatch);

  const auto& map = m_document.map();
  if (const auto& bounds = map.selectionBounds();
      bounds && renderContext.showSelectionGuide())
  {
    auto boundsRenderer = render::SelectionBoundsRenderer{*bounds};
    boundsRenderer.render(renderContext, renderBatch);
  }
}

void MapView2D::renderTools(
  MapViewToolBox& /* toolBox */,
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch)
{
  ToolBoxConnector::renderTools(renderContext, renderBatch);
}

void MapView2D::renderSoftWorldBounds(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  if (!renderContext.softMapBounds().is_empty())
  {
    auto renderService = render::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(pref(Preferences::SoftMapBoundsColor));
    renderService.renderBounds(renderContext.softMapBounds());
  }
}

void MapView2D::linkCamera(CameraLinkHelper& helper)
{
  helper.addCamera(m_camera.get());
}

} // namespace tb::ui
