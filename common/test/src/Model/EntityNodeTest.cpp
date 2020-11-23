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

#include "Model/BrushError.h"
#include "Model/EntityNode.h"
#include "Model/EntityRotationPolicy.h"
#include "Model/EntityAttributes.h"
#include "Model/MapFormat.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"

#include <kdl/result.h>

#include <vecmath/vec.h>
#include <vecmath/mat_ext.h>

#include <memory>
#include <string>

#include "Catch2.h"
#include "GTestCompat.h"

namespace TrenchBroom {
    namespace Model {
        static const std::string TestClassname = "something";

        class EntityNodeTest {
        protected:
            vm::bbox3d m_worldBounds;
            EntityNode* m_entity;
            WorldNode* m_world;

            EntityNodeTest() {
                m_worldBounds = vm::bbox3d(8192.0);
                m_entity = new EntityNode({
                    {AttributeNames::Classname, TestClassname}
                });
                m_world = new WorldNode(Model::Entity(), MapFormat::Standard);
            }

            virtual ~EntityNodeTest() {
                // Only some of the tests add the entity to the world
                if (m_entity->parent() == nullptr) {
                    delete m_entity;
                }
                delete m_world;
            }
        };

        TEST_CASE_METHOD(EntityNodeTest, "EntityNodeTest.originUpdateWithSetAttributes") {
            const vm::vec3 newOrigin(10, 20, 30);
            const vm::bbox3 newBounds(newOrigin - (EntityNode::DefaultBounds.size() / 2.0),
                                      newOrigin + (EntityNode::DefaultBounds.size() / 2.0));

            m_entity->setEntity(Entity({EntityAttribute("origin", "10 20 30")}));
            EXPECT_EQ(newOrigin, m_entity->entity().origin());
            EXPECT_EQ(newBounds, m_entity->logicalBounds());
        }

        TEST_CASE_METHOD(EntityNodeTest, "EntityNodeTest.originUpdateWithAddOrUpdateAttributes") {
            const vm::vec3 newOrigin(10, 20, 30);
            const vm::bbox3 newBounds(newOrigin - (EntityNode::DefaultBounds.size() / 2.0),
                                      newOrigin + (EntityNode::DefaultBounds.size() / 2.0));

            m_entity->setEntity(Entity({{"origin", "10 20 30"}}));
            EXPECT_EQ(newOrigin, m_entity->entity().origin());
            EXPECT_EQ(newBounds, m_entity->logicalBounds());
        }

        // Same as above, but add the entity to a world
        TEST_CASE_METHOD(EntityNodeTest, "EntityNodeTest.originUpdateInWorld") {
            m_world->defaultLayer()->addChild(m_entity);

            const vm::vec3 newOrigin(10, 20, 30);
            const vm::bbox3 newBounds(newOrigin - (EntityNode::DefaultBounds.size() / 2.0),
                                      newOrigin + (EntityNode::DefaultBounds.size() / 2.0));

            m_entity->setEntity(Entity({{"origin", "10 20 30"}}));
            EXPECT_EQ(newOrigin, m_entity->entity().origin());
            EXPECT_EQ(newBounds, m_entity->logicalBounds());
        }
    }
}
