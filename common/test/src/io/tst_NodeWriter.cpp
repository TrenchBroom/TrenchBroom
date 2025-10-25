/*
 Copyright (C) 2010 Kristian Duske

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
#include "io/NodeWriter.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceAttributes.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/LockState.h"
#include "mdl/MapFormat.h"
#include "mdl/VisibilityState.h"
#include "mdl/WorldNode.h"

#include "kdl/result.h"
#include "kdl/task_manager.h"

#include <fmt/format.h>

#include <sstream>
#include <vector>

#include "catch/CatchConfig.h"
#include "catch/Matchers.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::io
{
TEST_CASE("NodeWriter")
{
  auto taskManager = kdl::task_manager{};

  SECTION("writeEmptyMap")
  {
    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"classname" "worldspawn"
}
)";
    CHECK(actual == expected);
  }

  SECTION("writeWorldspawn")
  {
    auto map = mdl::WorldNode{{}, {{"message", "holy damn"}}, mdl::MapFormat::Standard};

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"message" "holy damn"
"classname" "worldspawn"
}
)";
    CHECK(actual == expected);
  }

  SECTION("writeDefaultLayerProperties")
  {
    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};
    map.defaultLayer()->setVisibilityState(mdl::VisibilityState::Hidden);
    map.defaultLayer()->setLockState(mdl::LockState::Locked);

    auto layer = map.defaultLayer()->layer();
    layer.setColor(Color(0.25f, 0.75f, 1.0f));
    layer.setOmitFromExport(true);
    map.defaultLayer()->setLayer(std::move(layer));

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"classname" "worldspawn"
"_tb_layer_color" "0.25 0.75 1 1"
"_tb_layer_locked" "1"
"_tb_layer_hidden" "1"
"_tb_layer_omit_from_export" "1"
}
)";
    CHECK(actual == expected);
  }

  SECTION("writeDaikatanaMap")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Daikatana};

    auto builder = mdl::BrushBuilder{map.mapFormat(), worldBounds};
    auto brush1 = builder.createCube(64.0, "none") | kdl::value();
    for (auto& face : brush1.faces())
    {
      auto attributes = face.attributes();
      attributes.setColor(Color{1.0f, 0.5f, 0.25f});
      face.setAttributes(attributes);
    }
    auto* brushNode1 = new mdl::BrushNode{std::move(brush1)};
    map.defaultLayer()->addChild(brushNode1);

    auto* brushNode2 =
      new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};
    map.defaultLayer()->addChild(brushNode2);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1 0 0 0 255 127 63
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1 0 0 0 255 127 63
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1 0 0 0 255 127 63
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1 0 0 0 255 127 63
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1 0 0 0 255 127 63
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1 0 0 0 255 127 63
}
// brush 1
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
}
}
)";
    CHECK(actual == expected);
  }

  SECTION("writeQuake2ValveMap")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Quake2_Valve};

    auto builder = mdl::BrushBuilder{map.mapFormat(), worldBounds};
    auto brush1 = builder.createCube(64.0, "e1u1/alarm0") | kdl::value();

    // set +Z face to e1u1/brwater with contents 0, flags 0, value 0
    {
      auto index = brush1.findFace(vm::vec3d{0, 0, 1});
      REQUIRE(index);

      auto& face = brush1.face(*index);
      auto attribs = face.attributes();
      attribs.setMaterialName("e1u1/brwater");
      attribs.setSurfaceContents(0);
      attribs.setSurfaceFlags(0);
      attribs.setSurfaceValue(0.0f);
      face.setAttributes(attribs);
    }
    // set -Z face to e1u1/brlava with contents 8, flags 9, value 700
    {
      auto index = brush1.findFace(vm::vec3d{0, 0, -1});
      REQUIRE(index);

      auto& face = brush1.face(*index);
      auto attribs = face.attributes();
      attribs.setMaterialName("e1u1/brlava");
      attribs.setSurfaceContents(8);
      attribs.setSurfaceFlags(9);
      attribs.setSurfaceValue(700.0f);
      face.setAttributes(attribs);
    }
    // other faces are e1u1/alarm0 with unset contents/flags/value

    auto* brushNode1 = new mdl::BrushNode{std::move(brush1)};
    map.defaultLayer()->addChild(brushNode1);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) e1u1/alarm0 [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) e1u1/alarm0 [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) e1u1/brlava [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1 8 9 700
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) e1u1/brwater [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1 0 0 0
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) e1u1/alarm0 [ -1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) e1u1/alarm0 [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
}
}
)";

    CHECK(actual == expected);
  }

  SECTION("writeQuake3ValveMap")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Quake3_Valve};

    auto builder = mdl::BrushBuilder{map.mapFormat(), worldBounds};
    auto* brushNode1 =
      new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};
    map.defaultLayer()->addChild(brushNode1);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none [ -1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
}
}
)";

    CHECK(actual == expected);
  }

  SECTION("writeWorldspawnWithBrushInDefaultLayer")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    auto builder = mdl::BrushBuilder{map.mapFormat(), worldBounds};
    auto* brushNode = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};
    map.defaultLayer()->addChild(brushNode);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
}
}
)";
    CHECK(actual == expected);
  }

  SECTION("writeWorldspawnWithBrushInCustomLayer")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    auto layer = mdl::Layer{"Custom Layer"};
    REQUIRE(layer.sortIndex() == mdl::Layer::invalidSortIndex());
    layer.setSortIndex(0);

    auto* layerNode = new mdl::LayerNode{std::move(layer)};
    map.addChild(layerNode);

    auto builder = mdl::BrushBuilder{map.mapFormat(), worldBounds};
    auto* brushNode = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};
    layerNode->addChild(brushNode);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected = fmt::format(
      R"(// entity 0
{{
"classname" "worldspawn"
}}
// entity 1
{{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Custom Layer"
"_tb_id" "{}"
"_tb_layer_sort_index" "0"
// brush 0
{{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
}}
}}
)",
      *layerNode->persistentId());
    CHECK(actual == expected);
  }

  SECTION("writeWorldspawnWithCustomLayerWithSortIndex")
  {
    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    auto layer = mdl::Layer{"Custom Layer"};
    layer.setSortIndex(1);
    layer.setOmitFromExport(true);

    auto* layerNode = new mdl::LayerNode{std::move(layer)};
    layerNode->setLockState(mdl::LockState::Locked);
    layerNode->setVisibilityState(mdl::VisibilityState::Hidden);

    map.addChild(layerNode);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected = fmt::format(
      R"(// entity 0
{{
"classname" "worldspawn"
}}
// entity 1
{{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Custom Layer"
"_tb_id" "{}"
"_tb_layer_sort_index" "1"
"_tb_layer_locked" "1"
"_tb_layer_hidden" "1"
"_tb_layer_omit_from_export" "1"
}}
)",
      *layerNode->persistentId());
    CHECK(actual == expected);
  }

  SECTION("writeMapWithGroupInDefaultLayer")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    auto* groupNode = new mdl::GroupNode{mdl::Group{"Group"}};
    mdl::setLinkId(*groupNode, "group_link_id");
    map.defaultLayer()->addChild(groupNode);

    auto builder = mdl::BrushBuilder{map.mapFormat(), worldBounds};
    auto* brushNode = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};
    groupNode->addChild(brushNode);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected = fmt::format(
      R"(// entity 0
{{
"classname" "worldspawn"
}}
// entity 1
{{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group"
"_tb_id" "{}"
"_tb_linked_group_id" "group_link_id"
// brush 0
{{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
}}
}}
)",
      *groupNode->persistentId());
    CHECK(actual == expected);
  }

  SECTION("writeMapWithGroupInCustomLayer")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    auto* layerNode = new mdl::LayerNode{mdl::Layer{"Custom Layer"}};
    map.addChild(layerNode);

    auto* groupNode = new mdl::GroupNode{mdl::Group{"Group"}};
    mdl::setLinkId(*groupNode, "group_link_id");
    layerNode->addChild(groupNode);

    auto builder = mdl::BrushBuilder{map.mapFormat(), worldBounds};
    auto* brushNode = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};
    groupNode->addChild(brushNode);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected = fmt::format(
      R"(// entity 0
{{
"classname" "worldspawn"
}}
// entity 1
{{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Custom Layer"
"_tb_id" "{0}"
}}
// entity 2
{{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group"
"_tb_id" "{1}"
"_tb_linked_group_id" "group_link_id"
"_tb_layer" "{0}"
// brush 0
{{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
}}
}}
)",
      *layerNode->persistentId(),
      *groupNode->persistentId());
    CHECK(actual == expected);
  }

  SECTION("writeMapWithNestedGroupInCustomLayer")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    auto* layerNode = new mdl::LayerNode{mdl::Layer{"Custom Layer"}};
    map.addChild(layerNode);

    auto* outerGroupNode = new mdl::GroupNode{mdl::Group{"Outer Group"}};
    mdl::setLinkId(*outerGroupNode, "outer_group_link_id");
    layerNode->addChild(outerGroupNode);

    auto* innerGroupNode = new mdl::GroupNode{mdl::Group{"Inner Group"}};
    mdl::setLinkId(*innerGroupNode, "inner_group_link_id");
    outerGroupNode->addChild(innerGroupNode);

    auto builder = mdl::BrushBuilder{map.mapFormat(), worldBounds};
    auto* brushNode = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};
    innerGroupNode->addChild(brushNode);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected = fmt::format(
      R"(// entity 0
{{
"classname" "worldspawn"
}}
// entity 1
{{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Custom Layer"
"_tb_id" "{0}"
}}
// entity 2
{{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Outer Group"
"_tb_id" "{1}"
"_tb_linked_group_id" "outer_group_link_id"
"_tb_layer" "{0}"
}}
// entity 3
{{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Inner Group"
"_tb_id" "{2}"
"_tb_linked_group_id" "inner_group_link_id"
"_tb_group" "{1}"
// brush 0
{{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
}}
}}
)",
      *layerNode->persistentId(),
      *outerGroupNode->persistentId(),
      *innerGroupNode->persistentId());
    CHECK(actual == expected);
  }

  SECTION("ensureLayerAndGroupPersistentIDs")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    auto* layerNode1 = new mdl::LayerNode{mdl::Layer{"Custom Layer 1"}};
    layerNode1->setPersistentId(1u);
    map.addChild(layerNode1);

    auto* outerGroupNode = new mdl::GroupNode{mdl::Group{"Outer Group"}};
    outerGroupNode->setPersistentId(21u);
    mdl::setLinkId(*outerGroupNode, "outer_group_link_id");
    layerNode1->addChild(outerGroupNode);

    auto* innerGroupNode = new mdl::GroupNode{mdl::Group{"Inner Group"}};
    innerGroupNode->setPersistentId(7u);
    mdl::setLinkId(*innerGroupNode, "inner_group_link_id");
    outerGroupNode->addChild(innerGroupNode);

    auto* layerNode2 = new mdl::LayerNode{mdl::Layer{"Custom Layer 2"}};
    layerNode2->setPersistentId(12u);
    map.addChild(layerNode2);

    auto builder = mdl::BrushBuilder{map.mapFormat(), worldBounds};
    auto* brushNode = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};
    innerGroupNode->addChild(brushNode);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Custom Layer 1"
"_tb_id" "1"
}
// entity 2
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Outer Group"
"_tb_id" "21"
"_tb_linked_group_id" "outer_group_link_id"
"_tb_layer" "1"
}
// entity 3
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Inner Group"
"_tb_id" "7"
"_tb_linked_group_id" "inner_group_link_id"
"_tb_group" "21"
// brush 0
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
}
}
// entity 4
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Custom Layer 2"
"_tb_id" "12"
}
)";
    CHECK(actual == expected);
  }

  SECTION("exportMapWithOmittedLayers")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};
    auto builder = mdl::BrushBuilder{map.mapFormat(), worldBounds};

    // default layer (omit from export)
    auto defaultLayer = map.defaultLayer()->layer();
    defaultLayer.setOmitFromExport(true);
    map.defaultLayer()->setLayer(std::move(defaultLayer));

    auto* defaultLayerPointEntityNode =
      new mdl::EntityNode{mdl::Entity{{{"classname", "defaultLayerPointEntity"}}}};

    auto* defaultLayerBrushNode =
      new mdl::BrushNode{builder.createCube(64.0, "defaultMaterial") | kdl::value()};
    map.defaultLayer()->addChild(defaultLayerPointEntityNode);
    map.defaultLayer()->addChild(defaultLayerBrushNode);

    // layer1 (omit from export)
    auto layer1 = mdl::Layer{"Custom Layer 1"};
    layer1.setOmitFromExport(true);

    auto* layerNode1 = new mdl::LayerNode{std::move(layer1)};
    map.addChild(layerNode1);

    auto* layer1PointEntityNode =
      new mdl::EntityNode{mdl::Entity{{{"classname", "layer1PointEntity"}}}};
    layerNode1->addChild(layer1PointEntityNode);

    auto* layer1BrushNode =
      new mdl::BrushNode{builder.createCube(64.0, "layer1Material") | kdl::value()};
    layerNode1->addChild(layer1BrushNode);

    // layer2
    auto* layerNode2 = new mdl::LayerNode{mdl::Layer{"Custom Layer 2"}};
    map.addChild(layerNode2);

    auto* layer2PointEntityNode =
      new mdl::EntityNode{mdl::Entity{{{"classname", "layer2PointEntity"}}}};
    layerNode2->addChild(layer2PointEntityNode);

    auto* layer2BrushNode =
      new mdl::BrushNode{builder.createCube(64.0, "layer2Material") | kdl::value()};
    layerNode2->addChild(layer2BrushNode);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.setExporting(true);
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"classname" "worldspawn"
"_tb_layer_omit_from_export" "1"
}
// entity 1
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Custom Layer 2"
"_tb_id" "*"
// brush 0
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) layer2Material 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) layer2Material 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) layer2Material 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) layer2Material 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) layer2Material 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) layer2Material 0 0 0 1 1
}
}
// entity 2
{
"classname" "layer2PointEntity"
"_tb_layer" "*"
}
)";
    CHECK_THAT(actual, MatchesGlob(expected));
  }

  SECTION("writeMapWithInheritedLock")
  {
    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    auto* layerNode = new mdl::LayerNode{mdl::Layer{"Custom Layer"}};
    map.addChild(layerNode);

    // WorldNode's lock state is not persisted.
    // TB uses it e.g. for locking everything when opening a group.
    // So this should result in both the default layer and custom layer being written
    // unlocked.

    map.setLockState(mdl::LockState::Locked);
    map.defaultLayer()->setLockState(mdl::LockState::Inherited);
    layerNode->setLockState(mdl::LockState::Inherited);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "Custom Layer"
"_tb_id" "*"
}
)";
    CHECK_THAT(actual, MatchesGlob(expected));
  }

  SECTION("writeNodesWithNestedGroup")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    auto builder = mdl::BrushBuilder{map.mapFormat(), worldBounds};

    auto* worldBrushNode =
      new mdl::BrushNode{builder.createCube(64.0, "some") | kdl::value()};
    auto* outerGroupNode = new mdl::GroupNode{mdl::Group{"Outer Group"}};
    auto* innerGroupNode = new mdl::GroupNode{mdl::Group{"Inner Group"}};
    auto* innerBrushNode =
      new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};

    mdl::setLinkId(*outerGroupNode, "outer_group_link_id");
    mdl::setLinkId(*innerGroupNode, "inner_group_link_id");

    innerGroupNode->addChild(innerBrushNode);
    outerGroupNode->addChild(innerGroupNode);
    map.defaultLayer()->addChild(worldBrushNode);
    map.defaultLayer()->addChild(outerGroupNode);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeNodes({innerGroupNode, worldBrushNode}, taskManager);

    const auto actual = str.str();
    const auto expected = fmt::format(
      R"(// entity 0
{{
"classname" "worldspawn"
// brush 0
{{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) some 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) some 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) some 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) some 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) some 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) some 0 0 0 1 1
}}
}}
// entity 1
{{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Inner Group"
"_tb_id" "{}"
"_tb_linked_group_id" "inner_group_link_id"
// brush 0
{{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
}}
}}
)",
      *innerGroupNode->persistentId());
    CHECK(actual == expected);
  }

  SECTION("writeMapWithLinkedGroups")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto worldNode = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    auto* groupNode = new mdl::GroupNode{mdl::Group{"Group"}};
    mdl::setLinkId(*groupNode, "group_link_id");
    worldNode.defaultLayer()->addChild(groupNode);

    SECTION("Group node with identity transformation does not write transformation")
    {
      auto str = std::stringstream{};
      auto writer = NodeWriter{worldNode, str};
      writer.writeMap(taskManager);

      const auto actual = str.str();
      const auto expected = fmt::format(
        R"(// entity 0
{{
"classname" "worldspawn"
}}
// entity 1
{{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group"
"_tb_id" "{}"
"_tb_linked_group_id" "group_link_id"
}}
)",
        *groupNode->persistentId());
      CHECK(actual == expected);
    }

    SECTION("Group node with changed transformation writes transformation")
    {
      mdl::transformNode(
        *groupNode, vm::translation_matrix(vm::vec3d{32, 0, 0}), worldBounds);

      auto str = std::stringstream{};
      auto writer = NodeWriter{worldNode, str};
      writer.writeMap(taskManager);

      const auto actual = str.str();
      const auto expected = fmt::format(
        R"(// entity 0
{{
"classname" "worldspawn"
}}
// entity 1
{{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group"
"_tb_id" "{0}"
"_tb_linked_group_id" "group_link_id"
"_tb_transformation" "1 0 0 32 0 1 0 0 0 0 1 0 0 0 0 1"
}}
)",
        *groupNode->persistentId());
      CHECK(actual == expected);
    }
  }

  SECTION("writeNodesWithLinkedGroup")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto worldNode = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    auto* groupNode = new mdl::GroupNode{mdl::Group{"Group"}};
    mdl::setLinkId(*groupNode, "asdf");
    mdl::transformNode(
      *groupNode, vm::translation_matrix(vm::vec3d(32.0, 0.0, 0.0)), worldBounds);
    worldNode.defaultLayer()->addChild(groupNode);

    auto* groupNodeClone =
      static_cast<mdl::GroupNode*>(groupNode->cloneRecursively(worldBounds));
    mdl::transformNode(
      *groupNodeClone, vm::translation_matrix(vm::vec3d(0.0, 16.0, 0.0)), worldBounds);

    worldNode.defaultLayer()->addChild(groupNodeClone);
    REQUIRE(groupNodeClone->linkId() == groupNode->linkId());

    auto str = std::stringstream{};
    auto writer = NodeWriter{worldNode, str};
    writer.writeNodes({groupNode}, taskManager);

    const auto actual = str.str();
    const auto expected = fmt::format(
      R"(// entity 0
{{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group"
"_tb_id" "{0}"
"_tb_linked_group_id" "asdf"
"_tb_transformation" "1 0 0 32 0 1 0 0 0 0 1 0 0 0 0 1"
}}
)",
      *groupNode->persistentId());
    CHECK(actual == expected);
  }

  SECTION("writeProtectedEntityProperties")
  {
    auto worldNode = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};

    SECTION("No protected properties")
    {
      auto entity = mdl::Entity{};
      entity.setProtectedProperties({});
      auto* entityNode = new mdl::EntityNode{std::move(entity)};
      worldNode.defaultLayer()->addChild(entityNode);

      auto str = std::stringstream{};
      auto writer = NodeWriter{worldNode, str};
      writer.writeNodes({entityNode}, taskManager);

      const auto actual = str.str();
      const auto expected =
        R"(// entity 0
{
}
)";
      CHECK(actual == expected);
    }

    SECTION("Some protected properties")
    {
      auto entity = mdl::Entity{};
      entity.setProtectedProperties({"asdf", "some", "with;semicolon"});
      auto* entityNode = new mdl::EntityNode{std::move(entity)};
      worldNode.defaultLayer()->addChild(entityNode);

      auto str = std::stringstream{};
      auto writer = NodeWriter{worldNode, str};
      writer.writeNodes({entityNode}, taskManager);

      const auto actual = str.str();
      const auto expected =
        R"(// entity 0
{
"_tb_protected_properties" "asdf;some;with\;semicolon"
}
)";
      CHECK(actual == expected);
    }
  }

  SECTION("writeFaces")
  {
    const auto worldBounds = vm::bbox3d{8192.0};

    auto map = mdl::WorldNode{{}, {}, mdl::MapFormat::Standard};
    auto builder = mdl::BrushBuilder{map.mapFormat(), worldBounds};
    auto* brushNode = new mdl::BrushNode{builder.createCube(64.0, "none") | kdl::value()};

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeBrushFaces(brushNode->brush().faces(), taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
)";

    CHECK(actual == expected);

    delete brushNode;
  }

  SECTION("writePropertiesWithQuotationMarks")
  {
    mdl::WorldNode map(
      {}, {{"message", "\"holy damn\", he said"}}, mdl::MapFormat::Standard);

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"message" "\"holy damn\", he said"
"classname" "worldspawn"
}
)";

    CHECK(actual == expected);
  }

  SECTION("writePropertiesWithEscapedQuotationMarks")
  {
    auto map = mdl::WorldNode{
      {}, {{"message", R"(\"holy damn\", he said)"}}, mdl::MapFormat::Standard};

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"message" "\"holy damn\", he said"
"classname" "worldspawn"
}
)";

    CHECK(actual == expected);
  }

  // https://github.com/TrenchBroom/TrenchBroom/issues/1739
  SECTION("writePropertiesWithNewlineEscapeSequence")
  {
    auto map =
      mdl::WorldNode{{}, {{"message", "holy damn\\nhe said"}}, mdl::MapFormat::Standard};

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"message" "holy damn\nhe said"
"classname" "worldspawn"
}
)";

    CHECK(actual == expected);
  }

  // https://github.com/TrenchBroom/TrenchBroom/issues/2556
  SECTION("writePropertiesWithTrailingBackslash")
  {
    auto map = mdl::WorldNode{
      {},
      {
        {R"(message\)", R"(holy damn\)"},
        {R"(message2)", R"(holy damn\\)"},
        {R"(message3)", R"(holy damn\\\)"},
      },
      mdl::MapFormat::Standard};

    auto str = std::stringstream{};
    auto writer = NodeWriter{map, str};
    writer.writeMap(taskManager);

    const auto actual = str.str();
    const auto expected =
      R"(// entity 0
{
"message" "holy damn"
"message2" "holy damn\\"
"message3" "holy damn\\"
"classname" "worldspawn"
}
)";

    CHECK(actual == expected);
  }
}

} // namespace tb::io
