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
#include "mdl/EditorContext.h"
#include "mdl/Hit.h"
#include "mdl/LayerNode.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PickResult.h"
#include "mdl/UvAttributes.h"
#include "mdl/WorldNode.h"
#include "ui/CatchConfig.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/PickRequest.h"
#include "ui/ToolController.h"
#include "ui/UvScaleTool.h"
#include "ui/UvViewHelper.h"

#include "kd/result.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("UvScaleTool")
{
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

  auto tool = UvScaleTool{document, helper};
  auto& controller = static_cast<ToolController&>(tool);

  const auto inputStateFor = [&](const vm::ray3d& ray) {
    auto inputState = InputState{};
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

  SECTION("pick")
  {
    SECTION("with an invalid helper, nothing is hit")
    {
      auto emptyCamera = gl::OrthographicCamera{};
      auto emptyHelper = UvViewHelper{emptyCamera};
      auto emptyTool = UvScaleTool{document, emptyHelper};
      auto& emptyController = static_cast<ToolController&>(emptyTool);

      auto inputState = InputState{};
      inputState.setPickRequest(
        PickRequest{vm::ray3d{{0, 0, 100}, {0, 0, -1}}, emptyCamera});

      auto pickResult = mdl::PickResult{};
      emptyController.pick(inputState, pickResult);
      CHECK(pickResult.empty());
    }

    SECTION("hits both scale handles at a grid intersection")
    {
      const auto inputState = inputStateFor(vm::ray3d{{0, 0, 100}, {0, 0, -1}});

      CHECK(
        inputState.pickResult()
          .all(mdl::HitFilters::type(UvScaleTool::XHandleHitType))
          .size()
        == 1u);
      CHECK(
        inputState.pickResult()
          .all(mdl::HitFilters::type(UvScaleTool::YHandleHitType))
          .size()
        == 1u);
    }
  }

  SECTION("acceptMouseDrag")
  {
    SECTION("returns nullptr with a modifier key held")
    {
      auto inputState = inputStateFor(vm::ray3d{{0, 0, 100}, {0, 0, -1}});
      inputState.setModifierKeys(ModifierKeys::Shift);
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("returns nullptr with the wrong mouse button")
    {
      auto inputState = inputStateFor(vm::ray3d{{0, 0, 100}, {0, 0, -1}});
      inputState.mouseDown(MouseButtons::Right);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("returns nullptr when no handle is hit")
    {
      // far from any grid intersection
      auto inputState = inputStateFor(vm::ray3d{{32, 32, 100}, {0, 0, -1}});
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("returns nullptr when the pick ray does not hit the face's plane")
    {
      // a handle hit is present, but the ray is parallel to the face's boundary
      auto inputState = InputState{};
      inputState.setPickRequest(
        PickRequest{vm::ray3d{{0, 0, 100}, {1, 0, 0}}, helper.camera()});
      auto pickResult = mdl::PickResult{};
      pickResult.addHit(
        mdl::Hit{UvScaleTool::XHandleHitType, 0.0, vm::vec3d{0, 0, 100}, 0});
      inputState.setPickResult(std::move(pickResult));
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("dragging a handle changes the face's UV scale or offset")
    {
      auto inputState = inputStateFor(vm::ray3d{{0, 0, 100}, {0, 0, -1}});
      inputState.mouseDown(MouseButtons::Left);

      auto tracker = controller.acceptMouseDrag(inputState);
      REQUIRE(tracker != nullptr);

      REQUIRE(faceHandle.face().uvAttributes().scale == vm::vec2f{1, 1});
      REQUIRE(faceHandle.face().uvAttributes().offset == vm::vec2f{0, 0});

      const auto dragInputState = inputStateFor(vm::ray3d{{32, 0, 100}, {0, 0, -1}});
      CHECK(tracker->update(dragInputState));

      const auto draggedScale = faceHandle.face().uvAttributes().scale;
      const auto draggedOffset = faceHandle.face().uvAttributes().offset;
      CHECK((draggedScale != vm::vec2f{1, 1} || draggedOffset != vm::vec2f{0, 0}));

      SECTION("ending the drag keeps the change")
      {
        tracker->end(dragInputState);
        CHECK(faceHandle.face().uvAttributes().scale == draggedScale);
        CHECK(faceHandle.face().uvAttributes().offset == draggedOffset);
      }

      SECTION("cancelling the drag reverts the change")
      {
        tracker->cancel();
        CHECK(faceHandle.face().uvAttributes().scale == vm::vec2f{1, 1});
        CHECK(faceHandle.face().uvAttributes().offset == vm::vec2f{0, 0});
      }
    }
  }
}

} // namespace tb::ui
