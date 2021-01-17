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
#include "Model/UpdateLinkedGroupsError.h"

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
        TEST_CASE("GroupNodeTest.updateLinkedGroups", "[GroupNodeTest]") {
            const auto worldBounds = vm::bbox3(8192.0);
            
            auto groupNode = GroupNode{Group{"name"}};
            auto* entityNode = new EntityNode{};
            groupNode.addChild(entityNode);

            transformNode(groupNode, vm::translation_matrix(vm::vec3(1.0, 0.0, 0.0)), worldBounds);
            REQUIRE(groupNode.group().transformation() == vm::translation_matrix(vm::vec3(1.0, 0.0, 0.0)));
            REQUIRE(entityNode->entity().origin() == vm::vec3(1.0, 0.0, 0.0));

            SECTION("Target group list is empty") {
                const auto updateResult = updateLinkedGroups(groupNode, {}, worldBounds);
                updateResult.visit(kdl::overload(
                    [&](const UpdateLinkedGroupsResult& r) {
                        CHECK(r.empty());
                    },
                    [](const auto&) {
                        FAIL();
                    }
                ));
            }

            SECTION("Target group list contains only source group") {
                const auto updateResult = updateLinkedGroups(groupNode, {&groupNode}, worldBounds);
                updateResult.visit(kdl::overload(
                    [&](const UpdateLinkedGroupsResult& r) {
                        CHECK(r.empty());
                    },
                    [](const auto&) {
                        FAIL();
                    }
                ));
            }

            SECTION("Update a single target group") {
                auto groupNodeClone = std::unique_ptr<GroupNode>{static_cast<GroupNode*>(groupNode.cloneRecursively(worldBounds))};
                REQUIRE(groupNodeClone->group().transformation() == vm::translation_matrix(vm::vec3(1.0, 0.0, 0.0)));

                transformNode(*groupNodeClone, vm::translation_matrix(vm::vec3(0.0, 2.0, 0.0)), worldBounds);
                REQUIRE(groupNodeClone->group().transformation() == vm::translation_matrix(vm::vec3(1.0, 2.0, 0.0)));
                REQUIRE(static_cast<EntityNode*>(groupNodeClone->children().front())->entity().origin() == vm::vec3(1.0, 2.0, 0.0));


                transformNode(*entityNode, vm::translation_matrix(vm::vec3(0.0, 0.0, 3.0)), worldBounds);
                REQUIRE(entityNode->entity().origin() == vm::vec3(1.0, 0.0, 3.0));

                const auto updateResult = updateLinkedGroups(groupNode, {groupNodeClone.get()}, worldBounds);
                updateResult.visit(kdl::overload(
                    [&](const UpdateLinkedGroupsResult& r) {
                        CHECK(r.size() == 1u);

                        const auto& p = r.front();
                        const auto& [groupNodeToUpdate, newChildren] = p;

                        CHECK(groupNodeToUpdate == groupNodeClone.get());
                        CHECK(newChildren.size() == 1u);

                        const auto* newEntityNode = dynamic_cast<EntityNode*>(newChildren.front().get());
                        CHECK(newEntityNode != nullptr);

                        CHECK(newEntityNode->entity().origin() == vm::vec3(1.0, 2.0, 3.0));
                    },
                    [](const auto&) {
                        FAIL();
                    }
                ));
            }
        }

        TEST_CASE("GroupNodeTest.updateNestedLinkedGroups", "[GroupNodeTest]") {
            const auto worldBounds = vm::bbox3(8192.0);
            
            auto outerGroupNode = GroupNode{Group{"outer"}};
            auto* innerGroupNode = new GroupNode{Group{"inner"}};
            outerGroupNode.addChild(innerGroupNode);

            auto* innerGroupEntityNode = new EntityNode();
            innerGroupNode->addChild(innerGroupEntityNode);

            auto innerGroupNodeClone = std::unique_ptr<GroupNode>{static_cast<GroupNode*>(innerGroupNode->cloneRecursively(worldBounds))};
            REQUIRE(innerGroupNodeClone->group().transformation() == vm::mat4x4());

            transformNode(*innerGroupNodeClone, vm::translation_matrix(vm::vec3(0.0, 2.0, 0.0)), worldBounds);
            REQUIRE(innerGroupNodeClone->group().transformation() == vm::translation_matrix(vm::vec3(0.0, 2.0, 0.0)));

            SECTION("Transforming the inner group node and updating the linked group") {
                transformNode(*innerGroupNode, vm::translation_matrix(vm::vec3(1.0, 0.0, 0.0)), worldBounds);
                REQUIRE(outerGroupNode.group().transformation() == vm::mat4x4());
                REQUIRE(innerGroupNode->group().transformation() == vm::translation_matrix(vm::vec3(1.0, 0.0, 0.0)));
                REQUIRE(innerGroupEntityNode->entity().origin() == vm::vec3(1.0, 0.0, 0.0));
                REQUIRE(innerGroupNodeClone->group().transformation() == vm::translation_matrix(vm::vec3(0.0, 2.0, 0.0)));

                const auto updateResult = updateLinkedGroups(*innerGroupNode, {innerGroupNodeClone.get()}, worldBounds);
                updateResult.visit(kdl::overload(
                    [&](const UpdateLinkedGroupsResult& r) {
                        CHECK(r.size() == 1u);

                        const auto& p = r.front();
                        const auto& [groupNodeToUpdate, newChildren] = p;

                        CHECK(groupNodeToUpdate == innerGroupNodeClone.get());
                        CHECK(newChildren.size() == 1u);

                        const auto* newEntityNode = dynamic_cast<EntityNode*>(newChildren.front().get());
                        CHECK(newEntityNode != nullptr);

                        CHECK(newEntityNode->entity().origin() == vm::vec3(0.0, 2.0, 0.0));
                    },
                    [](const auto&) {
                        FAIL();
                    }
                ));
            }

            SECTION("Transforming the inner group node's entity and updating the linked group") {
                transformNode(*innerGroupEntityNode, vm::translation_matrix(vm::vec3(1.0, 0.0, 0.0)), worldBounds);
                REQUIRE(outerGroupNode.group().transformation() == vm::mat4x4());
                REQUIRE(innerGroupNode->group().transformation() == vm::mat4x4());
                REQUIRE(innerGroupEntityNode->entity().origin() == vm::vec3(1.0, 0.0, 0.0));
                REQUIRE(innerGroupNodeClone->group().transformation() == vm::translation_matrix(vm::vec3(0.0, 2.0, 0.0)));

                const auto updateResult = updateLinkedGroups(*innerGroupNode, {innerGroupNodeClone.get()}, worldBounds);
                updateResult.visit(kdl::overload(
                    [&](const UpdateLinkedGroupsResult& r) {
                        CHECK(r.size() == 1u);

                        const auto& p = r.front();
                        const auto& [groupNodeToUpdate, newChildren] = p;

                        CHECK(groupNodeToUpdate == innerGroupNodeClone.get());
                        CHECK(newChildren.size() == 1u);

                        const auto* newEntityNode = dynamic_cast<EntityNode*>(newChildren.front().get());
                        CHECK(newEntityNode != nullptr);

                        CHECK(newEntityNode->entity().origin() == vm::vec3(1.0, 2.0, 0.0));
                    },
                    [](const auto&) {
                        FAIL();
                    }
                ));
            }
        }

        TEST_CASE("GroupNodeTest.updateLinkedGroupsRecursively", "[GroupNodeTest]") {
            const auto worldBounds = vm::bbox3(8192.0);
            
            auto outerGroupNode = GroupNode{Group{"outer"}};

            /*
            outerGroupNode
            */

            auto* innerGroupNode = new GroupNode{Group{"inner"}};
            outerGroupNode.addChild(innerGroupNode);

            /*
            outerGroupNode
            +- innerGroupNode
            */

            auto* innerGroupEntityNode = new EntityNode{};
            innerGroupNode->addChild(innerGroupEntityNode);

            /*
            outerGroupNode
            +-innerGroupNode
               +-innerGroupEntityNode
            */

            auto outerGroupNodeClone = std::unique_ptr<GroupNode>{static_cast<GroupNode*>(outerGroupNode.cloneRecursively(worldBounds))};
            REQUIRE(outerGroupNodeClone->group().transformation() == vm::mat4x4());
            REQUIRE(outerGroupNodeClone->childCount() == 1u);

            /*
            outerGroupNode
            +-innerGroupNode
               +-innerGroupEntityNode
            outerGroupNodeClone
            +-innerGroupNodeClone
               +-innerGroupEntityNodeClone
            */

            auto* innerGroupNodeClone = dynamic_cast<GroupNode*>(outerGroupNodeClone->children().front());
            REQUIRE(innerGroupNodeClone != nullptr);
            REQUIRE(innerGroupNodeClone->childCount() == 1u);

            auto* innerGroupEntityNodeClone = dynamic_cast<EntityNode*>(innerGroupNodeClone->children().front());
            REQUIRE(innerGroupEntityNodeClone != nullptr);

            const auto updateResult = updateLinkedGroups(outerGroupNode, {outerGroupNodeClone.get()}, worldBounds);
            updateResult.visit(kdl::overload(
                [&](const UpdateLinkedGroupsResult& r) {
                    REQUIRE(r.size() == 1u);
                    const auto& [groupNodeToUpdate, newChildren] = r.front();

                    REQUIRE(groupNodeToUpdate == outerGroupNodeClone.get());
                    REQUIRE(newChildren.size() == 1u);

                    auto* newInnerGroupNodeClone = dynamic_cast<GroupNode*>(newChildren.front().get());
                    CHECK(newInnerGroupNodeClone != nullptr);
                    CHECK(newInnerGroupNodeClone->group() == innerGroupNode->group());
                    CHECK(newInnerGroupNodeClone->childCount() == 1u);

                    auto* newInnerGroupEntityNodeClone = dynamic_cast<EntityNode*>(newInnerGroupNodeClone->children().front());
                    CHECK(newInnerGroupEntityNodeClone != nullptr);
                    CHECK(newInnerGroupEntityNodeClone->entity() == innerGroupEntityNode->entity());
                },
                [](const auto&) {
                    FAIL();
                }
            ));
        }

        TEST_CASE("GroupNodeTest.updateLinkedGroupsExceedsWorldBounds", "[GroupNodeTest]") {
            const auto worldBounds = vm::bbox3(8192.0);
            
            auto groupNode = GroupNode{Group{"name"}};
            auto* entityNode = new EntityNode{};
            groupNode.addChild(entityNode);


            auto groupNodeClone = std::unique_ptr<GroupNode>{static_cast<GroupNode*>(groupNode.cloneRecursively(worldBounds))};

            transformNode(*groupNodeClone, vm::translation_matrix(vm::vec3(8192.0 - 8.0, 0.0, 0.0)), worldBounds);
            REQUIRE(groupNodeClone->children().front()->logicalBounds() == vm::bbox3(vm::vec3(8192.0 - 16.0, -8.0, -8.0), vm::vec3(8192.0, 8.0, 8.0)));


            transformNode(*entityNode, vm::translation_matrix(vm::vec3(1.0, 0.0, 0.0)), worldBounds);
            REQUIRE(entityNode->entity().origin() == vm::vec3(1.0, 0.0, 0.0));

            const auto updateResult = updateLinkedGroups(groupNode, {groupNodeClone.get()}, worldBounds);
            updateResult.visit(kdl::overload(
                [&](const UpdateLinkedGroupsResult&) {
                    FAIL();
                },
                [](const BrushError&) {
                    FAIL();
                },
                [](const UpdateLinkedGroupsError& e) {
                    CHECK(e == UpdateLinkedGroupsError::UpdateExceedsWorldBounds);
                }
            ));
        }

        static void setGroupName(GroupNode& groupNode, const std::string& name) {
            auto group = groupNode.group();
            group.setName(name);
            groupNode.setGroup(std::move(group));
        }

        TEST_CASE("GroupNodeTest.updateLinkedGroupsAndPreserveNestedGroupNames", "[GroupNodeTest]") {
            const auto worldBounds = vm::bbox3(8192.0);

            auto outerGroupNode = GroupNode{Group{"outerGroupNode"}};
            auto* innerGroupNode = new GroupNode{Group{"innerGroupNode"}};
            outerGroupNode.addChild(innerGroupNode);

            auto innerGroupNodeClone = std::unique_ptr<GroupNode>(static_cast<GroupNode*>(innerGroupNode->cloneRecursively(worldBounds)));
            setGroupName(*innerGroupNodeClone, "innerGroupNodeClone");

            auto outerGroupNodeClone = std::unique_ptr<GroupNode>(static_cast<GroupNode*>(outerGroupNode.cloneRecursively(worldBounds)));
            setGroupName(*outerGroupNodeClone, "outerGroupNodeClone");

            auto* innerGroupNodeNestedClone = static_cast<GroupNode*>(outerGroupNodeClone->children().front());
            setGroupName(*innerGroupNodeNestedClone, "innerGroupNodeNestedClone");

            /*
            outerGroupNode-------+
            +-innerGroupNode-----|-------+
            innerGroupNodeClone--|-------+
            outerGroupNodeClone--+       |
            +-innerGroupNodeNestedClone--+
             */

            SECTION("Updating outerGroupNode retains the names of its linked group and the nested linked group") {
                const auto updateResult = updateLinkedGroups(outerGroupNode, {outerGroupNodeClone.get()}, worldBounds);
                updateResult.visit(kdl::overload(
                [&](const UpdateLinkedGroupsResult& r) {
                    REQUIRE(r.size() == 1u);

                    const auto& [groupNodeToUpdate, newChildren] = r.front();
                    REQUIRE(groupNodeToUpdate == outerGroupNodeClone.get());

                    const auto* innerReplacement = static_cast<GroupNode*>(newChildren.front().get());
                    CHECK(innerReplacement->name() == innerGroupNodeNestedClone->name());
                },
                [](const auto&) {
                    FAIL();
                }
                ));
            }
        }

        TEST_CASE("GroupNodeTest.updateLinkedGroupsAndPreserveEntityProperties", "[GroupNodeTest]") {
            const auto worldBounds = vm::bbox3(8192.0);

            auto sourceGroupNode = GroupNode{Group{"name"}};
            auto* sourceEntityNode = new EntityNode{};
            sourceGroupNode.addChild(sourceEntityNode);

            auto targetGroupNode = std::unique_ptr<GroupNode>{static_cast<GroupNode*>(sourceGroupNode.cloneRecursively(worldBounds))};

            auto* targetEntityNode = static_cast<EntityNode*>(targetGroupNode->children().front());
            REQUIRE_THAT(targetEntityNode->entity().properties(), Catch::Equals(sourceEntityNode->entity().properties()));

            using T = std::tuple<std::vector<std::string>, std::vector<std::string>, std::vector<EntityProperty>, std::vector<EntityProperty>, std::vector<EntityProperty>>;

            const auto
            [ srcProtProperties, trgtProtProperties, sourceProperties, 
                                                     targetProperties, 
                                                     expectedProperties ] = GENERATE(values<T>({
            // properties remain unchanged
            { {},                {},                 { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } } },

            { {},                { "some_key" },     { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } } },

            { { "some_key" },    {},                 { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } } },

            { { "some_key" },    { "some_key" },     { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } } },

            // property was added to source
            { {},                {},                 { { "some_key", "some_value" } },
                                                     {},
                                                     { { "some_key", "some_value" } } },

            { {},                { "some_key" },     { { "some_key", "some_value" } },
                                                     {},
                                                     {} },

            { { "some_key" },    {},                 { { "some_key", "some_value" } },
                                                     {},
                                                     {} },

            { { "some_key" },    { "some_key" },     { { "some_key", "some_value" } },
                                                     {},
                                                     {} },

            // property was changed in source
            { {},                {},                 { { "some_key", "other_value" } },
                                                     { { "some_key", "some_value" } },
                                                     { { "some_key", "other_value" } } },

            { { "some_key" },    {},                 { { "some_key", "other_value" } },
                                                     { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } } },

            { {},                { "some_key" },     { { "some_key", "other_value" } },
                                                     { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } } },

            { { "some_key" },    { "some_key" },     { { "some_key", "other_value" } },
                                                     { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } } },

            // property was removed in source
            { {},                {},                 {},
                                                     { { "some_key", "some_value" } },
                                                     {} },

            { { "some_key" },    {},                 {},
                                                     { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } } },

            { {},                { "some_key" },     {},
                                                     { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } } },

            { { "some_key" },    { "some_key" },     {},
                                                     { { "some_key", "some_value" } },
                                                     { { "some_key", "some_value" } } },
            }));

            CAPTURE(srcProtProperties, trgtProtProperties, sourceProperties, targetProperties, expectedProperties);

            {
                auto entity = sourceEntityNode->entity();
                entity.setProperties(sourceProperties);
                entity.setProtectedProperties(srcProtProperties);
                sourceEntityNode->setEntity(std::move(entity));
            }

            {
                auto entity = targetEntityNode->entity();
                entity.setProperties(targetProperties);
                entity.setProtectedProperties(trgtProtProperties);
                targetEntityNode->setEntity(std::move(entity));
            }

            // lambda can't capture structured bindings
            const auto expectedTargetProperties = expectedProperties;

            const auto updateResult = updateLinkedGroups(sourceGroupNode, {targetGroupNode.get()}, worldBounds);
            updateResult.visit(kdl::overload(
                [&](const UpdateLinkedGroupsResult& r) {
                    REQUIRE(r.size() == 1u);
                    const auto& p = r.front();

                    const auto& newChildren = p.second;
                    REQUIRE(newChildren.size() == 1u);

                    const auto* newEntityNode = dynamic_cast<EntityNode*>(newChildren.front().get());
                    REQUIRE(newEntityNode != nullptr);

                    CHECK_THAT(newEntityNode->entity().properties(), Catch::UnorderedEquals(expectedTargetProperties));
                    CHECK_THAT(newEntityNode->entity().protectedProperties(), Catch::UnorderedEquals(targetEntityNode->entity().protectedProperties()));
                },
                [](const auto&) {
                    FAIL();
                }
            ));
        }
    }
}
