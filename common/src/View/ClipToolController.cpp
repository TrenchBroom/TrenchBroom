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

#include "ClipToolController.h"

#include "Ensure.h"
#include "View/ClipTool.h"
#include "View/Grid.h"
#include "View/HandleDragTracker.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushGeometry.h"
#include "mdl/BrushNode.h"
#include "mdl/Hit.h"
#include "mdl/HitAdapter.h"
#include "mdl/HitFilter.h"
#include "mdl/PickResult.h"
#include "mdl/Polyhedron.h"
#include "render/Camera.h"
#include "render/RenderContext.h"

#include "kdl/optional_utils.h"
#include "kdl/vector_utils.h"

#include "vm/distance.h"
#include "vm/intersection.h"
#include "vm/vec.h"

#include <memory>
#include <optional>

namespace tb::View
{
namespace
{

class PartDelegateBase
{
protected:
  ClipTool& m_tool;

public:
  explicit PartDelegateBase(ClipTool& tool)
    : m_tool{tool}
  {
  }

  virtual ~PartDelegateBase() = default;

  ClipTool& tool() const { return m_tool; }

  std::optional<std::tuple<vm::vec3d, vm::vec3d>> addClipPoint(
    const InputState& inputState)
  {
    const auto positionAndHitPoint = doGetNewClipPointPositionAndHitPoint(inputState);
    if (!positionAndHitPoint)
    {
      return std::nullopt;
    }

    const auto position = std::get<0>(*positionAndHitPoint);
    if (!m_tool.canAddPoint(position))
    {
      return std::nullopt;
    }

    m_tool.addPoint(position, getHelpVectors(inputState, position));
    return positionAndHitPoint;
  }

  bool setClipFace(const InputState& inputState)
  {
    using namespace mdl::HitFilters;
    const auto& hit = inputState.pickResult().first(type(mdl::BrushNode::BrushHitType));
    if (const auto faceHandle = mdl::hitToFaceHandle(hit))
    {
      m_tool.setFace(*faceHandle);
      return true;
    }
    else
    {
      return false;
    }
  }

  virtual HandlePositionProposer makeHandlePositionProposer(
    const InputState& inputState,
    const vm::vec3d& initialHandlePosition,
    const vm::vec3d& handleOffset) const = 0;
  virtual std::vector<vm::vec3d> getHelpVectors(
    const InputState& inputState, const vm::vec3d& clipPoint) const = 0;

  void renderFeedback(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch)
  {
    if (inputState.anyToolDragging())
    {
      return;
    }

    const auto positionAndHitPoint = doGetNewClipPointPositionAndHitPoint(inputState);
    if (!positionAndHitPoint)
    {
      return;
    }

    const auto position = std::get<0>(*positionAndHitPoint);
    if (m_tool.canAddPoint(position))
    {
      m_tool.renderFeedback(renderContext, renderBatch, position);
    }
  }

private:
  virtual std::optional<std::tuple<vm::vec3d, vm::vec3d>>
  doGetNewClipPointPositionAndHitPoint(const InputState& inputState) const = 0;
};

class PartDelegate2D : public PartDelegateBase
{
public:
  explicit PartDelegate2D(ClipTool& tool)
    : PartDelegateBase{tool}
  {
  }

  HandlePositionProposer makeHandlePositionProposer(
    const InputState& inputState,
    const vm::vec3d& initialHandlePosition,
    const vm::vec3d& handleOffset) const override
  {
    return View::makeHandlePositionProposer(
      makePlaneHandlePicker(
        vm::plane3d{initialHandlePosition, vm::vec3d{inputState.camera().direction()}},
        handleOffset),
      makeAbsoluteHandleSnapper(m_tool.grid()));
  }

  std::vector<vm::vec3d> getHelpVectors(
    const InputState& inputState, const vm::vec3d& /* clipPoint */) const override
  {
    return std::vector{vm::vec3d{inputState.camera().direction()}};
  }

