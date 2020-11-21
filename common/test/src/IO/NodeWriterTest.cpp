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

#include <iostream>
#include <sstream>
#include <vector>

#include "Catch2.h"
#include "TestUtils.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace IO {
        TEST_CASE("NodeWriterTest.writeEmptyMap", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

            const std::string result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"classname\" \"worldspawn\"\n"
                         "}\n", result.c_str());
        }

        TEST_CASE("NodeWriterTest.writeWorldspawn", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity({
                {"message", "holy damn"}
            }), Model::MapFormat::Standard);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

            const std::string result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"message\" \"holy damn\"\n"
                         "\"classname\" \"worldspawn\"\n"
                         "}\n", result.c_str());
        }

        TEST_CASE("NodeWriterTest.writeDefaultLayerAttributes", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);
            map.defaultLayer()->setLayerColor(Color(0.25f, 0.75f, 1.0f));
            map.defaultLayer()->setVisibilityState(Model::VisibilityState::Visibility_Hidden);
            map.defaultLayer()->setLockState(Model::LockState::Lock_Locked);
            map.defaultLayer()->setOmitFromExport(true);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

            const std::string actual = str.str();
            const std::string expected = \
R"(// entity 0
{
"classname" "worldspawn"
"_tb_layer_color" "0.25 0.75 1"
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

            Model::BrushBuilder builder(&map, worldBounds);
            Model::Brush brush1 = builder.createCube(64.0, "none").value();
            for (Model::BrushFace& face : brush1.faces()) {
                Model::BrushFaceAttributes attributes = face.attributes();
                attributes.setColor(Color(1.0f, 2.0f, 3.0f));
                face.setAttributes(attributes);
            }
            Model::BrushNode* brushNode1 = map.createBrush(std::move(brush1));
            map.defaultLayer()->addChild(brushNode1);

            Model::BrushNode* brushNode2 = map.createBrush(builder.createCube(64.0, "none").value());
            map.defaultLayer()->addChild(brushNode2);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

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

            const std::string actual = str.str();
            ASSERT_EQ(actual, expected);
        }

        TEST_CASE("NodeWriterTest.writeQuake2ValveMap", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Quake2_Valve);

            Model::BrushBuilder builder(&map, worldBounds);
            Model::Brush brush1 = builder.createCube(64.0, "none").value();
            for (Model::BrushFace& face : brush1.faces()) {
                Model::BrushFaceAttributes attributes = face.attributes();
                attributes.setSurfaceValue(32.0f);
                face.setAttributes(attributes);
            }
            
            Model::BrushNode* brushNode1 = map.createBrush(std::move(brush1));
            map.defaultLayer()->addChild(brushNode1);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

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

            const std::string actual = str.str();
            ASSERT_EQ(actual, expected);
        }

        TEST_CASE("NodeWriterTest.writeQuake3ValveMap", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Quake3_Valve);

            Model::BrushBuilder builder(&map, worldBounds);
            Model::BrushNode* brush1 = map.createBrush(builder.createCube(64.0, "none").value());
            map.defaultLayer()->addChild(brush1);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

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

            const std::string actual = str.str();
            ASSERT_EQ(actual, expected);
        }

        TEST_CASE("NodeWriterTest.writeWorldspawnWithBrushInDefaultLayer", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::BrushBuilder builder(&map, worldBounds);
            Model::BrushNode* brushNode = map.createBrush(builder.createCube(64.0, "none").value());
            map.defaultLayer()->addChild(brushNode);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

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
            const std::string actual = str.str();
            ASSERT_EQ(expected, actual);
        }

        TEST_CASE("NodeWriterTest.writeWorldspawnWithBrushInCustomLayer", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::LayerNode* layer = map.createLayer("Custom Layer");
            CHECK(layer->sortIndex() == Model::LayerNode::invalidSortIndex());
            layer->setSortIndex(0);
            map.addChild(layer);

            Model::BrushBuilder builder(&map, worldBounds);
            Model::BrushNode* brushNode = map.createBrush(builder.createCube(64.0, "none").value());
            layer->addChild(brushNode);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

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
"_tb_layer_sort_index" "0"
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
            CHECK_THAT(str.str(), MatchesGlob(expected));
        }

        TEST_CASE("NodeWriterTest.writeWorldspawnWithCustomLayerWithSortIndex", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::LayerNode* layer = map.createLayer("Custom Layer");
            layer->setSortIndex(1);
            layer->setLockState(Model::LockState::Lock_Locked);
            layer->setVisibilityState(Model::VisibilityState::Visibility_Hidden);
            layer->setOmitFromExport(true);
            map.addChild(layer);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

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
"_tb_layer_sort_index" "1"
"_tb_layer_locked" "1"
"_tb_layer_hidden" "1"
"_tb_layer_omit_from_export" "1"
}
)";
            CHECK_THAT(str.str(), MatchesGlob(expected));
        }

        TEST_CASE("NodeWriterTest.writeMapWithGroupInDefaultLayer", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::GroupNode* group = map.createGroup("Group");
            map.defaultLayer()->addChild(group);

            Model::BrushBuilder builder(&map, worldBounds);
            Model::BrushNode* brushNode = map.createBrush(builder.createCube(64.0, "none").value());
            group->addChild(brushNode);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

            const std::string expected =
R"(// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group"
"_tb_id" "*"
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
            CHECK_THAT(str.str(), MatchesGlob(expected));
        }

        TEST_CASE("NodeWriterTest.writeMapWithGroupInCustomLayer", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::LayerNode* layer = map.createLayer("Custom Layer");
            map.addChild(layer);

            Model::GroupNode* group = map.createGroup("Group");
            layer->addChild(group);

            Model::BrushBuilder builder(&map, worldBounds);
            Model::BrushNode* brushNode = map.createBrush(builder.createCube(64.0, "none").value());
            group->addChild(brushNode);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

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
// entity 2
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Group"
"_tb_id" "*"
"_tb_layer" "*"
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
            CHECK_THAT(str.str(), MatchesGlob(expected));
        }

        TEST_CASE("NodeWriterTest.writeMapWithNestedGroupInCustomLayer", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::LayerNode* layer = map.createLayer("Custom Layer");
            map.addChild(layer);

            Model::GroupNode* outer = map.createGroup("Outer Group");
            layer->addChild(outer);

            Model::GroupNode* inner = map.createGroup("Inner Group");
            outer->addChild(inner);

            Model::BrushBuilder builder(&map, worldBounds);
            Model::BrushNode* brushNode = map.createBrush(builder.createCube(64.0, "none").value());
            inner->addChild(brushNode);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

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
// entity 2
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Outer Group"
"_tb_id" "*"
"_tb_layer" "*"
}
// entity 3
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Inner Group"
"_tb_id" "*"
"_tb_group" "*"
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
            CHECK_THAT(str.str(), MatchesGlob(expected));
        }

        TEST_CASE("NodeWriterTest.exportMapWithOmittedLayers", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);
            Model::BrushBuilder builder(&map, worldBounds);

            // default layer (omit from export)
            map.defaultLayer()->setOmitFromExport(true);

            auto* defaultLayerPointEntity = map.createEntity(Model::Entity({
                {"classname", "defaultLayerPointEntity"}
            }));

            auto* defaultLayerBrush = map.createBrush(builder.createCube(64.0, "defaultTexture").value());
            map.defaultLayer()->addChild(defaultLayerPointEntity);
            map.defaultLayer()->addChild(defaultLayerBrush);

            // layer1 (omit from export)
            auto* layer1 = map.createLayer("Custom Layer 1");
            map.addChild(layer1);
            layer1->setOmitFromExport(true);

            auto* layer1PointEntity = map.createEntity(Model::Entity({
                {"classname", "layer1PointEntity"}
            }));
            layer1->addChild(layer1PointEntity);

            auto* layer1Brush = map.createBrush(builder.createCube(64.0, "layer1Texture").value());
            layer1->addChild(layer1Brush);

            // layer2
            auto* layer2 = map.createLayer("Custom Layer 2");
            map.addChild(layer2);

            auto* layer2PointEntity = map.createEntity(Model::Entity({
                {"classname", "layer2PointEntity"}
            }));
            layer2->addChild(layer2PointEntity);

            auto* layer2Brush = map.createBrush(builder.createCube(64.0, "layer2Texture").value());
            layer2->addChild(layer2Brush);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.setExporting(true);
            writer.writeMap();

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
            CHECK_THAT(str.str(), MatchesGlob(expected));
        }

        TEST_CASE("NodeWriterTest.writeMapWithInheritedLock", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::LayerNode* layer = map.createLayer("Custom Layer");
            map.addChild(layer);

            // WorldNode's lock state is not persisted.
            // TB uses it e.g. for locking everything when opening a group.
            // So this should result in both the default layer and custom layer being written unlocked.

            map.setLockState(Model::LockState::Lock_Locked);
            map.defaultLayer()->setLockState(Model::LockState::Lock_Inherited);
            layer->setLockState(Model::LockState::Lock_Inherited);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

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
            CHECK_THAT(str.str(), MatchesGlob(expected));
        }

        TEST_CASE("NodeWriterTest.writeNodesWithNestedGroup", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);

            Model::BrushBuilder builder(&map, worldBounds);

            Model::BrushNode* worldBrush = map.createBrush(builder.createCube(64.0, "some").value());
            Model::GroupNode* outer = map.createGroup("Outer Group");
            Model::GroupNode* inner = map.createGroup("Inner Group");
            Model::BrushNode* innerBrush = map.createBrush(builder.createCube(64.0, "none").value());

            inner->addChild(innerBrush);
            outer->addChild(inner);
            map.defaultLayer()->addChild(worldBrush);
            map.defaultLayer()->addChild(outer);

            std::vector<Model::Node*> nodes;
            nodes.push_back(inner);
            nodes.push_back(worldBrush);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeNodes(nodes);

            const std::string expected =
