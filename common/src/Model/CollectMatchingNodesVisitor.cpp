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

#include "CollectMatchingNodesVisitor.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        NodeCollectionStrategy::~NodeCollectionStrategy() {}

        const std::vector<Node*>& NodeCollectionStrategy::nodes() const {
            return m_nodes;
        }

        StandardNodeCollectionStrategy::~StandardNodeCollectionStrategy() {}

        void StandardNodeCollectionStrategy::addNode(Node* node) {
            m_nodes.push_back(node);
        }

        UniqueNodeCollectionStrategy::~UniqueNodeCollectionStrategy() {}

        void UniqueNodeCollectionStrategy::addNode(Node* node) {
            if (m_addedNodes.insert(node).second) {
                m_nodes.push_back(node);
            }
        }

        bool NeverStopRecursion::operator()(const Node* /* node */, bool /* matched */) const { return false; }

        bool StopRecursionIfMatched::operator()(const Node* /* node */, bool matched) const { return matched; }

    }
}
