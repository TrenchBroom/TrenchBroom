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

#include <kdl/result.h>
#include <kdl/string_compare.h>

#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <fmt/format.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "CatchUtils/Matchers.h"

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("NodeWriterTest.writeEmptyMap")
{
  Model::WorldNode map({}, {}, Model::MapFormat::Standard);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected =
    R"(// entity 0
{
"classname" "worldspawn"
}
)";
  CHECK(actual == expected);
}

TEST_CASE("NodeWriterTest.writeWorldspawn")
{
  Model::WorldNode map({}, {{"message", "holy damn"}}, Model::MapFormat::Standard);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected =
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
  Model::WorldNode map({}, {}, Model::MapFormat::Standard);
  map.defaultLayer()->setVisibilityState(Model::VisibilityState::Hidden);
  map.defaultLayer()->setLockState(Model::LockState::Locked);

  auto layer = map.defaultLayer()->layer();
  layer.setColor(Color(0.25f, 0.75f, 1.0f));
  layer.setOmitFromExport(true);
  map.defaultLayer()->setLayer(std::move(layer));

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected =
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
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Daikatana);

  Model::BrushBuilder builder(map.mapFormat(), worldBounds);
  Model::Brush brush1 = builder.createCube(64.0, "none").value();
  for (Model::BrushFace& face : brush1.faces())
  {
    Model::BrushFaceAttributes attributes = face.attributes();
    attributes.setColor(Color(1.0f, 2.0f, 3.0f));
    face.setAttributes(attributes);
  }
  Model::BrushNode* brushNode1 = new Model::BrushNode(std::move(brush1));
  map.defaultLayer()->addChild(brushNode1);

  Model::BrushNode* brushNode2 =
    new Model::BrushNode(builder.createCube(64.0, "none").value());
  map.defaultLayer()->addChild(brushNode2);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected =
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
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Quake2_Valve);

  Model::BrushBuilder builder(map.mapFormat(), worldBounds);
  Model::Brush brush1 = builder.createCube(64.0, "e1u1/alarm0").value();

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

  Model::BrushNode* brushNode1 = new Model::BrushNode(std::move(brush1));
  map.defaultLayer()->addChild(brushNode1);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected =
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
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Quake3_Valve);

  Model::BrushBuilder builder(map.mapFormat(), worldBounds);
  Model::BrushNode* brushNode1 =
    new Model::BrushNode(builder.createCube(64.0, "none").value());
  map.defaultLayer()->addChild(brushNode1);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected =
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
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Standard);

  Model::BrushBuilder builder(map.mapFormat(), worldBounds);
  Model::BrushNode* brushNode =
    new Model::BrushNode(builder.createCube(64.0, "none").value());
  map.defaultLayer()->addChild(brushNode);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected =
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
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Standard);

  Model::Layer layer = Model::Layer("Custom Layer");
  REQUIRE(layer.sortIndex() == Model::Layer::invalidSortIndex());
  layer.setSortIndex(0);

  Model::LayerNode* layerNode = new Model::LayerNode(std::move(layer));
  map.addChild(layerNode);

  Model::BrushBuilder builder(map.mapFormat(), worldBounds);
  Model::BrushNode* brushNode =
    new Model::BrushNode(builder.createCube(64.0, "none").value());
  layerNode->addChild(brushNode);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected = fmt::format(
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
  Model::WorldNode map({}, {}, Model::MapFormat::Standard);

  Model::Layer layer = Model::Layer("Custom Layer");
  layer.setSortIndex(1);
  layer.setOmitFromExport(true);

  Model::LayerNode* layerNode = new Model::LayerNode(std::move(layer));
  layerNode->setLockState(Model::LockState::Locked);
  layerNode->setVisibilityState(Model::VisibilityState::Hidden);

  map.addChild(layerNode);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected = fmt::format(
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
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Standard);

  Model::GroupNode* groupNode = new Model::GroupNode(Model::Group("Group"));
  map.defaultLayer()->addChild(groupNode);

  Model::BrushBuilder builder(map.mapFormat(), worldBounds);
  Model::BrushNode* brushNode =
    new Model::BrushNode(builder.createCube(64.0, "none").value());
  groupNode->addChild(brushNode);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected = fmt::format(
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
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Standard);

  Model::LayerNode* layerNode = new Model::LayerNode(Model::Layer("Custom Layer"));
  map.addChild(layerNode);

  Model::GroupNode* groupNode = new Model::GroupNode(Model::Group("Group"));
  layerNode->addChild(groupNode);

  Model::BrushBuilder builder(map.mapFormat(), worldBounds);
  Model::BrushNode* brushNode =
    new Model::BrushNode(builder.createCube(64.0, "none").value());
  groupNode->addChild(brushNode);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected = fmt::format(
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
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Standard);

  Model::LayerNode* layerNode = new Model::LayerNode(Model::Layer("Custom Layer"));
  map.addChild(layerNode);

  Model::GroupNode* outerGroupNode = new Model::GroupNode(Model::Group("Outer Group"));
  layerNode->addChild(outerGroupNode);

  Model::GroupNode* innerGroupNode = new Model::GroupNode(Model::Group("Inner Group"));
  outerGroupNode->addChild(innerGroupNode);

  Model::BrushBuilder builder(map.mapFormat(), worldBounds);
  Model::BrushNode* brushNode =
    new Model::BrushNode(builder.createCube(64.0, "none").value());
  innerGroupNode->addChild(brushNode);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected = fmt::format(
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
"_tb_layer" "{0}"
}}
// entity 3
{{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Inner Group"
"_tb_id" "{2}"
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
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Standard);

  Model::LayerNode* layerNode1 = new Model::LayerNode(Model::Layer("Custom Layer 1"));
  layerNode1->setPersistentId(1u);
  map.addChild(layerNode1);

  Model::GroupNode* outerGroupNode = new Model::GroupNode(Model::Group("Outer Group"));
  outerGroupNode->setPersistentId(21u);
  layerNode1->addChild(outerGroupNode);

  Model::GroupNode* innerGroupNode = new Model::GroupNode(Model::Group("Inner Group"));
  innerGroupNode->setPersistentId(7u);
  outerGroupNode->addChild(innerGroupNode);

  Model::LayerNode* layerNode2 = new Model::LayerNode(Model::Layer("Custom Layer 2"));
  layerNode2->setPersistentId(12u);
  map.addChild(layerNode2);

  Model::BrushBuilder builder(map.mapFormat(), worldBounds);
  Model::BrushNode* brushNode =
    new Model::BrushNode(builder.createCube(64.0, "none").value());
  innerGroupNode->addChild(brushNode);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected =
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
"_tb_layer" "1"
}
// entity 3
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Inner Group"
"_tb_id" "7"
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
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Standard);
  Model::BrushBuilder builder(map.mapFormat(), worldBounds);

  // default layer (omit from export)
  auto defaultLayer = map.defaultLayer()->layer();
  defaultLayer.setOmitFromExport(true);
  map.defaultLayer()->setLayer(std::move(defaultLayer));

  auto* defaultLayerPointEntityNode =
    new Model::EntityNode(Model::Entity({}, {{"classname", "defaultLayerPointEntity"}}));

  auto* defaultLayerBrushNode =
    new Model::BrushNode(builder.createCube(64.0, "defaultTexture").value());
  map.defaultLayer()->addChild(defaultLayerPointEntityNode);
  map.defaultLayer()->addChild(defaultLayerBrushNode);

  // layer1 (omit from export)
  auto layer1 = Model::Layer("Custom Layer 1");
  layer1.setOmitFromExport(true);

  auto* layerNode1 = new Model::LayerNode(std::move(layer1));
  layerNode1->setLayer(std::move(layer1));

  map.addChild(layerNode1);

  auto* layer1PointEntityNode =
    new Model::EntityNode(Model::Entity({}, {{"classname", "layer1PointEntity"}}));
  layerNode1->addChild(layer1PointEntityNode);

  auto* layer1BrushNode =
    new Model::BrushNode(builder.createCube(64.0, "layer1Texture").value());
  layerNode1->addChild(layer1BrushNode);

  // layer2
  auto* layerNode2 = new Model::LayerNode(Model::Layer("Custom Layer 2"));
  map.addChild(layerNode2);

  auto* layer2PointEntityNode =
    new Model::EntityNode(Model::Entity({}, {{"classname", "layer2PointEntity"}}));
  layerNode2->addChild(layer2PointEntityNode);

  auto* layer2BrushNode =
    new Model::BrushNode(builder.createCube(64.0, "layer2Texture").value());
  layerNode2->addChild(layer2BrushNode);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.setExporting(true);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected =
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
  Model::WorldNode map({}, {}, Model::MapFormat::Standard);

  Model::LayerNode* layerNode = new Model::LayerNode(Model::Layer("Custom Layer"));
  map.addChild(layerNode);

  // WorldNode's lock state is not persisted.
  // TB uses it e.g. for locking everything when opening a group.
  // So this should result in both the default layer and custom layer being written
  // unlocked.

  map.setLockState(Model::LockState::Locked);
  map.defaultLayer()->setLockState(Model::LockState::Inherited);
  layerNode->setLockState(Model::LockState::Inherited);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected =
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
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Standard);

  Model::BrushBuilder builder(map.mapFormat(), worldBounds);

  Model::BrushNode* worldBrushNode =
    new Model::BrushNode(builder.createCube(64.0, "some").value());
  Model::GroupNode* outerGroupNode = new Model::GroupNode(Model::Group("Outer Group"));
  Model::GroupNode* innerGroupNode = new Model::GroupNode(Model::Group("Inner Group"));
  Model::BrushNode* innerBrushNode =
    new Model::BrushNode(builder.createCube(64.0, "none").value());

  innerGroupNode->addChild(innerBrushNode);
  outerGroupNode->addChild(innerGroupNode);
  map.defaultLayer()->addChild(worldBrushNode);
  map.defaultLayer()->addChild(outerGroupNode);

  std::vector<Model::Node*> nodes;
  nodes.push_back(innerGroupNode);
  nodes.push_back(worldBrushNode);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeNodes(nodes);

  const std::string actual = str.str();
  const std::string expected = fmt::format(
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
  const vm::bbox3 worldBounds(8192.0);

  auto worldNode = Model::WorldNode({}, {}, Model::MapFormat::Standard);

  auto group = Model::Group("Group");
  group.transform(vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)));
  auto* groupNode = new Model::GroupNode(std::move(group));

  worldNode.defaultLayer()->addChild(groupNode);

  SECTION("Group node without linked group ID does not write ID or transformation")
  {
    std::stringstream str;
    NodeWriter writer(worldNode, str);
    writer.writeMap();

    const std::string actual = str.str();
    const std::string expected = fmt::format(
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
}}
)",
      *groupNode->persistentId());
    CHECK(actual == expected);
  }

  SECTION("Group nodes with linked group ID does write ID and transformation")
  {
    group = groupNode->group();
    group.setLinkedGroupId("asdf");
    groupNode->setGroup(std::move(group));

    auto* groupNodeClone =
      static_cast<Model::GroupNode*>(groupNode->cloneRecursively(worldBounds));

    auto groupClone = groupNodeClone->group();
    groupClone.transform(vm::translation_matrix(vm::vec3(0.0, 16.0, 0.0)));
    groupNodeClone->setGroup(std::move(groupClone));

    worldNode.defaultLayer()->addChild(groupNodeClone);
    REQUIRE(
      groupNodeClone->group().linkedGroupId() == groupNode->group().linkedGroupId());

    std::stringstream str;
    NodeWriter writer(worldNode, str);
    writer.writeMap();

    const std::string actual = str.str();
    const std::string expected = fmt::format(
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
"_tb_linked_group_id" "asdf"
"_tb_transformation" "1 0 0 32 0 1 0 0 0 0 1 0 0 0 0 1"
}}
// entity 2
{{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group"
"_tb_id" "{1}"
"_tb_linked_group_id" "asdf"
"_tb_transformation" "1 0 0 32 0 1 0 16 0 0 1 0 0 0 0 1"
}}
)",
      *groupNode->persistentId(),
      *groupNodeClone->persistentId());
    CHECK(actual == expected);
  }
}

