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

#include "Renderer/OrthographicCamera.h"
#include "Renderer/PerspectiveCamera.h"
#include "View/MoveHandleDragTracker.h"
#include "View/PickRequest.h"

#include "vm/approx.h"
#include "vm/ray.h"
#include "vm/ray_io.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include "Catch2.h"

namespace vm
{
template <>
class approx<TrenchBroom::View::DragState>
{
private:
  using DragState = TrenchBroom::View::DragState;
  DragState m_value;
  FloatType m_epsilon;

public:
  explicit approx(const DragState value, const FloatType epsilon)
    : m_value(value)
    , m_epsilon(epsilon)
  {
    assert(epsilon >= FloatType(0));
  }
  explicit approx(const DragState value)
    : approx(value, vm::constants<FloatType>::almost_zero())
  {
  }

  friend bool operator==(const DragState& lhs, const approx<DragState>& rhs)
  {
    return lhs.initialHandlePosition
             == approx<vec3>{rhs.m_value.initialHandlePosition, rhs.m_epsilon}
           && lhs.currentHandlePosition
                == approx<vec3>{rhs.m_value.currentHandlePosition, rhs.m_epsilon}
           && lhs.handleOffset == approx<vec3>{rhs.m_value.handleOffset, rhs.m_epsilon};
  }

  friend bool operator==(const approx<DragState>& lhs, const DragState& rhs)
  {
    return rhs == lhs;
  }

  friend bool operator!=(const DragState& lhs, const approx<DragState>& rhs)
  {
    return !(lhs == rhs);
  }

  friend bool operator!=(const approx<DragState>& lhs, const DragState& rhs)
  {
    return !(lhs == rhs);
  }

  friend std::ostream& operator<<(std::ostream& str, const approx<DragState>& a)
  {
    str << a.m_value;
    return str;
  }
};
} // namespace vm

namespace TrenchBroom
{
namespace View
{
template <
  typename Move,
  typename End,
  typename Cancel,
  typename Render,
  typename MakeHandleSnapper>
struct TestDelegate : public MoveHandleDragTrackerDelegate
{
  Move m_move;
  End m_end;
  Cancel m_cancel;
  Render m_render;
  MakeHandleSnapper m_makeDragHandleSnapper;

  TestDelegate(
    Move i_move,
    End i_end,
    Cancel i_cancel,
    Render i_render,
    MakeHandleSnapper i_makeDragHandleSnapper)
    : m_move{std::move(i_move)}
    , m_end{std::move(i_end)}
    , m_cancel{std::move(i_cancel)}
    , m_render{std::move(i_render)}
    , m_makeDragHandleSnapper{std::move(i_makeDragHandleSnapper)}
  {
  }

  DragStatus move(
    const InputState& inputState,
    const DragState& dragState,
    const vm::vec3& proposedHandlePosition) override
  {
    return m_move(inputState, dragState, proposedHandlePosition);
  }

  void end(const InputState& inputState, const DragState& dragState) override
  {
    m_end(inputState, dragState);
  }

  void cancel(const DragState& dragState) override { m_cancel(dragState); }

  void render(
    const InputState& inputState,
    const DragState& dragState,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) const override
  {
    m_render(inputState, dragState, renderContext, renderBatch);
  }

