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

#include <gtest/gtest.h>

#include "Model/BrushBuilder.h"
#include "Model/EditorContext.h"
#include "Model/World.h"
#include "Model/Layer.h"
#include "Model/Group.h"
#include "Model/Entity.h"
#include "Model/Brush.h"

namespace TrenchBroom {
    namespace Model {
        class EditorContextTest : public ::testing::Test {
        protected:
            BBox3d worldBounds;
            World* world;
            EditorContext context;

            void SetUp() override {
                worldBounds = BBox3d(8192.0);
                world = new World(MapFormat::Standard, nullptr, worldBounds);
            }

            void TearDown() override {
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

            std::tuple<Entity*, Brush*> createTopLevelBrushEntity() {
                BrushBuilder builder(world, worldBounds);
                auto* brush = builder.createCube(32.0, "sometex");
                auto* entity = world->createEntity();
                entity->addChild(brush);
                world->defaultLayer()->addChild(entity);
                return std::make_tuple(entity, brush);
            }

            Brush* createTopLevelBrush() {
                BrushBuilder builder(world, worldBounds);
                auto* brush = builder.createCube(32.0, "sometex");
                world->defaultLayer()->addChild(brush);
                return brush;
            }
            
            std::tuple<Group*, Group*> createNestedGroup() {
                BrushBuilder builder(world, worldBounds);
                auto* innerBrush = builder.createCube(32.0, "sometex");
                auto* innerGroup = world->createGroup("inner");
                auto* outerGroup = world->createGroup("outer");
                
                innerGroup->addChild(innerBrush);
                outerGroup->addChild(innerGroup);
                world->defaultLayer()->addChild(outerGroup);
                
                return std::make_tuple(outerGroup, innerGroup);
            }

            std::tuple<Group*, Brush*> createGroupedBrush() {
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

            std::tuple<Group*, Entity*, Brush*> createGroupedBrushEntity() {
                BrushBuilder builder(world, worldBounds);
                auto* brush = builder.createCube(32.0, "sometex");
                auto* entity = world->createEntity();
                auto* group = world->createGroup("somegroup");

                entity->addChild(brush);
                group->addChild(entity);
                world->defaultLayer()->addChild(group);
                
                return std::make_tuple(group, entity, brush);
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

        TEST_F(EditorContextTest, testWorldVisible) {
            assertVisible(true, world, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, world, Visibility_Shown, Lock_Locked);
            assertVisible(false, world, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, world, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testWorldEditable) {
            assertEditable(true, world, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, world, Visibility_Shown, Lock_Locked);
            assertEditable(true, world, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, world, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testWorldPickable) {
            assertPickable(false, world, Visibility_Shown, Lock_Unlocked);
            assertPickable(false, world, Visibility_Shown, Lock_Locked);
            assertPickable(false, world, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, world, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testWorldSelectable) {
            assertSelectable(false, world, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, world, Visibility_Shown, Lock_Locked);
            assertSelectable(false, world, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, world, Visibility_Hidden, Lock_Locked);
        }

        /************* Default Layer Tests *************/

        TEST_F(EditorContextTest, testDefaultLayerVisible) {
            auto* layer = world->defaultLayer();
            assertVisible(true, layer, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, layer, Visibility_Shown, Lock_Locked);
            assertVisible(false, layer, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, layer, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testDefaultLayerEditable) {
            auto* layer = world->defaultLayer();
            assertEditable(true, layer, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, layer, Visibility_Shown, Lock_Locked);
            assertEditable(true, layer, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, layer, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testDefaultLayerPickable) {
            auto* layer = world->defaultLayer();
            assertPickable(false, layer, Visibility_Shown, Lock_Unlocked);
            assertPickable(false, layer, Visibility_Shown, Lock_Locked);
            assertPickable(false, layer, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, layer, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testDefaultLayerSelectable) {
            auto* layer = world->defaultLayer();
            assertSelectable(false, layer, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, layer, Visibility_Shown, Lock_Locked);
            assertSelectable(false, layer, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, layer, Visibility_Hidden, Lock_Locked);
        }

        /************* Top Level Group Tests *************/

        TEST_F(EditorContextTest, testTopLevelGroupVisible) {
            auto* group = createTopLevelGroup();
            assertVisible(true, group, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, group, Visibility_Shown, Lock_Locked);
            assertVisible(false, group, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, group, Visibility_Hidden, Lock_Locked);

            context.pushGroup(group);
            assertVisible(true, group, Visibility_Shown, Lock_Unlocked);
            context.popGroup();

            group->select();
            assertVisible(true, group, Visibility_Hidden, Lock_Unlocked);
        }

        TEST_F(EditorContextTest, testTopLevelGroupEditable) {
            auto* group = createTopLevelGroup();
            assertEditable(true, group, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, group, Visibility_Shown, Lock_Locked);
            assertEditable(true, group, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, group, Visibility_Hidden, Lock_Locked);

            context.pushGroup(group);
            assertEditable(true, group, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, group, Visibility_Shown, Lock_Locked);
            context.popGroup();
        }

        TEST_F(EditorContextTest, testTopLevelGroupPickable) {
            auto* group = createTopLevelGroup();
            assertPickable(true, group, Visibility_Shown, Lock_Unlocked);
            assertPickable(true, group, Visibility_Shown, Lock_Locked);
            assertPickable(false, group, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, group, Visibility_Hidden, Lock_Locked);

            context.pushGroup(group);
            assertPickable(false, group, Visibility_Shown, Lock_Unlocked);
            assertPickable(false, group, Visibility_Shown, Lock_Locked);
            context.popGroup();
        }

        TEST_F(EditorContextTest, testTopLevelGroupSelectable) {
            auto* group = createTopLevelGroup();
            assertSelectable(true, group, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, group, Visibility_Shown, Lock_Locked);
            assertSelectable(false, group, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, group, Visibility_Hidden, Lock_Locked);

            context.pushGroup(group);
            assertSelectable(false, group, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, group, Visibility_Shown, Lock_Locked);
            context.popGroup();
        }

        /************* Top Level Point Entity Tests *************/

        TEST_F(EditorContextTest, testTopLevelPointEntityVisible) {
            auto* entity = createTopLevelPointEntity();
            assertVisible(true, entity, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, entity, Visibility_Shown, Lock_Locked);
            assertVisible(false, entity, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, entity, Visibility_Hidden, Lock_Locked);
            
            entity->select();
            assertVisible(true, entity, Visibility_Hidden, Lock_Unlocked);
            entity->deselect();

            context.setShowPointEntities(false);
            assertVisible(false, entity, Visibility_Shown, Lock_Unlocked);
            assertVisible(false, entity, Visibility_Shown, Lock_Locked);
        }

        TEST_F(EditorContextTest, testTopLevelPointEntityEditable) {
            auto* entity = createTopLevelPointEntity();
            assertEditable(true, entity, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, entity, Visibility_Shown, Lock_Locked);
            assertEditable(true, entity, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, entity, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testTopLevelPointEntityPickable) {
            auto* entity = createTopLevelPointEntity();
            assertPickable(true, entity, Visibility_Shown, Lock_Unlocked);
            assertPickable(true, entity, Visibility_Shown, Lock_Locked);
            assertPickable(false, entity, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, entity, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testTopLevelPointEntitySelectable) {
            auto* entity = createTopLevelPointEntity();
            assertSelectable(true, entity, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, entity, Visibility_Shown, Lock_Locked);
            assertSelectable(false, entity, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, entity, Visibility_Hidden, Lock_Locked);
        }

        /************* Top Level Brush Entity Tests *************/

        TEST_F(EditorContextTest, testTopLevelBrushEntityVisible) {
            Entity* entity;
            Brush* brush;
            std::tie(entity, brush) = createTopLevelBrushEntity();
            
            assertVisible(true, entity, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, entity, Visibility_Shown, Lock_Locked);
            assertVisible(false, entity, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, entity, Visibility_Hidden, Lock_Locked);
            
            assertVisible(true, brush, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, brush, Visibility_Shown, Lock_Locked);
            assertVisible(false, brush, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, brush, Visibility_Hidden, Lock_Locked);

            brush->setVisibilityState(Visibility_Hidden);
            brush->setLockState(Lock_Unlocked);
            assertVisible(false, entity, Visibility_Shown, Lock_Unlocked);
            assertVisible(false, entity, Visibility_Shown, Lock_Locked);
        }

        TEST_F(EditorContextTest, testTopLevelBrushEntityEditable) {
            Entity* entity;
            Brush* brush;
            std::tie(entity, brush) = createTopLevelBrushEntity();
            
            assertEditable(true, entity, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, entity, Visibility_Shown, Lock_Locked);
            assertEditable(true, entity, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, entity, Visibility_Hidden, Lock_Locked);
            
            assertEditable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, brush, Visibility_Shown, Lock_Locked);
            assertEditable(true, brush, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, brush, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testTopLevelBrushEntityPickable) {
            Entity* entity;
            Brush* brush;
            std::tie(entity, brush) = createTopLevelBrushEntity();
            
            assertPickable(false, entity, Visibility_Shown, Lock_Unlocked);
            assertPickable(false, entity, Visibility_Shown, Lock_Locked);
            assertPickable(false, entity, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, entity, Visibility_Hidden, Lock_Locked);
            
            assertPickable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertPickable(true, brush, Visibility_Shown, Lock_Locked);
            assertPickable(false, brush, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, brush, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testTopLevelBrushEntitySelectable) {
            Entity* entity;
            Brush* brush;
            std::tie(entity, brush) = createTopLevelBrushEntity();
            
            assertSelectable(false, entity, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, entity, Visibility_Shown, Lock_Locked);
            assertSelectable(false, entity, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, entity, Visibility_Hidden, Lock_Locked);
            
            assertSelectable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, brush, Visibility_Shown, Lock_Locked);
            assertSelectable(false, brush, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, brush, Visibility_Hidden, Lock_Locked);
        }

        /************* Top Level Brush Tests *************/

        TEST_F(EditorContextTest, testTopLevelBrushVisible) {
            auto* brush = createTopLevelBrush();
            assertVisible(true, brush, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, brush, Visibility_Shown, Lock_Locked);
            assertVisible(false, brush, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, brush, Visibility_Hidden, Lock_Locked);

            brush->select();
            assertVisible(true, brush, Visibility_Hidden, Lock_Unlocked);
        }

        TEST_F(EditorContextTest, testTopLevelBrushEditable) {
            auto* brush = createTopLevelBrush();
            assertEditable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, brush, Visibility_Shown, Lock_Locked);
            assertEditable(true, brush, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, brush, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testTopLevelBrushPickable) {
            auto* brush = createTopLevelBrush();
            assertPickable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertPickable(true, brush, Visibility_Shown, Lock_Locked);
            assertPickable(false, brush, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, brush, Visibility_Hidden, Lock_Locked);
        }

        TEST_F(EditorContextTest, testTopLevelBrushSelectable) {
            auto* brush = createTopLevelBrush();
            assertSelectable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, brush, Visibility_Shown, Lock_Locked);
            assertSelectable(false, brush, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, brush, Visibility_Hidden, Lock_Locked);
        }

        /************* Nested Group Tests *************/
        
        TEST_F(EditorContextTest, testNestedGroupVisible) {
            Group *outer, *inner;
            std::tie(outer, inner) = createNestedGroup();
            
            assertVisible(true, inner, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, inner, Visibility_Shown, Lock_Locked);
            assertVisible(false, inner, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, inner, Visibility_Hidden, Lock_Locked);
            
            context.pushGroup(outer);
            assertVisible(true, inner, Visibility_Shown, Lock_Unlocked);
            context.pushGroup(inner);
            assertVisible(true, inner, Visibility_Shown, Lock_Unlocked);
            context.popGroup();
            inner->select();
            assertVisible(true, inner, Visibility_Hidden, Lock_Unlocked);
            inner->deselect();
            context.popGroup();
        }
        
        TEST_F(EditorContextTest, testNestedGroupEditable) {
            Group *outer, *inner;
            std::tie(outer, inner) = createNestedGroup();
            
            assertEditable(true, inner, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, inner, Visibility_Shown, Lock_Locked);
            assertEditable(true, inner, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, inner, Visibility_Hidden, Lock_Locked);
            
            context.pushGroup(outer);
            assertEditable(true, inner, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, inner, Visibility_Shown, Lock_Locked);
            context.pushGroup(inner);
            assertEditable(true, inner, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, inner, Visibility_Shown, Lock_Locked);
            context.popGroup();
            context.popGroup();
        }
        
        TEST_F(EditorContextTest, testNestedGroupPickable) {
            Group *outer, *inner;
            std::tie(outer, inner) = createNestedGroup();
            
            assertPickable(false, inner, Visibility_Shown, Lock_Unlocked);
            assertPickable(false, inner, Visibility_Shown, Lock_Locked);
            assertPickable(false, inner, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, inner, Visibility_Hidden, Lock_Locked);
            
            context.pushGroup(outer);
            assertPickable(true, inner, Visibility_Shown, Lock_Unlocked);
            assertPickable(true, inner, Visibility_Shown, Lock_Locked);
            context.pushGroup(inner);
            assertPickable(false, inner, Visibility_Shown, Lock_Unlocked);
            assertPickable(false, inner, Visibility_Shown, Lock_Locked);
            context.popGroup();
            context.popGroup();
        }
        
        TEST_F(EditorContextTest, testNestedGroupSelectable) {
            Group *outer, *inner;
            std::tie(outer, inner) = createNestedGroup();
            
            assertSelectable(false, inner, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, inner, Visibility_Shown, Lock_Locked);
            assertSelectable(false, inner, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, inner, Visibility_Hidden, Lock_Locked);
            
            context.pushGroup(outer);
            assertSelectable(true, inner, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, inner, Visibility_Shown, Lock_Locked);
            context.pushGroup(inner);
            assertSelectable(false, inner, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, inner, Visibility_Shown, Lock_Locked);
            context.popGroup();
            context.popGroup();
        }

        /************* Grouped Brush Tests *************/
        
        TEST_F(EditorContextTest, testGroupedBrushVisible) {
            Group* group;
            Brush* brush;
            std::tie(group, brush) = createGroupedBrush();
            
            assertVisible(true, brush, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, brush, Visibility_Shown, Lock_Locked);
            assertVisible(false, brush, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, brush, Visibility_Hidden, Lock_Locked);
            
            context.pushGroup(group);
            assertVisible(true, brush, Visibility_Shown, Lock_Unlocked);
            brush->select();
            assertVisible(true, brush, Visibility_Hidden, Lock_Unlocked);
            brush->deselect();
            context.popGroup();
        }
        
        TEST_F(EditorContextTest, testGroupedBrushEditable) {
            Group* group;
            Brush* brush;
            std::tie(group, brush) = createGroupedBrush();

            assertEditable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, brush, Visibility_Shown, Lock_Locked);
            assertEditable(true, brush, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, brush, Visibility_Hidden, Lock_Locked);
            
            context.pushGroup(group);
            assertEditable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, brush, Visibility_Shown, Lock_Locked);
            context.popGroup();
        }
        
        TEST_F(EditorContextTest, testGroupedBrushPickable) {
            Group* group;
            Brush* brush;
            std::tie(group, brush) = createGroupedBrush();
            
            assertPickable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertPickable(true, brush, Visibility_Shown, Lock_Locked);
            assertPickable(false, brush, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, brush, Visibility_Hidden, Lock_Locked);
            
            context.pushGroup(group);
            assertPickable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertPickable(true, brush, Visibility_Shown, Lock_Locked);
            context.popGroup();
        }
        
        TEST_F(EditorContextTest, testGroupedBrushSelectable) {
            Group* group;
            Brush* brush;
            std::tie(group, brush) = createGroupedBrush();
            
            assertSelectable(false, brush, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, brush, Visibility_Shown, Lock_Locked);
            assertSelectable(false, brush, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, brush, Visibility_Hidden, Lock_Locked);
            
            context.pushGroup(group);
            assertSelectable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, brush, Visibility_Shown, Lock_Locked);
            context.popGroup();
        }

        /************* Grouped Point Entity Tests *************/
        
        TEST_F(EditorContextTest, testGroupedPointEntityVisible) {
            Group* group;
            Entity* entity;
            std::tie(group, entity) = createGroupedPointEntity();
            
            assertVisible(true, entity, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, entity, Visibility_Shown, Lock_Locked);
            assertVisible(false, entity, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, entity, Visibility_Hidden, Lock_Locked);
            
            context.pushGroup(group);
            assertVisible(true, entity, Visibility_Shown, Lock_Unlocked);
            entity->select();
            assertVisible(true, entity, Visibility_Hidden, Lock_Unlocked);
            entity->deselect();

            context.setShowPointEntities(false);
            assertVisible(false, entity, Visibility_Shown, Lock_Unlocked);
            assertVisible(false, entity, Visibility_Shown, Lock_Locked);

            context.popGroup();

            assertVisible(false, entity, Visibility_Shown, Lock_Unlocked);
            assertVisible(false, entity, Visibility_Shown, Lock_Locked);
        }
        
        TEST_F(EditorContextTest, testGroupedPointEntityEditable) {
            Group* group;
            Entity* entity;
            std::tie(group, entity) = createGroupedPointEntity();
            
            assertEditable(true, entity, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, entity, Visibility_Shown, Lock_Locked);
            assertEditable(true, entity, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, entity, Visibility_Hidden, Lock_Locked);
            
            context.pushGroup(group);
            assertEditable(true, entity, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, entity, Visibility_Shown, Lock_Locked);
            context.popGroup();
        }
        
        TEST_F(EditorContextTest, testGroupedPointEntityPickable) {
            Group* group;
            Entity* entity;
            std::tie(group, entity) = createGroupedPointEntity();
            
            assertPickable(true, entity, Visibility_Shown, Lock_Unlocked);
            assertPickable(true, entity, Visibility_Shown, Lock_Locked);
            assertPickable(false, entity, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, entity, Visibility_Hidden, Lock_Locked);
            
            context.pushGroup(group);
            assertPickable(true, entity, Visibility_Shown, Lock_Unlocked);
            assertPickable(true, entity, Visibility_Shown, Lock_Locked);
            context.popGroup();
        }
        
        TEST_F(EditorContextTest, testGroupedPointEntitySelectable) {
            Group* group;
            Entity* entity;
            std::tie(group, entity) = createGroupedPointEntity();
            
            assertSelectable(false, entity, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, entity, Visibility_Shown, Lock_Locked);
            assertSelectable(false, entity, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, entity, Visibility_Hidden, Lock_Locked);
            
            context.pushGroup(group);
            assertSelectable(true, entity, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, entity, Visibility_Shown, Lock_Locked);
            context.popGroup();
        }

        /************* Grouped Brush Entity Tests *************/
        
        TEST_F(EditorContextTest, testGroupedBrushEntityVisible) {
            Group* group;
            Entity* entity;
            Brush* brush;
            std::tie(group, entity, brush) = createGroupedBrushEntity();
            
            assertVisible(true, entity, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, entity, Visibility_Shown, Lock_Locked);
            assertVisible(false, entity, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, entity, Visibility_Hidden, Lock_Locked);
            
            assertVisible(true, brush, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, brush, Visibility_Shown, Lock_Locked);
            assertVisible(false, brush, Visibility_Hidden, Lock_Unlocked);
            assertVisible(false, brush, Visibility_Hidden, Lock_Locked);

            context.pushGroup(group);
            // test brush first to leave it visible, which influences the entity's visibility
            assertVisible(true, brush, Visibility_Shown, Lock_Unlocked);
            assertVisible(true, entity, Visibility_Shown, Lock_Unlocked);
            context.popGroup();
        }
        
        TEST_F(EditorContextTest, testGroupedBrushEntityEditable) {
            Group* group;
            Entity* entity;
            Brush* brush;
            std::tie(group, entity, brush) = createGroupedBrushEntity();

            assertEditable(true, entity, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, entity, Visibility_Shown, Lock_Locked);
            assertEditable(true, entity, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, entity, Visibility_Hidden, Lock_Locked);

            assertEditable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, brush, Visibility_Shown, Lock_Locked);
            assertEditable(true, brush, Visibility_Hidden, Lock_Unlocked);
            assertEditable(false, brush, Visibility_Hidden, Lock_Locked);

            context.pushGroup(group);
            assertEditable(true, entity, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, entity, Visibility_Shown, Lock_Locked);
            assertEditable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertEditable(false, brush, Visibility_Shown, Lock_Locked);
            context.popGroup();
        }
        
        TEST_F(EditorContextTest, testGroupedBrushEntityPickable) {
            Group* group;
            Entity* entity;
            Brush* brush;
            std::tie(group, entity, brush) = createGroupedBrushEntity();

            assertPickable(false, entity, Visibility_Shown, Lock_Unlocked);
            assertPickable(false, entity, Visibility_Shown, Lock_Locked);
            assertPickable(false, entity, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, entity, Visibility_Hidden, Lock_Locked);

            assertPickable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertPickable(true, brush, Visibility_Shown, Lock_Locked);
            assertPickable(false, brush, Visibility_Hidden, Lock_Unlocked);
            assertPickable(false, brush, Visibility_Hidden, Lock_Locked);

            context.pushGroup(group);
            assertPickable(false, entity, Visibility_Shown, Lock_Unlocked);
            assertPickable(false, entity, Visibility_Shown, Lock_Locked);
            assertPickable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertPickable(true, brush, Visibility_Shown, Lock_Locked);
            context.popGroup();
        }
        
        TEST_F(EditorContextTest, testGroupedBrushEntitySelectable) {
            Group* group;
            Entity* entity;
            Brush* brush;
            std::tie(group, entity, brush) = createGroupedBrushEntity();

            assertSelectable(false, entity, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, entity, Visibility_Shown, Lock_Locked);
            assertSelectable(false, entity, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, entity, Visibility_Hidden, Lock_Locked);

            assertSelectable(false, brush, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, brush, Visibility_Shown, Lock_Locked);
            assertSelectable(false, brush, Visibility_Hidden, Lock_Unlocked);
            assertSelectable(false, brush, Visibility_Hidden, Lock_Locked);

            context.pushGroup(group);
            assertSelectable(false, entity, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, entity, Visibility_Shown, Lock_Locked);
            assertSelectable(true, brush, Visibility_Shown, Lock_Unlocked);
            assertSelectable(false, brush, Visibility_Shown, Lock_Locked);
            context.popGroup();
        }
    }
}
