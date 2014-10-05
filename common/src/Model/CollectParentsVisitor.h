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

#ifndef __TrenchBroom__CollectParentsVisitor__
#define __TrenchBroom__CollectParentsVisitor__

#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        class CollectParentsVisitor : public ConstNodeVisitor {
        private:
            NodeSet m_nodes;
        public:
            const NodeSet& parentSet() const;
            NodeList parentList() const;
        private:
            void doVisit(const World* world);
            void doVisit(const Layer* layer);
            void doVisit(const Group* group);
            void doVisit(const Entity* entity);
            void doVisit(const Brush* brush);
            void handleNode(const Node* node);
        };
    }
}

#endif /* defined(__TrenchBroom__CollectParentsVisitor__) */
