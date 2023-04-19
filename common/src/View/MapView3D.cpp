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

#include "MapView3D.h"

#include "Assets/EntityDefinitionManager.h"
#include "FloatType.h"
#include "Logger.h"
#include "Model/BezierPatch.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/PickResult.h"
#include "Model/PointTrace.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/BoundsGuideRenderer.h"
#include "Renderer/Compass3D.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/PerspectiveCamera.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/SelectionBoundsRenderer.h"
#include "View/CameraAnimation.h"
#include "View/CameraTool3D.h"
#include "View/ClipToolController.h"
#include "View/CreateComplexBrushToolController3D.h"
#include "View/CreateEntityToolController.h"
#include "View/CreatePrimitiveBrushToolController3D.h"
#include "View/CreateSimpleBrushToolController3D.h"
#include "View/EdgeTool.h"
#include "View/EdgeToolController.h"
#include "View/ExtrudeToolController.h"
#include "View/FaceTool.h"
#include "View/FaceToolController.h"
#include "View/FlyModeHelper.h"
#include "View/GLContextManager.h"
#include "View/Grid.h"
#include "View/MapViewToolBox.h"
#include "View/MoveObjectsToolController.h"
#include "View/QtUtils.h"
#include "View/RotateObjectsToolController.h"
#include "View/ScaleObjectsToolController.h"
#include "View/SelectionTool.h"
#include "View/SetBrushFaceAttributesTool.h"
#include "View/ShearObjectsToolController.h"
#include "View/VertexTool.h"
#include "View/VertexToolController.h"

#include <kdl/set_temp.h>

#include <vecmath/util.h>

#include <memory>

