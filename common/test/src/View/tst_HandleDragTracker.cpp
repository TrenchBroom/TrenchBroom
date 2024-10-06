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

#include "View/Grid.h"
#include "View/HandleDragTracker.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/PickResult.h"
#include "render/OrthographicCamera.h"

#include "vm/approx.h"

#include <tuple>
#include <vector>

#include "Catch2.h" // IWYU pragma: keep

namespace tb::View
{
namespace
{
struct TestDelegateData
{
  std::vector<std::tuple<vm::vec3d, vm::vec3d>> initializeArguments;
  HandlePositionProposer initialGetHandlePositionToReturn;

  std::vector<std::tuple<DragState, vm::vec3d>> dragArguments;
  DragStatus dragStatusToReturn{DragStatus::Continue};

  std::vector<DragState> endArguments;
  std::vector<DragState> cancelArguments;

  std::vector<DragState> modifierKeyChangeArguments;
  std::optional<UpdateDragConfig> updateDragConfigToReturn;

  std::vector<DragState> mouseScrollArguments;

  explicit TestDelegateData(HandlePositionProposer i_initialGetHandlePositionToReturn)
    : initialGetHandlePositionToReturn{std::move(i_initialGetHandlePositionToReturn)}
  {
  }
};

struct TestDelegate : public HandleDragTrackerDelegate
{
  TestDelegateData& data;

  explicit TestDelegate(TestDelegateData& i_data)
    : data{i_data}
  {
  }

  HandlePositionProposer start(
    const InputState&,
    const vm::vec3d& initialHandlePosition,
    const vm::vec3d& handleOffset) override
  {
    data.initializeArguments.emplace_back(initialHandlePosition, handleOffset);
    return data.initialGetHandlePositionToReturn;
  }

  DragStatus update(
    const InputState&,
    const DragState& dragState,
    const vm::vec3d& proposedHandlePosition) override
  {
    data.dragArguments.emplace_back(dragState, proposedHandlePosition);
    return data.dragStatusToReturn;
  }

  void end(const InputState&, const DragState& dragState) override
  {
    data.endArguments.emplace_back(dragState);
  }

  void cancel(const DragState& dragState) override
  {
    data.cancelArguments.emplace_back(dragState);
  }

  std::optional<UpdateDragConfig> modifierKeyChange(
    const InputState&, const DragState& dragState) override
  {
    data.modifierKeyChangeArguments.emplace_back(dragState);
    return data.updateDragConfigToReturn;
  }

  void mouseScroll(const InputState&, const DragState& dragState) override
  {
    data.mouseScrollArguments.emplace_back(dragState);
  }
};

auto makeHandleTracker(
  TestDelegateData& data,
  const vm::vec3d& initialHandlePosition,
  const vm::vec3d& handleOffset)
{
  return HandleDragTracker<TestDelegate>{
    TestDelegate{data}, InputState{}, initialHandlePosition, handleOffset};
}

} // namespace

TEST_CASE("RestrictedDragTracker.constructor")
{
  GIVEN("A delegate")
  {
    const auto initialHandlePosition = vm::vec3d{1, 1, 1};
    const auto initialHitPoint = vm::vec3d{1, 1, 0};
    const auto handleOffset = initialHandlePosition - initialHitPoint;

    auto data = TestDelegateData{makeHandlePositionProposer(
      // always returns the same handle position
      [](const auto&) { return vm::vec3d{2, 2, 2}; },
      makeIdentityHandleSnapper())};

    auto tracker = makeHandleTracker(data, initialHandlePosition, initialHitPoint);

    THEN("The initial handle position was passed to initialize")
    {
      CHECK(
        data.initializeArguments
        == std::vector<std::tuple<vm::vec3d, vm::vec3d>>{
          {initialHandlePosition, handleOffset}});

      AND_THEN(
        "The initial handle position is passed to drag for the initial and the last "
        "handle "
        "position")
      {
        tracker.update(InputState{});

        CHECK(
          data.dragArguments
          == std::vector<std::tuple<DragState, vm::vec3d>>{
            {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 1}},
             vm::vec3d{2, 2, 2}},
          });
      }
    }
  }
}

