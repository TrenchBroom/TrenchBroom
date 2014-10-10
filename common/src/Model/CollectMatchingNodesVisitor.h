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

#include "Model/NodeVisitor.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        class NodeCollectionStrategy {
        protected:
            NodeList m_nodes;
        protected:
            virtual void addNode(Node* node) = 0;
        public:
            virtual ~NodeCollectionStrategy();
            const NodeList& nodes() const;
        };
        
        class StandardNodeCollectionStrategy : public NodeCollectionStrategy {
        public:
            virtual ~StandardNodeCollectionStrategy();
        protected:
            void addNode(Node* node);
        };
        
        class UniqueNodeCollectionStrategy : public NodeCollectionStrategy {
        public:
            virtual ~UniqueNodeCollectionStrategy();
        protected:
            void addNode(Node* node);
        };
        
        struct NeverStopRecursion {
            bool operator()(const Node* node, bool matched) const;
        };

        struct StopRecursionIfMatched {
            bool operator()(const Node* node, bool matched) const;
        };
        
        template <typename P, typename C, typename S = NeverStopRecursion>
        class CollectMatchingNodesVisitor : public C, public NodeVisitor {
        private:
            P m_p;
            S m_s;
        public:
            CollectMatchingNodesVisitor(const P& p = P(), const S& s = S()) : m_p(p), m_s(s) {}
        private:
            void doVisit(World* world) {
                const bool match = m_p(world);
                if (match) C::addNode(world);
                if (m_s(world, match)) stopRecursion();
            }
            void doVisit(Layer* layer) {
                const bool match = m_p(layer);
                if (match) C::addNode(layer);
                if (m_s(layer, match)) stopRecursion();
            }
            void doVisit(Group* group) {
                const bool match = m_p(group);
                if (match) C::addNode(group);
                if (m_s(group, match)) stopRecursion();
            }
            void doVisit(Entity* entity) {
                const bool match = m_p(entity);
                if (match) C::addNode(entity);
                if (m_s(entity, match)) stopRecursion();
            }
            void doVisit(Brush* brush) {
                const bool match = m_p(brush);
                if (match) C::addNode(brush);
                if (m_s(brush, match)) stopRecursion();
            }
        };
    }
}

#endif /* defined(__TrenchBroom__CollectMatchingNodesVisitor__) */
