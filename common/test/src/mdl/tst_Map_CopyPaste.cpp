/*
 Copyright (C) 2025 Kristian Duske

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

#include "MapFixture.h"
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_CopyPaste.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Nodes.h"
#include "mdl/PasteType.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Map_CopyPaste")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  SECTION("paste")
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

      const auto& worldNode = *map.world();
      REQUIRE_FALSE(worldNode.entity().hasProperty("to_be_ignored"));

      const auto& defaultLayerNode = *worldNode.defaultLayer();
      REQUIRE(defaultLayerNode.childCount() == 0u);
      REQUIRE(worldNode.customLayers().empty());

      CHECK(paste(map, data) == PasteType::Node);
      CHECK_FALSE(worldNode.entity().hasProperty("to_be_ignored"));
      CHECK(worldNode.customLayers().empty());
      CHECK(defaultLayerNode.childCount() == 1u);
      CHECK(dynamic_cast<BrushNode*>(defaultLayerNode.children().front()) != nullptr);
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

      const auto& worldNode = *map.world();
      REQUIRE_FALSE(worldNode.entity().hasProperty("to_be_ignored"));

      const auto& defaultLayerNode = *worldNode.defaultLayer();
      REQUIRE(defaultLayerNode.childCount() == 0u);

      CHECK(paste(map, data) == PasteType::Node);
      CHECK_FALSE(worldNode.entity().hasProperty("to_be_ignored"));
      CHECK(defaultLayerNode.childCount() == 1u);

      const auto* groupNode =
        dynamic_cast<GroupNode*>(defaultLayerNode.children().front());
      CHECK(groupNode != nullptr);
      CHECK(groupNode->group().name() == "My Group");
      CHECK(groupNode->childCount() == 1u);
      CHECK(dynamic_cast<BrushNode*>(groupNode->children().front()) != nullptr);
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

      const auto& worldNode = *map.world();
      REQUIRE_FALSE(worldNode.entity().hasProperty("to_be_ignored"));

      const auto& defaultLayerNode = *worldNode.defaultLayer();
      REQUIRE(defaultLayerNode.childCount() == 0u);

      CHECK(paste(map, data) == PasteType::Node);
      CHECK_FALSE(worldNode.entity().hasProperty("to_be_ignored"));
      CHECK(defaultLayerNode.childCount() == 1u);

      const auto* entityNode =
        dynamic_cast<EntityNode*>(defaultLayerNode.children().front());
      CHECK(entityNode != nullptr);
      CHECK(entityNode->entity().classname() == "func_door");
      CHECK(entityNode->childCount() == 1u);
      CHECK(dynamic_cast<BrushNode*>(entityNode->children().front()) != nullptr);
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

      const auto& worldNode = *map.world();
      REQUIRE_FALSE(worldNode.entity().hasProperty("to_be_ignored"));

      const auto& defaultLayerNode = *worldNode.defaultLayer();
      REQUIRE(defaultLayerNode.childCount() == 0u);

      CHECK(paste(map, data) == PasteType::Node);
      CHECK_FALSE(worldNode.entity().hasProperty("to_be_ignored"));
      CHECK(defaultLayerNode.childCount() == 1u);
      CHECK(dynamic_cast<BrushNode*>(defaultLayerNode.children().front()) != nullptr);
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

      const auto& worldNode = *map.world();

      const auto& defaultLayerNode = *worldNode.defaultLayer();
      REQUIRE(defaultLayerNode.childCount() == 0u);

      CHECK(paste(map, data) == PasteType::Node);
      CHECK(defaultLayerNode.childCount() == 1u);
      CHECK(dynamic_cast<BrushNode*>(defaultLayerNode.children().front()) != nullptr);
    }

    SECTION("Paste single patch")
    {
      fixture.create({.mapFormat = MapFormat::Quake3});

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

      const auto& worldNode = *map.world();

      const auto& defaultLayerNode = *worldNode.defaultLayer();
      REQUIRE(defaultLayerNode.childCount() == 0u);

      REQUIRE(paste(map, data) == PasteType::Node);
      REQUIRE(defaultLayerNode.childCount() == 1u);
      CHECK(dynamic_cast<PatchNode*>(defaultLayerNode.children().front()) != nullptr);
    }

    SECTION("Paste and translate a group")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/2776

      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};
      const auto box = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};

      auto* brushNode1 =
        new BrushNode{builder.createCuboid(box, "material") | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode1}}});
      map.selectNodes({brushNode1});

      const auto groupName = std::string{"testGroup"};

      auto* groupNode = map.groupSelectedNodes(groupName);
      CHECK(groupNode != nullptr);
      map.selectNodes({groupNode});

      const auto copied = serializeSelectedNodes(map);

      const auto delta = vm::vec3d{16, 16, 16};
      CHECK(paste(map, copied) == PasteType::Node);
      CHECK(map.selection().groups.size() == 1u);
      CHECK(map.selection().groups.at(0)->name() == groupName);
      CHECK(translateSelection(map, delta));
      CHECK(map.selectionBounds() == box.translate(delta));
    }

    SECTION("Paste into open group")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/1734

      const auto data = std::string{
        R"({
"classname" "light"
"origin" "0 0 0"
})"};

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});
      map.selectNodes({brushNode});

      auto* groupNode = map.groupSelectedNodes("test");
      map.openGroup(groupNode);

      CHECK(paste(map, data) == PasteType::Node);
      CHECK(map.selection().hasOnlyEntities());
      CHECK(map.selection().entities.size() == 1u);

      auto* light = map.selection().entities.front();
      CHECK(light->parent() == groupNode);
    }

    SECTION("Undo and redo")
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

      const auto& worldNode = *map.world();

      const auto& defaultLayerNode = *worldNode.defaultLayer();
      REQUIRE(map.selection().brushes.size() == 0u);
      REQUIRE(defaultLayerNode.childCount() == 0u);

      REQUIRE(paste(map, data) == PasteType::Node);
      REQUIRE(defaultLayerNode.childCount() == 1u);
      REQUIRE(dynamic_cast<BrushNode*>(defaultLayerNode.children().front()) != nullptr);
      REQUIRE(map.selection().brushes.size() == 1u);

      CHECK(map.canUndoCommand());
      map.undoCommand();
      CHECK(defaultLayerNode.childCount() == 0u);
      CHECK(map.selection().brushes.size() == 0u);

      map.redoCommand();
      CHECK(defaultLayerNode.childCount() == 1u);
      CHECK(dynamic_cast<BrushNode*>(defaultLayerNode.children().front()) != nullptr);
      CHECK(map.selection().brushes.size() == 1u);
    }

    SECTION("Paste resets duplicate group IDs")
    {
      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      map.selectNodes({entityNode});
      auto* groupNode = map.groupSelectedNodes("test");

      const auto persistentGroupId = groupNode->persistentId();
      REQUIRE(persistentGroupId.has_value());

      map.deselectAll();
      map.selectNodes({groupNode});

      const auto str = serializeSelectedNodes(map);

      SECTION("Copy and paste resets persistent group ID")
      {
        map.deselectAll();
        REQUIRE(paste(map, str) == PasteType::Node);

        auto* pastedGroupNode =
          dynamic_cast<GroupNode*>(map.world()->defaultLayer()->children().back());
        REQUIRE(pastedGroupNode != nullptr);
        REQUIRE(pastedGroupNode != groupNode);

        CHECK(pastedGroupNode->persistentId() != persistentGroupId);
      }

      SECTION("Cut and paste retains persistent group ID")
      {
        removeSelectedNodes(map);
        map.deselectAll();
        REQUIRE(paste(map, str) == PasteType::Node);

        auto* pastedGroupNode =
          dynamic_cast<GroupNode*>(map.world()->defaultLayer()->children().back());
        REQUIRE(pastedGroupNode != nullptr);
        REQUIRE(pastedGroupNode != groupNode);

        CHECK(pastedGroupNode->persistentId() == persistentGroupId);
      }
    }

    SECTION("Paste resets duplicate link IDs")
    {
      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});
      map.selectNodes({brushNode});

      auto* groupNode = map.groupSelectedNodes("test");

      map.deselectAll();
      map.selectNodes({groupNode});
      auto* linkedGroup = map.createLinkedDuplicate();

      const auto originalGroupLinkId = linkedGroup->linkId();
      REQUIRE(originalGroupLinkId == groupNode->linkId());

      auto* linkedBrushNode = dynamic_cast<BrushNode*>(linkedGroup->children().front());
      REQUIRE(linkedBrushNode);

      const auto originalBrushLinkId = linkedBrushNode->linkId();
      REQUIRE(originalBrushLinkId == brushNode->linkId());

      map.deselectAll();

      SECTION("Pasting one linked brush")
      {
        map.deselectAll();
        map.openGroup(groupNode);

        map.selectNodes({brushNode});
        const auto data = serializeSelectedNodes(map);

        map.deselectAll();

        CHECK(paste(map, data) == PasteType::Node);
        CHECK(groupNode->childCount() == 2);

        const auto* pastedBrushNode =
          dynamic_cast<BrushNode*>(groupNode->children().back());
        REQUIRE(pastedBrushNode);

        CHECK(pastedBrushNode->linkId() != originalBrushLinkId);
      }

      SECTION("Pasting one linked group")
      {
        map.selectNodes({linkedGroup});
        const auto data = serializeSelectedNodes(map);

        map.deselectAll();

        SECTION("Pasting unknown linked group ID")
        {
          map.selectAllNodes();
          removeSelectedNodes(map);

          CHECK(paste(map, data) == PasteType::Node);
          CHECK(map.world()->defaultLayer()->childCount() == 1);

          const auto* pastedGroupNode =
            dynamic_cast<GroupNode*>(map.world()->defaultLayer()->children().back());
          REQUIRE(pastedGroupNode);

          CHECK(pastedGroupNode->linkId() == originalGroupLinkId);
        }

        SECTION("If only one linked group exists")
        {
          map.selectNodes({linkedGroup});
          removeSelectedNodes(map);

          CHECK(paste(map, data) == PasteType::Node);
          CHECK(map.world()->defaultLayer()->childCount() == 2);

          const auto* pastedGroupNode =
            dynamic_cast<GroupNode*>(map.world()->defaultLayer()->children().back());
          REQUIRE(pastedGroupNode);

          CHECK(pastedGroupNode->linkId() != originalGroupLinkId);

          const auto* pastedBrushNode =
            dynamic_cast<BrushNode*>(pastedGroupNode->children().front());
          REQUIRE(pastedBrushNode);

          CHECK(pastedBrushNode->linkId() != originalBrushLinkId);
        }

        SECTION("If more than one linked group exists")
        {
          CHECK(paste(map, data) == PasteType::Node);
          CHECK(map.world()->defaultLayer()->childCount() == 3);

          const auto* pastedGroupNode =
            dynamic_cast<GroupNode*>(map.world()->defaultLayer()->children().back());
          REQUIRE(pastedGroupNode);

          CHECK(pastedGroupNode->linkId() == originalGroupLinkId);

          const auto* pastedBrushNode =
            dynamic_cast<BrushNode*>(pastedGroupNode->children().front());
          REQUIRE(pastedBrushNode);

          CHECK(pastedBrushNode->linkId() == originalBrushLinkId);
        }

        SECTION("Pasting recursive linked group")
        {
          map.openGroup(groupNode);

          CHECK(paste(map, data) == PasteType::Node);
          CHECK(groupNode->childCount() == 2);
          CHECK(linkedGroup->childCount() == 2);

          auto* pastedGroup = dynamic_cast<GroupNode*>(groupNode->children().back());
          REQUIRE(pastedGroup);

          CHECK(pastedGroup->linkId() != originalGroupLinkId);

          const auto* pastedBrushNode =
            dynamic_cast<BrushNode*>(pastedGroup->children().front());
          REQUIRE(pastedBrushNode);

          CHECK(pastedBrushNode->linkId() != originalBrushLinkId);

          auto* linkedPastedGroupNode =
            dynamic_cast<GroupNode*>(linkedGroup->children().back());
          REQUIRE(linkedPastedGroupNode);

          CHECK(linkedPastedGroupNode->linkId() == pastedGroup->linkId());

          const auto* linkedPastedBrushNode =
            dynamic_cast<BrushNode*>(linkedPastedGroupNode->children().front());
          REQUIRE(pastedBrushNode);

          CHECK(linkedPastedBrushNode->linkId() == pastedBrushNode->linkId());
        }
      }

      SECTION("Pasting two linked groups")
      {
        map.selectNodes({groupNode, linkedGroup});
        const auto data = serializeSelectedNodes(map);

        map.deselectAll();

        SECTION("If only one original group exists")
        {
          map.selectNodes({linkedGroup});
          removeSelectedNodes(map);

          CHECK(paste(map, data) == PasteType::Node);
          CHECK(map.world()->defaultLayer()->childCount() == 3);

          const auto* pastedGroupNode1 =
            dynamic_cast<GroupNode*>(map.world()->defaultLayer()->children()[1]);
          REQUIRE(pastedGroupNode1);

          const auto* pastedGroupNode2 =
            dynamic_cast<GroupNode*>(map.world()->defaultLayer()->children()[2]);
          REQUIRE(pastedGroupNode2);

          CHECK(pastedGroupNode1->linkId() != originalGroupLinkId);
          CHECK(pastedGroupNode2->linkId() != originalGroupLinkId);
          CHECK(pastedGroupNode1->linkId() == pastedGroupNode2->linkId());

          const auto* pastedBrushNode1 =
            dynamic_cast<BrushNode*>(pastedGroupNode1->children().front());
          REQUIRE(pastedBrushNode1);

          CHECK(pastedBrushNode1->linkId() != originalBrushLinkId);

          const auto* pastedBrushNode2 =
            dynamic_cast<BrushNode*>(pastedGroupNode2->children().front());
          REQUIRE(pastedBrushNode2);

          CHECK(pastedBrushNode2->linkId() != originalBrushLinkId);

          CHECK(pastedBrushNode1->linkId() == pastedBrushNode2->linkId());
        }

        SECTION("If both original groups exist")
        {
          CHECK(paste(map, data) == PasteType::Node);
          CHECK(map.world()->defaultLayer()->childCount() == 4);

          const auto* pastedGroupNode1 =
            dynamic_cast<GroupNode*>(map.world()->defaultLayer()->children()[2]);
          REQUIRE(pastedGroupNode1);

          const auto* pastedGroupNode2 =
            dynamic_cast<GroupNode*>(map.world()->defaultLayer()->children()[3]);
          REQUIRE(pastedGroupNode2);

          CHECK(pastedGroupNode1->linkId() == originalGroupLinkId);
          CHECK(pastedGroupNode2->linkId() == originalGroupLinkId);

          const auto* pastedBrushNode1 =
            dynamic_cast<BrushNode*>(pastedGroupNode1->children().front());
          REQUIRE(pastedBrushNode1);

          CHECK(pastedBrushNode1->linkId() == originalBrushLinkId);

          const auto* pastedBrushNode2 =
            dynamic_cast<BrushNode*>(pastedGroupNode2->children().front());
          REQUIRE(pastedBrushNode2);

          CHECK(pastedBrushNode2->linkId() == originalBrushLinkId);
        }
      }
    }
  }
}

} // namespace tb::mdl
