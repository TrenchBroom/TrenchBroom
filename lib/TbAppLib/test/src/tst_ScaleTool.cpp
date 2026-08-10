/*
 Copyright (C) 2010 Kristian Duske
 Copyright (C) 2018 Eric Wasylishen

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
#include "gl/OrthographicCamera.h"
#include "gl/PerspectiveCamera.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h" // IWYU pragma: keep
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityNode.h"
#include "mdl/Hit.h"
#include "mdl/LayerNode.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h" // IWYU pragma: keep
#include "mdl/PickResult.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/ScaleTool.h"

#include "kd/ranges/to.h"
#include "kd/result.h"

#include "vm/approx.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::ui
{
namespace
{

/**
 * Build a camera positioned at the pick ray's origin looking along its direction, so
 * that the handle radius scaling used by picking is realistic.
 */
vm::vec3f upFor(const vm::vec3f& direction)
{
  return vm::abs(direction.z()) < 0.9f ? vm::vec3f{0, 0, 1} : vm::vec3f{0, 1, 0};
}

gl::PerspectiveCamera perspectiveCameraFor(const vm::ray3d& pickRay)
{
  const auto viewport = gl::Camera::Viewport{0, 0, 1920, 1080};
  const auto direction = vm::vec3f{vm::normalize(pickRay.direction)};
  return gl::PerspectiveCamera{
    90.0f,
    1.0f,
    8000.0f,
    viewport,
    vm::vec3f{pickRay.origin},
    direction,
    upFor(direction)};
}

gl::OrthographicCamera orthographicCameraFor(const vm::ray3d& pickRay)
{
  const auto viewport = gl::Camera::Viewport{0, 0, 1920, 1080};
  const auto direction = vm::vec3f{vm::normalize(pickRay.direction)};
  return gl::OrthographicCamera{
    1.0f, 8000.0f, viewport, vm::vec3f{pickRay.origin}, direction, upFor(direction)};
}

} // namespace

