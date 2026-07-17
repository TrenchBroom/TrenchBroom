/*
 Copyright (C) 2026 Kristian Duske

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

#include "ui/RotateHandleController.h"

#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "render/RenderContext.h"
#include "ui/AngleIndicatorRenderer.h"
#include "ui/HandleDragTracker.h"
#include "ui/InputState.h"
#include "ui/MoveHandleDragTracker.h"
#include "ui/ToolController.h"

#include "vm/intersection.h"
#include "vm/scalar.h"
#include "vm/vec.h"

#include <functional>
#include <memory>
#include <utility>

namespace tb::ui
{

RingDragTracker::~RingDragTracker() = default;

RotateHandleDelegate::~RotateHandleDelegate() = default;

void RotateHandleDelegate::handleClicked(RotateHandle::HitArea) {}

PointHandleDelegate::~PointHandleDelegate() = default;

namespace
{

using RenderHighlight = std::function<void(
  const InputState&,
  render::RenderContext&,
  render::RenderBatch&,
  RotateHandle::HitArea)>;

class RingDragDelegate : public HandleDragTrackerDelegate
{
private:
  RotateHandleDelegate& m_delegate;
  RotateHandle::HitArea m_area;
  RenderHighlight m_renderHighlight;
  std::unique_ptr<RingDragTracker> m_session;
  double m_angle = 0.0;

public:
  RingDragDelegate(
    RotateHandleDelegate& delegate,
    const RotateHandle::HitArea area,
    RenderHighlight renderHighlight)
    : m_delegate{delegate}
    , m_area{area}
    , m_renderHighlight{std::move(renderHighlight)}
    , m_session{delegate.beginRingDrag()}
  {
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3d& /* initialHandlePosition */,
    const vm::vec3d& handleOffset) override
  {
    const auto center = m_delegate.handleCenter();
    const auto axis = m_delegate.handle().rotationAxis(m_area);
    const auto radius = m_delegate.handle().majorHandleRadius(inputState.camera());

    return makeHandlePositionProposer(
      makeCircleHandlePicker(center, axis, radius, handleOffset),
      makeCircleHandleSnapper(
        m_delegate.grid(), m_delegate.handleSnapAngle(), center, axis, radius));
  }

  DragStatus update(
    const InputState&,
    const DragState& dragState,
    const vm::vec3d& proposedHandlePosition) override
  {
    const auto center = m_delegate.handleCenter();
    const auto axis = m_delegate.handle().rotationAxis(m_area);
    const auto ref = vm::normalize(dragState.initialHandlePosition - center);
    const auto vec = vm::normalize(proposedHandlePosition - center);
    m_angle = vm::measure_angle(vec, ref, axis);
    m_session->apply(center, axis, m_angle);

    return DragStatus::Continue;
  }

  void end(const InputState&, const DragState&) override { m_session->end(); }

  void cancel(const DragState&) override { m_session->cancel(); }

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

    const auto center = m_delegate.handleCenter();

    // show the shortest equivalent angle, e.g. -5 instead of 355
    const auto normalizedAngle =
      m_angle > vm::Cd::pi() ? m_angle - vm::Cd::two_pi() : m_angle;

    renderAngleIndicator(
      renderBatch,
      m_delegate.handle().majorHandleRadius(renderContext.camera()),
      center,
      m_delegate.handle().rotationAxis(m_area),
      dragState.initialHandlePosition,
      normalizedAngle);

    const auto degrees = vm::to_degrees(normalizedAngle);
    renderAngleText(renderContext, renderBatch, center, degrees);
  }
};

class RingHandlePart : public ToolController
{
protected:
  RotateHandleDelegate& m_delegate;

protected:
  explicit RingHandlePart(RotateHandleDelegate& delegate)
    : m_delegate{delegate}
  {
  }

private:
  Tool& tool() override { return m_delegate.tool(); }

  const Tool& tool() const override { return m_delegate.tool(); }

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

