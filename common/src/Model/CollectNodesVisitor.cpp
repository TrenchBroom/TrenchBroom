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

#include "CollectNodesVisitor.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        const NodeSet& CollectNodesVisitor::nodeSet() const {
            return m_nodes;
        }
        
        NodeList CollectNodesVisitor::nodeList() const {
            NodeList result;
            result.reserve(m_nodes.size());
            result.insert(result.end(), m_nodes.begin(), m_nodes.end());
            return result;
        }

        void CollectNodesVisitor::doVisit(World* world)   { handleNode(world); }
        void CollectNodesVisitor::doVisit(Layer* layer)   { handleNode(layer); }
        void CollectNodesVisitor::doVisit(Group* group)   { handleNode(group); }
        void CollectNodesVisitor::doVisit(Entity* entity) { handleNode(entity); }
        void CollectNodesVisitor::doVisit(Brush* brush)   { handleNode(brush); }

        void CollectNodesVisitor::handleNode(Node* node) {
            assert(node != NULL);
            m_nodes.insert(node);
        }
        
    }
}
