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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderService.h"
#include "Renderer/TextAnchor.h"
#include "View/HandleDragTracker.h"

#include "kdl/string_utils.h"

#include <vecmath/line.h>
#include <vecmath/plane.h>
#include <vecmath/vec.h>

#include <array>
#include <cassert>
#include <memory>
#include <type_traits>

namespace TrenchBroom {
namespace View {
enum class SnapMode
{
  /** Snap the delta between a previous and the proposed handle position. */
  Relative,
  /** Snap the proposed handle position to absolute values. */
  Absolute
};

/**
 * The move tracker's delegate. Provides callbacks which can be overridden to react to the different
 * events that can arise.
 */
struct MoveHandleDragTrackerDelegate {
  virtual ~MoveHandleDragTrackerDelegate() = default;

  /**
   * Called every time when a new proposed handle position is computed by the move tracker. This
   * function should be used to update the object being moved.
   *
   * If this function returns DragStatus::Continue, the current handle position is replaced by the
   * new proposed handle position. Return this value if the move can be applied without error. If
   * this function returns DragStatus::Deny, the current handle position is kept and the move
   * continues. Return this value if the move cannot be applied to the object being moved. If this
   * functions returns DragStatus::End, the end function is called and the move ends. Return this
   * value if the move cannot continue, i.e. because the object being moved was removed.
   *
   * @param inputState the current input state
   * @param dragState the last drag state
   * @param proposedHandlePosition handle position the next proposed handle position
   * @return a value of DragStatus that instructs the move tracker on how to continue
   */
  virtual DragStatus move(
    const InputState& inputState, const DragState& dragState,
    const vm::vec3& proposedHandlePosition) = 0;

  /**
   * Called when the move ends successfully, i.e. if the move callback returned DragStatus::End or
   * if the user released the mouse button. This function should commit any transactions.
   *
   * @param inputState the current input state
   * @param dragState the current drag state
   */
  virtual void end(const InputState& inputState, const DragState& dragState) = 0;

  /**
   * Called when the move is cancelled, i.e. if the user hit the escape key, or if the window loses
   * focus.
   *
   * @param dragState the current drag state
   */
  virtual void cancel(const DragState& dragState) = 0;

  /**
   * Calls if the mouse wheel is scrolled during a move.
   *
   * @param inputState the current input state
   * @param dragState the current drag state
   */
  virtual void mouseScroll(const InputState& inputState, const DragState& dragState);

  /**
   * Called once prior to rendering. The given input state and render context correspond to the view
   * being rendered, which may be a different view than the one in which the drag is taking place.
   *
   * @param inputState the current input state of the view being rendered
   * @param renderContext the render context of the view being rendered
   */
  virtual void setRenderOptions(
    const InputState& inputState, Renderer::RenderContext& renderContext) const;

