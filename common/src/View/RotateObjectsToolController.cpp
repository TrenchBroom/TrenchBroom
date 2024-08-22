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

#include "RotateObjectsToolController.h"

#include "FloatType.h"
#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/Circle.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/Renderable.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "View/HandleDragTracker.h"
#include "View/InputState.h"
#include "View/MoveHandleDragTracker.h"
#include "View/RotateObjectsTool.h"
#include "View/ToolController.h"

#include "vm/intersection.h"
#include "vm/mat_ext.h"
#include "vm/quat.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <functional>
#include <memory>
#include <sstream>

namespace TrenchBroom
{
namespace View
{
namespace
{
class AngleIndicatorRenderer : public Renderer::DirectRenderable
{
private:
  vm::vec3 m_position;
  Renderer::Circle m_circle;

public:
  AngleIndicatorRenderer(
    const vm::vec3& position,
    const float radius,
    const vm::axis::type axis,
    const vm::vec3& startAxis,
    const vm::vec3& endAxis)
    : m_position{position}
    , m_circle{radius, 24, true, axis, vm::vec3f{startAxis}, vm::vec3f{endAxis}}
  {
  }

private:
  void doPrepareVertices(Renderer::VboManager& vboManager) override
  {
    m_circle.prepare(vboManager);
  }

  void doRender(Renderer::RenderContext& renderContext) override
  {
    glAssert(glDisable(GL_DEPTH_TEST));

    glAssert(glPushAttrib(GL_POLYGON_BIT));
    glAssert(glDisable(GL_CULL_FACE));
    glAssert(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    auto translation = Renderer::MultiplyModelMatrix{
      renderContext.transformation(), vm::translation_matrix(vm::vec3f{m_position})};
    auto shader = Renderer::ActiveShader{
      renderContext.shaderManager(), Renderer::Shaders::VaryingPUniformCShader};
    shader.set("Color", Color(1.0f, 1.0f, 1.0f, 0.2f));
    m_circle.render();

    glAssert(glEnable(GL_DEPTH_TEST));
    glAssert(glPopAttrib());
  }
};

using RenderHighlight = std::function<void(
  const InputState&,
  Renderer::RenderContext&,
  Renderer::RenderBatch&,
  RotateObjectsHandle::HitArea)>;

class RotateObjectsDragDelegate : public HandleDragTrackerDelegate
{
private:
  RotateObjectsTool& m_tool;
  RotateObjectsHandle::HitArea m_area;
  RenderHighlight m_renderHighlight;
  FloatType m_angle{0.0};

public:
  RotateObjectsDragDelegate(
    RotateObjectsTool& tool,
    const RotateObjectsHandle::HitArea area,
    RenderHighlight renderHighlight)
    : m_tool{tool}
    , m_area{area}
    , m_renderHighlight{std::move(renderHighlight)}
  {
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3& /* initialHandlePosition */,
    const vm::vec3& handleOffset) override
  {
    const auto center = m_tool.rotationCenter();
    const auto axis = m_tool.rotationAxis(m_area);
    const auto radius = m_tool.majorHandleRadius(inputState.camera());

    return makeHandlePositionProposer(
      makeCircleHandlePicker(center, axis, radius, handleOffset),
      makeCircleHandleSnapper(m_tool.grid(), m_tool.angle(), center, axis, radius));
  }

  DragStatus drag(
    const InputState&,
    const DragState& dragState,
    const vm::vec3& proposedHandlePosition) override
  {
    const auto center = m_tool.rotationCenter();
    const auto axis = m_tool.rotationAxis(m_area);
    const vm::vec3 ref = vm::normalize(dragState.initialHandlePosition - center);
    const vm::vec3 vec = vm::normalize(proposedHandlePosition - center);
    m_angle = vm::measure_angle(vec, ref, axis);
    m_tool.applyRotation(center, axis, m_angle);

    return DragStatus::Continue;
  }

  void end(const InputState&, const DragState&) override { m_tool.commitRotation(); }

  void cancel(const DragState&) override { m_tool.cancelRotation(); }

  void setRenderOptions(
    const InputState&, Renderer::RenderContext& renderContext) const override
  {
    renderContext.setForceShowSelectionGuide();
  }

  void render(
    const InputState& inputState,
    const DragState& dragState,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) const override
  {
    m_renderHighlight(inputState, renderContext, renderBatch, m_area);
    renderAngleIndicator(renderContext, renderBatch, dragState.initialHandlePosition);
    renderAngleText(renderContext, renderBatch);
  }

private:
  void renderAngleIndicator(
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    const vm::vec3& initialHandlePosition) const
  {
    const auto center = m_tool.rotationCenter();
    const auto axis = m_tool.rotationAxis(m_area);
    const auto handleRadius =
      static_cast<float>(m_tool.majorHandleRadius(renderContext.camera()));
    const auto startAxis = vm::normalize(initialHandlePosition - center);
    const auto endAxis = vm::quat3{axis, m_angle} * startAxis;

    renderBatch.addOneShot(new AngleIndicatorRenderer{
      center, handleRadius, vm::find_abs_max_component(axis), startAxis, endAxis});
  }

