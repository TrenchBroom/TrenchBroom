/*
 Copyright (C) 2021 Kristian Duske
 Copyright (C) 2021 Eric Wasylishen

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

#include "MapDocumentTest.h"
#include "TestUtils.h"

#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "View/PasteType.h"

#include <kdl/result.h>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        TEST_CASE_METHOD(MapDocumentTest, "CopyPasteTest.paste") {
            SECTION("Paste worldspawn with single brush in layer") {
                const auto data = R"(
{
"classname" "worldspawn"
"to_be_ignored" "somevalue"
}
{
"classname" "func_group"
"_tb_type" "_tb_layer"
"_tb_name" "My Layer"
"_tb_id" "1"
{
( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
}
})";

                const auto& world = *document->world();
                REQUIRE_FALSE(world.entity().hasProperty("to_be_ignored"));

                const auto& defaultLayer = *world.defaultLayer();
                REQUIRE(defaultLayer.childCount() == 0u);
                REQUIRE(world.customLayers().empty());

                CHECK(document->paste(data) == PasteType::Node);
                CHECK_FALSE(world.entity().hasProperty("to_be_ignored"));
                CHECK(world.customLayers().empty());
                CHECK(defaultLayer.childCount() == 1u);
                CHECK(dynamic_cast<Model::BrushNode*>(defaultLayer.children().front()) != nullptr);
            }

            SECTION("Paste worldspawn with single brush in group") {
                const auto data = R"(
{
"classname" "worldspawn"
"to_be_ignored" "somevalue"
}
{
"classname" "func_group"
"_tb_type" "_tb_group"
"_tb_name" "My Group"
"_tb_id" "2"
{
( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
}
})";

                const auto& world = *document->world();
                REQUIRE_FALSE(world.entity().hasProperty("to_be_ignored"));

                const auto& defaultLayer = *world.defaultLayer();
                REQUIRE(defaultLayer.childCount() == 0u);

                CHECK(document->paste(data) == PasteType::Node);
                CHECK_FALSE(world.entity().hasProperty("to_be_ignored"));
                CHECK(defaultLayer.childCount() == 1u);

                const auto* groupNode = dynamic_cast<Model::GroupNode*>(defaultLayer.children().front());
                CHECK(groupNode != nullptr);
                CHECK(groupNode->group().name() == "My Group");
                CHECK(groupNode->childCount() == 1u);
                CHECK(dynamic_cast<Model::BrushNode*>(groupNode->children().front()) != nullptr);
            }

            SECTION("Paste worldspawn with single brush in entity") {
                const auto data = R"(
{
"classname" "worldspawn"
"to_be_ignored" "somevalue"
}
{
"classname" "func_door"
{
( -800 288 1024 ) ( -736 288 1024 ) ( -736 224 1024 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 288 1024 ) ( -800 224 1024 ) ( -800 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 224 1024 ) ( -736 288 1024 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -736 288 1024 ) ( -800 288 1024 ) ( -800 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 1024 ) ( -736 224 1024 ) ( -736 224 576 ) rtz/c_mf_v3c 56 -32 0 1 1
( -800 224 576 ) ( -736 224 576 ) ( -736 288 576 ) rtz/c_mf_v3c 56 -32 0 1 1
}
})";

                const auto& world = *document->world();
                REQUIRE_FALSE(world.entity().hasProperty("to_be_ignored"));

                const auto& defaultLayer = *world.defaultLayer();
                REQUIRE(defaultLayer.childCount() == 0u);

                CHECK(document->paste(data) == PasteType::Node);
                CHECK_FALSE(world.entity().hasProperty("to_be_ignored"));
                CHECK(defaultLayer.childCount() == 1u);

                const auto* entityNode = dynamic_cast<Model::EntityNode*>(defaultLayer.children().front());
                CHECK(entityNode != nullptr);
                CHECK(entityNode->entity().classname() == "func_door");
                CHECK(entityNode->childCount() == 1u);
                CHECK(dynamic_cast<Model::BrushNode*>(entityNode->children().front()) != nullptr);
            }

            SECTION("Paste worldspawn with single brush") {
                const auto data = R"(
{
"classname" "worldspawn"
"to_be_ignored" "somevalue"
{
( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) tex1 1 2 3 4 5
( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) tex2 0 0 0 1 1
( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) tex3 0 0 0 1 1
( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) tex4 0 0 0 1 1
( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) tex5 0 0 0 1 1
( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) tex6 0 0 0 1 1
}
})";

                const auto& world = *document->world();
                REQUIRE_FALSE(world.entity().hasProperty("to_be_ignored"));

                const auto& defaultLayer = *world.defaultLayer();
                REQUIRE(defaultLayer.childCount() == 0u);

                CHECK(document->paste(data) == PasteType::Node);
                CHECK_FALSE(world.entity().hasProperty("to_be_ignored"));
                CHECK(defaultLayer.childCount() == 1u);
                CHECK(dynamic_cast<Model::BrushNode*>(defaultLayer.children().front()) != nullptr);
            }

            SECTION("Paste single brush") {
                const auto data = R"(
{
( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) tex1 1 2 3 4 5
( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) tex2 0 0 0 1 1
( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) tex3 0 0 0 1 1
( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) tex4 0 0 0 1 1
( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) tex5 0 0 0 1 1
( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) tex6 0 0 0 1 1
})";

                const auto& world = *document->world();

                const auto& defaultLayer = *world.defaultLayer();
                REQUIRE(defaultLayer.childCount() == 0u);

                CHECK(document->paste(data) == PasteType::Node);
                CHECK(defaultLayer.childCount() == 1u);
                CHECK(dynamic_cast<Model::BrushNode*>(defaultLayer.children().front()) != nullptr);
            }
        }

        TEST_CASE_METHOD(Quake3MapDocumentTest, "CopyPasteTest.pastePatch") {
            SECTION("Paste single patch") {
                const auto data = R"(
{
patchDef2
{
common/caulk
( 5 3 0 0 0 )
(
( (-64 -64 4 0   0 ) (-64 0 4 0   -0.25 ) (-64 64 4 0   -0.5 ) )
( (  0 -64 4 0.2 0 ) (  0 0 4 0.2 -0.25 ) (  0 64 4 0.2 -0.5 ) )
( ( 64 -64 4 0.4 0 ) ( 64 0 4 0.4 -0.25 ) ( 64 64 4 0.4 -0.5 ) )
( (128 -64 4 0.6 0 ) (128 0 4 0.6 -0.25 ) (128 64 4 0.6 -0.5 ) )
( (192 -64 4 0.8 0 ) (192 0 4 0.8 -0.25 ) (192 64 4 0.8 -0.5 ) )
)
}
})";

                const auto& world = *document->world();

                const auto& defaultLayer = *world.defaultLayer();
                REQUIRE(defaultLayer.childCount() == 0u);

                CHECK(document->paste(data) == PasteType::Node);
                CHECK(defaultLayer.childCount() == 1u);
                CHECK(dynamic_cast<Model::PatchNode*>(defaultLayer.children().front()) != nullptr);
            }
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/2776
        TEST_CASE_METHOD(MapDocumentTest, "CopyPasteTest.pasteAndTranslateGroup") {
            // delete default brush
            document->selectAllNodes();
            document->deleteObjects();

            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            const auto box = vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64));

            auto* brushNode1 = new Model::BrushNode(builder.createCuboid(box, "texture").value());
            addNode(*document, document->parentForNodes(), brushNode1);
            document->select(brushNode1);

            const auto groupName = std::string("testGroup");

            auto* group = document->groupSelection(groupName);
            CHECK(group != nullptr);
            document->select(group);

            const std::string copied = document->serializeSelectedNodes();

            const auto delta = vm::vec3(16, 16, 16);
            CHECK(document->paste(copied) == PasteType::Node);
            CHECK(document->selectedNodes().groupCount() == 1u);
            CHECK(document->selectedNodes().groups().at(0)->name() == groupName);
            CHECK(document->translateObjects(delta));
            CHECK(document->selectionBounds() == box.translate(delta));
        }

        TEST_CASE_METHOD(MapDocumentTest, "CopyPasteTest.pasteInGroup", "[CopyPasteTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1734

            const std::string data("{"
                              "\"classname\" \"light\""
                              "\"origin\" \"0 0 0\""
                              "}");

            Model::BrushNode* brush = createBrushNode();
            addNode(*document, document->parentForNodes(), brush);
            document->select(brush);

            Model::GroupNode* group = document->groupSelection("test");
            document->openGroup(group);

            CHECK(document->paste(data) == PasteType::Node);
            CHECK(document->selectedNodes().hasOnlyEntities());
            CHECK(document->selectedNodes().entityCount() == 1u);

            Model::EntityNode* light = document->selectedNodes().entities().front();
            CHECK(light->parent() == group);
        }

        TEST_CASE_METHOD(MapDocumentTest, "CopyPasteTest.copyPasteGroupResetsDuplicateGroupId", "[CopyPasteTest]") {
            auto* entityNode = new Model::EntityNode{};
            document->addNodes({{document->parentForNodes(), {entityNode}}});

            document->select(entityNode);
            auto* groupNode = document->groupSelection("test");

            const auto persistentGroupId = groupNode->persistentId();
            REQUIRE(persistentGroupId.has_value());

            document->deselectAll();
            document->select(groupNode);

            const auto str = document->serializeSelectedNodes();

            SECTION("Copy and paste resets persistent group ID") {
                document->deselectAll();
                REQUIRE(document->paste(str) == PasteType::Node);

                auto* pastedGroupNode = dynamic_cast<Model::GroupNode*>(document->world()->defaultLayer()->children().back());
                REQUIRE(pastedGroupNode != nullptr);
                REQUIRE(pastedGroupNode != groupNode);

                CHECK(pastedGroupNode->persistentId() != persistentGroupId);
            }

            SECTION("Cut and paste retains persistent group ID") {
                document->deleteObjects();
                document->deselectAll();
                REQUIRE(document->paste(str) == PasteType::Node);

                auto* pastedGroupNode = dynamic_cast<Model::GroupNode*>(document->world()->defaultLayer()->children().back());
                REQUIRE(pastedGroupNode != nullptr);
                REQUIRE(pastedGroupNode != groupNode);

                CHECK(pastedGroupNode->persistentId() == persistentGroupId);
            }
        }
    }
}