TEST_CASE("RestrictedDragTracker.drag")
{
  GIVEN("A drag tracker")
  {
    const auto initialHandlePosition = vm::vec3d{1, 1, 1};
    const auto initialHitPoint = initialHandlePosition;
    auto handlePositionToReturn = vm::vec3d{};

    auto data = TestDelegateData{makeHandlePositionProposer(
      // always returns the same hit position
      [&](const auto&) { return handlePositionToReturn; },
      makeIdentityHandleSnapper())};

    auto tracker = makeHandleTracker(data, initialHandlePosition, initialHitPoint);

    WHEN("drag is called for the first time after the drag started")
    {
      handlePositionToReturn = vm::vec3d{2, 2, 2};
      REQUIRE(tracker.update(InputState{}));

      THEN("drag got the initial and the next handle positions")
      {
        CHECK(
          data.dragArguments
          == std::vector<std::tuple<DragState, vm::vec3d>>{
            {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 0}},
             vm::vec3d{2, 2, 2}},
          });

        AND_WHEN("drag is called again")
        {
          handlePositionToReturn = vm::vec3d{3, 3, 3};
          REQUIRE(tracker.update(InputState{}));

          THEN("drag got the last and the next handle positions")
          {
            CHECK(
              data.dragArguments
              == std::vector<std::tuple<DragState, vm::vec3d>>{
                {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 0}},
                 vm::vec3d{2, 2, 2}},
                {{vm::vec3d{1, 1, 1}, vm::vec3d{2, 2, 2}, vm::vec3d{0, 0, 0}},
                 vm::vec3d{3, 3, 3}},
              });
          }
        }
      }
    }

    WHEN("drag returns drag status deny")
    {
      handlePositionToReturn = vm::vec3d{2, 2, 2};
      data.dragStatusToReturn = DragStatus::Deny;
      REQUIRE(tracker.update(InputState{}));

      THEN("drag got the initial and the next handle positions")
      {
        CHECK(
          data.dragArguments
          == std::vector<std::tuple<DragState, vm::vec3d>>{
            {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 0}},
             vm::vec3d{2, 2, 2}},
          });

        AND_WHEN("drag is called again")
        {
          handlePositionToReturn = vm::vec3d{3, 3, 3};
          REQUIRE(tracker.update(InputState{}));

          THEN("drag got the initial handle position for the last handle position again")
          {
            CHECK(
              data.dragArguments
              == std::vector<std::tuple<DragState, vm::vec3d>>{
                {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 0}},
                 vm::vec3d{2, 2, 2}},
                {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 0}},
                 vm::vec3d{3, 3, 3}},
              });
          }
        }
      }
    }

    WHEN("drag returns drag status cancel")
    {
      handlePositionToReturn = vm::vec3d{2, 2, 2};
      data.dragStatusToReturn = DragStatus::End;
      const auto dragResult = tracker.update(InputState{});

      THEN("the drag tracker returns false")
      {
        CHECK_FALSE(dragResult);
      }
    }
  }
}

