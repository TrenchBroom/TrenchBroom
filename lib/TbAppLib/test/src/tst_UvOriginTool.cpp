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

#include "gl/Camera.h"
#include "gl/Material.h"
#include "gl/OrthographicCamera.h"
#include "gl/Texture.h"
#include "gl/TextureResource.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/HitFilter.h"
#include "mdl/MapFormat.h"
#include "mdl/PickResult.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/PickRequest.h"
#include "ui/ToolController.h"
#include "ui/UvOriginTool.h"
#include "ui/UvViewHelper.h"

#include "kd/result.h"

#include <memory>
#include <utility>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("UvOriginTool")
{
  auto material =
    gl::Material{"material", gl::createTextureResource(gl::Texture{64, 64})};

  const auto worldBounds = vm::bbox3d{8192.0};
  auto builder = mdl::BrushBuilder{mdl::MapFormat::Standard, worldBounds};

  auto brush = builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value();
  const auto topFaceIndex = *brush.findFace(vm::vec3d{0, 0, 1});
  brush.face(topFaceIndex).setMaterial(&material);

  auto brushNode = std::make_unique<mdl::BrushNode>(std::move(brush));
  const auto faceHandle = mdl::BrushFaceHandle{brushNode.get(), topFaceIndex};

  auto camera = gl::OrthographicCamera{
    1.0f,
    8000.0f,
    gl::Camera::Viewport{0, 0, 1024, 768},
    vm::vec3f{0, 0, 100},
    vm::vec3f{0, 0, -1},
    vm::vec3f{0, 1, 0}};
  auto helper = UvViewHelper{camera};
  helper.setFaceHandle(faceHandle);

  auto tool = UvOriginTool{helper};
  auto& controller = static_cast<ToolController&>(tool);

  // a vertical ray hits the (planar, z = 16) face exactly at (x, y, 16)
  const auto rayAt = [](const vm::vec3d& facePoint) {
    return vm::ray3d{{facePoint.x(), facePoint.y(), 100}, {0, 0, -1}};
  };

  const auto inputStateFor = [&](const vm::ray3d& ray) {
    auto inputState = InputState{0.0f, 0.0f};
    inputState.setPickRequest(PickRequest{ray, helper.camera()});
    auto pickResult = mdl::PickResult{};
    controller.pick(inputState, pickResult);
    inputState.setPickResult(std::move(pickResult));
    return inputState;
  };

  SECTION("cancel always returns false")
  {
    CHECK(!controller.cancel());
  }

  SECTION("tool returns itself")
  {
    CHECK(&controller.tool() == &tool);
  }

  SECTION("pick")
  {
    SECTION("with an invalid helper, nothing is hit")
    {
      auto emptyCamera = gl::OrthographicCamera{};
      auto emptyHelper = UvViewHelper{emptyCamera};
      auto emptyTool = UvOriginTool{emptyHelper};
      auto& emptyController = static_cast<ToolController&>(emptyTool);

      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest(
        PickRequest{vm::ray3d{{0, 0, 100}, {0, 0, -1}}, emptyCamera});
      auto pickResult = mdl::PickResult{};
      emptyController.pick(inputState, pickResult);

      CHECK(pickResult.empty());
    }

    SECTION("hits both handles near the origin point")
    {
      const auto inputState = inputStateFor(rayAt(helper.origin()));

      CHECK(inputState.pickResult()
              .first(mdl::HitFilters::type(UvOriginTool::XHandleHitType))
              .isMatch());
      CHECK(inputState.pickResult()
              .first(mdl::HitFilters::type(UvOriginTool::YHandleHitType))
              .isMatch());
    }

    SECTION("hits only the X handle away from the origin")
    {
      auto x1 = vm::vec3d{}, x2 = vm::vec3d{}, y1 = vm::vec3d{}, y2 = vm::vec3d{};
      helper.computeOriginHandleVertices(x1, x2, y1, y2);

      const auto inputState = inputStateFor(rayAt(x2));

      CHECK(inputState.pickResult()
              .first(mdl::HitFilters::type(UvOriginTool::XHandleHitType))
              .isMatch());
      CHECK(!inputState.pickResult()
               .first(mdl::HitFilters::type(UvOriginTool::YHandleHitType))
               .isMatch());
    }

    SECTION("hits only the Y handle away from the origin")
    {
      auto x1 = vm::vec3d{}, x2 = vm::vec3d{}, y1 = vm::vec3d{}, y2 = vm::vec3d{};
      helper.computeOriginHandleVertices(x1, x2, y1, y2);

      const auto inputState = inputStateFor(rayAt(y2));

      CHECK(!inputState.pickResult()
               .first(mdl::HitFilters::type(UvOriginTool::XHandleHitType))
               .isMatch());
      CHECK(inputState.pickResult()
              .first(mdl::HitFilters::type(UvOriginTool::YHandleHitType))
              .isMatch());
    }

    SECTION("hits nothing far from both handles")
    {
      const auto inputState = inputStateFor(rayAt(vm::vec3d{5, 5, 16}));

      CHECK(inputState.pickResult().empty());
    }
  }

  SECTION("acceptMouseDrag")
  {
    SECTION("returns nullptr with a modifier key held")
    {
      auto inputState = inputStateFor(rayAt(helper.origin()));
      inputState.setModifierKeys(ModifierKeys::Shift);
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("returns nullptr with the wrong mouse button")
    {
      auto inputState = inputStateFor(rayAt(helper.origin()));
      inputState.mouseDown(MouseButtons::Right);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("returns nullptr when no handle is hit")
    {
      auto inputState = inputStateFor(rayAt(vm::vec3d{5, 5, 16}));
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("dragging the origin handle moves the origin")
    {
      auto inputState = inputStateFor(rayAt(helper.origin()));
      inputState.mouseDown(MouseButtons::Left);

      auto tracker = controller.acceptMouseDrag(inputState);
      REQUIRE(tracker != nullptr);

      const auto originBefore = helper.originInFaceCoords();

      auto x1 = vm::vec3d{}, x2 = vm::vec3d{}, y1 = vm::vec3d{}, y2 = vm::vec3d{};
      helper.computeOriginHandleVertices(x1, x2, y1, y2);

      auto dragInputState = inputStateFor(rayAt(y2));
      dragInputState.setModifierKeys(ModifierKeys::CtrlCmd);
      CHECK(tracker->update(dragInputState));

      CHECK(helper.originInFaceCoords() != originBefore);

      // the no-op overrides should not crash
      tracker->end(dragInputState);
      tracker->cancel();
    }

    SECTION("dragging without the snapping modifier still moves the origin")
    {
      auto inputState = inputStateFor(rayAt(helper.origin()));
      inputState.mouseDown(MouseButtons::Left);

      auto tracker = controller.acceptMouseDrag(inputState);
      REQUIRE(tracker != nullptr);

      const auto originBefore = helper.originInFaceCoords();

      auto x1 = vm::vec3d{}, x2 = vm::vec3d{}, y1 = vm::vec3d{}, y2 = vm::vec3d{};
      helper.computeOriginHandleVertices(x1, x2, y1, y2);

      // no CtrlCmd held, so this exercises the snapDelta() codepath
      const auto dragInputState = inputStateFor(rayAt(y2));
      CHECK(tracker->update(dragInputState));

      CHECK(helper.originInFaceCoords() != originBefore);
    }
  }
}

} // namespace tb::ui
