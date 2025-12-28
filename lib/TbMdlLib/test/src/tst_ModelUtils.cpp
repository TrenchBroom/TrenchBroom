/*
 Copyright (C) 2021 Kristian Duske

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

#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/LockState.h"
#include "mdl/MapFormat.h"
#include "mdl/ModelUtils.h"
#include "mdl/PatchNode.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"

#include "kd/result.h"

#include "vm/bbox.h"
#include "vm/mat_ext.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("ModelUtils.findContainingLayer")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  auto* layerNode = new LayerNode{Layer{"layer"}};
  auto* groupNode = new GroupNode{Group{"group"}};
  auto* entityNode = new EntityNode{Entity{}};
  auto* brushNode = new BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto* patchNode = new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  groupNode->addChildren({entityNode, brushNode});
  layerNode->addChildren({groupNode, patchNode});
  worldNode.addChild(layerNode);

  CHECK(findContainingLayer(&worldNode) == nullptr);
  CHECK(findContainingLayer(layerNode) == layerNode);
  CHECK(findContainingLayer(groupNode) == layerNode);
  CHECK(findContainingLayer(entityNode) == layerNode);
  CHECK(findContainingLayer(brushNode) == layerNode);
  CHECK(findContainingLayer(patchNode) == layerNode);
}

TEST_CASE("ModelUtils.findContainingGroup")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  auto* layerNode = new LayerNode{Layer{"layer"}};
  auto* outerGroupNode = new GroupNode{Group{"outer"}};
  auto* innerGroupNode = new GroupNode{Group{"inner"}};
  auto* entityNode = new EntityNode{Entity{}};
  auto* brushNode = new BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto* patchNode = new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  innerGroupNode->addChildren({entityNode, brushNode});
  outerGroupNode->addChildren({innerGroupNode, patchNode});
  worldNode.defaultLayer()->addChild(outerGroupNode);

  CHECK(findContainingGroup(&worldNode) == nullptr);
  CHECK(findContainingGroup(layerNode) == nullptr);
  CHECK(findContainingGroup(outerGroupNode) == nullptr);
  CHECK(findContainingGroup(innerGroupNode) == outerGroupNode);
  CHECK(findContainingGroup(entityNode) == innerGroupNode);
  CHECK(findContainingGroup(brushNode) == innerGroupNode);
  CHECK(findContainingGroup(patchNode) == outerGroupNode);
}

TEST_CASE("ModelUtils.findOutermostClosedGroup")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  auto* layerNode = new LayerNode{Layer{"layer"}};
  auto* outerGroupNode = new GroupNode{Group{"outer"}};
  auto* innerGroupNode = new GroupNode{Group{"inner"}};
  auto* entityNode = new EntityNode{Entity{}};
  auto* brushNode = new BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto* patchNode = new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  innerGroupNode->addChildren({entityNode, brushNode});
  outerGroupNode->addChildren({innerGroupNode, patchNode});
  worldNode.defaultLayer()->addChild(outerGroupNode);

  SECTION("All groups closed")
  {
    CHECK(findOutermostClosedGroup(&worldNode) == nullptr);
    CHECK(findOutermostClosedGroup(layerNode) == nullptr);
    CHECK(findOutermostClosedGroup(outerGroupNode) == nullptr);
    CHECK(findOutermostClosedGroup(innerGroupNode) == outerGroupNode);
    CHECK(findOutermostClosedGroup(entityNode) == outerGroupNode);
    CHECK(findOutermostClosedGroup(brushNode) == outerGroupNode);
    CHECK(findOutermostClosedGroup(patchNode) == outerGroupNode);
  }

  SECTION("Outer group open")
  {
    outerGroupNode->open();

    CHECK(findOutermostClosedGroup(&worldNode) == nullptr);
    CHECK(findOutermostClosedGroup(layerNode) == nullptr);
    CHECK(findOutermostClosedGroup(outerGroupNode) == nullptr);
    CHECK(findOutermostClosedGroup(innerGroupNode) == nullptr);
    CHECK(findOutermostClosedGroup(entityNode) == innerGroupNode);
    CHECK(findOutermostClosedGroup(brushNode) == innerGroupNode);
    CHECK(findOutermostClosedGroup(patchNode) == nullptr);
  }

  SECTION("Both groups open")
  {
    outerGroupNode->open();
    innerGroupNode->open();

    CHECK(findOutermostClosedGroup(&worldNode) == nullptr);
    CHECK(findOutermostClosedGroup(layerNode) == nullptr);
    CHECK(findOutermostClosedGroup(outerGroupNode) == nullptr);
    CHECK(findOutermostClosedGroup(innerGroupNode) == nullptr);
    CHECK(findOutermostClosedGroup(entityNode) == nullptr);
    CHECK(findOutermostClosedGroup(brushNode) == nullptr);
    CHECK(findOutermostClosedGroup(patchNode) == nullptr);
  }
}

TEST_CASE("ModelUtils.collectTouchingNodes")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"outer"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};
  auto patchNode = PatchNode{BezierPatch{
    3,
    3,
    {{0, 0, 0},
     {1, 0, 1},
     {2, 0, 0},
     {0, 1, 1},
     {1, 1, 2},
     {2, 1, 1},
     {0, 2, 0},
     {1, 2, 1},
     {2, 2, 0}},
    "material"}};

  groupNode.addChild(new EntityNode{Entity{}});

  auto touchesAll = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(24.0, "material") | kdl::value()};
  REQUIRE_FALSE(touchesAll.intersects(&worldNode));
  REQUIRE_FALSE(touchesAll.intersects(&layerNode));
  REQUIRE(touchesAll.intersects(&groupNode));
  REQUIRE(touchesAll.intersects(&entityNode));
  REQUIRE(touchesAll.intersects(&brushNode));
  REQUIRE(touchesAll.intersects(&patchNode));

  auto touchesNothing = BrushNode{touchesAll.brush()};
  transformNode(
    touchesNothing, vm::translation_matrix(vm::vec3d{128, 0, 0}), worldBounds);
  REQUIRE_FALSE(touchesNothing.intersects(&worldNode));
  REQUIRE_FALSE(touchesNothing.intersects(&layerNode));
  REQUIRE_FALSE(touchesNothing.intersects(&groupNode));
  REQUIRE_FALSE(touchesNothing.intersects(&entityNode));
  REQUIRE_FALSE(touchesNothing.intersects(&brushNode));
  REQUIRE_FALSE(touchesNothing.intersects(&patchNode));

  auto touchesBrush = BrushNode{touchesAll.brush()};
  transformNode(touchesBrush, vm::translation_matrix(vm::vec3d{24, 0, 0}), worldBounds);
  REQUIRE_FALSE(touchesBrush.intersects(&worldNode));
  REQUIRE_FALSE(touchesBrush.intersects(&layerNode));
  REQUIRE_FALSE(touchesBrush.intersects(&groupNode));
  REQUIRE_FALSE(touchesBrush.intersects(&entityNode));
  REQUIRE(touchesBrush.intersects(&brushNode));
  REQUIRE_FALSE(touchesBrush.intersects(&patchNode));

  const auto allNodes = std::vector<Node*>{
    &worldNode, &layerNode, &groupNode, &entityNode, &brushNode, &patchNode};

  CHECK_THAT(
    collectTouchingNodes(allNodes, {&touchesAll}),
    Equals(std::vector<Node*>{&groupNode, &entityNode, &brushNode, &patchNode}));

  CHECK_THAT(
    collectTouchingNodes(allNodes, {&touchesNothing}), Equals(std::vector<Node*>{}));

  CHECK_THAT(
    collectTouchingNodes(allNodes, {&touchesBrush}),
    Equals(std::vector<Node*>{&brushNode}));

  CHECK_THAT(
    collectTouchingNodes(allNodes, {&touchesBrush, &touchesAll}),
    Equals(std::vector<Node*>{&groupNode, &entityNode, &brushNode, &patchNode}));
}

TEST_CASE("ModelUtils.collectContainedNodes")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"outer"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};
  auto patchNode = PatchNode{BezierPatch{
    3,
    3,
    {{0, 0, 0},
     {1, 0, 1},
     {2, 0, 0},
     {0, 1, 1},
     {1, 1, 2},
     {2, 1, 1},
     {0, 2, 0},
     {1, 2, 1},
     {2, 2, 0}},
    "material"}};

  groupNode.addChild(new EntityNode{Entity{}});

  auto containsAll = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(128.0, "material") | kdl::value()};
  REQUIRE_FALSE(containsAll.contains(&worldNode));
  REQUIRE_FALSE(containsAll.contains(&layerNode));
  REQUIRE(containsAll.contains(&groupNode));
  REQUIRE(containsAll.contains(&entityNode));
  REQUIRE(containsAll.contains(&brushNode));
  REQUIRE(containsAll.contains(&patchNode));

  auto containsNothing = BrushNode{containsAll.brush()};
  transformNode(
    containsNothing, vm::translation_matrix(vm::vec3d{-64, 0, 0}), worldBounds);
  REQUIRE_FALSE(containsNothing.contains(&worldNode));
  REQUIRE_FALSE(containsNothing.contains(&layerNode));
  REQUIRE_FALSE(containsNothing.contains(&groupNode));
  REQUIRE_FALSE(containsNothing.contains(&entityNode));
  REQUIRE_FALSE(containsNothing.contains(&brushNode));
  REQUIRE_FALSE(containsNothing.contains(&patchNode));

  auto containsPatch = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(8.0, "material") | kdl::value()};
  REQUIRE_FALSE(containsPatch.contains(&worldNode));
  REQUIRE_FALSE(containsPatch.contains(&layerNode));
  REQUIRE_FALSE(containsPatch.contains(&groupNode));
  REQUIRE_FALSE(containsPatch.contains(&entityNode));
  REQUIRE_FALSE(containsPatch.contains(&brushNode));
  REQUIRE(containsPatch.contains(&patchNode));

  const auto allNodes = std::vector<Node*>{
    &worldNode, &layerNode, &groupNode, &entityNode, &brushNode, &patchNode};

  CHECK_THAT(
    collectContainedNodes(allNodes, {&containsAll}),
    Equals(std::vector<Node*>{&groupNode, &entityNode, &brushNode, &patchNode}));

  CHECK_THAT(
    collectContainedNodes(allNodes, {&containsNothing}), Equals(std::vector<Node*>{}));

  CHECK_THAT(
    collectContainedNodes(allNodes, {&containsPatch}),
    Equals(std::vector<Node*>{&patchNode}));

  CHECK_THAT(
    collectContainedNodes(allNodes, {&containsPatch, &containsAll}),
    Equals(std::vector<Node*>{&groupNode, &entityNode, &brushNode, &patchNode}));
}

TEST_CASE("ModelUtils.collectSelectedNodes")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  auto* layerNode = new LayerNode{Layer{"layer"}};
  auto* outerGroupNode = new GroupNode{Group{"outer"}};
  auto* innerGroupNode = new GroupNode{Group{"inner"}};
  auto* entityNode = new EntityNode{Entity{}};
  auto* brushNode = new BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto* patchNode = new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  innerGroupNode->addChildren({entityNode, brushNode});
  outerGroupNode->addChildren({innerGroupNode, patchNode});
  worldNode.defaultLayer()->addChild(outerGroupNode);
  worldNode.addChild(layerNode);

  /*
  worldNode
  + defaultLayer
    + outerGroupNode
      + innerGroupNode
        + entityNode
        + brushNode
      + patchNode
  + layerNode
  */
  CHECK_THAT(collectSelectedNodes({&worldNode}), UnorderedEquals(std::vector<Node*>{}));

  brushNode->select();
  patchNode->select();

  CHECK_THAT(
    collectSelectedNodes({&worldNode}),
    UnorderedEquals(std::vector<Node*>{brushNode, patchNode}));

  CHECK_THAT(
    collectSelectedNodes({outerGroupNode}),
    UnorderedEquals(std::vector<Node*>{brushNode, patchNode}));

  CHECK_THAT(
    collectSelectedNodes({innerGroupNode}),
    UnorderedEquals(std::vector<Node*>{brushNode}));

  CHECK_THAT(
    collectSelectedNodes({innerGroupNode, patchNode}),
    UnorderedEquals(std::vector<Node*>{brushNode, patchNode}));

  CHECK_THAT(
    collectSelectedNodes({outerGroupNode, innerGroupNode}),
    UnorderedEquals(std::vector<Node*>{brushNode, patchNode}));

  innerGroupNode->select();
  CHECK_THAT(
    collectSelectedNodes({outerGroupNode, innerGroupNode}),
    UnorderedEquals(std::vector<Node*>{innerGroupNode, brushNode, patchNode}));
}

