/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "Model/BrushBuilder.h"
#include "Model/EditorContext.h"
#include "Model/LockState.h"
#include "Model/VisibilityState.h"
#include "Model/World.h"
#include "Model/Layer.h"
#include "Model/Group.h"
#include "Model/Entity.h"
#include "Model/BrushNode.h"

namespace TrenchBroom {
    namespace Model {
        class EditorContextTest {
        protected:
            vm::bbox3d worldBounds;
            World* world;
            EditorContext context;

            EditorContextTest() {
                worldBounds = vm::bbox3d(8192.0);
                world = new World(MapFormat::Standard);
            }

            virtual ~EditorContextTest() {
                context.reset();
                delete world;
                world = nullptr;
            }

            Group* createTopLevelGroup() {
                Group* group;
                std::tie(group, std::ignore) = createGroupedBrush();
                return group;
            }

            Entity* createTopLevelPointEntity() {
                auto* entity = world->createEntity();
                world->defaultLayer()->addChild(entity);
                return entity;
            }

            std::tuple<Entity*, BrushNode*> createTopLevelBrushEntity() {
                BrushBuilder builder(world, worldBounds);
                auto* brush = builder.createCube(32.0, "sometex");
                auto* entity = world->createEntity();
                entity->addChild(brush);
                world->defaultLayer()->addChild(entity);
                return std::make_tuple(entity, brush);
            }

            BrushNode* createTopLevelBrush() {
                BrushBuilder builder(world, worldBounds);
                auto* brush = builder.createCube(32.0, "sometex");
                world->defaultLayer()->addChild(brush);
                return brush;
            }

            std::tuple<Group*, Group*> createNestedGroup() {
                Group* outerGroup;
                Group* innerGroup;
                std::tie(outerGroup, innerGroup, std::ignore) = createdNestedGroupedBrush();

                return std::make_tuple(outerGroup, innerGroup);
            }

            std::tuple<Group*, BrushNode*> createGroupedBrush() {
                BrushBuilder builder(world, worldBounds);
                auto* brush = builder.createCube(32.0, "sometex");
                auto* group = world->createGroup("somegroup");

                group->addChild(brush);
                world->defaultLayer()->addChild(group);

                return std::make_tuple(group, brush);
            }

            std::tuple<Group*, Entity*> createGroupedPointEntity() {
                BrushBuilder builder(world, worldBounds);
                auto* entity = world->createEntity();
                auto* group = world->createGroup("somegroup");

                group->addChild(entity);
                world->defaultLayer()->addChild(group);

                return std::make_tuple(group, entity);
            }

            std::tuple<Group*, Entity*, BrushNode*> createGroupedBrushEntity() {
                BrushBuilder builder(world, worldBounds);
                auto* brush = builder.createCube(32.0, "sometex");
                auto* entity = world->createEntity();
                auto* group = world->createGroup("somegroup");

                entity->addChild(brush);
                group->addChild(entity);
                world->defaultLayer()->addChild(group);

                return std::make_tuple(group, entity, brush);
            }

            std::tuple<Group*, Group*, BrushNode*> createdNestedGroupedBrush() {
                BrushBuilder builder(world, worldBounds);
                auto* innerBrush = builder.createCube(32.0, "sometex");
                auto* innerGroup = world->createGroup("inner");
                auto* outerGroup = world->createGroup("outer");

                innerGroup->addChild(innerBrush);
                outerGroup->addChild(innerGroup);
                world->defaultLayer()->addChild(outerGroup);

                return std::make_tuple(outerGroup, innerGroup, innerBrush);
            }

            void assertVisible(const bool expected, Node* node, const VisibilityState visibilityState, const LockState lockState) {
                setState(node, visibilityState, lockState);
                ASSERT_EQ(expected, context.visible(node));
            }

            void assertEditable(const bool expected, Node* node, const VisibilityState visibilityState, const LockState lockState) {
                setState(node, visibilityState, lockState);
                ASSERT_EQ(expected, context.editable(node));
            }

            void assertPickable(const bool expected, Node* node, const VisibilityState visibilityState, const LockState lockState) {
                setState(node, visibilityState, lockState);
                ASSERT_EQ(expected, context.pickable(node));
            }

            void assertSelectable(const bool expected, Node* node, const VisibilityState visibilityState, const LockState lockState) {
                setState(node, visibilityState, lockState);
                ASSERT_EQ(expected, context.selectable(node));
            }
        private:
            void setState(Node* node, const VisibilityState visibilityState, const LockState lockState) {
                node->setVisibilityState(visibilityState);
                node->setLockState(lockState);
            }
        };

