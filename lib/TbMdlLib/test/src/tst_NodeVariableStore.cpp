/*
 Copyright (C) 2026 Kristian Duske

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

#include "el/Value.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/NodeVariableStore.h"
#include "mdl/PatchNode.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h"

#include "kd/result.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("NodeVariableStore")
{
  auto fixture = MapFixture{};
  auto& map = fixture.create();

  SECTION("world, layer, group, entity, brush, patch")
  {
    auto* entityNode = new EntityNode{Entity{{
      {"classname", "func_detail"},
      {"targetname", "door1"},
    }}};
    auto* brushNode = createBrushNode(
      map, "left", [](Brush& brush) { brush.face(0).setMaterialName("right"); });
    auto* patchNode = createPatchNode("patchMaterial");
    auto* layerNode = new LayerNode{Layer{"CustomLayer"}};
    auto* groupNode = new GroupNode{Group{"CustomGroup"}};

    addNodes(
      map, {{&parentForNodes(map), {entityNode, brushNode, patchNode, groupNode}}});
    addNodes(map, {{&map.worldNode(), {layerNode}}});

    const auto& worldNode = map.worldNode();
    auto* defaultLayer = worldNode.defaultLayer();

    SECTION("type")
    {
      CHECK(NodeVariableStore{map, worldNode}.value("type") == el::Value{"world"});
      CHECK(NodeVariableStore{map, *defaultLayer}.value("type") == el::Value{"layer"});
      CHECK(NodeVariableStore{map, *groupNode}.value("type") == el::Value{"group"});
      CHECK(NodeVariableStore{map, *entityNode}.value("type") == el::Value{"entity"});
      CHECK(NodeVariableStore{map, *brushNode}.value("type") == el::Value{"brush"});
      CHECK(NodeVariableStore{map, *patchNode}.value("type") == el::Value{"patch"});
    }

    SECTION("name")
    {
      CHECK(NodeVariableStore{map, worldNode}.value("name") == el::Value::Undefined);
      CHECK(NodeVariableStore{map, *layerNode}.value("name") == el::Value{"CustomLayer"});
      CHECK(NodeVariableStore{map, *groupNode}.value("name") == el::Value{"CustomGroup"});
      CHECK(
        NodeVariableStore{map, *entityNode}.value("name") == el::Value{"func_detail"});
      CHECK(NodeVariableStore{map, *brushNode}.value("name") == el::Value::Undefined);
      CHECK(NodeVariableStore{map, *patchNode}.value("name") == el::Value::Undefined);
    }

    SECTION("classname")
    {
      CHECK(NodeVariableStore{map, worldNode}.value("classname") == el::Value::Undefined);
      CHECK(
        NodeVariableStore{map, *layerNode}.value("classname") == el::Value::Undefined);
      CHECK(
        NodeVariableStore{map, *groupNode}.value("classname") == el::Value::Undefined);
      CHECK(
        NodeVariableStore{map, *entityNode}.value("classname")
        == el::Value{"func_detail"});
      CHECK(
        NodeVariableStore{map, *brushNode}.value("classname") == el::Value::Undefined);
      CHECK(
        NodeVariableStore{map, *patchNode}.value("classname") == el::Value::Undefined);
    }

    SECTION("properties")
    {
      CHECK(
        NodeVariableStore{map, worldNode}.value("properties") == el::Value::Undefined);
      CHECK(
        NodeVariableStore{map, *entityNode}.value("properties")
        == el::Value{el::MapType{
          {"classname", el::Value{"func_detail"}},
          {"targetname", el::Value{"door1"}},
        }});
      CHECK(
        NodeVariableStore{map, *brushNode}.value("properties") == el::Value::Undefined);
      CHECK(
        NodeVariableStore{map, *patchNode}.value("properties") == el::Value::Undefined);
    }

    SECTION("entity")
    {
      CHECK(NodeVariableStore{map, worldNode}.value("entity") == el::Value::Undefined);
      CHECK(NodeVariableStore{map, *layerNode}.value("entity") == el::Value::Undefined);
      CHECK(NodeVariableStore{map, *groupNode}.value("entity") == el::Value::Undefined);
      CHECK(
        NodeVariableStore{map, *entityNode}.value("entity")
        == el::Value{el::MapType{
          {"classname", el::Value{"func_detail"}},
          {"properties",
           el::Value{el::MapType{
             {"classname", el::Value{"func_detail"}},
             {"targetname", el::Value{"door1"}},
           }}},
        }});

      // structural brush/patch: owning entity is the world node, i.e. worldspawn
      CHECK(
        NodeVariableStore{map, *brushNode}.value("entity")
        == el::Value{el::MapType{
          {"classname", el::Value{EntityPropertyValues::WorldspawnClassname}},
          {"properties",
           el::Value{el::MapType{
             {"classname", el::Value{EntityPropertyValues::WorldspawnClassname}},
           }}},
        }});
      CHECK(
        NodeVariableStore{map, *patchNode}.value("entity")
        == el::Value{el::MapType{
          {"classname", el::Value{EntityPropertyValues::WorldspawnClassname}},
          {"properties",
           el::Value{el::MapType{
             {"classname", el::Value{EntityPropertyValues::WorldspawnClassname}},
           }}},
        }});
    }

    SECTION("materials")
    {
      CHECK(NodeVariableStore{map, worldNode}.value("materials") == el::Value::Undefined);
      CHECK(
        NodeVariableStore{map, *entityNode}.value("materials") == el::Value::Undefined);
      CHECK(
        NodeVariableStore{map, *brushNode}.value("materials")
        == el::Value{el::ArrayType{el::Value{"left"}, el::Value{"right"}}});
      CHECK(
        NodeVariableStore{map, *patchNode}.value("materials")
        == el::Value{el::ArrayType{el::Value{"patchMaterial"}}});
    }

    SECTION("bounds and center")
    {
      const auto& bounds = brushNode->logicalBounds();
      CHECK(NodeVariableStore{map, *brushNode}.value("bounds") == el::Value{bounds});
      CHECK(
        NodeVariableStore{map, *brushNode}.value("center") == el::Value{bounds.center()});
    }

    SECTION("layerName and groupName")
    {
      // added directly under the default layer, not inside any group
      CHECK(
        NodeVariableStore{map, *brushNode}.value("layerName")
        == el::Value{"Default Layer"});
      CHECK(
        NodeVariableStore{map, *brushNode}.value("groupName") == el::Value::Undefined);

      // world and (non-default) layers are never themselves "in" a layer or group
      CHECK(NodeVariableStore{map, worldNode}.value("layerName") == el::Value::Undefined);
      CHECK(NodeVariableStore{map, worldNode}.value("groupName") == el::Value::Undefined);
      CHECK(
        NodeVariableStore{map, *layerNode}.value("layerName") == el::Value::Undefined);
      CHECK(
        NodeVariableStore{map, *layerNode}.value("groupName") == el::Value::Undefined);

      auto* groupedBrushNode = createBrushNode(map);
      addNodes(map, {{groupNode, {groupedBrushNode}}});

      CHECK(
        NodeVariableStore{map, *groupedBrushNode}.value("layerName")
        == el::Value{"Default Layer"});
      CHECK(
        NodeVariableStore{map, *groupedBrushNode}.value("groupName")
        == el::Value{"CustomGroup"});
    }

    SECTION("linked")
    {
      CHECK(NodeVariableStore{map, worldNode}.value("linked") == el::Value{false});
      CHECK(NodeVariableStore{map, *layerNode}.value("linked") == el::Value{false});
      CHECK(NodeVariableStore{map, *groupNode}.value("linked") == el::Value{false});

      auto* otherGroupNode = new GroupNode{Group{"CustomGroup"}};
      otherGroupNode->setLinkId(groupNode->linkId());
      addNodes(map, {{&parentForNodes(map), {otherGroupNode}}});

      CHECK(NodeVariableStore{map, *groupNode}.value("linked") == el::Value{true});
      CHECK(NodeVariableStore{map, *otherGroupNode}.value("linked") == el::Value{true});
    }

    SECTION("tags")
    {
      // the default test fixture registers no smart tags, so every applicable node
      // kind evaluates to an empty array, not Undefined
      CHECK(NodeVariableStore{map, worldNode}.value("tags") == el::Value::Undefined);
      CHECK(NodeVariableStore{map, *layerNode}.value("tags") == el::Value::Undefined);
      CHECK(NodeVariableStore{map, *groupNode}.value("tags") == el::Value::Undefined);
      CHECK(
        NodeVariableStore{map, *entityNode}.value("tags") == el::Value{el::ArrayType{}});
      CHECK(
        NodeVariableStore{map, *brushNode}.value("tags") == el::Value{el::ArrayType{}});
      CHECK(
        NodeVariableStore{map, *patchNode}.value("tags") == el::Value{el::ArrayType{}});
    }

    SECTION("visible, locked, selected")
    {
      auto store = NodeVariableStore{map, *brushNode};

      CHECK(store.value("visible") == el::Value{true});
      CHECK(store.value("locked") == el::Value{false});
      CHECK(store.value("selected") == el::Value{false});

      brushNode->select();
      CHECK(store.value("selected") == el::Value{true});
    }

    SECTION("names and size")
    {
      auto store = NodeVariableStore{map, *brushNode};
      CHECK(store.names().size() == store.size());
      CHECK(store.size() == 15u);
    }
  }
}

} // namespace tb::mdl
