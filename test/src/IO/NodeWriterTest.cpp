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

#include <gtest/gtest.h>

#include "StringUtils.h"
#include "IO/NodeWriter.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/MapFormat.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace IO {
        void assertNodeWriterResult(const String& expected, const StringStream& actual);
        void assertNodeWriterResult(const String& expected, const StringStream& actual) {
            ASSERT_EQ(expected, actual.str());
        }

        TEST(NodeWriterTest, writeEmptyMap) {
            const vm::bbox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            
            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();
            
            assertNodeWriterResult(
R"(// entity 0
{
"classname" "worldspawn"
}
)",
            str);
        }
        
        TEST(NodeWriterTest, writeWorldspawn) {
            const vm::bbox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            map.addOrUpdateAttribute("message", "holy damn");
            
            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();
            
            assertNodeWriterResult(
R"(// entity 0
{
"classname" "worldspawn"
"message" "holy damn"
}
)",
            str);
        }

        TEST(NodeWriterTest, writeDaikatanaMap) {
            const vm::bbox3 worldBounds(8192.0);

            Model::World map(Model::MapFormat::Daikatana, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");

            Model::BrushBuilder builder(&map, worldBounds);
            Model::Brush* brush1 = builder.createCube(64.0, "none");
            for (auto* face : brush1->faces()) {
                face->setColor(Color(1.0f, 2.0f, 3.0f));
            }
            map.defaultLayer()->addChild(brush1);

            Model::Brush* brush2 = builder.createCube(64.0, "none");
            map.defaultLayer()->addChild(brush2);

            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();

            assertNodeWriterResult(
R"(// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1 0 0 0 1 2 3
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1 0 0 0 1 2 3
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1 0 0 0 1 2 3
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1 0 0 0 1 2 3
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1 0 0 0 1 2 3
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1 0 0 0 1 2 3
}
// brush 1
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
}
}
)",
            str);
        }
        
        TEST(NodeWriterTest, writeWorldspawnWithBrushInDefaultLayer) {
            const vm::bbox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            
            Model::BrushBuilder builder(&map, worldBounds);
            Model::Brush* brush = builder.createCube(64.0, "none");
            map.defaultLayer()->addChild(brush);
            
            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();
            
            assertNodeWriterResult(
R"(// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
}
}
)",
            str);
        }
        
        TEST(NodeWriterTest, writeWorldspawnWithBrushInCustomLayer) {
            const vm::bbox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            
            Model::Layer* layer = map.createLayer("Custom Layer", worldBounds);
            map.addChild(layer);
            
            Model::BrushBuilder builder(&map, worldBounds);
            Model::Brush* brush = builder.createCube(64.0, "none");
            layer->addChild(brush);
            
            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();
            
            ASSERT_TRUE(StringUtils::caseSensitiveMatchesPattern(str.str(),
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
// brush 0
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
}
}
)"));
        }
        
        TEST(NodeWriterTest, writeMapWithGroupInDefaultLayer) {
            const vm::bbox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            
            Model::Group* group = map.createGroup("Group");
            map.defaultLayer()->addChild(group);
            
            Model::BrushBuilder builder(&map, worldBounds);
            Model::Brush* brush = builder.createCube(64.0, "none");
            group->addChild(brush);
            
            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();
            
            ASSERT_TRUE(StringUtils::caseSensitiveMatchesPattern(str.str(),
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
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
}
}
)"));
        }
        
        TEST(NodeWriterTest, writeMapWithGroupInCustomLayer) {
            const vm::bbox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            
            Model::Layer* layer = map.createLayer("Custom Layer", worldBounds);
            map.addChild(layer);
            
            Model::Group* group = map.createGroup("Group");
            layer->addChild(group);
            
            Model::BrushBuilder builder(&map, worldBounds);
            Model::Brush* brush = builder.createCube(64.0, "none");
            group->addChild(brush);
            
            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();
            
            ASSERT_TRUE(StringUtils::caseSensitiveMatchesPattern(str.str(),
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
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
}
}
)"));
        }
        
        TEST(NodeWriterTest, writeMapWithNestedGroupInCustomLayer) {
            const vm::bbox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            
            Model::Layer* layer = map.createLayer("Custom Layer", worldBounds);
            map.addChild(layer);
            
            Model::Group* outer = map.createGroup("Outer Group");
            layer->addChild(outer);
            
            Model::Group* inner = map.createGroup("Inner Group");
            outer->addChild(inner);
            
            Model::BrushBuilder builder(&map, worldBounds);
            Model::Brush* brush = builder.createCube(64.0, "none");
            inner->addChild(brush);
            
            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();
            
            ASSERT_TRUE(StringUtils::caseSensitiveMatchesPattern(str.str(),
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
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
}
}
)"));
        }
        
        TEST(NodeWriterTest, writeNodesWithNestedGroup) {
            const vm::bbox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            
            Model::BrushBuilder builder(&map, worldBounds);
            
            Model::Brush* worldBrush = builder.createCube(64.0, "some");
            Model::Group* outer = map.createGroup("Outer Group");
            Model::Group* inner = map.createGroup("Inner Group");
            Model::Brush* innerBrush = builder.createCube(64.0, "none");
            
            inner->addChild(innerBrush);
            outer->addChild(inner);
            map.defaultLayer()->addChild(worldBrush);
            map.defaultLayer()->addChild(outer);
            
            Model::NodeList nodes;
            nodes.push_back(inner);
            nodes.push_back(worldBrush);
            
            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeNodes(nodes);
            
            ASSERT_TRUE(StringUtils::caseSensitiveMatchesPattern(str.str(),
R"(// entity 0
{
"classname" "worldspawn"
// brush 0
{
( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) some 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) some 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) some 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) some 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) some 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) some 0 0 0 1 1
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
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
}
}
)"));
        }
        
        TEST(NodeWriterTest, writeFaces) {
            const vm::bbox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            Model::BrushBuilder builder(&map, worldBounds);
            Model::Brush* brush = builder.createCube(64.0, "none");
            
            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeBrushFaces(brush->faces());
            
            assertNodeWriterResult(
R"(( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1
( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1
( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1
( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1
)", str);
            
            delete brush;
        }

        
        TEST(NodeWriterTest, writePropertiesWithQuotationMarks) {
            const vm::bbox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            map.addOrUpdateAttribute("message", R"("holy damn", he said)");
            
            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();
            
            assertNodeWriterResult(
R"(// entity 0
{
"classname" "worldspawn"
"message" "\"holy damn\", he said"
}
)", str);
        }
        
        TEST(NodeWriterTest, writePropertiesWithEscapedQuotationMarks) {
            const vm::bbox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            map.addOrUpdateAttribute("message", R"(\"holy damn\", he said)");
            
            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();
            
            assertNodeWriterResult(
R"(// entity 0
{
"classname" "worldspawn"
"message" "\"holy damn\", he said"
}
)", str);
        }
        
        // https://github.com/kduske/TrenchBroom/issues/1739
        TEST(NodeWriterTest, writePropertiesWithNewlineEscapeSequence) {            
            const vm::bbox3 worldBounds(8192.0);
            
            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            map.addOrUpdateAttribute("message", R"(holy damn\nhe said)");
            
            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();
            
            assertNodeWriterResult(
R"(// entity 0
{
"classname" "worldspawn"
"message" "holy damn\nhe said"
}
)", str);
        }


        // https://github.com/kduske/TrenchBroom/issues/2556
        TEST(NodeWriterTest, writePropertiesWithTrailingBackslash) {
            const vm::bbox3 worldBounds(8192.0);

            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            map.addOrUpdateAttribute(R"(message\)", R"(holy damn\)");

            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();

            assertNodeWriterResult(
R"(// entity 0
{
"classname" "worldspawn"
"message" "holy damn"
}
)", str);
        }
    }
}
