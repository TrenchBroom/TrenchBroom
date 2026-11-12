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

#include "Observer.h"
#include "gl/MaterialManager.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Layers.h"
#include "mdl/Map_NodeLocking.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Matchers.h"
#include "mdl/PatchNode.h"
#include "mdl/SurfaceAttributes.h"
#include "mdl/TagManager.h"
#include "mdl/TagMatcher.h"
#include "mdl/TestFactory.h"
#include "mdl/TestUtils.h"
#include "mdl/VisualEffect.h"
#include "mdl/WorldNode.h"

#include "kd/overload.h"
#include "kd/vector_utils.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{

namespace
{
class TestCallback : public TagMatcherCallback
{
private:
  size_t m_option;

public:
  explicit TestCallback(const size_t option)
    : m_option{option}
  {
  }

  size_t selectOption(const std::vector<std::string>&) override { return m_option; }
};
} // namespace

TEST_CASE("Map_Nodes")
{
  auto fixtureConfig = MapFixtureConfig{};
  fixtureConfig.gameInfo.gameConfig.smartTags = {
    SmartTag{
      "entity",
      {},
      std::make_unique<EntityClassNameTagMatcher>("brush_entity", ""),
    },
    SmartTag{
      "material",
      {},
      std::make_unique<MaterialNameTagMatcher>("tagged_material"),
    },
    SmartTag{
      "content",
      {},
      std::make_unique<ContentFlagsTagMatcher>(1),
    },
  };

  auto fixture = MapFixture{};
  auto& map = fixture.create(fixtureConfig);
  map.entityDefinitionManager().setDefinitions({
    {"point_entity",
     Color{},
     "this is a point entity",
     {},
     PointEntityDefinition{vm::bbox3d{16.0}, {}, {}}},
    {"brush_entity", Color{}, "this is a brush entity", {}},
  });

  const auto& pointEntityDefinition = map.entityDefinitionManager().definitions().front();

  SECTION("parentForNodes")
  {

    auto* customLayerNode =
      addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"custom layer"}));
    auto* groupNode =
      addNode(map, *customLayerNode, std::make_unique<GroupNode>(Group{"group"}));
    auto* groupedEntityNode =
      addNode(map, *groupNode, std::make_unique<EntityNode>(Entity{}));
    addNode(map, *map.worldNode().defaultLayer(), std::make_unique<EntityNode>(Entity{}));

    SECTION("Returns default layer if no group is open")
    {
      CHECK(&parentForNodes(map) == map.worldNode().defaultLayer());
    }

    SECTION("Returns currently opened group, if any")
    {
      openGroup(map, *groupNode);
      CHECK(&parentForNodes(map) == groupNode);
    }

    SECTION("Returns parent of first node in given vector")
    {
      CHECK(&parentForNodes(map, {groupedEntityNode}) == groupNode);
      CHECK(&parentForNodes(map, {groupNode}) == customLayerNode);
    }
  }

  SECTION("addNodes")
  {
    SECTION("Nodes added to a hidden layer are visible")
    {
      auto* layerNode1 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test1"}));
      addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test2"}));

      setCurrentLayer(map, layerNode1);

      // Create an entity in layer1
      auto* entityNode1 =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

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
      auto* entityNode2 =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

      REQUIRE(entityNode2->parent() == layerNode1);

      CHECK(entityNode1->visibilityState() == VisibilityState::Inherited);
      CHECK(!entityNode1->visible());
      CHECK(entityNode2->visibilityState() == VisibilityState::Shown);
      CHECK(entityNode2->visible());
    }

    SECTION("Nodes added to a locked layer are unlocked")
    {
      auto* layerNode1 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test1"}));
      addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test2"}));

      setCurrentLayer(map, layerNode1);

      // Create an entity in layer1
      auto* entityNode1 =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

      REQUIRE(entityNode1->parent() == layerNode1);

      CHECK(entityNode1->lockState() == LockState::Inherited);
      CHECK(!entityNode1->locked());

      lockNodes(map, {layerNode1});

      REQUIRE(entityNode1->lockState() == LockState::Inherited);
      REQUIRE(entityNode1->locked());

      auto* entityNode2 =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

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
        auto groupNodePtr = std::make_unique<GroupNode>(Group{"test"});
        auto* groupNode = groupNodePtr.get();
        groupNode->addChild(createBrushNode(map));
        addNode(map, parentForNodes(map), std::move(groupNodePtr));

        selectNodes(map, {groupNode});
        auto* linkedGroupNode = createLinkedDuplicate(map);
        deselectAll(map);

        using CreateNode = std::function<std::unique_ptr<Node>(const Map&)>;
        const auto createNode = GENERATE_COPY(
          CreateNode{[](const auto&) -> std::unique_ptr<Node> {
            return std::make_unique<EntityNode>(Entity{});
          }},
          CreateNode{
            [](const auto& m) -> std::unique_ptr<Node> { return createBrushNode(m); }},
          CreateNode{
            [](const auto&) -> std::unique_ptr<Node> { return createPatchNode(); }});

        auto* nodeToAdd = addNode(map, *groupNode, createNode(map));

        CHECK(linkedGroupNode->childCount() == 2u);

        auto* linkedNode = linkedGroupNode->children().back();
        linkedNode->accept(kdl::overload(
          [](const WorldNode&) {},
          [](const LayerNode&) {},
          [](const GroupNode&) {},
          [&](const EntityNode& linkedEntityNode) {
            const auto* originalEntityNode = dynamic_cast<EntityNode*>(nodeToAdd);
            REQUIRE(originalEntityNode);
            CHECK(originalEntityNode->entity() == linkedEntityNode.entity());
          },
          [&](const BrushNode& linkedBrushNode) {
            const auto* originalBrushNode = dynamic_cast<BrushNode*>(nodeToAdd);
            REQUIRE(originalBrushNode);
            CHECK(originalBrushNode->brush() == linkedBrushNode.brush());
          },
          [&](const PatchNode& linkedPatchNode) {
            const auto* originalPatchNode = dynamic_cast<PatchNode*>(nodeToAdd);
            REQUIRE(originalPatchNode);
            CHECK(originalPatchNode->patch() == linkedPatchNode.patch());
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
        auto* groupNode =
          addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"group"}));

        selectNodes(map, {groupNode});
        auto* linkedGroupNode = createLinkedDuplicate(map);
        deselectAll(map);

        selectNodes(map, {linkedGroupNode});
        translateSelection(map, vm::vec3d{32, 0, 0});
        deselectAll(map);

        auto* brushNode = addNode(map, *groupNode, createBrushNode(map));

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
        auto* groupNode =
          addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"group"}));

        selectNodes(map, {groupNode});
        auto* linkedGroupNode = createLinkedDuplicate(map);
        deselectAll(map);

        // adding a brush to the linked group node will fail because it will go out of
        // world bounds
        selectNodes(map, {linkedGroupNode});
        translateSelection(map, map.worldBounds().max);
        deselectAll(map);

        CHECK(addNode(map, *groupNode, createBrushNode(map)) == nullptr);

        CHECK(groupNode->childCount() == 0u);
        CHECK(linkedGroupNode->childCount() == 0u);
      }
    }
  }

  SECTION("duplicateSelectedNodes")
  {
    SECTION("Duplicated nodes are added to the source layer")
    {
      auto* layerNode1 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test1"}));
      auto* layerNode2 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test2"}));

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

    SECTION("Duplicating nodes triggers a visual effect")
    {
      auto triggerVisualEffect = Observer<VisualEffect>{map.triggerVisualEffectNotifier};

      auto* entityNode = createPointEntity(map, pointEntityDefinition, {0, 0, 0});
      selectNodes(map, {entityNode});
      duplicateSelectedNodes(map);

      CHECK(
        triggerVisualEffect.notifications == std::vector{VisualEffect::FlashSelection});
    }

    SECTION("Nodes duplicated in a hidden layer become visible")
    {
      auto* layerNode1 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test1"}));

      setCurrentLayer(map, layerNode1);
      hideLayers(map, {layerNode1});

      // Create entity1 and brush1 in the hidden layer1
      auto entityNode1Ptr = std::make_unique<EntityNode>(Entity{});
      auto* entityNode1 = entityNode1Ptr.get();
      auto brushNode1Ptr = createBrushNode(map);
      auto* brushNode1 = brushNode1Ptr.get();
      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(entityNode1Ptr), std::move(brushNode1Ptr)));

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
      auto* layer1 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"Layer 1"}));

      auto* layer2 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"Layer 2"}));

      CHECK(!reparentNodes(map, {{layer2, {layer1}}}));
    }

    SECTION("Reparent between layers")
    {
      auto* oldParent =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"Layer 1"}));

      auto* newParent =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"Layer 2"}));

      auto* entityNode = addNode(map, *oldParent, std::make_unique<EntityNode>(Entity{}));

      REQUIRE(entityNode->parent() == oldParent);
      CHECK(reparentNodes(map, {{newParent, {entityNode}}}));
      CHECK(entityNode->parent() == newParent);

      map.undoCommand();
      CHECK(entityNode->parent() == oldParent);
    }

    SECTION("Cannot reparent a group to itself")
    {
      auto* group =
        addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"Group"}));

      CHECK(!reparentNodes(map, {{group, {group}}}));
    }

    SECTION("Cannot reparent a group to its descendants")
    {
      auto* outer =
        addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"Outer"}));

      auto* inner = addNode(map, *outer, std::make_unique<GroupNode>(Group{"Inner"}));

      CHECK(!reparentNodes(map, {{inner, {outer}}}));
    }

    SECTION("Empty groups are removed after reparenting")
    {
      auto* group =
        addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"Group"}));

      auto* entity = addNode(map, *group, std::make_unique<EntityNode>(Entity{}));

      CHECK(reparentNodes(map, {{&parentForNodes(map), {entity}}}));
      CHECK(entity->parent() == &parentForNodes(map));
      CHECK(group->parent() == nullptr);

      map.undoCommand();
      CHECK(group->parent() == &parentForNodes(map));
      CHECK(entity->parent() == group);
    }

    SECTION("Empty groups are removed recursively after reparenting")
    {
      auto* outer =
        addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"Outer"}));

      auto* inner = addNode(map, *outer, std::make_unique<GroupNode>(Group{"Inner"}));

      auto* entity = addNode(map, *inner, std::make_unique<EntityNode>(Entity{}));

      CHECK(reparentNodes(map, {{&parentForNodes(map), {entity}}}));
      CHECK(entity->parent() == &parentForNodes(map));
      CHECK(inner->parent() == nullptr);
      CHECK(outer->parent() == nullptr);

      map.undoCommand();
      CHECK(outer->parent() == &parentForNodes(map));
      CHECK(inner->parent() == outer);
      CHECK(entity->parent() == inner);
    }

    SECTION("Empty entities are removed after reparenting")
    {
      auto* entity =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

      auto* brush = addNode(map, *entity, createBrushNode(map));

      CHECK(reparentNodes(map, {{&parentForNodes(map), {brush}}}));
      CHECK(brush->parent() == &parentForNodes(map));
      CHECK(entity->parent() == nullptr);

      map.undoCommand();
      CHECK(entity->parent() == &parentForNodes(map));
      CHECK(brush->parent() == entity);
    }

    SECTION("Empty groups and entities are removed after reparenting")
    {
      auto* group =
        addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"Group"}));

      auto* entity = addNode(map, *group, std::make_unique<EntityNode>(Entity{}));

      auto* brush = addNode(map, *entity, createBrushNode(map));

      CHECK(reparentNodes(map, {{&parentForNodes(map), {brush}}}));
      CHECK(brush->parent() == &parentForNodes(map));
      CHECK(group->parent() == nullptr);
      CHECK(entity->parent() == nullptr);

      map.undoCommand();
      CHECK(group->parent() == &parentForNodes(map));
      CHECK(entity->parent() == group);
      CHECK(brush->parent() == entity);
    }

    SECTION("Resetting link IDs")
    {
      auto nestedBrushNodePtr = createBrushNode(map);
      auto* nestedBrushNode = nestedBrushNodePtr.get();
      auto nestedEntityNodePtr = std::make_unique<EntityNode>(Entity{});
      auto* nestedEntityNode = nestedEntityNodePtr.get();

      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(nestedBrushNodePtr), std::move(nestedEntityNodePtr)));
      selectNodes(map, {nestedBrushNode, nestedEntityNode});

      auto* nestedGroupNode = groupSelectedNodes(map, "nested");

      deselectAll(map);
      selectNodes(map, {nestedGroupNode});

      auto* linkedNestedGroupNode = createLinkedDuplicate(map);

      auto brushNodePtr = createBrushNode(map);
      auto* brushNode = brushNodePtr.get();
      auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
      auto* entityNode = entityNodePtr.get();
      auto* entityBrushNode = &entityNode->addChild(createBrushNode(map));

      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(brushNodePtr), std::move(entityNodePtr)));

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
        REQUIRE(reparentNodes(map, {{&parentForNodes(map), {entityNode}}}));

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
      auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
      auto* groupNode = groupNodePtr.get();
      groupNode->addChild(createBrushNode(map));
      addNode(map, parentForNodes(map), std::move(groupNodePtr));

      selectNodes(map, {groupNode});
      auto* linkedGroupNode = createLinkedDuplicate(map);
      deselectAll(map);

      selectNodes(map, {linkedGroupNode});
      translateSelection(map, vm::vec3d{32, 0, 0});
      deselectAll(map);

      SECTION("Move node into group node")
      {
        auto* entityNode =
          addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

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

        CHECK(entityNode->parent() == &parentForNodes(map));
        CHECK(groupNode->childCount() == 1u);
        CHECK(linkedGroupNode->childCount() == 1u);
      }

      SECTION("Move node out of group node")
      {
        auto* entityNode =
          addNode(map, *groupNode, std::make_unique<EntityNode>(Entity{}));

        REQUIRE(groupNode->childCount() == 2u);
        REQUIRE(linkedGroupNode->childCount() == 2u);

        reparentNodes(map, {{&parentForNodes(map), {entityNode}}});

        CHECK(entityNode->parent() == &parentForNodes(map));
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
      auto* brushNode = addNode(map, parentForNodes(map), createBrushNode(map));
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
        CHECK(!reparentNodes(map, {{groupNode, {linkedGroupNode}}}));
      }

      SECTION(
        "Adding a group containing a nested linked sibling to a linked group does "
        "nothing")
      {
        selectNodes(map, {linkedGroupNode});

        auto* outerGroupNode = groupSelectedNodes(map, "outer");
        REQUIRE(outerGroupNode != nullptr);

        deselectAll(map);
        CHECK(!reparentNodes(map, {{groupNode, {outerGroupNode}}}));
      }
    }

    SECTION("Update linked groups after recursive deletion")
    {
      auto* outerGroupNode =
        addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"outer"}));

      openGroup(map, *outerGroupNode);

      auto outerEntityNodePtr = std::make_unique<EntityNode>(Entity{});
      auto* outerEntityNode = outerEntityNodePtr.get();
      auto innerGroupNodePtr = std::make_unique<GroupNode>(Group{"inner"});
      auto* innerGroupNode = innerGroupNodePtr.get();
      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(outerEntityNodePtr), std::move(innerGroupNodePtr)));

      openGroup(map, *innerGroupNode);

      auto* innerEntityNode =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

      closeGroup(map);
      closeGroup(map);

      selectNodes(map, {outerGroupNode});

      auto* linkedOuterGroupNode = createLinkedDuplicate(map);
      REQUIRE_THAT(
        outerGroupNode->children(),
        Catch::Matchers::RangeEquals(
          std::vector<Node*>{outerEntityNode, innerGroupNode}));
      REQUIRE_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));

      deselectAll(map);

      reparentNodes(map, {{&parentForNodes(map), {innerEntityNode}}});
      CHECK_THAT(
        outerGroupNode->children(),
        Catch::Matchers::RangeEquals(std::vector<Node*>{outerEntityNode}));
      CHECK_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));

      map.undoCommand();
      CHECK_THAT(
        outerGroupNode->children(),
        Catch::Matchers::RangeEquals(
          std::vector<Node*>{outerEntityNode, innerGroupNode}));
      REQUIRE_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));

      map.redoCommand();
      CHECK_THAT(
        outerGroupNode->children(),
        Catch::Matchers::RangeEquals(std::vector<Node*>{outerEntityNode}));
      CHECK_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));
    }

    SECTION("Linked group update fails")
    {
      auto* groupNode =
        addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"group"}));

      selectNodes(map, {groupNode});
      auto* linkedGroupNode = createLinkedDuplicate(map);
      deselectAll(map);

      // adding a brush to the linked group node will fail because it will go out of world
      // bounds
      selectNodes(map, {linkedGroupNode});
      translateSelection(map, map.worldBounds().max);
      deselectAll(map);

      auto* brushNode = addNode(map, parentForNodes(map), createBrushNode(map));

      CHECK(!reparentNodes(map, {{groupNode, {brushNode}}}));

      CHECK(groupNode->childCount() == 0u);
      CHECK(linkedGroupNode->childCount() == 0u);
    }

    SECTION("Cannot reparent between linked groups")
    {
      auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
      auto* groupNode = groupNodePtr.get();
      auto* brushNode = &groupNode->addChild(createBrushNode(map));

      addNode(map, parentForNodes(map), std::move(groupNodePtr));

      selectNodes(map, {groupNode});
      auto* linkedGroupNode = createLinkedDuplicate(map);
      deselectAll(map);

      CHECK(!reparentNodes(map, {{linkedGroupNode, {brushNode}}}));

      CHECK(groupNode->childCount() == 1u);
      CHECK(linkedGroupNode->childCount() == 1u);
    }
  }

  SECTION("canMakeStructural")
  {
    SECTION("Empty selection")
    {
      CHECK(!canMakeStructural(map, {}));
    }

    SECTION("Structural brush")
    {
      auto* brushNode = addNode(map, parentForNodes(map), createBrushNode(map));

      CHECK(!canMakeStructural(map, {brushNode}));
    }

    SECTION("Non-structural brush owned by an entity")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* brushNode = addNode(map, *entityNode, createBrushNode(map));

      CHECK(canMakeStructural(map, {brushNode}));
    }

    SECTION("Non-structural brush with a tagged face")
    {
      auto* brushNode =
        addNode(map, parentForNodes(map), createBrushNode(map, "tagged_material"));

      CHECK(canMakeStructural(map, {brushNode}));
    }

    SECTION("Structural patch")
    {
      auto* patchNode = addNode(map, parentForNodes(map), createPatchNode());

      CHECK(!canMakeStructural(map, {patchNode}));
    }

    SECTION("Non-structural patch owned by an entity")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* patchNode = addNode(map, *entityNode, createPatchNode());

      CHECK(canMakeStructural(map, {patchNode}));
    }

    SECTION("Selection containing a non-entity node is rejected")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* brushNode = addNode(map, *entityNode, createBrushNode(map));

      CHECK(!canMakeStructural(map, {brushNode, entityNode}));
    }

    SECTION("Mixed brush and patch selection with a non-structural member")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* brushNode = addNode(map, parentForNodes(map), createBrushNode(map));

      auto* patchNode = addNode(map, *entityNode, createPatchNode());

      CHECK(canMakeStructural(map, {brushNode, patchNode}));
    }
  }

  SECTION("makeStructural")
  {
    SECTION("Empty node list does nothing")
    {
      auto callback = TestCallback{0};
      CHECK(!makeStructural(map, {}, callback));
    }

    SECTION("Already-structural untagged brush does nothing")
    {
      auto* brushNode = addNode(map, parentForNodes(map), createBrushNode(map));

      auto callback = TestCallback{0};
      CHECK(!makeStructural(map, {brushNode}, callback));
      CHECK(brushNode->entity() == &map.worldNode());
    }

    SECTION("Brush owned by an entity is reparented and its entity tag is cleared")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* brushNode = addNode(map, *entityNode, createBrushNode(map));

      const auto& entityTag = map.tagManager().smartTag("entity");
      REQUIRE(brushNode->hasTag(entityTag));

      auto callback = TestCallback{0};
      CHECK(makeStructural(map, {brushNode}, callback));
      CHECK(brushNode->entity() == &map.worldNode());
      CHECK(!brushNode->hasTag(entityTag));

      map.undoCommand();
      CHECK(brushNode->entity() == entityNode);
      CHECK(brushNode->hasTag(entityTag));
    }

    SECTION("Structural brush with a tagged face has the content flags tag disabled")
    {
      // Material name tags (like "material" above) have no disable logic -- there is
      // no sensible default material to fall back to -- so use a flags-based tag,
      // which does support being disabled, to test that face tags are cleared.
      auto* brushNode = addNode(
        map, parentForNodes(map), createBrushNode(map, "material", [](auto& brush) {
          for (auto& face : brush.faces())
          {
            face.setSurfaceAttributes({.contents = 1});
          }
        }));

      REQUIRE(brushNode->anyFaceHasAnyTag());

      auto callback = TestCallback{0};
      CHECK(makeStructural(map, {brushNode}, callback));
      CHECK(!brushNode->anyFaceHasAnyTag());
    }

    SECTION("Patch owned by an entity is reparented and its entity tag is cleared")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* patchNode = addNode(map, *entityNode, createPatchNode());

      const auto& entityTag = map.tagManager().smartTag("entity");
      REQUIRE(patchNode->hasTag(entityTag));

      auto callback = TestCallback{0};
      CHECK(makeStructural(map, {patchNode}, callback));
      CHECK(patchNode->entity() == &map.worldNode());
      CHECK(!patchNode->hasTag(entityTag));

      map.undoCommand();
      CHECK(patchNode->entity() == entityNode);
      CHECK(patchNode->hasTag(entityTag));
    }

    SECTION("Mixed brush and patch selection both become structural in one transaction")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* brushNode = addNode(map, *entityNode, createBrushNode(map));

      auto* patchNode = addNode(map, *entityNode, createPatchNode());

      auto callback = TestCallback{0};
      CHECK(makeStructural(map, {brushNode, patchNode}, callback));
      CHECK(brushNode->entity() == &map.worldNode());
      CHECK(patchNode->entity() == &map.worldNode());

      map.undoCommand();
      CHECK(brushNode->entity() == entityNode);
      CHECK(patchNode->entity() == entityNode);
    }

    SECTION(
      "Selected brush that is the sole child of its entity is reparented without "
      "corrupting selection state")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* brushNode = addNode(map, *entityNode, createBrushNode(map));

      selectNodes(map, {brushNode});

      auto callback = TestCallback{0};
      CHECK(makeStructural(map, {brushNode}, callback));
      CHECK(brushNode->entity() == &map.worldNode());
      CHECK(entityNode->parent() == nullptr);
      CHECK(map.selection().nodes == std::vector<Node*>{brushNode});
    }

    SECTION(
      "Selected patch that is the sole child of its entity is reparented without "
      "corrupting selection state")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* patchNode = addNode(map, *entityNode, createPatchNode());

      selectNodes(map, {patchNode});

      auto callback = TestCallback{0};
      CHECK(makeStructural(map, {patchNode}, callback));
      CHECK(patchNode->entity() == &map.worldNode());
      CHECK(entityNode->parent() == nullptr);
      CHECK(map.selection().nodes == std::vector<Node*>{patchNode});
    }
  }

  SECTION("moveToEntity")
  {
    SECTION("Empty node list does nothing")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      CHECK(!moveToEntity(map, {}, *entityNode));
    }

    SECTION("Moves a structural brush into an entity and selects it")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* brushNode = addNode(map, parentForNodes(map), createBrushNode(map));

      CHECK(moveToEntity(map, {brushNode}, *entityNode));
      CHECK(brushNode->entity() == entityNode);
      CHECK(map.selection().brushes == std::vector<BrushNode*>{brushNode});

      map.undoCommand();
      CHECK(brushNode->entity() == &map.worldNode());
    }

    SECTION("Brush already parented under the target entity is skipped")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* brushNode = addNode(map, *entityNode, createBrushNode(map));

      CHECK(!moveToEntity(map, {brushNode}, *entityNode));
    }

    SECTION("Moves a structural patch into an entity and selects it")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* patchNode = addNode(map, parentForNodes(map), createPatchNode());

      CHECK(moveToEntity(map, {patchNode}, *entityNode));
      CHECK(patchNode->entity() == entityNode);
      CHECK(map.selection().patches == std::vector<PatchNode*>{patchNode});

      map.undoCommand();
      CHECK(patchNode->entity() == &map.worldNode());
    }

    SECTION("Moves a brush and a patch together and both pick up the entity's tag")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto* brushNode = addNode(map, parentForNodes(map), createBrushNode(map));

      auto* patchNode = addNode(map, parentForNodes(map), createPatchNode());

      const auto& entityTag = map.tagManager().smartTag("entity");
      REQUIRE(!brushNode->hasTag(entityTag));
      REQUIRE(!patchNode->hasTag(entityTag));

      CHECK(moveToEntity(map, {brushNode, patchNode}, *entityNode));
      CHECK(brushNode->hasTag(entityTag));
      CHECK(patchNode->hasTag(entityTag));
    }

    SECTION("Non-geometry node in the input is ignored")
    {
      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}}));

      auto otherEntityNodePtr = std::make_unique<EntityNode>(Entity{});
      auto* otherEntityNode = otherEntityNodePtr.get();
      auto brushNodePtr = createBrushNode(map);
      auto* brushNode = brushNodePtr.get();
      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(otherEntityNodePtr), std::move(brushNodePtr)));

      CHECK(moveToEntity(map, {otherEntityNode, brushNode}, *entityNode));
      CHECK(brushNode->entity() == entityNode);
      CHECK(otherEntityNode->parent() != entityNode);
    }
  }

  SECTION("removeNodes")
  {
    SECTION("Remove layer")
    {
      auto* layer =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"Layer 1"}));

      removeNodes(map, {layer});
      CHECK(layer->parent() == nullptr);

      map.undoCommand();
      CHECK(layer->parent() == &map.worldNode());
    }

    SECTION("Remove empty group")
    {
      auto* group =
        addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"group"}));

      openGroup(map, *group);

      auto* brush = addNode(map, parentForNodes(map), createBrushNode(map));

      removeNodes(map, {brush});
      CHECK(map.editorContext().currentGroup() == nullptr);
      CHECK(brush->parent() == nullptr);
      CHECK(group->parent() == nullptr);

      map.undoCommand();
      CHECK(map.editorContext().currentGroup() == group);
      CHECK(brush->parent() == group);
      CHECK(group->parent() == map.worldNode().defaultLayer());
    }

    SECTION("Recursively remove empty groups")
    {
      auto* outer =
        addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"outer"}));

      openGroup(map, *outer);

      auto* inner =
        addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"inner"}));

      openGroup(map, *inner);

      auto* brush = addNode(map, parentForNodes(map), createBrushNode(map));

      removeNodes(map, {brush});
      CHECK(map.editorContext().currentGroup() == nullptr);
      CHECK(brush->parent() == nullptr);
      CHECK(inner->parent() == nullptr);
      CHECK(outer->parent() == nullptr);

      map.undoCommand();
      CHECK(map.editorContext().currentGroup() == inner);
      CHECK(brush->parent() == inner);
      CHECK(inner->parent() == outer);
      CHECK(outer->parent() == map.worldNode().defaultLayer());
    }

    SECTION("Remove empty brush entitiy")
    {
      auto* layer =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"Layer 1"}));

      auto* entity = addNode(map, *layer, std::make_unique<EntityNode>(Entity{}));

      auto* brush = addNode(map, *entity, createBrushNode(map));

      removeNodes(map, {brush});
      CHECK(brush->parent() == nullptr);
      CHECK(entity->parent() == nullptr);

      map.undoCommand();
      CHECK(brush->parent() == entity);
      CHECK(entity->parent() == layer);
    }

    SECTION("Update linked groups")
    {
      auto groupNodePtr = std::make_unique<GroupNode>(Group{"test"});
      auto* groupNode = groupNodePtr.get();
      groupNode->addChild(createBrushNode(map));

      using CreateNode = std::function<std::unique_ptr<Node>(const Map&)>;
      const auto createNode = GENERATE_COPY(
        CreateNode{[](const auto&) -> std::unique_ptr<Node> {
          return std::make_unique<EntityNode>(Entity{});
        }},
        CreateNode{
          [](const auto& m) -> std::unique_ptr<Node> { return createBrushNode(m); }},
        CreateNode{
          [](const auto&) -> std::unique_ptr<Node> { return createPatchNode(); }});

      auto* nodeToRemove = &groupNode->addChild(createNode(map));
      addNode(map, parentForNodes(map), std::move(groupNodePtr));

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
      auto* outerGroupNode =
        addNode(map, parentForNodes(map), std::make_unique<GroupNode>(Group{"outer"}));

      openGroup(map, *outerGroupNode);

      auto outerEntityNodePtr = std::make_unique<EntityNode>(Entity{});
      auto* outerEntityNode = outerEntityNodePtr.get();
      auto innerGroupNodePtr = std::make_unique<GroupNode>(Group{"inner"});
      auto* innerGroupNode = innerGroupNodePtr.get();
      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(outerEntityNodePtr), std::move(innerGroupNodePtr)));

      openGroup(map, *innerGroupNode);

      auto* innerEntityNode =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

      closeGroup(map);
      closeGroup(map);

      selectNodes(map, {outerGroupNode});

      auto* linkedOuterGroupNode = createLinkedDuplicate(map);
      deselectAll(map);

      REQUIRE_THAT(
        outerGroupNode->children(),
        Catch::Matchers::RangeEquals(
          std::vector<Node*>{outerEntityNode, innerGroupNode}));
      REQUIRE_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));

      removeNodes(map, {innerEntityNode});
      REQUIRE_THAT(
        outerGroupNode->children(),
        Catch::Matchers::RangeEquals(std::vector<Node*>{outerEntityNode}));
      CHECK_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));

      map.undoCommand();
      REQUIRE_THAT(
        outerGroupNode->children(),
        Catch::Matchers::RangeEquals(
          std::vector<Node*>{outerEntityNode, innerGroupNode}));
      CHECK_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));

      map.redoCommand();
      REQUIRE_THAT(
        outerGroupNode->children(),
        Catch::Matchers::RangeEquals(std::vector<Node*>{outerEntityNode}));
      CHECK_THAT(*linkedOuterGroupNode, MatchesNode(*outerGroupNode));
    }
  }

  SECTION("removeSelectedNodes")
  {
    auto* entityNode =
      addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));
    selectNodes(map, {entityNode});

    removeSelectedNodes(map);
    CHECK(map.selection().nodes == std::vector<Node*>{});
    CHECK_THAT(
      map.worldNode().defaultLayer()->children(),
      Catch::Matchers::RangeEquals(std::vector<Node*>{}));
  }

  SECTION("updateNodeContents")
  {
    SECTION("Update brushes")
    {
      auto* brushNode = addNode(map, parentForNodes(map), createBrushNode(map));

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
      auto* patchNode = addNode(map, parentForNodes(map), createPatchNode());

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
      setEntityProperty(map, EntityPropertyKeys::Wad, "test/mdl/Map/cr8_czg.wad");

      constexpr auto MaterialName = "bongs2";
      const auto* material = map.materialManager().material(MaterialName);
      REQUIRE(material != nullptr);

      auto* brushNode =
        addNode(map, parentForNodes(map), createBrushNode(map, MaterialName));

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

    SECTION("Update patch material usage counts")
    {
      deselectAll(map);
      setEntityProperty(map, EntityPropertyKeys::Wad, "test/mdl/Map/cr8_czg.wad");

      constexpr auto MaterialName = "bongs2";
      const auto* material = map.materialManager().material(MaterialName);
      REQUIRE(material != nullptr);

      auto* patchNode = addNode(map, parentForNodes(map), createPatchNode(MaterialName));

      const auto& originalPatch = patchNode->patch();
      auto modifiedPatch = originalPatch;
      modifiedPatch.transform(vm::translation_matrix(vm::vec3d{16, 0, 0}));

      auto nodesToSwap = std::vector<std::pair<Node*, NodeContents>>{};
      nodesToSwap.emplace_back(patchNode, std::move(modifiedPatch));

      REQUIRE(material->usageCount() == 1u);

      updateNodeContents(map, "Update Nodes", std::move(nodesToSwap), {});
      CHECK(material->usageCount() == 1u);

      map.undoCommand();
      CHECK(material->usageCount() == 1u);
    }

    SECTION("Update entity definition usage counts")
    {
      constexpr auto Classname = "point_entity";


      auto* entityNode = addNode(
        map,
        parentForNodes(map),
        std::make_unique<EntityNode>(Entity{{
          {EntityPropertyKeys::Classname, Classname},
        }}));

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
      auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
      auto* groupNode = groupNodePtr.get();
      auto* brushNode = &groupNode->addChild(createBrushNode(map));
      addNode(map, parentForNodes(map), std::move(groupNodePtr));

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
      auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
      auto* groupNode = groupNodePtr.get();
      auto* brushNode = &groupNode->addChild(createBrushNode(map));
      addNode(map, parentForNodes(map), std::move(groupNodePtr));

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
      CHECK(!translateSelection(map, vm::vec3d{0, 16, 0}));

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