TEST_CASE("ScaleTool")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
  auto* brushNode = mdl::createBrushNode(map);
  auto* patchNode = mdl::createPatchNode("some_material");

  auto nodes = std::vector<mdl::Node*>{entityNode, brushNode, patchNode};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), nodes}});

  constexpr size_t iEntityNode = 0;
  constexpr size_t iBrushNode = 1;
  constexpr size_t iPatchNode = 2;

  auto tool = ScaleTool{document};

  SECTION("applies")
  {
    const auto [nodesIndicesToSelect, expectedApplies] =
      GENERATE(table<std::vector<size_t>, bool>({
        {std::vector<size_t>{}, false},
        {std::vector<size_t>{iEntityNode}, true},
        {std::vector<size_t>{iBrushNode}, true},
        {std::vector<size_t>{iPatchNode}, true},
        {std::vector<size_t>{iEntityNode, iBrushNode, iPatchNode}, true},
      }));

    CAPTURE(nodesIndicesToSelect);

    const auto nodesToSelect =
      nodesIndicesToSelect
      | std::views::transform([&](const auto i) -> mdl::Node* { return nodes[i]; })
      | kdl::ranges::to<std::vector>();

    mdl::selectNodes(map, nodesToSelect);
    CHECK(tool.applies() == expectedApplies);
  }

  SECTION("bounds")
  {
    constexpr auto brushBounds = vm::bbox3d{16.0};
    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* boundedBrush =
      new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {boundedBrush}}});
    mdl::selectNodes(map, {boundedBrush});

    CHECK(tool.bounds() == brushBounds);
  }

  SECTION("pick2D")
  {
    constexpr auto brushBounds = vm::bbox3d{16.0};
    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* boundedBrush =
      new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {boundedBrush}}});
    mdl::selectNodes(map, {boundedBrush});

    SECTION("origin inside bounds is ignored")
    {
      const auto pickRay = vm::ray3d{{0, 0, 0}, {0, 0, -1}};
      auto pickResult = mdl::PickResult{};
      tool.pick2D(pickRay, orthographicCameraFor(pickRay), pickResult);
      CHECK(pickResult.empty());
    }

    SECTION("hits a vertical edge endpoint")
    {
      // vertical (Z-parallel) edges are the only ones considered in 2D views
      const auto pickRay = vm::ray3d{{16, 16, 1000}, {0, 0, -1}};
      auto pickResult = mdl::PickResult{};
      tool.pick2D(pickRay, orthographicCameraFor(pickRay), pickResult);

      REQUIRE(pickResult.all().size() == 1);
      const auto& hit = pickResult.all().front();
      CHECK(hit.type() == ScaleTool::EdgeHitType);
      CHECK(hit.hitPoint() == vm::vec3d{16, 16, 22});
    }

    SECTION("falls back to a back side when nothing is hit directly")
    {
      const auto pickRay = vm::ray3d{{16, 200, 1000}, {0, 0, -1}};
      auto pickResult = mdl::PickResult{};
      tool.pick2D(pickRay, orthographicCameraFor(pickRay), pickResult);

      REQUIRE(pickResult.all().size() == 1);
      const auto& hit = pickResult.all().front();
      CHECK(hit.type() == ScaleTool::SideHitType);
      CHECK(hit.hitPoint() == vm::vec3d{16, 200, 16});
    }
  }

  SECTION("pick3D")
  {
    constexpr auto brushBounds = vm::bbox3d{16.0};
    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* boundedBrush =
      new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {boundedBrush}}});
    mdl::selectNodes(map, {boundedBrush});

    SECTION("origin inside bounds is ignored")
    {
      const auto pickRay = vm::ray3d{{0, 0, 0}, {0, 0, -1}};
      auto pickResult = mdl::PickResult{};
      tool.pick3D(pickRay, perspectiveCameraFor(pickRay), pickResult);
      CHECK(pickResult.empty());
    }

    SECTION("hits a corner")
    {
      const auto pickRay =
        vm::ray3d{{100, 100, 100}, vm::normalize(vm::vec3d{-1, -1, -1})};
      auto pickResult = mdl::PickResult{};
      tool.pick3D(pickRay, perspectiveCameraFor(pickRay), pickResult);

      REQUIRE(pickResult.all().size() == 1);
      const auto& hit = pickResult.all().front();
      CHECK(hit.type() == ScaleTool::CornerHitType);
      CHECK(hit.target<BBoxCorner>() == BBoxCorner{{1, 1, 1}});
      // pickPointHandle intersects a sphere around the handle, so the hit point is the
      // near intersection, not the handle position itself
      CHECK(hit.hitPoint() == vm::approx{vm::vec3d{17.8667, 17.8667, 17.8667}});
    }

    SECTION("hits an edge")
    {
      const auto pickRay = vm::ray3d{{16, 0, 500}, {0, 0, -1}};
      auto pickResult = mdl::PickResult{};
      tool.pick3D(pickRay, perspectiveCameraFor(pickRay), pickResult);

      REQUIRE(pickResult.all().size() == 1);
      const auto& hit = pickResult.all().front();
      CHECK(hit.type() == ScaleTool::EdgeHitType);
      // the ray only varies in Z, so X/Y are exact even though the sphere intersection
      // offsets the hit point from the handle position along the ray
      CHECK(hit.hitPoint() == vm::approx{vm::vec3d{16, 0, 21.3778}});
    }

    SECTION("hits a side")
    {
      const auto pickRay = vm::ray3d{{0, 0, 500}, {0, 0, -1}};
      auto pickResult = mdl::PickResult{};
      tool.pick3D(pickRay, perspectiveCameraFor(pickRay), pickResult);

      REQUIRE(pickResult.all().size() == 1);
      const auto& hit = pickResult.all().front();
      CHECK(hit.type() == ScaleTool::SideHitType);
      CHECK(hit.target<BBoxSide>() == BBoxSide{{0, 0, 1}});
      CHECK(hit.hitPoint() == vm::approx{vm::vec3d{0, 0, 16}});
    }

    SECTION("falls back to a back side when nothing is hit directly")
    {
      const auto pickRay = vm::ray3d{{5, 5, 17}, {0, 0, 1}};
      auto pickResult = mdl::PickResult{};
      tool.pick3D(pickRay, perspectiveCameraFor(pickRay), pickResult);

      REQUIRE(pickResult.all().size() == 1);
      const auto& hit = pickResult.all().front();
      CHECK(hit.type() == ScaleTool::SideHitType);
      CHECK(hit.hitPoint() == vm::approx{vm::vec3d{5, 5, 17}});
    }
  }

  SECTION("polygonsHighlightedByDrag, drag handle accessors and drag anchor")
  {
    constexpr auto brushBounds = vm::bbox3d{16.0};
    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* boundedBrush =
      new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {boundedBrush}}});
    mdl::selectNodes(map, {boundedBrush});

    SECTION("no drag hit")
    {
      CHECK(tool.polygonsHighlightedByDrag().empty());
      CHECK(!tool.hasDragSide());
      CHECK(!tool.hasDragEdge());
      CHECK(!tool.hasDragCorner());
      CHECK(!tool.hasDragAnchor());
    }

    SECTION("side drag hit")
    {
      auto pickResult = mdl::PickResult{};
      pickResult.addHit(
        mdl::Hit{ScaleTool::SideHitType, 0.0, {0, 0, 16}, BBoxSide{{0, 0, 1}}});
      tool.updatePickedHandle(pickResult);

      CHECK(tool.hasDragSide());
      CHECK(tool.dragSide().vertexCount() == 4);
      CHECK(!tool.hasDragEdge());
      CHECK(!tool.hasDragCorner());
      CHECK(tool.hasDragAnchor());
      CHECK(tool.dragAnchor() == vm::vec3f{0, 0, -16});
      CHECK(!tool.polygonsHighlightedByDrag().empty());

      SECTION("center anchor also highlights the opposite side")
      {
        tool.setAnchorPos(AnchorPos::Center);
        CHECK(tool.dragAnchor() == vm::vec3f{0, 0, 0});
        CHECK(tool.polygonsHighlightedByDrag().size() == 2);
      }
    }

    SECTION("edge drag hit")
    {
      const auto edge = BBoxEdge{{1, 1, -1}, {1, 1, 1}};
      auto pickResult = mdl::PickResult{};
      pickResult.addHit(mdl::Hit{ScaleTool::EdgeHitType, 0.0, {16, 16, 0}, edge});
      tool.updatePickedHandle(pickResult);

      CHECK(!tool.hasDragSide());
      CHECK(tool.hasDragEdge());
      CHECK(tool.dragEdge() == vm::segment3f{{16, 16, -16}, {16, 16, 16}});
      CHECK(!tool.hasDragCorner());
      CHECK(tool.hasDragAnchor());
      CHECK(tool.dragAnchor() == vm::vec3f{-16, -16, 0});
      // the highlighted sides are the two sides touching this edge
      CHECK(tool.polygonsHighlightedByDrag().size() == 2);
    }

    SECTION("corner drag hit")
    {
      const auto corner = BBoxCorner{{1, 1, 1}};
      auto pickResult = mdl::PickResult{};
      pickResult.addHit(mdl::Hit{ScaleTool::CornerHitType, 0.0, {16, 16, 16}, corner});
      tool.updatePickedHandle(pickResult);

      CHECK(!tool.hasDragSide());
      CHECK(!tool.hasDragEdge());
      CHECK(tool.hasDragCorner());
      CHECK(tool.dragCorner() == vm::vec3f{16, 16, 16});
      CHECK(tool.hasDragAnchor());
      CHECK(tool.dragAnchor() == vm::vec3f{-16, -16, -16});
      // the highlighted sides are the three sides touching this corner
      CHECK(tool.polygonsHighlightedByDrag().size() == 3);
    }
  }

  SECTION("cornerHandles")
  {
    SECTION("selected brush has 8 corner handles")
    {
      constexpr auto brushBounds = vm::bbox3d{16.0};
      auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
      auto* boundedBrush =
        new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};
      mdl::addNodes(map, {{map.editorContext().currentLayer(), {boundedBrush}}});
      mdl::selectNodes(map, {boundedBrush});

      CHECK(tool.cornerHandles().size() == 8);
    }
  }

  SECTION(
    "updatePickedHandle does not refresh views for a repeated hit on the same handle")
  {
    constexpr auto brushBounds = vm::bbox3d{16.0};
    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* boundedBrush =
      new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {boundedBrush}}});
    mdl::selectNodes(map, {boundedBrush});

    auto refreshCount = 0;
    auto connection = tool.refreshViewsNotifier.connect([&](Tool&) { ++refreshCount; });

    auto pickResult = mdl::PickResult{};
    pickResult.addHit(
      mdl::Hit{ScaleTool::SideHitType, 0.0, {0, 0, 16}, BBoxSide{{0, 0, 1}}});
    tool.updatePickedHandle(pickResult);
    CHECK(refreshCount == 1);

    // hitting the same side again should not trigger another refresh
    tool.updatePickedHandle(pickResult);
    CHECK(refreshCount == 1);

    // hitting a different side should trigger a refresh
    auto otherPickResult = mdl::PickResult{};
    otherPickResult.addHit(
      mdl::Hit{ScaleTool::SideHitType, 0.0, {16, 0, 0}, BBoxSide{{1, 0, 0}}});
    tool.updatePickedHandle(otherPickResult);
    CHECK(refreshCount == 2);
  }

  SECTION("anchor position and proportional axes")
  {
    CHECK(tool.anchorPos() == AnchorPos::Opposite);
    tool.setAnchorPos(AnchorPos::Center);
    CHECK(tool.anchorPos() == AnchorPos::Center);

    CHECK(!tool.proportionalAxes().isAxisProportional(0));
    tool.setProportionalAxes(ProportionalAxes::All());
    CHECK(tool.proportionalAxes().allAxesProportional());
  }

  SECTION("scale lifecycle")
  {
    constexpr auto brushBounds = vm::bbox3d{16.0};
    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* boundedBrush =
      new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {boundedBrush}}});
    mdl::selectNodes(map, {boundedBrush});

    const auto hit =
      mdl::Hit{ScaleTool::SideHitType, 0.0, {16, 0, 0}, BBoxSide{{1, 0, 0}}};

    SECTION("scaling and committing changes the selection bounds")
    {
      tool.startScaleWithHit(hit);
      CHECK(tool.dragStartHit().type() == ScaleTool::SideHitType);
      CHECK(tool.bboxAtDragStart() == brushBounds);

      tool.scaleByDelta({25, 0, 0});
      CHECK(tool.bounds() == vm::bbox3d{{-16, -16, -16}, {41, 16, 16}});

      tool.commitScale();
      CHECK(tool.bounds() == vm::bbox3d{{-16, -16, -16}, {41, 16, 16}});
    }

    SECTION("committing with zero delta cancels the transaction")
    {
      tool.startScaleWithHit(hit);
      tool.commitScale();

      CHECK(tool.bounds() == brushBounds);
    }

    SECTION("cancelling discards the scale")
    {
      tool.startScaleWithHit(hit);
      tool.scaleByDelta({25, 0, 0});
      tool.cancelScale();

      CHECK(tool.bounds() == brushBounds);
    }
  }
}

