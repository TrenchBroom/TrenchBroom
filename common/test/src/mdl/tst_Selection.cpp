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
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/MapFormat.h"
#include "mdl/PatchNode.h"
#include "mdl/Selection.h"

#include "kdl/result.h"

#include <vector>

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Selection.empty")
{
  auto selection = Selection{};
  CHECK(selection.empty());

  auto entityNode = EntityNode{Entity{}};
  selection.addNode(&entityNode);
  REQUIRE_THAT(
    selection.nodes, Catch::Matchers::UnorderedEquals(std::vector<Node*>{&entityNode}));

  CHECK_FALSE(selection.empty());
}

TEST_CASE("Selection.has")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3d{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  auto selection = Selection{};

  REQUIRE_FALSE(selection.hasLayers());
  REQUIRE_FALSE(selection.hasOnlyLayers());
  REQUIRE_FALSE(selection.hasGroups());
  REQUIRE_FALSE(selection.hasOnlyGroups());
  REQUIRE_FALSE(selection.hasEntities());
  REQUIRE_FALSE(selection.hasOnlyEntities());
  REQUIRE_FALSE(selection.hasBrushes());
  REQUIRE_FALSE(selection.hasOnlyBrushes());
  REQUIRE_FALSE(selection.hasPatches());
  REQUIRE_FALSE(selection.hasOnlyPatches());

  SECTION("layer")
  {
    selection.addNode(&layerNode);
    CHECK(selection.hasLayers());
    CHECK(selection.hasOnlyLayers());

    selection.addNode(&brushNode);
    CHECK(selection.hasLayers());
    CHECK_FALSE(selection.hasOnlyLayers());
  }

  SECTION("groups")
  {
    selection.addNode(&groupNode);
    CHECK(selection.hasGroups());
    CHECK(selection.hasOnlyGroups());

    selection.addNode(&brushNode);
    CHECK(selection.hasGroups());
    CHECK_FALSE(selection.hasOnlyGroups());
  }

  SECTION("entities")
  {
    selection.addNode(&entityNode);
    CHECK(selection.hasEntities());
    CHECK(selection.hasOnlyEntities());

    selection.addNode(&brushNode);
    CHECK(selection.hasEntities());
    CHECK_FALSE(selection.hasOnlyEntities());
  }

  SECTION("brushes")
  {
    SECTION("only top level")
    {
      selection.addNode(&brushNode);
      CHECK(selection.hasBrushes());
      CHECK(selection.hasOnlyBrushes());

      selection.addNode(&layerNode);
      CHECK(selection.hasBrushes());
      CHECK_FALSE(selection.hasOnlyBrushes());
    }

    SECTION("nested brushes")
    {
      auto* entityNodePtr = static_cast<Node*>(&entityNode);
      auto* groupNodePtr = static_cast<Node*>(&groupNode);
      auto* node = GENERATE_COPY(entityNodePtr, groupNodePtr);

      SECTION("adding already nested brush")
      {
        node->addChild(brushNode.clone(worldBounds));

        selection.addNode(node);
        CHECK_FALSE(selection.hasBrushes());
        CHECK_FALSE(selection.hasOnlyBrushes());
      }

      SECTION("adding brushes to containers")
      {
        selection.addNode(node);
        REQUIRE_FALSE(selection.hasBrushes());
        REQUIRE_FALSE(selection.hasOnlyBrushes());

        node->addChild(brushNode.clone(worldBounds));
        CHECK_FALSE(selection.hasBrushes());
        CHECK_FALSE(selection.hasOnlyBrushes());
      }
    }
  }

  SECTION("patches")
  {
    selection.addNode(&patchNode);
    CHECK(selection.hasPatches());
    CHECK(selection.hasOnlyPatches());

    selection.addNode(&brushNode);
    CHECK(selection.hasPatches());
    CHECK_FALSE(selection.hasOnlyPatches());
  }
}

TEST_CASE("Selection.collections")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3d{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  auto selection = Selection{};

  REQUIRE_THAT(selection.nodes, Catch::Matchers::UnorderedEquals(std::vector<Node*>{}));

  selection.addNodes({&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});

  CHECK_THAT(
    selection.nodes,
    Catch::Matchers::UnorderedEquals(
      std::vector<Node*>{&layerNode, &groupNode, &entityNode, &brushNode, &patchNode}));

  CHECK_THAT(
    selection.layers,
    Catch::Matchers::UnorderedEquals(std::vector<LayerNode*>{&layerNode}));

  CHECK_THAT(
    selection.groups,
    Catch::Matchers::UnorderedEquals(std::vector<GroupNode*>{&groupNode}));

  CHECK_THAT(
    selection.entities,
    Catch::Matchers::UnorderedEquals(std::vector<EntityNode*>{&entityNode}));

  CHECK_THAT(
    selection.brushes,
    Catch::Matchers::UnorderedEquals(std::vector<BrushNode*>{&brushNode}));

  CHECK_THAT(
    selection.patches,
    Catch::Matchers::UnorderedEquals(std::vector<PatchNode*>{&patchNode}));

  SECTION("nested brushes")
  {
    auto* brushInLayer = new BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};
    auto* brushInGroup = new BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};
    auto* brushInEntity = new BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

    layerNode.addChild(brushInLayer);
    groupNode.addChild(brushInGroup);
    entityNode.addChild(brushInEntity);
  }
}