namespace TrenchBroom
{
namespace View
{
MapView3D::MapView3D(
  std::weak_ptr<MapDocument> document,
  MapViewToolBox& toolBox,
  Renderer::MapRenderer& renderer,
  GLContextManager& contextManager,
  Logger* logger)
  : MapViewBase(logger, std::move(document), toolBox, renderer, contextManager)
  , m_camera(std::make_unique<Renderer::PerspectiveCamera>())
  , m_flyModeHelper(std::make_unique<FlyModeHelper>(*m_camera))
  , m_ignoreCameraChangeEvents(false)
{
  bindEvents();
  connectObservers();
  initializeCamera();
  initializeToolChain(toolBox);

  m_camera->setFov(pref(Preferences::CameraFov));

  mapViewBaseVirtualInit();
}

MapView3D::~MapView3D() = default;

void MapView3D::initializeCamera()
{
  m_camera->moveTo(vm::vec3f(-80.0f, -128.0f, 96.0f));
  m_camera->lookAt(vm::vec3f::zero(), vm::vec3f::pos_z());
}

void MapView3D::initializeToolChain(MapViewToolBox& toolBox)
{
  addTool(std::make_unique<CameraTool3D>(*m_camera));
  addTool(std::make_unique<MoveObjectsToolController>(toolBox.moveObjectsTool()));
  addTool(std::make_unique<RotateObjectsToolController3D>(toolBox.rotateObjectsTool()));
  addTool(std::make_unique<ScaleObjectsToolController3D>(
    toolBox.scaleObjectsTool(), m_document));
  addTool(std::make_unique<ShearObjectsToolController3D>(
    toolBox.shearObjectsTool(), m_document));
  addTool(std::make_unique<ExtrudeToolController3D>(toolBox.extrudeTool()));
  addTool(std::make_unique<CreateComplexBrushToolController3D>(
    toolBox.createComplexBrushTool()));
  addTool(std::make_unique<CreatePrimitiveBrushToolController3D>(
    toolBox.createPrimitiveBrushTool(), m_document));
  addTool(std::make_unique<ClipToolController3D>(toolBox.clipTool()));
  addTool(std::make_unique<VertexToolController>(toolBox.vertexTool()));
  addTool(std::make_unique<EdgeToolController>(toolBox.edgeTool()));
  addTool(std::make_unique<FaceToolController>(toolBox.faceTool()));
  addTool(std::make_unique<CreateEntityToolController3D>(toolBox.createEntityTool()));
  addTool(std::make_unique<SetBrushFaceAttributesTool>(m_document));
  addTool(std::make_unique<SelectionTool>(m_document));
  addTool(std::make_unique<CreateSimpleBrushToolController3D>(
    toolBox.createSimpleBrushTool(), m_document));
}

void MapView3D::connectObservers()
{
  m_notifierConnection +=
    m_camera->cameraDidChangeNotifier.connect(this, &MapView3D::cameraDidChange);

  PreferenceManager& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &MapView3D::preferenceDidChange);
}

void MapView3D::cameraDidChange(const Renderer::Camera* /* camera */)
{
  if (!m_ignoreCameraChangeEvents)
  {
    // Don't refresh if the camera was changed in doPreRender!
    update();
  }
}

void MapView3D::preferenceDidChange(const IO::Path& path)
{
  if (path == Preferences::CameraFov.path())
  {
    m_camera->setFov(pref(Preferences::CameraFov));
    update();
  }
}

void MapView3D::keyPressEvent(QKeyEvent* event)
{
  m_flyModeHelper->keyDown(event);

  MapViewBase::keyPressEvent(event);
}

void MapView3D::keyReleaseEvent(QKeyEvent* event)
{
  m_flyModeHelper->keyUp(event);

  MapViewBase::keyReleaseEvent(event);
}

void MapView3D::focusInEvent(QFocusEvent* event)
{
  m_flyModeHelper->resetKeys();

  MapViewBase::focusInEvent(event);
}

void MapView3D::focusOutEvent(QFocusEvent* event)
{
  m_flyModeHelper->resetKeys();

  MapViewBase::focusOutEvent(event);
}

void MapView3D::initializeGL()
{
  MapViewBase::initializeGL();
  setCompass(std::make_unique<Renderer::Compass3D>());
}

void MapView3D::bindEvents()
{
  // Fly mode animation
  connect(this, &QOpenGLWidget::frameSwapped, this, &MapView3D::updateFlyMode);
}

void MapView3D::updateFlyMode()
{
  if (m_flyModeHelper->anyKeyDown())
  {
    update();
  }
}

void MapView3D::resetFlyModeKeys()
{
  m_flyModeHelper->resetKeys();
}

PickRequest MapView3D::doGetPickRequest(const float x, const float y) const
{
  return PickRequest(vm::ray3(m_camera->pickRay(x, y)), *m_camera);
}

Model::PickResult MapView3D::doPick(const vm::ray3& pickRay) const
{
  auto document = kdl::mem_lock(m_document);
  Model::PickResult pickResult = Model::PickResult::byDistance();

  document->pick(pickRay, pickResult);
  return pickResult;
}

void MapView3D::doUpdateViewport(
  const int x, const int y, const int width, const int height)
{
  m_camera->setViewport(Renderer::Camera::Viewport(x, y, width, height));
}

vm::vec3 MapView3D::doGetPasteObjectsDelta(
  const vm::bbox3& bounds, const vm::bbox3& /* referenceBounds */) const
{
  auto document = kdl::mem_lock(m_document);
  const auto& grid = document->grid();

  const QPoint pos = QCursor::pos();
  const auto clientCoords = mapFromGlobal(pos);

  if (QRect(0, 0, width(), height()).contains(clientCoords))
  {
    const auto pickRay = vm::ray3(m_camera->pickRay(
      static_cast<float>(clientCoords.x()), static_cast<float>(clientCoords.y())));
    auto pickResult = Model::PickResult::byDistance();

    document->pick(pickRay, pickResult);

    using namespace Model::HitFilters;
    const auto& hit = pickResult.first(type(Model::BrushNode::BrushHitType));
    if (const auto faceHandle = Model::hitToFaceHandle(hit))
    {
      const auto& face = faceHandle->face();
      return grid.moveDeltaForBounds(
        face.boundary(), bounds, document->worldBounds(), pickRay);
    }
    else
    {
      const auto point = vm::vec3(grid.snap(m_camera->defaultPoint(pickRay)));
      const auto targetPlane = vm::plane3(point, -vm::vec3(m_camera->direction()));
      return grid.moveDeltaForBounds(
        targetPlane, bounds, document->worldBounds(), pickRay);
    }
  }
  else
  {
    const auto oldMin = bounds.min;
    const auto oldCenter = bounds.center();
    const auto newCenter = vm::vec3(m_camera->defaultPoint());
    const auto newMin = oldMin + (newCenter - oldCenter);
    return grid.snap(newMin);
  }
}

bool MapView3D::doCanSelectTall()
{
  return false;
}

void MapView3D::doSelectTall() {}

void MapView3D::doFocusCameraOnSelection(const bool animate)
{
  auto document = kdl::mem_lock(m_document);
  const auto& nodes = document->selectedNodes().nodes();
  if (!nodes.empty())
  {
    const auto newPosition = focusCameraOnObjectsPosition(nodes);
    moveCameraToPosition(newPosition, animate);
  }
}

static vm::vec3 computeCameraTargetPosition(const std::vector<Model::Node*>& nodes)
{
  auto center = vm::vec3();
  size_t count = 0u;

  const auto handlePoint = [&](const vm::vec3& point) {
    center = center + point;
    ++count;
  };

  Model::Node::visitAll(
    nodes,
    kdl::overload(
      [](
        auto&& thisLambda, Model::WorldNode* world) { world->visitChildren(thisLambda); },
      [](
        auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
      [](
        auto&& thisLambda, Model::GroupNode* group) { group->visitChildren(thisLambda); },
      [&](auto&& thisLambda, Model::EntityNode* entity) {
        if (!entity->hasChildren())
        {
          entity->logicalBounds().for_each_vertex(handlePoint);
        }
        else
        {
          entity->visitChildren(thisLambda);
        }
      },
      [&](Model::BrushNode* brush) {
        for (const auto* vertex : brush->brush().vertices())
        {
          handlePoint(vertex->position());
        }
      },
      [&](Model::PatchNode* patchNode) {
        for (const auto& controlPoint : patchNode->patch().controlPoints())
        {
          handlePoint(controlPoint.xyz());
        }
      }));

  return center / static_cast<FloatType>(count);
}

static FloatType computeCameraOffset(
  const Renderer::Camera& camera, const std::vector<Model::Node*>& nodes)
{
  vm::plane3f frustumPlanes[4];
  camera.frustumPlanes(
    frustumPlanes[0], frustumPlanes[1], frustumPlanes[2], frustumPlanes[3]);

  float offset = std::numeric_limits<float>::min();
  const auto handlePoint = [&](const vm::vec3& point, const vm::plane3f& plane) {
    const auto ray = vm::ray3f(camera.position(), -camera.direction());
    const auto newPlane =
      vm::plane3f(vm::vec3f(point) + 64.0f * plane.normal, plane.normal);
    const auto dist = vm::intersect_ray_plane(ray, newPlane);
    if (!vm::is_nan(dist) && dist > 0.0f)
    {
      offset = std::max(offset, dist);
    }
  };

  Model::Node::visitAll(
    nodes,
    kdl::overload(
      [](
        auto&& thisLambda, Model::WorldNode* world) { world->visitChildren(thisLambda); },
      [](
        auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
      [](
        auto&& thisLambda, Model::GroupNode* group) { group->visitChildren(thisLambda); },
      [&](auto&& thisLambda, Model::EntityNode* entity) {
        if (!entity->hasChildren())
        {
          for (size_t i = 0u; i < 4u; ++i)
          {
            entity->logicalBounds().for_each_vertex(
              [&](const auto& point) { handlePoint(point, frustumPlanes[i]); });
          }
        }
        else
        {
          entity->visitChildren(thisLambda);
        }
      },
      [&](Model::BrushNode* brush) {
        for (const auto* vertex : brush->brush().vertices())
        {
          for (size_t i = 0u; i < 4u; ++i)
          {
            handlePoint(vertex->position(), frustumPlanes[i]);
          }
        }
      },
      [&](Model::PatchNode* patchNode) {
        for (const auto& controlPoint : patchNode->patch().controlPoints())
        {
          for (size_t i = 0u; i < 4u; ++i)
          {
            handlePoint(controlPoint.xyz(), frustumPlanes[i]);
          }
        }
      }));

  return static_cast<FloatType>(offset);
}

vm::vec3 MapView3D::focusCameraOnObjectsPosition(const std::vector<Model::Node*>& nodes)
{
  const auto newPosition = computeCameraTargetPosition(nodes);

  // act as if the camera were there already:
  const auto oldPosition = m_camera->position();
  m_camera->moveTo(vm::vec3f(newPosition));

  const auto offset = computeCameraOffset(*m_camera, nodes);

  // jump back
  m_camera->moveTo(oldPosition);
  return newPosition - vm::vec3(m_camera->direction()) * offset;
}

void MapView3D::doMoveCameraToPosition(const vm::vec3& position, const bool animate)
{
  if (animate)
  {
    animateCamera(vm::vec3f(position), m_camera->direction(), m_camera->up());
  }
  else
  {
    m_camera->moveTo(vm::vec3f(position));
  }
}

void MapView3D::animateCamera(
  const vm::vec3f& position,
  const vm::vec3f& direction,
  const vm::vec3f& up,
  const int duration)
{
  auto animation =
    std::make_unique<CameraAnimation>(*m_camera, position, direction, up, duration);
  m_animationManager->runAnimation(std::move(animation), true);
}

void MapView3D::doMoveCameraToCurrentTracePoint()
{
  auto document = kdl::mem_lock(m_document);
  assert(document->isPointFileLoaded());

  if (const auto& pointFile = document->pointFile())
  {
    const auto position = pointFile->trace.currentPoint() + vm::vec3f{0.0f, 0.0f, 16.0f};
    const auto direction = pointFile->trace.currentDirection();
    animateCamera(position, direction, vm::vec3f::pos_z());
  }
}

vm::vec3 MapView3D::doGetMoveDirection(const vm::direction direction) const
{
  switch (direction)
  {
  case vm::direction::forward: {
    const auto plane = vm::plane3(vm::vec3(m_camera->position()), vm::vec3::pos_z());
    const auto projectedDirection = plane.project_vector(vm::vec3(m_camera->direction()));
    if (vm::is_zero(projectedDirection, vm::C::almost_zero()))
    {
      // camera is looking straight down or up
      if (m_camera->direction().z() < 0.0f)
      {
        return vm::vec3(vm::get_abs_max_component_axis(m_camera->up()));
      }
      else
      {
        return vm::vec3(-vm::get_abs_max_component_axis(m_camera->up()));
      }
    }
    return vm::get_abs_max_component_axis(projectedDirection);
  }
  case vm::direction::backward:
    return -doGetMoveDirection(vm::direction::forward);
  case vm::direction::left:
    return -doGetMoveDirection(vm::direction::right);
  case vm::direction::right: {
    auto dir = vm::vec3(vm::get_abs_max_component_axis(m_camera->right()));
    if (dir == doGetMoveDirection(vm::direction::forward))
    {
      dir = cross(dir, vm::vec3::pos_z());
    }
    return dir;
  }
  case vm::direction::up:
    return vm::vec3::pos_z();
  case vm::direction::down:
    return vm::vec3::neg_z();
    switchDefault();
  }
}

size_t MapView3D::doGetFlipAxis(const vm::direction direction) const
{
  return vm::find_abs_max_component(doGetMoveDirection(direction));
}

vm::vec3 MapView3D::doComputePointEntityPosition(const vm::bbox3& bounds) const
{
  auto document = kdl::mem_lock(m_document);

  auto& grid = document->grid();
  const auto& worldBounds = document->worldBounds();

  using namespace Model::HitFilters;
  const auto& hit = pickResult().first(type(Model::BrushNode::BrushHitType));
  if (const auto faceHandle = Model::hitToFaceHandle(hit))
  {
    const auto& face = faceHandle->face();
    return grid.moveDeltaForBounds(face.boundary(), bounds, worldBounds, pickRay());
  }
  else
  {
    const auto newPosition = Renderer::Camera::defaultPoint(pickRay());
    const auto defCenter = bounds.center();
    return grid.moveDeltaForPoint(defCenter, newPosition - defCenter);
  }
}

ActionContext::Type MapView3D::doGetActionContext() const
{
  return ActionContext::View3D;
}

ActionView MapView3D::doGetActionView() const
{
  return ActionView_Map3D;
}

bool MapView3D::doCancel()
{
  return false;
}

Renderer::RenderMode MapView3D::doGetRenderMode()
{
  return Renderer::RenderMode::Render3D;
}

Renderer::Camera& MapView3D::doGetCamera()
{
  return *m_camera;
}

void MapView3D::doPreRender()
{
  const kdl::set_temp ignoreCameraUpdates(m_ignoreCameraChangeEvents);
  m_flyModeHelper->pollAndUpdate();
}

void MapView3D::doRenderGrid(Renderer::RenderContext&, Renderer::RenderBatch&) {}

void MapView3D::doRenderMap(
  Renderer::MapRenderer& renderer,
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch)
{
  renderer.render(renderContext, renderBatch);

  auto document = kdl::mem_lock(m_document);
  if (renderContext.showSelectionGuide() && document->hasSelectedNodes())
  {
    const vm::bbox3& bounds = document->selectionBounds();
    Renderer::SelectionBoundsRenderer boundsRenderer(bounds);
    boundsRenderer.render(renderContext, renderBatch);

    Renderer::BoundsGuideRenderer* guideRenderer =
      new Renderer::BoundsGuideRenderer(m_document);
    guideRenderer->setColor(pref(Preferences::SelectionBoundsColor));
    guideRenderer->setBounds(bounds);
    renderBatch.addOneShot(guideRenderer);
  }
}

void MapView3D::doRenderTools(
  MapViewToolBox& /* toolBox */,
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch)
{
  renderTools(renderContext, renderBatch);
}

void MapView3D::doRenderSoftWorldBounds(Renderer::RenderContext&, Renderer::RenderBatch&)
{
  // the bounds rect itself is only rendered in MapView2D, it just clutters the 3D view
}

bool MapView3D::doBeforePopupMenu()
{
  m_flyModeHelper->resetKeys();
  return true;
}

void MapView3D::doLinkCamera(CameraLinkHelper& /* helper */) {}
} // namespace View
} // namespace TrenchBroom