TEST_CASE("BBoxSide")
{
  SECTION("valid normals")
  {
    CHECK(BBoxSide{{1, 0, 0}}.normal == vm::vec3d{1, 0, 0});
    CHECK(BBoxSide{{-1, 0, 0}}.normal == vm::vec3d{-1, 0, 0});
    CHECK(BBoxSide{{0, 1, 0}}.normal == vm::vec3d{0, 1, 0});
    CHECK(BBoxSide{{0, 0, -1}}.normal == vm::vec3d{0, 0, -1});
  }

  SECTION("invalid normals throw")
  {
    CHECK_THROWS_AS((BBoxSide{{0, 0, 0}}), std::invalid_argument);
    CHECK_THROWS_AS((BBoxSide{{1, 1, 0}}), std::invalid_argument);
    CHECK_THROWS_AS((BBoxSide{{0.5, 0, 0}}), std::invalid_argument);
  }

  SECTION("oppositeSide")
  {
    CHECK(oppositeSide(BBoxSide{{1, 0, 0}}) == BBoxSide{{-1, 0, 0}});
    CHECK(oppositeSide(BBoxSide{{0, -1, 0}}) == BBoxSide{{0, 1, 0}});
  }
}

TEST_CASE("BBoxCorner")
{
  SECTION("valid corners")
  {
    CHECK(BBoxCorner{{1, 1, 1}}.corner == vm::vec3d{1, 1, 1});
    CHECK(BBoxCorner{{-1, 1, -1}}.corner == vm::vec3d{-1, 1, -1});
  }

  SECTION("invalid corners throw")
  {
    CHECK_THROWS_AS((BBoxCorner{{1, 1, 0}}), std::invalid_argument);
    CHECK_THROWS_AS((BBoxCorner{{0.5, 1, 1}}), std::invalid_argument);
  }

  SECTION("oppositeCorner")
  {
    CHECK(oppositeCorner(BBoxCorner{{1, 1, 1}}) == BBoxCorner{{-1, -1, -1}});
    CHECK(oppositeCorner(BBoxCorner{{1, -1, 1}}) == BBoxCorner{{-1, 1, -1}});
  }
}

