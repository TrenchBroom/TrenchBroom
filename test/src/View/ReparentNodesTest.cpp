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

#include "MathUtils.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/MapFormat.h"
#include "Model/TestGame.h"
#include "Model/World.h"
#include "View/MapDocument.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        class ReparentNodesTest : public ::testing::Test {
        protected:
            MapDocumentSPtr m_document;
            
            void SetUp() {
                m_document = MapDocumentCommandFacade::newMapDocument();
                m_document->newDocument(Model::MapFormat::Standard, BBox3(8192.0), Model::GamePtr(new Model::TestGame()));
            }
        };
        
        TEST_F(ReparentNodesTest, reparentLayerToLayer) {
            Model::Layer* layer1 = new Model::Layer("Layer 1", m_document->worldBounds());
            m_document->addNode(layer1, m_document->world());
            
            Model::Layer* layer2 = new Model::Layer("Layer 2", m_document->worldBounds());
            m_document->addNode(layer2, m_document->world());
            
            ASSERT_FALSE(m_document->reparentNodes(layer2, Model::NodeList(1, layer1)));
        }
        
        TEST_F(ReparentNodesTest, reparentBetweenLayers) {
            Model::Layer* oldParent = new Model::Layer("Layer 1", m_document->worldBounds());
            m_document->addNode(oldParent, m_document->world());
            
            Model::Layer* newParent = new Model::Layer("Layer 2", m_document->worldBounds());
            m_document->addNode(newParent, m_document->world());
            
            Model::Entity* entity = new Model::Entity();
            m_document->addNode(entity, oldParent);
            
            assert(entity->parent() == oldParent);
            ASSERT_TRUE(m_document->reparentNodes(newParent, Model::NodeList(1, entity)));
            ASSERT_EQ(newParent, entity->parent());
            
            m_document->undoLastCommand();
            ASSERT_EQ(oldParent, entity->parent());
        }
        
        TEST_F(ReparentNodesTest, reparentGroupToItself) {
            Model::Group* group = new Model::Group("Group");
            m_document->addNode(group, m_document->currentParent());
            
            ASSERT_FALSE(m_document->reparentNodes(group, Model::NodeList(1, group)));
        }
        
        TEST_F(ReparentNodesTest, reparentGroupToChild) {
            Model::Group* outer = new Model::Group("Outer");
            m_document->addNode(outer, m_document->currentParent());
            
            Model::Group* inner = new Model::Group("Inner");
            m_document->addNode(inner, outer);
            
            ASSERT_FALSE(m_document->reparentNodes(inner, Model::NodeList(1, outer)));
        }
        
        TEST_F(ReparentNodesTest, removeEmptyGroup) {
            Model::Group* group = new Model::Group("Group");
            m_document->addNode(group, m_document->currentParent());

            Model::Entity* entity = new Model::Entity();
            m_document->addNode(entity, group);
            
            ASSERT_TRUE(m_document->reparentNodes(m_document->currentParent(), Model::NodeList(1, entity)));
            ASSERT_EQ(m_document->currentParent(), entity->parent());
            ASSERT_TRUE(group->parent() == NULL);
            
            m_document->undoLastCommand();
            ASSERT_EQ(m_document->currentParent(), group->parent());
            ASSERT_EQ(group, entity->parent());
        }
        
        TEST_F(ReparentNodesTest, recursivelyRemoveEmptyGroups) {
            Model::Group* outer = new Model::Group("Outer");
            m_document->addNode(outer, m_document->currentParent());
            
            Model::Group* inner = new Model::Group("Inner");
            m_document->addNode(inner, outer);
            
            Model::Entity* entity = new Model::Entity();
            m_document->addNode(entity, inner);
            
            ASSERT_TRUE(m_document->reparentNodes(m_document->currentParent(), Model::NodeList(1, entity)));
            ASSERT_EQ(m_document->currentParent(), entity->parent());
            ASSERT_TRUE(inner->parent() == NULL);
            ASSERT_TRUE(outer->parent() == NULL);
            
            m_document->undoLastCommand();
            ASSERT_EQ(m_document->currentParent(), outer->parent());
            ASSERT_EQ(outer, inner->parent());
            ASSERT_EQ(inner, entity->parent());
        }
        
        TEST_F(ReparentNodesTest, removeEmptyEntity) {
            Model::Entity* entity = new Model::Entity();
            m_document->addNode(entity, m_document->currentParent());
            
            Model::BrushBuilder builder(m_document->world(), m_document->worldBounds());
            Model::Brush* brush = builder.createCube(32.0, "texture");
            m_document->addNode(brush, entity);
            
            ASSERT_TRUE(m_document->reparentNodes(m_document->currentParent(), Model::NodeList(1, brush)));
            ASSERT_EQ(m_document->currentParent(), brush->parent());
            ASSERT_TRUE(entity->parent() == NULL);
            
            m_document->undoLastCommand();
            ASSERT_EQ(m_document->currentParent(), entity->parent());
            ASSERT_EQ(entity, brush->parent());
        }
        
        TEST_F(ReparentNodesTest, removeEmptyGroupAndEntity) {
            Model::Group* group = new Model::Group("Group");
            m_document->addNode(group, m_document->currentParent());
            
            Model::Entity* entity = new Model::Entity();
            m_document->addNode(entity, group);
            
            Model::BrushBuilder builder(m_document->world(), m_document->worldBounds());
            Model::Brush* brush = builder.createCube(32.0, "texture");
            m_document->addNode(brush, entity);
            
            ASSERT_TRUE(m_document->reparentNodes(m_document->currentParent(), Model::NodeList(1, brush)));
            ASSERT_EQ(m_document->currentParent(), brush->parent());
            ASSERT_TRUE(group->parent() == NULL);
            ASSERT_TRUE(entity->parent() == NULL);
            
            m_document->undoLastCommand();
            ASSERT_EQ(m_document->currentParent(), group->parent());
            ASSERT_EQ(group, entity->parent());
            ASSERT_EQ(entity, brush->parent());
        }
    }
}
