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

#include "Catch2.h"
#include "Model/BezierPatch.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/Layer.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/NodeCollection.h"
#include "Model/PatchNode.h"

#include <kdl/result.h>
#include <kdl/result_io.h>

#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>

#include <vector>

namespace TrenchBroom
{
namespace Model
{
TEST_CASE("NodeCollectionTest.empty")
{
  auto nodeCollection = NodeCollection{};
  CHECK(nodeCollection.empty());

  auto entityNode = EntityNode{Entity{}};
  nodeCollection.addNode(&entityNode);
  REQUIRE_THAT(
    nodeCollection.nodes(),
    Catch::Matchers::UnorderedEquals(std::vector<Node*>{&entityNode}));

  CHECK_FALSE(nodeCollection.empty());
}

TEST_CASE("NodeCollection.counts")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3{8192.0};

  auto nodeCollection = NodeCollection{};
  REQUIRE(nodeCollection.nodeCount() == 0u);
  REQUIRE(nodeCollection.layerCount() == 0u);
  REQUIRE(nodeCollection.groupCount() == 0u);
  REQUIRE(nodeCollection.entityCount() == 0u);
  REQUIRE(nodeCollection.brushCount() == 0u);
  REQUIRE(nodeCollection.patchCount() == 0u);

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode =
    BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  nodeCollection.addNodes({&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});
  CHECK(nodeCollection.nodeCount() == 5u);
  CHECK(nodeCollection.layerCount() == 1u);
  CHECK(nodeCollection.groupCount() == 1u);
  CHECK(nodeCollection.entityCount() == 1u);
  CHECK(nodeCollection.brushCount() == 1u);
  CHECK(nodeCollection.patchCount() == 1u);
}

TEST_CASE("NodeCollection.has")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode =
    BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  auto nodeCollection = NodeCollection{};

  REQUIRE_FALSE(nodeCollection.hasLayers());
  REQUIRE_FALSE(nodeCollection.hasOnlyLayers());
  REQUIRE_FALSE(nodeCollection.hasGroups());
  REQUIRE_FALSE(nodeCollection.hasOnlyGroups());
  REQUIRE_FALSE(nodeCollection.hasEntities());
  REQUIRE_FALSE(nodeCollection.hasOnlyEntities());
  REQUIRE_FALSE(nodeCollection.hasBrushes());
  REQUIRE_FALSE(nodeCollection.hasOnlyBrushes());
  REQUIRE_FALSE(nodeCollection.hasPatches());
  REQUIRE_FALSE(nodeCollection.hasOnlyPatches());

  SECTION("layer")
  {
    nodeCollection.addNode(&layerNode);
    CHECK(nodeCollection.hasLayers());
    CHECK(nodeCollection.hasOnlyLayers());

    nodeCollection.addNode(&brushNode);
    CHECK(nodeCollection.hasLayers());
    CHECK_FALSE(nodeCollection.hasOnlyLayers());
  }

  SECTION("groups")
  {
    nodeCollection.addNode(&groupNode);
    CHECK(nodeCollection.hasGroups());
    CHECK(nodeCollection.hasOnlyGroups());

    nodeCollection.addNode(&brushNode);
    CHECK(nodeCollection.hasGroups());
    CHECK_FALSE(nodeCollection.hasOnlyGroups());
  }

  SECTION("entities")
  {
    nodeCollection.addNode(&entityNode);
    CHECK(nodeCollection.hasEntities());
    CHECK(nodeCollection.hasOnlyEntities());

    nodeCollection.addNode(&brushNode);
    CHECK(nodeCollection.hasEntities());
    CHECK_FALSE(nodeCollection.hasOnlyEntities());
  }