TEST_CASE("BBoxEdge")
{
  SECTION("valid edges")
  {
    const auto edge = BBoxEdge{{1, 1, 1}, {1, 1, -1}};
    CHECK(edge.point0 == vm::vec3d{1, 1, 1});
    CHECK(edge.point1 == vm::vec3d{1, 1, -1});
  }

  SECTION("invalid endpoints throw")
  {
    CHECK_THROWS_AS((BBoxEdge{{1, 1, 0}, {1, 1, -1}}), std::invalid_argument);
    CHECK_THROWS_AS((BBoxEdge{{1, 1, 1}, {1, 1, 0}}), std::invalid_argument);
  }

  SECTION("oppositeEdge")
  {
    CHECK(
      oppositeEdge(BBoxEdge{{1, 1, 1}, {1, 1, -1}})
      == BBoxEdge{{-1, -1, -1}, {-1, -1, 1}});
  }
}

TEST_CASE("ProportionalAxes")
{
  SECTION("All and None")
  {
    CHECK(ProportionalAxes::All().allAxesProportional());
    CHECK(!ProportionalAxes::None().allAxesProportional());
    for (const auto axis : {0u, 1u, 2u})
    {
      CHECK(ProportionalAxes::All().isAxisProportional(axis));
      CHECK(!ProportionalAxes::None().isAxisProportional(axis));
    }
  }

  SECTION("setAxisProportional")
  {
    auto axes = ProportionalAxes::None();
    axes.setAxisProportional(1, true);

    CHECK(!axes.isAxisProportional(0));
    CHECK(axes.isAxisProportional(1));
    CHECK(!axes.isAxisProportional(2));
    CHECK(!axes.allAxesProportional());

    axes.setAxisProportional(1, false);
    CHECK(!axes.isAxisProportional(1));
  }
}

