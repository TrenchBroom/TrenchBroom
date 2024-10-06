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
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"
#include "ui/PasteType.h"

#include "kdl/result.h"

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(MapDocumentTest, "CopyPasteTest.paste")
{
  SECTION("Paste worldspawn with single brush in layer")
  {
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

    const auto& worldNode = *document->world();
    REQUIRE_FALSE(worldNode.entity().hasProperty("to_be_ignored"));

    const auto& defaultLayerNode = *worldNode.defaultLayer();
    REQUIRE(defaultLayerNode.childCount() == 0u);
    REQUIRE(worldNode.customLayers().empty());

    CHECK(document->paste(data) == PasteType::Node);
    CHECK_FALSE(worldNode.entity().hasProperty("to_be_ignored"));
    CHECK(worldNode.customLayers().empty());
    CHECK(defaultLayerNode.childCount() == 1u);
    CHECK(dynamic_cast<mdl::BrushNode*>(defaultLayerNode.children().front()) != nullptr);
  }

  SECTION("Paste worldspawn with single brush in group")
  {
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

    const auto& worldNode = *document->world();
    REQUIRE_FALSE(worldNode.entity().hasProperty("to_be_ignored"));

    const auto& defaultLayerNode = *worldNode.defaultLayer();
    REQUIRE(defaultLayerNode.childCount() == 0u);

    CHECK(document->paste(data) == PasteType::Node);
    CHECK_FALSE(worldNode.entity().hasProperty("to_be_ignored"));
    CHECK(defaultLayerNode.childCount() == 1u);

    const auto* groupNode =
      dynamic_cast<mdl::GroupNode*>(defaultLayerNode.children().front());
    CHECK(groupNode != nullptr);
    CHECK(groupNode->group().name() == "My Group");
    CHECK(groupNode->childCount() == 1u);
    CHECK(dynamic_cast<mdl::BrushNode*>(groupNode->children().front()) != nullptr);
  }

  SECTION("Paste worldspawn with single brush in entity")
  {
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

    const auto& worldNode = *document->world();
    REQUIRE_FALSE(worldNode.entity().hasProperty("to_be_ignored"));

    const auto& defaultLayerNode = *worldNode.defaultLayer();
    REQUIRE(defaultLayerNode.childCount() == 0u);

    CHECK(document->paste(data) == PasteType::Node);
    CHECK_FALSE(worldNode.entity().hasProperty("to_be_ignored"));
    CHECK(defaultLayerNode.childCount() == 1u);

    const auto* entityNode =
      dynamic_cast<mdl::EntityNode*>(defaultLayerNode.children().front());
    CHECK(entityNode != nullptr);
    CHECK(entityNode->entity().classname() == "func_door");
    CHECK(entityNode->childCount() == 1u);
    CHECK(dynamic_cast<mdl::BrushNode*>(entityNode->children().front()) != nullptr);
  }

  SECTION("Paste worldspawn with single brush")
  {
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

    const auto& worldNode = *document->world();
    REQUIRE_FALSE(worldNode.entity().hasProperty("to_be_ignored"));

    const auto& defaultLayerNode = *worldNode.defaultLayer();
    REQUIRE(defaultLayerNode.childCount() == 0u);

    CHECK(document->paste(data) == PasteType::Node);
    CHECK_FALSE(worldNode.entity().hasProperty("to_be_ignored"));
    CHECK(defaultLayerNode.childCount() == 1u);
    CHECK(dynamic_cast<mdl::BrushNode*>(defaultLayerNode.children().front()) != nullptr);
  }

  SECTION("Paste single brush")
  {
    const auto data = R"(
{
( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) tex1 1 2 3 4 5
( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) tex2 0 0 0 1 1
( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) tex3 0 0 0 1 1
( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) tex4 0 0 0 1 1
( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) tex5 0 0 0 1 1
( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) tex6 0 0 0 1 1
})";

    const auto& worldNode = *document->world();

    const auto& defaultLayerNode = *worldNode.defaultLayer();
    REQUIRE(defaultLayerNode.childCount() == 0u);

    CHECK(document->paste(data) == PasteType::Node);
    CHECK(defaultLayerNode.childCount() == 1u);
    CHECK(dynamic_cast<mdl::BrushNode*>(defaultLayerNode.children().front()) != nullptr);
  }
}

