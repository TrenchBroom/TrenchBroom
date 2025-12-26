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

#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/NodeIndex.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("NodeIndex")
{
  auto i = NodeIndex{};

  SECTION("Indexing nodes")
  {
    SECTION("WorldNode")
    {
      auto worldNode = WorldNode{
        {},
        {
          {"some_key", "a_value"},
        },
        MapFormat::Quake3};

      i.addNode(worldNode);

      CHECK_THAT(
        i.findNodes("some_key"), UnorderedEquals(std::vector<Node*>{&worldNode}));
      CHECK_THAT(i.findNodes("a_value"), UnorderedEquals(std::vector<Node*>{&worldNode}));

      i.removeNode(worldNode);

      CHECK_THAT(i.findNodes("some_key"), UnorderedEquals(std::vector<Node*>{}));
      CHECK_THAT(i.findNodes("a_value"), UnorderedEquals(std::vector<Node*>{}));
    }

    SECTION("LayerNode")
    {
      auto layerNode = LayerNode{Layer{"layer_name"}};

      i.addNode(layerNode);
      CHECK_THAT(i.findNodes("layer_name"), UnorderedEquals(std::vector<Node*>{}));

      i.removeNode(layerNode);
      CHECK_THAT(i.findNodes("layer_name"), UnorderedEquals(std::vector<Node*>{}));
    }

    SECTION("GroupNode")
    {
      auto layerNode = GroupNode{Group{"group_name"}};

      i.addNode(layerNode);
      CHECK_THAT(
        i.findNodes("group_name"), UnorderedEquals(std::vector<Node*>{&layerNode}));

      i.removeNode(layerNode);
      CHECK_THAT(i.findNodes("group_name"), UnorderedEquals(std::vector<Node*>{}));
    }

    SECTION("EntityNode")
    {
      auto entityNode = EntityNode{Entity{{
        {"some_key", "a_value"},
      }}};

      i.addNode(entityNode);

      CHECK_THAT(
        i.findNodes("some_key"), UnorderedEquals(std::vector<Node*>{&entityNode}));
      CHECK_THAT(
        i.findNodes("a_value"), UnorderedEquals(std::vector<Node*>{&entityNode}));
    }

    SECTION("BrushNode")
    {
      const auto builder =
        BrushBuilder{MapFormat::Valve, vm::bbox3d{8192.0}, BrushFaceAttributes{""}};

      auto brush = builder.createCube(32.0, "default_material").value();
      brush.face(0).setAttributes(BrushFaceAttributes{"material_0"});
      brush.face(2).setAttributes(BrushFaceAttributes{"material_1"});
      brush.face(3).setAttributes(BrushFaceAttributes{"material_2"});

      auto brushNode = BrushNode{std::move(brush)};

      i.addNode(brushNode);
      CHECK_THAT(
        i.findNodes("material_0"), UnorderedEquals(std::vector<Node*>{&brushNode}));
      CHECK_THAT(
        i.findNodes("material_1"), UnorderedEquals(std::vector<Node*>{&brushNode}));

      i.removeNode(brushNode);
      CHECK_THAT(i.findNodes("material_0"), UnorderedEquals(std::vector<Node*>{}));
      CHECK_THAT(i.findNodes("material_1"), UnorderedEquals(std::vector<Node*>{}));
    }

    SECTION("PatchNode")
    {
      // clang-format off
    auto patchNode = PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "some_material"}};
      // clang-format on

      i.addNode(patchNode);
      CHECK_THAT(
        i.findNodes("some_material"), UnorderedEquals(std::vector<Node*>{&patchNode}));

      i.removeNode(patchNode);
      CHECK_THAT(i.findNodes("some_material"), UnorderedEquals(std::vector<Node*>{}));
    }

    SECTION("mixed nodes")
    {
      auto entityNode1 = EntityNode{Entity{{
        {"some_key", "a_value"},
        {"some_other_key", "another_value"},
      }}};
      auto entityNode2 = EntityNode{Entity{{
        {"some_yet_other_key", "yet_another_value"},
      }}};
      auto groupNode = GroupNode{Group{"some_group"}};

      i.addNode(entityNode1);
      i.addNode(entityNode2);
      i.addNode(groupNode);

      CHECK_THAT(i.findNodes("asdf"), UnorderedEquals(std::vector<Node*>{}));
      CHECK_THAT(
        i.findNodes<EntityNode>("asdf"), UnorderedEquals(std::vector<EntityNode*>{}));
      CHECK_THAT(
        i.findNodes<GroupNode>("asdf"), UnorderedEquals(std::vector<GroupNode*>{}));
      CHECK_THAT(
        i.findNodes<BrushNode>("asdf"), UnorderedEquals(std::vector<BrushNode*>{}));

      CHECK_THAT(
        i.findNodes("some_key"), UnorderedEquals(std::vector<Node*>{&entityNode1}));
      CHECK_THAT(
        i.findNodes<EntityNode>("some_key"),
        UnorderedEquals(std::vector<EntityNode*>{&entityNode1}));
      CHECK_THAT(
        i.findNodes<GroupNode>("some_key"), UnorderedEquals(std::vector<GroupNode*>{}));
      CHECK_THAT(
        i.findNodes<BrushNode>("some_key"), UnorderedEquals(std::vector<BrushNode*>{}));

      CHECK_THAT(
        i.findNodes("some*"),
        UnorderedEquals(std::vector<Node*>{&entityNode1, &entityNode2, &groupNode}));
      CHECK_THAT(
        i.findNodes<EntityNode>("some*"),
        UnorderedEquals(std::vector<EntityNode*>{&entityNode1, &entityNode2}));
      CHECK_THAT(
        i.findNodes<GroupNode>("some*"),
        UnorderedEquals(std::vector<GroupNode*>{&groupNode}));
      CHECK_THAT(
        i.findNodes<BrushNode>("some*"), UnorderedEquals(std::vector<BrushNode*>{}));
    }
  }

  SECTION("clear")
  {
    auto entityNode = EntityNode{Entity{{
      {"some_key", "a_value"},
    }}};

    i.addNode(entityNode);

    REQUIRE_THAT(
      i.findNodes("some_key"), UnorderedEquals(std::vector<Node*>{&entityNode}));

    i.clear();

    CHECK_THAT(i.findNodes("some_key"), UnorderedEquals(std::vector<Node*>{}));
  }
}

} // namespace tb::mdl
