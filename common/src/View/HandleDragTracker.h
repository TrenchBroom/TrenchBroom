/*
 Copyright (C) 2021 Kristian Duske

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

#pragma once

#include "FloatType.h"
#include "Model/HitFilter.h"
#include "Renderer/Camera.h"
#include "View/DragTracker.h"
#include "View/InputState.h"

#include "kdl/reflection_decl.h"

#include "vm/forward.h"
#include "vm/vec.h"

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>

namespace TrenchBroom
{
namespace View
{
class Grid;

/**
 * The state of a drag.
 */
struct DragState
{
  vm::vec3 initialHandlePosition;
  vm::vec3 currentHandlePosition;
  vm::vec3 handleOffset;

  kdl_reflect_decl(DragState, initialHandlePosition, currentHandlePosition, handleOffset);
};

/**
 * Maps the input state and the drag state to a new proposed handle position.
 *
 * If this function returns nullopt, the drag will continue, but the delegate's drag
 * callback will not be called.
 */
using HandlePositionProposer =
  std::function<std::optional<vm::vec3>(const InputState&, const DragState&)>;

/**
 * Controls whether the initial handle position should be updated to the current handle
 * position.
 */
enum class ResetInitialHandlePosition
{
  Keep,
  Reset
};

/**
 * Returned from the delegate's modifierKeyChange callback. The tracker's handle position
 * mapping function is updated with proposeHandlePosition, and if
 * resetInitialHandlePosition is set to Reset, the drag state's initial handle position is
 * updated to the current handle position.
 */
struct UpdateDragConfig
{
  HandlePositionProposer proposeHandlePosition;
  ResetInitialHandlePosition resetInitialHandlePosition;
};

/**
 * The status of a drag. This is returned from a handle drag tracker's delegate when it
 * reacts to a drag event.
 */
enum class DragStatus
{
  /** The drag should continue. */
  Continue,
  /** The drag should continue, but the current event could not be applied to the object
   * being dragged. The current handle position is not updated in this case. */
  Deny,
  /** The drag should be cancelled. */
  End
};

/**
 * The drag tracker's delegate. This provides callbacks which can be overridden to react
 * to the different events that can arise during a drag.
 */
struct HandleDragTrackerDelegate
{
  virtual ~HandleDragTrackerDelegate() = default;