  SECTION("brushes")
  {
    SECTION("only top level")
    {
      nodeCollection.addNode(&brushNode);
      CHECK(nodeCollection.hasBrushes());
      CHECK(nodeCollection.hasOnlyBrushes());

      nodeCollection.addNode(&layerNode);
      CHECK(nodeCollection.hasBrushes());
      CHECK_FALSE(nodeCollection.hasOnlyBrushes());
    }

    SECTION("nested brushes")
    {
      auto* entityNodePtr = static_cast<Node*>(&entityNode);
      auto* groupNodePtr = static_cast<Node*>(&groupNode);
      auto* node = GENERATE_COPY(entityNodePtr, groupNodePtr);

      SECTION("adding already nested brush")
      {
        node->addChild(brushNode.clone(worldBounds));

        nodeCollection.addNode(node);
        CHECK_FALSE(nodeCollection.hasBrushes());
        CHECK_FALSE(nodeCollection.hasOnlyBrushes());
      }

      SECTION("adding brushes to containers")
      {
        nodeCollection.addNode(node);
        REQUIRE_FALSE(nodeCollection.hasBrushes());
        REQUIRE_FALSE(nodeCollection.hasOnlyBrushes());

        node->addChild(brushNode.clone(worldBounds));
        CHECK_FALSE(nodeCollection.hasBrushes());
        CHECK_FALSE(nodeCollection.hasOnlyBrushes());
      }
    }
  }

  SECTION("patches")
  {
    nodeCollection.addNode(&patchNode);
    CHECK(nodeCollection.hasPatches());
    CHECK(nodeCollection.hasOnlyPatches());

    nodeCollection.addNode(&brushNode);
    CHECK(nodeCollection.hasPatches());
    CHECK_FALSE(nodeCollection.hasOnlyPatches());
  }
}

TEST_CASE("NodeCollection.iterators")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode =
    BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  auto nodeCollection = NodeCollection{};

  REQUIRE_THAT(
    std::vector<Node*>(nodeCollection.begin(), nodeCollection.end()),
    Catch::Matchers::UnorderedEquals(std::vector<Node*>{}));

  nodeCollection.addNodes({&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});

  CHECK_THAT(
    std::vector<Node*>(nodeCollection.begin(), nodeCollection.end()),
    Catch::Matchers::UnorderedEquals(
      std::vector<Node*>{&layerNode, &groupNode, &entityNode, &brushNode, &patchNode}));
}

TEST_CASE("NodeCollection.collections")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode =
    BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  auto nodeCollection = NodeCollection{};

  REQUIRE_THAT(
    std::vector<Node*>(nodeCollection.begin(), nodeCollection.end()),
    Catch::Matchers::UnorderedEquals(std::vector<Node*>{}));

  nodeCollection.addNodes({&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});

  CHECK_THAT(
    nodeCollection.nodes(),
    Catch::Matchers::UnorderedEquals(
      std::vector<Node*>{&layerNode, &groupNode, &entityNode, &brushNode, &patchNode}));

  CHECK_THAT(
    nodeCollection.layers(),
    Catch::Matchers::UnorderedEquals(std::vector<LayerNode*>{&layerNode}));

  CHECK_THAT(
    nodeCollection.groups(),
    Catch::Matchers::UnorderedEquals(std::vector<GroupNode*>{&groupNode}));

  CHECK_THAT(
    nodeCollection.entities(),
    Catch::Matchers::UnorderedEquals(std::vector<EntityNode*>{&entityNode}));

  CHECK_THAT(
    nodeCollection.brushes(),
    Catch::Matchers::UnorderedEquals(std::vector<BrushNode*>{&brushNode}));

  CHECK_THAT(
    nodeCollection.patches(),
    Catch::Matchers::UnorderedEquals(std::vector<PatchNode*>{&patchNode}));

  SECTION("nested brushes")
  {
    auto* brushInLayer = new BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};
    auto* brushInGroup = new BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};
    auto* brushInEntity = new BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

    layerNode.addChild(brushInLayer);
    groupNode.addChild(brushInGroup);
    entityNode.addChild(brushInEntity);
  }
}

TEST_CASE("NodeCollection.addNode")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode =
    BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  auto nodeCollection = NodeCollection{};

  REQUIRE_THAT(
    std::vector<Node*>(nodeCollection.begin(), nodeCollection.end()),
    Catch::Matchers::UnorderedEquals(std::vector<Node*>{}));

  SECTION("layer")
  {
    nodeCollection.addNode(&layerNode);
    CHECK(nodeCollection.nodes() == std::vector<Node*>{&layerNode});
    CHECK(nodeCollection.layers() == std::vector<LayerNode*>{&layerNode});
  }

  SECTION("group")
  {
    nodeCollection.addNode(&groupNode);
    CHECK(nodeCollection.nodes() == std::vector<Node*>{&groupNode});
    CHECK(nodeCollection.groups() == std::vector<GroupNode*>{&groupNode});
  }

  SECTION("entity")
  {
    nodeCollection.addNode(&entityNode);
    CHECK(nodeCollection.nodes() == std::vector<Node*>{&entityNode});
    CHECK(nodeCollection.entities() == std::vector<EntityNode*>{&entityNode});
  }

  SECTION("brush")
  {
    nodeCollection.addNode(&brushNode);
    CHECK(nodeCollection.nodes() == std::vector<Node*>{&brushNode});
    CHECK(nodeCollection.brushes() == std::vector<BrushNode*>{&brushNode});
  }

  SECTION("patch")
  {
    nodeCollection.addNode(&patchNode);
    CHECK(nodeCollection.nodes() == std::vector<Node*>{&patchNode});
    CHECK(nodeCollection.patches() == std::vector<PatchNode*>{&patchNode});
  }
}

