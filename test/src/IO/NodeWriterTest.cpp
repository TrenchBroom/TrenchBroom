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
        TEST(NodeWriterTest, writeEmptyMap) {
            const vm::bbox3 worldBounds(8192.0);

            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);

            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();

            const String result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"classname\" \"worldspawn\"\n"
                         "}\n", result.c_str());
        }

        TEST(NodeWriterTest, writeWorldspawn) {
            const vm::bbox3 worldBounds(8192.0);

            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            map.addOrUpdateAttribute("message", "holy damn");

            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();

            const String result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"classname\" \"worldspawn\"\n"
                         "\"message\" \"holy damn\"\n"
                         "}\n", result.c_str());
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

            const String result = str.str();
            ASSERT_STREQ("// entity 0\n"
                                 "{\n"
                                 "\"classname\" \"worldspawn\"\n"
                                 "// brush 0\n"
                                 "{\n"
                                 "( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1 0 0 0 1 2 3\n"
                                 "( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1 0 0 0 1 2 3\n"
                                 "( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1 0 0 0 1 2 3\n"
                                 "( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1 0 0 0 1 2 3\n"
                                 "( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1 0 0 0 1 2 3\n"
                                 "( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1 0 0 0 1 2 3\n"
                                 "}\n"
                                 "// brush 1\n"
                                 "{\n"
                                 "( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1\n"
                                 "( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1\n"
                                 "( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1\n"
                                 "( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1\n"
                                 "( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1\n"
                                 "( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1\n"
                                 "}\n"
                                 "}\n", result.c_str());
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

            const String result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"classname\" \"worldspawn\"\n"
                         "// brush 0\n"
                         "{\n"
                         "( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1\n"
                         "( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1\n"
                         "( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1\n"
                         "( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1\n"
                         "( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1\n"
                         "( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1\n"
                         "}\n"
                         "}\n", result.c_str());
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
                                                                 "// entity 0\n"
                                                                 "{\n"
                                                                 "\"classname\" \"worldspawn\"\n"
                                                                 "}\n"
                                                                 "// entity 1\n"
                                                                 "{\n"
                                                                 "\"classname\" \"func_group\"\n"
                                                                 "\"_tb_type\" \"_tb_layer\"\n"
                                                                 "\"_tb_name\" \"Custom Layer\"\n"
                                                                 "\"_tb_id\" \"*\"\n"
                                                                 "// brush 0\n"
                                                                 "{\n"
                                                                 "( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1\n"
                                                                 "( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1\n"
                                                                 "( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1\n"
                                                                 "}\n"
                                                                 "}\n"
                                                                 ));
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
                                                                 "// entity 0\n"
                                                                 "{\n"
                                                                 "\"classname\" \"worldspawn\"\n"
                                                                 "}\n"
                                                                 "// entity 1\n"
                                                                 "{\n"
                                                                 "\"classname\" \"func_group\"\n"
                                                                 "\"_tb_type\" \"_tb_group\"\n"
                                                                 "\"_tb_name\" \"Group\"\n"
                                                                 "\"_tb_id\" \"*\"\n"
                                                                 "// brush 0\n"
                                                                 "{\n"
                                                                 "( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1\n"
                                                                 "( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1\n"
                                                                 "( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1\n"
                                                                 "}\n"
                                                                 "}\n"
                                                                 ));
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
                                                                 "// entity 0\n"
                                                                 "{\n"
                                                                 "\"classname\" \"worldspawn\"\n"
                                                                 "}\n"
                                                                 "// entity 1\n"
                                                                 "{\n"
                                                                 "\"classname\" \"func_group\"\n"
                                                                 "\"_tb_type\" \"_tb_layer\"\n"
                                                                 "\"_tb_name\" \"Custom Layer\"\n"
                                                                 "\"_tb_id\" \"*\"\n"
                                                                 "}\n"
                                                                 "// entity 2\n"
                                                                 "{\n"
                                                                 "\"classname\" \"func_group\"\n"
                                                                 "\"_tb_type\" \"_tb_group\"\n"
                                                                 "\"_tb_name\" \"Group\"\n"
                                                                 "\"_tb_id\" \"*\"\n"
                                                                 "\"_tb_layer\" \"*\"\n"
                                                                 "// brush 0\n"
                                                                 "{\n"
                                                                 "( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1\n"
                                                                 "( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1\n"
                                                                 "( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1\n"
                                                                 "}\n"
                                                                 "}\n"
                                                                 ));
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
                                                                 "// entity 0\n"
                                                                 "{\n"
                                                                 "\"classname\" \"worldspawn\"\n"
                                                                 "}\n"
                                                                 "// entity 1\n"
                                                                 "{\n"
                                                                 "\"classname\" \"func_group\"\n"
                                                                 "\"_tb_type\" \"_tb_layer\"\n"
                                                                 "\"_tb_name\" \"Custom Layer\"\n"
                                                                 "\"_tb_id\" \"*\"\n"
                                                                 "}\n"
                                                                 "// entity 2\n"
                                                                 "{\n"
                                                                 "\"classname\" \"func_group\"\n"
                                                                 "\"_tb_type\" \"_tb_group\"\n"
                                                                 "\"_tb_name\" \"Outer Group\"\n"
                                                                 "\"_tb_id\" \"*\"\n"
                                                                 "\"_tb_layer\" \"*\"\n"
                                                                 "}\n"
                                                                 "// entity 3\n"
                                                                 "{\n"
                                                                 "\"classname\" \"func_group\"\n"
                                                                 "\"_tb_type\" \"_tb_group\"\n"
                                                                 "\"_tb_name\" \"Inner Group\"\n"
                                                                 "\"_tb_id\" \"*\"\n"
                                                                 "\"_tb_group\" \"*\"\n"
                                                                 "// brush 0\n"
                                                                 "{\n"
                                                                 "( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1\n"
                                                                 "( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1\n"
                                                                 "( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1\n"
                                                                 "}\n"
                                                                 "}\n"
                                                                 ));
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
                                                                 "// entity 0\n"
                                                                 "{\n"
                                                                 "\"classname\" \"worldspawn\"\n"
                                                                 "// brush 0\n"
                                                                 "{\n"
                                                                 "( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) some 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) some 0 0 0 1 1\n"
                                                                 "( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) some 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) some 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) some 0 0 0 1 1\n"
                                                                 "( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) some 0 0 0 1 1\n"
                                                                 "}\n"
                                                                 "}\n"
                                                                 "// entity 1\n"
                                                                 "{\n"
                                                                 "\"classname\" \"func_group\"\n"
                                                                 "\"_tb_type\" \"_tb_group\"\n"
                                                                 "\"_tb_name\" \"Inner Group\"\n"
                                                                 "\"_tb_id\" \"*\"\n"
                                                                 "// brush 0\n"
                                                                 "{\n"
                                                                 "( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1\n"
                                                                 "( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1\n"
                                                                 "( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1\n"
                                                                 "( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1\n"
                                                                 "}\n"
                                                                 "}\n"
                                                                 ));
        }

        TEST(NodeWriterTest, writeFaces) {
            const vm::bbox3 worldBounds(8192.0);

            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            Model::BrushBuilder builder(&map, worldBounds);
            Model::Brush* brush = builder.createCube(64.0, "none");

            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeBrushFaces(brush->faces());

            const String result = str.str();
            ASSERT_STREQ("( -32 -32 -32 ) ( -32 -31 -32 ) ( -32 -32 -31 ) none 0 0 0 1 1\n"
                         "( 32 32 32 ) ( 32 32 33 ) ( 32 33 32 ) none 0 0 0 1 1\n"
                         "( -32 -32 -32 ) ( -32 -32 -31 ) ( -31 -32 -32 ) none 0 0 0 1 1\n"
                         "( 32 32 32 ) ( 33 32 32 ) ( 32 32 33 ) none 0 0 0 1 1\n"
                         "( 32 32 32 ) ( 32 33 32 ) ( 33 32 32 ) none 0 0 0 1 1\n"
                         "( -32 -32 -32 ) ( -31 -32 -32 ) ( -32 -31 -32 ) none 0 0 0 1 1\n",
                         result.c_str());

            delete brush;
        }


        TEST(NodeWriterTest, writePropertiesWithQuotationMarks) {
            const vm::bbox3 worldBounds(8192.0);

            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            map.addOrUpdateAttribute("message", "\"holy damn\", he said");

            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();

            const String result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"classname\" \"worldspawn\"\n"
                         "\"message\" \"\\\"holy damn\\\", he said\"\n"
                         "}\n", result.c_str());
        }

        TEST(NodeWriterTest, writePropertiesWithEscapedQuotationMarks) {
            const vm::bbox3 worldBounds(8192.0);

            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            map.addOrUpdateAttribute("message", "\\\"holy damn\\\", he said");

            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();

            const String result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"classname\" \"worldspawn\"\n"
                         "\"message\" \"\\\"holy damn\\\", he said\"\n"
                         "}\n", result.c_str());
        }

        // https://github.com/kduske/TrenchBroom/issues/1739
        TEST(NodeWriterTest, writePropertiesWithNewlineEscapeSequence) {
            const vm::bbox3 worldBounds(8192.0);

            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            map.addOrUpdateAttribute("message", "holy damn\\nhe said");

            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();

            const String result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"classname\" \"worldspawn\"\n"
                         "\"message\" \"holy damn\\nhe said\"\n"
                         "}\n", result.c_str());
        }

        // https://github.com/kduske/TrenchBroom/issues/2556
        TEST(NodeWriterTest, writePropertiesWithTrailingBackslash) {
            const vm::bbox3 worldBounds(8192.0);

            Model::World map(Model::MapFormat::Standard, nullptr, worldBounds);
            map.addOrUpdateAttribute("classname", "worldspawn");
            map.addOrUpdateAttribute("message\\", "holy damn\\");

            StringStream str;
            NodeWriter writer(&map, str);
            writer.writeMap();

            const String result = str.str();
            ASSERT_STREQ("// entity 0\n"
                         "{\n"
                         "\"classname\" \"worldspawn\"\n"
                         "\"message\" \"holy damn\"\n"
                         "}\n", result.c_str());
        }
    }
}
