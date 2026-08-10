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
#include "gl/PerspectiveCamera.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/Hit.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/NodeHandleManager.h"
#include "mdl/NodeHandles.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/VertexTool.h"

#include "kd/result.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

gl::PerspectiveCamera perspectiveCameraFor(const vm::ray3d& pickRay)
{
  const auto viewport = gl::Camera::Viewport{0, 0, 1920, 1080};
  const auto direction = vm::vec3f{vm::normalize(pickRay.direction)};
  const auto up = vm::abs(direction.z()) < 0.9f ? vm::vec3f{0, 0, 1} : vm::vec3f{0, 1, 0};
  return gl::PerspectiveCamera{
    90.0f, 1.0f, 8000.0f, viewport, vm::vec3f{pickRay.origin}, direction, up};
}

} // namespace

TEST_CASE("VertexTool")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
  auto* brushNode =
    new mdl::BrushNode{builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value()};
  mdl::addNodes(map, {{map.editorContext().currentLayer(), {brushNode}}});
  mdl::selectNodes(map, {brushNode});

  auto tool = VertexTool{document};

  SECTION("activation populates handles for the selected brush")
  {
    REQUIRE(tool.activate());

    CHECK(map.nodeHandles().allHandles<mdl::VertexHandle>().size() == 8u);
    CHECK(map.nodeHandles().allHandles<mdl::EdgeHandle>().size() == 12u);
    CHECK(map.nodeHandles().allHandles<mdl::FaceHandle>().size() == 6u);

    CHECK(tool.deactivate());

    CHECK(map.nodeHandles().allHandles<mdl::VertexHandle>().empty());
    CHECK(map.nodeHandles().allHandles<mdl::EdgeHandle>().empty());
    CHECK(map.nodeHandles().allHandles<mdl::FaceHandle>().empty());
  }

  SECTION("activating with nothing selected, then selecting a node, adds handles")
  {
    mdl::deselectAll(map);

    auto emptyTool = VertexTool{document};
    REQUIRE(emptyTool.activate());
    CHECK(map.nodeHandles().allHandles<mdl::VertexHandle>().empty());

    mdl::selectNodes(map, {brushNode});
    CHECK(map.nodeHandles().allHandles<mdl::VertexHandle>().size() == 8u);
    CHECK(map.nodeHandles().allHandles<mdl::EdgeHandle>().size() == 12u);
    CHECK(map.nodeHandles().allHandles<mdl::FaceHandle>().size() == 6u);
  }

  SECTION("with the tool active")
  {
    REQUIRE(tool.activate());

    SECTION("pick")
    {
      SECTION("hits a vertex")
      {
        // not aimed through the cube's center, so it only passes near one corner
        const auto pickRay =
          vm::ray3d{{100, 100, 50}, vm::normalize(vm::vec3d{-84, -84, -34})};
        auto pickResult = mdl::PickResult{};
        tool.pick(pickRay, perspectiveCameraFor(pickRay), 3.0, pickResult);

        const auto vertexHits =
          pickResult.all(mdl::HitFilters::type(mdl::VertexHandle::HandleHitType));
        REQUIRE(vertexHits.size() == 1u);
        CHECK(
          vertexHits.front().target<mdl::VertexHandle>().position
          == vm::vec3d{16, 16, 16});
      }

      SECTION("hits a face")
      {
        // straight through the cube's center, so this hits both the top and bottom
        // face handles (handle picking isn't occluded by the solid)
        const auto pickRay = vm::ray3d{{0, 0, 500}, {0, 0, -1}};
        auto pickResult = mdl::PickResult{};
        tool.pick(pickRay, perspectiveCameraFor(pickRay), 3.0, pickResult);

        const auto faceHits =
          pickResult.all(mdl::HitFilters::type(mdl::FaceHandle::HandleHitType));
        REQUIRE(faceHits.size() == 2u);
        CHECK(std::ranges::any_of(faceHits, [](const auto& hit) {
          const auto data = hit.template target<mdl::FaceHandle::GridHandleHitData>();
          return std::get<0>(data).position.vertices().size() == 4u;
        }));
      }
    }

    SECTION("findDraggableHandle and collectDraggableHandles")
    {
      const auto vertexHandle = mdl::VertexHandle{vm::vec3d{16, 16, 16}};
      const auto vertexHit = mdl::Hit{
        mdl::VertexHandle::HandleHitType, 0.0, vertexHandle.position, vertexHandle};

      SECTION("a direct vertex hit is returned as-is")
      {
        auto inputState = InputState{0.0f, 0.0f};
        auto pickResult = mdl::PickResult{};
        pickResult.addHit(vertexHit);
        inputState.setPickResult(std::move(pickResult));

        const auto hit =
          tool.findDraggableHandle(inputState, mdl::VertexHandle::HandleHitType);
        CHECK(hit.target<mdl::VertexHandle>() == vertexHandle);

        const auto hits =
          tool.collectDraggableHandles(inputState, mdl::VertexHandle::HandleHitType);
        REQUIRE(hits.size() == 1u);
        CHECK(hits.front().target<mdl::VertexHandle>() == vertexHandle);
      }

      SECTION("without a vertex hit, Shift falls back to a face/edge hit")
      {
        const auto faceHandle = mdl::FaceHandle{
          vm::polygon3d{{{-16, -16, 16}, {16, -16, 16}, {16, 16, 16}, {-16, 16, 16}}}};
        const auto faceHit =
          mdl::Hit{mdl::FaceHandle::HandleHitType, 0.0, vm::vec3d{0, 0, 16}, faceHandle};

        auto inputState = InputState{0.0f, 0.0f};
        inputState.setModifierKeys(ModifierKeys::Shift);
        auto pickResult = mdl::PickResult{};
        pickResult.addHit(faceHit);
        inputState.setPickResult(std::move(pickResult));

        const auto hit =
          tool.findDraggableHandle(inputState, mdl::VertexHandle::HandleHitType);
        CHECK(hit.target<mdl::FaceHandle>() == faceHandle);
      }

      SECTION("without a vertex hit and without Shift, nothing is returned")
      {
        auto inputState = InputState{0.0f, 0.0f};
        auto pickResult = mdl::PickResult{};
        pickResult.addHit(vertexHit);
        // hitType requested does not match what's in the pick result
        inputState.setPickResult(std::move(pickResult));

        const auto hit =
          tool.findDraggableHandle(inputState, mdl::FaceHandle::HandleHitType);
        CHECK(!hit.isMatch());
      }

      SECTION("collectDraggableHandles without a vertex hit")
      {
        const auto edgeHandle =
          mdl::EdgeHandle{vm::segment3d{{16, 16, -16}, {16, 16, 16}}};
        const auto edgeHit =
          mdl::Hit{mdl::EdgeHandle::HandleHitType, 0.0, vm::vec3d{16, 16, 0}, edgeHandle};

        const auto faceHandle = mdl::FaceHandle{
          vm::polygon3d{{{-16, -16, 16}, {16, -16, 16}, {16, 16, 16}, {-16, 16, 16}}}};
        const auto faceHit =
          mdl::Hit{mdl::FaceHandle::HandleHitType, 0.0, vm::vec3d{0, 0, 16}, faceHandle};

        SECTION("Shift with a top edge hit returns all edge hits")
        {
          auto inputState = InputState{0.0f, 0.0f};
          inputState.setModifierKeys(ModifierKeys::Shift);
          auto pickResult = mdl::PickResult{};
          pickResult.addHit(edgeHit);
          inputState.setPickResult(std::move(pickResult));

          const auto hits =
            tool.collectDraggableHandles(inputState, mdl::VertexHandle::HandleHitType);
          REQUIRE(hits.size() == 1u);
          CHECK(hits.front().target<mdl::EdgeHandle>() == edgeHandle);
        }

        SECTION("Shift with a top face hit returns all face hits")
        {
          auto inputState = InputState{0.0f, 0.0f};
          inputState.setModifierKeys(ModifierKeys::Shift);
          auto pickResult = mdl::PickResult{};
          pickResult.addHit(faceHit);
          inputState.setPickResult(std::move(pickResult));

          const auto hits =
            tool.collectDraggableHandles(inputState, mdl::VertexHandle::HandleHitType);
          REQUIRE(hits.size() == 1u);
          CHECK(hits.front().target<mdl::FaceHandle>() == faceHandle);
        }

        SECTION("Shift with an unrelated top hit falls back to face hits")
        {
          // an unrelated hit type (neither vertex, edge, nor face) that is closer
          // (distance 0) than the face hit, so it is the "top" hit but does not
          // itself qualify as a draggable handle
          static const auto OtherHitType = mdl::HitType::freeType();
          const auto otherHit = mdl::Hit{OtherHitType, 0.0, vm::vec3d{0, 0, 0}, 0};
          const auto fartherFaceHit = mdl::Hit{
            mdl::FaceHandle::HandleHitType, 10.0, vm::vec3d{0, 0, 16}, faceHandle};

          auto inputState = InputState{0.0f, 0.0f};
          inputState.setModifierKeys(ModifierKeys::Shift);
          auto pickResult = mdl::PickResult{};
          pickResult.addHit(otherHit);
          pickResult.addHit(fartherFaceHit);
          inputState.setPickResult(std::move(pickResult));

          const auto hits =
            tool.collectDraggableHandles(inputState, mdl::VertexHandle::HandleHitType);
          REQUIRE(hits.size() == 1u);
          CHECK(hits.front().target<mdl::FaceHandle>() == faceHandle);
        }

        SECTION("without Shift, nothing is returned")
        {
          auto inputState = InputState{0.0f, 0.0f};
          auto pickResult = mdl::PickResult{};
          pickResult.addHit(faceHit);
          inputState.setPickResult(std::move(pickResult));

          const auto hits =
            tool.collectDraggableHandles(inputState, mdl::VertexHandle::HandleHitType);
          CHECK(hits.empty());
        }
      }
    }

    SECTION("handlePositionAndHitPoint and getHandlePosition")
    {
      SECTION("vertex hit")
      {
        const auto handle = mdl::VertexHandle{vm::vec3d{16, 16, 16}};
        const auto hit =
          mdl::Hit{mdl::VertexHandle::HandleHitType, 0.0, vm::vec3d{1, 2, 3}, handle};

        const auto [position, hitPoint] = tool.handlePositionAndHitPoint({hit});
        CHECK(position == vm::vec3d{16, 16, 16});
        CHECK(hitPoint == vm::vec3d{1, 2, 3});
        CHECK(tool.getHandlePosition(hit) == vm::vec3d{16, 16, 16});
      }

      SECTION("edge hit")
      {
        const auto handle = mdl::EdgeHandle{vm::segment3d{{16, 16, -16}, {16, 16, 16}}};
        const auto data =
          mdl::EdgeHandle::GridHandleHitData{handle, vm::vec3d{16, 16, 0}};
        const auto hit =
          mdl::Hit{mdl::EdgeHandle::HandleHitType, 0.0, vm::vec3d{1, 2, 3}, data};

        const auto [position, hitPoint] = tool.handlePositionAndHitPoint({hit});
        CHECK(position == vm::vec3d{16, 16, 0});
        CHECK(hitPoint == vm::vec3d{1, 2, 3});
        CHECK(tool.getHandlePosition(hit) == vm::vec3d{16, 16, 0});
      }

      SECTION("face hit")
      {
        const auto handle = mdl::FaceHandle{
          vm::polygon3d{{{-16, -16, 16}, {16, -16, 16}, {16, 16, 16}, {-16, 16, 16}}}};
        const auto data = mdl::FaceHandle::GridHandleHitData{handle, vm::vec3d{0, 0, 16}};
        const auto hit =
          mdl::Hit{mdl::FaceHandle::HandleHitType, 0.0, vm::vec3d{1, 2, 3}, data};

        const auto [position, hitPoint] = tool.handlePositionAndHitPoint({hit});
        CHECK(position == vm::vec3d{0, 0, 16});
        CHECK(hitPoint == vm::vec3d{1, 2, 3});
        CHECK(tool.getHandlePosition(hit) == vm::vec3d{0, 0, 16});
      }
    }

    SECTION("moving a vertex")
    {
      const auto handle = mdl::VertexHandle{vm::vec3d{16, 16, 16}};
      const auto hit =
        mdl::Hit{mdl::VertexHandle::HandleHitType, 0.0, handle.position, handle};

      REQUIRE(tool.startMove({hit}));
      CHECK(tool.actionName() == "Move Vertex");

      SECTION("a successful move updates the brush geometry")
      {
        CHECK(tool.move(vm::vec3d{0, 0, -8}) == VertexTool::MoveResult::Continue);
        tool.endMove();

        CHECK(brushNode->brush().hasVertex(vm::vec3d{16, 16, 8}));
        CHECK(!brushNode->brush().hasVertex(vm::vec3d{16, 16, 16}));
      }

      SECTION("cancelling a move restores the original geometry")
      {
        CHECK(tool.move(vm::vec3d{0, 0, -8}) == VertexTool::MoveResult::Continue);
        tool.cancelMove();

        CHECK(brushNode->brush().hasVertex(vm::vec3d{16, 16, 16}));
      }
    }

    SECTION("splitting an edge")
    {
      const auto edgeHandle = mdl::EdgeHandle{vm::segment3d{{16, 16, -16}, {16, 16, 16}}};
      const auto data =
        mdl::EdgeHandle::GridHandleHitData{edgeHandle, vm::vec3d{16, 16, 0}};
      const auto hit =
        mdl::Hit{mdl::EdgeHandle::HandleHitType, 0.0, vm::vec3d{16, 16, 0}, data};

      REQUIRE(tool.startMove({hit}));
      CHECK(tool.actionName() == "Split Edge");
      CHECK(map.nodeHandles().selectedHandleCount<mdl::EdgeHandle>() == 1u);

      const auto originalVertexCount = brushNode->brush().vertexCount();
      CHECK(tool.move(vm::vec3d{8, 0, 0}) == VertexTool::MoveResult::Continue);
      tool.endMove();

      CHECK(brushNode->brush().vertexCount() == originalVertexCount + 1);
      CHECK(brushNode->brush().hasVertex(vm::vec3d{24, 16, 0}));
    }

    SECTION("splitting a face")
    {
      const auto faceHandle = mdl::FaceHandle{
        vm::polygon3d{{{-16, -16, 16}, {16, -16, 16}, {16, 16, 16}, {-16, 16, 16}}}};
      const auto data =
        mdl::FaceHandle::GridHandleHitData{faceHandle, vm::vec3d{0, 0, 16}};
      const auto hit =
        mdl::Hit{mdl::FaceHandle::HandleHitType, 0.0, vm::vec3d{0, 0, 16}, data};

      REQUIRE(tool.startMove({hit}));
      CHECK(tool.actionName() == "Split Face");
      CHECK(map.nodeHandles().selectedHandleCount<mdl::FaceHandle>() == 1u);

      const auto originalVertexCount = brushNode->brush().vertexCount();
      CHECK(tool.move(vm::vec3d{0, 0, 8}) == VertexTool::MoveResult::Continue);
      tool.endMove();

      CHECK(brushNode->brush().vertexCount() == originalVertexCount + 1);
      CHECK(brushNode->brush().hasVertex(vm::vec3d{0, 0, 24}));
    }

    SECTION("allowAbsoluteSnapping is true")
    {
      CHECK(tool.allowAbsoluteSnapping());
    }

    SECTION("deselectAll")
    {
      CHECK(!tool.deselectAll());

      map.nodeHandles().selectHandle<mdl::VertexHandle>(
        mdl::VertexHandle{vm::vec3d{16, 16, 16}});
      CHECK(tool.deselectAll());
      CHECK(!map.nodeHandles().anyHandleSelected<mdl::VertexHandle>());
    }

    SECTION("removeSelection removes the selected vertex")
    {
      map.nodeHandles().selectHandle<mdl::VertexHandle>(
        mdl::VertexHandle{vm::vec3d{16, 16, 16}});
      REQUIRE(tool.canRemoveSelection());

      tool.removeSelection();

      CHECK(!brushNode->brush().hasVertex(vm::vec3d{16, 16, 16}));
      CHECK(brushNode->brush().vertexCount() == 7u);
    }

    SECTION("actionName reflects the selected vertex count")
    {
      // zero selected handles is plural, matching kdl::str_plural(0, ...)
      CHECK(tool.actionName() == "Move Vertices");

      map.nodeHandles().selectHandle<mdl::VertexHandle>(
        mdl::VertexHandle{vm::vec3d{16, 16, 16}});
      CHECK(tool.actionName() == "Move Vertex");

      map.nodeHandles().selectHandle<mdl::VertexHandle>(
        mdl::VertexHandle{vm::vec3d{-16, 16, 16}});
      CHECK(tool.actionName() == "Move Vertices");
    }
  }
}

} // namespace tb::ui
