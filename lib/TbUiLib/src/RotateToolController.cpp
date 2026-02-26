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

#include "ui/RotateToolController.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "gl/ActiveShader.h"
#include "gl/GlInterface.h"
#include "gl/Shaders.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "render/Circle.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"
#include "render/RenderService.h"
#include "render/Renderable.h"
#include "ui/HandleDragTracker.h"
#include "ui/InputState.h"
#include "ui/MoveHandleDragTracker.h"
#include "ui/RotateHandle.h"
#include "ui/RotateTool.h"
#include "ui/ToolController.h"

#include "vm/intersection.h"
#include "vm/mat_ext.h"
#include "vm/quat.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <functional>
#include <memory>
#include <sstream>

namespace tb::ui
{
namespace
{

class AngleIndicatorRenderer : public render::DirectRenderable
{
private:
  vm::vec3d m_position;
  render::Circle m_circle;

public:
  AngleIndicatorRenderer(
    const vm::vec3d& position,
    const float radius,
    const vm::axis::type axis,
    const vm::vec3d& startAxis,
    const vm::vec3d& endAxis)
    : m_position{position}
    , m_circle{radius, 24, true, axis, vm::vec3f{startAxis}, vm::vec3f{endAxis}}
  {
  }

  void prepare(gl::Gl& gl, gl::VboManager& vboManager) override
  {
    m_circle.prepare(gl, vboManager);
  }

  void render(render::RenderContext& renderContext) override
  {
    auto& gl = renderContext.gl();

    gl.disable(GL_DEPTH_TEST);

    gl.pushAttrib(GL_POLYGON_BIT);
    gl.disable(GL_CULL_FACE);
    gl.polygonMode(GL_FRONT_AND_BACK, GL_FILL);

    auto translation = render::MultiplyModelMatrix{
      renderContext.transformation(), vm::translation_matrix(vm::vec3f{m_position})};
    auto shader = gl::ActiveShader{
      gl, renderContext.shaderManager(), gl::Shaders::VaryingPUniformCShader};
    shader.set("Color", RgbaF{1.0f, 1.0f, 1.0f, 0.2f});
    m_circle.render(gl, shader.program());

    gl.enable(GL_DEPTH_TEST);
    gl.popAttrib();
  }
};

using RenderHighlight = std::function<void(
  const InputState&,
  render::RenderContext&,
  render::RenderBatch&,
  RotateHandle::HitArea)>;

class RotateDragDelegate : public HandleDragTrackerDelegate
{
private:
  RotateTool& m_tool;
  RotateHandle::HitArea m_area;
  RenderHighlight m_renderHighlight;
  double m_angle = 0.0;

public:
  RotateDragDelegate(
    RotateTool& tool, const RotateHandle::HitArea area, RenderHighlight renderHighlight)
    : m_tool{tool}
    , m_area{area}
    , m_renderHighlight{std::move(renderHighlight)}
  {
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3d& /* initialHandlePosition */,
    const vm::vec3d& handleOffset) override
  {
    const auto center = m_tool.rotationCenter();
    const auto axis = m_tool.rotationAxis(m_area);
    const auto radius = m_tool.majorHandleRadius(inputState.camera());

    return makeHandlePositionProposer(
      makeCircleHandlePicker(center, axis, radius, handleOffset),
      makeCircleHandleSnapper(m_tool.grid(), m_tool.angle(), center, axis, radius));
  }

  DragStatus update(
    const InputState&,
    const DragState& dragState,
    const vm::vec3d& proposedHandlePosition) override
  {
    const auto center = m_tool.rotationCenter();
    const auto axis = m_tool.rotationAxis(m_area);
    const auto ref = vm::normalize(dragState.initialHandlePosition - center);
    const auto vec = vm::normalize(proposedHandlePosition - center);
    m_angle = vm::measure_angle(vec, ref, axis);
    m_tool.applyRotation(center, axis, m_angle);

    return DragStatus::Continue;
  }

