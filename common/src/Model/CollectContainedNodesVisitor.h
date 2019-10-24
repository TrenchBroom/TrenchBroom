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

#ifndef TrenchBroom_CollectContainedNodesVisitor
#define TrenchBroom_CollectContainedNodesVisitor

#include "Model/CollectMatchingNodesVisitor.h"
#include "Model/MatchSelectableNodes.h"
#include "Model/NodePredicates.h"

namespace TrenchBroom {
    namespace Model {
        template <typename I>
        class MatchContainedNodes {
        private:
            const I m_begin;
            const I m_end;
        public:
            MatchContainedNodes(I begin, I end) :
            m_begin(begin),
            m_end(end) {}

            bool operator()(const Node* node) const {
                I cur = m_begin;
                while (cur != m_end) {
                    if (*cur != node && (*cur)->contains(node))
                        return true;
                    ++cur;
                }
                return false;
            }
        };

        template <typename I>
        class CollectContainedNodesVisitor : public CollectMatchingNodesVisitor<NodePredicates::And<MatchSelectableNodes, MatchContainedNodes<I> >, UniqueNodeCollectionStrategy, StopRecursionIfMatched> {
        public:
            CollectContainedNodesVisitor(I begin, I end, const Model::EditorContext& editorContext) :
            CollectMatchingNodesVisitor<NodePredicates::And<MatchSelectableNodes, MatchContainedNodes<I> >, UniqueNodeCollectionStrategy, StopRecursionIfMatched>(NodePredicates::And<MatchSelectableNodes, MatchContainedNodes<I> >(MatchSelectableNodes(editorContext), MatchContainedNodes<I>(begin, end))) {}
        };
    }
}
#endif /* defined(TrenchBroom_CollectContainedNodesVisitor) */
