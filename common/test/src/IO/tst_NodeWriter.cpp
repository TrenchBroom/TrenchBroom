/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Exceptions.h"
#include "IO/NodeWriter.h"
#include "Model/BezierPatch.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/LockState.h"
#include "Model/MapFormat.h"
#include "Model/PatchNode.h"
#include "Model/VisibilityState.h"
#include "Model/WorldNode.h"
#include "TestUtils.h"

#include "kdl/result.h"
#include "kdl/string_compare.h"

#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/mat_io.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include <fmt/format.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "CatchUtils/Matchers.h"

#include "Catch2.h"

namespace TrenchBroom::IO
{

TEST_CASE("NodeWriterTest.writeEmptyMap")
{
  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

  const auto actual = str.str();
  const auto expected =
    R"(// entity 0
{
"classname" "worldspawn"
}
)";
  CHECK(actual == expected);
}

TEST_CASE("NodeWriterTest.writeWorldspawn")
{
  auto map = Model::WorldNode{{}, {{"message", "holy damn"}}, Model::MapFormat::Standard};

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.writeDefaultLayerProperties")
{
  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};
  map.defaultLayer()->setVisibilityState(Model::VisibilityState::Hidden);
  map.defaultLayer()->setLockState(Model::LockState::Locked);

  auto layer = map.defaultLayer()->layer();
  layer.setColor(Color(0.25f, 0.75f, 1.0f));
  layer.setOmitFromExport(true);
  map.defaultLayer()->setLayer(std::move(layer));

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.writeDaikatanaMap")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Daikatana};

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto brush1 = builder.createCube(64.0, "none").value();
  for (auto& face : brush1.faces())
  {
    auto attributes = face.attributes();
    attributes.setColor(Color{1.0f, 2.0f, 3.0f});
    face.setAttributes(attributes);
  }
  auto* brushNode1 = new Model::BrushNode{std::move(brush1)};
  map.defaultLayer()->addChild(brushNode1);

  auto* brushNode2 = new Model::BrushNode{builder.createCube(64.0, "none").value()};
  map.defaultLayer()->addChild(brushNode2);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

  const auto actual = str.str();
  const auto expected =
    R"(// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1 0 0 0 1 2 3
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1 0 0 0 1 2 3
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1 0 0 0 1 2 3
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1 0 0 0 1 2 3
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1 0 0 0 1 2 3
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1 0 0 0 1 2 3
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

TEST_CASE("NodeWriterTest.writeQuake2ValveMap")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Quake2_Valve};

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto brush1 = builder.createCube(64.0, "e1u1/alarm0").value();

  // set +Z face to e1u1/brwater with contents 0, flags 0, value 0
  {
    auto index = brush1.findFace(vm::vec3::pos_z());
    REQUIRE(index);

    auto& face = brush1.face(*index);
    auto attribs = face.attributes();
    attribs.setTextureName("e1u1/brwater");
    attribs.setSurfaceContents(0);
    attribs.setSurfaceFlags(0);
    attribs.setSurfaceValue(0.0f);
    face.setAttributes(attribs);
  }
  // set -Z face to e1u1/brlava with contents 8, flags 9, value 700
  {
    auto index = brush1.findFace(vm::vec3::neg_z());
    REQUIRE(index);

    auto& face = brush1.face(*index);
    auto attribs = face.attributes();
    attribs.setTextureName("e1u1/brlava");
    attribs.setSurfaceContents(8);
    attribs.setSurfaceFlags(9);
    attribs.setSurfaceValue(700.0f);
    face.setAttributes(attribs);
  }
  // other faces are e1u1/alarm0 with unset contents/flags/value

  auto* brushNode1 = new Model::BrushNode{std::move(brush1)};
  map.defaultLayer()->addChild(brushNode1);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.writeQuake3ValveMap")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Quake3_Valve};

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto* brushNode1 = new Model::BrushNode{builder.createCube(64.0, "none").value()};
  map.defaultLayer()->addChild(brushNode1);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.writeWorldspawnWithBrushInDefaultLayer")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto* brushNode = new Model::BrushNode{builder.createCube(64.0, "none").value()};
  map.defaultLayer()->addChild(brushNode);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.writeWorldspawnWithBrushInCustomLayer")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  auto layer = Model::Layer{"Custom Layer"};
  REQUIRE(layer.sortIndex() == Model::Layer::invalidSortIndex());
  layer.setSortIndex(0);

  auto* layerNode = new Model::LayerNode{std::move(layer)};
  map.addChild(layerNode);

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto* brushNode = new Model::BrushNode{builder.createCube(64.0, "none").value()};
  layerNode->addChild(brushNode);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.writeWorldspawnWithCustomLayerWithSortIndex")
{
  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  auto layer = Model::Layer{"Custom Layer"};
  layer.setSortIndex(1);
  layer.setOmitFromExport(true);

  auto* layerNode = new Model::LayerNode{std::move(layer)};
  layerNode->setLockState(Model::LockState::Locked);
  layerNode->setVisibilityState(Model::VisibilityState::Hidden);

  map.addChild(layerNode);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.writeMapWithGroupInDefaultLayer")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  auto* groupNode = new Model::GroupNode{Model::Group{"Group"}};
  Model::setLinkId(*groupNode, "group_link_id");
  map.defaultLayer()->addChild(groupNode);

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto* brushNode = new Model::BrushNode{builder.createCube(64.0, "none").value()};
  groupNode->addChild(brushNode);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.writeMapWithGroupInCustomLayer")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  auto* layerNode = new Model::LayerNode{Model::Layer{"Custom Layer"}};
  map.addChild(layerNode);

  auto* groupNode = new Model::GroupNode{Model::Group{"Group"}};
  Model::setLinkId(*groupNode, "group_link_id");
  layerNode->addChild(groupNode);

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto* brushNode = new Model::BrushNode{builder.createCube(64.0, "none").value()};
  groupNode->addChild(brushNode);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.writeMapWithNestedGroupInCustomLayer")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  auto* layerNode = new Model::LayerNode{Model::Layer{"Custom Layer"}};
  map.addChild(layerNode);

  auto* outerGroupNode = new Model::GroupNode{Model::Group{"Outer Group"}};
  Model::setLinkId(*outerGroupNode, "outer_group_link_id");
  layerNode->addChild(outerGroupNode);

  auto* innerGroupNode = new Model::GroupNode{Model::Group{"Inner Group"}};
  Model::setLinkId(*innerGroupNode, "inner_group_link_id");
  outerGroupNode->addChild(innerGroupNode);

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto* brushNode = new Model::BrushNode{builder.createCube(64.0, "none").value()};
  innerGroupNode->addChild(brushNode);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.ensureLayerAndGroupPersistentIDs")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  auto* layerNode1 = new Model::LayerNode{Model::Layer{"Custom Layer 1"}};
  layerNode1->setPersistentId(1u);
  map.addChild(layerNode1);

  auto* outerGroupNode = new Model::GroupNode{Model::Group{"Outer Group"}};
  outerGroupNode->setPersistentId(21u);
  Model::setLinkId(*outerGroupNode, "outer_group_link_id");
  layerNode1->addChild(outerGroupNode);

  auto* innerGroupNode = new Model::GroupNode{Model::Group{"Inner Group"}};
  innerGroupNode->setPersistentId(7u);
  Model::setLinkId(*innerGroupNode, "inner_group_link_id");
  outerGroupNode->addChild(innerGroupNode);

  auto* layerNode2 = new Model::LayerNode{Model::Layer{"Custom Layer 2"}};
  layerNode2->setPersistentId(12u);
  map.addChild(layerNode2);

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto* brushNode = new Model::BrushNode{builder.createCube(64.0, "none").value()};
  innerGroupNode->addChild(brushNode);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.exportMapWithOmittedLayers")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};
  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};

  // default layer (omit from export)
  auto defaultLayer = map.defaultLayer()->layer();
  defaultLayer.setOmitFromExport(true);
  map.defaultLayer()->setLayer(std::move(defaultLayer));

  auto* defaultLayerPointEntityNode =
    new Model::EntityNode{Model::Entity{{}, {{"classname", "defaultLayerPointEntity"}}}};

  auto* defaultLayerBrushNode =
    new Model::BrushNode{builder.createCube(64.0, "defaultTexture").value()};
  map.defaultLayer()->addChild(defaultLayerPointEntityNode);
  map.defaultLayer()->addChild(defaultLayerBrushNode);

  // layer1 (omit from export)
  auto layer1 = Model::Layer{"Custom Layer 1"};
  layer1.setOmitFromExport(true);

  auto* layerNode1 = new Model::LayerNode{std::move(layer1)};
  layerNode1->setLayer(std::move(layer1));

  map.addChild(layerNode1);

  auto* layer1PointEntityNode =
    new Model::EntityNode{Model::Entity{{}, {{"classname", "layer1PointEntity"}}}};
  layerNode1->addChild(layer1PointEntityNode);

  auto* layer1BrushNode =
    new Model::BrushNode{builder.createCube(64.0, "layer1Texture").value()};
  layerNode1->addChild(layer1BrushNode);

  // layer2
  auto* layerNode2 = new Model::LayerNode{Model::Layer{"Custom Layer 2"}};
  map.addChild(layerNode2);

  auto* layer2PointEntityNode =
    new Model::EntityNode{Model::Entity{{}, {{"classname", "layer2PointEntity"}}}};
  layerNode2->addChild(layer2PointEntityNode);

  auto* layer2BrushNode =
    new Model::BrushNode{builder.createCube(64.0, "layer2Texture").value()};
  layerNode2->addChild(layer2BrushNode);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.setExporting(true);
  writer.writeMap();

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
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) layer2Texture 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) layer2Texture 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) layer2Texture 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) layer2Texture 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) layer2Texture 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) layer2Texture 0 0 0 1 1
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

