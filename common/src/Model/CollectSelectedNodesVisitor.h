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

#ifndef CollectSelectedNodesVisitor_h
#define CollectSelectedNodesVisitor_h

#include "Model/CollectMatchingNodesVisitor.h"
#include "Model/MatchSelectedNodes.h"

namespace TrenchBroom {
    namespace Model {
        template <typename C, typename M>
        class CollectSelectedNodesTemplate : public CollectMatchingNodesVisitor<M, C, NeverStopRecursion> {
        public:
            CollectSelectedNodesTemplate() :
            CollectMatchingNodesVisitor<M, C, NeverStopRecursion>(M()) {}
        };

        using CollectSelectedNodesVisitor = CollectSelectedNodesTemplate<StandardNodeCollectionStrategy, MatchSelectedNodes<true> > ;
        using CollectUnselectedNodesVisitor = CollectSelectedNodesTemplate<StandardNodeCollectionStrategy, MatchSelectedNodes<false> >;
        using CollectTransitivelySelectedNodesVisitor = CollectSelectedNodesTemplate<StandardNodeCollectionStrategy, MatchTransitivelySelectedNodes<true> > ;
        using CollectTransitivelyUnselectedNodesVisitor = CollectSelectedNodesTemplate<StandardNodeCollectionStrategy, MatchTransitivelySelectedNodes<false> >;

        using CollectTransitivelySelectedOrDescendantSelectedNodesVisitor = CollectSelectedNodesTemplate<StandardNodeCollectionStrategy, MatchTransitivelySelectedOrDescendantSelectedNodes<true>>;
        using CollectNotTransitivelySelectedOrDescendantSelectedNodesVisitor = CollectSelectedNodesTemplate<StandardNodeCollectionStrategy, MatchTransitivelySelectedOrDescendantSelectedNodes<false>>;
    }
}

#endif /* CollectSelectedNodesVisitor_h */