  void end(const InputState&, const DragState&) override { m_tool.commitRotation(); }

  void cancel(const DragState&) override { m_tool.cancelRotation(); }

  void setRenderOptions(
    const InputState&, render::RenderContext& renderContext) const override
  {
    renderContext.setForceShowSelectionGuide();
  }

  void render(
    const InputState& inputState,
    const DragState& dragState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) const override
  {
    m_renderHighlight(inputState, renderContext, renderBatch, m_area);
    renderAngleIndicator(renderContext, renderBatch, dragState.initialHandlePosition);
    renderAngleText(renderContext, renderBatch);
  }

private:
  void renderAngleIndicator(
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    const vm::vec3d& initialHandlePosition) const
  {
    const auto center = m_tool.rotationCenter();
    const auto axis = m_tool.rotationAxis(m_area);
    const auto handleRadius =
      static_cast<float>(m_tool.majorHandleRadius(renderContext.camera()));
    const auto startAxis = vm::normalize(initialHandlePosition - center);
    const auto endAxis = vm::quatd{axis, m_angle} * startAxis;

    renderBatch.addOneShot(new AngleIndicatorRenderer{
      center, handleRadius, vm::find_abs_max_component(axis), startAxis, endAxis});
  }

  void renderAngleText(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const
  {
    const auto center = m_tool.rotationCenter();

    auto renderService = render::RenderService{renderContext, renderBatch};

    renderService.setForegroundColor(pref(Preferences::SelectedInfoOverlayTextColor));
    renderService.setBackgroundColor(
      pref(Preferences::SelectedInfoOverlayBackgroundColor));
    renderService.renderString(angleString(vm::to_degrees(m_angle)), vm::vec3f{center});
  }

  std::string angleString(const double angle) const
  {
    auto str = std::stringstream{};
    str.precision(2);
    str.setf(std::ios::fixed);
    str << angle;
    return str.str();
  }
};

class RotatePartBase : public ToolController
{
protected:
  RotateTool& m_tool;

protected:
  explicit RotatePartBase(RotateTool& tool)
    : m_tool(tool)
  {
  }

private:
  Tool& tool() override { return m_tool; }

  const Tool& tool() const override { return m_tool; }

  bool mouseClick(const InputState& inputState) override
  {
    using namespace mdl::HitFilters;

    if (!inputState.mouseButtonsPressed(MouseButtons::Left))
    {
      return false;
    }

    const auto& hit = inputState.pickResult().first(type(RotateHandle::HandleHitType));
    if (!hit.isMatch())
    {
      return false;
    }

    const auto area = hit.target<RotateHandle::HitArea>();
    if (area == RotateHandle::HitArea::Center)
    {
      return false;
    }

    m_tool.updateToolPageAxis(area);
    return true;
  }

  std::unique_ptr<GestureTracker> acceptMouseDrag(const InputState& inputState) override
  {
    using namespace mdl::HitFilters;

    if (
      inputState.mouseButtons() != MouseButtons::Left
      || inputState.modifierKeys() != ModifierKeys::None)
    {
      return nullptr;
    }

    const auto& hit = inputState.pickResult().first(type(RotateHandle::HandleHitType));
    if (!hit.isMatch())
    {
      return nullptr;
    }

    const auto area = hit.target<RotateHandle::HitArea>();
    if (area == RotateHandle::HitArea::Center)
    {
      return nullptr;
    }

    // We cannot use the hit's hitpoint because it is on the surface of the handle torus,
    // whereas our drag snapper expects it to be on the plane defined by the rotation
    // handle center and the rotation axis.
    const auto center = m_tool.rotationCenter();
    const auto axis = m_tool.rotationAxis(area);

    if (
      const auto distance =
        vm::intersect_ray_plane(inputState.pickRay(), vm::plane3d{center, axis}))
    {
      const auto initialHandlePosition =
        vm::point_at_distance(inputState.pickRay(), *distance);
      auto renderHighlight = [this](
                               const auto& inputState_,
                               auto& renderContext,
                               auto& renderBatch,
                               const auto area_) {
        doRenderHighlight(inputState_, renderContext, renderBatch, area_);
      };

      m_tool.beginRotation();
      return createHandleDragTracker(
        RotateDragDelegate{m_tool, area, std::move(renderHighlight)},
        inputState,
        initialHandlePosition,
        initialHandlePosition);
    }

    return nullptr;
  }