TEST_CASE_METHOD(Quake3MapDocumentTest, "CopyPasteTest.pastePatch")
{
  SECTION("Paste single patch")
  {
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

    const auto& worldNode = *document->world();

    const auto& defaultLayerNode = *worldNode.defaultLayer();
    REQUIRE(defaultLayerNode.childCount() == 0u);

    CHECK(document->paste(data) == PasteType::Node);
    CHECK(defaultLayerNode.childCount() == 1u);
    CHECK(dynamic_cast<mdl::PatchNode*>(defaultLayerNode.children().front()) != nullptr);
  }
}

TEST_CASE_METHOD(MapDocumentTest, "CopyPasteTest.copyPasteGroupResetsDuplicateGroupId")
{
  auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});

  document->selectNodes({entityNode});
  auto* groupNode = document->groupSelection("test");

  const auto persistentGroupId = groupNode->persistentId();
  REQUIRE(persistentGroupId.has_value());

  document->deselectAll();
  document->selectNodes({groupNode});

  const auto str = document->serializeSelectedNodes();

  SECTION("Copy and paste resets persistent group ID")
  {
    document->deselectAll();
    REQUIRE(document->paste(str) == PasteType::Node);

    auto* pastedGroupNode =
      dynamic_cast<mdl::GroupNode*>(document->world()->defaultLayer()->children().back());
    REQUIRE(pastedGroupNode != nullptr);
    REQUIRE(pastedGroupNode != groupNode);

    CHECK(pastedGroupNode->persistentId() != persistentGroupId);
  }

  SECTION("Cut and paste retains persistent group ID")
  {
    document->deleteObjects();
    document->deselectAll();
    REQUIRE(document->paste(str) == PasteType::Node);

    auto* pastedGroupNode =
      dynamic_cast<mdl::GroupNode*>(document->world()->defaultLayer()->children().back());
    REQUIRE(pastedGroupNode != nullptr);
    REQUIRE(pastedGroupNode != groupNode);

    CHECK(pastedGroupNode->persistentId() == persistentGroupId);
  }
}

// https://github.com/TrenchBroom/TrenchBroom/issues/2776
TEST_CASE_METHOD(MapDocumentTest, "CopyPasteTest.pasteAndTranslateGroup")
{
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  const auto builder =
    mdl::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};
  const auto box = vm::bbox3d{vm::vec3d{0, 0, 0}, vm::vec3d{64, 64, 64}};

  auto* brushNode1 =
    new mdl::BrushNode{builder.createCuboid(box, "material") | kdl::value()};
  document->addNodes({{document->parentForNodes(), {brushNode1}}});
  document->selectNodes({brushNode1});

  const auto groupName = std::string{"testGroup"};

  auto* groupNode = document->groupSelection(groupName);
  CHECK(groupNode != nullptr);
  document->selectNodes({groupNode});

  const auto copied = document->serializeSelectedNodes();

  const auto delta = vm::vec3d{16, 16, 16};
  CHECK(document->paste(copied) == PasteType::Node);
  CHECK(document->selectedNodes().groupCount() == 1u);
  CHECK(document->selectedNodes().groups().at(0)->name() == groupName);
  CHECK(document->translateObjects(delta));
  CHECK(document->selectionBounds() == box.translate(delta));
}

TEST_CASE_METHOD(MapDocumentTest, "CopyPasteTest.pasteInGroup")
{
  // https://github.com/TrenchBroom/TrenchBroom/issues/1734

  const auto data = std::string{
    R"({
"classname" "light"
"origin" "0 0 0"
})"};

  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  auto* groupNode = document->groupSelection("test");
  document->openGroup(groupNode);

  CHECK(document->paste(data) == PasteType::Node);
  CHECK(document->selectedNodes().hasOnlyEntities());
  CHECK(document->selectedNodes().entityCount() == 1u);

  auto* light = document->selectedNodes().entities().front();
  CHECK(light->parent() == groupNode);
}