TEST_CASE("NodeWriterTest.writeNodesWithLinkedGroup")
{
  const vm::bbox3 worldBounds(8192.0);

  auto worldNode = Model::WorldNode({}, {}, Model::MapFormat::Standard);

  auto group = Model::Group("Group");
  group.transform(vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)));
  group.setLinkedGroupId("asdf");

  auto* groupNode = new Model::GroupNode(std::move(group));
  worldNode.defaultLayer()->addChild(groupNode);

  auto* groupNodeClone =
    static_cast<Model::GroupNode*>(groupNode->cloneRecursively(worldBounds));
  auto groupClone = groupNodeClone->group();
  groupClone.transform(vm::translation_matrix(vm::vec3(0.0, 16.0, 0.0)));
  groupNodeClone->setGroup(std::move(groupClone));

  worldNode.defaultLayer()->addChild(groupNodeClone);
  REQUIRE(groupNodeClone->group().linkedGroupId() == groupNode->group().linkedGroupId());

  std::stringstream str;
  NodeWriter writer(worldNode, str);
  writer.writeNodes(std::vector<Model::Node*>{groupNode});

  const std::string actual = str.str();
  const std::string expected = fmt::format(
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

    std::stringstream str;
    NodeWriter writer(worldNode, str);
    writer.writeNodes(std::vector<Model::Node*>{entityNode});

    const std::string actual = str.str();
    const std::string expected =
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

    std::stringstream str;
    NodeWriter writer(worldNode, str);
    writer.writeNodes(std::vector<Model::Node*>{entityNode});

    const std::string actual = str.str();
    const std::string expected =
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
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Standard);
  Model::BrushBuilder builder(map.mapFormat(), worldBounds);
  Model::BrushNode* brushNode =
    new Model::BrushNode(builder.createCube(64.0, "none").value());

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeBrushFaces(brushNode->brush().faces());

  const std::string actual = str.str();
  const std::string expected =
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

  std::stringstream str;
  NodeWriter writer(map, str);
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
  Model::WorldNode map(
    {}, {{"message", "\\\"holy damn\\\", he said"}}, Model::MapFormat::Standard);

  std::stringstream str;
  NodeWriter writer(map, str);
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
  Model::WorldNode map(
    {}, {{"message", "holy damn\\nhe said"}}, Model::MapFormat::Standard);

  std::stringstream str;
  NodeWriter writer(map, str);
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
  Model::WorldNode map(
    {},
    {
      {"message\\", "holy damn\\"},
      {"message2", "holy damn\\\\"},
      {"message3", "holy damn\\\\\\"},
    },
    Model::MapFormat::Standard);

  std::stringstream str;
  NodeWriter writer(map, str);
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

