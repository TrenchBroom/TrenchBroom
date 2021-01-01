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
#include "Model/BrushNode.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/LockState.h"
#include "Model/MapFormat.h"
#include "Model/WorldNode.h"
#include "Model/VisibilityState.h"

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

#include "Catch2.h"
#include "TestUtils.h"

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("NodeWriterTest.writeEmptyMap", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

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

        TEST_CASE("NodeWriterTest.writeWorldspawn", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity({
                {"message", "holy damn"}
            }), Model::MapFormat::Standard);

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

        TEST_CASE("NodeWriterTest.writeDefaultLayerProperties", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);
            map.defaultLayer()->setVisibilityState(Model::VisibilityState::Visibility_Hidden);
            map.defaultLayer()->setLockState(Model::LockState::Lock_Locked);

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

        TEST_CASE("NodeWriterTest.writeDaikatanaMap", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Daikatana);

            Model::BrushBuilder builder(map.mapFormat(), worldBounds);
            Model::Brush brush1 = builder.createCube(64.0, "none").value();
            for (Model::BrushFace& face : brush1.faces()) {
                Model::BrushFaceAttributes attributes = face.attributes();
                attributes.setColor(Color(1.0f, 2.0f, 3.0f));
                face.setAttributes(attributes);
            }
            Model::BrushNode* brushNode1 = new Model::BrushNode(std::move(brush1));
            map.defaultLayer()->addChild(brushNode1);

            Model::BrushNode* brushNode2 = new Model::BrushNode(builder.createCube(64.0, "none").value());
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

        TEST_CASE("NodeWriterTest.writeQuake2ValveMap", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Quake2_Valve);

            Model::BrushBuilder builder(map.mapFormat(), worldBounds);
            Model::Brush brush1 = builder.createCube(64.0, "none").value();
            for (Model::BrushFace& face : brush1.faces()) {
                Model::BrushFaceAttributes attributes = face.attributes();
                attributes.setSurfaceValue(32.0f);
                face.setAttributes(attributes);
            }
            
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
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1 0 0 32
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1 0 0 32
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1 0 0 32
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1 0 0 32
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none [ -1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1 0 0 32
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1 0 0 32
}
}
)";

            CHECK(actual == expected);
        }

        TEST_CASE("NodeWriterTest.writeQuake3ValveMap", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Quake3_Valve);

            Model::BrushBuilder builder(map.mapFormat(), worldBounds);
            Model::BrushNode* brushNode1 = new Model::BrushNode(builder.createCube(64.0, "none").value());
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
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none [ 0 -1 0 0 ] [ 0 0 -1 0 ] 0 1 1 0 0 0
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1 0 0 0
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none [ -1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1 0 0 0
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1 0 0 0
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none [ -1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1 0 0 0
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1 0 0 0
}
}
)";

            CHECK(actual == expected);
        }

        TEST_CASE("NodeWriterTest.writeWorldspawnWithBrushInDefaultLayer", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::BrushBuilder builder(map.mapFormat(), worldBounds);
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCube(64.0, "none").value());
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

        TEST_CASE("NodeWriterTest.writeWorldspawnWithBrushInCustomLayer", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::Layer layer = Model::Layer("Custom Layer");
            REQUIRE(layer.sortIndex() == Model::Layer::invalidSortIndex());
            layer.setSortIndex(0);

            Model::LayerNode* layerNode = new Model::LayerNode(std::move(layer));
            map.addChild(layerNode);

            Model::BrushBuilder builder(map.mapFormat(), worldBounds);
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCube(64.0, "none").value());
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
)", *layerNode->persistentId());
            CHECK(actual == expected);
        }

        TEST_CASE("NodeWriterTest.writeWorldspawnWithCustomLayerWithSortIndex", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::Layer layer = Model::Layer("Custom Layer");
            layer.setSortIndex(1);
            layer.setOmitFromExport(true);

            Model::LayerNode* layerNode = new Model::LayerNode(std::move(layer));
            layerNode->setLockState(Model::LockState::Lock_Locked);
            layerNode->setVisibilityState(Model::VisibilityState::Visibility_Hidden);

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
)", *layerNode->persistentId());
            CHECK(actual == expected);
        }

        TEST_CASE("NodeWriterTest.writeMapWithGroupInDefaultLayer", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::GroupNode* groupNode = new Model::GroupNode(Model::Group("Group"));
            map.defaultLayer()->addChild(groupNode);

            Model::BrushBuilder builder(map.mapFormat(), worldBounds);
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCube(64.0, "none").value());
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
)", *groupNode->persistentId());
            CHECK(actual == expected);
        }

        TEST_CASE("NodeWriterTest.writeMapWithGroupInCustomLayer", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::LayerNode* layerNode = new Model::LayerNode(Model::Layer("Custom Layer"));
            map.addChild(layerNode);

            Model::GroupNode* groupNode = new Model::GroupNode(Model::Group("Group"));
            layerNode->addChild(groupNode);

            Model::BrushBuilder builder(map.mapFormat(), worldBounds);
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCube(64.0, "none").value());
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
)", *layerNode->persistentId(), *groupNode->persistentId());
            CHECK(actual == expected);
        }

        TEST_CASE("NodeWriterTest.writeMapWithNestedGroupInCustomLayer", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::LayerNode* layerNode = new Model::LayerNode(Model::Layer("Custom Layer"));
            map.addChild(layerNode);

            Model::GroupNode* outerGroupNode = new Model::GroupNode(Model::Group("Outer Group"));
            layerNode->addChild(outerGroupNode);

            Model::GroupNode* innerGroupNode = new Model::GroupNode(Model::Group("Inner Group"));
            outerGroupNode->addChild(innerGroupNode);

            Model::BrushBuilder builder(map.mapFormat(), worldBounds);
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCube(64.0, "none").value());
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
)", *layerNode->persistentId(), *outerGroupNode->persistentId(), *innerGroupNode->persistentId());
            CHECK(actual == expected);
        }

        TEST_CASE("NodeWriterTest.ensureLayerAndGroupPersistentIDs", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

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
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCube(64.0, "none").value());
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

        TEST_CASE("NodeWriterTest.exportMapWithOmittedLayers", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);
            Model::BrushBuilder builder(map.mapFormat(), worldBounds);

            // default layer (omit from export)
            auto defaultLayer = map.defaultLayer()->layer();
            defaultLayer.setOmitFromExport(true);
            map.defaultLayer()->setLayer(std::move(defaultLayer));

            auto* defaultLayerPointEntityNode = new Model::EntityNode(Model::Entity({
                {"classname", "defaultLayerPointEntity"}
            }));

            auto* defaultLayerBrushNode = new Model::BrushNode(builder.createCube(64.0, "defaultTexture").value());
            map.defaultLayer()->addChild(defaultLayerPointEntityNode);
            map.defaultLayer()->addChild(defaultLayerBrushNode);

            // layer1 (omit from export)
            auto layer1 = Model::Layer("Custom Layer 1");
            layer1.setOmitFromExport(true);

            auto* layerNode1 = new Model::LayerNode(std::move(layer1));
            layerNode1->setLayer(std::move(layer1));

            map.addChild(layerNode1);

            auto* layer1PointEntityNode = new Model::EntityNode(Model::Entity({
                {"classname", "layer1PointEntity"}
            }));
            layerNode1->addChild(layer1PointEntityNode);

            auto* layer1BrushNode = new Model::BrushNode(builder.createCube(64.0, "layer1Texture").value());
            layerNode1->addChild(layer1BrushNode);

            // layer2
            auto* layerNode2 = new Model::LayerNode(Model::Layer("Custom Layer 2"));
            map.addChild(layerNode2);

            auto* layer2PointEntityNode = new Model::EntityNode(Model::Entity({
                {"classname", "layer2PointEntity"}
            }));
            layerNode2->addChild(layer2PointEntityNode);

            auto* layer2BrushNode = new Model::BrushNode(builder.createCube(64.0, "layer2Texture").value());
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

        TEST_CASE("NodeWriterTest.writeMapWithInheritedLock", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::LayerNode* layerNode = new Model::LayerNode(Model::Layer("Custom Layer"));
            map.addChild(layerNode);

            // WorldNode's lock state is not persisted.
            // TB uses it e.g. for locking everything when opening a group.
            // So this should result in both the default layer and custom layer being written unlocked.

            map.setLockState(Model::LockState::Lock_Locked);
            map.defaultLayer()->setLockState(Model::LockState::Lock_Inherited);
            layerNode->setLockState(Model::LockState::Lock_Inherited);

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

        TEST_CASE("NodeWriterTest.writeNodesWithNestedGroup", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::BrushBuilder builder(map.mapFormat(), worldBounds);

            Model::BrushNode* worldBrushNode = new Model::BrushNode(builder.createCube(64.0, "some").value());
            Model::GroupNode* outerGroupNode = new Model::GroupNode(Model::Group("Outer Group"));
            Model::GroupNode* innerGroupNode = new Model::GroupNode(Model::Group("Inner Group"));
            Model::BrushNode* innerBrushNode = new Model::BrushNode(builder.createCube(64.0, "none").value());

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
)", *innerGroupNode->persistentId());
            CHECK(actual == expected);
        }

        TEST_CASE("NodeWriterTest.writeFaces", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);
            Model::BrushBuilder builder(map.mapFormat(), worldBounds);
            Model::BrushNode* brushNode = new Model::BrushNode(builder.createCube(64.0, "none").value());

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

        TEST_CASE("NodeWriterTest.writePropertiesWithQuotationMarks", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity({
                {"message", "\"holy damn\", he said"}
            }), Model::MapFormat::Standard);

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

        TEST_CASE("NodeWriterTest.writePropertiesWithEscapedQuotationMarks", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity({
                {"message", "\\\"holy damn\\\", he said"}
            }), Model::MapFormat::Standard);

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
        TEST_CASE("NodeWriterTest.writePropertiesWithNewlineEscapeSequence", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity({
                {"message", "holy damn\\nhe said"}
            }), Model::MapFormat::Standard);

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
        TEST_CASE("NodeWriterTest.writePropertiesWithTrailingBackslash", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity({
                {"message\\", "holy damn\\"},
                {"message2", "holy damn\\\\"},
                {"message3", "holy damn\\\\\\"},
            }), Model::MapFormat::Standard);

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

        TEST_CASE("NodeWriterTest.writeSmallValuesWithoutScientificNotation", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Quake2);

            Model::BrushBuilder builder(map.mapFormat(), worldBounds);
            auto brush = builder.createCube(64.0, "defaultTexture").value();
            REQUIRE(brush.transform(worldBounds, vm::rotation_matrix(vm::to_radians(15.0), vm::to_radians(22.0), vm::to_radians(89.0)), false).is_success());

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
            const std::string expected = \