        /************* World Tests *************/

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testWorldVisible") {
            assertVisible(true, world, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, world, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, world, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, world, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testWorldEditable") {
            assertEditable(true, world, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, world, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, world, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, world, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testWorldPickable") {
            assertPickable(false, world, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(false, world, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, world, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, world, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testWorldSelectable") {
            assertSelectable(false, world, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, world, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, world, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, world, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        /************* Default Layer Tests *************/

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testDefaultLayerVisible") {
            auto* layer = world->defaultLayer();
            assertVisible(true, layer, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, layer, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, layer, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, layer, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testDefaultLayerEditable") {
            auto* layer = world->defaultLayer();
            assertEditable(true, layer, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, layer, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, layer, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, layer, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testDefaultLayerPickable") {
            auto* layer = world->defaultLayer();
            assertPickable(false, layer, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(false, layer, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, layer, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, layer, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testDefaultLayerSelectable") {
            auto* layer = world->defaultLayer();
            assertSelectable(false, layer, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, layer, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, layer, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, layer, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        /************* Top Level Group Tests *************/

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelGroupVisible") {
            auto* group = createTopLevelGroup();
            assertVisible(true, group, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, group, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, group, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, group, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertVisible(true, group, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            context.popGroup();

            group->select();
            assertVisible(true, group, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelGroupEditable") {
            auto* group = createTopLevelGroup();
            assertEditable(true, group, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, group, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, group, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, group, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertEditable(true, group, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, group, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelGroupPickable") {
            auto* group = createTopLevelGroup();
            assertPickable(true, group, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, group, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, group, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, group, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertPickable(false, group, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(false, group, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelGroupSelectable") {
            auto* group = createTopLevelGroup();
            assertSelectable(true, group, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, group, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, group, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, group, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertSelectable(false, group, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, group, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
        }

        /************* Top Level Point Entity Tests *************/

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelPointEntityVisible") {
            auto* entity = createTopLevelPointEntity();
            assertVisible(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            entity->select();
            assertVisible(true, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            entity->deselect();

            context.setShowPointEntities(false);
            assertVisible(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelPointEntityEditable") {
            auto* entity = createTopLevelPointEntity();
            assertEditable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelPointEntityPickable") {
            auto* entity = createTopLevelPointEntity();
            assertPickable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelPointEntitySelectable") {
            auto* entity = createTopLevelPointEntity();
            assertSelectable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        /************* Top Level Brush Entity Tests *************/

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelBrushEntityVisible") {
            Entity* entity;
            BrushNode* brush;
            std::tie(entity, brush) = createTopLevelBrushEntity();

            assertVisible(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            brush->setVisibilityState(VisibilityState::Visibility_Hidden);
            brush->setLockState(LockState::Lock_Unlocked);
            assertVisible(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelBrushEntityEditable") {
            Entity* entity;
            BrushNode* brush;
            std::tie(entity, brush) = createTopLevelBrushEntity();

            assertEditable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertEditable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelBrushEntityPickable") {
            Entity* entity;
            BrushNode* brush;
            std::tie(entity, brush) = createTopLevelBrushEntity();

            assertPickable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelBrushEntitySelectable") {
            Entity* entity;
            BrushNode* brush;
            std::tie(entity, brush) = createTopLevelBrushEntity();

            assertSelectable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertSelectable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        /************* Top Level Brush Tests *************/

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelBrushVisible") {
            auto* brush = createTopLevelBrush();
            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            brush->select();
            assertVisible(true, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelBrushEditable") {
            auto* brush = createTopLevelBrush();
            assertEditable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelBrushPickable") {
            auto* brush = createTopLevelBrush();
            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testTopLevelBrushSelectable") {
            auto* brush = createTopLevelBrush();
            assertSelectable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);
        }

        /************* Nested Group Tests *************/

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testNestedGroupVisible") {
            Group *outer, *inner;
            std::tie(outer, inner) = createNestedGroup();

            assertVisible(true, inner, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, inner, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, inner, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, inner, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(outer);
            assertVisible(true, inner, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            context.pushGroup(inner);
            assertVisible(true, inner, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            context.popGroup();
            inner->select();
            assertVisible(true, inner, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            inner->deselect();
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testNestedGroupEditable") {
            Group *outer, *inner;
            std::tie(outer, inner) = createNestedGroup();

            assertEditable(true, inner, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, inner, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, inner, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, inner, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(outer);
            assertEditable(true, inner, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, inner, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.pushGroup(inner);
            assertEditable(true, inner, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, inner, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testNestedGroupPickable") {
            Group *outer, *inner;
            std::tie(outer, inner) = createNestedGroup();

            assertPickable(false, inner, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(false, inner, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, inner, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, inner, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(outer);
            assertPickable(true, inner, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, inner, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.pushGroup(inner);
            assertPickable(false, inner, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(false, inner, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testNestedGroupSelectable") {
            Group *outer, *inner;
            std::tie(outer, inner) = createNestedGroup();

            assertSelectable(false, inner, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, inner, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, inner, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, inner, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(outer);
            assertSelectable(true, inner, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, inner, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.pushGroup(inner);
            assertSelectable(false, inner, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, inner, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
            context.popGroup();
        }

        /************* Grouped Brush Tests *************/

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedBrushVisible") {
            Group* group;
            BrushNode* brush;
            std::tie(group, brush) = createGroupedBrush();

            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            brush->select();
            assertVisible(true, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            brush->deselect();
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedBrushEditable") {
            Group* group;
            BrushNode* brush;
            std::tie(group, brush) = createGroupedBrush();

            assertEditable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertEditable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedBrushPickable") {
            Group* group;
            BrushNode* brush;
            std::tie(group, brush) = createGroupedBrush();

            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedBrushSelectable") {
            Group* group;
            BrushNode* brush;
            std::tie(group, brush) = createGroupedBrush();

            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertSelectable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
        }

        /************* Grouped Point Entity Tests *************/

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedPointEntityVisible") {
            Group* group;
            Entity* entity;
            std::tie(group, entity) = createGroupedPointEntity();

            assertVisible(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertVisible(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            entity->select();
            assertVisible(true, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            entity->deselect();

            context.setShowPointEntities(false);
            assertVisible(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);

            context.popGroup();

            assertVisible(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedPointEntityEditable") {
            Group* group;
            Entity* entity;
            std::tie(group, entity) = createGroupedPointEntity();

            assertEditable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertEditable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedPointEntityPickable") {
            Group* group;
            Entity* entity;
            std::tie(group, entity) = createGroupedPointEntity();

            assertPickable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertPickable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedPointEntitySelectable") {
            Group* group;
            Entity* entity;
            std::tie(group, entity) = createGroupedPointEntity();

            assertSelectable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertSelectable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
        }

        /************* Grouped Brush Entity Tests *************/

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedBrushEntityVisible") {
            Group* group;
            Entity* entity;
            BrushNode* brush;
            std::tie(group, entity, brush) = createGroupedBrushEntity();

            assertVisible(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            // test brush first to leave it visible, which influences the entity's visibility
            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedBrushEntityEditable") {
            Group* group;
            Entity* entity;
            BrushNode* brush;
            std::tie(group, entity, brush) = createGroupedBrushEntity();

            assertEditable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertEditable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertEditable(true, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedBrushEntityPickable") {
            Group* group;
            Entity* entity;
            BrushNode* brush;
            std::tie(group, entity, brush) = createGroupedBrushEntity();

            assertPickable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertPickable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
        }

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testGroupedBrushEntitySelectable") {
            Group* group;
            Entity* entity;
            BrushNode* brush;
            std::tie(group, entity, brush) = createGroupedBrushEntity();

            assertSelectable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, entity, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(group);
            assertSelectable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, entity, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            context.popGroup();
        }

        /************* Special Case Tests *************/

        TEST_CASE_METHOD(EditorContextTest, "EditorContextTest.testNestedGroupedBrushVisible") {
            Group* outerGroup;
            Group* innerGroup;
            BrushNode* brush;
            std::tie(outerGroup, innerGroup, brush) = createdNestedGroupedBrush();

            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertVisible(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertVisible(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertEditable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertEditable(true, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(outerGroup);
            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);

            assertEditable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);

            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.pushGroup(innerGroup);
            assertVisible(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);

            assertEditable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertEditable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);

            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertPickable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertPickable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            assertSelectable(true, brush, VisibilityState::Visibility_Shown, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Shown, LockState::Lock_Locked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Unlocked);
            assertSelectable(false, brush, VisibilityState::Visibility_Hidden, LockState::Lock_Locked);

            context.popGroup();
            context.popGroup();
        }
    }
}
