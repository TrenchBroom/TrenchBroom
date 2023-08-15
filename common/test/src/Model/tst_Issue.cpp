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

#include "Error.h"
#include "Model/BezierPatch.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/Issue.h"
#include "Model/MapFormat.h"
#include "Model/PatchNode.h"

#include <kdl/result.h>
#include <kdl/result_io.h>

#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>

#include <vector>

#include "Catch2.h"

namespace TrenchBroom
{
namespace Model
{
class TestIssue : public Issue
{
public:
  TestIssue(Node& node)
    : Issue{0, node, ""}
  {
  }
};

TEST_CASE("Issue.addSelectableNodes")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto outerGroupNode = GroupNode{Group{"outer"}};

  auto* innerGroupNode = new GroupNode{Group{"inner"}};
  auto* pointEntityNode = new EntityNode{Entity{}};
  auto* brushNode = new BrushNode{
    BrushBuilder{MapFormat::Quake3, worldBounds}.createCube(64.0, "texture").value()};

  auto* brushEntityNode = new EntityNode{Entity{}};
  auto* entityBrushNode = new BrushNode{
    BrushBuilder{MapFormat::Quake3, worldBounds}.createCube(64.0, "texture").value()};
  brushEntityNode->addChild(entityBrushNode);

  // clang-format off
  auto* patchNode = new PatchNode{BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  outerGroupNode.addChildren(
    {innerGroupNode, pointEntityNode, brushNode, brushEntityNode, patchNode});

  const auto getSelectableNodes = [](const auto& issue) {
    auto nodes = std::vector<Node*>{};
    issue.addSelectableNodes(nodes);
    return nodes;
  };

  const auto hasSelectableNodes = [](const auto& issue) {
    auto nodes = std::vector<Node*>{};
    return issue.addSelectableNodes(nodes);
  };

  CHECK_FALSE(hasSelectableNodes(TestIssue{outerGroupNode}));
  CHECK_THAT(
    getSelectableNodes(TestIssue{outerGroupNode}),
    Catch::Matchers::UnorderedEquals(std::vector<Node*>{}));

  CHECK(hasSelectableNodes(TestIssue{*innerGroupNode}));
  CHECK_THAT(
    getSelectableNodes(TestIssue{*innerGroupNode}),
    Catch::Matchers::UnorderedEquals(std::vector<Node*>{innerGroupNode}));

  CHECK(hasSelectableNodes(TestIssue{*pointEntityNode}));
  CHECK_THAT(
    getSelectableNodes(TestIssue{*pointEntityNode}),
    Catch::Matchers::UnorderedEquals(std::vector<Node*>{pointEntityNode}));

  CHECK(hasSelectableNodes(TestIssue{*brushNode}));
  CHECK_THAT(
    getSelectableNodes(TestIssue{*brushNode}),
    Catch::Matchers::UnorderedEquals(std::vector<Node*>{brushNode}));

  CHECK(hasSelectableNodes(TestIssue{*brushEntityNode}));
  CHECK_THAT(
    getSelectableNodes(TestIssue{*brushEntityNode}),
    Catch::Matchers::UnorderedEquals(std::vector<Node*>{entityBrushNode}));

  CHECK(hasSelectableNodes(TestIssue{*entityBrushNode}));
  CHECK_THAT(
    getSelectableNodes(TestIssue{*entityBrushNode}),
    Catch::Matchers::UnorderedEquals(std::vector<Node*>{entityBrushNode}));

  CHECK(hasSelectableNodes(TestIssue{*patchNode}));
  CHECK_THAT(
    getSelectableNodes(TestIssue{*patchNode}),
    Catch::Matchers::UnorderedEquals(std::vector<Node*>{patchNode}));
}
} // namespace Model
} // namespace TrenchBroom
