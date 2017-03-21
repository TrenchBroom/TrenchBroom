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

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        class GroupNodesTest : public MapDocumentTest {};

        TEST_F(GroupNodesTest, createEmptyGroup) {
            ASSERT_EQ(NULL, document->groupSelection("test"));
        }
        
        TEST_F(GroupNodesTest, createGroupWithOneNode) {
            Model::Brush* brush = createBrush();
            document->addNode(brush, document->currentParent());
            document->select(brush);
            
            Model::Group* group = document->groupSelection("test");
            ASSERT_TRUE(group != NULL);
            
            ASSERT_EQ(group, brush->parent());
            ASSERT_TRUE(group->selected());
            ASSERT_FALSE(brush->selected());
            
            document->undoLastCommand();
            ASSERT_EQ(NULL, group->parent());
            ASSERT_EQ(document->currentParent(), brush->parent());
            ASSERT_TRUE(brush->selected());
        }
        
        TEST_F(GroupNodesTest, createGroupWithPartialBrushEntity) {
            Model::Brush* brush1 = createBrush();
            document->addNode(brush1, document->currentParent());
            
            Model::Brush* brush2 = createBrush();
            document->addNode(brush2, document->currentParent());
            
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());
            document->reparentNodes(entity, VectorUtils::create<Model::Node*>(brush1, brush2));

            document->select(brush1);
            
            Model::Group* group = document->groupSelection("test");
            ASSERT_TRUE(group != NULL);
            
            ASSERT_EQ(entity, brush1->parent());
            ASSERT_EQ(entity, brush2->parent());
            ASSERT_EQ(group, entity->parent());
            ASSERT_TRUE(group->selected());
            ASSERT_FALSE(brush1->selected());
            
            document->undoLastCommand();
            ASSERT_EQ(NULL, group->parent());
            ASSERT_EQ(entity, brush1->parent());
            ASSERT_EQ(entity, brush2->parent());
            ASSERT_EQ(document->currentParent(), entity->parent());
            ASSERT_FALSE(group->selected());
            ASSERT_TRUE(brush1->selected());
        }
        
        TEST_F(GroupNodesTest, createGroupWithFullBrushEntity) {
            Model::Brush* brush1 = createBrush();
            document->addNode(brush1, document->currentParent());
            
            Model::Brush* brush2 = createBrush();
            document->addNode(brush2, document->currentParent());
            
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());
            document->reparentNodes(entity, VectorUtils::create<Model::Node*>(brush1, brush2));
            
            document->select(VectorUtils::create<Model::Node*>(brush1, brush2));
            
            Model::Group* group = document->groupSelection("test");
            ASSERT_TRUE(group != NULL);
            
            ASSERT_EQ(entity, brush1->parent());
            ASSERT_EQ(entity, brush2->parent());
            ASSERT_EQ(group, entity->parent());
            ASSERT_TRUE(group->selected());
            ASSERT_FALSE(brush1->selected());
            ASSERT_FALSE(brush2->selected());
            
            document->undoLastCommand();
            ASSERT_EQ(NULL, group->parent());
            ASSERT_EQ(entity, brush1->parent());
            ASSERT_EQ(entity, brush2->parent());
            ASSERT_EQ(document->currentParent(), entity->parent());
            ASSERT_FALSE(group->selected());
            ASSERT_TRUE(brush1->selected());
            ASSERT_TRUE(brush2->selected());
        }
        
        static bool hasEmptyName(const Model::AttributeNameSet& names) {
            for (const Model::AttributeName& name : names) {
                if (name.empty())
                    return true;
            }
            return false;
        }
        
        TEST_F(GroupNodesTest, undoMoveGroupContainingBrushEntity) {
            // Test for issue #1715
            
            Model::Brush* brush1 = createBrush();
            document->addNode(brush1, document->currentParent());
            
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());
            document->reparentNodes(entity, VectorUtils::create<Model::Node*>(brush1));
            
            document->select(VectorUtils::create<Model::Node*>(brush1));
            
            Model::Group* group = document->groupSelection("test");
            ASSERT_TRUE(group->selected());
            
            ASSERT_TRUE(document->translateObjects(Vec3(16,0,0)));
            
            ASSERT_FALSE(hasEmptyName(entity->attributeNames()));
            
            document->undoLastCommand();
            
            ASSERT_FALSE(hasEmptyName(entity->attributeNames()));
        }
    }
}