TEST_CASE("ModelUtils.collectSelectableNodes")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  auto* layerNode = new LayerNode{Layer{"layer"}};
  auto* outerGroupNode = new GroupNode{Group{"outer"}};
  auto* innerGroupNode = new GroupNode{Group{"inner"}};
  auto* entityNode = new EntityNode{Entity{}};
  auto* brushNode = new BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto* patchNode = new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  innerGroupNode->addChildren({entityNode, brushNode});
  outerGroupNode->addChildren({innerGroupNode, patchNode});
  worldNode.defaultLayer()->addChild(outerGroupNode);
  worldNode.addChild(layerNode);

  auto editorContext = EditorContext{};

  CHECK_THAT(collectSelectableNodes({}, editorContext), Equals(std::vector<Node*>{}));

  CHECK_THAT(
    collectSelectableNodes({&worldNode}, editorContext),
    Equals(std::vector<Node*>{outerGroupNode}));

  editorContext.pushGroup(*outerGroupNode);
  CHECK_THAT(
    collectSelectableNodes({&worldNode}, editorContext),
    Equals(std::vector<Node*>{innerGroupNode, patchNode}));

  editorContext.pushGroup(*innerGroupNode);
  CHECK_THAT(
    collectSelectableNodes({&worldNode}, editorContext),
    Equals(std::vector<Node*>{outerGroupNode}));

  CHECK_THAT(
    collectSelectableNodes({&worldNode, innerGroupNode}, editorContext),
    Equals(std::vector<Node*>{outerGroupNode, entityNode, brushNode}));
}

