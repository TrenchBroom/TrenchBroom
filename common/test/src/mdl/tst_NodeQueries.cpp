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
#include "mdl/NodeQueries.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kdl/result.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

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
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  // clang-format off
  auto* patchNode = new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
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
    CHECK_THAT(collectAncestors({&worldNode}), UnorderedEquals(std::vector<Node*>{}));
    CHECK_THAT(
      collectAncestors({layerNode}), UnorderedEquals(std::vector<Node*>{&worldNode}));
    CHECK_THAT(
      collectAncestors({outerGroupNode}),
      UnorderedEquals(std::vector<Node*>{&worldNode, layerNode}));
    CHECK_THAT(
      collectAncestors({innerGroupNode}),
      UnorderedEquals(std::vector<Node*>{&worldNode, layerNode, outerGroupNode}));
    CHECK_THAT(
      collectAncestors({entityNode}),
      UnorderedEquals(
        std::vector<Node*>{&worldNode, layerNode, outerGroupNode, innerGroupNode}));
    CHECK_THAT(
      collectAncestors({brushNode}),
      UnorderedEquals(
        std::vector<Node*>{&worldNode, layerNode, outerGroupNode, innerGroupNode}));
    CHECK_THAT(
      collectAncestors({patchNode}),
      UnorderedEquals(std::vector<Node*>{&worldNode, layerNode, outerGroupNode}));
    CHECK_THAT(
      collectAncestors({brushNode, patchNode}),
      UnorderedEquals(
        std::vector<Node*>{&worldNode, layerNode, outerGroupNode, innerGroupNode}));
    CHECK_THAT(
      collectAncestors({brushNode, patchNode}, [](const LayerNode*) { return true; }),
      UnorderedEquals(std::vector<Node*>{layerNode}));
  }

  SECTION("collectNodesAndAncestors")
  {
    CHECK_THAT(
      collectNodesAndAncestors({&worldNode}),
      UnorderedEquals(std::vector<Node*>{&worldNode}));
    CHECK_THAT(
      collectNodesAndAncestors({brushNode, patchNode}),
      UnorderedEquals(std::vector<Node*>{
        &worldNode, layerNode, outerGroupNode, innerGroupNode, brushNode, patchNode}));
    CHECK_THAT(
      collectNodesAndAncestors(
        {brushNode, patchNode}, [](const GroupNode*) { return true; }),
      UnorderedEquals(std::vector<Node*>{outerGroupNode, innerGroupNode}));
  }

  SECTION("collectDescendants")
  {
    CHECK_THAT(
      collectDescendants({&worldNode}),
      UnorderedEquals(std::vector<Node*>{
        worldNode.defaultLayer(),
        layerNode,
        outerGroupNode,
        innerGroupNode,
        entityNode,
        brushNode,
        patchNode}));
    CHECK_THAT(
      collectDescendants({layerNode}),
      UnorderedEquals(std::vector<Node*>{
        outerGroupNode, innerGroupNode, entityNode, brushNode, patchNode}));
    CHECK_THAT(
      collectDescendants({outerGroupNode}),
      UnorderedEquals(
        std::vector<Node*>{innerGroupNode, entityNode, brushNode, patchNode}));
    CHECK_THAT(
      collectDescendants({innerGroupNode}),
      UnorderedEquals(std::vector<Node*>{entityNode, brushNode}));
    CHECK_THAT(collectDescendants({entityNode}), UnorderedEquals(std::vector<Node*>{}));
    CHECK_THAT(
      collectDescendants({innerGroupNode, outerGroupNode}),
      UnorderedEquals(
        std::vector<Node*>{innerGroupNode, entityNode, brushNode, patchNode}));
    CHECK_THAT(
      collectDescendants({&worldNode}, [](const GroupNode*) { return true; }),
      UnorderedEquals(std::vector<Node*>{outerGroupNode, innerGroupNode}));
  }

  SECTION("collectNodesAndDescendants")
  {
    CHECK_THAT(
      collectNodesAndDescendants({innerGroupNode}),
      UnorderedEquals(std::vector<Node*>{innerGroupNode, entityNode, brushNode}));
    CHECK_THAT(
      collectNodesAndDescendants({entityNode}),
      UnorderedEquals(std::vector<Node*>{entityNode}));
    CHECK_THAT(
      collectNodesAndDescendants({innerGroupNode, outerGroupNode}),
      UnorderedEquals(std::vector<Node*>{
        outerGroupNode, innerGroupNode, entityNode, brushNode, patchNode}));
    CHECK_THAT(
      collectNodesAndDescendants(
        {innerGroupNode, outerGroupNode}, [](const GroupNode*) { return true; }),
      UnorderedEquals(std::vector<Node*>{outerGroupNode, innerGroupNode}));
  }
}

TEST_CASE("collectBrushFaces")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  auto worldNode = WorldNode{{}, {}, mapFormat};
  auto* brushNode = new BrushNode{
    BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

  worldNode.defaultLayer()->addChild(brushNode);

  CHECK_THAT(collectBrushFaces({&worldNode}), UnorderedEquals(toHandles(brushNode)));
  CHECK_THAT(
    collectBrushFaces({brushNode, brushNode}), UnorderedEquals(toHandles(brushNode)));
}

} // namespace tb::mdl
