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

#include "MapView2D.h"

#include "Assets/EntityDefinitionManager.h"
#include "Exceptions.h"
#include "Logger.h"
#include "Macros.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushError.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/Hit.h"
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
#include "View/CreateSimpleBrushToolController2D.h"
#include "View/EdgeTool.h"
#include "View/EdgeToolController.h"
#include "View/FaceTool.h"
#include "View/FaceToolController.h"
#include "View/GLContextManager.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/MapViewToolBox.h"
#include "View/MoveObjectsToolController.h"
#include "View/ResizeBrushesToolController.h"
#include "View/RotateObjectsToolController.h"
#include "View/ScaleObjectsToolController.h"
#include "View/SelectionTool.h"
#include "View/ShearObjectsToolController.h"
#include "View/VertexTool.h"
#include "View/VertexToolController.h"

#include <kdl/overload.h>
#include <kdl/result.h>

#include <vecmath/util.h>

#include <memory>
#include <vector>

namespace TrenchBroom {
namespace View {
MapView2D::MapView2D(
  std::weak_ptr<MapDocument> document, MapViewToolBox& toolBox, Renderer::MapRenderer& renderer,
  GLContextManager& contextManager, ViewPlane viewPlane, Logger* logger)
  : MapViewBase(logger, document, toolBox, renderer, contextManager)
  , m_camera(std::make_unique<Renderer::OrthographicCamera>()) {
  connectObservers();
  initializeCamera(viewPlane);
  initializeToolChain(toolBox);

  switch (viewPlane) {
    case ViewPlane_XY:
      setObjectName("XY View");
      break;
    case ViewPlane_YZ:
      setObjectName("YZ View");
      break;
    case ViewPlane_XZ:
      setObjectName("XZ View");
      break;
      switchDefault()
  }

  mapViewBaseVirtualInit();
}

void MapView2D::initializeCamera(const ViewPlane viewPlane) {
  auto document = kdl::mem_lock(m_document);
  const auto worldBounds = vm::bbox3f(document->worldBounds());

  switch (viewPlane) {
    case MapView2D::ViewPlane_XY:
      m_camera->setDirection(vm::vec3f::neg_z(), vm::vec3f::pos_y());
      m_camera->moveTo(vm::vec3f(0.0f, 0.0f, worldBounds.max.z()));
      break;
    case MapView2D::ViewPlane_XZ:
      m_camera->setDirection(vm::vec3f::pos_y(), vm::vec3f::pos_z());
      m_camera->moveTo(vm::vec3f(0.0f, worldBounds.min.y(), 0.0f));
      break;
    case MapView2D::ViewPlane_YZ:
      m_camera->setDirection(vm::vec3f::neg_x(), vm::vec3f::pos_z());
      m_camera->moveTo(vm::vec3f(worldBounds.max.x(), 0.0f, 0.0f));
      break;
  }
  m_camera->setNearPlane(1.0f);
  // NOTE: GridRenderer draws at the far side of the map bounds, so add some extra margin so it's
  // not fighting the far plane.
  m_camera->setFarPlane(worldBounds.size().x() + 16.0f);
}

void MapView2D::initializeToolChain(MapViewToolBox& toolBox) {
  addTool(std::make_unique<CameraTool2D>(*m_camera));
  addTool(std::make_unique<MoveObjectsToolController>(toolBox.moveObjectsTool()));
  addTool(std::make_unique<RotateObjectsToolController2D>(toolBox.rotateObjectsTool()));
  addTool(std::make_unique<ScaleObjectsToolController2D>(toolBox.scaleObjectsTool(), m_document));
  addTool(std::make_unique<ShearObjectsToolController2D>(toolBox.shearObjectsTool(), m_document));
  addTool(std::make_unique<ResizeBrushesToolController2D>(toolBox.resizeBrushesTool()));
  addTool(std::make_unique<ClipToolController2D>(toolBox.clipTool()));
  addTool(std::make_unique<VertexToolController>(toolBox.vertexTool()));
  addTool(std::make_unique<EdgeToolController>(toolBox.edgeTool()));
  addTool(std::make_unique<FaceToolController>(toolBox.faceTool()));
  addTool(std::make_unique<CreateEntityToolController2D>(toolBox.createEntityTool()));
  addTool(std::make_unique<SelectionTool>(m_document));
  addTool(std::make_unique<CreateSimpleBrushToolController2D>(
    toolBox.createSimpleBrushTool(), m_document));
}

void MapView2D::connectObservers() {
  m_notifierConnection +=
    m_camera->cameraDidChangeNotifier.connect(this, &MapView2D::cameraDidChange);
}

void MapView2D::cameraDidChange(const Renderer::Camera*) {
  update();
}

PickRequest MapView2D::doGetPickRequest(const float x, const float y) const {
  return PickRequest(vm::ray3(m_camera->pickRay(x, y)), *m_camera);
}

Model::PickResult MapView2D::doPick(const vm::ray3& pickRay) const {
  auto document = kdl::mem_lock(m_document);
  const auto axis = vm::find_abs_max_component(pickRay.direction);

  auto pickResult = Model::PickResult::bySize(axis);
  document->pick(pickRay, pickResult);

  return pickResult;
}

void MapView2D::initializeGL() {
  MapViewBase::initializeGL();
  setCompass(std::make_unique<Renderer::Compass2D>());
}

void MapView2D::doUpdateViewport(const int x, const int y, const int width, const int height) {
  m_camera->setViewport(Renderer::Camera::Viewport(x, y, width, height));
}

vm::vec3 MapView2D::doGetPasteObjectsDelta(
  const vm::bbox3& bounds, const vm::bbox3& referenceBounds) const {
  auto document = kdl::mem_lock(m_document);
  const auto& grid = document->grid();
  const auto& worldBounds = document->worldBounds();

  const auto& pickRay = MapView2D::pickRay();

  const auto toMin = referenceBounds.min - pickRay.origin;
  const auto toMax = referenceBounds.max - pickRay.origin;
  const auto anchor = dot(toMin, pickRay.direction) > dot(toMax, pickRay.direction)
                        ? referenceBounds.min
                        : referenceBounds.max;
  const auto dragPlane = vm::plane3(anchor, -pickRay.direction);

  const auto distance = vm::intersect_ray_plane(pickRay, dragPlane);
  if (vm::is_nan(distance)) {
    return vm::vec3::zero();
  } else {
    return grid.moveDeltaForBounds(dragPlane, bounds, worldBounds, pickRay);
  }
}

bool MapView2D::doCanSelectTall() {
  return true;
}

void MapView2D::doSelectTall() {
  const auto document = kdl::mem_lock(m_document);
  const vm::axis::type cameraAxis = vm::find_abs_max_component(m_camera->direction());
  document->selectTall(cameraAxis);
}

void MapView2D::doFocusCameraOnSelection(const bool animate) {
  const auto document = kdl::mem_lock(m_document);
  const auto& bounds = document->referenceBounds();
  const auto diff = bounds.center() - vm::vec3(m_camera->position());
  const auto delta = diff * vm::vec3(m_camera->up() + m_camera->right());
  moveCameraToPosition(vm::vec3(m_camera->position()) + delta, animate);
}

void MapView2D::doMoveCameraToPosition(const vm::vec3& position, const bool animate) {
  if (animate) {
    animateCamera(vm::vec3f(position), m_camera->direction(), m_camera->up());
  } else {
    m_camera->moveTo(vm::vec3f(position));
  }
}

void MapView2D::animateCamera(
  const vm::vec3f& position, const vm::vec3f& /* direction */, const vm::vec3f& /* up */,
  const int duration) {
  const auto actualPosition =
    dot(position, m_camera->up()) * m_camera->up() +
    dot(position, m_camera->right()) * m_camera->right() +
    dot(m_camera->position(), m_camera->direction()) * m_camera->direction();
  auto animation = std::make_unique<CameraAnimation>(
    *m_camera, actualPosition, m_camera->direction(), m_camera->up(), duration);
  m_animationManager->runAnimation(std::move(animation), true);
}

void MapView2D::doMoveCameraToCurrentTracePoint() {
  auto document = kdl::mem_lock(m_document);
  assert(document->isPointFileLoaded());

  if (const auto& pointFile = document->pointFile()) {
    moveCameraToPosition(vm::vec3{pointFile->trace.currentPoint()}, true);
  }
}

vm::vec3 MapView2D::doGetMoveDirection(const vm::direction direction) const {
  // The mapping is a bit counter intuitive, but it makes sense considering that the cursor up key
  // is usually bounds to the forward action (which makes sense in 3D), but should move objects "up"
  // in 2D.
  switch (direction) {
    case vm::direction::forward:
      return vm::vec3(vm::get_abs_max_component_axis(m_camera->up()));
    case vm::direction::backward:
      return vm::vec3(-vm::get_abs_max_component_axis(m_camera->up()));
    case vm::direction::left:
      return vm::vec3(-vm::get_abs_max_component_axis(m_camera->right()));
    case vm::direction::right:
      return vm::vec3(vm::get_abs_max_component_axis(m_camera->right()));
    case vm::direction::up:
      return vm::vec3(-vm::get_abs_max_component_axis(m_camera->direction()));
    case vm::direction::down:
      return vm::vec3(vm::get_abs_max_component_axis(m_camera->direction()));
      switchDefault()
  }
}

size_t MapView2D::doGetFlipAxis(const vm::direction direction) const {
  switch (direction) {
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
      // Vertical flip. In 2D views, this corresponds to the vertical axis of the viewport.
      return vm::find_abs_max_component(m_camera->up());
      switchDefault()
  }
}

vm::vec3 MapView2D::doComputePointEntityPosition(const vm::bbox3& bounds) const {
  auto document = kdl::mem_lock(m_document);

  const auto& grid = document->grid();
  const auto& worldBounds = document->worldBounds();

  using namespace Model::HitFilters;
  const auto& hit = pickResult().first(type(Model::BrushNode::BrushHitType) && selected());
  if (const auto faceHandle = Model::hitToFaceHandle(hit)) {
    const auto& face = faceHandle->face();
    return grid.moveDeltaForBounds(face.boundary(), bounds, worldBounds, pickRay());
  } else {
    const auto referenceBounds = document->referenceBounds();
    const auto& pickRay = MapView2D::pickRay();

    const auto toMin = referenceBounds.min - pickRay.origin;
    const auto toMax = referenceBounds.max - pickRay.origin;
    const auto anchor = dot(toMin, pickRay.direction) > dot(toMax, pickRay.direction)
                          ? referenceBounds.min
                          : referenceBounds.max;
    const auto dragPlane = vm::plane3(anchor, -pickRay.direction);

    const auto distance = vm::intersect_ray_plane(pickRay, dragPlane);
    if (vm::is_nan(distance)) {
      return vm::vec3::zero();
    } else {
      return grid.moveDeltaForBounds(dragPlane, bounds, worldBounds, pickRay);
    }
  }
}

ActionContext::Type MapView2D::doGetActionContext() const {
  return ActionContext::View2D;
}

ActionView MapView2D::doGetActionView() const {
  return ActionView_Map2D;
}

bool MapView2D::doCancel() {
  return false;
}

Renderer::RenderMode MapView2D::doGetRenderMode() {
  return Renderer::RenderMode::Render2D;
}

Renderer::Camera& MapView2D::doGetCamera() {
  return *m_camera;
}

void MapView2D::doRenderGrid(Renderer::RenderContext&, Renderer::RenderBatch& renderBatch) {
  auto document = kdl::mem_lock(m_document);
  renderBatch.addOneShot(new Renderer::GridRenderer(*m_camera, document->worldBounds()));
}

void MapView2D::doRenderMap(
  Renderer::MapRenderer& renderer, Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch) {
  renderer.render(renderContext, renderBatch);

  auto document = kdl::mem_lock(m_document);
  if (renderContext.showSelectionGuide() && document->hasSelectedNodes()) {
    const vm::bbox3& bounds = document->selectionBounds();
    Renderer::SelectionBoundsRenderer boundsRenderer(bounds);
    boundsRenderer.render(renderContext, renderBatch);
  }
}

void MapView2D::doRenderTools(
  MapViewToolBox& /* toolBox */, Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch) {
  renderTools(renderContext, renderBatch);
}

void MapView2D::doRenderExtras(Renderer::RenderContext&, Renderer::RenderBatch&) {}

void MapView2D::doRenderSoftWorldBounds(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
  if (!renderContext.softMapBounds().is_empty()) {
    auto document = kdl::mem_lock(m_document);

    Renderer::RenderService renderService(renderContext, renderBatch);
    renderService.setForegroundColor(pref(Preferences::SoftMapBoundsColor));
    renderService.renderBounds(renderContext.softMapBounds());
  }
}

void MapView2D::doLinkCamera(CameraLinkHelper& helper) {
  helper.addCamera(m_camera.get());
}
} // namespace View
} // namespace TrenchBroom