TEST_CASE("NodeWriterTest.writeSmallValuesWithoutScientificNotation")
{
  const vm::bbox3 worldBounds(8192.0);

  Model::WorldNode map({}, {}, Model::MapFormat::Quake2);

  Model::BrushBuilder builder(map.mapFormat(), worldBounds);
  auto brush = builder.createCube(64.0, "defaultTexture").value();
  REQUIRE(brush
            .transform(
              worldBounds,
              vm::rotation_matrix(
                vm::to_radians(15.0), vm::to_radians(22.0), vm::to_radians(89.0)),
              false)
            .is_success());

  auto& face = brush.face(0);
  auto faceAttributes = face.attributes();
  faceAttributes.setXOffset(0.00001f);
  faceAttributes.setYOffset(0.000002f);
  faceAttributes.setRotation(0.003f);
  faceAttributes.setXScale(0.004f);
  faceAttributes.setYScale(0.005f);
  faceAttributes.setSurfaceValue(0.006f);
  face.setAttributes(std::move(faceAttributes));

  auto* brushNode = new Model::BrushNode(std::move(brush));
  map.defaultLayer()->addChild(brushNode);

  std::stringstream str;
  NodeWriter writer(map, str);
  writer.writeMap();

  const std::string actual = str.str();
  const std::string expected =
#if defined(__clang_major__) && __clang_major__ >= 15
    R"(// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -21.849932013225562 44.73955142106093 24.350626473659066 ) ( -21.833750423753578 45.66659406103575 23.976019880243154 ) ( -21.5848373706685 45.096821478853556 25.246217304503375 ) defaultTexture 1e-05 2e-06 0.003 0.004 0.005 0 0 0.006
( 21.849932013225562 -44.73955142106093 -24.350626473659066 ) ( 21.866113602697553 -43.81250878108611 -24.725233067074978 ) ( 20.885845405783215 -44.62575313692022 -24.110653633785617 ) defaultTexture 0 0 0 1 1
( 21.849932013225562 -44.73955142106093 -24.350626473659066 ) ( 20.885845405783215 -44.62575313692022 -24.110653633785617 ) ( 22.11502665578263 -44.382281363268305 -23.45503564281476 ) defaultTexture 0 0 0 1 1
( -21.849932013225562 44.73955142106093 24.350626473659066 ) ( -21.5848373706685 45.096821478853556 25.246217304503375 ) ( -22.814018620667916 44.85334970520164 24.59059931353252 ) defaultTexture 0 0 0 1 1
( -21.849932013225562 44.73955142106093 24.350626473659066 ) ( -22.814018620667916 44.85334970520164 24.59059931353252 ) ( -21.833750423753578 45.66659406103575 23.976019880243154 ) defaultTexture 0 0 0 1 1
( 21.849932013225562 -44.73955142106093 -24.350626473659066 ) ( 22.11502665578263 -44.382281363268305 -23.45503564281476 ) ( 21.866113602697553 -43.81250878108611 -24.725233067074978 ) defaultTexture 0 0 0 1 1
}
}
)";
#else
    R"(// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -21.849932013225562 44.73955142106092 24.350626473659066 ) ( -21.833750423753578 45.66659406103575 23.976019880243154 ) ( -21.5848373706685 45.09682147885355 25.24621730450337 ) defaultTexture 1e-05 2e-06 0.003 0.004 0.005 0 0 0.006
