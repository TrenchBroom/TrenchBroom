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

#include "CollectParentsVisitor.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        const NodeSet& CollectParentsVisitor::parentSet() const {
            return m_nodes;
        }
        
        NodeList CollectParentsVisitor::parentList() const {
            NodeList result;
            result.reserve(m_nodes.size());
            result.insert(result.end(), m_nodes.begin(), m_nodes.end());
            return result;
        }

        void CollectParentsVisitor::doVisit(const World* world)   { handleNode(world); }
        void CollectParentsVisitor::doVisit(const Layer* layer)   { handleNode(layer); }
        void CollectParentsVisitor::doVisit(const Group* group)   { handleNode(group); }
        void CollectParentsVisitor::doVisit(const Entity* entity) { handleNode(entity); }
        void CollectParentsVisitor::doVisit(const Brush* brush)   { handleNode(brush); }

        void CollectParentsVisitor::handleNode(const Node* node) {
            Node* parent = node->parent();
            if (parent != NULL)
                m_nodes.insert(parent);
        }
        
    }
}
