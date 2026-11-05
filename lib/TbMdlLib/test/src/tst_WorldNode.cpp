/*
 Copyright (C) 2020 Kristian Duske

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

#include "mdl/BezierPatch.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/MapFormat.h"
#include "mdl/Octree.h"
#include "mdl/PatchNode.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"

#include "kd/memory_utils.h"
#include "kd/result.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("WorldNode")
{
  constexpr auto worldBounds = vm::bbox3d{8192.0};
  constexpr auto mapFormat = MapFormat::Quake3;

  SECTION("canAddChild")
  {
    const auto worldNode = WorldNode{{}, {}, mapFormat};
    auto layerNode = LayerNode{Layer{"layer"}};
    auto groupNode = GroupNode{Group{"group"}};
    auto entityNode = EntityNode{Entity{}};
    auto brushNode = BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

    // clang-format off
    auto patchNode = PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
    // clang-format on

    CHECK(!worldNode.canAddChild(worldNode));
    CHECK(worldNode.canAddChild(layerNode));
    CHECK(!worldNode.canAddChild(groupNode));
    CHECK(!worldNode.canAddChild(entityNode));
    CHECK(!worldNode.canAddChild(brushNode));
    CHECK(!worldNode.canAddChild(patchNode));
  }

  SECTION("canRemoveChild")
  {
    const auto worldNode = WorldNode{{}, {}, mapFormat};
    auto layerNode = LayerNode{Layer{"layer"}};
    auto groupNode = GroupNode{Group{"group"}};
    auto entityNode = EntityNode{Entity{}};
    auto brushNode = BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

    // clang-format off
    auto patchNode = PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
    // clang-format on

    CHECK(!worldNode.canRemoveChild(worldNode));
    CHECK(worldNode.canRemoveChild(layerNode));
    CHECK(!worldNode.canRemoveChild(*worldNode.defaultLayer()));
    CHECK(!worldNode.canRemoveChild(groupNode));
    CHECK(!worldNode.canRemoveChild(entityNode));
    CHECK(!worldNode.canRemoveChild(brushNode));
    CHECK(!worldNode.canRemoveChild(patchNode));
  }

  SECTION("nodeTree")
  {
    auto worldNode = WorldNode{{}, {}, mapFormat};
    auto layerNodePtr = std::make_unique<LayerNode>(Layer{"layer"});
    auto* layerNode = layerNodePtr.get();
    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();

    // clang-format off
    auto childNodes = kdl::vec_from<std::unique_ptr<Node>>(
      std::make_unique<EntityNode>(Entity{}),
      std::make_unique<BrushNode>(
        BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()),
      std::make_unique<PatchNode>(BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}));
    // clang-format on

    constexpr size_t entityNodeIndex = 0;
    constexpr size_t brushNodeIndex = 1;
    constexpr size_t patchNodeIndex = 2;
    auto* entityNode = childNodes[entityNodeIndex].get();
    auto* brushNode = childNodes[brushNodeIndex].get();
    auto* patchNode = childNodes[patchNodeIndex].get();
    const auto nodes = std::vector<Node*>{entityNode, brushNode, patchNode};

    const auto& nodeTree = worldNode.nodeTree();

    // Each section moves from childNodes, but Catch2 runs the setup above again for
    // every section.
    // NOLINTBEGIN(bugprone-use-after-move)
    SECTION("Adding a single node inserts into node tree")
    {
      const auto index = GENERATE_COPY(entityNodeIndex, brushNodeIndex, patchNodeIndex);
      auto* node = nodes[index];

      REQUIRE_FALSE(nodeTree.contains(node));
      worldNode.defaultLayer()->addChild(std::move(childNodes[index]));
      CHECK(nodeTree.contains(node));
    }

    SECTION("Adding a nested node inserts into node tree")
    {
      worldNode.defaultLayer()->addChild(std::move(groupNodePtr));

      const auto index = GENERATE_COPY(entityNodeIndex, brushNodeIndex, patchNodeIndex);
      auto* node = nodes[index];

      REQUIRE_FALSE(nodeTree.contains(node));
      groupNode->addChild(std::move(childNodes[index]));
      CHECK(nodeTree.contains(node));
    }

    SECTION("Adding a layer does not insert it into node tree")
    {
      REQUIRE_FALSE(nodeTree.contains(layerNode));
      worldNode.addChild(std::move(layerNodePtr));
      CHECK(!nodeTree.contains(layerNode));
    }

    SECTION("Adding a group node does not insert it into node tree")
    {
      groupNode->addChild(std::move(childNodes[entityNodeIndex]));

      REQUIRE_FALSE(nodeTree.contains(groupNode));
      worldNode.defaultLayer()->addChild(std::move(groupNodePtr));
      CHECK(!nodeTree.contains(groupNode));
    }

    SECTION("Adding a subtree inserts all children into node tree")
    {
      groupNode->addChildren(std::move(childNodes));

      REQUIRE_FALSE(nodeTree.contains(groupNode));
      REQUIRE_FALSE(nodeTree.contains(entityNode));
      REQUIRE_FALSE(nodeTree.contains(brushNode));
      REQUIRE_FALSE(nodeTree.contains(patchNode));
      worldNode.defaultLayer()->addChild(std::move(groupNodePtr));
      CHECK(!nodeTree.contains(groupNode));
      CHECK(nodeTree.contains(entityNode));
      CHECK(nodeTree.contains(brushNode));
      CHECK(nodeTree.contains(patchNode));
    }

    SECTION("Removing a single node removes from node tree")
    {
      const auto index = GENERATE_COPY(entityNodeIndex, brushNodeIndex, patchNodeIndex);
      auto* node = nodes[index];

      worldNode.defaultLayer()->addChild(std::move(childNodes[index]));
      REQUIRE(nodeTree.contains(node));

      const auto removedNode = worldNode.defaultLayer()->removeChild(node);
      CHECK(!nodeTree.contains(node));
    }

    SECTION("Removing a nested node removes from node tree")
    {
      groupNode->addChildren(std::move(childNodes));
      worldNode.defaultLayer()->addChild(std::move(groupNodePtr));

      auto* node = nodes[GENERATE_COPY(entityNodeIndex, brushNodeIndex, patchNodeIndex)];
      REQUIRE(nodeTree.contains(node));

      const auto removedNode = groupNode->removeChild(node);
      CHECK(!nodeTree.contains(node));
    }

    SECTION("Removing a subtree removes all children from node tree")
    {
      groupNode->addChildren(std::move(childNodes));

      worldNode.defaultLayer()->addChild(std::move(groupNodePtr));
      REQUIRE(nodeTree.contains(entityNode));
      REQUIRE(nodeTree.contains(brushNode));
      REQUIRE(nodeTree.contains(patchNode));

      const auto removedNode = worldNode.defaultLayer()->removeChild(groupNode);
      CHECK(!nodeTree.contains(entityNode));
      CHECK(!nodeTree.contains(brushNode));
      CHECK(!nodeTree.contains(patchNode));
    }

    SECTION("Updating a descendant updates it in node tree")
    {
      groupNode->addChildren(std::move(childNodes));
      worldNode.defaultLayer()->addChild(std::move(groupNodePtr));

      REQUIRE(nodeTree.contains(entityNode));
      REQUIRE(nodeTree.contains(brushNode));
      REQUIRE(nodeTree.contains(patchNode));
      REQUIRE_THAT(
        nodeTree.find_containers(vm::vec3d{0, 0, 0}),
        UnorderedEquals(std::vector<Node*>{entityNode, brushNode, patchNode}));
      REQUIRE_THAT(
        nodeTree.find_containers(vm::vec3d{384, 384, 384}),
        UnorderedEquals(std::vector<Node*>{}));

      transformNode(
        *entityNode, vm::translation_matrix(vm::vec3d(384, 384, 384)), worldBounds);
      transformNode(
        *brushNode, vm::translation_matrix(vm::vec3d(384, 384, 384)), worldBounds);
      transformNode(
        *patchNode, vm::translation_matrix(vm::vec3d(384, 384, 384)), worldBounds);

      CHECK(nodeTree.contains(entityNode));
      CHECK(nodeTree.contains(brushNode));
      CHECK(nodeTree.contains(patchNode));
      CHECK_THAT(
        nodeTree.find_containers(vm::vec3d{0, 0, 0}),
        UnorderedEquals(std::vector<Node*>{}));
      CHECK_THAT(
        nodeTree.find_containers(vm::vec3d{384, 384, 384}),
        UnorderedEquals(std::vector<Node*>{entityNode, brushNode, patchNode}));
    }
    // NOLINTEND(bugprone-use-after-move)
  }

  SECTION("rebuildNodeTree")
  {
    auto worldNode = WorldNode{{}, {}, mapFormat};
    auto* layerNode = &worldNode.addChild(std::make_unique<LayerNode>(Layer{"layer"}));
    auto* entityNode =
      &worldNode.defaultLayer()->addChild(std::make_unique<EntityNode>(Entity{}));
    auto* groupNode =
      &worldNode.defaultLayer()->addChild(std::make_unique<GroupNode>(Group{"group"}));
    auto* brushNode = &groupNode->addChild(std::make_unique<BrushNode>(
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()));

    // clang-format off
    auto* patchNode = &groupNode->addChild(std::make_unique<PatchNode>(BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}));
    // clang-format on

    const auto& nodeTree = worldNode.nodeTree();

    REQUIRE_FALSE(nodeTree.contains(layerNode));
    REQUIRE_FALSE(nodeTree.contains(groupNode));
    REQUIRE(nodeTree.contains(entityNode));
    REQUIRE(nodeTree.contains(brushNode));
    REQUIRE(nodeTree.contains(patchNode));

    worldNode.rebuildNodeTree();
    CHECK(!nodeTree.contains(layerNode));
    CHECK(!nodeTree.contains(groupNode));
    CHECK(nodeTree.contains(entityNode));
    CHECK(nodeTree.contains(brushNode));
    CHECK(nodeTree.contains(patchNode));
  }

  SECTION("disableNodeTreeUpdates")
  {
    auto worldNode = WorldNode{{}, {}, mapFormat};

    // clang-format off
    const auto patchNode = std::make_unique<PatchNode>(BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"});
    // clang-format on

    worldNode.disableNodeTreeUpdates();
    worldNode.addChild(std::make_unique<LayerNode>(Layer{"layer"}));
    auto* entityNode =
      &worldNode.defaultLayer()->addChild(std::make_unique<EntityNode>(Entity{}));
    auto* groupNode =
      &worldNode.defaultLayer()->addChild(std::make_unique<GroupNode>(Group{"group"}));
    auto* brushNode = &groupNode->addChild(std::make_unique<BrushNode>(
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()));

    const auto& nodeTree = worldNode.nodeTree();

    CHECK(!nodeTree.contains(entityNode));
    CHECK(!nodeTree.contains(brushNode));
    CHECK(!nodeTree.contains(patchNode.get()));
  }

  SECTION("enableNodeTreeUpdates")
  {
    auto worldNode = WorldNode{{}, {}, mapFormat};

    // clang-format off
    auto patchNodePtr = std::make_unique<PatchNode>(BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"});
    // clang-format on
    auto* patchNode = patchNodePtr.get();

    worldNode.disableNodeTreeUpdates();
    worldNode.addChild(std::make_unique<LayerNode>(Layer{"layer"}));
    auto* groupNode =
      &worldNode.defaultLayer()->addChild(std::make_unique<GroupNode>(Group{"group"}));

    const auto& nodeTree = worldNode.nodeTree();
    REQUIRE_FALSE(nodeTree.contains(patchNode));

    worldNode.enableNodeTreeUpdates();
    groupNode->addChild(std::move(patchNodePtr));
    CHECK(nodeTree.contains(patchNode));
  }

  SECTION("persistent IDs")
  {
    SECTION("a fresh default layer has no persistent ID")
    {
      auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
      CHECK(worldNode.defaultLayer()->persistentId() == std::nullopt);
    }

    SECTION("adding a layer assigns it a persistent ID")
    {
      auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
      auto initialLayerNodePtr = std::make_unique<LayerNode>(Layer{"name"});
      auto* initialLayerNode = initialLayerNodePtr.get();

      REQUIRE(initialLayerNode->persistentId() == std::nullopt);
      worldNode.addChild(std::move(initialLayerNodePtr));
      CHECK(initialLayerNode->persistentId() == 1u);

      SECTION("Adding a layer node that already has a persistent ID")
      {
        const auto id = *initialLayerNode->persistentId() + 10u;

        auto layerNodeWithIdPtr = std::make_unique<LayerNode>(Layer{"name"});
        auto* layerNodeWithId = layerNodeWithIdPtr.get();
        layerNodeWithId->setPersistentId(id);

        worldNode.addChild(std::move(layerNodeWithIdPtr));
        CHECK(layerNodeWithId->persistentId() == id);

        auto* layerNodeWithoutId =
          &worldNode.addChild(std::make_unique<LayerNode>(Layer{"name"}));
        REQUIRE(layerNodeWithoutId->persistentId() != std::nullopt);
        CHECK(*layerNodeWithoutId->persistentId() == id + 1u);

        SECTION("Adding a layer node that already has a lower persistent ID")
        {
          const auto lowerId = id - 1u;

          auto layerNodeWithLowerIdPtr = std::make_unique<LayerNode>(Layer{"name"});
          auto* layerNodeWithLowerId = layerNodeWithLowerIdPtr.get();
          layerNodeWithLowerId->setPersistentId(lowerId);

          worldNode.addChild(std::move(layerNodeWithLowerIdPtr));
          REQUIRE(layerNodeWithLowerId->persistentId() == lowerId);

          layerNodeWithoutId =
            &worldNode.addChild(std::make_unique<LayerNode>(Layer{"name"}));
          REQUIRE(layerNodeWithoutId->persistentId() != std::nullopt);
          CHECK(*layerNodeWithoutId->persistentId() == id + 2u);
        }
      }
    }

    SECTION("adding a group assigns it a persistent ID")
    {
      auto worldNode = WorldNode{{}, {}, MapFormat::Standard};
      auto initialGroupNodePtr = std::make_unique<GroupNode>(Group{"name"});
      auto* initialGroupNode = initialGroupNodePtr.get();

      REQUIRE(initialGroupNode->persistentId() == std::nullopt);
      worldNode.defaultLayer()->addChild(std::move(initialGroupNodePtr));
      CHECK(initialGroupNode->persistentId() == 1u);

      SECTION("Adding a layer node that already has a persistent ID")
      {
        const auto id = *initialGroupNode->persistentId() + 10u;

        auto groupNodeWithIdPtr = std::make_unique<GroupNode>(Group{"name"});
        auto* groupNodeWithId = groupNodeWithIdPtr.get();
        groupNodeWithId->setPersistentId(id);

        worldNode.defaultLayer()->addChild(std::move(groupNodeWithIdPtr));
        CHECK(groupNodeWithId->persistentId() == id);

        auto* groupNodeWithoutId =
          &worldNode.defaultLayer()->addChild(std::make_unique<GroupNode>(Group{"name"}));
        REQUIRE(groupNodeWithoutId->persistentId() != std::nullopt);
        CHECK(*groupNodeWithoutId->persistentId() == id + 1u);

        SECTION("Adding a layer node that already has a lower persistent ID")
        {
          const auto lowerId = id - 1u;

          auto groupNodeWithLowerIdPtr = std::make_unique<GroupNode>(Group{"name"});
          auto* groupNodeWithLowerId = groupNodeWithLowerIdPtr.get();
          groupNodeWithLowerId->setPersistentId(lowerId);

          worldNode.defaultLayer()->addChild(std::move(groupNodeWithLowerIdPtr));
          REQUIRE(groupNodeWithLowerId->persistentId() == lowerId);

          groupNodeWithoutId = &worldNode.defaultLayer()->addChild(
            std::make_unique<GroupNode>(Group{"name"}));
          REQUIRE(groupNodeWithoutId->persistentId() != std::nullopt);
          CHECK(*groupNodeWithoutId->persistentId() == id + 2u);
        }
      }
    }

    SECTION("persistent IDs are shared between layers and groups")
    {
      auto worldNode = WorldNode{{}, {}, MapFormat::Standard};

      auto* layerNode = &worldNode.addChild(std::make_unique<LayerNode>(Layer{"name"}));
      CHECK(layerNode->persistentId() == 1u);

      auto* groupNode = &layerNode->addChild(std::make_unique<GroupNode>(Group{"name"}));
      CHECK(groupNode->persistentId() == 2u);
    }
  }

  SECTION("cloneRecursively")
  {
    auto worldNode = WorldNode{{}, Entity{{{"classname", "worldspawn"}}}, mapFormat};

    auto* brushNode = &worldNode.defaultLayer()->addChild(std::make_unique<BrushNode>(
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()));

    SECTION("with only the default layer")
    {
      const auto clone =
        kdl::static_pointer_cast<WorldNode>(worldNode.cloneRecursively(worldBounds));

      CHECK(clone.get() != &worldNode);
      CHECK(clone->entity() == worldNode.entity());
      CHECK(clone->mapFormat() == worldNode.mapFormat());
      CHECK(clone->defaultLayer() != worldNode.defaultLayer());
      CHECK(clone->children().size() == 1u);
      CHECK(clone->defaultLayer()->children().size() == 1u);
      CHECK(clone->defaultLayer()->children().front() != brushNode);
    }

    SECTION("with an additional custom layer")
    {
      auto* layerNode = &worldNode.addChild(std::make_unique<LayerNode>(Layer{"custom"}));
      auto* entityNode = &layerNode->addChild(std::make_unique<EntityNode>(Entity{}));

      const auto clone =
        kdl::static_pointer_cast<WorldNode>(worldNode.cloneRecursively(worldBounds));

      CHECK(clone->children().size() == 2u);
      CHECK(clone->defaultLayer()->children().size() == 1u);

      auto* clonedLayerNode = clone->children().back();
      CHECK(clonedLayerNode != layerNode);
      CHECK(clonedLayerNode->children().size() == 1u);
      CHECK(clonedLayerNode->children().front() != entityNode);
    }
  }
}

} // namespace tb::mdl