( 21.849932013225562 -44.73955142106092 -24.350626473659066 ) ( 21.866113602697553 -43.81250878108611 -24.725233067074978 ) ( 20.885845405783215 -44.62575313692022 -24.110653633785617 ) defaultTexture 0 0 0 1 1
( 21.849932013225562 -44.73955142106092 -24.350626473659066 ) ( 20.885845405783215 -44.62575313692022 -24.110653633785617 ) ( 22.11502665578263 -44.3822813632683 -23.45503564281476 ) defaultTexture 0 0 0 1 1
( -21.849932013225562 44.73955142106092 24.350626473659066 ) ( -21.5848373706685 45.09682147885355 25.24621730450337 ) ( -22.814018620667916 44.85334970520164 24.59059931353252 ) defaultTexture 0 0 0 1 1
( -21.849932013225562 44.73955142106092 24.350626473659066 ) ( -22.814018620667916 44.85334970520164 24.59059931353252 ) ( -21.833750423753578 45.66659406103575 23.976019880243154 ) defaultTexture 0 0 0 1 1
( 21.849932013225562 -44.73955142106092 -24.350626473659066 ) ( 22.11502665578263 -44.3822813632683 -23.45503564281476 ) ( 21.866113602697553 -43.81250878108611 -24.725233067074978 ) defaultTexture 0 0 0 1 1
}
}
)";
#endif

  CHECK(actual == expected);
}