  void renderAngleText(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const
  {
    const auto center = m_tool.rotationCenter();

    auto renderService = Renderer::RenderService{renderContext, renderBatch};

    renderService.setForegroundColor(pref(Preferences::SelectedInfoOverlayTextColor));
    renderService.setBackgroundColor(
      pref(Preferences::SelectedInfoOverlayBackgroundColor));
    renderService.renderString(angleString(vm::to_degrees(m_angle)), vm::vec3f{center});
  }

  std::string angleString(const FloatType angle) const
  {
    auto str = std::stringstream{};
    str.precision(2);
    str.setf(std::ios::fixed);
    str << angle;
    return str.str();
  }
};

class RotateObjectsBase : public ToolController
{
protected:
  RotateObjectsTool& m_tool;

protected:
  explicit RotateObjectsBase(RotateObjectsTool& tool)
    : m_tool(tool)
  {
  }

private:
  Tool& tool() override { return m_tool; }

  const Tool& tool() const override { return m_tool; }

  bool mouseClick(const InputState& inputState) override
  {
    using namespace Model::HitFilters;

    if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
    {
      return false;
    }

    const Model::Hit& hit =
      inputState.pickResult().first(type(RotateObjectsHandle::HandleHitType));
    if (!hit.isMatch())
    {
      return false;
    }

    const RotateObjectsHandle::HitArea area = hit.target<RotateObjectsHandle::HitArea>();
    if (area == RotateObjectsHandle::HitArea::Center)
    {
      return false;
    }

    m_tool.updateToolPageAxis(area);
    return true;
  }

  std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override
  {
    using namespace Model::HitFilters;

    if (
      inputState.mouseButtons() != MouseButtons::MBLeft
      || inputState.modifierKeys() != ModifierKeys::MKNone)
    {
      return nullptr;
    }

    const Model::Hit& hit =
      inputState.pickResult().first(type(RotateObjectsHandle::HandleHitType));
    if (!hit.isMatch())
    {
      return nullptr;
    }

    const RotateObjectsHandle::HitArea area = hit.target<RotateObjectsHandle::HitArea>();
    if (area == RotateObjectsHandle::HitArea::Center)
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
        vm::intersect_ray_plane(inputState.pickRay(), vm::plane3{center, axis}))
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
        RotateObjectsDragDelegate{m_tool, area, std::move(renderHighlight)},
        inputState,
        initialHandlePosition,
        initialHandlePosition);
    }

    return nullptr;
  }

  void render(
    const InputState& inputState,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) override
  {
    using namespace Model::HitFilters;

    if (!inputState.anyToolDragging())
    {
      const Model::Hit& hit =
        inputState.pickResult().first(type(RotateObjectsHandle::HandleHitType));
      if (hit.isMatch())
      {
        const RotateObjectsHandle::HitArea area =
          hit.target<RotateObjectsHandle::HitArea>();
        if (area != RotateObjectsHandle::HitArea::Center)
        {
          doRenderHighlight(
            inputState,
            renderContext,
            renderBatch,
            hit.target<RotateObjectsHandle::HitArea>());
        }
      }
    }
  }

  bool cancel() override { return false; }

private:
  virtual void doRenderHighlight(
    const InputState& inputState,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    RotateObjectsHandle::HitArea area) = 0;
};

class MoveRotationCenterDragDelegate : public MoveHandleDragTrackerDelegate
{
private:
  RotateObjectsTool& m_tool;
  RenderHighlight m_renderHighlight;

public:
  MoveRotationCenterDragDelegate(RotateObjectsTool& tool, RenderHighlight renderHighlight)
    : m_tool{tool}
    , m_renderHighlight{std::move(renderHighlight)}
  {
  }