TEST_CASE("ModelUtils.collectSelectedBrushFaces")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  SECTION("Face selection")
  {
    auto* brushNode = new BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

    worldNode.defaultLayer()->addChild(brushNode);
    brushNode->selectFace(0);
    brushNode->selectFace(1);

    CHECK_THAT(
      collectSelectedBrushFaces({&worldNode}),
      UnorderedEquals(std::vector<BrushFaceHandle>{{brushNode, 0u}, {brushNode, 1u}}));
  }

  SECTION("Node selection")
  {
    auto* selectedBrushNode = new BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};
    auto* unselectedBrushNode = new BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

    worldNode.defaultLayer()->addChild(selectedBrushNode);
    worldNode.defaultLayer()->addChild(unselectedBrushNode);
    selectedBrushNode->select();

    CHECK(collectSelectedBrushFaces({&worldNode}).empty());
  }
}

TEST_CASE("ModelUtils.collectSelectableBrushFaces")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};
  auto* selectableBrushNode = new BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};
  auto* unselectableBrushNode = new BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  worldNode.defaultLayer()->addChild(selectableBrushNode);
  worldNode.defaultLayer()->addChild(unselectableBrushNode);
  unselectableBrushNode->setLockState(LockState::Locked);

  auto editorContext = EditorContext{};

  CHECK_THAT(
    collectSelectableBrushFaces({&worldNode}, editorContext),
    UnorderedEquals(toHandles(selectableBrushNode)));
}

