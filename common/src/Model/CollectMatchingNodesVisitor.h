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
        struct NeverStopRecursion {
            bool operator()(const Node* node, bool matched) const { return false; }
        };

        struct StopRecursionIfMatched {
            bool operator()(const Node* node, bool matched) const { return matched; }
        };
        
        template <typename P, typename S = NeverStopRecursion>
        class CollectMatchingNodesVisitor : public NodeVisitor {
        private:
            P m_p;
            S m_s;
            NodeList m_nodes;
        public:
            CollectMatchingNodesVisitor(const P& p = P(), const S& s = S()) : m_p(p), m_s(s) {}
            const NodeList& nodes() const { return m_nodes; }
        private:
            void doVisit(World* world) {
                const bool match = m_p(world);
                if (match) m_nodes.push_back(world);
                if (m_s(world, match)) stopRecursion();
            }
            void doVisit(Layer* layer) {
                const bool match = m_p(layer);
                if (match) m_nodes.push_back(layer);
                if (m_s(layer, match)) stopRecursion();
            }
            void doVisit(Group* group) {
                const bool match = m_p(group);
                if (match) m_nodes.push_back(group);
                if (m_s(group, match)) stopRecursion();
            }
            void doVisit(Entity* entity) {
                const bool match = m_p(entity);
                if (match) m_nodes.push_back(entity);
                if (m_s(entity, match)) stopRecursion();
            }
            void doVisit(Brush* brush) {
                const bool match = m_p(brush);
                if (match) m_nodes.push_back(brush);
                if (m_s(brush, match)) stopRecursion();
            }
        };

        template <typename P, typename S = NeverStopRecursion>
        class CollectMatchingUniqueNodesVisitor : public NodeVisitor {
        private:
            P m_p;
            S m_s;
            NodeList m_nodes;
        public:
            CollectMatchingUniqueNodesVisitor(const P& p = P(), const S& s = S()) : m_p(p), m_s(s) {}
            const NodeList& nodes() const { return m_nodes; }
        private:
            void doVisit(World* world) {
                const bool match = m_p(world);
                if (match) VectorUtils::setInsert(m_nodes, world);
                if (m_s(world, match)) stopRecursion();
            }
            void doVisit(Layer* layer) {
                const bool match = m_p(layer);
                if (match) VectorUtils::setInsert(m_nodes, layer);
                if (m_s(layer, match)) stopRecursion();
            }
            void doVisit(Group* group) {
                const bool match = m_p(group);
                if (match) VectorUtils::setInsert(m_nodes, group);
                if (m_s(group, match)) stopRecursion();
            }
            void doVisit(Entity* entity) {
                const bool match = m_p(entity);
                if (match) VectorUtils::setInsert(m_nodes, entity);
                if (m_s(entity, match)) stopRecursion();
            }
            void doVisit(Brush* brush) {
                const bool match = m_p(brush);
                if (match) VectorUtils::setInsert(m_nodes, brush);
                if (m_s(brush, match)) stopRecursion();
            }
        };
    }
}

#endif /* defined(__TrenchBroom__CollectMatchingNodesVisitor__) */
