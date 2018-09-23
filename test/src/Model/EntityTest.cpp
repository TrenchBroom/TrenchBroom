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
#include <gmock/gmock.h>

#include <memory>

#include "Model/Entity.h"
#include "Model/EntityAttributes.h"
#include "Model/MapFormat.h"
#include "Model/Layer.h"
#include "Model/World.h"

#include <vecmath/vec.h>

namespace TrenchBroom {
    namespace Model {
        class EntityTest : public ::testing::Test {
        protected:
            Entity* m_entity;
            World* m_world;

            void SetUp() override {
                m_entity = new Entity();
                m_world = new World(MapFormat::Standard, nullptr, vm::bbox3d(8192.0));
            }

            void TearDown() override {
                // Only some of the tests add the entity to the world
                if (m_entity->parent() == nullptr) {
                    delete m_entity;
                }
                delete m_world;
            }
        };

        TEST_F(EntityTest, defaults) {
            EXPECT_EQ(vm::vec3::zero, m_entity->origin());
            EXPECT_EQ(vm::mat4x4::identity, m_entity->rotation());
            EXPECT_TRUE(m_entity->pointEntity());
            EXPECT_EQ(Entity::DefaultBounds, m_entity->bounds());
        }

        TEST_F(EntityTest, originUpdateWithSetAttributes) {
            const vm::vec3 newOrigin(10, 20, 30);
            const vm::bbox3 newBounds(newOrigin - (Entity::DefaultBounds.size() / 2.0),
                                      newOrigin + (Entity::DefaultBounds.size() / 2.0));

            m_entity->setAttributes({EntityAttribute("origin", "10 20 30")});
            EXPECT_EQ(newOrigin, m_entity->origin());
            EXPECT_EQ(newBounds, m_entity->bounds());
        }

        TEST_F(EntityTest, originUpdateWithAddOrUpdateAttributes) {
            const vm::vec3 newOrigin(10, 20, 30);
            const vm::bbox3 newBounds(newOrigin - (Entity::DefaultBounds.size() / 2.0),
                                      newOrigin + (Entity::DefaultBounds.size() / 2.0));

            m_entity->addOrUpdateAttribute("origin", "10 20 30");
            EXPECT_EQ(newOrigin, m_entity->origin());
            EXPECT_EQ(newBounds, m_entity->bounds());
        }

        // Same as above, but add the entity to a world
        TEST_F(EntityTest, originUpdateInWorld) {
            m_world->defaultLayer()->addChild(m_entity);

            const vm::vec3 newOrigin(10, 20, 30);
            const vm::bbox3 newBounds(newOrigin - (Entity::DefaultBounds.size() / 2.0),
                                      newOrigin + (Entity::DefaultBounds.size() / 2.0));

            m_entity->addOrUpdateAttribute("origin", "10 20 30");
            EXPECT_EQ(newOrigin, m_entity->origin());
            EXPECT_EQ(newBounds, m_entity->bounds());
        }
    }
}