R"(// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -21.849932013225562 44.73955142106092 24.350626473659066 ) ( -21.833750423753578 45.66659406103575 23.976019880243154 ) ( -21.5848373706685 45.09682147885355 25.24621730450337 ) defaultTexture 1e-05 2e-06 0.003 0.004 0.005 0 0 0.006
( 21.849932013225562 -44.73955142106092 -24.350626473659066 ) ( 21.866113602697553 -43.81250878108611 -24.725233067074978 ) ( 20.885845405783215 -44.62575313692022 -24.110653633785617 ) defaultTexture 0 0 0 1 1 0 0 0
( 21.849932013225562 -44.73955142106092 -24.350626473659066 ) ( 20.885845405783215 -44.62575313692022 -24.110653633785617 ) ( 22.11502665578263 -44.3822813632683 -23.45503564281476 ) defaultTexture 0 0 0 1 1 0 0 0
( -21.849932013225562 44.73955142106092 24.350626473659066 ) ( -21.5848373706685 45.09682147885355 25.24621730450337 ) ( -22.814018620667916 44.85334970520164 24.59059931353252 ) defaultTexture 0 0 0 1 1 0 0 0
( -21.849932013225562 44.73955142106092 24.350626473659066 ) ( -22.814018620667916 44.85334970520164 24.59059931353252 ) ( -21.833750423753578 45.66659406103575 23.976019880243154 ) defaultTexture 0 0 0 1 1 0 0 0
( 21.849932013225562 -44.73955142106092 -24.350626473659066 ) ( 22.11502665578263 -44.3822813632683 -23.45503564281476 ) ( 21.866113602697553 -43.81250878108611 -24.725233067074978 ) defaultTexture 0 0 0 1 1 0 0 0
}
}
)";
            CHECK(actual == expected);
        }
    }
}
