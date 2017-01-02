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

#include "CollectAttributableNodesVisitor.h"

#include "Model/AttributableNode.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        const AttributableNodeList& CollectAttributableNodesVisitor::nodes() const {
            return m_nodes;
        }

        void CollectAttributableNodesVisitor::doVisit(World* world) {
            addNode(world);
        }
        
        void CollectAttributableNodesVisitor::doVisit(Layer* layer) {}
        void CollectAttributableNodesVisitor::doVisit(Group* group) {}
        
        void CollectAttributableNodesVisitor::doVisit(Entity* entity) {
            addNode(entity);
        }
        
        void CollectAttributableNodesVisitor::doVisit(Brush* brush) {
            Model::AttributableNode* entity = brush->entity();
            ensure(entity != NULL, "entity is null");
            addNode(entity);
        }

        void CollectAttributableNodesVisitor::addNode(AttributableNode* node) {
            if (m_addedNodes.insert(node).second)
                m_nodes.push_back(node);
        }
    }
}