    m_delegate.handleClicked(area);
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
    // whereas our drag snapper expects it to be on the plane defined by the handle
    // center and the rotation axis.
    const auto center = m_delegate.handleCenter();
    const auto axis = m_delegate.handle().rotationAxis(area);

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

      return createHandleDragTracker(
        RingDragDelegate{m_delegate, area, std::move(renderHighlight)},
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

class RingHandlePart2D : public RingHandlePart
{
public:
  explicit RingHandlePart2D(RotateHandleDelegate& delegate)
    : RingHandlePart{delegate}
  {
  }

private:
  void doRenderHighlight(
    const InputState&,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    RotateHandle::HitArea area) override
  {
    m_delegate.handle().renderHighlight2D(renderContext, renderBatch, area);
  }
};

class RingHandlePart3D : public RingHandlePart
{
public:
  explicit RingHandlePart3D(RotateHandleDelegate& delegate)
    : RingHandlePart{delegate}
  {
  }

private:
  void doRenderHighlight(
    const InputState&,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch,
    RotateHandle::HitArea area) override
  {
    m_delegate.handle().renderHighlight3D(renderContext, renderBatch, area);
  }
};

class PointHandleDragDelegate : public MoveHandleDragTrackerDelegate
{
private:
  PointHandleDelegate& m_delegate;

public:
  explicit PointHandleDragDelegate(PointHandleDelegate& delegate)
    : m_delegate{delegate}
  {
  }

  DragStatus move(
    const InputState&, const DragState&, const vm::vec3d& proposedHandlePosition) override
  {
    m_delegate.setHandlePosition(proposedHandlePosition);
    return DragStatus::Continue;
  }

  void end(const InputState&, const DragState&) override {}

  void cancel(const DragState& dragState) override
  {
    m_delegate.setHandlePosition(dragState.initialHandlePosition);
  }

  void render(
    const InputState&,
    const DragState&,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) const override
  {
    m_delegate.renderHighlight(renderContext, renderBatch);
  }

  DragHandleSnapper makeDragHandleSnapper(
    const InputState&, const SnapMode snapMode) const override
  {
    return makeDragHandleSnapperFromSnapMode(m_delegate.grid(), snapMode);
  }
};

class PointHandlePart : public ToolController
{
private:
  PointHandleDelegate& m_delegate;
  mdl::HitFilter m_hitFilter;
  AcceptPointHandleDrag m_acceptDrag;

public:
  PointHandlePart(
    PointHandleDelegate& delegate,
    mdl::HitFilter hitFilter,
    AcceptPointHandleDrag acceptDrag)
    : m_delegate{delegate}
    , m_hitFilter{std::move(hitFilter)}
    , m_acceptDrag{std::move(acceptDrag)}
  {
  }

private:
  Tool& tool() override { return m_delegate.tool(); }

  const Tool& tool() const override { return m_delegate.tool(); }

  std::unique_ptr<GestureTracker> acceptMouseDrag(const InputState& inputState) override
  {
    if (!m_acceptDrag(inputState))
    {
      return nullptr;
    }

    const auto& hit = inputState.pickResult().first(m_hitFilter);
    if (!hit.isMatch())
    {
      return nullptr;
    }

    return createMoveHandleDragTracker(
      PointHandleDragDelegate{m_delegate},
      inputState,
      m_delegate.handlePosition(),
      hit.hitPoint());
  }

  void render(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override
  {
    if (
      !inputState.anyToolDragging()
      && inputState.pickResult().first(m_hitFilter).isMatch())
    {
      m_delegate.renderHighlight(renderContext, renderBatch);
    }
  }

  bool cancel() override { return false; }
};

mdl::HitFilter centerHandleHitFilter()
{
  using namespace mdl::HitFilters;
  return type(RotateHandle::HandleHitType) && mdl::HitFilter{[](const mdl::Hit& hit) {
           return hit.target<RotateHandle::HitArea>() == RotateHandle::HitArea::Center;
         }};
}

bool acceptCenterHandleDrag(const InputState& inputState)
{
  return inputState.mouseButtonsPressed(MouseButtons::Left)
         && inputState.checkModifierKeys(
           ModifierKeyPressed::No, ModifierKeyPressed::DontCare, ModifierKeyPressed::No);
}

class CenterHandlePointDelegate : public PointHandleDelegate
{
protected:
  RotateHandleDelegate& m_delegate;

public:
  explicit CenterHandlePointDelegate(RotateHandleDelegate& delegate)
    : m_delegate{delegate}
  {
  }

  Tool& tool() override { return m_delegate.tool(); }

  const Tool& tool() const override { return m_delegate.tool(); }

  const mdl::Grid& grid() const override { return m_delegate.grid(); }

  vm::vec3d handlePosition() const override { return m_delegate.handleCenter(); }

  void setHandlePosition(const vm::vec3d& position) override
  {
    m_delegate.setHandleCenter(position);
  }
};

class CenterHandlePointDelegate2D : public CenterHandlePointDelegate
{
public:
  using CenterHandlePointDelegate::CenterHandlePointDelegate;

  void renderHighlight(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const override
  {
    m_delegate.handle().renderHighlight2D(
      renderContext, renderBatch, RotateHandle::HitArea::Center);
  }
};

class CenterHandlePointDelegate3D : public CenterHandlePointDelegate
{
public:
  using CenterHandlePointDelegate::CenterHandlePointDelegate;

  void renderHighlight(
    render::RenderContext& renderContext, render::RenderBatch& renderBatch) const override
  {
    m_delegate.handle().renderHighlight3D(
      renderContext, renderBatch, RotateHandle::HitArea::Center);
  }
};

// the point delegate base is constructed before the PointHandlePart base regardless of
// initializer list order, since base classes are constructed in declaration order -- so
// the reference PointHandlePart stores below always refers to a fully constructed object
class CenterHandlePart2D : private CenterHandlePointDelegate2D, public PointHandlePart
{
public:
  explicit CenterHandlePart2D(RotateHandleDelegate& delegate)
    : CenterHandlePointDelegate2D{delegate}
    , PointHandlePart{
        static_cast<CenterHandlePointDelegate2D&>(*this),
        centerHandleHitFilter(),
        acceptCenterHandleDrag}
  {
  }
};

class CenterHandlePart3D : private CenterHandlePointDelegate3D, public PointHandlePart
{
public:
  explicit CenterHandlePart3D(RotateHandleDelegate& delegate)
    : CenterHandlePointDelegate3D{delegate}
    , PointHandlePart{
        static_cast<CenterHandlePointDelegate3D&>(*this),
        centerHandleHitFilter(),
        acceptCenterHandleDrag}
  {
  }
};

} // namespace

void setHandleRenderOptions(
  const InputState& inputState,
  render::RenderContext& renderContext,
  const mdl::HitType::Type handleHitType)
{
  using namespace mdl::HitFilters;
  if (inputState.pickResult().first(type(handleHitType)).isMatch())
  {
    renderContext.setForceShowSelectionGuide();
  }
}

std::unique_ptr<ToolController> createRingHandlePart2D(RotateHandleDelegate& delegate)
{
  return std::make_unique<RingHandlePart2D>(delegate);
}

std::unique_ptr<ToolController> createRingHandlePart3D(RotateHandleDelegate& delegate)
{
  return std::make_unique<RingHandlePart3D>(delegate);
}

std::unique_ptr<ToolController> createCenterHandlePart2D(RotateHandleDelegate& delegate)
{
  return std::make_unique<CenterHandlePart2D>(delegate);
}

std::unique_ptr<ToolController> createCenterHandlePart3D(RotateHandleDelegate& delegate)
{
  return std::make_unique<CenterHandlePart3D>(delegate);
}

std::unique_ptr<ToolController> createPointHandlePart(
  PointHandleDelegate& delegate,
  mdl::HitFilter hitFilter,
  AcceptPointHandleDrag acceptDrag)
{
  return std::make_unique<PointHandlePart>(
    delegate, std::move(hitFilter), std::move(acceptDrag));
}

} // namespace tb::ui