TEST_CASE("NodeCollection.addNodes")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode =
    BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  auto nodeCollection = NodeCollection{};

  REQUIRE_THAT(
    std::vector<Node*>(nodeCollection.begin(), nodeCollection.end()),
    Catch::Matchers::UnorderedEquals(std::vector<Node*>{}));

  nodeCollection.addNodes({&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});

  CHECK(
    nodeCollection.nodes()
    == std::vector<Node*>{&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});
  CHECK(nodeCollection.layers() == std::vector<LayerNode*>{&layerNode});
  CHECK(nodeCollection.groups() == std::vector<GroupNode*>{&groupNode});
  CHECK(nodeCollection.entities() == std::vector<EntityNode*>{&entityNode});
  CHECK(nodeCollection.brushes() == std::vector<BrushNode*>{&brushNode});
  CHECK(nodeCollection.patches() == std::vector<PatchNode*>{&patchNode});
}

TEST_CASE("NodeCollection.removeNode")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode =
    BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  auto nodeCollection = NodeCollection{};
  nodeCollection.addNodes({&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});
  REQUIRE(
    nodeCollection.nodes()
    == std::vector<Node*>{&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});

  SECTION("layer")
  {
    nodeCollection.removeNode(&layerNode);
    CHECK(
      nodeCollection.nodes()
      == std::vector<Node*>{&groupNode, &entityNode, &brushNode, &patchNode});
    CHECK(nodeCollection.layers() == std::vector<LayerNode*>{});
  }

  SECTION("group")
  {
    nodeCollection.removeNode(&groupNode);
    CHECK(
      nodeCollection.nodes()
      == std::vector<Node*>{&layerNode, &entityNode, &brushNode, &patchNode});
    CHECK(nodeCollection.groups() == std::vector<GroupNode*>{});
  }

  SECTION("entity")
  {
    nodeCollection.removeNode(&entityNode);
    CHECK(
      nodeCollection.nodes()
      == std::vector<Node*>{&layerNode, &groupNode, &brushNode, &patchNode});
    CHECK(nodeCollection.entities() == std::vector<EntityNode*>{});
  }

  SECTION("brush")
  {
    nodeCollection.removeNode(&brushNode);
    CHECK(
      nodeCollection.nodes()
      == std::vector<Node*>{&layerNode, &groupNode, &entityNode, &patchNode});
    CHECK(nodeCollection.brushes() == std::vector<BrushNode*>{});
  }

  SECTION("patch")
  {
    nodeCollection.removeNode(&patchNode);
    CHECK(
      nodeCollection.nodes()
      == std::vector<Node*>{&layerNode, &groupNode, &entityNode, &brushNode});
    CHECK(nodeCollection.patches() == std::vector<PatchNode*>{});
  }
}

TEST_CASE("NodeCollection.clear")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode =
    BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  auto nodeCollection = NodeCollection{};
  nodeCollection.addNodes({&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});
  REQUIRE(
    nodeCollection.nodes()
    == std::vector<Node*>{&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});

  nodeCollection.clear();

  CHECK(nodeCollection.nodes() == std::vector<Node*>{});
  CHECK(nodeCollection.layers() == std::vector<LayerNode*>{});
  CHECK(nodeCollection.groups() == std::vector<GroupNode*>{});
  CHECK(nodeCollection.entities() == std::vector<EntityNode*>{});
  CHECK(nodeCollection.brushes() == std::vector<BrushNode*>{});
  CHECK(nodeCollection.patches() == std::vector<PatchNode*>{});
}
} // namespace Model
} // namespace TrenchBroom
