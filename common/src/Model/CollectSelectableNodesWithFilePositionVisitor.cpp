/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CollectSelectableNodesWithFilePositionVisitor.h"

namespace TrenchBroom {
    namespace Model {
        MatchNodesWithFilePosition::MatchNodesWithFilePosition(const std::vector<size_t>& positions) :
        m_positions(positions) {}
        
        bool MatchNodesWithFilePosition::operator()(const Model::Node* node) const {
            for (size_t i = 0; i < m_positions.size(); ++i)
                if (node->containsLine(m_positions[i]))
                    return true;
            return false;
        }

        CollectSelectableNodesWithFilePositionVisitor::CollectSelectableNodesWithFilePositionVisitor(const EditorContext& editorContext, const std::vector<size_t>& positions) :
        CollectMatchingNodesVisitor(NodePredicates::And<MatchSelectableNodes, MatchNodesWithFilePosition>(MatchSelectableNodes(editorContext), MatchNodesWithFilePosition(positions))) {}
    }
}
