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

#include "mdl/CatchConfig.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/Tool.h"
#include "ui/ToolController.h"

#include <memory>
#include <optional>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

class FakeTool : public Tool
{
public:
  FakeTool()
    : Tool{true}
  {
  }
};

/**
 * Records whether the tool had a gesture in progress when the gesture ended or was
 * cancelled.
 */
class FakeGestureTracker : public GestureTracker
{
private:
  const Tool& m_tool;
  std::optional<bool>& m_gestureInProgressWhenDone;

public:
  FakeGestureTracker(const Tool& tool, std::optional<bool>& gestureInProgressWhenDone)
    : m_tool{tool}
    , m_gestureInProgressWhenDone{gestureInProgressWhenDone}
  {
  }

  bool update(const InputState&) override { return true; }

  void end(const InputState&) override
  {
    m_gestureInProgressWhenDone = m_tool.gestureInProgress();
  }

  void cancel() override { m_gestureInProgressWhenDone = m_tool.gestureInProgress(); }
};

class FakeToolController : public ToolController
{
public:
  FakeTool fakeTool;
  bool acceptResult = false;
  std::optional<bool> gestureInProgressWhenDone;

  Tool& tool() override { return fakeTool; }
  const Tool& tool() const override { return fakeTool; }

private:
  std::unique_ptr<GestureTracker> doAcceptMouseDrag(const InputState&) override
  {
    return acceptResult
             ? std::make_unique<FakeGestureTracker>(fakeTool, gestureInProgressWhenDone)
             : nullptr;
  }

  std::unique_ptr<GestureTracker> doAcceptGesture(const InputState&) override
  {
    return acceptResult
             ? std::make_unique<FakeGestureTracker>(fakeTool, gestureInProgressWhenDone)
             : nullptr;
  }
};

} // namespace

TEST_CASE("ToolController")
{
  auto controller = FakeToolController{};
  const auto inputState = InputState{0.0f, 0.0f};

  SECTION("acceptMouseDrag")
  {
    SECTION("returns null if the controller does not accept the drag")
    {
      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
      CHECK(!controller.tool().gestureInProgress());
    }

    SECTION("marks the tool's gesture as in progress until the drag is over")
    {
      controller.acceptResult = true;

      auto tracker = controller.acceptMouseDrag(inputState);
      REQUIRE(tracker != nullptr);
      CHECK(controller.tool().gestureInProgress());
      CHECK(tracker->update(inputState));
      CHECK(controller.tool().gestureInProgress());

      SECTION("ending the drag resets the flag before the tracker is notified")
      {
        tracker->end(inputState);
        CHECK(!controller.tool().gestureInProgress());
        CHECK(controller.gestureInProgressWhenDone == false);
      }

      SECTION("cancelling the drag resets the flag before the tracker is notified")
      {
        tracker->cancel();
        CHECK(!controller.tool().gestureInProgress());
        CHECK(controller.gestureInProgressWhenDone == false);
      }
    }
  }

  SECTION("acceptGesture")
  {
    SECTION("returns null if the controller does not accept the gesture")
    {
      CHECK(controller.acceptGesture(inputState) == nullptr);
      CHECK(!controller.tool().gestureInProgress());
    }

    SECTION("marks the tool's gesture as in progress until the gesture is over")
    {
      controller.acceptResult = true;

      auto tracker = controller.acceptGesture(inputState);
      REQUIRE(tracker != nullptr);
      CHECK(controller.tool().gestureInProgress());

      SECTION("ending the gesture resets the flag before the tracker is notified")
      {
        tracker->end(inputState);
        CHECK(!controller.tool().gestureInProgress());
        CHECK(controller.gestureInProgressWhenDone == false);
      }

      SECTION("cancelling the gesture resets the flag before the tracker is notified")
      {
        tracker->cancel();
        CHECK(!controller.tool().gestureInProgress());
        CHECK(controller.gestureInProgressWhenDone == false);
      }
    }
  }
}

} // namespace tb::ui
