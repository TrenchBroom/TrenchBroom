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

#ifndef TrenchBroom_ModelUtils
#define TrenchBroom_ModelUtils

#include "Model/CollectUniqueNodesVisitor.h"
#include "Model/Node.h"

#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Node;

        std::vector<Node*> collectParents(const std::vector<Node*>& nodes);
        std::vector<Node*> collectParents(const std::map<Node*, std::vector<Node*>>& nodes);

        template <typename I>
        std::vector<Node*> collectParents(const I begin, const I end) {
            CollectUniqueNodesVisitor visitor;
            Node::escalate(begin, end, visitor);
            return visitor.nodes();
        }

        std::vector<Node*> collectChildren(const std::map<Node*, std::vector<Node*>>& nodes);
        std::vector<Node*> collectDescendants(const std::vector<Node*>& nodes);
        std::map<Node*, std::vector<Node*>> parentChildrenMap(const std::vector<Node*>& nodes);
    }
}

#endif /* defined(TrenchBroom_ModelUtils) */
