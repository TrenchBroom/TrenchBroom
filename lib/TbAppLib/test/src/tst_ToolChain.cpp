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
#include "mdl/PickResult.h"
#include "ui/DropTracker.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/Tool.h"
#include "ui/ToolChain.h"
#include "ui/ToolController.h"

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

class FakeTool : public Tool
{
public:
  explicit FakeTool(const bool initiallyActive)
    : Tool{initiallyActive}
  {
  }
};

class FakeGestureTracker : public GestureTracker
{
public:
  bool update(const InputState&) override { return true; }
  void end(const InputState&) override {}
  void cancel() override {}
};

class FakeDropTracker : public DropTracker
{
public:
  bool move(const InputState&) override { return true; }
  bool drop(const InputState&) override { return true; }
  void leave(const InputState&) override {}
};

/**
 * A ToolController test double that counts how often each callback is invoked, records
 * its name to a shared call order log (if one is set), and returns configurable results,
 * so ToolChain's dispatch and short-circuit logic can be observed from outside.
 */
class RecordingToolController : public ToolController
{
public:
  std::string name;
  std::vector<std::string>* callOrder = nullptr;
  FakeTool fakeTool;

  int pickCount = 0;
  int modifierKeyChangeCount = 0;
  int mouseDownCount = 0;
  int mouseUpCount = 0;
  int mouseScrollCount = 0;
  int mouseMoveCount = 0;
  mutable int setRenderOptionsCount = 0;
  int renderCount = 0;

  int mouseClickCount = 0;
  bool mouseClickResult = false;

  int mouseDoubleClickCount = 0;
  bool mouseDoubleClickResult = false;

  int acceptMouseDragCount = 0;
  bool acceptMouseDragResult = false;

  int acceptGestureCount = 0;
  bool acceptGestureResult = false;

  mutable int shouldAcceptDropCount = 0;
  bool shouldAcceptDropResult = false;

  int acceptDropCount = 0;
  bool acceptDropResult = false;

  int cancelCount = 0;
  bool cancelResult = false;

  explicit RecordingToolController(
    std::string name_ = {}, const bool initiallyActive = true)
    : name{std::move(name_)}
    , fakeTool{initiallyActive}
  {
  }

  Tool& tool() override { return fakeTool; }
  const Tool& tool() const override { return fakeTool; }

  void logCall() const
  {
    if (callOrder)
    {
      callOrder->push_back(name);
    }
  }

  void pick(const InputState&, mdl::PickResult&) override
  {
    ++pickCount;
    logCall();
  }

  void modifierKeyChange(const InputState&) override
  {
    ++modifierKeyChangeCount;
    logCall();
  }

  void mouseDown(const InputState&) override
  {
    ++mouseDownCount;
    logCall();
  }

  void mouseUp(const InputState&) override
  {
    ++mouseUpCount;
    logCall();
  }

  bool mouseClick(const InputState&) override
  {
    ++mouseClickCount;
    logCall();
    return mouseClickResult;
  }

  bool mouseDoubleClick(const InputState&) override
  {
    ++mouseDoubleClickCount;
    logCall();
    return mouseDoubleClickResult;
  }

  void mouseScroll(const InputState&) override
  {
    ++mouseScrollCount;
    logCall();
  }

  void mouseMove(const InputState&) override
  {
    ++mouseMoveCount;
    logCall();
  }

  std::unique_ptr<GestureTracker> acceptMouseDrag(const InputState&) override
  {
    ++acceptMouseDragCount;
    logCall();
    return acceptMouseDragResult ? std::make_unique<FakeGestureTracker>() : nullptr;
  }

  std::unique_ptr<GestureTracker> acceptGesture(const InputState&) override
  {
    ++acceptGestureCount;
    logCall();
    return acceptGestureResult ? std::make_unique<FakeGestureTracker>() : nullptr;
  }

  bool shouldAcceptDrop(const InputState&, const std::string&) const override
  {
    ++shouldAcceptDropCount;
    logCall();
    return shouldAcceptDropResult;
  }

  std::unique_ptr<DropTracker> acceptDrop(const InputState&, const std::string&) override
  {
    ++acceptDropCount;
    logCall();
    return acceptDropResult ? std::make_unique<FakeDropTracker>() : nullptr;
  }

  bool cancel() override
  {
    ++cancelCount;
    logCall();
    return cancelResult;
  }
};

} // namespace

