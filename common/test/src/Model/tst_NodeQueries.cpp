/*
 Copyright (C) 2023 Kristian Duske

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

#include "Error.h"
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
#include "Model/NodeQueries.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"

#include "kdl/result.h"

#include "Catch2.h"

namespace TrenchBroom::Model
{

TEST_CASE("NodeQueries")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};

  auto* layerNode = new LayerNode{Layer{"layer"}};
  auto* outerGroupNode = new GroupNode{Group{"outer"}};
  auto* innerGroupNode = new GroupNode{Group{"inner"}};
  auto* entityNode = new EntityNode{Entity{}};
  auto* brushNode = new BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

  // clang-format off
  auto* patchNode = new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  innerGroupNode->addChildren({entityNode, brushNode});
  outerGroupNode->addChildren({innerGroupNode, patchNode});
  layerNode->addChild(outerGroupNode);
  worldNode.addChild(layerNode);

  /*
  worldNode
  + defaultLayer
  + layerNode
    + outerGroupNode
      + innerGroupNode
        + entityNode
        + brushNode
      + patchNode
  */

  SECTION("collectNodes")
  {
    CHECK(collectNodes({}).empty());
    CHECK(collectNodes({&worldNode}) == std::vector<Node*>{&worldNode});
    CHECK(
      collectNodes({outerGroupNode, entityNode})
      == std::vector<Node*>{outerGroupNode, entityNode});
    CHECK(
      collectNodes({outerGroupNode, entityNode}, [&](const EntityNode*) { return true; })
      == std::vector<Node*>{entityNode});
  }

  SECTION("collectAncestors")
  {
    CHECK_THAT(
      collectAncestors({&worldNode}), Catch::UnorderedEquals(std::vector<Node*>{}));
    CHECK_THAT(
      collectAncestors({layerNode}),
      Catch::UnorderedEquals(std::vector<Node*>{&worldNode}));
    CHECK_THAT(
      collectAncestors({outerGroupNode}),
      Catch::UnorderedEquals(std::vector<Node*>{&worldNode, layerNode}));
    CHECK_THAT(
      collectAncestors({innerGroupNode}),
      Catch::UnorderedEquals(std::vector<Node*>{&worldNode, layerNode, outerGroupNode}));
    CHECK_THAT(
      collectAncestors({entityNode}),
      Catch::UnorderedEquals(
        std::vector<Node*>{&worldNode, layerNode, outerGroupNode, innerGroupNode}));
    CHECK_THAT(
      collectAncestors({brushNode}),
      Catch::UnorderedEquals(
        std::vector<Node*>{&worldNode, layerNode, outerGroupNode, innerGroupNode}));
    CHECK_THAT(
      collectAncestors({patchNode}),
      Catch::UnorderedEquals(std::vector<Node*>{&worldNode, layerNode, outerGroupNode}));
    CHECK_THAT(
      collectAncestors({brushNode, patchNode}),
      Catch::UnorderedEquals(
        std::vector<Node*>{&worldNode, layerNode, outerGroupNode, innerGroupNode}));
    CHECK_THAT(
      collectAncestors({brushNode, patchNode}, [](const LayerNode*) { return true; }),
      Catch::UnorderedEquals(std::vector<Node*>{layerNode}));
  }

  SECTION("collectNodesAndAncestors")
  {
    CHECK_THAT(
      collectNodesAndAncestors({&worldNode}),
      Catch::UnorderedEquals(std::vector<Node*>{&worldNode}));
    CHECK_THAT(
      collectNodesAndAncestors({brushNode, patchNode}),
      Catch::UnorderedEquals(std::vector<Node*>{
        &worldNode, layerNode, outerGroupNode, innerGroupNode, brushNode, patchNode}));
    CHECK_THAT(
      collectNodesAndAncestors(
        {brushNode, patchNode}, [](const GroupNode*) { return true; }),
      Catch::UnorderedEquals(std::vector<Node*>{outerGroupNode, innerGroupNode}));
  }

  SECTION("collectDescendants")
  {
    CHECK_THAT(
      collectDescendants({&worldNode}),
      Catch::UnorderedEquals(std::vector<Node*>{
        worldNode.defaultLayer(),
        layerNode,
        outerGroupNode,
        innerGroupNode,
        entityNode,
        brushNode,
        patchNode}));
    CHECK_THAT(
      collectDescendants({layerNode}),
      Catch::UnorderedEquals(std::vector<Node*>{
        outerGroupNode, innerGroupNode, entityNode, brushNode, patchNode}));
    CHECK_THAT(
      collectDescendants({outerGroupNode}),
      Catch::UnorderedEquals(
        std::vector<Node*>{innerGroupNode, entityNode, brushNode, patchNode}));
    CHECK_THAT(
      collectDescendants({innerGroupNode}),
      Catch::UnorderedEquals(std::vector<Node*>{entityNode, brushNode}));
    CHECK_THAT(
      collectDescendants({entityNode}), Catch::UnorderedEquals(std::vector<Node*>{}));
    CHECK_THAT(
      collectDescendants({innerGroupNode, outerGroupNode}),
      Catch::UnorderedEquals(
        std::vector<Node*>{innerGroupNode, entityNode, brushNode, patchNode}));
    CHECK_THAT(
      collectDescendants({&worldNode}, [](const GroupNode*) { return true; }),
      Catch::UnorderedEquals(std::vector<Node*>{outerGroupNode, innerGroupNode}));
  }

  SECTION("collectNodesAndDescendants")
  {
    CHECK_THAT(
      collectNodesAndDescendants({innerGroupNode}),
      Catch::UnorderedEquals(std::vector<Node*>{innerGroupNode, entityNode, brushNode}));
    CHECK_THAT(
      collectNodesAndDescendants({entityNode}),
      Catch::UnorderedEquals(std::vector<Node*>{entityNode}));
    CHECK_THAT(
      collectNodesAndDescendants({innerGroupNode, outerGroupNode}),
      Catch::UnorderedEquals(std::vector<Node*>{
        outerGroupNode, innerGroupNode, entityNode, brushNode, patchNode}));
    CHECK_THAT(
      collectNodesAndDescendants(
        {innerGroupNode, outerGroupNode}, [](const GroupNode*) { return true; }),
      Catch::UnorderedEquals(std::vector<Node*>{outerGroupNode, innerGroupNode}));
  }
}

TEST_CASE("collectBrushFaces")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};
  auto* brushNode = new BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};

  worldNode.defaultLayer()->addChild(brushNode);

  CHECK_THAT(
    collectBrushFaces({&worldNode}),
    Catch::Matchers::UnorderedEquals(toHandles(brushNode)));
  CHECK_THAT(
    collectBrushFaces({brushNode, brushNode}),
    Catch::Matchers::UnorderedEquals(toHandles(brushNode)));
}

} // namespace TrenchBroom::Model
