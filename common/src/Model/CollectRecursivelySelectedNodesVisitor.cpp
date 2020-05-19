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

#include "CollectRecursivelySelectedNodesVisitor.h"

#include "Model/GroupNode.h"

namespace TrenchBroom {
    namespace Model {
        MatchRecursivelySelectedNodes::MatchRecursivelySelectedNodes(const bool selected) :
        m_selected(selected) {}

        bool MatchRecursivelySelectedNodes::operator()(const Node* node) const {
            return node->parentSelected() == m_selected;
        }

        CollectRecursivelySelectedNodesVisitor::CollectRecursivelySelectedNodesVisitor(const bool selected) :
        CollectMatchingNodesVisitor(MatchRecursivelySelectedNodes(selected)) {}
    }
}
