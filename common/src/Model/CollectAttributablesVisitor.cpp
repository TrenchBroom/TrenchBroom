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

#include "CollectAttributablesVisitor.h"

#include "Model/Attributable.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        const AttributableList& CollectAttributablesVisitor::nodes() const {
            return m_nodes;
        }

        void CollectAttributablesVisitor::doVisit(World* world) {
            addNode(world);
        }
        
        void CollectAttributablesVisitor::doVisit(Layer* layer) {}
        void CollectAttributablesVisitor::doVisit(Group* group) {}
        
        void CollectAttributablesVisitor::doVisit(Entity* entity) {
            addNode(entity);
        }
        
        void CollectAttributablesVisitor::doVisit(Brush* brush) {
            addNode(brush->entity());
        }

        void CollectAttributablesVisitor::addNode(Attributable* node) {
            if (m_addedNodes.insert(node).second)
                m_nodes.push_back(node);
        }
    }
}