  DragHandleSnapper makeDragHandleSnapper(
    const InputState& inputState, const SnapMode snapMode) const override
  {
    return m_makeDragHandleSnapper(inputState, snapMode);
  }
};

static auto makeMoveTracker(
  const InputState& inputState,
  const vm::vec3& initialHandlePosition,
  const vm::vec3& handleOffset)
{
  auto move = [&](
                const InputState&,
                const DragState&,
                const vm::vec3& /* proposedHandlePosition */) -> DragStatus {
    return DragStatus::Continue;
  };
  auto end = [](const InputState&, const DragState&) {};
  auto cancel = [](const DragState&) {};
  auto render = [](
                  const InputState&,
                  const DragState&,
                  Renderer::RenderContext&,
                  Renderer::RenderBatch&) {};
  auto makeDragHandleSnapper = [](const InputState&, const SnapMode) {
    return [](const InputState&, const DragState&, const vm::vec3& currentHitPosition) {
      return currentHitPosition;
    };
  };

  auto delegate = TestDelegate<
    decltype(move),
    decltype(end),
    decltype(cancel),
    decltype(render),
    decltype(makeDragHandleSnapper)>{
    std::move(move),
    std::move(end),
    std::move(cancel),
    std::move(render),
    std::move(makeDragHandleSnapper)};

  return HandleDragTracker<MoveHandleDragDelegate<decltype(delegate)>>{
    MoveHandleDragDelegate{std::move(delegate)},
    inputState,
    initialHandlePosition,
    handleOffset};
}

static auto makeInputState(
  const vm::vec3& rayOrigin,
  const vm::vec3& rayDirection,
  Renderer::Camera& camera,
  ModifierKeyState modifierKeys = ModifierKeys::MKNone)
{
  auto inputState = InputState{};
  inputState.setPickRequest(
    PickRequest{vm::ray3{rayOrigin, vm::normalize(rayDirection)}, camera});
  inputState.setModifierKeys(modifierKeys);
  return inputState;
}

TEST_CASE("MoveDragTracker.constructor")
{
  constexpr auto initialHandlePosition = vm::vec3{0, 64, 0};
  constexpr auto initialHitPoint = initialHandlePosition;
  constexpr auto handleOffset = initialHandlePosition - initialHitPoint;

  GIVEN("A 3D camera")
  {
    auto camera3d = Renderer::PerspectiveCamera{};

    WHEN("A tracker is created without any modifier keys pressed")
    {
      auto tracker = makeMoveTracker(
        makeInputState(vm::vec3{0, 0, 64}, vm::vec3{0, 1, -1}, camera3d),
        initialHandlePosition,
        initialHitPoint);

      THEN("The tracker has set the initial and current handle positions correctly")
      {
        CHECK(
          tracker.dragState()
          == DragState{initialHandlePosition, initialHandlePosition, handleOffset});

        AND_THEN("The tracker is using a default hit finder")
        {
          // we check this indirectly by observing how the move handle position changes
          // when dragging
          REQUIRE(tracker.drag(
            makeInputState(vm::vec3{16, 16, 64}, vm::vec3{0, 1, -1}, camera3d)));
          CHECK(
            tracker.dragState()
            == DragState{initialHandlePosition, vm::vec3{16, 80, 0}, handleOffset});
        }
      }
    }

    WHEN("A tracker is created with the alt modifier pressed")
    {
      auto tracker = makeMoveTracker(
        makeInputState(
          vm::vec3{0, 0, 64}, vm::vec3{0, 1, -1}, camera3d, ModifierKeys::MKAlt),
        initialHandlePosition,
        initialHitPoint);

      THEN("The tracker is using a vertical hit finder")
      {
        // we check this indirectly by observing how the move handle position changes when
        // dragging
        REQUIRE(tracker.drag(
          makeInputState(vm::vec3{16, 16, 64}, vm::vec3{0, 1, -1}, camera3d)));
        CHECK(
          tracker.dragState()
          == vm::approx{
            DragState{initialHandlePosition, vm::vec3{0, 64, 16}, handleOffset}});
      }
    }
  }

  GIVEN("A 2D camera")
  {
    auto camera2d = Renderer::OrthographicCamera{};
    camera2d.moveTo(vm::vec3f{0, 0, 64});
    camera2d.lookAt(vm::vec3f{0, 0, -1}, vm::vec3f{0, 1, 0});

    WHEN("A tracker is created without any modifier keys pressed")
    {
      auto tracker = makeMoveTracker(
        makeInputState(vm::vec3{0, 64, 64}, vm::vec3{0, 0, -1}, camera2d),
        initialHandlePosition,
        initialHitPoint);

      THEN("The tracker has set the initial and current handle positions correctly")
      {
        CHECK(
          tracker.dragState()
          == DragState{initialHandlePosition, initialHandlePosition, handleOffset});

        AND_THEN("The tracker is using a default hit finder")
        {
          // we check this indirectly by observing how the move handle position changes
          // when dragging
          REQUIRE(tracker.drag(
            makeInputState(vm::vec3{16, 80, 64}, vm::vec3{0, 0, -1}, camera2d)));
          CHECK(
            tracker.dragState()
            == DragState{initialHandlePosition, vm::vec3{16, 80, 0}, handleOffset});
        }
      }
    }

    WHEN("A tracker is created with the alt modifier pressed")
    {
      auto tracker = makeMoveTracker(
        makeInputState(
          vm::vec3{0, 0, 64}, vm::vec3{0, 1, -1}, camera2d, ModifierKeys::MKAlt),
        initialHandlePosition,
        initialHitPoint);

      THEN("The tracker is using a default hit finder")
      {
        // we check this indirectly by observing how the move handle position changes when
        // dragging
        REQUIRE(tracker.drag(
          makeInputState(vm::vec3{16, 80, 64}, vm::vec3{0, 0, -1}, camera2d)));
        CHECK(
          tracker.dragState()
          == DragState{initialHandlePosition, vm::vec3{16, 80, 0}, handleOffset});
      }
    }
  }
}

TEST_CASE("MoveDragTracker.modifierKeyChange")
{
  constexpr auto initialHandlePosition = vm::vec3{0, 64, 0};
  constexpr auto initialHitPoint = initialHandlePosition;
  constexpr auto handleOffset = initialHandlePosition - initialHitPoint;

  GIVEN("A tracker created with a 3D camera")
  {
    auto camera3d = Renderer::PerspectiveCamera{};
    auto tracker = makeMoveTracker(
      makeInputState(vm::vec3{0, 0, 64}, vm::vec3{0, 1, -1}, camera3d),
      initialHandlePosition,
      initialHitPoint);
    REQUIRE(
      tracker.dragState()
      == DragState{initialHandlePosition, initialHandlePosition, handleOffset});

    WHEN("The alt modifier is pressed")
    {
      tracker.modifierKeyChange(makeInputState(
        vm::vec3{0, 0, 64}, vm::vec3{0, 1, -1}, camera3d, ModifierKeys::MKAlt));

      THEN("The tracker switches to a vertical hit finder")
      {
        // we check this indirectly by observing how the move handle position changes when
        // dragging
        REQUIRE(tracker.drag(
          makeInputState(vm::vec3{16, 16, 64}, vm::vec3{0, 1, -1}, camera3d)));
        CHECK(
          tracker.dragState()
          == vm::approx{
            DragState{initialHandlePosition, vm::vec3{0, 64, 16}, handleOffset}});
      }

      AND_WHEN("The alt modifier is released")
      {
        tracker.modifierKeyChange(
          makeInputState(vm::vec3{0, 0, 64}, vm::vec3{0, 1, -1}, camera3d));

        THEN("The tracker switches to a default hit finder")
        {
          // we check this indirectly by observing how the move handle position changes
          // when dragging
          REQUIRE(tracker.drag(
            makeInputState(vm::vec3{16, 16, 64}, vm::vec3{0, 1, -1}, camera3d)));
          CHECK(
            tracker.dragState()
            == DragState{vm::vec3{0, 64, 0}, vm::vec3{16, 80, 0}, handleOffset});
        }
      }
    }

    WHEN("The shift modifier is pressed before the handle is moved")
    {
      tracker.modifierKeyChange(makeInputState(
        vm::vec3{0, 0, 64}, vm::vec3{0, 1, -1}, camera3d, ModifierKeys::MKShift));

      THEN("The tracker still has a default hit finder")
      {
        // we check this indirectly by observing how the move handle position changes when
        // dragging
        REQUIRE(tracker.drag(
          makeInputState(vm::vec3{16, 16, 64}, vm::vec3{0, 1, -1}, camera3d)));
        CHECK(
          tracker.dragState()
          == DragState{initialHandlePosition, vm::vec3{16, 80, 0}, handleOffset});
      }
    }

    WHEN("The shift modifier is pressed after the handle is moved diagonally")
    {
      REQUIRE(
        tracker.drag(makeInputState(vm::vec3{16, 16, 64}, vm::vec3{0, 1, -1}, camera3d)));
      REQUIRE(
        tracker.dragState()
        == DragState{initialHandlePosition, vm::vec3{16, 80, 0}, handleOffset});

      tracker.modifierKeyChange(makeInputState(
        vm::vec3{16, 16, 64}, vm::vec3{0, 1, -1}, camera3d, ModifierKeys::MKShift));

      THEN("The tracker still has a default hit finder")
      {
        // we check this indirectly by observing how the move handle position changes when
        // dragging
        CHECK(
          tracker.dragState()
          == DragState{initialHandlePosition, vm::vec3{16, 80, 0}, handleOffset});
      }
    }

    WHEN("The shift modifier is pressed after the handle is moved non-diagonally")
    {
      REQUIRE(
        tracker.drag(makeInputState(vm::vec3{16, 32, 64}, vm::vec3{0, 1, -1}, camera3d)));
      REQUIRE(
        tracker.dragState()
        == DragState{initialHandlePosition, vm::vec3{16, 96, 0}, handleOffset});

      tracker.modifierKeyChange(makeInputState(
        vm::vec3{16, 32, 64}, vm::vec3{0, 1, -1}, camera3d, ModifierKeys::MKShift));

      THEN("The tracker has a constricted hit finder")
      {
        // we check this indirectly by observing how the move handle position changes when
        // dragging
        CHECK(
          tracker.dragState()
          == DragState{vm::vec3{0, 64, 0}, vm::vec3{0, 96, 0}, handleOffset});
      }

      AND_WHEN("The shift modifier is released")
      {
        tracker.modifierKeyChange(
          makeInputState(vm::vec3{16, 32, 64}, vm::vec3{0, 1, -1}, camera3d));

        THEN("The tracker switches back to a default hit finder")
        {
          // we check this indirectly by observing how the move handle position changes
          // when dragging
          CHECK(
            tracker.dragState()
            == DragState{vm::vec3{0, 64, 0}, vm::vec3{16, 96, 0}, handleOffset});
        }
      }
    }
  }

  GIVEN("A tracker created with a 3D camera")
  {
    auto camera2d = Renderer::OrthographicCamera{};
    camera2d.moveTo(vm::vec3f{0, 0, 64});
    camera2d.lookAt(vm::vec3f{0, 0, -1}, vm::vec3f{0, 1, 0});

    auto tracker = makeMoveTracker(
      makeInputState(vm::vec3{0, 0, 64}, vm::vec3{0, 1, -1}, camera2d),
      initialHandlePosition,
      initialHitPoint);
    REQUIRE(
      tracker.dragState()
      == DragState{initialHandlePosition, initialHandlePosition, handleOffset});

    WHEN("The alt modifier is pressed")
    {
      tracker.modifierKeyChange(makeInputState(
        vm::vec3{0, 64, 64}, vm::vec3{0, 0, -1}, camera2d, ModifierKeys::MKAlt));

      THEN("The tracker does not change the hit finder")
      {
        // we check this indirectly by observing how the move handle position changes when
        // dragging
        REQUIRE(tracker.drag(
          makeInputState(vm::vec3{16, 80, 64}, vm::vec3{0, 0, -1}, camera2d)));
        CHECK(
          tracker.dragState()
          == DragState{initialHandlePosition, vm::vec3{16, 80, 0}, handleOffset});
      }
    }
  }
}
} // namespace View
} // namespace TrenchBroom