  void render(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override
  {
    using namespace mdl::HitFilters;

    if (!inputState.anyToolDragging())
    {
      const auto& hit = inputState.pickResult().first(type(RotateHandle::HandleHitType));
      if (hit.isMatch())
      {
        const auto area = hit.target<RotateHandle::HitArea>();
        if (area != RotateHandle::HitArea::Center)
        {
          doRenderHighlight(
            inputState, renderContext, renderBatch, hit.target<RotateHandle::HitArea>());
        }
      }
    }
  }

  bool cancel() override { return false; }

private:
  virtual void doRenderHighlight(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    RotateHandle::HitArea area) = 0;
};

class MoveRotationCenterDragDelegate : public MoveHandleDragTrackerDelegate
{
private:
  RotateTool& m_tool;
  RenderHighlight m_renderHighlight;

public:
  MoveRotationCenterDragDelegate(RotateTool& tool, RenderHighlight renderHighlight)
    : m_tool{tool}
    , m_renderHighlight{std::move(renderHighlight)}
  {
  }

  DragStatus move(
    const InputState&, const DragState&, const vm::vec3d& currentHandlePosition) override
  {
    m_tool.setRotationCenter(currentHandlePosition);
    return DragStatus::Continue;
  }

  void end(const InputState&, const DragState&) override {}

  void cancel(const DragState& dragState) override
  {
    m_tool.setRotationCenter(dragState.initialHandlePosition);
  }

  void render(
    const InputState& inputState,
    const DragState&,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) const override
  {
    m_renderHighlight(
      inputState, renderContext, renderBatch, RotateHandle::HitArea::Center);
  }

  DragHandleSnapper makeDragHandleSnapper(
    const InputState&, const SnapMode snapMode) const override
  {
    return makeDragHandleSnapperFromSnapMode(m_tool.grid(), snapMode);
  }
};

class MoveCenterBase : public ToolController
{
protected:
  RotateTool& m_tool;

protected:
  explicit MoveCenterBase(RotateTool& tool)
    : m_tool{tool}
  {
  }

  Tool& tool() override { return m_tool; }

  const Tool& tool() const override { return m_tool; }

  std::unique_ptr<GestureTracker> acceptMouseDrag(const InputState& inputState) override
  {
    using namespace mdl::HitFilters;

    if (
      !inputState.mouseButtonsPressed(MouseButtons::Left)
      || !inputState.checkModifierKeys(
        ModifierKeyPressed::No, ModifierKeyPressed::DontCare, ModifierKeyPressed::No))
    {
      return nullptr;
    }

    const auto& hit = inputState.pickResult().first(type(RotateHandle::HandleHitType));
    if (!hit.isMatch())
    {
      return nullptr;
    }

    if (hit.target<RotateHandle::HitArea>() != RotateHandle::HitArea::Center)
    {
      return nullptr;
    }

    auto renderHighlight = [this](
                             const auto& inputState_,
                             auto& renderContext,
                             auto& renderBatch,
                             const auto area_) {
      doRenderHighlight(inputState_, renderContext, renderBatch, area_);
    };

    return createMoveHandleDragTracker(
      MoveRotationCenterDragDelegate{m_tool, std::move(renderHighlight)},
      inputState,
      m_tool.rotationCenter(),
      hit.hitPoint());
  }