TEST_CASE("RestrictedDragTracker.handlePositionComputations")
{
  const auto initialHandlePosition = vm::vec3d{1, 1, 1};
  const auto initialHitPoint = vm::vec3d{1, 1, 0};

  auto getHandlePositionArguments = std::vector<std::tuple<DragState, vm::vec3d>>{};
  auto handlePositionToReturn = vm::vec3d{};

  GIVEN("A drag tracker")
  {
    auto data = TestDelegateData{makeHandlePositionProposer(
      // returns the handle position set above
      [&](const InputState&) { return handlePositionToReturn; },
      // returns the proposed handle position, but records the arguments
      [&](const auto&, const auto& dragState, const auto& proposedHandlePosition) {
        getHandlePositionArguments.emplace_back(dragState, proposedHandlePosition);
        return proposedHandlePosition;
      })};

    auto tracker = makeHandleTracker(data, initialHandlePosition, initialHitPoint);

    WHEN("drag is called for the first time")
    {
      handlePositionToReturn = vm::vec3d{2, 2, 2};
      REQUIRE(tracker.update(InputState{}));

      THEN("getHandlePosition is called with the expected arguments")
      {
        CHECK(
          getHandlePositionArguments
          == std::vector<std::tuple<DragState, vm::vec3d>>{
            {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 1}},
             vm::vec3d{2, 2, 2}},
          });

        AND_THEN("The new handle position was passed to the delegate's drag function")
        {
          CHECK(
            data.dragArguments
            == std::vector<std::tuple<DragState, vm::vec3d>>{
              {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 1}},
               vm::vec3d{2, 2, 2}},
            });
        }
      }

      AND_WHEN("drag is called again")
      {
        handlePositionToReturn = vm::vec3d{3, 3, 3};
        REQUIRE(tracker.update(InputState{}));

        THEN("getHandlePosition is called with the expected arguments")
        {
          CHECK(
            getHandlePositionArguments
            == std::vector<std::tuple<DragState, vm::vec3d>>{
              {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 1}},
               vm::vec3d{2, 2, 2}},
              {{vm::vec3d{1, 1, 1}, vm::vec3d{2, 2, 2}, vm::vec3d{0, 0, 1}},
               vm::vec3d{3, 3, 3}},
            });

          AND_THEN("The hit position was passed to the delegate's drag function")
          {
            CHECK(
              data.dragArguments
              == std::vector<std::tuple<DragState, vm::vec3d>>{
                {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 1}},
                 vm::vec3d{2, 2, 2}},
                {{vm::vec3d{1, 1, 1}, vm::vec3d{2, 2, 2}, vm::vec3d{0, 0, 1}},
                 vm::vec3d{3, 3, 3}},
              });
          }
        }
      }
    }
  }
}

TEST_CASE("RestrictedDragTracker.modifierKeyChange")
{
  const auto initialHandlePosition = vm::vec3d{1, 1, 1};
  const auto initialHitPoint = vm::vec3d{1, 1, 0};

  auto initialGetHandlePositionArguments =
    std::vector<std::tuple<DragState, vm::vec3d>>{};

  GIVEN("A delegate that returns null from modifierKeyChange")
  {
    auto data = TestDelegateData{makeHandlePositionProposer(
      // returns a constant handle position
      [&](const InputState&) { return vm::vec3d{2, 2, 2}; },
      // returns the proposed handle position, but records the arguments
      [&](const auto&, const auto& dragState, const auto& proposedHandlePosition) {
        initialGetHandlePositionArguments.emplace_back(dragState, proposedHandlePosition);
        return proposedHandlePosition;
      })};

    auto tracker = makeHandleTracker(data, initialHandlePosition, initialHitPoint);

    tracker.update(InputState{});
    REQUIRE(initialGetHandlePositionArguments.size() == 1);

    WHEN("A modifier key change is notified")
    {
      tracker.modifierKeyChange(InputState{});

      THEN("The drag state are passed to the delegate")
      {
        CHECK(
          data.modifierKeyChangeArguments
          == std::vector<DragState>{
            {vm::vec3d{1, 1, 1}, vm::vec3d{2, 2, 2}, vm::vec3d{0, 0, 1}}});

        AND_THEN("The next call to drag uses the initial drag config")
        {
          tracker.update(InputState{});
          CHECK(initialGetHandlePositionArguments.size() == 2);
        }
      }
    }
  }

  GIVEN("A delegate that returns a new drag config from modifierKeyChange")
  {
    auto otherGetHandlePositionArguments =
      std::vector<std::tuple<DragState, vm::vec3d>>{};
    auto otherHitPositionToReturn = vm::vec3d{};

    auto data = TestDelegateData{makeHandlePositionProposer(
      // returns a constant hit position
      [&](const InputState&) { return vm::vec3d{2, 2, 2}; },
      // returns the proposed handle position, but records the arguments
      [&](const auto&, const auto& dragState, const auto& proposedHandlePosition) {
        initialGetHandlePositionArguments.emplace_back(dragState, proposedHandlePosition);
        return proposedHandlePosition;
      })};

    data.updateDragConfigToReturn = UpdateDragConfig{
      makeHandlePositionProposer(
        // returns a constant hit position
        [&](const InputState&) { return otherHitPositionToReturn; },
        // returns the proposed handle position, but records the arguments
        [&](const auto&, const auto& dragState, const auto& proposedHandlePosition) {
          otherGetHandlePositionArguments.emplace_back(dragState, proposedHandlePosition);
          return proposedHandlePosition;
        }),
      ResetInitialHandlePosition::Keep};

    auto tracker = makeHandleTracker(data, initialHandlePosition, initialHitPoint);

    tracker.update(InputState{});
    REQUIRE(initialGetHandlePositionArguments.size() == 1);
    REQUIRE(
      data.dragArguments
      == std::vector<std::tuple<DragState, vm::vec3d>>{
        {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 1}},
         vm::vec3d{2, 2, 2}},
      });

    WHEN("A modifier key change is notified")
    {
      otherHitPositionToReturn = vm::vec3d{3, 3, 3};
      tracker.modifierKeyChange(InputState{});

      THEN("The drag state was passed to the delegate")
      {
        CHECK(
          data.modifierKeyChangeArguments
          == std::vector<DragState>{
            {vm::vec3d{1, 1, 1}, vm::vec3d{2, 2, 2}, vm::vec3d{0, 0, 1}}});

        AND_THEN(
          "A synthetic drag to the new handle position happens using the other drag "
          "config")
        {
          CHECK(initialGetHandlePositionArguments.size() == 1);
          CHECK(otherGetHandlePositionArguments.size() == 1);

          CHECK(
            data.dragArguments
            == std::vector<std::tuple<DragState, vm::vec3d>>{
              {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 1}},
               vm::vec3d{2, 2, 2}},
              {{vm::vec3d{1, 1, 1}, vm::vec3d{2, 2, 2}, vm::vec3d{0, 0, 1}},
               vm::vec3d{3, 3, 3}},
            });
        }

        AND_WHEN("drag is called again")
        {
          otherHitPositionToReturn = vm::vec3d{4, 4, 4};
          tracker.update(InputState{});

          AND_THEN("The other handle position is passed")
          {
            CHECK(
              data.dragArguments
              == std::vector<std::tuple<DragState, vm::vec3d>>{
                {{vm::vec3d{1, 1, 1}, vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, 1}},
                 vm::vec3d{2, 2, 2}},
                {{vm::vec3d{1, 1, 1}, vm::vec3d{2, 2, 2}, vm::vec3d{0, 0, 1}},
                 vm::vec3d{3, 3, 3}},
                {{vm::vec3d{1, 1, 1}, vm::vec3d{3, 3, 3}, vm::vec3d{0, 0, 1}},
                 vm::vec3d{4, 4, 4}},
              });

            AND_THEN("The other drag config was used")
            {
              CHECK(initialGetHandlePositionArguments.size() == 1);
              CHECK(otherGetHandlePositionArguments.size() == 2);
            }
          }
        }
      }
    }
  }
}

