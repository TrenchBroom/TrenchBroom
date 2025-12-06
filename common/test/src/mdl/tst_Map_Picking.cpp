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

#include "TestUtils.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/HitAdapter.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Picking.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"

#include "vm/approx.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("Map_Picking")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

  SECTION("pick")
  {
    SECTION("Single brush")
    {
      auto* brushNode1 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material")
        | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode1}}});

      auto pickResult = PickResult{};
      pick(map, vm::ray3d{{-32, 0, 0}, {1, 0, 0}}, pickResult);

      auto hits = pickResult.all();
      CHECK(hits.size() == 1u);

      const auto& brush1 = brushNode1->brush();
      CHECK(
        hitToFaceHandle(hits.front())->face()
        == brush1.face(*brush1.findFace(vm::vec3d{-1, 0, 0})));
      CHECK(hits.front().distance() == vm::approx{32.0});

      pickResult.clear();
      pick(map, vm::ray3d{{-32, 0, 0}, {-1, 0, 0}}, pickResult);
      CHECK(pickResult.all().empty());
    }

    SECTION("Single entity")
    {
      auto* entityNode1 = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode1}}});

      const auto origin = entityNode1->entity().origin();
      const auto bounds = entityNode1->logicalBounds();

      const auto rayOrigin =
        origin + vm::vec3d{-32.0, bounds.size().y() / 2.0, bounds.size().z() / 2.0};

      auto pickResult = PickResult{};
      pick(map, vm::ray3d{rayOrigin, {1, 0, 0}}, pickResult);

      auto hits = pickResult.all();
      CHECK(hits.size() == 1u);

      CHECK(hits.front().target<EntityNode*>() == entityNode1);
      CHECK(hits.front().distance() == vm::approx{32.0 - bounds.size().x() / 2.0});

      pickResult.clear();
      pick(map, vm::ray3d{{-32, 0, 0}, {-1, 0, 0}}, pickResult);
      CHECK(pickResult.all().empty());
    }

    SECTION("Simple group")
    {
      using namespace HitFilters;

      auto* brushNode1 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material")
        | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode1}}});

      auto* brushNode2 = new BrushNode{
        builder.createCuboid(
          vm::bbox3d{{0, 0, 0}, {64, 64, 64}}.translate({0, 0, 128}), "material")
        | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode2}}});

      selectAllNodes(map);
      auto* group = groupSelectedNodes(map, "test");

      auto pickResult = PickResult{};
      pick(map, vm::ray3d{{-32, 0, 0}, {1, 0, 0}}, pickResult);

      // picking a grouped object when the containing group is closed should return the
      // object, which is converted to the group when hitsToNodesWithGroupPicking() is
      // used.
      auto hits = pickResult.all(type(BrushNode::BrushHitType));
      CHECK(hits.size() == 1u);

      const auto& brush1 = brushNode1->brush();
      CHECK(
        hitToFaceHandle(hits.front())->face()
        == brush1.face(*brush1.findFace(vm::vec3d{-1, 0, 0})));
      CHECK(hits.front().distance() == vm::approx{32.0});

      CHECK_THAT(hitsToNodesWithGroupPicking(hits), Equals(std::vector<Node*>{group}));

      // hitting both objects in the group should return the group only once
      pickResult.clear();
      pick(map, vm::ray3d{{32, 32, -32}, {0, 0, 1}}, pickResult);

      hits = pickResult.all(type(BrushNode::BrushHitType));
      CHECK(hits.size() == 2u);

      CHECK_THAT(hitsToNodesWithGroupPicking(hits), Equals(std::vector<Node*>{group}));

      // hitting the group bounds doesn't count as a hit
      pickResult.clear();
      pick(map, vm::ray3d{{-32, 0, 96}, {1, 0, 0}}, pickResult);

      hits = pickResult.all(type(BrushNode::BrushHitType));
      CHECK(hits.empty());

      // hitting a grouped object when the containing group is open should return the
      // object only
      openGroup(map, *group);

      pickResult.clear();
      pick(map, vm::ray3d{{-32, 0, 0}, {1, 0, 0}}, pickResult);

      hits = pickResult.all(type(BrushNode::BrushHitType));
      CHECK(hits.size() == 1u);

      CHECK(
        hitToFaceHandle(hits.front())->face()
        == brush1.face(*brush1.findFace(vm::vec3d{-1, 0, 0})));
      CHECK(hits.front().distance() == vm::approx{32.0});

      CHECK_THAT(
        hitsToNodesWithGroupPicking(hits), Equals(std::vector<Node*>{brushNode1}));
    }

    SECTION("Nested group")
    {
      using namespace HitFilters;

      auto* brushNode1 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material")
        | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode1}}});

      auto* brushNode2 = new BrushNode{
        builder.createCuboid(
          vm::bbox3d{{0, 0, 0}, {64, 64, 64}}.translate({0, 0, 128}), "material")
        | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode2}}});

      selectAllNodes(map);
      auto* innerGroup = groupSelectedNodes(map, "inner");

      deselectAll(map);
      auto* brushNode3 = new BrushNode{
        builder.createCuboid(
          vm::bbox3d{{0, 0, 0}, {64, 64, 64}}.translate({0, 0, 256}), "material")
        | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode3}}});

      selectAllNodes(map);
      auto* outerGroup = groupSelectedNodes(map, "outer");

      const vm::ray3d highRay({-32, 0, +32 + 256}, {1, 0, 0});
      const vm::ray3d lowRay({-32, 0, +32}, {1, 0, 0});

      /*
       *          Z
       *         /|\
       *          |
       *          | ______________
       *          | |   ______   |
       *  hiRay *-->|   | b3 |   |
       *          | |   |____|   |
       *          | |            |
       *          | |   outer    |
       *          | | __________ |
       *          | | | ______ | |
       *          | | | | b2 | | |
       *          | | | |____| | |
       *          | | |        | |
       *          | | |  inner | |
       *          | | | ______ | |
       * lowRay *-->| | | b1 | | |
       *        0_| | | |____| | |
       *          | | |________| |
       *          | |____________|
       * ---------|--------------------> X
       *                |
       *                0
       */

      /*
       * world
       * * outer (closed)
       *   * inner (closed)
       *     * brush1
       *     * brush2
       *   * brush3
       */

      auto pickResult = PickResult{};

      // hitting a grouped object when the containing group is open should return the
      // object only
      openGroup(map, *outerGroup);

      /*
       * world
       * * outer (open)
       *   * inner (closed)
       *     * brush1
       *     * brush2
       *   * brush3
       */

      pickResult.clear();
      pick(map, highRay, pickResult);

      auto hits = pickResult.all(type(BrushNode::BrushHitType));
      CHECK(hits.size() == 1u);

      const auto& brush3 = brushNode3->brush();
      CHECK(
        hitToFaceHandle(hits.front())->face()
        == brush3.face(*brush3.findFace(vm::vec3d{-1, 0, 0})));
      CHECK(hits.front().distance() == vm::approx{32.0});

      CHECK_THAT(
        hitsToNodesWithGroupPicking(hits), Equals(std::vector<Node*>{brushNode3}));

      // hitting the brush in the inner group should return the inner group when
      // hitsToNodesWithGroupPicking() is used
      pickResult.clear();
      pick(map, lowRay, pickResult);

      hits = pickResult.all(type(BrushNode::BrushHitType));
      CHECK(hits.size() == 1u);

      const auto& brush1 = brushNode1->brush();
      CHECK(
        hitToFaceHandle(hits.front())->face()
        == brush1.face(*brush1.findFace(vm::vec3d{-1, 0, 0})));
      CHECK(hits.front().distance() == vm::approx{32.0});
      CHECK_THAT(
        hitsToNodesWithGroupPicking(hits), Equals(std::vector<Node*>{innerGroup}));

      // open the inner group, too. hitsToNodesWithGroupPicking() should no longer return
      // groups, since all groups are open.
      openGroup(map, *innerGroup);

      /*
       * world
       * * outer (open)
       *   * inner (open)
       *     * brush1
       *     * brush2
       *   * brush3
       */

      CHECK(innerGroup->opened());
      CHECK_FALSE(outerGroup->opened());
      CHECK(outerGroup->hasOpenedDescendant());

      // pick a brush in the outer group
      pickResult.clear();
      pick(map, highRay, pickResult);

      hits = pickResult.all(type(BrushNode::BrushHitType));
      CHECK(hits.size() == 1u);

      CHECK(
        hitToFaceHandle(hits.front())->face()
        == brush3.face(*brush3.findFace(vm::vec3d{-1, 0, 0})));
      CHECK(hits.front().distance() == vm::approx{32.0});
      CHECK_THAT(
        hitsToNodesWithGroupPicking(hits), Equals(std::vector<Node*>{brushNode3}));

      // pick a brush in the inner group
      pickResult.clear();
      pick(map, lowRay, pickResult);

      hits = pickResult.all(type(BrushNode::BrushHitType));
      CHECK(hits.size() == 1u);

      CHECK(
        hitToFaceHandle(hits.front())->face()
        == brush1.face(*brush1.findFace(vm::vec3d{-1, 0, 0})));
      CHECK(hits.front().distance() == vm::approx{32.0});
      CHECK_THAT(
        hitsToNodesWithGroupPicking(hits), Equals(std::vector<Node*>{brushNode1}));
    }

    SECTION("Brush entity")
    {
      map.entityDefinitionManager().setDefinitions({
        {"brush_entity", Color{}, "this is a brush entity", {}},
      });

      const auto& brushEntityDefinition =
        map.entityDefinitionManager().definitions().front();

      auto* brushNode1 = new mdl::BrushNode{
        builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material")
        | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode1}}});

      auto* brushNode2 = new mdl::BrushNode{
        builder.createCuboid(
          vm::bbox3d{{0, 0, 0}, {64, 64, 64}}.translate({0, 0, 128}), "material")
        | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode2}}});

      selectAllNodes(map);

      createBrushEntity(map, brushEntityDefinition);
      deselectAll(map);

      auto pickResult = mdl::PickResult{};

      // picking entity brushes should only return the brushes and not the entity
      pick(map, vm::ray3d{{-32, 0, 0}, {1, 0, 0}}, pickResult);

      auto hits = pickResult.all();
      CHECK(hits.size() == 1u);

      const auto& brush1 = brushNode1->brush();
      CHECK(
        mdl::hitToFaceHandle(hits.front())->face()
        == brush1.face(*brush1.findFace(vm::vec3d{-1, 0, 0})));
      CHECK(hits.front().distance() == vm::approx{32.0});
    }
  }

  SECTION("findNodesContaining")
  {
    auto* brushNode = new BrushNode{
      builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material")
      | kdl::value()};

    auto* entityNode = new EntityNode{Entity{}};
    REQUIRE(entityNode->logicalBounds() == vm::bbox3d{{-8, -8, -8}, {8, 8, 8}});

    auto* groupedBrushNode = new BrushNode{
      builder.createCuboid(
        vm::bbox3d{{0, 0, 0}, {64, 64, 64}}.translate({0, 0, 32}), "material")
      | kdl::value()};
    addNodes(map, {{parentForNodes(map), {brushNode, entityNode, groupedBrushNode}}});

    selectNodes(map, {groupedBrushNode});
    groupSelectedNodes(map, "test");

    CHECK(findNodesContaining(map, {0, 0, 1024}) == std::vector<Node*>{});
    CHECK_THAT(
      findNodesContaining(map, {0, 0, 0}),
      Catch::Matchers::UnorderedEquals(std::vector<Node*>{brushNode, entityNode}));
    CHECK_THAT(
      findNodesContaining(map, {32, 32, 24}),
      Catch::Matchers::UnorderedEquals(std::vector<Node*>{brushNode}));
    CHECK_THAT(
      findNodesContaining(map, {32, 32, 72}),
      Catch::Matchers::UnorderedEquals(std::vector<Node*>{groupedBrushNode}));
  }
}

} // namespace tb::mdl