TEST_CASE("ToolChain")
{
  auto chain = ToolChain{};
  auto callOrder = std::vector<std::string>{};

  auto aOwner = std::make_unique<RecordingToolController>("A");
  aOwner->callOrder = &callOrder;
  auto& a = *aOwner.get();

  auto bOwner = std::make_unique<RecordingToolController>("B");
  bOwner->callOrder = &callOrder;
  auto& b = *bOwner.get();

  const auto inputState = InputState{0.0f, 0.0f};

  SECTION("append builds the chain in append order")
  {
    chain.append(std::move(aOwner));
    chain.append(std::move(bOwner));

    auto pickResult = mdl::PickResult{};
    chain.pick(inputState, pickResult);

    CHECK(a.pickCount == 1);
    CHECK(b.pickCount == 1);
    CHECK(callOrder == std::vector<std::string>{"A", "B"});
  }

  SECTION("pick")
  {
    chain.append(std::move(aOwner));
    chain.append(std::move(bOwner));

    auto pickResult = mdl::PickResult{};
    chain.pick(inputState, pickResult);

    CHECK(a.pickCount == 1);
    CHECK(b.pickCount == 1);
  }

  SECTION("inactive tools are skipped but do not stop traversal")
  {
    a.fakeTool.deactivate();
    chain.append(std::move(aOwner));
    chain.append(std::move(bOwner));

    auto pickResult = mdl::PickResult{};
    chain.pick(inputState, pickResult);

    CHECK(a.pickCount == 0);
    CHECK(b.pickCount == 1);
    CHECK(callOrder == std::vector<std::string>{"B"});
  }

  SECTION("modifierKeyChange dispatches to every active tool")
  {
    chain.append(std::move(aOwner));
    chain.append(std::move(bOwner));

    chain.modifierKeyChange(inputState);

    CHECK(a.modifierKeyChangeCount == 1);
    CHECK(b.modifierKeyChangeCount == 1);
  }

  SECTION("mouseDown dispatches to every active tool")
  {
    chain.append(std::move(aOwner));
    chain.append(std::move(bOwner));

    chain.mouseDown(inputState);

    CHECK(a.mouseDownCount == 1);
    CHECK(b.mouseDownCount == 1);
  }

  SECTION("mouseUp dispatches to every active tool")
  {
    chain.append(std::move(aOwner));
    chain.append(std::move(bOwner));

    chain.mouseUp(inputState);

    CHECK(a.mouseUpCount == 1);
    CHECK(b.mouseUpCount == 1);
  }

  SECTION("mouseScroll dispatches to every active tool")
  {
    chain.append(std::move(aOwner));
    chain.append(std::move(bOwner));

    chain.mouseScroll(inputState);

    CHECK(a.mouseScrollCount == 1);
    CHECK(b.mouseScrollCount == 1);
  }

  SECTION("mouseMove dispatches to every active tool")
  {
    chain.append(std::move(aOwner));
    chain.append(std::move(bOwner));

    chain.mouseMove(inputState);

    CHECK(a.mouseMoveCount == 1);
    CHECK(b.mouseMoveCount == 1);
  }

  SECTION("mouseClick stops at the first tool that handles it")
  {
    SECTION("the first tool handles it")
    {
      a.mouseClickResult = true;
      chain.append(std::move(aOwner));
      chain.append(std::move(bOwner));

      CHECK(chain.mouseClick(inputState));
      CHECK(a.mouseClickCount == 1);
      CHECK(b.mouseClickCount == 0);
    }

    SECTION("the second tool handles it")
    {
      b.mouseClickResult = true;
      chain.append(std::move(aOwner));
      chain.append(std::move(bOwner));

      CHECK(chain.mouseClick(inputState));
      CHECK(a.mouseClickCount == 1);
      CHECK(b.mouseClickCount == 1);
    }

    SECTION("no tool handles it")
    {
      chain.append(std::move(aOwner));
      chain.append(std::move(bOwner));

      CHECK(!chain.mouseClick(inputState));
      CHECK(a.mouseClickCount == 1);
      CHECK(b.mouseClickCount == 1);
    }
  }

  SECTION("mouseDoubleClick stops at the first tool that handles it")
  {
    a.mouseDoubleClickResult = true;
    chain.append(std::move(aOwner));
    chain.append(std::move(bOwner));

    CHECK(chain.mouseDoubleClick(inputState));
    CHECK(a.mouseDoubleClickCount == 1);
    CHECK(b.mouseDoubleClickCount == 0);
  }

  SECTION("acceptMouseDrag returns the first non-null tracker")
  {
    b.acceptMouseDragResult = true;
    chain.append(std::move(aOwner));
    chain.append(std::move(bOwner));

    CHECK(chain.acceptMouseDrag(inputState) != nullptr);
    CHECK(a.acceptMouseDragCount == 1);
    CHECK(b.acceptMouseDragCount == 1);
  }

  SECTION("acceptGesture returns the first non-null tracker")
  {
    b.acceptGestureResult = true;
    chain.append(std::move(aOwner));
    chain.append(std::move(bOwner));

    CHECK(chain.acceptGesture(inputState) != nullptr);
    CHECK(a.acceptGestureCount == 1);
    CHECK(b.acceptGestureCount == 1);
  }

  SECTION("shouldAcceptDrop")
  {
    SECTION("is true if any active tool accepts the drop")
    {
      b.shouldAcceptDropResult = true;
      chain.append(std::move(aOwner));
      chain.append(std::move(bOwner));

      CHECK(chain.shouldAcceptDrop(inputState, "payload"));
    }

    SECTION("is false if no tool accepts the drop")
    {
      chain.append(std::move(aOwner));
      chain.append(std::move(bOwner));

      CHECK(!chain.shouldAcceptDrop(inputState, "payload"));
    }

    SECTION("skips inactive tools")
    {
      a.fakeTool.deactivate();
      a.shouldAcceptDropResult = true;
      chain.append(std::move(aOwner));
      chain.append(std::move(bOwner));

      CHECK(!chain.shouldAcceptDrop(inputState, "payload"));
    }
  }

  SECTION("dragEnter returns the first accepted drop tracker")
  {
    b.acceptDropResult = true;
    chain.append(std::move(aOwner));
    chain.append(std::move(bOwner));

    CHECK(chain.dragEnter(inputState, "payload") != nullptr);
    CHECK(a.acceptDropCount == 1);
    CHECK(b.acceptDropCount == 1);
  }

  SECTION("cancel stops at the first tool that cancels")
  {
    SECTION("no tool cancels")
    {
      chain.append(std::move(aOwner));
      chain.append(std::move(bOwner));

      CHECK(!chain.cancel());
      CHECK(a.cancelCount == 1);
      CHECK(b.cancelCount == 1);
    }

    SECTION("the first tool cancels")
    {
      a.cancelResult = true;
      chain.append(std::move(aOwner));
      chain.append(std::move(bOwner));

      CHECK(chain.cancel());
      CHECK(a.cancelCount == 1);
      CHECK(b.cancelCount == 0);
    }
  }
}

} // namespace tb::ui
