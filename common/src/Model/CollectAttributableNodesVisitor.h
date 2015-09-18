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

#ifndef TrenchBroom_CollectAttributableNodesVisitor
#define TrenchBroom_CollectAttributableNodesVisitor

#include "Model/NodeVisitor.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class CollectAttributableNodesVisitor : public NodeVisitor {
        private:
            NodeSet m_addedNodes;
            AttributableNodeList m_nodes;
        public:
            const AttributableNodeList& nodes() const;
        private:
            void doVisit(World* world);
            void doVisit(Layer* layer);
            void doVisit(Group* group);
            void doVisit(Entity* entity);
            void doVisit(Brush* brush);
            
            void addNode(AttributableNode* node);
        };
    }
}

#endif /* defined(TrenchBroom_CollectAttributableNodesVisitor) */