  void render(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override
  {
    using namespace mdl::HitFilters;

    if (!inputState.anyToolDragging())
    {
      const auto& hit = inputState.pickResult().first(type(RotateHandle::HandleHitType));
      if (
        hit.isMatch()
        && hit.target<RotateHandle::HitArea>() == RotateHandle::HitArea::Center)
      {
        doRenderHighlight(
          inputState, renderContext, renderBatch, RotateHandle::HitArea::Center);
      }
    }
  }

  bool cancel() override { return false; }

private:
  virtual void doRenderHighlight(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    RotateHandle::HitArea area) = 0;
};

class MoveCenterPart2D : public MoveCenterBase
{
public:
  explicit MoveCenterPart2D(RotateTool& tool)
    : MoveCenterBase{tool}
  {
  }

private:
  void doRenderHighlight(
    const InputState&,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    RotateHandle::HitArea area) override
  {
    m_tool.renderHighlight2D(renderContext, renderBatch, area);
  }
};

class RotatePart2D : public RotatePartBase
{
public:
  explicit RotatePart2D(RotateTool& tool)
    : RotatePartBase{tool}
  {
  }

private:
  void doRenderHighlight(
    const InputState&,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    RotateHandle::HitArea area) override
  {
    m_tool.renderHighlight2D(renderContext, renderBatch, area);
  }
};

class MoveCenterPart3D : public MoveCenterBase
{
public:
  explicit MoveCenterPart3D(RotateTool& tool)
    : MoveCenterBase{tool}
  {
  }

private:
  void doRenderHighlight(
    const InputState&,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    RotateHandle::HitArea area) override
  {
    m_tool.renderHighlight3D(renderContext, renderBatch, area);
  }
};

class RotatePart3D : public RotatePartBase
{
public:
  explicit RotatePart3D(RotateTool& tool)
    : RotatePartBase{tool}
  {
  }

private:
  void doRenderHighlight(
    const InputState&,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    RotateHandle::HitArea area) override
  {
    m_tool.renderHighlight3D(renderContext, renderBatch, area);
  }
};

} // namespace

RotateToolController::RotateToolController(RotateTool& tool)
  : m_tool{tool}
{
}

RotateToolController::~RotateToolController() = default;

Tool& RotateToolController::tool()
{
  return m_tool;
}

const Tool& RotateToolController::tool() const
{
  return m_tool;
}

void RotateToolController::pick(const InputState& inputState, mdl::PickResult& pickResult)
{
  const auto hit = doPick(inputState);
  if (hit.isMatch())
  {
    pickResult.addHit(hit);
  }
}

void RotateToolController::setRenderOptions(
  const InputState& inputState, render::RenderContext& renderContext) const
{
  using namespace mdl::HitFilters;
  if (inputState.pickResult().first(type(RotateHandle::HandleHitType)).isMatch())
  {
    renderContext.setForceShowSelectionGuide();
  }
}

void RotateToolController::render(
  const InputState& inputState,
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch)
{
  doRenderHandle(renderContext, renderBatch);
  ToolControllerGroup::render(inputState, renderContext, renderBatch);
}

bool RotateToolController::cancel()
{
  return false;
}

RotateToolController2D::RotateToolController2D(RotateTool& tool)
  : RotateToolController{tool}
{
  addController(std::make_unique<MoveCenterPart2D>(tool));
  addController(std::make_unique<RotatePart2D>(tool));
}

mdl::Hit RotateToolController2D::doPick(const InputState& inputState)
{
  return m_tool.pick2D(inputState.pickRay(), inputState.camera());
}

void RotateToolController2D::doRenderHandle(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  m_tool.renderHandle2D(renderContext, renderBatch);
}

RotateToolController3D::RotateToolController3D(RotateTool& tool)
  : RotateToolController{tool}
{
  addController(std::make_unique<MoveCenterPart3D>(tool));
  addController(std::make_unique<RotatePart3D>(tool));
}

mdl::Hit RotateToolController3D::doPick(const InputState& inputState)
{
  return m_tool.pick3D(inputState.pickRay(), inputState.camera());
}

void RotateToolController3D::doRenderHandle(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  m_tool.renderHandle3D(renderContext, renderBatch);
}

} // namespace tb::ui
