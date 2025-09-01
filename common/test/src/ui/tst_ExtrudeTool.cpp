/*
 Copyright (C) 2010 Kristian Duske
 Copyright (C) 2020 Eric Wasylishen

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

#include "MapFixture.h"
#include "TestUtils.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityNode.h"
#include "mdl/Game.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Picking.h"
#include "mdl/ModelUtils.h"
#include "mdl/NodeQueries.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"
#include "ui/ExtrudeTool.h"

#include "kdl/result.h"
#include "kdl/vector_utils.h"

#include "vm/approx.h"
#include "vm/ray.h"
#include "vm/vec.h"

#include <filesystem>

#include "catch/Matchers.h"

#include "Catch2.h"

namespace tb::ui
{
namespace
{

vm::vec3d n(const vm::vec3d& v)
{
  return vm::normalize(v);
}

mdl::PickResult performPick(mdl::Map& map, ExtrudeTool& tool, const vm::ray3d& pickRay)
{
  auto pickResult = mdl::PickResult::byDistance();
  pick(map, pickRay, pickResult);

  const auto hit = tool.pick3D(pickRay, pickResult);
  CHECK(hit.type() == ExtrudeTool::ExtrudeHitType);
  CHECK_FALSE(vm::is_nan(hit.hitPoint()));

  REQUIRE(hit.isMatch());
  pickResult.addHit(hit);

  REQUIRE(tool.proposedDragHandles().empty());
  tool.updateProposedDragHandles(pickResult);
  REQUIRE_FALSE(tool.proposedDragHandles().empty());

  return pickResult;
}

} // namespace

TEST_CASE("ExtrudeTool")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  auto tool = ExtrudeTool{map};

  SECTION("pick2D")
  {
    constexpr auto brushBounds = vm::bbox3d{16.0};

    auto builder = mdl::BrushBuilder{map.world()->mapFormat(), map.worldBounds()};
    auto* brushNode1 =
      new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};

    addNodes(map, {{map.editorContext().currentLayer(), {brushNode1}}});
    map.selectNodes({brushNode1});

    SECTION("Pick ray hits brush directly")
    {
      constexpr auto pickRay = vm::ray3d{{0, 0, 32}, {0, 0, -1}};

      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);

      REQUIRE(pickResult.all().size() == 1);

      const auto hit = tool.pick2D(pickRay, pickResult);
      CHECK_FALSE(hit.isMatch());
    }

    SECTION("Pick ray does not hit brush directly")
    {
      using T =
        std::tuple<vm::vec3d, vm::vec3d, vm::vec3d, vm::vec3d, vm::plane3d, vm::vec3d>;

      // clang-format off
      const auto
      [origin,     direction,     expectedFaceNormal, expectedHitPoint, expectedDragReference,          expectedHandlePosition] = GENERATE(values<T>({
      // shoot from above downwards just past the top west edge, picking the west face
      {{-17, 0, 32}, { 0, 0, -1}, {-1, 0, 0},         {-17, 0, 16},     {{-16, 0, 16}, {0, 0, -1}},     {-16, 0, 16}},
      // shoot diagonally past the top west edge, picking the west face
      {{ -1, 0, 33}, {-1, 0, -1}, {-1, 0, 0},         {-17, 0, 17},     {{-16, 0, 16}, n({-1, 0, -1})}, {-16, 0, 16}},
      }));
      // clang-format on

      CAPTURE(brushBounds, origin, direction);

      const auto hit = tool.pick2D(vm::ray3d{origin, vm::normalize(direction)}, {});

      CHECK(hit.isMatch());
      CHECK(hit.type() == ExtrudeTool::ExtrudeHitType);
      CHECK(hit.hitPoint() == expectedHitPoint);
      CHECK(hit.distance() == vm::approx{vm::length(expectedHitPoint - origin)});

      CHECK(
        hit.target<ExtrudeHitData>()
        == ExtrudeHitData{
          {brushNode1, *brushNode1->brush().findFace(expectedFaceNormal)},
          expectedDragReference,
          expectedHandlePosition});
    }
  }

  SECTION("pick3D")
  {
    constexpr auto brushBounds = vm::bbox3d{16.0};

    auto builder = mdl::BrushBuilder{map.world()->mapFormat(), map.worldBounds()};
    auto* brushNode1 =
      new mdl::BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};

    addNodes(map, {{map.editorContext().currentLayer(), {brushNode1}}});
    map.selectNodes({brushNode1});

    SECTION("Pick ray hits brush directly")
    {
      const auto pickRay = vm::ray3d{{0, 0, 24}, vm::normalize(vm::vec3d{-1, 0, -1})};

      auto pickResult = mdl::PickResult{};
      pick(map, pickRay, pickResult);

      REQUIRE(pickResult.all().size() == 1);

      const auto hit = tool.pick3D(pickRay, pickResult);

      CHECK(hit.isMatch());
      CHECK(hit.type() == ExtrudeTool::ExtrudeHitType);
      CHECK(hit.hitPoint() == vm::vec3d{-8, 0, 16});
      CHECK(hit.distance() == vm::approx{vm::length(hit.hitPoint() - pickRay.origin)});

      CHECK(
        hit.target<ExtrudeHitData>()
        == ExtrudeHitData{
          {brushNode1, *brushNode1->brush().findFace(vm::vec3d{0, 0, 1})},
          vm::line3d{hit.hitPoint(), {0, 0, 1}},
          hit.hitPoint()});
    }

    SECTION("Pick ray does not hit brush directly")
    {
      using T =
        std::tuple<vm::vec3d, vm::vec3d, vm::vec3d, vm::vec3d, vm::plane3d, vm::vec3d>;

      // clang-format off
      const auto
      [origin,     direction,     expectedFaceNormal, expectedHitPoint, expectedDragReference,     expectedHandlePosition] = GENERATE(values<T>({
      // shoot from above downwards just past the top west edge, picking the west face
      {{-17, 0, 32}, { 0, 0, -1}, {-1, 0, 0},         {-17, 0, 16},     {{-16, 0, 16}, {0, 0, 1}}, {-16, 0, 16}},
      // shoot diagonally past the top west edge, picking the west face
      {{ -1, 0, 33}, {-1, 0, -1}, {-1, 0, 0},         {-17, 0, 17},     {{-16, 0, 16}, {0, 0, 1}}, {-16, 0, 16}},
      }));
      // clang-format on

      CAPTURE(brushBounds, origin, direction);

      const auto hit = tool.pick3D(vm::ray3d{origin, vm::normalize(direction)}, {});

      CHECK(hit.isMatch());
      CHECK(hit.type() == ExtrudeTool::ExtrudeHitType);
      CHECK(hit.hitPoint() == expectedHitPoint);
      CHECK(hit.distance() == vm::approx{vm::length(expectedHitPoint - origin)});

      CHECK(
        hit.target<ExtrudeHitData>()
        == ExtrudeHitData{
          {brushNode1, *brushNode1->brush().findFace(expectedFaceNormal)},
          expectedDragReference,
          expectedHandlePosition});
    }
  }

  SECTION("findDragFaces")
  {
    // https://github.com/TrenchBroom/TrenchBroom/issues/3726

    using T = std::tuple<std::filesystem::path, std::vector<std::string>>;

    // clang-format off
    const auto 
    [mapName,                              expectedDragFaceMaterialNames] = GENERATE(values<T>({
    {"findDragFaces_noCoplanarFaces.map",  {"larger_top_face"}},
    {"findDragFaces_twoCoplanarFaces.map", {"larger_top_face", "smaller_top_face"}}
    }));
    // clang-format on

    const auto mapPath = "fixture/test/ui/ExtrudeToolTest" / mapName;
    fixture.load(
      mapPath,
      {.mapFormat = mdl::MapFormat::Valve, .game = mdl::LoadGameFixture{"Quake"}});

    map.selectAllNodes();

    auto brushes = map.selection().brushes;
    REQUIRE(brushes.size() == 2);

    const auto brushIt = std::find_if(
      std::begin(brushes), std::end(brushes), [](const mdl::BrushNode* brushNode) {
        return brushNode->brush().findFace("larger_top_face").has_value();
      });
    REQUIRE(brushIt != std::end(brushes));

    const auto* brushNode = *brushIt;
    const auto& largerTopFace =
      brushNode->brush().face(brushNode->brush().findFace("larger_top_face").value());

    // Find the entity defining the camera position for our test
    auto* cameraEntity = kdl::vec_filter(map.selection().entities, [](const auto* e) {
                           return e->entity().classname() == "trigger_relay";
                         }).front();

    // Fire a pick ray at largerTopFace
    const auto pickRay = vm::ray3d{
      cameraEntity->entity().origin(),
      vm::normalize(largerTopFace.center() - cameraEntity->entity().origin())};

    const auto pickResult = performPick(map, tool, pickRay);
    REQUIRE(
      pickResult.all().front().target<mdl::BrushFaceHandle>().face() == largerTopFace);

    CHECK_THAT(
      kdl::vec_transform(
        tool.proposedDragHandles(),
        [](const auto& h) { return h.faceAtDragStart().attributes().materialName(); }),
      Catch::UnorderedEquals(expectedDragFaceMaterialNames));
  }

  SECTION("splitBrushes")
  {
    using namespace mdl::HitFilters;


    const auto mapPath = "fixture/test/ui/ExtrudeToolTest/splitBrushes.map";
    fixture.load(
      mapPath,
      {.mapFormat = mdl::MapFormat::Valve, .game = mdl::LoadGameFixture{"Quake"}});

    map.selectAllNodes();

    auto brushes = map.selection().brushes;
    REQUIRE(brushes.size() == 2);

    // Find the entity defining the camera position for our test
    const auto* cameraEntity =
      kdl::vec_filter(map.selection().entities, [](const auto* node) {
        return node->entity().classname() == "trigger_relay";
      }).front();

    const auto* cameraTarget =
      kdl::vec_filter(map.selection().entities, [](const auto* node) {
        return node->entity().classname() == "info_null";
      }).front();

    const auto* funcDetailNode =
      kdl::vec_filter(
        mdl::filterEntityNodes(mdl::collectDescendants({map.world()})),
        [](const auto* node) { return node->entity().classname() == "func_detail"; })
        .front();

    // Fire a pick ray at cameraTarget
    const auto pickRay = vm::ray3d(
      cameraEntity->entity().origin(),
      vm::normalize(cameraTarget->entity().origin() - cameraEntity->entity().origin()));

    const auto pickResult = performPick(map, tool, pickRay);

    // We are going to drag the 2 faces with +Y normals
    CHECK(
      kdl::vec_transform(
        tool.proposedDragHandles(),
        [](const auto& h) { return h.faceAtDragStart().normal(); })
      == std::vector<vm::vec3d>{vm::vec3d{0, 1, 0}, vm::vec3d{0, 1, 0}});

    const auto hit = pickResult.first(type(ExtrudeTool::ExtrudeHitType));
    auto dragState = ExtrudeDragState{
      tool.proposedDragHandles(),
      ExtrudeTool::getDragFaces(tool.proposedDragHandles()),
      false,
      vm::vec3d{0, 0, 0}};

    SECTION("split brushes inwards 32 units towards -Y")
    {
      const auto delta = vm::vec3d(0, -32, 0);

      dragState.splitBrushes = true;
      tool.beginExtrude();

      REQUIRE(tool.extrude(delta, dragState));
      tool.commit(dragState);

      CHECK(map.selection().brushes.size() == 4);

      SECTION("check 2 resulting worldspawn brushes")
      {
        const auto nodes =
          mdl::filterBrushNodes(map.editorContext().currentLayer()->children());
        const auto bounds = kdl::vec_transform(
          nodes, [](const auto* node) { return node->logicalBounds(); });
        const auto expectedBounds = std::vector<vm::bbox3d>{
          {{-32, 144, 16}, {-16, 192, 32}}, {{-32, 192, 16}, {-16, 224, 32}}};
        CHECK_THAT(bounds, Catch::UnorderedEquals(expectedBounds));
      }

      SECTION("check 2 resulting func_detail brushes")
      {
        const auto nodes = mdl::filterBrushNodes(funcDetailNode->children());
        const auto bounds = kdl::vec_transform(
          nodes, [](const auto* node) { return node->logicalBounds(); });
        const auto expectedBounds = std::vector<vm::bbox3d>{
          {{-16, 176, 16}, {16, 192, 32}}, {{-16, 192, 16}, {16, 224, 32}}};
        CHECK_THAT(bounds, Catch::UnorderedEquals(expectedBounds));
      }

      CHECK_THAT(
        kdl::vec_transform(
          map.selection().brushes,
          [](const auto* brushNode) { return brushNode->linkId(); }),
        AllDifferent<std::vector<std::string>>());
    }

    SECTION("split brushes inwards 48 units towards -Y")
    {
      const auto delta = vm::vec3d(0, -48, 0);

      dragState.splitBrushes = true;
      tool.beginExtrude();

      REQUIRE(tool.extrude(delta, dragState));
      tool.commit(dragState);

      CHECK(map.selection().brushes.size() == 3);

      SECTION("check 2 resulting worldspawn brushes")
      {
        const auto nodes =
          mdl::filterBrushNodes(map.editorContext().currentLayer()->children());
        const auto bounds = kdl::vec_transform(
          nodes, [](const auto* node) { return node->logicalBounds(); });
        const auto expectedBounds = std::vector<vm::bbox3d>{
          {{-32, 144, 16}, {-16, 176, 32}}, {{-32, 176, 16}, {-16, 224, 32}}};
        CHECK_THAT(bounds, Catch::UnorderedEquals(expectedBounds));
      }

      SECTION("check 1 resulting func_detail brush")
      {
        const auto nodes = mdl::filterBrushNodes(funcDetailNode->children());
        const auto bounds = kdl::vec_transform(
          nodes, [](const auto* node) { return node->logicalBounds(); });
        const auto expectedBounds =
          std::vector<vm::bbox3d>{{{-16, 176, 16}, {16, 224, 32}}};
        CHECK_THAT(bounds, Catch::UnorderedEquals(expectedBounds));
      }
    }

    SECTION("extrude inwards 32 units towards -Y")
    {
      const auto delta = vm::vec3d{0, -32, 0};

      dragState.splitBrushes = false;
      tool.beginExtrude();

      REQUIRE(tool.extrude(delta, dragState));
      tool.commit(dragState);

      CHECK(map.selection().brushes.size() == 2);

      SECTION("check 1 resulting worldspawn brushes")
      {
        const auto nodes =
          mdl::filterBrushNodes(map.editorContext().currentLayer()->children());
        const auto bounds = kdl::vec_transform(
          nodes, [](const auto* node) { return node->logicalBounds(); });
        const auto expectedBounds = std::vector<vm::bbox3d>{
          {{-32, 144, 16}, {-16, 192, 32}},
        };
        CHECK_THAT(bounds, Catch::UnorderedEquals(expectedBounds));
      }

      SECTION("check 1 resulting func_detail brush")
      {
        const auto nodes = mdl::filterBrushNodes(funcDetailNode->children());
        const auto bounds = kdl::vec_transform(
          nodes, [](const auto* node) { return node->logicalBounds(); });
        const auto expectedBounds =
          std::vector<vm::bbox3d>{{{-16, 176, 16}, {16, 192, 32}}};
        CHECK_THAT(bounds, Catch::UnorderedEquals(expectedBounds));
      }
    }

    SECTION("split brushes outwards 16 units towards +Y")
    {
      const auto delta = vm::vec3d{0, 16, 0};

      dragState.splitBrushes = true;
      tool.beginExtrude();

      REQUIRE(tool.extrude(delta, dragState));
      tool.commit(dragState);

      CHECK(map.selection().brushes.size() == 2);

      SECTION("check 1 resulting worldspawn brush")
      {
        auto nodes =
          mdl::filterBrushNodes(map.editorContext().currentLayer()->children());
        nodes = kdl::vec_filter(
          std::move(nodes), [](const auto* node) { return node->selected(); });

        const auto bounds = kdl::vec_transform(
          nodes, [](const auto* node) { return node->logicalBounds(); });
        const auto expectedBounds = std::vector<vm::bbox3d>{
          {{-32, 224, 16}, {-16, 240, 32}},
        };
        CHECK_THAT(bounds, Catch::UnorderedEquals(expectedBounds));
      }

      SECTION("check 1 resulting func_detail brush")
      {
        auto nodes = mdl::filterBrushNodes(funcDetailNode->children());
        nodes = kdl::vec_filter(
          std::move(nodes), [](const auto* node) { return node->selected(); });

        const auto bounds = kdl::vec_transform(
          nodes, [](const auto* node) { return node->logicalBounds(); });
        const auto expectedBounds =
          std::vector<vm::bbox3d>{{{-16, 224, 16}, {16, 240, 32}}};
        CHECK_THAT(bounds, Catch::UnorderedEquals(expectedBounds));
      }

      CHECK_THAT(
        kdl::vec_transform(
          map.selection().brushes,
          [](const auto* brushNode) { return brushNode->linkId(); }),
        AllDifferent<std::vector<std::string>>());
    }
  }
}

} // namespace tb::ui
