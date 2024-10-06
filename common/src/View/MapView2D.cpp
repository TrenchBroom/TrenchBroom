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
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"
#include "Model/ModelUtils.h"
#include "Model/PickResult.h"
#include "Model/PointTrace.h"
#include "Renderer/Compass2D.h"
#include "Renderer/GridRenderer.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SelectionBoundsRenderer.h"
#include "View/CameraAnimation.h"
#include "View/CameraLinkHelper.h"
#include "View/CameraTool2D.h"
#include "View/ClipToolController.h"
#include "View/CreateEntityToolController.h"
#include "View/DrawShapeToolController2D.h"
#include "View/EdgeTool.h"
#include "View/EdgeToolController.h"
#include "View/ExtrudeToolController.h"
#include "View/FaceTool.h" // IWYU pragma: keep
#include "View/FaceToolController.h"
#include "View/GLContextManager.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapViewToolBox.h"
#include "View/MoveObjectsToolController.h"
#include "View/RotateObjectsToolController.h"
#include "View/ScaleObjectsToolController.h"
#include "View/SelectionTool.h"
#include "View/ShearObjectsToolController.h"
#include "View/VertexTool.h"
#include "View/VertexToolController.h"

#include "vm/util.h"

namespace tb::View
{

MapView2D::MapView2D(
  std::weak_ptr<MapDocument> document,
  MapViewToolBox& toolBox,
  Renderer::MapRenderer& renderer,
  GLContextManager& contextManager,
  ViewPlane viewPlane)
  : MapViewBase{std::move(document), toolBox, renderer, contextManager}
  , m_camera{std::make_unique<Renderer::OrthographicCamera>()}
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
  auto document = kdl::mem_lock(m_document);
  const auto worldBounds = vm::bbox3f(document->worldBounds());

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
  addTool(std::make_unique<CameraTool2D>(*m_camera));
  addTool(std::make_unique<MoveObjectsToolController>(toolBox.moveObjectsTool()));
  addTool(std::make_unique<RotateObjectsToolController2D>(toolBox.rotateObjectsTool()));
  addTool(std::make_unique<ScaleObjectsToolController2D>(
    toolBox.scaleObjectsTool(), m_document));
  addTool(std::make_unique<ShearObjectsToolController2D>(
    toolBox.shearObjectsTool(), m_document));
  addTool(std::make_unique<ExtrudeToolController2D>(toolBox.extrudeTool()));
  addTool(std::make_unique<ClipToolController2D>(toolBox.clipTool()));
  addTool(std::make_unique<VertexToolController>(toolBox.vertexTool()));
  addTool(std::make_unique<EdgeToolController>(toolBox.edgeTool()));
  addTool(std::make_unique<FaceToolController>(toolBox.faceTool()));
  addTool(std::make_unique<CreateEntityToolController2D>(toolBox.createEntityTool()));
  addTool(std::make_unique<SelectionTool>(m_document));
  addTool(
    std::make_unique<DrawShapeToolController2D>(toolBox.drawShapeTool(), m_document));
}

void MapView2D::connectObservers()
{
  m_notifierConnection +=
    m_camera->cameraDidChangeNotifier.connect(this, &MapView2D::cameraDidChange);
}

void MapView2D::cameraDidChange(const Renderer::Camera*)
{
  update();
}

PickRequest MapView2D::pickRequest(const float x, const float y) const
{
  return {vm::ray3d{m_camera->pickRay(x, y)}, *m_camera};
}

Model::PickResult MapView2D::pick(const vm::ray3d& pickRay) const
{
  auto document = kdl::mem_lock(m_document);
  const auto axis = vm::find_abs_max_component(pickRay.direction);

  auto pickResult = Model::PickResult::bySize(axis);
  document->pick(pickRay, pickResult);

  return pickResult;
}

void MapView2D::initializeGL()
{
  MapViewBase::initializeGL();
  setCompass(std::make_unique<Renderer::Compass2D>());
}

void MapView2D::updateViewport(
  const int x, const int y, const int width, const int height)
{
  m_camera->setViewport({x, y, width, height});
}

vm::vec3d MapView2D::pasteObjectsDelta(
  const vm::bbox3d& bounds, const vm::bbox3d& referenceBounds) const
{
  auto document = kdl::mem_lock(m_document);
  const auto& grid = document->grid();
  const auto& worldBounds = document->worldBounds();

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
  const auto document = kdl::mem_lock(m_document);
  const vm::axis::type cameraAxis = vm::find_abs_max_component(m_camera->direction());
  document->selectTall(cameraAxis);
}

void MapView2D::reset2dCameras(const Renderer::Camera& masterCamera, const bool animate)
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
  const auto document = kdl::mem_lock(m_document);
  const auto bounds = vm::bbox3f{document->referenceBounds()};
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
  auto document = kdl::mem_lock(m_document);
  assert(document->isPointFileLoaded());

  if (const auto* pointFile = document->pointFile())
  {
    moveCameraToPosition(pointFile->currentPoint(), true);
  }
}

Renderer::Camera& MapView2D::camera()
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
  using namespace Model::HitFilters;

  auto document = kdl::mem_lock(m_document);

  const auto& grid = document->grid();
  const auto& worldBounds = document->worldBounds();

  const auto& hit =
    pickResult().first(type(Model::BrushNode::BrushHitType) && selected());
  if (const auto faceHandle = Model::hitToFaceHandle(hit))
  {
    const auto& face = faceHandle->face();
    return grid.moveDeltaForBounds(face.boundary(), bounds, worldBounds, pickRay());
  }
  else
  {
    const auto referenceBounds = document->referenceBounds();
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

Renderer::RenderMode MapView2D::renderMode()
{
  return Renderer::RenderMode::Render2D;
}

void MapView2D::renderGrid(Renderer::RenderContext&, Renderer::RenderBatch& renderBatch)
{
  auto document = kdl::mem_lock(m_document);
  renderBatch.addOneShot(new Renderer::GridRenderer(*m_camera, document->worldBounds()));
}

void MapView2D::renderMap(
  Renderer::MapRenderer& renderer,
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch)
{
  renderer.render(renderContext, renderBatch);

  auto document = kdl::mem_lock(m_document);
  if (renderContext.showSelectionGuide() && document->hasSelectedNodes())
  {
    auto boundsRenderer = Renderer::SelectionBoundsRenderer{document->selectionBounds()};
    boundsRenderer.render(renderContext, renderBatch);
  }
}

void MapView2D::renderTools(
  MapViewToolBox& /* toolBox */,
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch)
{
  ToolBoxConnector::renderTools(renderContext, renderBatch);
}

void MapView2D::renderSoftWorldBounds(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch)
{
  if (!renderContext.softMapBounds().is_empty())
  {
    auto document = kdl::mem_lock(m_document);

    auto renderService = Renderer::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(pref(Preferences::SoftMapBoundsColor));
    renderService.renderBounds(renderContext.softMapBounds());
  }
}

void MapView2D::linkCamera(CameraLinkHelper& helper)
{
  helper.addCamera(m_camera.get());
}

} // namespace tb::View