  /**
   * Called once in a render pass. The given input state, render context and render batch correspond
   * to the view being rendered, which may be a different view than the one in which the drag is
   * taking place.
   *
   * @param inputState the current input state of the view being rendered
   * @param dragState the current drag state
   * @param renderContext the render context of the view being rendered
   * @param renderBatch the render batch of the view being rendered
   */
  virtual void render(
    const InputState& inputState, const DragState& dragState,
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const;

  /**
   * Returns a handle snapper. This is called once when the move start and when a modifier key is
   * pressed or released such that the move direction or snap mode changes.
   *
   * The passed snap mode can be ignored if only one snap mode is supported.
   *
   * @param inputState the current input state
   * @param snapMode the snap mode -- relative or absolute
   */
  virtual DragHandleSnapper makeDragHandleSnapper(
    const InputState& inputState, SnapMode snapMode) const = 0;
};

/**
 * A drag delegate that implements TrenchBroom's usual pattern for moving objects.
 *
 * This is a drag delegate for HandleDragTracker, but its behavior can be implemented and adapted by
 * providing it with its own delegate derived from MoveHandleDragTrackerDelegate.
 *
 * This drag delegate provides the following behavior:
 *
 * === In 3D Views ===
 *
 * By default, objects are dragged on a horizontal plane. Using the Alt key, the user can switch to
 * moving on a vertical line during a move. Holding shift can restrict a move to one axis in a
 * horizontal move. Holding the Ctrl (Cmd on macOS) switches the snap mode between relative and
 * absolute snapping, if supported by the delegate.
 *
 * === In 2D Views ===
 *
 * By default, objects are dragged on a plane that is orthogonal to the coordinate system axis that
 * best matches the camera's view direction. If this is the X axis, then the move happens on the Y/Z
 * plane, with the camera looking along the negative X axis. If the best matching axis is the Y
 * axis, then the move happens on the X/Z plane, with the camera looking along the positive Y axis.
 * Otherwise, the move happens on the X/Y plane, with the camera looking towards the negative Z
 * axis.
 *
 * +Z ^              +Z ^              +Y ^
 *    |                 | +Y              |
 *    |                 |/                |
 *    '------>          '------>          '------>
 *   /      +Y                +X         /      +X
 *  +X                                  +Z
 *
 * Like in the 3D views, holding shift can restrict the move to one axis, and Ctrl (Cmd on macOS)
 * switches between relative and absolute snapping if supported. The Alt key has no effect in 2D
 * views.
 *
 * In both view types, this delegate renders a move trace while a move is in progress. The move
 * trace is a set of lines parallel to the coordinate system axes (one for each axes). It
 * illustrates the total movement of the handle being moved.
 */
template <typename Delegate> class MoveHandleDragDelegate : public HandleDragTrackerDelegate {
private:
  static_assert(
    std::is_base_of_v<MoveHandleDragTrackerDelegate, Delegate>,
    "Delegate must extend MoveHandleDragTrackerDelegate");

  /** The different modes of moving. */
  enum class MoveMode
  {
    /** A vertical move (3D views only) */
    Vertical,
    /** A constricted move (move along only one axis of a horizontal plane) */
    Constricted,
    /** Default move mode (X/Y plane for 3D views, orthogonal plane for 2D views) */
    Default
  };

  Delegate m_delegate;

  MoveMode m_lastMoveMode{MoveMode::Default};
  SnapMode m_lastSnapMode{SnapMode::Relative};
  size_t m_lastConstrictedMoveAxis{0};

public:
  /**
   * Creates a new delegate for HandleDragTracker. The given delegate must extend
   * MoveHandleDragTrackerDelegate and is used to implement the actual effects and refine the
   * behavior of this delegate.
   */
  MoveHandleDragDelegate(Delegate delegate)
    : m_delegate{std::move(delegate)} {}

  /**
   * Called when the drag starts.
   *
   * Returns a handle proposer constructed according to the modifier keys held.
   */
  HandlePositionProposer start(
    const InputState& inputState, const vm::vec3& initialHandlePosition,
    const vm::vec3& handleOffset) override {
    const bool verticalMove = isVerticalMove(inputState);
    m_lastMoveMode = verticalMove ? MoveMode::Vertical : MoveMode::Default;
    m_lastSnapMode = snapMode(inputState);

    auto dragHandlePicker =
      verticalMove ? makeVerticalDragHandlePicker(inputState, initialHandlePosition, handleOffset)
                   : makeDefaultDragHandlePicker(inputState, initialHandlePosition, handleOffset);

    return makeHandlePositionProposer(
      std::move(dragHandlePicker), m_delegate.makeDragHandleSnapper(inputState, m_lastSnapMode));
  }

  /**
   * Forwards to the delegate's drag() function.
   */
  DragStatus drag(
    const InputState& inputState, const DragState& dragState,
    const vm::vec3& proposedHandlePosition) override {
    return m_delegate.move(inputState, dragState, proposedHandlePosition);
  }

  /**
   * Forwards to the delegate's end() function.
   */
  void end(const InputState& inputState, const DragState& dragState) override {
    m_delegate.end(inputState, dragState);
  }

  /**
   * Forwards to the delegate's cancel() function.
   */
  void cancel(const DragState& dragState) override { m_delegate.cancel(dragState); }

  /**
   * Updates the handle proposer function and the drag state according to the modifier keys held.
   */
  std::optional<UpdateDragConfig> modifierKeyChange(
    const InputState& inputState, const DragState& dragState) override {
    const auto nextMoveMode = moveMode(inputState, dragState);
    const auto nextSnapMode = snapMode(inputState);

    if (nextMoveMode != m_lastMoveMode) {
      const auto resetInitialHandlePosition =
        (m_lastMoveMode == MoveMode::Vertical ? ResetInitialHandlePosition::Reset
                                              : ResetInitialHandlePosition::Keep);

      if (nextMoveMode == MoveMode::Constricted) {
        m_lastConstrictedMoveAxis = vm::find_abs_max_component(
          dragState.currentHandlePosition - dragState.initialHandlePosition);
      }
      m_lastMoveMode = nextMoveMode;
      m_lastSnapMode = nextSnapMode;

      return UpdateDragConfig{
        makeHandlePositionProposer(
          makeDragHandlePicker(nextMoveMode, inputState, dragState),
          m_delegate.makeDragHandleSnapper(inputState, m_lastSnapMode)),
        resetInitialHandlePosition};
    } else if (nextSnapMode != m_lastSnapMode) {
      m_lastSnapMode = nextSnapMode;
      return UpdateDragConfig{
        makeHandlePositionProposer(
          makeDragHandlePicker(nextMoveMode, inputState, dragState),
          m_delegate.makeDragHandleSnapper(inputState, m_lastSnapMode)),
        ResetInitialHandlePosition::Keep};
    }

    return std::nullopt;
  }

  /**
   * Forwards to the delegate's mouseScroll() function.
   */
  void mouseScroll(const InputState& inputState, const DragState& dragState) override {
    m_delegate.mouseScroll(inputState, dragState);
  }

  /**
   * Forwards to the delegate's setRenderOptions() function.
   */
  void setRenderOptions(
    const InputState& inputState, Renderer::RenderContext& renderContext) const override {
    m_delegate.setRenderOptions(inputState, renderContext);
  }

  /**
   * Renders a move trace and then forwards to the delegate's render() function.
   */
  void render(
    const InputState& inputState, const DragState& dragState,
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) const override {
    if (dragState.currentHandlePosition != dragState.initialHandlePosition) {
      const auto vec = dragState.currentHandlePosition - dragState.initialHandlePosition;

      auto renderService = Renderer::RenderService{renderContext, renderBatch};
      renderService.setShowOccludedObjects();
      renderService.setBackgroundColor(pref(Preferences::InfoOverlayBackgroundColor));

      const auto stages = std::array<vm::vec3, 3>{
        vec * vm::vec3::pos_x(),
        vec * vm::vec3::pos_y(),
        vec * vm::vec3::pos_z(),
      };

      const auto colors = std::array<Color, 3>{
        pref(Preferences::XAxisColor),
        pref(Preferences::YAxisColor),
        pref(Preferences::ZAxisColor),
      };

      const auto lineWidths = std::array<float, 3>{
        m_lastMoveMode == MoveMode::Constricted && m_lastConstrictedMoveAxis == 0 ? 2.0f : 1.0f,
        m_lastMoveMode == MoveMode::Constricted && m_lastConstrictedMoveAxis == 1 ? 2.0f : 1.0f,
        m_lastMoveMode == MoveMode::Constricted && m_lastConstrictedMoveAxis == 2 ? 2.0f : 1.0f,
      };

      static const auto axisLabels = std::array<std::string, 3>{"X: ", "Y: ", "Z: "};

      auto lastPos = dragState.initialHandlePosition;
      for (size_t i = 0; i < 3; ++i) {
        const auto& stage = stages[i];
        if (stage != vm::vec3::zero()) {
          const auto curPos = lastPos + stage;
          const auto midPoint = (lastPos + curPos) / 2.0;
          const auto str = axisLabels[i] + kdl::str_to_string(stage[i]);

          renderService.setForegroundColor(colors[i]);
          renderService.setLineWidth(lineWidths[i]);
          renderService.renderLine(vm::vec3f{lastPos}, vm::vec3f{curPos});

          renderService.setForegroundColor(pref(Preferences::InfoOverlayTextColor));
          renderService.renderString(
            str, Renderer::SimpleTextAnchor{vm::vec3f{midPoint}, Renderer::TextAlignment::Bottom});

          lastPos = curPos;
        }
      }
    }

    m_delegate.render(inputState, dragState, renderContext, renderBatch);
  }

private:
  static MoveMode moveMode(const InputState& inputState, const DragState& dragState) {
    if (isVerticalMove(inputState)) {
      return MoveMode::Vertical;
    } else if (isConstrictedMove(inputState, dragState)) {
      return MoveMode::Constricted;
    } else {
      return MoveMode::Default;
    }
  }

  static bool isVerticalMove(const InputState& inputState) {
    const Renderer::Camera& camera = inputState.camera();
    return camera.perspectiveProjection() &&
           inputState.checkModifierKey(MK_Yes, ModifierKeys::MKAlt);
  }

  static bool isConstrictedMove(const InputState& inputState, const DragState& dragState) {
    if (inputState.checkModifierKey(MK_Yes, ModifierKeys::MKShift)) {
      const auto delta = dragState.currentHandlePosition - dragState.initialHandlePosition;
      return vm::get_abs_max_component(delta, 0) != vm::get_abs_max_component(delta, 1);
    }

    return false;
  }

  static SnapMode snapMode(const InputState& inputState) {
    return inputState.checkModifierKey(MK_Yes, ModifierKeys::MKCtrlCmd) ? SnapMode::Absolute
                                                                        : SnapMode::Relative;
  }

  static DragHandlePicker makeDragHandlePicker(
    const MoveMode moveMode, const InputState& inputState, const DragState& dragState) {
    switch (moveMode) {
      case MoveMode::Vertical:
        return makeVerticalDragHandlePicker(
          inputState, dragState.currentHandlePosition, dragState.handleOffset);
      case MoveMode::Constricted:
        return makeConstrictedDragHandlePicker(dragState);
      case MoveMode::Default:
        return makeDefaultDragHandlePicker(
          inputState, dragState.currentHandlePosition, dragState.handleOffset);
        switchDefault();
    }
  }

  static DragHandlePicker makeVerticalDragHandlePicker(
    [[maybe_unused]] const InputState& inputState, const vm::vec3& origin,
    const vm::vec3& handleOffset) {
    assert(inputState.camera().perspectiveProjection());

    const auto axis = vm::vec3::pos_z();
    return makeLineHandlePicker(vm::line3{origin, axis}, handleOffset);
  }

  static DragHandlePicker makeConstrictedDragHandlePicker(const DragState& dragState) {
    const auto delta = dragState.currentHandlePosition - dragState.initialHandlePosition;
    const auto axis = vm::get_abs_max_component_axis(delta);
    return makeLineHandlePicker(
      vm::line3{dragState.initialHandlePosition, axis}, dragState.handleOffset);
  }

  static DragHandlePicker makeDefaultDragHandlePicker(
    const InputState& inputState, const vm::vec3& origin, const vm::vec3& handleOffset) {
    const auto& camera = inputState.camera();
    const auto axis = camera.perspectiveProjection()
                        ? vm::vec3::pos_z()
                        : vm::vec3(vm::get_abs_max_component_axis(camera.direction()));
    return makePlaneHandlePicker(vm::plane3{origin, axis}, handleOffset);
  }
};

/**
 * Creates a new handle drag tracker that uses a MoveHandleDragDelegate, which in turn uses the
 * given delegate.
 */
template <typename Delegate>
std::unique_ptr<HandleDragTracker<MoveHandleDragDelegate<Delegate>>> createMoveHandleDragTracker(
  Delegate delegate, const InputState& inputState, const vm::vec3& initialHandlePosition,
  const vm::vec3& handleOffset) {
  return std::make_unique<HandleDragTracker<MoveHandleDragDelegate<Delegate>>>(
    MoveHandleDragDelegate{std::move(delegate)}, inputState, initialHandlePosition, handleOffset);
}

/**
 * Returns a relative or an absolute handle snapper according to the given snap mode.
 */
DragHandleSnapper makeDragHandleSnapperFromSnapMode(const Grid& grid, SnapMode snapMode);
} // namespace View
} // namespace TrenchBroom