TEST_CASE("makeLineHandlePicker")
{
  using T = std::tuple<vm::line3d, vm::vec3d, vm::ray3d, vm::vec3d>;

  // clang-format off
  const auto 
  [line,                                            handleOffset,         pickRay,                                         expectedHandlePosition] = GENERATE(values<T>({
  {vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{ 0,  0,  0}, vm::ray3d{vm::vec3d{0, -1, 0}, vm::vec3d{0, 1, 0}}, vm::vec3d{0, 0, 0}},
  {vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{-1, -1, -1}, vm::ray3d{vm::vec3d{1, -1, 1}, vm::vec3d{0, 1, 0}}, vm::vec3d{0, 0, 0}}, // hitPoint is at {1 1 1}
  {vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{-1, -1, -1}, vm::ray3d{vm::vec3d{1, -1, 2}, vm::vec3d{0, 1, 0}}, vm::vec3d{0, 0, 1}}, // hitPoint is at {1 1 1}
  }));
  // clang-format on

  CAPTURE(line, handleOffset, pickRay);

  const auto camera = render::OrthographicCamera{};
  auto inputState = InputState{};
  inputState.setPickRequest(PickRequest{pickRay, camera});

  CHECK(makeLineHandlePicker(line, handleOffset)(inputState) == expectedHandlePosition);
}