  /**
   * Called once when the drag starts. Use this function to start a transaction if
   * necessary.
   *
   * @param inputState the current input state at the start of the drag
   * @param initialHandlePosition the initial handle position (as passed to the drag
   * tracker's constructor)
   * @param initialHitPoint the hit point (as passed to the drag tracker's constructor)
   * @return a function that maps the input state and drag state to a handle position
   */
  virtual HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3& initialHandlePosition,
    const vm::vec3& initialHitPoint) = 0;

  /**
   * Called every time when a new proposed handle position is computed by the drag
   * tracker. This function should be used to update the object being dragged.
   *
   * If this function returns DragStatus::Continue, the current handle position is
   * replaced by the new proposed handle position. Return this value if the drag can be
   * applied without error. If this function returns DragStatus::Deny, the current handle
   * position is kept and the drag continues. Return this value if the drag cannot be
   * applied to the object being dragged. If this functions returns DragStatus::End, the
   * end function is called and the drag ends. Return this value if the drag cannot
   * continue, i.e. because the object being dragged was removed.
   *
   * @param inputState the current input state
   * @param dragState the last drag state
   * @param proposedHandlePosition the next proposed handle position
   * @return a value of DragStatus that instructs the drag tracker on how to continue with
   * the drag
   */
  virtual DragStatus drag(
    const InputState& inputState,
    const DragState& dragState,
    const vm::vec3& proposedHandlePosition) = 0;

  /**
   * Called when the drag ends successfully, i.e. if the drag callback returned
   * DragStatus::End or if the user released the mouse button. This function should commit
   * any transactions.
   *
   * @param inputState the current input state
   * @param dragState the current drag state
   */
  virtual void end(const InputState& inputState, const DragState& dragState) = 0;

  /**
   * Called when the drag is cancelled, i.e. if the user hit the escape key, or if the
   * window loses focus.
   *
   * @param dragState the current drag state
   */
  virtual void cancel(const DragState& dragState) = 0;

  /**
   * Called when any modifier key is pressed or released. Can be overridden to update the
   * function that the drag tracker uses to compute proposed handle positions. For
   * example, a tool might change how the handle position is snapped mid drag when a
   * modifier key is pressed.
   *
   * If this function returns nullopt, the current handle proposer and initial handle
   * position is kept.
   *
   * @param inputState the current inputState
   * @param dragState the current drag state
   * @return a struct with a new function to propose handle positions, and an instruction
   * whether or not to update the initial handle position
   */
  virtual std::optional<UpdateDragConfig> modifierKeyChange(
    const InputState& inputState, const DragState& dragState);

  /**
   * Calls if the mouse wheel is scrolled during a drag.
   *
   * @param inputState the current input state
   * @param dragState the current drag state
   */
  virtual void mouseScroll(const InputState& inputState, const DragState& dragState);

  /**
   * Called once prior to rendering. The given input state and render context correspond
   * to the view being rendered, which may be a different view than the one in which the
   * drag is taking place.
   *
   * @param inputState the current input state of the view being rendered
   * @param renderContext the render context of the view being rendered
   */
  virtual void setRenderOptions(
    const InputState& inputState, Renderer::RenderContext& renderContext) const;

  /**
   * Called once in a render pass. The given input state, render context and render batch
   * correspond to the view being rendered, which may be a different view than the one in
   * which the drag is taking place.
   *
   * @param inputState the current input state of the view being rendered
   * @param dragState the current drag state
   * @param renderContext the render context of the view being rendered
   * @param renderBatch the render batch of the view being rendered
   */
  virtual void render(
    const InputState& inputState,
    const DragState& dragState,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) const;
};

/**
 * A drag tracker that supports dragging handles.
 *
 * In this context, a handle is a 3D point. This drag tracker keeps track of the initial
 * handle position and the current handle position. The initial handle position is the
 * position that was passed to the constructor. It can be updated if the drag mode changes
 * in response to a modifier key change.
 *
 * The drag tracker also keeps track of a handle offset. This corresponds to the distance
 * between the handle position and the hit point where the pick ray initially intersected
 * the handle's representation on the screen. In case of a point handle, the hit point is
 * a point on the spherical representation of the handle. It holds that handle offset =
 * handle position - hit point The handle offset is passed to the HandlePositionProposer
 * function that the tracker uses to compute a new handle position from the current input
 * state.
 *
 * The current handle position updates in response to calls to drag() or a modifier key
 * change.
 *
 * The delegate's start function is called once when this drag tracker is constructed. It
 * must return the handle proposer function to use initially. The delegate's
 * modifierKeyChange function can optionally return a new handle proposer function and it
 * can instruct the tracker to update the initial handle position. This can be used to
 * change the characteristics of the drag in response to a modifier key change. For
 * example, in a 3D view, the user may hold a modifier key to switch between dragging
 * horizontally and vertically.
 */
template <typename Delegate>
class HandleDragTracker : public DragTracker
{
private:
  enum class IdenticalPositionPolicy
  {
    SkipDrag,
    ForceDrag
  };

  static_assert(
    std::is_base_of_v<HandleDragTrackerDelegate, Delegate>,
    "Delegate must extend HandleDragTrackerDelegate");
  Delegate m_delegate;

  DragState m_dragState;
  HandlePositionProposer m_proposeHandlePosition;

public:
  /**
   * Creates a new handle drag tracker with the given delegate.
   */
  HandleDragTracker(
    Delegate delegate,
    const InputState& inputState,
    const vm::vec3& initialHandlePosition,
    const vm::vec3& initialHitPoint)
    : m_delegate{std::move(delegate)}
    , m_dragState{initialHandlePosition, initialHandlePosition, initialHandlePosition - initialHitPoint}
    , m_proposeHandlePosition{m_delegate.start(
        inputState, m_dragState.initialHandlePosition, m_dragState.handleOffset)}
  {
  }

