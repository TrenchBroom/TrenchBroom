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

#include "TestUtils.h"

#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"

#include <vecmath/bbox.h>
#include <vecmath/bbox_io.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>

#include <kdl/result.h>

#include <memory>
#include <vector>

#include "Catch2.h"

namespace TrenchBroom {
    namespace Model {
        TEST_CASE("GroupNodeTest.constructor", "[GroupNodeTest]") {
            auto groupNode = GroupNode(Group("name"));
            CHECK(!groupNode.connectedToLinkSet());
            CHECK_THAT(groupNode.linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{}));
        }

        TEST_CASE("GroupNodeTest.clone", "[GroupNodeTest]") {
            const auto worldBounds = vm::bbox3d(8192.0);
            auto groupNode = GroupNode(Group("name"));
            groupNode.connectToLinkSet();

            SECTION("Cloning a group node with a singleton link set") {
                auto groupNodeClone = std::unique_ptr<GroupNode>{static_cast<GroupNode*>(groupNode.clone(worldBounds))};
                CHECK_FALSE(inSameLinkSet(groupNode, *groupNodeClone));
                CHECK_THAT(groupNodeClone->linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{}));

                SECTION("Cloning a group node with a non singleton link set") {
                    groupNode.addToLinkSet(*groupNodeClone);
                    REQUIRE(inSameLinkSet(groupNode, *groupNodeClone));
                    
                    groupNodeClone->connectToLinkSet();
                    REQUIRE_THAT(groupNodeClone->linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{&groupNode, groupNodeClone.get()}));

                    auto linkedGroupNodeClone = std::unique_ptr<GroupNode>{static_cast<GroupNode*>(groupNode.clone(worldBounds))};
                    linkedGroupNodeClone->connectToLinkSet();
                    CHECK(inSameLinkSet(groupNode, *linkedGroupNodeClone));
                    CHECK_THAT(groupNode.linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{&groupNode, groupNodeClone.get(), linkedGroupNodeClone.get()}));
                }
            }
        }

        TEST_CASE("GroupNodeTest.cloneRecursively", "[GroupNodeTest]") {
            const auto worldBounds = vm::bbox3d(8192.0);
            auto groupNode = GroupNode(Group("name"));
            groupNode.connectToLinkSet();

            SECTION("Cloning a group node with a singleton link set") {
                auto groupNodeClone = std::unique_ptr<GroupNode>{static_cast<GroupNode*>(groupNode.cloneRecursively(worldBounds))};
                CHECK_FALSE(inSameLinkSet(groupNode, *groupNodeClone));
                CHECK_THAT(groupNodeClone->linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{}));

                SECTION("Cloning a group node with a non singleton link set") {
                    groupNode.addToLinkSet(*groupNodeClone);
                    REQUIRE(inSameLinkSet(groupNode, *groupNodeClone));
                    
                    groupNodeClone->connectToLinkSet();
                    REQUIRE_THAT(groupNodeClone->linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{&groupNode, groupNodeClone.get()}));

                    auto linkedGroupNodeClone = std::unique_ptr<GroupNode>{static_cast<GroupNode*>(groupNode.cloneRecursively(worldBounds))};
                    linkedGroupNodeClone->connectToLinkSet();
                    CHECK(inSameLinkSet(groupNode, *linkedGroupNodeClone));
                    CHECK_THAT(groupNode.linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{&groupNode, groupNodeClone.get(), linkedGroupNodeClone.get()}));
                }
            }
        }

        TEST_CASE("GroupNodeTest.cloneRecursivelyWithoutLinking", "[GroupNodeTest]") {
            const auto worldBounds = vm::bbox3d(8192.0);
            auto groupNode = GroupNode(Group("name"));
            groupNode.connectToLinkSet();

            SECTION("Cloning a group node with a singleton link set") {
                auto groupNodeClone = groupNode.cloneRecursivelyWithoutLinking(worldBounds);
                CHECK_FALSE(inSameLinkSet(groupNode, *groupNodeClone));
                CHECK_THAT(groupNodeClone->linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{}));

                SECTION("Cloning a group node with a non singleton link set") {
                    groupNode.addToLinkSet(*groupNodeClone);
                    REQUIRE(inSameLinkSet(groupNode, *groupNodeClone));
                    
                    groupNodeClone->connectToLinkSet();
                    REQUIRE_THAT(groupNodeClone->linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{&groupNode, groupNodeClone.get()}));

                    auto linkedGroupNodeClone = groupNode.cloneRecursivelyWithoutLinking(worldBounds);
                    linkedGroupNodeClone->connectToLinkSet();

                    CHECK_FALSE(inSameLinkSet(groupNode, *linkedGroupNodeClone));
                    CHECK_THAT(groupNode.linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{&groupNode, groupNodeClone.get()}));
                    CHECK_THAT(linkedGroupNodeClone->linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{linkedGroupNodeClone.get()}));
                }
            }
        }

        TEST_CASE("GroupNodeTest.inSameLinkSet", "[GroupNodeTest]") {
            auto groupNode = GroupNode(Group("name"));
            CHECK(inSameLinkSet(groupNode, groupNode));

            auto otherNode = GroupNode(Group("other"));
            groupNode.addToLinkSet(otherNode);
            CHECK(inSameLinkSet(groupNode, otherNode));
            CHECK(inSameLinkSet(otherNode, groupNode));

            CHECK_FALSE(inSameLinkSet(GroupNode(Group("other")), groupNode));
            CHECK_FALSE(inSameLinkSet(groupNode, GroupNode(Group("other"))));
            CHECK_FALSE(inSameLinkSet(GroupNode(Group("other")), otherNode));
            CHECK_FALSE(inSameLinkSet(otherNode, GroupNode(Group("other"))));
        }

        TEST_CASE("GroupNodeTest.addToLinkSet", "[GroupNodeTest]") {
            auto groupNode = GroupNode(Group("name"));
            auto otherNode = GroupNode(Group("other"));
            
            REQUIRE_FALSE(inSameLinkSet(otherNode, groupNode));
            REQUIRE_FALSE(otherNode.connectedToLinkSet());

            groupNode.addToLinkSet(otherNode);
            CHECK(inSameLinkSet(otherNode, groupNode));
            CHECK_FALSE(otherNode.connectedToLinkSet());
        }

        TEST_CASE("GroupNodeTest.connectToLinkSet", "[GroupNodeTest]") {
            auto groupNode = GroupNode(Group("name"));
            REQUIRE_FALSE(groupNode.connectedToLinkSet());
            REQUIRE_THAT(groupNode.linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{}));

            groupNode.connectToLinkSet();
            CHECK(groupNode.connectedToLinkSet());
            CHECK_THAT(groupNode.linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{&groupNode}));
        }

        TEST_CASE("GroupNodeTest.disconnectFromLinkSet", "[GroupNodeTest]") {
            auto groupNode = GroupNode(Group("name"));
            groupNode.connectToLinkSet();
            REQUIRE(groupNode.connectedToLinkSet());
            REQUIRE_THAT(groupNode.linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{&groupNode}));

            groupNode.disconnectFromLinkSet();
            CHECK_FALSE(groupNode.connectedToLinkSet());
            CHECK_THAT(groupNode.linkedGroups(), Catch::UnorderedEquals(std::vector<GroupNode*>{}));
        }

        TEST_CASE("GroupNodeTest.transform", "[GroupNodeTest]") {
            const auto worldBounds = vm::bbox3(8192.0);

            auto groupNode = GroupNode(Group("name"));
            groupNode.connectToLinkSet();
            REQUIRE(groupNode.group().transformation() == vm::mat4x4());

            auto* entityNode = new EntityNode();
            groupNode.addChild(entityNode);
            
            transformNode(groupNode, vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)), worldBounds);
            CHECK(groupNode.group().transformation() == vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)));

            transformNode(groupNode, vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0)), worldBounds);
            CHECK(groupNode.group().transformation() == vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0)) * vm::translation_matrix(vm::vec3(32.0, 0.0, 0.0)));

            auto testEntityNode = EntityNode();
            transformNode(testEntityNode, groupNode.group().transformation(), worldBounds);

            CHECK(testEntityNode.entity() == entityNode->entity());
        }
    }
}