TEST_CASE("makePlaneHandlePicker")
{
  using T = std::tuple<vm::plane3d, vm::vec3d, vm::ray3d, vm::vec3d>;

  // clang-format off
  const auto
  [plane,                                            handleOffset,         pickRay,                                         expectedHandlePosition] = GENERATE(values<T>({
  {vm::plane3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{ 0,  0,  0}, vm::ray3d{vm::vec3d{0, 0, 1}, vm::vec3d{0, 0, -1}}, vm::vec3d{0, 0, 0}},
  {vm::plane3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{-1, -1, -1}, vm::ray3d{vm::vec3d{1, 1, 1}, vm::vec3d{0, 0, -1}}, vm::vec3d{0, 0, 0}}, // hitPoint is at {1 1 1}
  {vm::plane3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{-1, -1, -1}, vm::ray3d{vm::vec3d{1, 2, 1}, vm::vec3d{0, 0, -1}}, vm::vec3d{0, 1, 0}}, // hitPoint is at {1 1 1}
  }));
  // clang-format on

  CAPTURE(plane, handleOffset, pickRay);

  const auto camera = render::OrthographicCamera{};
  auto inputState = InputState{};
  inputState.setPickRequest(PickRequest{pickRay, camera});

  CHECK(makePlaneHandlePicker(plane, handleOffset)(inputState) == expectedHandlePosition);
}

TEST_CASE("makeCircleHandlePicker")
{
  using T = std::tuple<vm::vec3d, vm::vec3d, double, vm::vec3d, vm::ray3d, vm::vec3d>;

  // clang-format off
  const auto
  [center,            normal,            radius, handleOffset,         pickRay,                                         expectedHandlePosition] = GENERATE(values<T>({
  {vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, 10.0,   vm::vec3d{ 0,  0,  0}, vm::ray3d{vm::vec3d{5, 0, 1}, vm::vec3d{0, 0, -1}}, 10.0 * vm::normalize(vm::vec3d{1, 0, 0})},
  {vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, 10.0,   vm::vec3d{ 0,  0,  1}, vm::ray3d{vm::vec3d{5, 0, 1}, vm::vec3d{0, 0, -1}}, 10.0 * vm::normalize(vm::vec3d{1, 0, 0})},
  {vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, 10.0,   vm::vec3d{ 0,  0,  0}, vm::ray3d{vm::vec3d{5, 5, 1}, vm::vec3d{0, 0, -1}}, 10.0 * vm::normalize(vm::vec3d{1, 1, 0})},
  {vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, 10.0,   vm::vec3d{ 1,  1,  1}, vm::ray3d{vm::vec3d{5, 5, 1}, vm::vec3d{0, 0, -1}}, 10.0 * vm::normalize(vm::vec3d{1, 1, 0})},
  }));
  // clang-format on

  CAPTURE(center, normal, radius, handleOffset, pickRay);

  const auto camera = render::OrthographicCamera{};
  auto inputState = InputState{};
  inputState.setPickRequest(PickRequest{pickRay, camera});

  CHECK(
    makeCircleHandlePicker(center, normal, radius, handleOffset)(inputState)
    == vm::approx{expectedHandlePosition});
}

TEST_CASE("makeSurfaceHandlePicker")
{
  using namespace mdl::HitFilters;

  static const auto HitType = mdl::HitType::freeType();
  static const auto OtherHitType = mdl::HitType::freeType();
  static const auto BothTypes = HitType | OtherHitType;

  const auto hit = mdl::Hit{HitType, 10.0, vm::vec3d{0, 0, 10}, size_t{1}};
  const auto otherHit = mdl::Hit{OtherHitType, 12.0, vm::vec3d{0, 0, 12}, size_t{2}};

  using T = std::tuple<mdl::HitFilter, vm::vec3d, vm::ray3d, vm::vec3d>;

  // clang-format off
  const auto
  [hitFilter,          handleOffset,      pickRay,                                          expectedHandlePosition] = GENERATE_REF(values<T>({
  {type(HitType),      vm::vec3d{0, 0, 0}, vm::ray3d{vm::vec3d{0, 0, 20}, vm::vec3d{0, 0, -1}}, vm::vec3d{hit.hitPoint()}},
  {type(OtherHitType), vm::vec3d{0, 0, 0}, vm::ray3d{vm::vec3d{0, 0, 20}, vm::vec3d{0, 0, -1}}, vm::vec3d{otherHit.hitPoint()}},
  {type(BothTypes),    vm::vec3d{0, 0, 0}, vm::ray3d{vm::vec3d{0, 0, 20}, vm::vec3d{0, 0, -1}}, vm::vec3d{hit.hitPoint()}},
  {type(HitType),      vm::vec3d{1, 1, 1}, vm::ray3d{vm::vec3d{0, 0, 20}, vm::vec3d{0, 0, -1}}, vm::vec3d{hit.hitPoint() + vm::vec3d{1, 1, 1}}},
  }));
  // clang-format on

  CAPTURE(handleOffset, pickRay);

  const auto camera = render::OrthographicCamera{};
  auto inputState = InputState{};
  inputState.setPickRequest(PickRequest{pickRay, camera});

  auto pickResult = mdl::PickResult{};
  pickResult.addHit(hit);
  pickResult.addHit(otherHit);
  inputState.setPickResult(std::move(pickResult));

  CHECK(
    makeSurfaceHandlePicker(hitFilter, handleOffset)(inputState)
    == expectedHandlePosition);
}