R"(// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) some 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) some 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) some 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) some 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) some 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) some 0 0 0 1 1
}
}
// entity 1
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "Inner Group"
"_tb_id" "*"
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
            CHECK_THAT(str.str(), MatchesGlob(expected));
        }

        TEST_CASE("NodeWriterTest.writeFaces", "[NodeWriterTest]") {
            const vm::bbox3 worldBounds(8192.0);

            Model::WorldNode map(Model::Entity(), Model::MapFormat::Standard);
            Model::BrushBuilder builder(&map, worldBounds);
            Model::BrushNode* brushNode = map.createBrush(builder.createCube(64.0, "none").value());

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeBrushFaces(brushNode->brush().faces());

            const std::string expected =
R"(( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
)";

            const std::string actual = str.str();
            ASSERT_EQ(expected, actual);

            delete brushNode;
        }

        TEST_CASE("NodeWriterTest.writePropertiesWithQuotationMarks", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity({
                {"message", "\"holy damn\", he said"}
            }), Model::MapFormat::Standard);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

            const std::string result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"message\" \"\\\"holy damn\\\", he said\"\n"
                         "\"classname\" \"worldspawn\"\n"
                         "}\n", result.c_str());
        }

        TEST_CASE("NodeWriterTest.writePropertiesWithEscapedQuotationMarks", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity({
                {"message", "\\\"holy damn\\\", he said"}
            }), Model::MapFormat::Standard);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

            const std::string result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"message\" \"\\\"holy damn\\\", he said\"\n"
                         "\"classname\" \"worldspawn\"\n"
                         "}\n", result.c_str());
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/1739
        TEST_CASE("NodeWriterTest.writePropertiesWithNewlineEscapeSequence", "[NodeWriterTest]") {
            Model::WorldNode map(Model::Entity({
                {"message", "holy damn\\nhe said"}
            }), Model::MapFormat::Standard);

            std::stringstream str;
            NodeWriter writer(map, str);
            writer.writeMap();

            const std::string result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"message\" \"holy damn\\nhe said\"\n"
                         "\"classname\" \"worldspawn\"\n"
                         "}\n", result.c_str());
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

            const std::string result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"message\" \"holy damn\"\n"
                         "\"message2\" \"holy damn\\\\\"\n"
                         "\"message3\" \"holy damn\\\\\"\n"
                         "\"classname\" \"worldspawn\"\n"
                         "}\n", result.c_str());
        }
    }
}