TEST_CASE_METHOD(MapDocumentTest, "CopyPasteTest.copyPasteGroupResetsDuplicatedLinkIds")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  auto* groupNode = document->groupSelection("test");

  document->deselectAll();
  document->selectNodes({groupNode});
  auto* linkedGroup = document->createLinkedDuplicate();

  const auto originalGroupLinkId = linkedGroup->linkId();
  REQUIRE(originalGroupLinkId == groupNode->linkId());

  auto* linkedBrushNode = dynamic_cast<mdl::BrushNode*>(linkedGroup->children().front());
  REQUIRE(linkedBrushNode);

  const auto originalBrushLinkId = linkedBrushNode->linkId();
  REQUIRE(originalBrushLinkId == brushNode->linkId());

  document->deselectAll();

  SECTION("Pasting one linked brush")
  {
    document->deselectAll();
    document->openGroup(groupNode);

    document->selectNodes({brushNode});
    const auto data = document->serializeSelectedNodes();

    document->deselectAll();

    CHECK(document->paste(data) == PasteType::Node);
    CHECK(groupNode->childCount() == 2);

    const auto* pastedBrushNode =
      dynamic_cast<mdl::BrushNode*>(groupNode->children().back());
    REQUIRE(pastedBrushNode);

    CHECK(pastedBrushNode->linkId() != originalBrushLinkId);
  }

  SECTION("Pasting one linked group")
  {
    document->selectNodes({linkedGroup});
    const auto data = document->serializeSelectedNodes();

    document->deselectAll();

    SECTION("Pasting unknown linked group ID")
    {
      document->selectAllNodes();
      document->deleteObjects();

      CHECK(document->paste(data) == PasteType::Node);
      CHECK(document->world()->defaultLayer()->childCount() == 1);

      const auto* pastedGroupNode = dynamic_cast<mdl::GroupNode*>(
        document->world()->defaultLayer()->children().back());
      REQUIRE(pastedGroupNode);

      CHECK(pastedGroupNode->linkId() == originalGroupLinkId);
    }

    SECTION("If only one linked group exists")
    {
      document->selectNodes({linkedGroup});
      document->deleteObjects();

      CHECK(document->paste(data) == PasteType::Node);
      CHECK(document->world()->defaultLayer()->childCount() == 2);

      const auto* pastedGroupNode = dynamic_cast<mdl::GroupNode*>(
        document->world()->defaultLayer()->children().back());
      REQUIRE(pastedGroupNode);

      CHECK(pastedGroupNode->linkId() != originalGroupLinkId);

      const auto* pastedBrushNode =
        dynamic_cast<mdl::BrushNode*>(pastedGroupNode->children().front());
      REQUIRE(pastedBrushNode);

      CHECK(pastedBrushNode->linkId() != originalBrushLinkId);
    }

    SECTION("If more than one linked group exists")
    {
      CHECK(document->paste(data) == PasteType::Node);
      CHECK(document->world()->defaultLayer()->childCount() == 3);

      const auto* pastedGroupNode = dynamic_cast<mdl::GroupNode*>(
        document->world()->defaultLayer()->children().back());
      REQUIRE(pastedGroupNode);

      CHECK(pastedGroupNode->linkId() == originalGroupLinkId);

      const auto* pastedBrushNode =
        dynamic_cast<mdl::BrushNode*>(pastedGroupNode->children().front());
      REQUIRE(pastedBrushNode);

      CHECK(pastedBrushNode->linkId() == originalBrushLinkId);
    }

    SECTION("Pasting recursive linked group")
    {
      document->openGroup(groupNode);

      CHECK(document->paste(data) == PasteType::Node);
      CHECK(groupNode->childCount() == 2);
      CHECK(linkedGroup->childCount() == 2);

      auto* pastedGroup = dynamic_cast<mdl::GroupNode*>(groupNode->children().back());
      REQUIRE(pastedGroup);

      CHECK(pastedGroup->linkId() != originalGroupLinkId);

      const auto* pastedBrushNode =
        dynamic_cast<mdl::BrushNode*>(pastedGroup->children().front());
      REQUIRE(pastedBrushNode);

      CHECK(pastedBrushNode->linkId() != originalBrushLinkId);

      auto* linkedPastedGroupNode =
        dynamic_cast<mdl::GroupNode*>(linkedGroup->children().back());
      REQUIRE(linkedPastedGroupNode);

      CHECK(linkedPastedGroupNode->linkId() == pastedGroup->linkId());

      const auto* linkedPastedBrushNode =
        dynamic_cast<mdl::BrushNode*>(linkedPastedGroupNode->children().front());
      REQUIRE(pastedBrushNode);

      CHECK(linkedPastedBrushNode->linkId() == pastedBrushNode->linkId());
    }
  }

  SECTION("Pasting two linked groups")
  {
    document->selectNodes({groupNode, linkedGroup});
    const auto data = document->serializeSelectedNodes();

    document->deselectAll();

    SECTION("If only one original group exists")
    {
      document->selectNodes({linkedGroup});
      document->deleteObjects();

      CHECK(document->paste(data) == PasteType::Node);
      CHECK(document->world()->defaultLayer()->childCount() == 3);

      const auto* pastedGroupNode1 =
        dynamic_cast<mdl::GroupNode*>(document->world()->defaultLayer()->children()[1]);
      REQUIRE(pastedGroupNode1);

      const auto* pastedGroupNode2 =
        dynamic_cast<mdl::GroupNode*>(document->world()->defaultLayer()->children()[2]);
      REQUIRE(pastedGroupNode2);

      CHECK(pastedGroupNode1->linkId() != originalGroupLinkId);
      CHECK(pastedGroupNode2->linkId() != originalGroupLinkId);
      CHECK(pastedGroupNode1->linkId() == pastedGroupNode2->linkId());

      const auto* pastedBrushNode1 =
        dynamic_cast<mdl::BrushNode*>(pastedGroupNode1->children().front());
      REQUIRE(pastedBrushNode1);

      CHECK(pastedBrushNode1->linkId() != originalBrushLinkId);

      const auto* pastedBrushNode2 =
        dynamic_cast<mdl::BrushNode*>(pastedGroupNode2->children().front());
      REQUIRE(pastedBrushNode2);

      CHECK(pastedBrushNode2->linkId() != originalBrushLinkId);

      CHECK(pastedBrushNode1->linkId() == pastedBrushNode2->linkId());
    }

    SECTION("If both original groups exist")
    {
      CHECK(document->paste(data) == PasteType::Node);
      CHECK(document->world()->defaultLayer()->childCount() == 4);

      const auto* pastedGroupNode1 =
        dynamic_cast<mdl::GroupNode*>(document->world()->defaultLayer()->children()[2]);
      REQUIRE(pastedGroupNode1);

      const auto* pastedGroupNode2 =
        dynamic_cast<mdl::GroupNode*>(document->world()->defaultLayer()->children()[3]);
      REQUIRE(pastedGroupNode2);

      CHECK(pastedGroupNode1->linkId() == originalGroupLinkId);
      CHECK(pastedGroupNode2->linkId() == originalGroupLinkId);

      const auto* pastedBrushNode1 =
        dynamic_cast<mdl::BrushNode*>(pastedGroupNode1->children().front());
      REQUIRE(pastedBrushNode1);

      CHECK(pastedBrushNode1->linkId() == originalBrushLinkId);

      const auto* pastedBrushNode2 =
        dynamic_cast<mdl::BrushNode*>(pastedGroupNode2->children().front());
      REQUIRE(pastedBrushNode2);

      CHECK(pastedBrushNode2->linkId() == originalBrushLinkId);
    }
  }
}

