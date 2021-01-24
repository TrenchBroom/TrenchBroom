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

#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/Layer.h"
#include "Model/LayerNode.h"
#include "Model/MapFormat.h"
#include "Model/WorldNode.h"

#include "Catch2.h"

namespace TrenchBroom {
    namespace Model {
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

        TEST_CASE("WorldNodeTest.setSharedPersistentIdWhenAddingGroup", "[WorldNodeTest]") {
            auto worldNode = WorldNode{Entity{}, MapFormat::Standard};

            auto* groupNode = new GroupNode{Group{"name"}};
            worldNode.defaultLayer()->addChild(groupNode);

            CHECK(groupNode->sharedPersistentId());

            auto* linkedGroupNode = new GroupNode{Group{"name"}};
            groupNode->addToLinkSet(*linkedGroupNode);
            worldNode.defaultLayer()->addChild(linkedGroupNode);

            CHECK(linkedGroupNode->sharedPersistentId() == groupNode->sharedPersistentId());
        }
    }
}