TEST_CASE("bbox handle geometry")
{
  const auto box = vm::bbox3d{{-16, -16, -16}, {16, 16, 16}};

  SECTION("allSides")
  {
    const auto sides = allSides();
    CHECK(sides.size() == 6);
    for (const auto n :
         {vm::vec3d{1, 0, 0},
          vm::vec3d{-1, 0, 0},
          vm::vec3d{0, 1, 0},
          vm::vec3d{0, -1, 0},
          vm::vec3d{0, 0, 1},
          vm::vec3d{0, 0, -1}})
    {
      CHECK(std::ranges::find(sides, BBoxSide{n}) != sides.end());
    }
  }

  SECTION("allEdges")
  {
    CHECK(allEdges().size() == 12);
  }

  SECTION("allCorners")
  {
    const auto corners = allCorners();
    CHECK(corners.size() == 8);
    for (const auto x : {-1.0, 1.0})
    {
      for (const auto y : {-1.0, 1.0})
      {
        for (const auto z : {-1.0, 1.0})
        {
          CHECK(std::ranges::find(corners, BBoxCorner{{x, y, z}}) != corners.end());
        }
      }
    }
  }

  SECTION("pointForBBoxCorner")
  {
    CHECK(pointForBBoxCorner(box, BBoxCorner{{1, 1, 1}}) == vm::vec3d{16, 16, 16});
    CHECK(pointForBBoxCorner(box, BBoxCorner{{-1, 1, -1}}) == vm::vec3d{-16, 16, -16});
  }

  SECTION("pointsForBBoxEdge")
  {
    const auto edge = BBoxEdge{{1, 1, 1}, {1, 1, -1}};
    CHECK(
      pointsForBBoxEdge(box, edge)
      == vm::segment3d{vm::vec3d{16, 16, 16}, vm::vec3d{16, 16, -16}});
  }

  SECTION("polygonForBBoxSide")
  {
    const auto polygon = polygonForBBoxSide(box, BBoxSide{{0, 0, 1}});
    CHECK(polygon.vertexCount() == 4);
    for (const auto& vertex : polygon.vertices())
    {
      CHECK(vertex.z() == 16.0);
    }
  }

  SECTION("centerForBBoxSide")
  {
    CHECK(centerForBBoxSide(box, BBoxSide{{0, 0, 1}}) == vm::vec3d{0, 0, 16});
    CHECK(centerForBBoxSide(box, BBoxSide{{1, 0, 0}}) == vm::vec3d{16, 0, 0});
  }

  SECTION("handleLineForHit")
  {
    SECTION("side hit")
    {
      const auto hit = mdl::Hit{ScaleTool::SideHitType, 0.0, {}, BBoxSide{{0, 0, 1}}};
      CHECK(handleLineForHit(box, hit) == vm::line3d{vm::vec3d{0, 0, 16}, {0, 0, 1}});
    }

    SECTION("edge hit")
    {
      const auto hit =
        mdl::Hit{ScaleTool::EdgeHitType, 0.0, {}, BBoxEdge{{1, 1, -1}, {1, 1, 1}}};
      const auto line = handleLineForHit(box, hit);
      // the line passes through both edge midpoints
      CHECK(line.point == vm::vec3d{-16, -16, 0});
      CHECK(line.direction == vm::vec3d{1, 1, 0} / vm::length(vm::vec3d{1, 1, 0}));
    }

    SECTION("corner hit")
    {
      const auto hit = mdl::Hit{ScaleTool::CornerHitType, 0.0, {}, BBoxCorner{{1, 1, 1}}};
      const auto line = handleLineForHit(box, hit);
      CHECK(line.point == vm::vec3d{-16, -16, -16});
      const auto expectedDir = vm::normalize(vm::vec3d{1, 1, 1});
      CHECK(line.direction == vm::approx{expectedDir});
    }
  }

  SECTION("moveBBoxForHit dispatches by hit type")
  {
    SECTION("side hit")
    {
      const auto hit = mdl::Hit{ScaleTool::SideHitType, 0.0, {}, BBoxSide{{1, 0, 0}}};
      const auto result = moveBBoxForHit(
        box, hit, {25, 0, 0}, ProportionalAxes::None(), AnchorPos::Opposite);
      CHECK(
        result
        == moveBBoxSide(
          box,
          BBoxSide{{1, 0, 0}},
          {25, 0, 0},
          ProportionalAxes::None(),
          AnchorPos::Opposite));
    }

    SECTION("edge hit")
    {
      const auto edge = BBoxEdge{{1, 1, -1}, {1, 1, 1}};
      const auto hit = mdl::Hit{ScaleTool::EdgeHitType, 0.0, {}, edge};
      const auto result = moveBBoxForHit(
        box, hit, {25, 25, 0}, ProportionalAxes::None(), AnchorPos::Opposite);
      CHECK(
        result
        == moveBBoxEdge(
          box, edge, {25, 25, 0}, ProportionalAxes::None(), AnchorPos::Opposite));
    }

    SECTION("corner hit")
    {
      const auto corner = BBoxCorner{{1, 1, 1}};
      const auto hit = mdl::Hit{ScaleTool::CornerHitType, 0.0, {}, corner};
      const auto result = moveBBoxForHit(
        box, hit, {25, 25, 25}, ProportionalAxes::None(), AnchorPos::Opposite);
      CHECK(result == moveBBoxCorner(box, corner, {25, 25, 25}, AnchorPos::Opposite));
    }
  }
}