TEST_CASE("ModelUtils.computeLogicalBounds")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  auto* layerNode = new LayerNode{Layer{"layer"}};
  auto* outerGroupNode = new GroupNode{Group{"outer"}};
  auto* innerGroupNode = new GroupNode{Group{"inner"}};
  auto* entityNode = new EntityNode{Entity{}};
  auto* brushNode = new BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto* patchNode = new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  innerGroupNode->addChildren({entityNode, brushNode});
  outerGroupNode->addChildren({innerGroupNode, patchNode});
  worldNode.defaultLayer()->addChild(outerGroupNode);
  worldNode.addChild(layerNode);

  transformNode(*brushNode, vm::translation_matrix(vm::vec3d{64, 0, 0}), worldBounds);

  CHECK(computeLogicalBounds({&worldNode}) == vm::bbox3d{});
  CHECK(computeLogicalBounds({layerNode}) == vm::bbox3d{});
  CHECK(
    computeLogicalBounds({entityNode})
    == vm::bbox3d{vm::vec3d{-8, -8, -8}, vm::vec3d{8, 8, 8}});
  CHECK(
    computeLogicalBounds({brushNode})
    == vm::bbox3d{vm::vec3d{32, -32, -32}, vm::vec3d{96, 32, 32}});
  CHECK(
    computeLogicalBounds({patchNode})
    == vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{2, 2, 2}});
  CHECK(
    computeLogicalBounds({entityNode, brushNode})
    == vm::bbox3d{vm::vec3d{-8, -32, -32}, vm::vec3d{96, 32, 32}});
}