TEST_CASE("NodeWriterTest.quoteTextureNamesIfNecessary")
{
  using FormatInfo = std::tuple<Model::MapFormat, std::string>;

  // clang-format off
  const auto [mapFormat, expectedSerializationTemplate] = GENERATE(values<FormatInfo>({
  {Model::MapFormat::Standard,
R"(// entity 0
{{
"classname" "worldspawn"
// brush 0
{{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) {0} 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) {0} 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) {0} 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) {0} 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) {0} 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) {0} 0 0 0 1 1
}}
}}
)"},
  {Model::MapFormat::Valve,
R"(// entity 0
{{
"classname" "worldspawn"
// brush 0
{{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) {0} [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) {0} [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) {0} [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) {0} [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) {0} [ -1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) {0} [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1
}}
}}
)"},
  }));
  // clang-format on

  using NameInfo = std::tuple<std::string, std::string>;

  // clang-format off
  const auto [textureName, expectedName] = GENERATE(values<NameInfo>({
  { R"(some_name)", R"(some_name)" },
  { R"(some name)", R"("some name")" },
  { R"(some\name)", R"("some\\name")" },
  { R"(some"name)", R"("some\"name")" },
  }));
  // clang-format on

  CAPTURE(textureName);

  const auto worldBounds = vm::bbox3{8192.0};

  auto map = Model::WorldNode{{}, {}, mapFormat};

  auto builder = Model::BrushBuilder(map.mapFormat(), worldBounds);
  auto brush = builder.createCube(64.0, textureName).value();
  map.defaultLayer()->addChild(new Model::BrushNode{std::move(brush)});

  auto str = std::stringstream{};
  auto writer = NodeWriter{map, str};
  writer.writeMap();

  CHECK(str.str() == fmt::format(expectedSerializationTemplate, expectedName));
}

