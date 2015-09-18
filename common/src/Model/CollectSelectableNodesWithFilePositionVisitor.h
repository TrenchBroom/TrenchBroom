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

#ifndef TrenchBroom_CollectSelectableNodesWithFilePositionVisitor
#define TrenchBroom_CollectSelectableNodesWithFilePositionVisitor

#include "Model/CollectMatchingNodesVisitor.h"
#include "Model/MatchSelectableNodes.h"
#include "Model/NodePredicates.h"

namespace TrenchBroom {
    namespace Model {
        class EditorContext;
        class Node;
        
        class MatchNodesWithFilePosition {
        private:
            const std::vector<size_t> m_positions;
        public:
            MatchNodesWithFilePosition(const std::vector<size_t>& positions);
            bool operator()(const Model::Node* node) const;
        };

        class CollectSelectableNodesWithFilePositionVisitor :
        public CollectMatchingNodesVisitor<NodePredicates::And<MatchSelectableNodes, MatchNodesWithFilePosition>, UniqueNodeCollectionStrategy> {
        public:
            CollectSelectableNodesWithFilePositionVisitor(const EditorContext& editorContext, const std::vector<size_t>& positions);
        };
    }
}

#endif /* defined(TrenchBroom_CollectSelectableNodesWithFilePositionVisitor) */
