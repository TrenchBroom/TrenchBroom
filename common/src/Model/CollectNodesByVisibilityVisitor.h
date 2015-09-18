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

#ifndef TrenchBroom_CollectNodesByVisibilityVisitor
#define TrenchBroom_CollectNodesByVisibilityVisitor

#include "Model/CollectMatchingNodesVisitor.h"
#include "Model/MatchNodesByVisibility.h"
#include "Model/ModelTypes.h"
#include "Model/NodePredicates.h"

namespace TrenchBroom {
    namespace Model {
        template <template <typename> class Op, typename M>
        class CollectNodesByVisibilityTemplate : public CollectMatchingNodesVisitor<Op<M>, StandardNodeCollectionStrategy, NeverStopRecursion> {
        public:
            CollectNodesByVisibilityTemplate(const VisibilityState visibility) :
            CollectMatchingNodesVisitor<Op<M>, StandardNodeCollectionStrategy, NeverStopRecursion>(Op<M>(M(visibility))) {}
        };
        
        typedef CollectNodesByVisibilityTemplate<NodePredicates::Id, MatchNodesByVisibility> CollectNodesWithVisibilityVisitor;
        typedef CollectNodesByVisibilityTemplate<NodePredicates::Not, MatchNodesByVisibility> CollectNodesWithoutVisibilityVisitor;
    }
}

#endif /* defined(TrenchBroom_CollectNodesByVisibilityVisitor) */