  std::optional<std::tuple<vm::vec3d, vm::vec3d>> doGetNewClipPointPositionAndHitPoint(
    const InputState& inputState) const override
  {
    const auto& camera = inputState.camera();
    const auto viewDir = vm::get_abs_max_component_axis(vm::vec3d(camera.direction()));

    const auto& pickRay = inputState.pickRay();
    const auto defaultPos = m_tool.defaultClipPointPos();
    return kdl::optional_transform(
      vm::intersect_ray_plane(pickRay, vm::plane3d{defaultPos, viewDir}),
      [&](const auto distance) {
        const auto hitPoint = vm::point_at_distance(pickRay, distance);
        const auto position = m_tool.grid().snap(hitPoint);
        return std::tuple{position, hitPoint};
      });
  }
};

std::vector<const mdl::BrushFace*> selectIncidentFaces(
  const mdl::BrushNode* brushNode, const mdl::BrushFace& face, const vm::vec3d& hitPoint)
{
  static const auto MaxDistance = vm::constants<double>::almost_zero();

  // First, try to see if the clip point is almost equal to a vertex:
  double closestVertexDistance = MaxDistance;
  const mdl::BrushVertex* closestVertex = nullptr;
  for (const auto* vertex : face.vertices())
  {
    const auto distance = vm::distance(vertex->position(), hitPoint);
    if (distance < closestVertexDistance)
    {
      closestVertex = vertex;
      closestVertexDistance = distance;
    }
  }

  if (closestVertex != nullptr)
  {
    const mdl::Brush& brush = brushNode->brush();
    return brush.incidentFaces(closestVertex);
  }

  // Next, try the edges:
  double closestEdgeDistance = MaxDistance;
  const mdl::BrushEdge* closestEdge = nullptr;
  for (const auto* edge : face.edges())
  {
    const auto distance = vm::distance(edge->segment(), hitPoint).distance;
    if (distance < closestEdgeDistance)
    {
      closestEdge = edge;
      closestEdgeDistance = distance;
    }
  }

  if (closestEdge != nullptr)
  {
    const auto firstFaceIndex = closestEdge->firstFace()->payload();
    const auto secondFaceIndex = closestEdge->secondFace()->payload();

    if (firstFaceIndex && secondFaceIndex)
    {
      return {
        &brushNode->brush().face(*firstFaceIndex),
        &brushNode->brush().face(*secondFaceIndex)};
    }
  }

  return {&face};
}

std::vector<vm::vec3d> selectHelpVectors(
  const mdl::BrushNode* brushNode, const mdl::BrushFace& face, const vm::vec3d& hitPoint)
{
  auto result = std::vector<vm::vec3d>{};
  for (const mdl::BrushFace* incidentFace :
       selectIncidentFaces(brushNode, face, hitPoint))
  {
    const vm::vec3d& normal = incidentFace->boundary().normal;
    result.push_back(vm::get_abs_max_component_axis(normal));
  }

  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

class PartDelegate3D : public PartDelegateBase
{
public:
  explicit PartDelegate3D(ClipTool& tool)
    : PartDelegateBase{tool}
  {
  }

  HandlePositionProposer makeHandlePositionProposer(
    const InputState&,
    const vm::vec3d& /* initialHandlePosition */,
    const vm::vec3d& /* handleOffset */) const override
  {
    return makeBrushFaceHandleProposer(m_tool.grid());
  }

  std::vector<vm::vec3d> getHelpVectors(
    const InputState& inputState, const vm::vec3d& clipPoint) const override
  {
    using namespace mdl::HitFilters;

    auto hit =
      inputState.pickResult().first(type(mdl::BrushNode::BrushHitType) && selected());
    if (!hit.isMatch())
    {
      hit = inputState.pickResult().first(type(mdl::BrushNode::BrushHitType));
    }
    const auto faceHandle = mdl::hitToFaceHandle(hit);
    ensure(faceHandle, "hit is not a match");

    return selectHelpVectors(faceHandle->node(), faceHandle->face(), clipPoint);
  }

  std::optional<std::tuple<vm::vec3d, vm::vec3d>> doGetNewClipPointPositionAndHitPoint(
    const InputState& inputState) const override
  {
    using namespace mdl::HitFilters;

    const auto& hit = inputState.pickResult().first(type(mdl::BrushNode::BrushHitType));
    if (const auto faceHandle = mdl::hitToFaceHandle(hit))
    {
      const auto& grid = m_tool.grid();
      const auto position = grid.snap(hit.hitPoint(), faceHandle->face().boundary());
      return {{position, hit.hitPoint()}};
    }
    return std::nullopt;
  }
};

class PartBase
{
protected:
  std::unique_ptr<PartDelegateBase> m_delegate;

  explicit PartBase(std::unique_ptr<PartDelegateBase> delegate)
    : m_delegate{std::move(delegate)}
  {
    ensure(m_delegate != nullptr, "delegate is null");
  }

public:
  virtual ~PartBase() = default;
};

class AddClipPointDragDelegate : public HandleDragTrackerDelegate
{
private:
  PartDelegateBase& m_delegate;
  bool m_secondPointSet{false};

public:
  explicit AddClipPointDragDelegate(PartDelegateBase& delegate)
    : m_delegate{delegate}
  {
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3d& initialHandlePosition,
    const vm::vec3d& handleOffset) override
  {
    return m_delegate.makeHandlePositionProposer(
      inputState, initialHandlePosition, handleOffset);
  }

  DragStatus update(
    const InputState& inputState,
    const DragState&,
    const vm::vec3d& proposedHandlePosition) override
  {
    if (!m_secondPointSet)
    {
      if (m_delegate.addClipPoint(inputState))
      {
        m_delegate.tool().beginDragLastPoint();
        m_secondPointSet = true;
        return DragStatus::Continue;
      }
    }
    else
    {
      if (m_delegate.tool().dragPoint(
            proposedHandlePosition,
            m_delegate.getHelpVectors(inputState, proposedHandlePosition)))
      {
        return DragStatus::Continue;
      }
    }
    return DragStatus::Deny;
  }

  void end(const InputState&, const DragState&) override
  {
    if (m_secondPointSet)
    {
      m_delegate.tool().endDragPoint();
    }
  }

  void cancel(const DragState&) override
  {
    if (m_secondPointSet)
    {
      m_delegate.tool().cancelDragPoint();
      m_delegate.tool().removeLastPoint();
    }
    m_delegate.tool().removeLastPoint();
  }
};

class AddClipPointPart : public ToolController, protected PartBase
{
public:
  explicit AddClipPointPart(std::unique_ptr<PartDelegateBase> delegate)
    : PartBase{std::move(delegate)}
  {
  }

private:
  Tool& tool() override { return m_delegate->tool(); }

  const Tool& tool() const override { return m_delegate->tool(); }

  bool mouseClick(const InputState& inputState) override
  {
    if (
      !inputState.mouseButtonsPressed(MouseButtons::Left)
      || !inputState.modifierKeysPressed(ModifierKeys::None))
    {
      return false;
    }

    return m_delegate->addClipPoint(inputState) != std::nullopt;
  }

  bool mouseDoubleClick(const InputState& inputState) override
  {
    if (
      !inputState.mouseButtonsPressed(MouseButtons::Left)
      || !inputState.modifierKeysPressed(ModifierKeys::None))
    {
      return false;
    }
    return m_delegate->setClipFace(inputState);
  }

  std::unique_ptr<GestureTracker> acceptMouseDrag(const InputState& inputState) override
  {
    if (
      inputState.mouseButtons() != MouseButtons::Left
      || inputState.modifierKeys() != ModifierKeys::None)
    {
      return nullptr;
    }

    const auto initialHandlePositionAndHitPoint = m_delegate->addClipPoint(inputState);
    if (!initialHandlePositionAndHitPoint)
    {
      return nullptr;
    }

    const auto [initialHandlePosition, hitPoint] = *initialHandlePositionAndHitPoint;
    return createHandleDragTracker(
      AddClipPointDragDelegate{*m_delegate}, inputState, initialHandlePosition, hitPoint);
  }

  void render(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override
  {
    m_delegate->renderFeedback(inputState, renderContext, renderBatch);
  }

  bool cancel() override { return false; }
};

class MoveClipPointDragDelegate : public HandleDragTrackerDelegate
{
private:
  PartDelegateBase& m_delegate;

public:
  explicit MoveClipPointDragDelegate(PartDelegateBase& delegate)
    : m_delegate{delegate}
  {
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3d& initialHandlePosition,
    const vm::vec3d& handleOffset) override
  {
    return m_delegate.makeHandlePositionProposer(
      inputState, initialHandlePosition, handleOffset);
  }

  DragStatus update(
    const InputState& inputState,
    const DragState&,
    const vm::vec3d& proposedHandlePosition) override
  {
    if (m_delegate.tool().dragPoint(
          proposedHandlePosition,
          m_delegate.getHelpVectors(inputState, proposedHandlePosition)))
    {
      return DragStatus::Continue;
    }
    else
    {
      return DragStatus::Deny;
    }
  }

  void end(const InputState&, const DragState&) override
  {
    m_delegate.tool().endDragPoint();
  }

  void cancel(const DragState&) override { m_delegate.tool().cancelDragPoint(); }
};

class MoveClipPointPart : public ToolController, protected PartBase
{
public:
  explicit MoveClipPointPart(std::unique_ptr<PartDelegateBase> delegate)
    : PartBase{std::move(delegate)}
  {
  }

private:
  Tool& tool() override { return m_delegate->tool(); }

  const Tool& tool() const override { return m_delegate->tool(); }

  std::unique_ptr<GestureTracker> acceptMouseDrag(const InputState& inputState) override
  {
    if (
      inputState.mouseButtons() != MouseButtons::Left
      || inputState.modifierKeys() != ModifierKeys::None)
    {
      return nullptr;
    }

    const auto initialHandlePositionAndHitPoint =
      m_delegate->tool().beginDragPoint(inputState.pickResult());
    if (!initialHandlePositionAndHitPoint)
    {
      return nullptr;
    }

    const auto [initialHandlePosition, handleOffset] = *initialHandlePositionAndHitPoint;
    return createHandleDragTracker(
      MoveClipPointDragDelegate{*m_delegate},
      inputState,
      initialHandlePosition,
      handleOffset);
  }

  bool cancel() override { return false; }
};

} // namespace

ClipToolControllerBase::ClipToolControllerBase(ClipTool& tool)
  : m_tool{tool}
{
}

ClipToolControllerBase::~ClipToolControllerBase() = default;

Tool& ClipToolControllerBase::tool()
{
  return m_tool;
}

const Tool& ClipToolControllerBase::tool() const
{
  return m_tool;
}

void ClipToolControllerBase::pick(
  const InputState& inputState, mdl::PickResult& pickResult)
{
  m_tool.pick(inputState.pickRay(), inputState.camera(), pickResult);
}

void ClipToolControllerBase::setRenderOptions(
  const InputState&, render::RenderContext& renderContext) const
{
  if (m_tool.hasBrushes())
  {
    renderContext.setHideSelection();
    renderContext.setForceHideSelectionGuide();
  }
}

void ClipToolControllerBase::render(
  const InputState& inputState,
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch)
{
  m_tool.render(renderContext, renderBatch, inputState.pickResult());
  ToolControllerGroup::render(inputState, renderContext, renderBatch);
}

bool ClipToolControllerBase::cancel()
{
  if (m_tool.removeLastPoint())
  {
    if (!m_tool.hasPoints())
    {
      m_tool.reset();
    }
    return true;
  }
  return false;
}

ClipToolController2D::ClipToolController2D(ClipTool& tool)
  : ClipToolControllerBase{tool}
{
  addController(
    std::make_unique<MoveClipPointPart>(std::make_unique<PartDelegate2D>(tool)));
  addController(
    std::make_unique<AddClipPointPart>(std::make_unique<PartDelegate2D>(tool)));
}

ClipToolController3D::ClipToolController3D(ClipTool& tool)
  : ClipToolControllerBase{tool}
{
  addController(
    std::make_unique<MoveClipPointPart>(std::make_unique<PartDelegate3D>(tool)));
  addController(
    std::make_unique<AddClipPointPart>(std::make_unique<PartDelegate3D>(tool)));
}

} // namespace tb::View