TEST_CASE("makeIdentityHandleSnapper")
{
  using T = std::tuple<vm::vec3d, vm::vec3d>;

  // clang-format off
  const auto 
  [proposedHandlePosition, expectedHandlePosition] = GENERATE(values<T>({
  {vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 0}},
  {vm::vec3d{1, 2, 3}, vm::vec3d{1, 2, 3}},
  }));
  // clang-format on

  CAPTURE(proposedHandlePosition);

  CHECK(
    makeIdentityHandleSnapper()(InputState{}, DragState{}, proposedHandlePosition)
    == expectedHandlePosition);
}

TEST_CASE("makeRelativeHandleSnapper")
{
  using T = std::tuple<vm::vec3d, vm::vec3d, int, vm::vec3d>;

  // clang-format off
  const auto
  [initialHandlePosition, proposedHandlePosition, gridSize, expectedHandlePosition] = GENERATE(values<T>({
  {vm::vec3d{3, 1, 2},     vm::vec3d{3, 1, 2},      4,        vm::vec3d{3, 1, 2}},
  {vm::vec3d{3, 1, 2},     vm::vec3d{7, 1, 2},      4,        vm::vec3d{3, 1, 2}},
  {vm::vec3d{3, 1, 2},     vm::vec3d{8, 1, 2},      3,        vm::vec3d{11, 1, 2}},
  {vm::vec3d{3, 1, 2},     vm::vec3d{10, 1, 2},     4,        vm::vec3d{3, 1, 2}},
  {vm::vec3d{3, 1, 2},     vm::vec3d{11, 1, 2},     4,        vm::vec3d{19, 1, 2}},
  {vm::vec3d{3, 1, 2},     vm::vec3d{33, 1, 2},     4,        vm::vec3d{35, 1, 2}},
  }));
  // clang-format on

  CAPTURE(initialHandlePosition, proposedHandlePosition, gridSize);

  const auto grid = Grid{gridSize};
  CHECK(
    makeRelativeHandleSnapper(grid)(
      InputState{},
      DragState{initialHandlePosition, vm::vec3d{}, vm::vec3d{}},
      proposedHandlePosition)
    == expectedHandlePosition);
}

TEST_CASE("makeAbsoluteHandleSnapper")
{
  using T = std::tuple<vm::vec3d, int, vm::vec3d>;

  // clang-format off
  const auto
  [proposedHandlePosition, gridSize, expectedHandlePosition] = GENERATE(values<T>({
  {vm::vec3d{0, 0, 0},      4,        vm::vec3d{0, 0, 0}},
  {vm::vec3d{4, 3, 2},      4,        vm::vec3d{0, 0, 0}},
  {vm::vec3d{4, 3, 22},     3,        vm::vec3d{8, 0, 24}},
  {vm::vec3d{7, 0, 0},      4,        vm::vec3d{0, 0, 0}},
  {vm::vec3d{8, 17, 31},    4,        vm::vec3d{16, 16, 32}},
  }));
  // clang-format on

  CAPTURE(proposedHandlePosition, gridSize);

  const auto grid = Grid{gridSize};
  CHECK(
    makeAbsoluteHandleSnapper(grid)(
      InputState{},
      DragState{vm::vec3d{}, vm::vec3d{}, vm::vec3d{}},
      proposedHandlePosition)
    == expectedHandlePosition);
}

