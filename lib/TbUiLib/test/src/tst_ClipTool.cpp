/*
 Copyright (C) 2024 Kristian Duske
 Copyright (C) 2019 Eric Wasylishen

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
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_CopyPaste.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PasteType.h"
#include "mdl/PickResult.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"
#include "ui/CatchConfig.h"
#include "ui/ClipTool.h"
#include "ui/ClipToolController.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

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

TEST_CASE("ClipTool")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  SECTION("Clipped brushes get new link IDs")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/4461
    const auto data = R"(// entity 0
{
"mapversion" "220"
"wad" ""
"classname" "worldspawn"
// brush 0
{
( -64 -64 -16 ) ( -64 -63 -16 ) ( -64 -64 -15 ) __TB_empty [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -64 -64 -15 ) ( -63 -64 -16 ) __TB_empty [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -64 -64 -16 ) ( -63 -64 -16 ) ( -64 -63 -16 ) __TB_empty [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 64 65 16 ) ( 65 64 16 ) __TB_empty [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 64 64 16 ) ( 65 64 16 ) ( 64 64 17 ) __TB_empty [ -1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 64 64 16 ) ( 64 64 17 ) ( 64 65 16 ) __TB_empty [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
}
}
)";
    REQUIRE(paste(map, data) == mdl::PasteType::Node);

    const auto* defaultLayer = map.worldNode().defaultLayer();

    const auto* originalBrushNode =
      dynamic_cast<const mdl::BrushNode*>(defaultLayer->children().front());
    REQUIRE(originalBrushNode);

    const auto originalLinkId = originalBrushNode->linkId();

    auto tool = ClipTool{document};
    REQUIRE(tool.activate());

    tool.addPoint(vm::vec3d{0, 16, 16}, {});
    tool.addPoint(vm::vec3d{0, -16, 16}, {});
    tool.addPoint(vm::vec3d{0, -64, 0}, {});

    REQUIRE(tool.canClip());
    tool.toggleSide();
    tool.performClip();

    REQUIRE(defaultLayer->childCount() == 2);
    const auto* clippedBrushNode1 =
      dynamic_cast<const mdl::BrushNode*>(defaultLayer->children().front());
    const auto* clippedBrushNode2 =
      dynamic_cast<const mdl::BrushNode*>(defaultLayer->children().back());

    REQUIRE(clippedBrushNode1);
    REQUIRE(clippedBrushNode2);

    CHECK(clippedBrushNode1->linkId() != originalLinkId);
    CHECK(clippedBrushNode2->linkId() != originalLinkId);
    CHECK(clippedBrushNode1->linkId() != clippedBrushNode2->linkId());
  }

  SECTION("hasBrushes")
  {
    auto tool = ClipTool{document};
    CHECK(!tool.hasBrushes());

    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* brushNode = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {brushNode}}});
    mdl::selectNodes(map, {brushNode});

    CHECK(tool.hasBrushes());
  }

  SECTION("defaultClipPointPos")
  {
    auto tool = ClipTool{document};
    CHECK(!tool.defaultClipPointPos().has_value());

    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* brushNode = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {brushNode}}});
    mdl::selectNodes(map, {brushNode});

    CHECK(tool.defaultClipPointPos() == vm::vec3d{0, 0, 0});
  }

  SECTION("adding and removing points")
  {
    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* brushNode = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {brushNode}}});
    mdl::selectNodes(map, {brushNode});

    auto tool = ClipTool{document};
    REQUIRE(tool.activate());

    CHECK(!tool.hasPoints());
    CHECK(!tool.canRemoveLastPoint());
    CHECK(!tool.removeLastPoint());
    CHECK(tool.canAddPoint({0, 0, 0}));

    tool.addPoint(vm::vec3d{-16, -16, 16}, {});
    CHECK(tool.hasPoints());
    CHECK(tool.canRemoveLastPoint());
    // adding the same point again is rejected
    CHECK(!tool.canAddPoint(vm::vec3d{-16, -16, 16}));

    tool.addPoint(vm::vec3d{16, -16, 16}, {});
    // a third point that is colinear with the first two is rejected
    CHECK(!tool.canAddPoint(vm::vec3d{0, -16, 16}));
    CHECK(!tool.canClip());

    tool.addPoint(vm::vec3d{16, 16, 16}, {});
    CHECK(tool.canClip());

    CHECK(tool.removeLastPoint());
    CHECK(!tool.canClip());
    CHECK(tool.hasPoints());

    CHECK(tool.removeLastPoint());
    CHECK(tool.removeLastPoint());
    CHECK(!tool.hasPoints());
    CHECK(!tool.canRemoveLastPoint());
  }

  SECTION("dragging a point")
  {
    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* brushNode = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {brushNode}}});
    mdl::selectNodes(map, {brushNode});

    auto tool = ClipTool{document};
    REQUIRE(tool.activate());

    constexpr auto point0 = vm::vec3d{-16, -16, 16};
    constexpr auto point1 = vm::vec3d{16, -16, 16};
    tool.addPoint(point0, {});
    tool.addPoint(point1, {});

    const auto pick = [&](const vm::vec3d& at) {
      const auto pickRay = vm::ray3d{at + vm::vec3d{0, 0, 100}, {0, 0, -1}};
      auto pickResult = mdl::PickResult{};
      tool.pick(pickRay, perspectiveCameraFor(pickRay), pickResult);
      return pickResult;
    };

    auto pickResult = pick(point0);
    REQUIRE(pickResult.all().size() == 1);
    REQUIRE(pickResult.all().front().target<size_t>() == 0u);

    SECTION("dragging moves the point")
    {
      const auto handleAndHitPoint = tool.beginDragPoint(pickResult);
      REQUIRE(handleAndHitPoint);
      CHECK(std::get<0>(*handleAndHitPoint) == point0);

      constexpr auto newPos = vm::vec3d{-16, 16, 16};
      CHECK(tool.dragPoint(newPos, {}));
      tool.endDragPoint();

      // the point moved: picking at the old position no longer hits index 0,
      // picking at the new position does
      CHECK(pick(point0).empty());
      const auto afterDrag = pick(newPos);
      REQUIRE(afterDrag.all().size() == 1);
      CHECK(afterDrag.all().front().target<size_t>() == 0u);
    }

    SECTION("dragging onto another point is rejected")
    {
      REQUIRE(tool.beginDragPoint(pickResult));
      CHECK(!tool.dragPoint(point1, {}));
      tool.endDragPoint();

      // the point did not move
      REQUIRE(pick(point0).all().size() == 1);
    }

    SECTION("cancelling a drag restores the original position")
    {
      REQUIRE(tool.beginDragPoint(pickResult));
      CHECK(tool.dragPoint(vm::vec3d{-16, 16, 16}, {}));
      tool.cancelDragPoint();

      REQUIRE(pick(point0).all().size() == 1);
    }

    SECTION("beginDragLastPoint targets the most recently added point")
    {
      const auto lastPointPick = pick(point1);
      REQUIRE(lastPointPick.all().size() == 1);
      REQUIRE(lastPointPick.all().front().target<size_t>() == 1u);

      tool.beginDragLastPoint();
      CHECK(tool.dragPoint(vm::vec3d{16, 16, 16}, {}));
      tool.endDragPoint();

      CHECK(pick(point1).empty());
    }
  }

  SECTION("setFace and reset")
  {
    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* brushNode = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {brushNode}}});
    mdl::selectNodes(map, {brushNode});

    auto tool = ClipTool{document};
    REQUIRE(tool.activate());

    CHECK(!tool.reset());
    CHECK(!tool.canClip());

    tool.setFace(mdl::BrushFaceHandle{brushNode, 0});
    CHECK(tool.canClip());

    // a face clip has no draggable points of its own
    CHECK(!tool.hasPoints());
    CHECK(!tool.canAddPoint({0, 0, 0}));
    CHECK(!tool.canRemoveLastPoint());
    CHECK(!tool.removeLastPoint());

    auto pickResult = mdl::PickResult{};
    tool.pick(
      vm::ray3d{{0, 0, 100}, {0, 0, -1}},
      perspectiveCameraFor({{0, 0, 100}, {0, 0, -1}}),
      pickResult);
    CHECK(pickResult.empty());
    CHECK(!tool.beginDragPoint(pickResult));

    CHECK(tool.reset());
    CHECK(!tool.canClip());
  }

  SECTION("deactivating clears the strategy")
  {
    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* brushNode = new mdl::BrushNode{
      builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {brushNode}}});
    mdl::selectNodes(map, {brushNode});

    auto tool = ClipTool{document};
    REQUIRE(tool.activate());

    tool.addPoint(vm::vec3d{-16, -16, 16}, {});
    REQUIRE(tool.hasPoints());

    CHECK(tool.deactivate());
    CHECK(!tool.hasPoints());
  }

  SECTION("activation requires a selection of only brushes")
  {
    auto tool = ClipTool{document};
    CHECK(!tool.activate());

    auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {entityNode}}});
    mdl::selectNodes(map, {entityNode});

    auto toolWithEntitySelected = ClipTool{document};
    CHECK(!toolWithEntitySelected.activate());
  }
}

} // namespace tb::ui
