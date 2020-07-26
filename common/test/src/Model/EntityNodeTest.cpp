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

#include "Model/BrushError.h"
#include "Model/EntityNode.h"
#include "Model/EntityRotationPolicy.h"
#include "Model/EntityAttributes.h"
#include "Model/MapFormat.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"

#include <vecmath/vec.h>
#include <vecmath/mat_ext.h>

#include <memory>
#include <string>

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
                m_entity = new EntityNode();
                m_entity->addOrUpdateAttribute(AttributeNames::Classname, TestClassname);
                m_world = new WorldNode(MapFormat::Standard);
            }

            virtual ~EntityNodeTest() {
                // Only some of the tests add the entity to the world
                if (m_entity->parent() == nullptr) {
                    delete m_entity;
                }
                delete m_world;
            }
        };

        TEST_CASE_METHOD(EntityNodeTest, "EntityTest.defaults") {
            EXPECT_EQ(vm::vec3::zero(), m_entity->origin());
            EXPECT_EQ(vm::mat4x4::identity(), m_entity->rotation());
            EXPECT_TRUE(m_entity->pointEntity());
            EXPECT_EQ(EntityNode::DefaultBounds, m_entity->logicalBounds());
        }

        TEST_CASE_METHOD(EntityNodeTest, "EntityTest.originUpdateWithSetAttributes") {
            const vm::vec3 newOrigin(10, 20, 30);
            const vm::bbox3 newBounds(newOrigin - (EntityNode::DefaultBounds.size() / 2.0),
                                      newOrigin + (EntityNode::DefaultBounds.size() / 2.0));

            m_entity->setAttributes({EntityAttribute("origin", "10 20 30")});
            EXPECT_EQ(newOrigin, m_entity->origin());
            EXPECT_EQ(newBounds, m_entity->logicalBounds());
        }

        TEST_CASE_METHOD(EntityNodeTest, "EntityTest.originUpdateWithAddOrUpdateAttributes") {
            const vm::vec3 newOrigin(10, 20, 30);
            const vm::bbox3 newBounds(newOrigin - (EntityNode::DefaultBounds.size() / 2.0),
                                      newOrigin + (EntityNode::DefaultBounds.size() / 2.0));

            m_entity->addOrUpdateAttribute("origin", "10 20 30");
            EXPECT_EQ(newOrigin, m_entity->origin());
            EXPECT_EQ(newBounds, m_entity->logicalBounds());
        }

        // Same as above, but add the entity to a world
        TEST_CASE_METHOD(EntityNodeTest, "EntityTest.originUpdateInWorld") {
            m_world->defaultLayer()->addChild(m_entity);

            const vm::vec3 newOrigin(10, 20, 30);
            const vm::bbox3 newBounds(newOrigin - (EntityNode::DefaultBounds.size() / 2.0),
                                      newOrigin + (EntityNode::DefaultBounds.size() / 2.0));

            m_entity->addOrUpdateAttribute("origin", "10 20 30");
            EXPECT_EQ(newOrigin, m_entity->origin());
            EXPECT_EQ(newBounds, m_entity->logicalBounds());
        }

        TEST_CASE_METHOD(EntityNodeTest, "EntityTest.requiresClassnameForRotation") {
            m_world->defaultLayer()->addChild(m_entity);
            m_entity->removeAttribute(AttributeNames::Classname);

            EXPECT_EQ(vm::mat4x4::identity(), m_entity->rotation());

            const auto rotMat = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));
            REQUIRE(m_entity->transform(m_worldBounds, rotMat, true));

            // rotation had no effect
            EXPECT_EQ(vm::mat4x4::identity(), m_entity->rotation());
        }

        TEST_CASE_METHOD(EntityNodeTest, "EntityTest.rotateAndTranslate") {
            m_world->defaultLayer()->addChild(m_entity);

            const auto rotMat = vm::rotation_matrix(0.0, 0.0, vm::to_radians(90.0));

            EXPECT_EQ(vm::mat4x4::identity(), m_entity->rotation());
            REQUIRE(m_entity->transform(m_worldBounds, rotMat, true).is_success());
            EXPECT_EQ(rotMat, m_entity->rotation());

            REQUIRE(m_entity->transform(m_worldBounds, vm::translation_matrix(vm::vec3d(100.0, 0.0, 0.0)), true));
            EXPECT_EQ(rotMat, m_entity->rotation());
        }

        TEST_CASE_METHOD(EntityNodeTest, "EntityTest.rotationMatrixToEulerAngles") {
            const auto roll  = vm::to_radians(12.0);
            const auto pitch = vm::to_radians(13.0);
            const auto yaw   = vm::to_radians(14.0);

            const auto rotMat = vm::rotation_matrix(roll, pitch, yaw);

            const auto yawPitchRoll = EntityRotationPolicy::getYawPitchRoll(vm::mat4x4::identity(), rotMat);

            EXPECT_DOUBLE_EQ(12.0, yawPitchRoll.z());
            EXPECT_DOUBLE_EQ(13.0, yawPitchRoll.y());
            EXPECT_DOUBLE_EQ(14.0, yawPitchRoll.x());
        }

        TEST_CASE_METHOD(EntityNodeTest, "EntityTest.rotationMatrixToEulerAngles_uniformScale") {
            const auto roll = vm::to_radians(12.0);
            const auto pitch = vm::to_radians(13.0);
            const auto yaw = vm::to_radians(14.0);

            const auto scaleMat = vm::scaling_matrix(vm::vec3(2.0, 2.0, 2.0));
            const auto rotMat = vm::rotation_matrix(roll, pitch, yaw);

            const auto yawPitchRoll = EntityRotationPolicy::getYawPitchRoll(scaleMat, rotMat);

            // The uniform scale has no effect
            EXPECT_DOUBLE_EQ(12.0, yawPitchRoll.z());
            EXPECT_DOUBLE_EQ(13.0, yawPitchRoll.y());
            EXPECT_DOUBLE_EQ(14.0, yawPitchRoll.x());
        }

        TEST_CASE_METHOD(EntityNodeTest, "EntityTest.rotationMatrixToEulerAngles_nonUniformScale") {
            const auto roll = vm::to_radians(0.0);
            const auto pitch = vm::to_radians(45.0);
            const auto yaw = vm::to_radians(0.0);

            const auto scaleMat = vm::scaling_matrix(vm::vec3(2.0, 1.0, 1.0));
            const auto rotMat = vm::rotation_matrix(roll, pitch, yaw);

            const auto yawPitchRoll = EntityRotationPolicy::getYawPitchRoll(scaleMat, rotMat);

            const auto expectedPitch = vm::to_degrees(std::atan(0.5)); // ~= 26.57 degrees

            EXPECT_DOUBLE_EQ(0.0, yawPitchRoll.z());
            EXPECT_DOUBLE_EQ(expectedPitch, yawPitchRoll.y());
            EXPECT_DOUBLE_EQ(0.0, yawPitchRoll.x());
        }

        TEST_CASE_METHOD(EntityNodeTest, "EntityTest.rotationMatrixToEulerAngles_flip") {
            const auto roll = vm::to_radians(10.0);
            const auto pitch = vm::to_radians(45.0);
            const auto yaw = vm::to_radians(0.0);

            const auto scaleMat = vm::scaling_matrix(vm::vec3(-1.0, 1.0, 1.0));
            const auto rotMat = vm::rotation_matrix(roll, pitch, yaw);

            const auto yawPitchRoll = EntityRotationPolicy::getYawPitchRoll(scaleMat, rotMat);

            EXPECT_DOUBLE_EQ(-10.0, yawPitchRoll.z());
            EXPECT_DOUBLE_EQ(45.0, yawPitchRoll.y());
            EXPECT_DOUBLE_EQ(180.0, yawPitchRoll.x());
        }
    }
}