  /**
   * Returns the current drag state. Exposed for testing.
   */
  const DragState& dragState() const { return m_dragState; }

  /**
   * React to modifier key changes. This is delegated to the delegate, and if it returns a
   * new handle position proposer function, the drag tracker's proposer function is
   * replaced. Optionally, the initial handle position is updated according to the value
   * of the returned ResetInitialHandlePosition value.
   *
   * If a new proposer function is returned by the delegate, it is called with the current
   * drag state and drag() is called with the new proposed handle position.
   */
  void modifierKeyChange(const InputState& inputState) override
  {
    if (auto dragConfig = m_delegate.modifierKeyChange(inputState, m_dragState))
    {
      if (dragConfig->resetInitialHandlePosition == ResetInitialHandlePosition::Reset)
      {
        const auto newInitialHandlePosition =
          dragConfig->proposeHandlePosition(inputState, m_dragState);
        if (!newInitialHandlePosition)
        {
          return;
        }

        m_dragState.initialHandlePosition = *newInitialHandlePosition;
      }

      m_proposeHandlePosition = std::move(dragConfig->proposeHandlePosition);

      assertResult(drag(inputState, IdenticalPositionPolicy::ForceDrag));
    }
  }

  /**
   * Forward the scroll event to the delegate.
   */
  void mouseScroll(const InputState& inputState) override
  {
    m_delegate.mouseScroll(inputState, m_dragState);
  }

  /**
   * Called when the mouse is moved during a drag. Delegates to the delegate to apply
   * changes to the objects being dragged.
   *
   * Returns true to indicate succes. If this function returns false, the drag ends and
   * end() is called.
   */
  bool drag(const InputState& inputState) override
  {
    return drag(inputState, IdenticalPositionPolicy::SkipDrag);
  }

  /**
   * Called when the drag ends normally (e.g. by releasing a mouse button) or if drag()
   * returns false. The delegate should commit any changes made in result of the drag.
   */
  void end(const InputState& inputState) override
  {
    m_delegate.end(inputState, m_dragState);
  }

  /**
   * Called when the drag ends abnormally (e.g. by hitting escape during a drag). The
   * delegate should undo any changes made in result of the drag.
   */
  void cancel() override { m_delegate.cancel(m_dragState); }

  /**
   * Called during the drag to allow the delegate to set render options.
   */
  void setRenderOptions(
    const InputState& inputState, Renderer::RenderContext& renderContext) const override
  {
    m_delegate.setRenderOptions(inputState, renderContext);
  }

  /**
   * Called during the drag to allow the delegate to render into the corresponding view.
   */
  void render(
    const InputState& inputState,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) const override
  {
    m_delegate.render(inputState, m_dragState, renderContext, renderBatch);
  }

private:
  bool drag(
    const InputState& inputState, const IdenticalPositionPolicy identicalPositionPolicy)
  {
    const auto proposedHandlePosition = m_proposeHandlePosition(inputState, m_dragState);
    if (
      !proposedHandlePosition
      || (*proposedHandlePosition == m_dragState.currentHandlePosition && identicalPositionPolicy == IdenticalPositionPolicy::SkipDrag))
    {
      return true;
    }

    const auto dragResult =
      m_delegate.drag(inputState, m_dragState, *proposedHandlePosition);
    if (dragResult == DragStatus::End)
    {
      return false;
    }

    if (dragResult == DragStatus::Continue)
    {
      m_dragState.currentHandlePosition = *proposedHandlePosition;
    }

    return true;
  }
};

/**
 * Creates a new handle drag tracker.
 *
 * @param delegate the delegate to use
 * @param inputState the current input state
 * @param initialHandlePosition the initial handle position
 * @param initialHitPoint the initial hit point
 */
template <typename Delegate>
std::unique_ptr<HandleDragTracker<Delegate>> createHandleDragTracker(
  Delegate delegate,
  const InputState& inputState,
  const vm::vec3& initialHandlePosition,
  const vm::vec3& initialHitPoint)
{
  return std::make_unique<HandleDragTracker<Delegate>>(
    std::move(delegate), inputState, initialHandlePosition, initialHitPoint);
}

