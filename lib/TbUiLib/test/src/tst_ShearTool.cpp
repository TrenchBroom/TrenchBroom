/*
 Copyright (C) 2025 Kristian Duske

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
#include "mdl/BrushNode.h" // IWYU pragma: keep
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Hit.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h" // IWYU pragma: keep
#include "mdl/PickResult.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/ScaleTool.h"
#include "ui/ShearTool.h"

#include "kd/ranges/to.h"
#include "kd/result.h"

#include "vm/approx.h"

#include <ranges>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

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

TEST_CASE("ShearTool")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
  auto* brushNode = mdl::createBrushNode(map);
  auto* patchNode = mdl::createPatchNode("some_material");
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {brushNode, entityNode, patchNode}}});

  auto nodes = std::vector<mdl::Node*>{entityNode, brushNode, patchNode};
  constexpr size_t iEntityNode = 0;
  constexpr size_t iBrushNode = 1;
  constexpr size_t iPatchNode = 2;

  auto tool = ShearTool{document};

  SECTION("applies")
  {
    using T = std::tuple<std::vector<size_t>, bool>;

    const auto [nodesIndicesToSelect, expectedApplies] = GENERATE(values<T>({
      {std::vector<size_t>{}, false},
      {std::vector<size_t>{iEntityNode}, true},
      {std::vector<size_t>{iBrushNode}, true},
      {std::vector<size_t>{iPatchNode}, true},
      {std::vector<size_t>{iEntityNode, iBrushNode, iPatchNode}, true},
    }));

    const auto nodesToSelect =
      nodesIndicesToSelect
      | std::views::transform([&](const auto i) -> mdl::Node* { return nodes[i]; })
      | kdl::ranges::to<std::vector>();

    mdl::selectNodes(map, nodesToSelect);
    CHECK(tool.applies() == expectedApplies);
  }

  SECTION("constrainVertical")
  {
    CHECK(!tool.constrainVertical());
    tool.setConstrainVertical(true);
    CHECK(tool.constrainVertical());
  }

  SECTION("with a selected brush of known bounds")
  {
    constexpr auto brushBounds = vm::bbox3d{16.0};
    auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* boundedBrush =
      new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};
    mdl::addNodes(map, {{map.editorContext().currentLayer(), {boundedBrush}}});
    mdl::selectNodes(map, {boundedBrush});

    SECTION("bounds")
    {
      CHECK(tool.bounds() == brushBounds);
      CHECK(&tool.grid() == &map.grid());
      CHECK(tool.bboxAtDragStart() == brushBounds);
    }

    SECTION("pick2D")
    {
      SECTION("origin inside bounds is ignored")
      {
        const auto pickRay = vm::ray3d{{0, 0, 0}, {0, 0, -1}};
        auto pickResult = mdl::PickResult{};
        tool.pick2D(pickRay, perspectiveCameraFor(pickRay), pickResult);
        CHECK(pickResult.empty());
      }

      SECTION("falls back to a back side outside the bounds")
      {
        const auto pickRay = vm::ray3d{{16, 200, 1000}, {0, 0, -1}};
        auto pickResult = mdl::PickResult{};
        tool.pick2D(pickRay, perspectiveCameraFor(pickRay), pickResult);

        REQUIRE(pickResult.all().size() == 1u);
        const auto& hit = pickResult.all().front();
        CHECK(hit.type() == ShearTool::ShearToolSideHitType);
        CHECK(!vm::is_nan(hit.hitPoint()));
      }
    }

    SECTION("pick3D")
    {
      SECTION("origin inside bounds is ignored")
      {
        const auto pickRay = vm::ray3d{{0, 0, 0}, {0, 0, -1}};
        auto pickResult = mdl::PickResult{};
        tool.pick3D(pickRay, perspectiveCameraFor(pickRay), pickResult);
        CHECK(pickResult.empty());
      }

      SECTION("hits a side directly")
      {
        const auto pickRay = vm::ray3d{{0, 0, 500}, {0, 0, -1}};
        auto pickResult = mdl::PickResult{};
        tool.pick3D(pickRay, perspectiveCameraFor(pickRay), pickResult);

        REQUIRE(pickResult.all().size() == 1u);
        const auto& hit = pickResult.all().front();
        CHECK(hit.type() == ShearTool::ShearToolSideHitType);
        CHECK(hit.target<BBoxSide>() == BBoxSide{{0, 0, 1}});
        CHECK(hit.hitPoint() == vm::approx{vm::vec3d{0, 0, 16}});
      }

      SECTION("falls back to a back side outside the bounds")
      {
        const auto pickRay = vm::ray3d{{5, 5, 17}, {0, 0, 1}};
        auto pickResult = mdl::PickResult{};
        tool.pick3D(pickRay, perspectiveCameraFor(pickRay), pickResult);

        REQUIRE(pickResult.all().size() == 1u);
        const auto& hit = pickResult.all().front();
        CHECK(hit.type() == ShearTool::ShearToolSideHitType);
        CHECK(!vm::is_nan(hit.hitPoint()));
      }
    }

    SECTION("updatePickedSide refreshes views only when the handle changes")
    {
      auto refreshCount = 0;
      auto connection = tool.refreshViewsNotifier.connect([&](Tool&) { ++refreshCount; });

      auto pickResult = mdl::PickResult{};
      pickResult.addHit(
        mdl::Hit{ShearTool::ShearToolSideHitType, 0.0, {0, 0, 16}, BBoxSide{{0, 0, 1}}});
      tool.updatePickedSide(pickResult);
      CHECK(refreshCount == 1);
      CHECK(tool.dragStartHit().target<BBoxSide>() == BBoxSide{{0, 0, 1}});

      // hitting the same side again should not trigger another refresh
      tool.updatePickedSide(pickResult);
      CHECK(refreshCount == 1);

      // hitting a different side should trigger a refresh
      auto otherPickResult = mdl::PickResult{};
      otherPickResult.addHit(
        mdl::Hit{ShearTool::ShearToolSideHitType, 0.0, {16, 0, 0}, BBoxSide{{1, 0, 0}}});
      tool.updatePickedSide(otherPickResult);
      CHECK(refreshCount == 2);
    }

    SECTION("shear lifecycle")
    {
      const auto hit =
        mdl::Hit{ShearTool::ShearToolSideHitType, 0.0, {16, 0, 0}, BBoxSide{{1, 0, 0}}};

      SECTION("before a drag starts, the shear handle and matrix are trivial")
      {
        CHECK(tool.bboxShearMatrix() == vm::mat4x4d::identity());
        CHECK(!tool.shearHandle());
      }

      SECTION("shearing and committing changes the brush's geometry")
      {
        tool.startShearWithHit(hit);
        CHECK(tool.dragStartHit().type() == ShearTool::ShearToolSideHitType);
        CHECK(tool.bboxAtDragStart() == brushBounds);

        tool.shearByDelta({0, 0, 10});
        CHECK(tool.bboxShearMatrix() != vm::mat4x4d::identity());
        REQUIRE(tool.shearHandle());

        tool.commitShear();
        // the +X side (the one sheared) moves by the full delta; the opposite (-X)
        // side is the shear's fixed pivot and stays put
        CHECK(boundedBrush->brush().hasVertex(vm::vec3d{-16, -16, -16}));
        CHECK(boundedBrush->brush().hasVertex(vm::vec3d{16, -16, -6}));
        CHECK(!boundedBrush->brush().hasVertex(vm::vec3d{16, -16, -16}));
      }

      SECTION("committing with zero delta cancels the transaction")
      {
        tool.startShearWithHit(hit);
        tool.commitShear();

        CHECK(tool.bounds() == brushBounds);
      }

      SECTION("cancelling discards the shear")
      {
        tool.startShearWithHit(hit);
        tool.shearByDelta({0, 0, 10});
        tool.cancelShear();

        CHECK(tool.bounds() == brushBounds);
      }
    }
  }
}

} // namespace tb::ui