TEST_CASE("Selection.addNode")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3d{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  auto selection = Selection{};

  REQUIRE_THAT(selection.nodes, Catch::Matchers::UnorderedEquals(std::vector<Node*>{}));

  SECTION("layer")
  {
    selection.addNode(&layerNode);
    CHECK(selection.nodes == std::vector<Node*>{&layerNode});
    CHECK(selection.layers == std::vector<LayerNode*>{&layerNode});
  }

  SECTION("group")
  {
    selection.addNode(&groupNode);
    CHECK(selection.nodes == std::vector<Node*>{&groupNode});
    CHECK(selection.groups == std::vector<GroupNode*>{&groupNode});
  }

  SECTION("entity")
  {
    selection.addNode(&entityNode);
    CHECK(selection.nodes == std::vector<Node*>{&entityNode});
    CHECK(selection.entities == std::vector<EntityNode*>{&entityNode});
  }

  SECTION("brush")
  {
    selection.addNode(&brushNode);
    CHECK(selection.nodes == std::vector<Node*>{&brushNode});
    CHECK(selection.brushes == std::vector<BrushNode*>{&brushNode});
  }

  SECTION("patch")
  {
    selection.addNode(&patchNode);
    CHECK(selection.nodes == std::vector<Node*>{&patchNode});
    CHECK(selection.patches == std::vector<PatchNode*>{&patchNode});
  }
}

TEST_CASE("Selection.addNodes")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3d{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  auto selection = Selection{};

  REQUIRE_THAT(selection.nodes, Catch::Matchers::UnorderedEquals(std::vector<Node*>{}));

  selection.addNodes({&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});

  CHECK(
    selection.nodes
    == std::vector<Node*>{&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});
  CHECK(selection.layers == std::vector<LayerNode*>{&layerNode});
  CHECK(selection.groups == std::vector<GroupNode*>{&groupNode});
  CHECK(selection.entities == std::vector<EntityNode*>{&entityNode});
  CHECK(selection.brushes == std::vector<BrushNode*>{&brushNode});
  CHECK(selection.patches == std::vector<PatchNode*>{&patchNode});
}

TEST_CASE("Selection.removeNode")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3d{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  auto selection = Selection{};
  selection.addNodes({&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});
  REQUIRE(
    selection.nodes
    == std::vector<Node*>{&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});

  SECTION("layer")
  {
    selection.removeNode(&layerNode);
    CHECK(
      selection.nodes
      == std::vector<Node*>{&groupNode, &entityNode, &brushNode, &patchNode});
    CHECK(selection.layers == std::vector<LayerNode*>{});
  }

  SECTION("group")
  {
    selection.removeNode(&groupNode);
    CHECK(
      selection.nodes
      == std::vector<Node*>{&layerNode, &entityNode, &brushNode, &patchNode});
    CHECK(selection.groups == std::vector<GroupNode*>{});
  }

  SECTION("entity")
  {
    selection.removeNode(&entityNode);
    CHECK(
      selection.nodes
      == std::vector<Node*>{&layerNode, &groupNode, &brushNode, &patchNode});
    CHECK(selection.entities == std::vector<EntityNode*>{});
  }

  SECTION("brush")
  {
    selection.removeNode(&brushNode);
    CHECK(
      selection.nodes
      == std::vector<Node*>{&layerNode, &groupNode, &entityNode, &patchNode});
    CHECK(selection.brushes == std::vector<BrushNode*>{});
  }

  SECTION("patch")
  {
    selection.removeNode(&patchNode);
    CHECK(
      selection.nodes
      == std::vector<Node*>{&layerNode, &groupNode, &entityNode, &brushNode});
    CHECK(selection.patches == std::vector<PatchNode*>{});
  }
}

TEST_CASE("Selection.clear")
{
  const auto mapFormat = MapFormat::Quake3;
  const auto worldBounds = vm::bbox3d{8192.0};

  auto layerNode = LayerNode{Layer{"layer"}};
  auto groupNode = GroupNode{Group{"group"}};
  auto entityNode = EntityNode{Entity{}};
  auto brushNode = BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto patchNode = PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
  // clang-format on

  auto selection = Selection{};
  selection.addNodes({&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});
  REQUIRE(
    selection.nodes
    == std::vector<Node*>{&layerNode, &groupNode, &entityNode, &brushNode, &patchNode});

  selection.clear();

  CHECK(selection.nodes == std::vector<Node*>{});
  CHECK(selection.layers == std::vector<LayerNode*>{});
  CHECK(selection.groups == std::vector<GroupNode*>{});
  CHECK(selection.entities == std::vector<EntityNode*>{});
  CHECK(selection.brushes == std::vector<BrushNode*>{});
  CHECK(selection.patches == std::vector<PatchNode*>{});
}

} // namespace tb::mdl
