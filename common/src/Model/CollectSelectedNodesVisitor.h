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

#ifndef CollectSelectedNodesVisitor_h
#define CollectSelectedNodesVisitor_h

#include "Model/CollectMatchingNodesVisitor.h"
#include "Model/MatchSelectedNodes.h"

namespace TrenchBroom {
    namespace Model {
        class EditorContext;
        
        template <typename C, bool MatchSelected>
        class CollectSelectedNodesTemplate : public CollectMatchingNodesVisitor<MatchSelectedNodes<MatchSelected>, C, StopRecursionIfMatched> {
        public:
            CollectSelectedNodesTemplate() :
            CollectMatchingNodesVisitor<MatchSelectedNodes<MatchSelected>, C, StopRecursionIfMatched>(MatchSelectedNodes<MatchSelected>()) {}
        };
        
        typedef CollectSelectedNodesTemplate<StandardNodeCollectionStrategy, true>  CollectSelectedNodesVisitor;
        typedef CollectSelectedNodesTemplate<StandardNodeCollectionStrategy, false> CollectUnselectedNodesVisitor;
    }
}

#endif /* CollectSelectedNodesVisitor_h */
