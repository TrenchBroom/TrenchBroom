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
#include "mdl/EditorContext.h"
#include "mdl/HitFilter.h"
#include "mdl/LayerNode.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PickResult.h"
#include "mdl/UvAttributes.h"
#include "mdl/WorldNode.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/PickRequest.h"
#include "ui/ToolController.h"
#include "ui/UvRotateTool.h"
#include "ui/UvViewHelper.h"

#include "kd/result.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("UvRotateTool")
{
  // declared before the fixture, so it outlives (is destroyed after) the brush that
  // references it via a raw, non-owning pointer
  auto material =
    gl::Material{"material", gl::createTextureResource(gl::Texture{64, 64})};

  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
  auto brush = builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value();
  const auto topFaceIndex = *brush.findFace(vm::vec3d{0, 0, 1});

  auto* brushNode = new mdl::BrushNode{std::move(brush)};
  mdl::addNodes(map, {{map.editorContext().currentLayer(), {brushNode}}});

  // addNodes resolves materials by name against the (empty, in this fixture) material
  // collection, which would clear a material set before insertion. Set it afterwards,
  // directly via setBrush, which does not touch material resolution.
  auto updatedBrush = brushNode->brush();
  updatedBrush.face(topFaceIndex).setMaterial(&material);
  brushNode->setBrush(std::move(updatedBrush));

  const auto faceHandle = mdl::BrushFaceHandle{brushNode, topFaceIndex};
  mdl::selectBrushFaces(map, {faceHandle});

  auto camera = gl::OrthographicCamera{
    1.0f,
    8000.0f,
    gl::Camera::Viewport{0, 0, 1024, 768},
    vm::vec3f{0, 0, 100},
    vm::vec3f{0, 0, -1},
    vm::vec3f{0, 1, 0}};
  auto helper = UvViewHelper{camera};
  helper.setFaceHandle(faceHandle);

  auto tool = UvRotateTool{document, helper};
  auto& controller = static_cast<ToolController&>(tool);

  // matches the private RotateHandleRadius / RotateHandleWidth constants in
  // UvRotateTool.cpp
  constexpr auto RotateHandleRadius = 32.0;

  // a vertical ray hits the (planar, z = 16) face exactly at (x, y, 16)
  const auto rayAt = [](const vm::vec3d& facePoint) {
    return vm::ray3d{{facePoint.x(), facePoint.y(), 100}, {0, 0, -1}};
  };

  const auto pointOnRing = [&] {
    const auto zoom = double(helper.camera().zoom());
    return helper.origin() + vm::vec3d{RotateHandleRadius / zoom, 0, 0};
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
    CHECK(&controller.tool() == static_cast<Tool*>(&tool));
  }

  SECTION("pick")
  {
    SECTION("with an invalid helper, nothing is hit")
    {
      auto emptyCamera = gl::OrthographicCamera{};
      auto emptyHelper = UvViewHelper{emptyCamera};
      auto emptyTool = UvRotateTool{document, emptyHelper};
      auto& emptyController = static_cast<ToolController&>(emptyTool);

      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest(
        PickRequest{vm::ray3d{{0, 0, 100}, {0, 0, -1}}, emptyCamera});
      auto pickResult = mdl::PickResult{};
      emptyController.pick(inputState, pickResult);

      CHECK(pickResult.empty());
    }

    SECTION("hits the rotate ring")
    {
      const auto inputState = inputStateFor(rayAt(pointOnRing()));

      CHECK(inputState.pickResult()
              .first(mdl::HitFilters::type(UvRotateTool::AngleHandleHitType))
              .isMatch());
    }

    SECTION("does not hit at the origin, far from the ring")
    {
      const auto inputState = inputStateFor(rayAt(helper.origin()));

      CHECK(!inputState.pickResult()
               .first(mdl::HitFilters::type(UvRotateTool::AngleHandleHitType))
               .isMatch());
    }

    SECTION("does not hit when the ray misses the face's plane")
    {
      auto inputState = InputState{0.0f, 0.0f};
      inputState.setPickRequest(
        PickRequest{vm::ray3d{{0, 0, 100}, {1, 0, 0}}, helper.camera()});
      auto pickResult = mdl::PickResult{};
      controller.pick(inputState, pickResult);

      CHECK(pickResult.empty());
    }
  }

  SECTION("acceptMouseDrag")
  {
    SECTION("returns nullptr with an unsupported modifier key held")
    {
      auto inputState = inputStateFor(rayAt(pointOnRing()));
      inputState.setModifierKeys(ModifierKeys::Shift);
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("returns nullptr with the wrong mouse button")
    {
      auto inputState = inputStateFor(rayAt(pointOnRing()));
      inputState.mouseDown(MouseButtons::Right);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("returns nullptr without a ring hit and without CtrlCmd")
    {
      auto inputState = inputStateFor(rayAt(helper.origin()));
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("starts a drag on a ring hit")
    {
      auto inputState = inputStateFor(rayAt(pointOnRing()));
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) != nullptr);
    }

    SECTION("with CtrlCmd, starts a drag anywhere on the face")
    {
      auto inputState = inputStateFor(rayAt(helper.origin()));
      inputState.setModifierKeys(ModifierKeys::CtrlCmd);
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) != nullptr);
    }

    SECTION("dragging the ring changes the face's UV rotation")
    {
      auto inputState = inputStateFor(rayAt(pointOnRing()));
      inputState.mouseDown(MouseButtons::Left);

      auto tracker = controller.acceptMouseDrag(inputState);
      REQUIRE(tracker != nullptr);
      REQUIRE(faceHandle.face().uvAttributes().rotation == 0.0f);

      // rotate a quarter turn around the origin
      const auto zoom = double(helper.camera().zoom());
      const auto quarterTurnPoint =
        helper.origin() + vm::vec3d{0, RotateHandleRadius / zoom, 0};
      auto dragInputState = inputStateFor(rayAt(quarterTurnPoint));
      dragInputState.setModifierKeys(ModifierKeys::CtrlCmd);

      CHECK(tracker->update(dragInputState));
      CHECK(faceHandle.face().uvAttributes().rotation != 0.0f);

      SECTION("ending the drag keeps the change")
      {
        const auto rotationAfterDrag = faceHandle.face().uvAttributes().rotation;
        tracker->end(dragInputState);
        CHECK(faceHandle.face().uvAttributes().rotation == rotationAfterDrag);
      }

      SECTION("cancelling the drag reverts the change")
      {
        tracker->cancel();
        CHECK(faceHandle.face().uvAttributes().rotation == 0.0f);
      }
    }
  }
}

} // namespace tb::ui