/**
 * Picks a handle position for the current input state. The returned point is a handle
 * position and not a hit position, so it must be corrected by the handle offset if the
 * offset is not zero.
 */
using DragHandlePicker = std::function<std::optional<vm::vec3>(const InputState&)>;

/**
 * Returns a drag handle picker that picks a point on a line. The given line should be
 * based on the initial handle position and not the hit point, i.e., it should contain the
 * handle position.
 */
DragHandlePicker makeLineHandlePicker(
  const vm::line3& line, const vm::vec3& handleOffset);

/**
 * Returns a drag handle picker that picks a point on a plane. The given plane should be
 * based on the initial handle position and not the hit point, i.e. it should contain the
 * handle position.
 */
DragHandlePicker makePlaneHandlePicker(
  const vm::plane3& plane, const vm::vec3& handleOffset);

/**
 * Returns a drag handle picker that picks a point on a circle. The distance of the
 * returned point and the given center is always equal to the given adius.
 */
DragHandlePicker makeCircleHandlePicker(
  const vm::vec3& center,
  const vm::vec3& normal,
  FloatType radius,
  const vm::vec3& handleOffset);

/**
 * Returns a drag handle picker that picks a point on a surface. The surface is determined
 * by the given hit filter. It is used to find a hit in the input state's pick result, and
 * that hit's hit point is returned, corrected by the given handle offset.
 */
DragHandlePicker makeSurfaceHandlePicker(
  Model::HitFilter filter, const vm::vec3& handleOffset);

/**
 * Snaps a proposed handle position to its final position.
 */
using DragHandleSnapper = std::function<std::optional<vm::vec3>(
  const InputState&, const DragState&, const vm::vec3& /* proposedHandlePosition */)>;

/**
 * Returns a snapper function that just returns the proposed handle position.
 */
DragHandleSnapper makeIdentityHandleSnapper();

/**
 * Returns a snapper function that snaps the proposed handle position such that the
 * distance to the initial handle position (passed in the drag state) is snapped to the
 * grid.
 */
DragHandleSnapper makeRelativeHandleSnapper(const Grid& grid);

/**
 * Returns a snapper function that snaps the proposed handle position to the grid.
 */
DragHandleSnapper makeAbsoluteHandleSnapper(const Grid& grid);

/**
 * Returns a snapper function that snaps the proposed handle position to the closest point
 * on the given line such that the distance between that point and the initial handle
 * position is a multiple of the grid size. If the initial handle position is not on the
 * line itself, it is orthogonally projected onto the line.
 */
DragHandleSnapper makeRelativeLineHandleSnapper(const Grid& grid, const vm::line3& line);

/**
 * Returns a snapper function that snaps the proposed handle position to the closest point
 * on the given line such that any of its components is a multiple of the grid size.
 */
DragHandleSnapper makeAbsoluteLineHandleSnapper(const Grid& grid, const vm::line3& line);

/**
 * Returns a snapper function that snaps the proposed handle position to a point on a
 * circle such that the angle between the vectors A and B is a multiple of the given snap
 * angle. Thereby, vector A = proposed handle position - center and vector B = initial
 * handle position - center.
 *
 *         *   *
 *      *         o proposed handle position
 *     *     |-----o snapped handle position
 *      *    |    *
 *         * o *
 *           initial handle position
 *
 * In this example, the snap angle is 45, so the angle between the initial handle position
 * and the proposed handle position is snapped to 90°.
 */
DragHandleSnapper makeCircleHandleSnapper(
  const Grid& grid,
  FloatType snapAngle,
  const vm::vec3& center,
  const vm::vec3& normal,
  FloatType radius);

/**
 * Returns a handle proposer that proposes the position of the first brush face hit in the
 * current pick result, snapped to the grid projected onto that face.
 */
HandlePositionProposer makeBrushFaceHandleProposer(const Grid& grid);

/**
 * Composes a drag handle picker and a drag handle snapper into one function.
 */
HandlePositionProposer makeHandlePositionProposer(
  DragHandlePicker pickHandlePosition, DragHandleSnapper snapHandlePosition);
} // namespace View
} // namespace TrenchBroom