TEST_CASE("pickBackSideOfBox")
{
  const auto box = vm::bbox3d{{-16, -16, -16}, {16, 16, 16}};
  const auto camera = gl::PerspectiveCamera{};

  // The ray runs straight down along the box's +X face (x=16, y=0), so it passes
  // exactly through the midpoint of that face's bottom edge (16, 0, -16), which is the
  // unique closest point among all "back-facing" (relative to the ray direction) faces.
  const auto pickRay = vm::ray3d{{16, 0, 500}, {0, 0, -1}};

  const auto result = pickBackSideOfBox(pickRay, camera, box);

  CHECK(result.pickedSideNormal == vm::vec3d{1, 0, 0});
  CHECK(result.distAlongRay == vm::approx{516.0});
}

TEST_CASE("moveBBox")
{
  SECTION("moveBBoxSide")
  {
    SECTION("non proportional")
    {
      const auto input1 = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};

      const auto exp1 = vm::bbox3d{{-100, -100, -100}, {125, 100, 100}};

      CHECK(
        moveBBoxSide(
          input1,
          BBoxSide{{1, 0, 0}},
          vm::vec3d{25, 0, 0},
          ProportionalAxes::None(),
          AnchorPos::Opposite)
        == exp1);

      // attempting to collapse the bbox returns an empty box
      CHECK(moveBBoxSide(
              input1,
              BBoxSide{{1, 0, 0}},
              vm::vec3d{-200, 0, 0},
              ProportionalAxes::None(),
              AnchorPos::Opposite)
              .is_empty());
      CHECK(moveBBoxSide(
              input1,
              BBoxSide{{1, 0, 0}},
              vm::vec3d{-225, 0, 0},
              ProportionalAxes::None(),
              AnchorPos::Opposite)
              .is_empty());

      // test with center anchor
      const auto exp2 = vm::bbox3d{{-125, -100, -100}, {125, 100, 100}};

      CHECK(
        moveBBoxSide(
          input1,
          BBoxSide{{1, 0, 0}},
          vm::vec3d{25, 0, 0},
          ProportionalAxes::None(),
          AnchorPos::Center)
        == exp2);
      CHECK(moveBBoxSide(
              input1,
              BBoxSide{{1, 0, 0}},
              vm::vec3d{-100, 0, 0},
              ProportionalAxes::None(),
              AnchorPos::Center)
              .is_empty());
      CHECK(moveBBoxSide(
              input1,
              BBoxSide{{1, 0, 0}},
              vm::vec3d{-125, 0, 0},
              ProportionalAxes::None(),
              AnchorPos::Center)
              .is_empty());
    }

    SECTION("proportional")
    {
      const auto input1 = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};

      const auto exp1 = vm::bbox3d{{-100, -112.5, -112.5}, {125, 112.5, 112.5}};

      CHECK(exp1.size() == vm::vec3d{225, 225, 225});
      CHECK(
        moveBBoxSide(
          input1,
          BBoxSide{{1, 0, 0}},
          vm::vec3d{25, 0, 0},
          ProportionalAxes::All(),
          AnchorPos::Opposite)
        == exp1);

      // attempting to collapse the bbox returns an empty box
      CHECK(moveBBoxSide(
              input1,
              BBoxSide{{1, 0, 0}},
              vm::vec3d{-200, 0, 0},
              ProportionalAxes::All(),
              AnchorPos::Opposite)
              .is_empty());
      CHECK(moveBBoxSide(
              input1,
              BBoxSide{{1, 0, 0}},
              vm::vec3d{-225, 0, 0},
              ProportionalAxes::All(),
              AnchorPos::Opposite)
              .is_empty());

      // test with center anchor
      const auto exp2 = vm::bbox3d{{-125, -125, -125}, {125, 125, 125}};

      CHECK(
        moveBBoxSide(
          input1,
          BBoxSide{{1, 0, 0}},
          vm::vec3d{25, 0, 0},
          ProportionalAxes::All(),
          AnchorPos::Center)
        == exp2);
      CHECK(moveBBoxSide(
              input1,
              BBoxSide{{1, 0, 0}},
              vm::vec3d{-100, 0, 0},
              ProportionalAxes::All(),
              AnchorPos::Center)
              .is_empty());
      CHECK(moveBBoxSide(
              input1,
              BBoxSide{{1, 0, 0}},
              vm::vec3d{-125, 0, 0},
              ProportionalAxes::All(),
              AnchorPos::Center)
              .is_empty());
    }
  }

  SECTION("moveBBoxCorner")
  {
    const auto input1 = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};

    const auto exp1 = vm::bbox3d{{-100, -100, -100}, {125, 125, 125}};

    CHECK(
      moveBBoxCorner(
        input1, BBoxCorner{{1, 1, 1}}, vm::vec3d{25, 25, 25}, AnchorPos::Opposite)
      == exp1);

    // attempting to collapse the bbox returns an empty box
    CHECK(moveBBoxCorner(
            input1, BBoxCorner{{1, 1, 1}}, vm::vec3d{-200, 0, 0}, AnchorPos::Opposite)
            .is_empty());
    CHECK(moveBBoxCorner(
            input1, BBoxCorner{{1, 1, 1}}, vm::vec3d{-225, 0, 0}, AnchorPos::Opposite)
            .is_empty());

    // test with center anchor
    const auto exp2 = vm::bbox3d{{-125, -125, -125}, {125, 125, 125}};

    CHECK(
      moveBBoxCorner(
        input1, BBoxCorner{{1, 1, 1}}, vm::vec3d{25, 25, 25}, AnchorPos::Center)
      == exp2);
    CHECK(moveBBoxCorner(
            input1, BBoxCorner{{1, 1, 1}}, vm::vec3d{-100, 0, 0}, AnchorPos::Center)
            .is_empty());
    CHECK(moveBBoxCorner(
            input1, BBoxCorner{{1, 1, 1}}, vm::vec3d{-125, 0, 0}, AnchorPos::Center)
            .is_empty());
  }

  SECTION("moveBBoxEdge")
  {
    SECTION("non proportional")
    {
      const auto input1 = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};

      const auto exp1 = vm::bbox3d{{-100, -100, -100}, {125, 125, 100}};

      // move the (+X, +Y, +/-Z) edge by X=25, Y=25
      CHECK(
        moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, -1}, {1, 1, 1}},
          vm::vec3d{25, 25, 0},
          ProportionalAxes::None(),
          AnchorPos::Opposite)
        == exp1);

      // attempting to collapse the bbox returns an empty box
      CHECK(moveBBoxEdge(
              input1,
              BBoxEdge{{1, 1, -1}, {1, 1, 1}},
              vm::vec3d{-200, -200, 0},
              ProportionalAxes::None(),
              AnchorPos::Opposite)
              .is_empty());
      CHECK(moveBBoxEdge(
              input1,
              BBoxEdge{{1, 1, -1}, {1, 1, 1}},
              vm::vec3d{-225, -225, 0},
              ProportionalAxes::None(),
              AnchorPos::Opposite)
              .is_empty());

      // test with center anchor
      const auto exp2 = vm::bbox3d{{-125, -125, -100}, {125, 125, 100}};

      // move the (+X, +Y, +/-Z) edge by X=25, Y=25
      CHECK(
        moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, -1}, {1, 1, 1}},
          vm::vec3d{25, 25, 0},
          ProportionalAxes::None(),
          AnchorPos::Center)
        == exp2);
      CHECK(moveBBoxEdge(
              input1,
              BBoxEdge{{1, 1, -1}, {1, 1, 1}},
              vm::vec3d{-100, -200, 0},
              ProportionalAxes::None(),
              AnchorPos::Center)
              .is_empty());
      CHECK(moveBBoxEdge(
              input1,
              BBoxEdge{{1, 1, -1}, {1, 1, 1}},
              vm::vec3d{-125, -225, 0},
              ProportionalAxes::None(),
              AnchorPos::Center)
              .is_empty());
    }

    SECTION("non proportional (negative Y axis)")
    {
      const auto input1 = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};

      const auto exp1 = vm::bbox3d{{-100, -125, -100}, {100, 100, 125}};

      // move the (+Z, -Y, +/-X) edge by Z=25, Y=-25
      CHECK(
        moveBBoxEdge(
          input1,
          BBoxEdge{{1, -1, 1}, {-1, -1, 1}},
          vm::vec3d{0, -25, 25},
          ProportionalAxes::None(),
          AnchorPos::Opposite)
        == exp1);

      // test with center anchor
      const auto exp2 = vm::bbox3d{{-100, -125, -125}, {100, 125, 125}};

      CHECK(
        moveBBoxEdge(
          input1,
          BBoxEdge{{1, -1, 1}, {-1, -1, 1}},
          vm::vec3d{0, -25, 25},
          ProportionalAxes::None(),
          AnchorPos::Center)
        == exp2);
    }

    SECTION("proportional")
    {
      const auto input1 = vm::bbox3d{{-100, -100, -100}, {100, 100, 100}};

      const auto exp1 = vm::bbox3d{{-100, -100, -112.5}, {125, 125, 112.5}};

      // move the (+X, +Y, +/-Z) edge by X=25, Y=25
      CHECK(
        moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, -1}, {1, 1, 1}},
          vm::vec3d{25, 25, 0},
          ProportionalAxes::All(),
          AnchorPos::Opposite)
        == exp1);

      // attempting to collapse the bbox returns an empty box
      CHECK(moveBBoxEdge(
              input1,
              BBoxEdge{{1, 1, -1}, {1, 1, 1}},
              vm::vec3d{-200, -200, 0},
              ProportionalAxes::All(),
              AnchorPos::Opposite)
              .is_empty());
      CHECK(moveBBoxEdge(
              input1,
              BBoxEdge{{1, 1, -1}, {1, 1, 1}},
              vm::vec3d{-225, -225, 0},
              ProportionalAxes::All(),
              AnchorPos::Opposite)
              .is_empty());

      // test with center anchor
      const auto exp2 = vm::bbox3d{{-125, -125, -125}, {125, 125, 125}};

      CHECK(
        moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, -1}, {1, 1, 1}},
          vm::vec3d{25, 25, 0},
          ProportionalAxes::All(),
          AnchorPos::Center)
        == exp2);
      CHECK(moveBBoxEdge(
              input1,
              BBoxEdge{{1, 1, -1}, {1, 1, 1}},
              vm::vec3d{-100, -100, 0},
              ProportionalAxes::All(),
              AnchorPos::Center)
              .is_empty());
      CHECK(moveBBoxEdge(
              input1,
              BBoxEdge{{1, 1, -1}, {1, 1, 1}},
              vm::vec3d{-125, -125, 0},
              ProportionalAxes::All(),
              AnchorPos::Center)
              .is_empty());
    }

    SECTION("mixed proportional and non proportional")
    {
      const auto input1 = vm::bbox3d{{-64, -64, -16}, {64, 64, 16}};

      const auto exp1 = vm::bbox3d{{-64, -64, -16}, {128, 64, 48}};

      // NOTE: the Y=64 part of the delta is ignored because the edge we are moving points
      // along the Y axis
      const auto delta = vm::vec3d{64, 64, 32};

      CHECK(
        moveBBoxEdge(
          input1,
          BBoxEdge{{1, 1, 1}, {1, -1, 1}},
          delta,
          ProportionalAxes(true, false, true),
          AnchorPos::Opposite)
        == exp1);
    }
  }
}

} // namespace tb::ui