  DragStatus move(
    const InputState&, const DragState&, const vm::vec3& currentHandlePosition) override
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
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) const override
  {
    m_renderHighlight(
      inputState, renderContext, renderBatch, RotateObjectsHandle::HitArea::Center);
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
  RotateObjectsTool& m_tool;

protected:
  explicit MoveCenterBase(RotateObjectsTool& tool)
    : m_tool(tool)
  {
  }

  Tool& tool() override { return m_tool; }

  const Tool& tool() const override { return m_tool; }

  std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override
  {
    using namespace Model::HitFilters;

    if (
      !inputState.mouseButtonsPressed(MouseButtons::MBLeft)
      || !inputState.checkModifierKeys(
        ModifierKeyPressed::MK_No,
        ModifierKeyPressed::MK_DontCare,
        ModifierKeyPressed::MK_No))
    {
      return nullptr;
    }

    const Model::Hit& hit =
      inputState.pickResult().first(type(RotateObjectsHandle::HandleHitType));
    if (!hit.isMatch())
    {
      return nullptr;
    }

    if (
      hit.target<RotateObjectsHandle::HitArea>() != RotateObjectsHandle::HitArea::Center)
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
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) override
  {
    using namespace Model::HitFilters;

    if (!inputState.anyToolDragging())
    {
      const Model::Hit& hit =
        inputState.pickResult().first(type(RotateObjectsHandle::HandleHitType));
      if (
        hit.isMatch()
        && hit.target<RotateObjectsHandle::HitArea>()
             == RotateObjectsHandle::HitArea::Center)
      {
        doRenderHighlight(
          inputState, renderContext, renderBatch, RotateObjectsHandle::HitArea::Center);
      }
    }
  }

  bool cancel() override { return false; }

private:
  virtual void doRenderHighlight(
    const InputState& inputState,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    RotateObjectsHandle::HitArea area) = 0;
};

class MoveCenterPart2D : public MoveCenterBase
{
public:
  explicit MoveCenterPart2D(RotateObjectsTool& tool)
    : MoveCenterBase(tool)
  {
  }

private:
  void doRenderHighlight(
    const InputState&,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    RotateObjectsHandle::HitArea area) override
  {
    m_tool.renderHighlight2D(renderContext, renderBatch, area);
  }
};

class RotateObjectsPart2D : public RotateObjectsBase
{
public:
  explicit RotateObjectsPart2D(RotateObjectsTool& tool)
    : RotateObjectsBase(tool)
  {
  }

private:
  void doRenderHighlight(
    const InputState&,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    RotateObjectsHandle::HitArea area) override
  {
    m_tool.renderHighlight2D(renderContext, renderBatch, area);
  }
};

class MoveCenterPart3D : public MoveCenterBase
{
public:
  explicit MoveCenterPart3D(RotateObjectsTool& tool)
    : MoveCenterBase(tool)
  {
  }

private:
  void doRenderHighlight(
    const InputState&,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    RotateObjectsHandle::HitArea area) override
  {
    m_tool.renderHighlight3D(renderContext, renderBatch, area);
  }
};

class RotateObjectsPart3D : public RotateObjectsBase
{
public:
  explicit RotateObjectsPart3D(RotateObjectsTool& tool)
    : RotateObjectsBase(tool)
  {
  }

private:
  void doRenderHighlight(
    const InputState&,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch,
    RotateObjectsHandle::HitArea area) override
  {
    m_tool.renderHighlight3D(renderContext, renderBatch, area);
  }
};
} // namespace

RotateObjectsToolController::RotateObjectsToolController(RotateObjectsTool& tool)
  : m_tool(tool)
{
}

RotateObjectsToolController::~RotateObjectsToolController() = default;

Tool& RotateObjectsToolController::tool()
{
  return m_tool;
}

const Tool& RotateObjectsToolController::tool() const
{
  return m_tool;
}

void RotateObjectsToolController::pick(
  const InputState& inputState, Model::PickResult& pickResult)
{
  const Model::Hit hit = doPick(inputState);
  if (hit.isMatch())
  {
    pickResult.addHit(hit);
  }
}

void RotateObjectsToolController::setRenderOptions(
  const InputState& inputState, Renderer::RenderContext& renderContext) const
{
  using namespace Model::HitFilters;
  if (inputState.pickResult().first(type(RotateObjectsHandle::HandleHitType)).isMatch())
  {
    renderContext.setForceShowSelectionGuide();
  }
}

void RotateObjectsToolController::render(
  const InputState& inputState,
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch)
{
  doRenderHandle(renderContext, renderBatch);
  ToolControllerGroup::render(inputState, renderContext, renderBatch);
}

bool RotateObjectsToolController::cancel()
{
  return false;
}

RotateObjectsToolController2D::RotateObjectsToolController2D(RotateObjectsTool& tool)
  : RotateObjectsToolController(tool)
{
  addController(std::make_unique<MoveCenterPart2D>(tool));
  addController(std::make_unique<RotateObjectsPart2D>(tool));
}

Model::Hit RotateObjectsToolController2D::doPick(const InputState& inputState)
{
  return m_tool.pick2D(inputState.pickRay(), inputState.camera());
}

void RotateObjectsToolController2D::doRenderHandle(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch)
{
  m_tool.renderHandle2D(renderContext, renderBatch);
}

RotateObjectsToolController3D::RotateObjectsToolController3D(RotateObjectsTool& tool)
  : RotateObjectsToolController(tool)
{
  addController(std::make_unique<MoveCenterPart3D>(tool));
  addController(std::make_unique<RotateObjectsPart3D>(tool));
}

Model::Hit RotateObjectsToolController3D::doPick(const InputState& inputState)
{
  return m_tool.pick3D(inputState.pickRay(), inputState.camera());
}

void RotateObjectsToolController3D::doRenderHandle(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch)
{
  m_tool.renderHandle3D(renderContext, renderBatch);
}
} // namespace View
} // namespace TrenchBroom
