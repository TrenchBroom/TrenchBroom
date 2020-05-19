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

#ifndef TrenchBroom_CollectMatchingNodesVisitor
#define TrenchBroom_CollectMatchingNodesVisitor

#include "Model/NodeVisitor.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/LayerNode.h"
#include "Model/World.h"

#include <kdl/vector_set.h>

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class NodeCollectionStrategy {
        protected:
            std::vector<Node*> m_nodes;
        public:
            virtual ~NodeCollectionStrategy();

            virtual void addNode(Node* node) = 0;
            const std::vector<Node*>& nodes() const;
        };

        class StandardNodeCollectionStrategy : public NodeCollectionStrategy {
        public:
            virtual ~StandardNodeCollectionStrategy() override;
        public:
            void addNode(Node* node) override;
        };

        class UniqueNodeCollectionStrategy : public NodeCollectionStrategy {
        private:
            kdl::vector_set<Node*> m_addedNodes;
        public:
            virtual ~UniqueNodeCollectionStrategy() override;
        public:
            void addNode(Node* node) override;
        };

        template <class D>
        class FilteringNodeCollectionStrategy {
        private:
            D m_delegate;
        public:
            virtual ~FilteringNodeCollectionStrategy() {}

            const std::vector<Node*>& nodes() const {
                return m_delegate.nodes();
            }

            template <typename T>
            void addNode(T* node) {
                Node* actual = getNode(node);
                if (actual != nullptr)
                    m_delegate.addNode(actual);
            }
        private:
            virtual Node* getNode(World* world) const   { return world;  }
            virtual Node* getNode(LayerNode* layer) const   { return layer;  }
            virtual Node* getNode(Group* group) const   { return group;  }
            virtual Node* getNode(Entity* entity) const { return entity; }
            virtual Node* getNode(BrushNode* brush) const   { return brush;  }
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
            void doVisit(World* world)   override { C::addNode(world);  }
            void doVisit(LayerNode* layer)   override { C::addNode(layer);  }
            void doVisit(Group* group)   override { C::addNode(group);  }
            void doVisit(Entity* entity) override { C::addNode(entity); }
            void doVisit(BrushNode* brush)   override { C::addNode(brush);  }
        };

        template <typename V, typename I>
        std::vector<Node*> collectMatchingNodes(I cur, I end, Node* root) {
            kdl::vector_set<Node*> result;
            while (cur != end) {
                V visitor(*cur);
                root->acceptAndRecurse(visitor);

                const auto& nodes = visitor.nodes();
                result.insert(std::begin(nodes), std::end(nodes));
                ++cur;
            }
            return result.release_data();
        }

    }
}

#endif /* defined(TrenchBroom_CollectMatchingNodesVisitor) */
