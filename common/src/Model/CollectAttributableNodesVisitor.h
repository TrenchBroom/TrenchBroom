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

#ifndef TrenchBroom_CollectAttributableNodesVisitor
#define TrenchBroom_CollectAttributableNodesVisitor

#include "Model/NodeVisitor.h"

#include <set>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class AttributableNode;

        class CollectAttributableNodesVisitor : public NodeVisitor {
        private:
            std::set<Node*> m_addedNodes;
            std::vector<AttributableNode*> m_nodes;
        public:
            const std::vector<AttributableNode*>& nodes() const;
        private:
            void doVisit(World* world) override;
            void doVisit(LayerNode* layer) override;
            void doVisit(GroupNode* group) override;
            void doVisit(Entity* entity) override;
            void doVisit(BrushNode* brush) override;

            void addNode(AttributableNode* node);
        };
    }
}

#endif /* defined(TrenchBroom_CollectAttributableNodesVisitor) */