TEST_CASE("makeRelativeLineHandleSnapper")
{
  using T = std::tuple<vm::vec3d, vm::vec3d, int, vm::line3d, vm::vec3d>;

  // clang-format off
  const auto
  [initialHandlePosition, proposedHandlePosition, gridSize, line,                                            expectedHandlePosition] = GENERATE(values<T>({
  {vm::vec3d{0, 0, 0},     vm::vec3d{0, 0, 0},      4,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 0}},
  {vm::vec3d{0, 0, 0},     vm::vec3d{0, 0, 7},      4,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 0}},
  {vm::vec3d{0, 0, 0},     vm::vec3d{2, 9, 7},      4,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 0}},
  {vm::vec3d{0, 0, 0},     vm::vec3d{2, 9, 8},      4,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 16}},
  {vm::vec3d{0, 0, 1},     vm::vec3d{2, 9, 8},      4,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 1}},
  {vm::vec3d{0, 0, 1},     vm::vec3d{2, 9, 9},      4,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 17}},
  {vm::vec3d{22, 9, 1},    vm::vec3d{2, 9, 9},      4,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 17}},
  }));
  // clang-format on

  CAPTURE(initialHandlePosition, proposedHandlePosition, gridSize, line);

  const auto grid = Grid{gridSize};
  CHECK(
    makeRelativeLineHandleSnapper(grid, line)(
      InputState{},
      DragState{initialHandlePosition, vm::vec3d{}, vm::vec3d{}},
      proposedHandlePosition)
    == expectedHandlePosition);
}

TEST_CASE("makeAbsoluteLineHandleSnapper")
{
  using T = std::tuple<vm::vec3d, int, vm::line3d, vm::vec3d>;

  // clang-format off
  const auto
  [proposedHandlePosition, gridSize, line,                                            expectedHandlePosition] = GENERATE(values<T>({
  {vm::vec3d{0, 0, 0},      4,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 0}},
  {vm::vec3d{0, 0, 7},      4,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 0}},
  {vm::vec3d{0, 0, 7},      3,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 8}},
  {vm::vec3d{2, 9, 7},      4,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 0}},
  {vm::vec3d{2, 9, 9},      4,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 16}},
  {vm::vec3d{2, 9, 31},     4,        vm::line3d{vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}}, vm::vec3d{0, 0, 32}},
  }));
  // clang-format on

  CAPTURE(proposedHandlePosition, gridSize, line);

  const auto grid = Grid{gridSize};
  CHECK(
    makeAbsoluteLineHandleSnapper(grid, line)(
      InputState{},
      DragState{vm::vec3d{}, vm::vec3d{}, vm::vec3d{}},
      proposedHandlePosition)
    == expectedHandlePosition);
}

TEST_CASE("makeCircleHandleSnapper")
{
  using T = std::tuple<vm::vec3d, vm::vec3d, double, vm::vec3d>;

  // clang-format off
  const auto
  [initialHandlePosition, proposedHandlePosition, snapAngle, expectedHandlePosition] = GENERATE(values<T>({
  {vm::vec3d{1, 0, 0},     vm::vec3d{1, 0, 0},      15.0,      vm::normalize(vm::vec3d{1, 0, 0})},
  {vm::vec3d{1, 0, 0},     vm::vec3d{1, 1, 0},      15.0,      vm::normalize(vm::vec3d{1, 1, 0})},
  {vm::vec3d{1, 0, 0},     vm::vec3d{1, 2, 0},      15.0,      vm::normalize(vm::vec3d{0.5, 0.866025, 0})},
  {vm::vec3d{1, 0, 0},     vm::vec3d{1, 1, 0},      45.0,      vm::normalize(vm::vec3d{1, 1, 0})},
  }));
  // clang-format on

  CAPTURE(initialHandlePosition, proposedHandlePosition, snapAngle);

  const auto grid = Grid{4};
  const auto center = vm::vec3d{0, 0, 0};
  const auto normal = vm::vec3d{0, 0, 1};
  const auto radius = 10.0;
  CHECK(
    makeCircleHandleSnapper(grid, vm::to_radians(snapAngle), center, normal, radius)(
      InputState{},
      DragState{initialHandlePosition, vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 0}},
      proposedHandlePosition)
    == vm::approx{radius * expectedHandlePosition});
}

} // namespace tb::View
