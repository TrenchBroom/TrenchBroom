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

#ifndef __TrenchBroom__CollectMatchingNodesVisitor__
#define __TrenchBroom__CollectMatchingNodesVisitor__

#include "CollectionUtils.h"
#include "Model/NodeVisitor.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        template <typename P>
        class CollectMatchingNodesVisitor : public NodeVisitor {
        private:
            P m_p;
            NodeList m_nodes;
        public:
            CollectMatchingNodesVisitor(const P& p = P()) : m_p(p) {}
            const NodeList& nodes() const { return m_nodes; }
        private:
            void doVisit(World* world)   { if (m_p(world))  m_nodes.push_back(world);  }
            void doVisit(Layer* layer)   { if (m_p(layer))  m_nodes.push_back(layer);  }
            void doVisit(Group* group)   { if (m_p(group))  m_nodes.push_back(group);  }
            void doVisit(Entity* entity) { if (m_p(entity)) m_nodes.push_back(entity); }
            void doVisit(Brush* brush)   { if (m_p(brush))  m_nodes.push_back(brush);  }
        };

        template <typename P>
        class CollectMatchingUniqueNodesVisitor : public NodeVisitor {
        private:
            P m_p;
            NodeList m_nodes;
        public:
            CollectMatchingUniqueNodesVisitor(const P& p = P()) : m_p(p) {}
            const NodeList& nodes() const { return m_nodes; }
        private:
            void doVisit(World* world)   { if (m_p(world))  VectorUtils::setInsert(m_nodes, world);  }
            void doVisit(Layer* layer)   { if (m_p(layer))  VectorUtils::setInsert(m_nodes, layer);  }
            void doVisit(Group* group)   { if (m_p(group))  VectorUtils::setInsert(m_nodes, group);  }
            void doVisit(Entity* entity) { if (m_p(entity)) VectorUtils::setInsert(m_nodes, entity); }
            void doVisit(Brush* brush)   { if (m_p(brush))  VectorUtils::setInsert(m_nodes, brush);  }
        };
    }
}

#endif /* defined(__TrenchBroom__CollectMatchingNodesVisitor__) */