TEST_CASE("NodeWriterTest.writeMapWithInheritedLock")
{
  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  auto* layerNode = new Model::LayerNode{Model::Layer{"Custom Layer"}};
  map.addChild(layerNode);

  // WorldNode's lock state is not persisted.
  // TB uses it e.g. for locking everything when opening a group.
  // So this should result in both the default layer and custom layer being written
  // unlocked.

  map.setLockState(Model::LockState::Locked);
  map.defaultLayer()->setLockState(Model::LockState::Inherited);
  layerNode->setLockState(Model::LockState::Inherited);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.writeNodesWithNestedGroup")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};

  auto* worldBrushNode = new Model::BrushNode{builder.createCube(64.0, "some").value()};
  auto* outerGroupNode = new Model::GroupNode{Model::Group{"Outer Group"}};
  auto* innerGroupNode = new Model::GroupNode{Model::Group{"Inner Group"}};
  auto* innerBrushNode = new Model::BrushNode{builder.createCube(64.0, "none").value()};

  Model::setLinkId(*outerGroupNode, "outer_group_link_id");
  Model::setLinkId(*innerGroupNode, "inner_group_link_id");

  innerGroupNode->addChild(innerBrushNode);
  outerGroupNode->addChild(innerGroupNode);
  map.defaultLayer()->addChild(worldBrushNode);
  map.defaultLayer()->addChild(outerGroupNode);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeNodes({innerGroupNode, worldBrushNode});

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

TEST_CASE("NodeWriterTest.writeMapWithLinkedGroups")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto worldNode = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  auto* groupNode = new Model::GroupNode{Model::Group{"Group"}};
  Model::setLinkId(*groupNode, "group_link_id");
  worldNode.defaultLayer()->addChild(groupNode);

  SECTION("Group node with identity transformation does not write transformation")
  {
    auto str = std::stringstream{};
    auto writer = NodeWriter{worldNode, str};
    writer.writeMap();

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
    Model::transformNode(
      *groupNode, vm::translation_matrix(vm::vec3{32, 0, 0}), worldBounds);

    auto str = std::stringstream{};
    auto writer = NodeWriter{worldNode, str};
    writer.writeMap();

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

TEST_CASE("NodeWriterTest.writeNodesWithLinkedGroup")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto worldNode = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  auto* groupNode = new Model::GroupNode{Model::Group{"Group"}};
  Model::setLinkId(*groupNode, "asdf");
  Model::transformNode(
    *groupNode, vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)), worldBounds);
  worldNode.defaultLayer()->addChild(groupNode);

  auto* groupNodeClone = static_cast<Model::GroupNode*>(
    groupNode->cloneRecursively(worldBounds, Model::SetLinkId::keep));
  Model::transformNode(
    *groupNodeClone, vm::translation_matrix(vm::vec3(0.0, 16.0, 0.0)), worldBounds);

  worldNode.defaultLayer()->addChild(groupNodeClone);
  REQUIRE(groupNodeClone->linkId() == groupNode->linkId());

  auto str = std::stringstream{};
  auto writer = NodeWriter{worldNode, str};
  writer.writeNodes({groupNode});

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

