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
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Layers.h"
#include "mdl/Map_NodeLocking.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/MaterialManager.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"

#include "kd/overload.h"

#include "catch/CatchConfig.h"
#include "catch/Matchers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

TEST_CASE("Map_Nodes")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  map.entityDefinitionManager().setDefinitions({
    {"point_entity",
     Color{},
     "this is a point entity",
     {},
     PointEntityDefinition{vm::bbox3d{16.0}, {}, {}}},
  });

  const auto& pointEntityDefinition = map.entityDefinitionManager().definitions().front();

  SECTION("parentForNodes")
  {
    auto* customLayerNode = new LayerNode{Layer{"custom layer"}};
    auto* groupNode = new GroupNode{Group{"group"}};
    auto* groupedEntityNode = new EntityNode{Entity{}};
    auto* defaultLayerEntityNode = new EntityNode{Entity{}};

    addNodes(map, {{map.world(), {customLayerNode}}});
    addNodes(map, {{customLayerNode, {groupNode}}});
    addNodes(map, {{groupNode, {groupedEntityNode}}});
    addNodes(map, {{map.world()->defaultLayer(), {defaultLayerEntityNode}}});

    SECTION("Returns default layer if no group is open")
    {
      CHECK(parentForNodes(map) == map.world()->defaultLayer());
    }

    SECTION("Returns currently opened group, if any")
    {
      openGroup(map, *groupNode);
      CHECK(parentForNodes(map) == groupNode);
    }

    SECTION("Returns parent of first node in given vector")
    {
      CHECK(parentForNodes(map, {groupedEntityNode}) == groupNode);
      CHECK(parentForNodes(map, {groupNode}) == customLayerNode);
    }
  }

  SECTION("addNodes")
  {
    SECTION("Nodes added to a hidden layer are visible")
    {
      auto* layerNode1 = new LayerNode{Layer{"test1"}};
      auto* layerNode2 = new LayerNode{Layer{"test2"}};
      addNodes(map, {{map.world(), {layerNode1}}});
      addNodes(map, {{map.world(), {layerNode2}}});

      setCurrentLayer(map, layerNode1);

      // Create an entity in layer1
      auto* entityNode1 = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode1}}});

      REQUIRE(entityNode1->parent() == layerNode1);

      CHECK(entityNode1->visibilityState() == VisibilityState::Inherited);
      CHECK(entityNode1->visible());

      // Hide layer1. If any nodes in the layer were Visibility_Shown they would be reset
      // to Visibility_Inherited
      hideLayers(map, {layerNode1});

      REQUIRE(entityNode1->visibilityState() == VisibilityState::Inherited);
      REQUIRE(!entityNode1->visible());

      // Create another entity in layer1. It will be visible, while entity1 will still be
      // hidden.
      auto* entityNode2 = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode2}}});

      REQUIRE(entityNode2->parent() == layerNode1);

      CHECK(entityNode1->visibilityState() == VisibilityState::Inherited);
      CHECK(!entityNode1->visible());
      CHECK(entityNode2->visibilityState() == VisibilityState::Shown);
      CHECK(entityNode2->visible());
    }

    SECTION("Nodes added to a locked layer are unlocked")
    {
      auto* layerNode1 = new LayerNode{Layer{"test1"}};
      auto* layerNode2 = new LayerNode{Layer{"test2"}};
      addNodes(map, {{map.world(), {layerNode1}}});
      addNodes(map, {{map.world(), {layerNode2}}});

      setCurrentLayer(map, layerNode1);

      // Create an entity in layer1
      auto* entityNode1 = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode1}}});

      REQUIRE(entityNode1->parent() == layerNode1);

      CHECK(entityNode1->lockState() == LockState::Inherited);
      CHECK(!entityNode1->locked());

      lockNodes(map, {layerNode1});

      REQUIRE(entityNode1->lockState() == LockState::Inherited);
      REQUIRE(entityNode1->locked());

      auto* entityNode2 = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode2}}});

      REQUIRE(entityNode2->parent() == layerNode1);

      CHECK(entityNode1->lockState() == LockState::Inherited);
      CHECK(entityNode1->locked());
      CHECK(entityNode2->lockState() == LockState::Unlocked);
      CHECK(!entityNode2->locked());
    }

    SECTION("Linked groups")
    {
      SECTION("Child nodes are added to linked groups")
      {
        auto* groupNode = new GroupNode{Group{"test"}};
        auto* brushNode = createBrushNode(map);
        groupNode->addChild(brushNode);
        addNodes(map, {{parentForNodes(map), {groupNode}}});

        selectNodes(map, {groupNode});
        auto* linkedGroupNode = createLinkedDuplicate(map);
        deselectAll(map);

        using CreateNode = std::function<Node*(const Map&)>;
        CreateNode createNode = GENERATE_COPY(
          CreateNode{[](const auto&) -> Node* { return new EntityNode{Entity{}}; }},
          CreateNode{[](const auto& m) -> Node* { return createBrushNode(m); }},
          CreateNode{[](const auto&) -> Node* { return createPatchNode(); }});

        auto* nodeToAdd = createNode(map);
        addNodes(map, {{groupNode, {nodeToAdd}}});

        CHECK(linkedGroupNode->childCount() == 2u);

        auto* linkedNode = linkedGroupNode->children().back();
        linkedNode->accept(kdl::overload(
          [](const WorldNode*) {},
          [](const LayerNode*) {},
          [](const GroupNode*) {},
          [&](const EntityNode* linkedEntityNode) {
            const auto* originalEntityNode = dynamic_cast<EntityNode*>(nodeToAdd);
            REQUIRE(originalEntityNode);
            CHECK(originalEntityNode->entity() == linkedEntityNode->entity());
          },
          [&](const BrushNode* linkedBrushNode) {
            const auto* originalBrushNode = dynamic_cast<BrushNode*>(nodeToAdd);
            REQUIRE(originalBrushNode);
            CHECK(originalBrushNode->brush() == linkedBrushNode->brush());
          },
          [&](const PatchNode* linkedPatchNode) {
            const auto* originalPatchNode = dynamic_cast<PatchNode*>(nodeToAdd);
            REQUIRE(originalPatchNode);
            CHECK(originalPatchNode->patch() == linkedPatchNode->patch());
          }));

        map.undoCommand();
        REQUIRE(groupNode->childCount() == 1u);
        CHECK(linkedGroupNode->childCount() == 1u);

        map.redoCommand();

        REQUIRE(groupNode->childCount() == 2u);
        CHECK(linkedGroupNode->childCount() == 2u);
      }

      SECTION("Linked nodes inherit the group's transformation when they are added")
      {
        auto* groupNode = new GroupNode{Group{"group"}};
        addNodes(map, {{parentForNodes(map), {groupNode}}});

        selectNodes(map, {groupNode});
        auto* linkedGroupNode = createLinkedDuplicate(map);
        deselectAll(map);

        selectNodes(map, {linkedGroupNode});
        translateSelection(map, vm::vec3d{32, 0, 0});
        deselectAll(map);

        auto* brushNode = createBrushNode(map);
        addNodes(map, {{groupNode, {brushNode}}});

        REQUIRE(groupNode->childCount() == 1u);
        REQUIRE(linkedGroupNode->childCount() == 1u);

        auto* linkedBrushNode =
          dynamic_cast<BrushNode*>(linkedGroupNode->children().front());
        REQUIRE(linkedBrushNode != nullptr);

        CHECK(
          linkedBrushNode->physicalBounds()
          == brushNode->physicalBounds().transform(
            linkedGroupNode->group().transformation()));

        map.undoCommand();
        REQUIRE(groupNode->childCount() == 0u);
        REQUIRE(linkedGroupNode->childCount() == 0u);

        map.redoCommand();
        REQUIRE(groupNode->childCount() == 1u);
        REQUIRE(linkedGroupNode->childCount() == 1u);
        CHECK(
          linkedBrushNode->physicalBounds()
          == brushNode->physicalBounds().transform(
            linkedGroupNode->group().transformation()));
      }

      SECTION("Child cannot be added because adding it to a linked group fails")
      {
        auto* groupNode = new GroupNode{Group{"group"}};
        addNodes(map, {{parentForNodes(map), {groupNode}}});

        selectNodes(map, {groupNode});
        auto* linkedGroupNode = createLinkedDuplicate(map);
        deselectAll(map);

        // adding a brush to the linked group node will fail because it will go out of
        // world bounds
        selectNodes(map, {linkedGroupNode});
        translateSelection(map, map.worldBounds().max);
        deselectAll(map);

        auto* brushNode = createBrushNode(map);
        CHECK(addNodes(map, {{groupNode, {brushNode}}}).empty());

        CHECK(groupNode->childCount() == 0u);
        CHECK(linkedGroupNode->childCount() == 0u);
      }
    }
  }

  SECTION("duplicateSelectedNodes")
  {
    SECTION("Duplicated nodes are added to the source layer")
    {
      auto* layerNode1 = new LayerNode{Layer{"test1"}};
      auto* layerNode2 = new LayerNode{Layer{"test2"}};
      addNodes(map, {{map.world(), {layerNode1}}});
      addNodes(map, {{map.world(), {layerNode2}}});

      setCurrentLayer(map, layerNode1);
      auto* entityNode = createPointEntity(map, pointEntityDefinition, {0, 0, 0});
      CHECK(entityNode->parent() == layerNode1);
      CHECK(layerNode1->childCount() == 1);

      setCurrentLayer(map, layerNode2);
      selectNodes(map, {entityNode});
      duplicateSelectedNodes(map);

      REQUIRE(map.selection().entities.size() == 1);

      auto* entityClone = map.selection().entities.at(0);
      CHECK(entityClone->parent() == layerNode1);
      CHECK(layerNode1->childCount() == 2);
      CHECK(map.editorContext().currentLayer() == layerNode2);
    }

    SECTION("Nodes duplicated in a hidden layer become visible")
    {
      auto* layerNode1 = new LayerNode{Layer{"test1"}};
      addNodes(map, {{map.world(), {layerNode1}}});

      setCurrentLayer(map, layerNode1);
      hideLayers(map, {layerNode1});

      // Create entity1 and brush1 in the hidden layer1
      auto* entityNode1 = new EntityNode{Entity{}};
      auto* brushNode1 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {entityNode1, brushNode1}}});

      REQUIRE(entityNode1->parent() == layerNode1);
      REQUIRE(brushNode1->parent() == layerNode1);
      REQUIRE(layerNode1->childCount() == 2u);

      REQUIRE(entityNode1->visibilityState() == VisibilityState::Shown);
      REQUIRE(brushNode1->visibilityState() == VisibilityState::Shown);
      REQUIRE(entityNode1->visible());
      REQUIRE(brushNode1->visible());

      selectNodes(map, {entityNode1, brushNode1});

      // Duplicate entity1 and brush1
      duplicateSelectedNodes(map);
      REQUIRE(map.selection().entities.size() == 1u);
      REQUIRE(map.selection().brushes.size() == 1u);
      auto* entityNode2 = map.selection().entities.front();
      auto* brushNode2 = map.selection().brushes.front();

      REQUIRE(entityNode2 != entityNode1);
      REQUIRE(brushNode2 != brushNode1);

      CHECK(entityNode2->visibilityState() == VisibilityState::Shown);
      CHECK(entityNode2->visible());

      CHECK(brushNode2->visibilityState() == VisibilityState::Shown);
      CHECK(brushNode2->visible());
    }
  }

  SECTION("reparentNodes")
  {
    SECTION("Cannot reparent layer to layer")
    {
      auto* layer1 = new LayerNode{Layer{"Layer 1"}};
      addNodes(map, {{map.world(), {layer1}}});

      auto* layer2 = new LayerNode{Layer{"Layer 2"}};
      addNodes(map, {{map.world(), {layer2}}});

      CHECK_FALSE(reparentNodes(map, {{layer2, {layer1}}}));
    }

    SECTION("Reparent between layers")
    {
      auto* oldParent = new LayerNode{Layer{"Layer 1"}};
      addNodes(map, {{map.world(), {oldParent}}});

      auto* newParent = new LayerNode{Layer{"Layer 2"}};
      addNodes(map, {{map.world(), {newParent}}});

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{oldParent, {entityNode}}});

      assert(entityNode->parent() == oldParent);
      CHECK(reparentNodes(map, {{newParent, {entityNode}}}));
      CHECK(entityNode->parent() == newParent);

      map.undoCommand();
      CHECK(entityNode->parent() == oldParent);
    }

    SECTION("Cannot reparent a group to itself")
    {
      auto* group = new GroupNode{Group{"Group"}};
      addNodes(map, {{parentForNodes(map), {group}}});

      CHECK_FALSE(reparentNodes(map, {{group, {group}}}));
    }

    SECTION("Cannot reparent a group to its descendants")
    {
      auto* outer = new GroupNode{Group{"Outer"}};
      addNodes(map, {{parentForNodes(map), {outer}}});

      auto* inner = new GroupNode{Group{"Inner"}};
      addNodes(map, {{outer, {inner}}});

      CHECK_FALSE(reparentNodes(map, {{inner, {outer}}}));
    }

    SECTION("Empty groups are removed after reparenting")
    {
      auto* group = new GroupNode{Group{"Group"}};
      addNodes(map, {{parentForNodes(map), {group}}});

      auto* entity = new EntityNode{Entity{}};
      addNodes(map, {{group, {entity}}});

      CHECK(reparentNodes(map, {{parentForNodes(map), {entity}}}));
      CHECK(entity->parent() == parentForNodes(map));
      CHECK(group->parent() == nullptr);

      map.undoCommand();
      CHECK(group->parent() == parentForNodes(map));
      CHECK(entity->parent() == group);
    }

    SECTION("Empty groups are removed recursively after reparenting")
    {
      auto* outer = new GroupNode{Group{"Outer"}};
      addNodes(map, {{parentForNodes(map), {outer}}});

      auto* inner = new GroupNode{Group{"Inner"}};
      addNodes(map, {{outer, {inner}}});

      auto* entity = new EntityNode{Entity{}};
      addNodes(map, {{inner, {entity}}});

      CHECK(reparentNodes(map, {{parentForNodes(map), {entity}}}));
      CHECK(entity->parent() == parentForNodes(map));
      CHECK(inner->parent() == nullptr);
      CHECK(outer->parent() == nullptr);

      map.undoCommand();
      CHECK(outer->parent() == parentForNodes(map));
      CHECK(inner->parent() == outer);
      CHECK(entity->parent() == inner);
    }

    SECTION("Empty entities are removed after reparenting")
    {
      auto* entity = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entity}}});

      auto* brush = createBrushNode(map);
      addNodes(map, {{entity, {brush}}});

      CHECK(reparentNodes(map, {{parentForNodes(map), {brush}}}));
      CHECK(brush->parent() == parentForNodes(map));
      CHECK(entity->parent() == nullptr);

      map.undoCommand();
      CHECK(entity->parent() == parentForNodes(map));
      CHECK(brush->parent() == entity);
    }

    SECTION("Empty groups and entities are removed after reparenting")
    {
      auto* group = new GroupNode{Group{"Group"}};
      addNodes(map, {{parentForNodes(map), {group}}});

      auto* entity = new EntityNode{Entity{}};
      addNodes(map, {{group, {entity}}});

      auto* brush = createBrushNode(map);
      addNodes(map, {{entity, {brush}}});

      CHECK(reparentNodes(map, {{parentForNodes(map), {brush}}}));
      CHECK(brush->parent() == parentForNodes(map));
      CHECK(group->parent() == nullptr);
      CHECK(entity->parent() == nullptr);

      map.undoCommand();
      CHECK(group->parent() == parentForNodes(map));
      CHECK(entity->parent() == group);
      CHECK(brush->parent() == entity);
    }

    SECTION("Resetting link IDs")
    {
      auto* nestedBrushNode = createBrushNode(map);
      auto* nestedEntityNode = new EntityNode{Entity{}};

      addNodes(map, {{parentForNodes(map), {nestedBrushNode, nestedEntityNode}}});
      selectNodes(map, {nestedBrushNode, nestedEntityNode});

      auto* nestedGroupNode = groupSelectedNodes(map, "nested");

      deselectAll(map);
      selectNodes(map, {nestedGroupNode});

      auto* linkedNestedGroupNode = createLinkedDuplicate(map);

      auto* brushNode = createBrushNode(map);
      auto* entityNode = new EntityNode{Entity{}};
      auto* entityBrushNode = createBrushNode(map);
      entityNode->addChild(entityBrushNode);

      addNodes(map, {{parentForNodes(map), {brushNode, entityNode}}});

      selectNodes(map, {brushNode, entityNode, nestedGroupNode});
      auto* groupNode = groupSelectedNodes(map, "group");

      deselectAll(map);
      selectNodes(map, {groupNode});

      auto* linkedGroupNode = createLinkedDuplicate(map);
      auto* linkedGroupNode2 = createLinkedDuplicate(map);

      deselectAll(map);

      const auto originalNestedBrushLinkId = nestedBrushNode->linkId();
      const auto originalBrushLinkId = brushNode->linkId();
      const auto originalEntityLinkId = entityNode->linkId();
      const auto originalEntityBrushLinkId = entityBrushNode->linkId();

      REQUIRE_THAT(*linkedNestedGroupNode, MatchesNode(*nestedGroupNode));
      REQUIRE_THAT(*linkedGroupNode, MatchesNode(*groupNode));
      REQUIRE_THAT(*linkedGroupNode2, MatchesNode(*groupNode));

      SECTION("Moving a brush entity to the world resets its link IDs")
      {
        REQUIRE(reparentNodes(map, {{parentForNodes(map), {entityNode}}}));

        CHECK(entityNode->linkId() != originalEntityLinkId);
        CHECK(entityBrushNode->linkId() != originalEntityBrushLinkId);

        CHECK_THAT(*linkedNestedGroupNode, MatchesNode(*nestedGroupNode));
        CHECK_THAT(*linkedGroupNode, MatchesNode(*groupNode));
        CHECK_THAT(*linkedGroupNode2, MatchesNode(*groupNode));
      }

      SECTION(
        "Moving objects out of a nested group into the container resets their link IDs")
      {
        REQUIRE(reparentNodes(map, {{groupNode, {nestedBrushNode}}}));
        CHECK(nestedBrushNode->linkId() != originalNestedBrushLinkId);

        CHECK_THAT(*linkedNestedGroupNode, MatchesNode(*nestedGroupNode));
        CHECK_THAT(*linkedGroupNode, MatchesNode(*groupNode));
        CHECK_THAT(*linkedGroupNode2, MatchesNode(*groupNode));
      }

      SECTION("Moving objects into a nested linked group keeps their link IDs")
      {
        REQUIRE(reparentNodes(map, {{nestedGroupNode, {brushNode}}}));
        CHECK(brushNode->linkId() == originalBrushLinkId);

        CHECK_THAT(*linkedNestedGroupNode, MatchesNode(*nestedGroupNode));
        CHECK_THAT(*linkedGroupNode, MatchesNode(*groupNode));
        CHECK_THAT(*linkedGroupNode2, MatchesNode(*groupNode));
      }

      SECTION("Grouping objects within a linked group keeps their link IDs")
      {
        selectNodes(map, {entityNode});
        groupSelectedNodes(map, "new group");
        CHECK(entityNode->linkId() == originalEntityLinkId);
        CHECK(entityBrushNode->linkId() == originalEntityBrushLinkId);

        CHECK_THAT(*linkedNestedGroupNode, MatchesNode(*nestedGroupNode));
        CHECK_THAT(*linkedGroupNode, MatchesNode(*groupNode));
        CHECK_THAT(*linkedGroupNode2, MatchesNode(*groupNode));
      }
    }

    SECTION("Linked groups")
    {
      auto* groupNode = new GroupNode{Group{"group"}};
      auto* brushNode = createBrushNode(map);
      groupNode->addChild(brushNode);
      addNodes(map, {{parentForNodes(map), {groupNode}}});

      selectNodes(map, {groupNode});
      auto* linkedGroupNode = createLinkedDuplicate(map);
      deselectAll(map);

      selectNodes(map, {linkedGroupNode});
      translateSelection(map, vm::vec3d{32, 0, 0});
      deselectAll(map);

      SECTION("Move node into group node")
      {
        auto* entityNode = new EntityNode{Entity{}};
        addNodes(map, {{parentForNodes(map), {entityNode}}});

        REQUIRE(groupNode->childCount() == 1u);
        REQUIRE(linkedGroupNode->childCount() == 1u);

        reparentNodes(map, {{groupNode, {entityNode}}});

        CHECK(groupNode->childCount() == 2u);
        CHECK(linkedGroupNode->childCount() == 2u);

        auto* linkedEntityNode =
          dynamic_cast<EntityNode*>(linkedGroupNode->children().back());
        CHECK(linkedEntityNode != nullptr);

        CHECK(
          linkedEntityNode->physicalBounds()
          == entityNode->physicalBounds().transform(
            linkedGroupNode->group().transformation()));

        map.undoCommand();

        CHECK(entityNode->parent() == parentForNodes(map));
        CHECK(groupNode->childCount() == 1u);
        CHECK(linkedGroupNode->childCount() == 1u);
      }

      SECTION("Move node out of group node")
      {
        auto* entityNode = new EntityNode{Entity{}};
        addNodes(map, {{groupNode, {entityNode}}});

        REQUIRE(groupNode->childCount() == 2u);
        REQUIRE(linkedGroupNode->childCount() == 2u);

        reparentNodes(map, {{parentForNodes(map), {entityNode}}});

        CHECK(entityNode->parent() == parentForNodes(map));
        CHECK(groupNode->childCount() == 1u);
        CHECK(linkedGroupNode->childCount() == 1u);

        map.undoCommand();

        CHECK(entityNode->parent() == groupNode);
        CHECK(groupNode->childCount() == 2u);
        CHECK(linkedGroupNode->childCount() == 2u);
      }
    }

    SECTION("Nested linked groups")
    {
      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});
      selectNodes(map, {brushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      deselectAll(map);
      selectNodes(map, {groupNode});
      auto* linkedGroupNode = createLinkedDuplicate(map);
      deselectAll(map);

      REQUIRE_THAT(*linkedGroupNode, MatchesNode(*groupNode));

      SECTION("Adding a linked group to its linked sibling does nothing")
      {
        CHECK_FALSE(reparentNodes(map, {{groupNode, {linkedGroupNode}}}));
      }

      SECTION(
        "Adding a group containing a nested linked sibling to a linked group does "
        "nothing")
      {
        selectNodes(map, {linkedGroupNode});

        auto* outerGroupNode = groupSelectedNodes(map, "outer");
        REQUIRE(outerGroupNode != nullptr);

        deselectAll(map);
        CHECK_FALSE(reparentNodes(map, {{groupNode, {outerGroupNode}}}));
      }
    }

    SECTION("Update linked groups after recursive deletion")
    {
      auto* outerGroupNode = new GroupNode{Group{"outer"}};
      addNodes(map, {{parentForNodes(map), {outerGroupNode}}});

      openGroup(map, *outerGroupNode);

      auto* outerEntityNode = new EntityNode{Entity{}};
      auto* innerGroupNode = new GroupNode{Group{"inner"}};
      addNodes(map, {{parentForNodes(map), {outerEntityNode, innerGroupNode}}});

      openGroup(map, *innerGroupNode);

      auto* innerEntityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {innerEntityNode}}});

      closeGroup(map);
      closeGroup(map);

      selectNodes(map, {outerGroupNode});

      auto* linkedOuterGroupNode = createLinkedDuplicate(map);
      REQUIRE(
        outerGroupNode->children()
        == std::vector<Node*>{outerEntityNode, innerGroupNode});
      REQUIRE_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));

      deselectAll(map);

      reparentNodes(map, {{parentForNodes(map), {innerEntityNode}}});
      CHECK(outerGroupNode->children() == std::vector<Node*>{outerEntityNode});
      CHECK_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));

      map.undoCommand();
      CHECK(
        outerGroupNode->children()
        == std::vector<Node*>{outerEntityNode, innerGroupNode});
      REQUIRE_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));

      map.redoCommand();
      CHECK(outerGroupNode->children() == std::vector<Node*>{outerEntityNode});
      CHECK_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));
    }

    SECTION("Linked group update fails")
    {
      auto* groupNode = new GroupNode{Group{"group"}};
      addNodes(map, {{parentForNodes(map), {groupNode}}});

      selectNodes(map, {groupNode});
      auto* linkedGroupNode = createLinkedDuplicate(map);
      deselectAll(map);

      // adding a brush to the linked group node will fail because it will go out of world
      // bounds
      selectNodes(map, {linkedGroupNode});
      translateSelection(map, map.worldBounds().max);
      deselectAll(map);

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      CHECK_FALSE(reparentNodes(map, {{groupNode, {brushNode}}}));

      CHECK(groupNode->childCount() == 0u);
      CHECK(linkedGroupNode->childCount() == 0u);
    }

    SECTION("Cannot reparent between linked groups")
    {
      auto* groupNode = new GroupNode{Group{"group"}};
      auto* brushNode = createBrushNode(map);
      groupNode->addChild(brushNode);

      addNodes(map, {{parentForNodes(map), {groupNode}}});

      selectNodes(map, {groupNode});
      auto* linkedGroupNode = createLinkedDuplicate(map);
      deselectAll(map);

      CHECK_FALSE(reparentNodes(map, {{linkedGroupNode, {brushNode}}}));

      CHECK(groupNode->childCount() == 1u);
      CHECK(linkedGroupNode->childCount() == 1u);
    }
  }

  SECTION("removeNodes")
  {
    SECTION("Remove layer")
    {
      auto* layer = new LayerNode{Layer{"Layer 1"}};
      addNodes(map, {{map.world(), {layer}}});

      removeNodes(map, {layer});
      CHECK(layer->parent() == nullptr);

      map.undoCommand();
      CHECK(layer->parent() == map.world());
    }

    SECTION("Remove empty group")
    {
      auto* group = new GroupNode{Group{"group"}};
      addNodes(map, {{parentForNodes(map), {group}}});

      openGroup(map, *group);

      auto* brush = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brush}}});

      removeNodes(map, {brush});
      CHECK(map.editorContext().currentGroup() == nullptr);
      CHECK(brush->parent() == nullptr);
      CHECK(group->parent() == nullptr);

      map.undoCommand();
      CHECK(map.editorContext().currentGroup() == group);
      CHECK(brush->parent() == group);
      CHECK(group->parent() == map.world()->defaultLayer());
    }

    SECTION("Recursively remove empty groups")
    {
      auto* outer = new GroupNode{Group{"outer"}};
      addNodes(map, {{parentForNodes(map), {outer}}});

      openGroup(map, *outer);

      auto* inner = new GroupNode{Group{"inner"}};
      addNodes(map, {{parentForNodes(map), {inner}}});

      openGroup(map, *inner);

      auto* brush = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brush}}});

      removeNodes(map, {brush});
      CHECK(map.editorContext().currentGroup() == nullptr);
      CHECK(brush->parent() == nullptr);
      CHECK(inner->parent() == nullptr);
      CHECK(outer->parent() == nullptr);

      map.undoCommand();
      CHECK(map.editorContext().currentGroup() == inner);
      CHECK(brush->parent() == inner);
      CHECK(inner->parent() == outer);
      CHECK(outer->parent() == map.world()->defaultLayer());
    }

    SECTION("Remove empty brush entitiy")
    {
      auto* layer = new LayerNode{Layer{"Layer 1"}};
      addNodes(map, {{map.world(), {layer}}});

      auto* entity = new EntityNode{Entity{}};
      addNodes(map, {{layer, {entity}}});

      auto* brush = createBrushNode(map);
      addNodes(map, {{entity, {brush}}});

      removeNodes(map, {brush});
      CHECK(brush->parent() == nullptr);
      CHECK(entity->parent() == nullptr);

      map.undoCommand();
      CHECK(brush->parent() == entity);
      CHECK(entity->parent() == layer);
    }

    SECTION("Update linked groups")
    {
      auto* groupNode = new GroupNode{Group{"test"}};
      auto* brushNode = createBrushNode(map);

      using CreateNode = std::function<Node*(const Map&)>;
      CreateNode createNode = GENERATE_COPY(
        CreateNode{[](const auto&) -> Node* { return new EntityNode{Entity{}}; }},
        CreateNode{[](const auto& m) -> Node* { return createBrushNode(m); }},
        CreateNode{[](const auto&) -> Node* { return createPatchNode(); }});

      auto* nodeToRemove = createNode(map);
      groupNode->addChildren({brushNode, nodeToRemove});
      addNodes(map, {{parentForNodes(map), {groupNode}}});

      selectNodes(map, {groupNode});
      auto* linkedGroupNode = createLinkedDuplicate(map);
      deselectAll(map);

      removeNodes(map, {nodeToRemove});

      CHECK(linkedGroupNode->childCount() == 1u);

      map.undoCommand();

      REQUIRE(groupNode->childCount() == 2u);
      CHECK(linkedGroupNode->childCount() == 2u);
    }

    SECTION("Update linked groups with recursion")
    {
      auto* outerGroupNode = new GroupNode{Group{"outer"}};
      addNodes(map, {{parentForNodes(map), {outerGroupNode}}});

      openGroup(map, *outerGroupNode);

      auto* outerEntityNode = new EntityNode{Entity{}};
      auto* innerGroupNode = new GroupNode{Group{"inner"}};
      addNodes(map, {{parentForNodes(map), {outerEntityNode, innerGroupNode}}});

      openGroup(map, *innerGroupNode);

      auto* innerEntityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {innerEntityNode}}});

      closeGroup(map);
      closeGroup(map);

      selectNodes(map, {outerGroupNode});

      auto* linkedOuterGroupNode = createLinkedDuplicate(map);
      deselectAll(map);

      REQUIRE(
        outerGroupNode->children()
        == std::vector<Node*>{outerEntityNode, innerGroupNode});
      REQUIRE_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));

      removeNodes(map, {innerEntityNode});
      REQUIRE(outerGroupNode->children() == std::vector<Node*>{outerEntityNode});
      CHECK_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));

      map.undoCommand();
      REQUIRE(
        outerGroupNode->children()
        == std::vector<Node*>{outerEntityNode, innerGroupNode});
      CHECK_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));

      map.redoCommand();
      REQUIRE(outerGroupNode->children() == std::vector<Node*>{outerEntityNode});
      CHECK_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));
    }
  }

  SECTION("removeSelectedNodes")
  {
    auto* entityNode = new EntityNode{Entity{}};
    addNodes(map, {{parentForNodes(map), {entityNode}}});
    selectNodes(map, {entityNode});

    removeSelectedNodes(map);
    CHECK(map.selection().nodes == std::vector<Node*>{});
    CHECK(map.world()->defaultLayer()->children() == std::vector<Node*>{});
  }

  SECTION("updateNodeContents")
  {
    SECTION("Update brushes")
    {
      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      const auto originalBrush = brushNode->brush();
      auto modifiedBrush = originalBrush;
      REQUIRE(modifiedBrush.transform(
        map.worldBounds(), vm::translation_matrix(vm::vec3d(16, 0, 0)), false));

      auto nodesToSwap = std::vector<std::pair<Node*, NodeContents>>{};
      nodesToSwap.emplace_back(brushNode, modifiedBrush);

      updateNodeContents(map, "Update Nodes", std::move(nodesToSwap), {});
      CHECK(brushNode->brush() == modifiedBrush);

      map.undoCommand();
      CHECK(brushNode->brush() == originalBrush);
    }

    SECTION("Update patches")
    {
      auto* patchNode = createPatchNode();
      addNodes(map, {{parentForNodes(map), {patchNode}}});

      const auto originalPatch = patchNode->patch();
      auto modifiedPatch = originalPatch;
      modifiedPatch.transform(vm::translation_matrix(vm::vec3d{16, 0, 0}));

      auto nodesToSwap = std::vector<std::pair<Node*, NodeContents>>{};
      nodesToSwap.emplace_back(patchNode, modifiedPatch);

      updateNodeContents(map, "Update Nodes", std::move(nodesToSwap), {});
      CHECK(patchNode->patch() == modifiedPatch);

      map.undoCommand();
      CHECK(patchNode->patch() == originalPatch);
    }

    SECTION("Update material usage counts")
    {
      deselectAll(map);
      setEntityProperty(map, EntityPropertyKeys::Wad, "fixture/test/io/Wad/cr8_czg.wad");

      constexpr auto MaterialName = "bongs2";
      const auto* material = map.materialManager().material(MaterialName);
      REQUIRE(material != nullptr);

      auto* brushNode = createBrushNode(map, MaterialName);
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      const auto& originalBrush = brushNode->brush();
      auto modifiedBrush = originalBrush;
      REQUIRE(modifiedBrush.transform(
        map.worldBounds(), vm::translation_matrix(vm::vec3d(16, 0, 0)), false));

      auto nodesToSwap = std::vector<std::pair<Node*, NodeContents>>{};
      nodesToSwap.emplace_back(brushNode, std::move(modifiedBrush));

      REQUIRE(material->usageCount() == 6u);

      updateNodeContents(map, "Update Nodes", std::move(nodesToSwap), {});
      CHECK(material->usageCount() == 6u);

      map.undoCommand();
      CHECK(material->usageCount() == 6u);
    }

    SECTION("Update entity definition usage counts")
    {
      constexpr auto Classname = "point_entity";

      auto* entityNode = new EntityNode{Entity{{
        {EntityPropertyKeys::Classname, Classname},
      }}};

      addNodes(map, {{parentForNodes(map), {entityNode}}});

      const auto& originalEntity = entityNode->entity();
      auto modifiedEntity = originalEntity;
      modifiedEntity.addOrUpdateProperty("this", "that");

      auto nodesToSwap = std::vector<std::pair<Node*, NodeContents>>{};
      nodesToSwap.emplace_back(entityNode, std::move(modifiedEntity));

      REQUIRE(pointEntityDefinition.usageCount() == 1u);

      updateNodeContents(map, "Update Nodes", std::move(nodesToSwap), {});
      CHECK(pointEntityDefinition.usageCount() == 1u);

      map.undoCommand();
      CHECK(pointEntityDefinition.usageCount() == 1u);
    }

    SECTION("Update linked groups")
    {
      auto* groupNode = new GroupNode{Group{"group"}};
      auto* brushNode = createBrushNode(map);
      groupNode->addChild(brushNode);
      addNodes(map, {{parentForNodes(map), {groupNode}}});

      selectNodes(map, {groupNode});
      auto* linkedGroupNode = createLinkedDuplicate(map);

      deselectAll(map);
      selectNodes(map, {linkedGroupNode});
      translateSelection(map, vm::vec3d{32, 0, 0});
      deselectAll(map);

      const auto originalBrushBounds = brushNode->physicalBounds();

      selectNodes(map, {brushNode});
      translateSelection(map, vm::vec3d{0, 16, 0});

      REQUIRE(
        brushNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d{0, 16, 0}));

      REQUIRE(linkedGroupNode->childCount() == 1u);
      auto* linkedBrushNode =
        dynamic_cast<BrushNode*>(linkedGroupNode->children().front());
      REQUIRE(linkedBrushNode != nullptr);

      CHECK(
        linkedBrushNode->physicalBounds()
        == brushNode->physicalBounds().transform(
          linkedGroupNode->group().transformation()));

      map.undoCommand();

      linkedBrushNode = dynamic_cast<BrushNode*>(linkedGroupNode->children().front());
      REQUIRE(linkedBrushNode != nullptr);

      CHECK(
        linkedBrushNode->physicalBounds()
        == brushNode->physicalBounds().transform(
          linkedGroupNode->group().transformation()));
    }

    SECTION("Update linked groups failure")
    {
      auto* groupNode = new GroupNode{Group{"group"}};
      auto* brushNode = createBrushNode(map);
      groupNode->addChild(brushNode);
      addNodes(map, {{parentForNodes(map), {groupNode}}});

      selectNodes(map, {groupNode});
      auto* linkedGroupNode = createLinkedDuplicate(map);
      deselectAll(map);

      // moving the brush in linked group node will fail because it will go out of world
      // bounds
      selectNodes(map, {linkedGroupNode});
      REQUIRE(translateSelection(
        map, map.worldBounds().max - linkedGroupNode->physicalBounds().size()));
      deselectAll(map);

      const auto originalBrushBounds = brushNode->physicalBounds();

      selectNodes(map, {brushNode});
      CHECK_FALSE(translateSelection(map, vm::vec3d{0, 16, 0}));

      REQUIRE(brushNode->physicalBounds() == originalBrushBounds);

      REQUIRE(linkedGroupNode->childCount() == 1u);
      auto* linkedBrushNode =
        dynamic_cast<BrushNode*>(linkedGroupNode->children().front());
      REQUIRE(linkedBrushNode != nullptr);

      CHECK(
        linkedBrushNode->physicalBounds()
        == brushNode->physicalBounds().transform(
          linkedGroupNode->group().transformation()));
    }
  }
}

} // namespace tb::mdl
