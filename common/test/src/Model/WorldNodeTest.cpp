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

#include "AABBTree.h"
#include "Model/BezierPatch.h"
#include "Model/BrushNode.h"
#include "Model/BrushBuilder.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/Layer.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"

#include <kdl/result.h>
#include <kdl/result_io.h>
#include <kdl/string_utils.h>

#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>

#include "TestUtils.h"
#include "Catch2.h"

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("WorldNodeTest.canAddChild") {
            constexpr auto worldBounds = vm::bbox3d{8192.0};
            constexpr auto mapFormat = MapFormat::Quake3;

            const auto worldNode = WorldNode{Entity{}, mapFormat};
            auto layerNode = LayerNode{Layer{"layer"}};
            auto groupNode = GroupNode{Group{"group"}};
            auto entityNode = EntityNode{Entity{}};
            auto brushNode = BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};
            auto patchNode = PatchNode{BezierPatch{3, 3, {
                {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
                {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
                {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};

            CHECK_FALSE(worldNode.canAddChild(&worldNode));
            CHECK(worldNode.canAddChild(&layerNode));
            CHECK_FALSE(worldNode.canAddChild(&groupNode));
            CHECK_FALSE(worldNode.canAddChild(&entityNode));
            CHECK_FALSE(worldNode.canAddChild(&brushNode));
            CHECK_FALSE(worldNode.canAddChild(&patchNode));
        }

        TEST_CASE("WorldNodeTest.canRemoveChild") {
            constexpr auto worldBounds = vm::bbox3d{8192.0};
            constexpr auto mapFormat = MapFormat::Quake3;

            const auto worldNode = WorldNode{Entity{}, mapFormat};
            auto layerNode = LayerNode{Layer{"layer"}};
            auto groupNode = GroupNode{Group{"group"}};
            auto entityNode = EntityNode{Entity{}};
            auto brushNode = BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};
            auto patchNode = PatchNode{BezierPatch{3, 3, {
                {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
                {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
                {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};

            CHECK_FALSE(worldNode.canRemoveChild(&worldNode));
            CHECK(worldNode.canRemoveChild(&layerNode));
            CHECK_FALSE(worldNode.canRemoveChild(worldNode.defaultLayer()));
            CHECK_FALSE(worldNode.canRemoveChild(&groupNode));
            CHECK_FALSE(worldNode.canRemoveChild(&entityNode));
            CHECK_FALSE(worldNode.canRemoveChild(&brushNode));
            CHECK_FALSE(worldNode.canRemoveChild(&patchNode));
        }

        TEST_CASE("WorldNodeTest.nodeTreeUpdates") {
            constexpr auto worldBounds = vm::bbox3d{8192.0};
            constexpr auto mapFormat = MapFormat::Quake3;

            auto worldNode = WorldNode{Entity{}, mapFormat};
            auto* layerNode = new LayerNode{Layer{"layer"}};
            auto* groupNode = new GroupNode{Group{"group"}};
            auto* entityNode = new EntityNode{Entity{}};
            auto* brushNode = new BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};
            auto* patchNode = new PatchNode{BezierPatch{3, 3, {
                {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
                {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
                {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
            
            const auto& nodeTree = worldNode.nodeTree();

            SECTION("Adding a single node inserts into node tree") {
                auto* node = GENERATE_COPY(entityNode, brushNode, patchNode);

                REQUIRE_FALSE(nodeTree.contains(node));
                worldNode.defaultLayer()->addChild(node);
                CHECK(nodeTree.contains(node));
            }

            SECTION("Adding a nested node inserts into node tree") {
                worldNode.defaultLayer()->addChild(groupNode);

                auto* node = GENERATE_COPY(entityNode, brushNode, patchNode);

                REQUIRE_FALSE(nodeTree.contains(node));
                groupNode->addChild(node);
                CHECK(nodeTree.contains(node));
            }

            SECTION("Adding a layer does not insert it into node tree") {
                REQUIRE_FALSE(nodeTree.contains(layerNode));
                worldNode.addChild(layerNode);
                CHECK_FALSE(nodeTree.contains(layerNode));
            }

            SECTION("Adding a group node does not insert it into node tree") {
                groupNode->addChild(entityNode);

                REQUIRE_FALSE(nodeTree.contains(groupNode));
                worldNode.defaultLayer()->addChild(groupNode);
                CHECK_FALSE(nodeTree.contains(groupNode));
            }

            SECTION("Adding a subtree inserts all children into node tree") {
                groupNode->addChildren({entityNode, brushNode, patchNode});

                REQUIRE_FALSE(nodeTree.contains(groupNode));
                REQUIRE_FALSE(nodeTree.contains(entityNode));
                REQUIRE_FALSE(nodeTree.contains(brushNode));
                REQUIRE_FALSE(nodeTree.contains(patchNode));
                worldNode.defaultLayer()->addChild(groupNode);
                CHECK_FALSE(nodeTree.contains(groupNode));
                CHECK(nodeTree.contains(entityNode));
                CHECK(nodeTree.contains(brushNode));
                CHECK(nodeTree.contains(patchNode));
            }

            SECTION("Removing a single node removes from node tree") {
                auto* node = GENERATE_COPY(entityNode, brushNode, patchNode);

                worldNode.defaultLayer()->addChild(node);
                REQUIRE(nodeTree.contains(node));

                worldNode.defaultLayer()->removeChild(node);
                CHECK_FALSE(nodeTree.contains(node));
            }

            SECTION("Removing a nested node removes from node tree") {
                groupNode->addChildren({entityNode, brushNode, patchNode});
                worldNode.defaultLayer()->addChild(groupNode);

                auto* node = GENERATE_COPY(entityNode, brushNode, patchNode);
                REQUIRE(nodeTree.contains(node));

                groupNode->removeChild(node);
                CHECK_FALSE(nodeTree.contains(node));
            }

            SECTION("Removing a subtree removes all children from node tree") {
                groupNode->addChildren({entityNode, brushNode, patchNode});
                
                worldNode.defaultLayer()->addChild(groupNode);
                REQUIRE(nodeTree.contains(entityNode));
                REQUIRE(nodeTree.contains(brushNode));
                REQUIRE(nodeTree.contains(patchNode));

                worldNode.defaultLayer()->removeChild(groupNode);
                CHECK_FALSE(nodeTree.contains(entityNode));
                CHECK_FALSE(nodeTree.contains(brushNode));
                CHECK_FALSE(nodeTree.contains(patchNode));
            }

            SECTION("Updating a descendant updates it in node tree") {
                groupNode->addChildren({entityNode, brushNode, patchNode});
                worldNode.defaultLayer()->addChild(groupNode);
                
                REQUIRE(nodeTree.contains(entityNode));
                REQUIRE(nodeTree.contains(brushNode));
                REQUIRE(nodeTree.contains(patchNode));
                REQUIRE_THAT(nodeTree.findContainers(vm::vec3d::zero()), Catch::UnorderedEquals(std::vector<Node*>{
                    entityNode, brushNode, patchNode
                }));
                REQUIRE_THAT(nodeTree.findContainers(vm::vec3d{64, 0, 0}), Catch::UnorderedEquals(std::vector<Node*>{}));

                transformNode(*entityNode, vm::translation_matrix(vm::vec3d(64, 0, 0)), worldBounds);
                transformNode(*brushNode, vm::translation_matrix(vm::vec3d(64, 0, 0)), worldBounds);
                transformNode(*patchNode, vm::translation_matrix(vm::vec3d(64, 0, 0)), worldBounds);

                CHECK(nodeTree.contains(entityNode));
                CHECK(nodeTree.contains(brushNode));
                CHECK(nodeTree.contains(patchNode));
                CHECK_THAT(nodeTree.findContainers(vm::vec3d::zero()), Catch::UnorderedEquals(std::vector<Node*>{}));
                CHECK_THAT(nodeTree.findContainers(vm::vec3d{64, 0, 0}), Catch::UnorderedEquals(std::vector<Node*>{
                    entityNode, brushNode, patchNode
                }));
            }
        }

        TEST_CASE("WorldNodeTest.rebuildNodeTree") {
            constexpr auto worldBounds = vm::bbox3d{8192.0};
            constexpr auto mapFormat = MapFormat::Quake3;

            auto worldNode = WorldNode{Entity{}, mapFormat};
            auto* layerNode = new LayerNode{Layer{"layer"}};
            auto* groupNode = new GroupNode{Group{"group"}};
            auto* entityNode = new EntityNode{Entity{}};
            auto* brushNode = new BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};
            auto* patchNode = new PatchNode{BezierPatch{3, 3, {
                {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
                {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
                {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
            
            worldNode.addChild(layerNode);
            worldNode.defaultLayer()->addChild(entityNode);
            worldNode.defaultLayer()->addChild(groupNode);
            groupNode->addChild(brushNode);
            groupNode->addChild(patchNode);

            const auto& nodeTree = worldNode.nodeTree();
            
            REQUIRE_FALSE(nodeTree.contains(layerNode));
            REQUIRE_FALSE(nodeTree.contains(groupNode));
            REQUIRE(nodeTree.contains(entityNode));
            REQUIRE(nodeTree.contains(brushNode));
            REQUIRE(nodeTree.contains(patchNode));

            worldNode.rebuildNodeTree();
            CHECK_FALSE(nodeTree.contains(layerNode));
            CHECK_FALSE(nodeTree.contains(groupNode));
            CHECK(nodeTree.contains(entityNode));
            CHECK(nodeTree.contains(brushNode));
            CHECK(nodeTree.contains(patchNode));
        }

        TEST_CASE("WorldNodeTest.disableNodeTreeUpdates") {
            constexpr auto worldBounds = vm::bbox3d{8192.0};
            constexpr auto mapFormat = MapFormat::Quake3;

            auto worldNode = WorldNode{Entity{}, mapFormat};
            auto* layerNode = new LayerNode{Layer{"layer"}};
            auto* groupNode = new GroupNode{Group{"group"}};
            auto* entityNode = new EntityNode{Entity{}};
            auto* brushNode = new BrushNode{BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "texture").value()};
            auto* patchNode = new PatchNode{BezierPatch{3, 3, {
                {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
                {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
                {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
            
            worldNode.disableNodeTreeUpdates();
            worldNode.addChild(layerNode);
            worldNode.defaultLayer()->addChild(entityNode);
            worldNode.defaultLayer()->addChild(groupNode);
            groupNode->addChild(brushNode);

            const auto& nodeTree = worldNode.nodeTree();
            
            CHECK_FALSE(nodeTree.contains(entityNode));
            CHECK_FALSE(nodeTree.contains(brushNode));
            
            REQUIRE_FALSE(nodeTree.contains(patchNode));

            worldNode.enableNodeTreeUpdates();
            groupNode->addChild(patchNode);
            CHECK(nodeTree.contains(patchNode));
        }

        TEST_CASE("WorldNodeTest.persistentIdOfDefaultLayer", "[WorldNodeTest]") {
            auto worldNode = WorldNode{Entity{}, MapFormat::Standard};
            CHECK(worldNode.defaultLayer()->persistentId() == std::nullopt);
        }

        TEST_CASE("WorldNodeTest.setPersistentIdWhenAddingLayer", "[WorldNodeTest]") {
            auto worldNode = WorldNode{Entity{}, MapFormat::Standard};
            auto* initialLayerNode = new LayerNode{Layer{ "name"}};

            REQUIRE(initialLayerNode->persistentId() == std::nullopt);
            worldNode.addChild(initialLayerNode);
            CHECK(initialLayerNode->persistentId() == 1u);

            SECTION("Adding a layer node that already has a persistent ID") {
                const auto id = *initialLayerNode->persistentId() + 10u;

                auto* layerNodeWithId = new LayerNode{Layer{"name"}};
                layerNodeWithId->setPersistentId(id);

                worldNode.addChild(layerNodeWithId);
                CHECK(layerNodeWithId->persistentId() == id);

                auto* layerNodeWithoutId = new LayerNode{Layer{"name"}};

                worldNode.addChild(layerNodeWithoutId);
                REQUIRE(layerNodeWithoutId->persistentId() != std::nullopt);
                CHECK(*layerNodeWithoutId->persistentId() == id + 1u);

                SECTION("Adding a layer node that already has a lower persistent ID") {
                    const auto lowerId = id - 1u;

                    auto* layerNodeWithLowerId = new LayerNode{Layer{"name"}};
                    layerNodeWithLowerId->setPersistentId(lowerId);

                    worldNode.addChild(layerNodeWithLowerId);
                    REQUIRE(layerNodeWithLowerId->persistentId() == lowerId);

                    layerNodeWithoutId = new LayerNode{Layer{"name"}};

                    worldNode.addChild(layerNodeWithoutId);
                    REQUIRE(layerNodeWithoutId->persistentId() != std::nullopt);
                    CHECK(*layerNodeWithoutId->persistentId() == id + 2u);
                }
            }
        }

        TEST_CASE("WorldNodeTest.setPersistentIdWhenAddingGroup", "[WorldNodeTest]") {
            auto worldNode = WorldNode{Entity{}, MapFormat::Standard};
            auto* initialGroupNode = new GroupNode{Group{ "name"}};

            REQUIRE(initialGroupNode->persistentId() == std::nullopt);
            worldNode.defaultLayer()->addChild(initialGroupNode);
            CHECK(initialGroupNode->persistentId() == 1u);

            SECTION("Adding a layer node that already has a persistent ID") {
                const auto id = *initialGroupNode->persistentId() + 10u;

                auto* groupNodeWithId = new GroupNode{Group{ "name"}};
                groupNodeWithId->setPersistentId(id);

                worldNode.defaultLayer()->addChild(groupNodeWithId);
                CHECK(groupNodeWithId->persistentId() == id);

                auto* groupNodeWithoutId = new GroupNode{Group{ "name"}};

                worldNode.defaultLayer()->addChild(groupNodeWithoutId);
                REQUIRE(groupNodeWithoutId->persistentId() != std::nullopt);
                CHECK(*groupNodeWithoutId->persistentId() == id + 1u);

                SECTION("Adding a layer node that already has a lower persistent ID") {
                    const auto lowerId = id - 1u;

                    auto* groupNodeWithLowerId = new GroupNode{Group{ "name"}};
                    groupNodeWithLowerId->setPersistentId(lowerId);

                    worldNode.defaultLayer()->addChild(groupNodeWithLowerId);
                    REQUIRE(groupNodeWithLowerId->persistentId() == lowerId);

                    groupNodeWithoutId = new GroupNode{Group{ "name"}};

                    worldNode.defaultLayer()->addChild(groupNodeWithoutId);
                    REQUIRE(groupNodeWithoutId->persistentId() != std::nullopt);
                    CHECK(*groupNodeWithoutId->persistentId() == id + 2u);
                }
            }
        }

        TEST_CASE("WorldNodeTest.setPersistentIdsWhenAddingLayersAndGroups", "[WorldNodeTest]") {
            auto worldNode = WorldNode{Entity{}, MapFormat::Standard};

            auto* layerNode = new LayerNode{Layer{"name"}};
            worldNode.addChild(layerNode);
            CHECK(layerNode->persistentId() == 1u);

            auto* groupNode = new GroupNode{Group{"name"}};
            layerNode->addChild(groupNode);
            CHECK(groupNode->persistentId() == 2u);
        }
    }
}