TEST_CASE("NodeWriterTest.writeProtectedEntityProperties")
{
  auto worldNode = Model::WorldNode{{}, {}, Model::MapFormat::Standard};

  SECTION("No protected properties")
  {
    auto entity = Model::Entity{};
    entity.setProtectedProperties({});
    auto* entityNode = new Model::EntityNode{std::move(entity)};
    worldNode.defaultLayer()->addChild(entityNode);

    auto str = std::stringstream{};
    auto writer = NodeWriter{worldNode, str};
    writer.writeNodes({entityNode});

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
    auto entity = Model::Entity{};
    entity.setProtectedProperties({"asdf", "some", "with;semicolon"});
    auto* entityNode = new Model::EntityNode{std::move(entity)};
    worldNode.defaultLayer()->addChild(entityNode);

    auto str = std::stringstream{};
    auto writer = NodeWriter{worldNode, str};
    writer.writeNodes({entityNode});

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

TEST_CASE("NodeWriterTest.writeFaces")
{
  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};
  auto builder = Model::BrushBuilder{map.mapFormat(), worldBounds};
  auto* brushNode = new Model::BrushNode{builder.createCube(64.0, "none").value()};

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeBrushFaces(brushNode->brush().faces());

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

TEST_CASE("NodeWriterTest.writePropertiesWithQuotationMarks")
{
  Model::WorldNode map(
    {}, {{"message", "\"holy damn\", he said"}}, Model::MapFormat::Standard);

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

TEST_CASE("NodeWriterTest.writePropertiesWithEscapedQuotationMarks")
{
  auto map = Model::WorldNode{
    {}, {{"message", R"(\"holy damn\", he said)"}}, Model::MapFormat::Standard};

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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
TEST_CASE("NodeWriterTest.writePropertiesWithNewlineEscapeSequence")
{
  auto map = Model::WorldNode{
    {}, {{"message", "holy damn\\nhe said"}}, Model::MapFormat::Standard};

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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
TEST_CASE("NodeWriterTest.writePropertiesWithTrailingBackslash")
{
  auto map = Model::WorldNode{
    {},
    {
      {R"(message\)", R"(holy damn\)"},
      {R"(message2)", R"(holy damn\\)"},
      {R"(message3)", R"(holy damn\\\)"},
    },
    Model::MapFormat::Standard};

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

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

} // namespace TrenchBroom::IO