TEST_CASE_METHOD(MapDocumentTest, "CopyPasteTest.undoRedo")
{
  // https://github.com/TrenchBroom/TrenchBroom/issues/4174

  const auto data = R"(
{
( -0 -0 -16 ) ( -0 -0  -0 ) ( 64 -0 -16 ) tex1 1 2 3 4 5
( -0 -0 -16 ) ( -0 64 -16 ) ( -0 -0  -0 ) tex2 0 0 0 1 1
( -0 -0 -16 ) ( 64 -0 -16 ) ( -0 64 -16 ) tex3 0 0 0 1 1
( 64 64  -0 ) ( -0 64  -0 ) ( 64 64 -16 ) tex4 0 0 0 1 1
( 64 64  -0 ) ( 64 64 -16 ) ( 64 -0  -0 ) tex5 0 0 0 1 1
( 64 64  -0 ) ( 64 -0  -0 ) ( -0 64  -0 ) tex6 0 0 0 1 1
})";

  const auto& worldNode = *document->world();

  const auto& defaultLayerNode = *worldNode.defaultLayer();
  REQUIRE(document->selectedNodes().brushCount() == 0u);
  REQUIRE(defaultLayerNode.childCount() == 0u);

  REQUIRE(document->paste(data) == PasteType::Node);
  REQUIRE(defaultLayerNode.childCount() == 1u);
  REQUIRE(dynamic_cast<mdl::BrushNode*>(defaultLayerNode.children().front()) != nullptr);
  REQUIRE(document->selectedNodes().brushCount() == 1u);

  CHECK(document->canUndoCommand());
  document->undoCommand();
  CHECK(defaultLayerNode.childCount() == 0u);
  CHECK(document->selectedNodes().brushCount() == 0u);

  document->redoCommand();
  CHECK(defaultLayerNode.childCount() == 1u);
  CHECK(dynamic_cast<mdl::BrushNode*>(defaultLayerNode.children().front()) != nullptr);
  CHECK(document->selectedNodes().brushCount() == 1u);
}

} // namespace tb::ui