TEST_CASE("ModelUtils.computePhysicalBounds")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  auto* layerNode = new LayerNode{Layer{"layer"}};
  auto* outerGroupNode = new GroupNode{Group{"outer"}};
  auto* innerGroupNode = new GroupNode{Group{"inner"}};
  auto* entityNode = new EntityNode{Entity{}};
  auto* brushNode = new BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto* patchNode = new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  innerGroupNode->addChildren({entityNode, brushNode});
  outerGroupNode->addChildren({innerGroupNode, patchNode});
  worldNode.defaultLayer()->addChild(outerGroupNode);
  worldNode.addChild(layerNode);

  transformNode(*brushNode, vm::translation_matrix(vm::vec3d{64, 0, 0}), worldBounds);

  CHECK(computePhysicalBounds({&worldNode}) == vm::bbox3d{});
  CHECK(computePhysicalBounds({layerNode}) == vm::bbox3d{});
  CHECK(
    computePhysicalBounds({entityNode})
    == vm::bbox3d{vm::vec3d{-8, -8, -8}, vm::vec3d{8, 8, 8}});
  CHECK(
    computePhysicalBounds({brushNode})
    == vm::bbox3d{vm::vec3d{32, -32, -32}, vm::vec3d{96, 32, 32}});
  CHECK(
    computePhysicalBounds({patchNode})
    == vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{2, 2, 1}});
  CHECK(
    computePhysicalBounds({entityNode, brushNode})
    == vm::bbox3d{vm::vec3d{-8, -32, -32}, vm::vec3d{96, 32, 32}});
}

TEST_CASE("ModelUtils.filterNodes")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"outer"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};
  auto patchNode = PatchNode{BezierPatch{
    3,
    3,
    {{0, 0, 0},
     {1, 0, 1},
     {2, 0, 0},
     {0, 1, 1},
     {1, 1, 2},
     {2, 1, 1},
     {0, 2, 0},
     {1, 2, 1},
     {2, 2, 0}},
    "material"}};

  SECTION("Filter brush nodes")
  {
    CHECK(
      filterBrushNodes(
        {&worldNode, &layerNode, &groupNode, &entityNode, &brushNode, &patchNode})
      == std::vector<BrushNode*>{&brushNode});
  }

  SECTION("Filter entity nodes")
  {
    CHECK(
      filterEntityNodes(
        {&worldNode, &layerNode, &groupNode, &entityNode, &brushNode, &patchNode})
      == std::vector<EntityNode*>{&entityNode});
  }
}

} // namespace tb::mdl
