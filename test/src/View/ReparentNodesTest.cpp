/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        class ReparentNodesTest : public MapDocumentTest {};
        
        TEST_F(ReparentNodesTest, reparentLayerToLayer) {
            Model::Layer* layer1 = new Model::Layer("Layer 1", document->worldBounds());
            document->addNode(layer1, document->world());
            
            Model::Layer* layer2 = new Model::Layer("Layer 2", document->worldBounds());
            document->addNode(layer2, document->world());
            
            ASSERT_FALSE(document->reparentNodes(layer2, Model::NodeList(1, layer1)));
        }
        
        TEST_F(ReparentNodesTest, reparentBetweenLayers) {
            Model::Layer* oldParent = new Model::Layer("Layer 1", document->worldBounds());
            document->addNode(oldParent, document->world());
            
            Model::Layer* newParent = new Model::Layer("Layer 2", document->worldBounds());
            document->addNode(newParent, document->world());
            
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, oldParent);
            
            assert(entity->parent() == oldParent);
            ASSERT_TRUE(document->reparentNodes(newParent, Model::NodeList(1, entity)));
            ASSERT_EQ(newParent, entity->parent());
            
            document->undoLastCommand();
            ASSERT_EQ(oldParent, entity->parent());
        }
        
        TEST_F(ReparentNodesTest, reparentGroupToItself) {
            Model::Group* group = new Model::Group("Group");
            document->addNode(group, document->currentParent());
            
            ASSERT_FALSE(document->reparentNodes(group, Model::NodeList(1, group)));
        }
        
        TEST_F(ReparentNodesTest, reparentGroupToChild) {
            Model::Group* outer = new Model::Group("Outer");
            document->addNode(outer, document->currentParent());
            
            Model::Group* inner = new Model::Group("Inner");
            document->addNode(inner, outer);
            
            ASSERT_FALSE(document->reparentNodes(inner, Model::NodeList(1, outer)));
        }
        
        TEST_F(ReparentNodesTest, removeEmptyGroup) {
            Model::Group* group = new Model::Group("Group");
            document->addNode(group, document->currentParent());

            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, group);
            
            ASSERT_TRUE(document->reparentNodes(document->currentParent(), Model::NodeList(1, entity)));
            ASSERT_EQ(document->currentParent(), entity->parent());
            ASSERT_TRUE(group->parent() == NULL);
            
            document->undoLastCommand();
            ASSERT_EQ(document->currentParent(), group->parent());
            ASSERT_EQ(group, entity->parent());
        }
        
        TEST_F(ReparentNodesTest, recursivelyRemoveEmptyGroups) {
            Model::Group* outer = new Model::Group("Outer");
            document->addNode(outer, document->currentParent());
            
            Model::Group* inner = new Model::Group("Inner");
            document->addNode(inner, outer);
            
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, inner);
            
            ASSERT_TRUE(document->reparentNodes(document->currentParent(), Model::NodeList(1, entity)));
            ASSERT_EQ(document->currentParent(), entity->parent());
            ASSERT_TRUE(inner->parent() == NULL);
            ASSERT_TRUE(outer->parent() == NULL);
            
            document->undoLastCommand();
            ASSERT_EQ(document->currentParent(), outer->parent());
            ASSERT_EQ(outer, inner->parent());
            ASSERT_EQ(inner, entity->parent());
        }
        
        TEST_F(ReparentNodesTest, removeEmptyEntity) {
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());
            
            Model::Brush* brush = createBrush();
            document->addNode(brush, entity);
            
            ASSERT_TRUE(document->reparentNodes(document->currentParent(), Model::NodeList(1, brush)));
            ASSERT_EQ(document->currentParent(), brush->parent());
            ASSERT_TRUE(entity->parent() == NULL);
            
            document->undoLastCommand();
            ASSERT_EQ(document->currentParent(), entity->parent());
            ASSERT_EQ(entity, brush->parent());
        }
        
        TEST_F(ReparentNodesTest, removeEmptyGroupAndEntity) {
            Model::Group* group = new Model::Group("Group");
            document->addNode(group, document->currentParent());
            
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, group);
            
            Model::Brush* brush = createBrush();
            document->addNode(brush, entity);
            
            ASSERT_TRUE(document->reparentNodes(document->currentParent(), Model::NodeList(1, brush)));
            ASSERT_EQ(document->currentParent(), brush->parent());
            ASSERT_TRUE(group->parent() == NULL);
            ASSERT_TRUE(entity->parent() == NULL);
            
            document->undoLastCommand();
            ASSERT_EQ(document->currentParent(), group->parent());
            ASSERT_EQ(group, entity->parent());
            ASSERT_EQ(entity, brush->parent());
        }
    }
}