TEST_CASE("NodeWriterTest.writePatch")
{
  auto patch = Model::BezierPatch{
    5,
    3,
    {{-64, -64, 4, 0, 0},
     {-64, 0, 4, 0, -0.25},
     {-64, 64, 4, 0, -0.5},
     {0, -64, 4, 0.2, 0},
     {0, 0, 4, 0.2, -0.25},
     {0, 64, 4, 0.2, -0.5},
     {64, -64, 4, 0.4, 0},
     {64, 0, 4, 0.4, -0.25},
     {64, 64, 4, 0.4, -0.5},
     {128, -64, 4, 0.6, 0},
     {128, 0, 4, 0.6, -0.25},
     {128, 64, 4, 0.6, -0.5},
     {192, -64, 4, 0.8, 0},
     {192, 0, 4, 0.8, -0.25},
     {192, 64, 4, 0.8, -0.5}},
    "common/caulk"};

  auto map = Model::WorldNode{{}, {}, Model::MapFormat::Standard};
  map.defaultLayer()->addChild(new Model::PatchNode{std::move(patch)});

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
patchDef2
{
common/caulk
( 5 3 0 0 0 )
(
( ( -64 -64 4 0 0 ) ( -64 0 4 0 -0.25 ) ( -64 64 4 0 -0.5 ) )
( ( 0 -64 4 0.2 0 ) ( 0 0 4 0.2 -0.25 ) ( 0 64 4 0.2 -0.5 ) )
( ( 64 -64 4 0.4 0 ) ( 64 0 4 0.4 -0.25 ) ( 64 64 4 0.4 -0.5 ) )
( ( 128 -64 4 0.6 0 ) ( 128 0 4 0.6 -0.25 ) ( 128 64 4 0.6 -0.5 ) )
( ( 192 -64 4 0.8 0 ) ( 192 0 4 0.8 -0.25 ) ( 192 64 4 0.8 -0.5 ) )
)
}
}
}
)";
  CHECK(actual == expected);
}

} // namespace IO
} // namespace TrenchBroom
