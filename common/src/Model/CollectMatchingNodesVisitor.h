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

#ifndef TrenchBroom_CollectMatchingNodesVisitor
#define TrenchBroom_CollectMatchingNodesVisitor

#include "CollectionUtils.h"
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
        public:
            virtual ~NodeCollectionStrategy();
            
            virtual void addNode(Node* node) = 0;
            const NodeList& nodes() const;
        };
        
        class StandardNodeCollectionStrategy : public NodeCollectionStrategy {
        public:
            virtual ~StandardNodeCollectionStrategy();
        public:
            void addNode(Node* node);
        };
        
        class UniqueNodeCollectionStrategy : public NodeCollectionStrategy {
        private:
            NodeSet m_addedNodes;
        public:
            virtual ~UniqueNodeCollectionStrategy();
        public:
            void addNode(Node* node);
        };

        template <class D>
        class FilteringNodeCollectionStrategy {
        private:
            D m_delegate;
        public:
            virtual ~FilteringNodeCollectionStrategy() {}
            
            const NodeList& nodes() const {
                return m_delegate.nodes();
            }
            
            template <typename T>
            void addNode(T* node) {
                Node* actual = getNode(node);
                if (actual != NULL)
                    m_delegate.addNode(actual);
            }
        private:
            virtual Node* getNode(World* world) const   { return world;  }
            virtual Node* getNode(Layer* layer) const   { return layer;  }
            virtual Node* getNode(Group* group) const   { return group;  }
            virtual Node* getNode(Entity* entity) const { return entity; }
            virtual Node* getNode(Brush* brush) const   { return brush;  }
        };
        
        template <
            typename P,
            typename C = StandardNodeCollectionStrategy,
            typename S = NeverStopRecursion
        >
        class CollectMatchingNodesVisitor : public C, public MatchingNodeVisitor<P,S> {
        public:
            CollectMatchingNodesVisitor(const P& p = P(), const S& s = S()) : MatchingNodeVisitor<P,S>(p, s) {}
        private:
            void doVisit(World* world)   { C::addNode(world);  }
            void doVisit(Layer* layer)   { C::addNode(layer);  }
            void doVisit(Group* group)   { C::addNode(group);  }
            void doVisit(Entity* entity) { C::addNode(entity); }
            void doVisit(Brush* brush)   { C::addNode(brush);  }
        };

        template <typename V, typename I>
        Model::NodeList collectMatchingNodes(I cur, I end, Node* root) {
            NodeList result;
            while (cur != end) {
                V visitor(*cur);
                root->acceptAndRecurse(visitor);
                result = VectorUtils::setUnion(result, visitor.nodes());
                ++cur;
            }
            return result;
        }
        
    }
}

#endif /* defined(TrenchBroom_CollectMatchingNodesVisitor) */
