/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/Entity.h"
#include "Model/Issue.h"
#include "Model/IssueGenerator.h"
#include "Model/Layer.h"
#include "Model/MapFormat.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        class TestIssue : public Issue {
        public:
            TestIssue(Node* node) :
            Issue(node) {}
        };
        
        class MockIssueGenerator : public IssueGenerator {
        private:
            void doGenerate(Node* node, IssueList& issues) const {
                mockGenerate(node, issues);
                issues.push_back(new TestIssue(node));
            }
        public:
            MOCK_CONST_METHOD2(mockGenerate, void(const Node*, IssueList&));
        };
        
        TEST(IssueTest, addRemoveNode) {
            using namespace ::testing;
            
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL);
            Entity* entity = world.createEntity();
            Brush* brush = BrushBuilder(&world, worldBounds).createCube(32.0, "sometexture");
            MockIssueGenerator* generator = new MockIssueGenerator();
            
            EXPECT_CALL(*generator, mockGenerate(&world, _));
            EXPECT_CALL(*generator, mockGenerate(world.defaultLayer(), _));
            world.registerIssueGenerators(IssueGeneratorList(1, generator));
            ASSERT_EQ(2u, world.familyIssueCount());
            ASSERT_EQ(1u, world.defaultLayer()->familyIssueCount());

            EXPECT_CALL(*generator, mockGenerate(entity, _));
            EXPECT_CALL(*generator, mockGenerate(world.defaultLayer(), _));
            EXPECT_CALL(*generator, mockGenerate(&world, _));
            world.defaultLayer()->addChild(entity);
            ASSERT_EQ(3u, world.familyIssueCount());
            ASSERT_EQ(2u, world.defaultLayer()->familyIssueCount());
            ASSERT_EQ(1u, entity->familyIssueCount());
            
            EXPECT_CALL(*generator, mockGenerate(brush, _));
            EXPECT_CALL(*generator, mockGenerate(entity, _));
            EXPECT_CALL(*generator, mockGenerate(world.defaultLayer(), _));
            EXPECT_CALL(*generator, mockGenerate(&world, _));
            entity->addChild(brush);
            ASSERT_EQ(4u, world.familyIssueCount());
            ASSERT_EQ(3u, world.defaultLayer()->familyIssueCount());
            ASSERT_EQ(2u, entity->familyIssueCount());
            ASSERT_EQ(1u, brush->familyIssueCount());
            
            EXPECT_CALL(*generator, mockGenerate(world.defaultLayer(), _));
            EXPECT_CALL(*generator, mockGenerate(&world, _));
            world.defaultLayer()->removeChild(entity);
            ASSERT_EQ(2u, world.familyIssueCount());
            ASSERT_EQ(1u, world.defaultLayer()->familyIssueCount());
            
            delete entity;
        }
        
        TEST(IssueTest, registerGenerator) {
            using namespace ::testing;
            
            const BBox3 worldBounds(8192.0);
            World world(MapFormat::Standard, NULL);
            Entity* entity = world.createEntity();
            Brush* brush = BrushBuilder(&world, worldBounds).createCube(32.0, "sometexture");
            MockIssueGenerator* generator = new MockIssueGenerator();
            
            world.defaultLayer()->addChild(entity);
            entity->addChild(brush);
            
            EXPECT_CALL(*generator, mockGenerate(&world, _));
            EXPECT_CALL(*generator, mockGenerate(world.defaultLayer(), _));
            EXPECT_CALL(*generator, mockGenerate(entity, _));
            EXPECT_CALL(*generator, mockGenerate(brush, _));
            world.registerIssueGenerators(IssueGeneratorList(1, generator));
            ASSERT_EQ(4u, world.familyIssueCount());
            ASSERT_EQ(3u, world.defaultLayer()->familyIssueCount());
            ASSERT_EQ(2u, entity->familyIssueCount());
            ASSERT_EQ(1u, brush->familyIssueCount());
        }
    }
}
