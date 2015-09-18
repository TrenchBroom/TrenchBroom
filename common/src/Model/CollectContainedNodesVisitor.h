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

#ifndef TrenchBroom_CollectContainedNodesVisitor
#define TrenchBroom_CollectContainedNodesVisitor

#include "Model/CollectMatchingNodesVisitor.h"
#include "Model/ModelTypes.h"
#include "Model/NodePredicates.h"

namespace TrenchBroom {
    namespace Model {
        class MatchContainedNodes {
        private:
            const Object* m_object;
        public:
            MatchContainedNodes(const Object* object);
            bool operator()(const Node* node) const;
        };
        
        class CollectContainedNodesVisitor : public CollectMatchingNodesVisitor<NodePredicates::And<NodePredicates::Not<NodePredicates::EqualsObject>, MatchContainedNodes>, UniqueNodeCollectionStrategy> {
        public:
            CollectContainedNodesVisitor(const Object* object);
        };
    }
}
#endif /* defined(TrenchBroom_CollectContainedNodesVisitor) */
