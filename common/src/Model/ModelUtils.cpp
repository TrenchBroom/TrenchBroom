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

#include "ModelUtils.h"
#include "Model/CollectNodesVisitor.h"

namespace TrenchBroom {
    namespace Model {
        NodeList collectParents(const NodeList& nodes) {
            return collectParents(std::begin(nodes), std::end(nodes));
        }
        
        NodeList collectParents(const ParentChildrenMap& nodes) {
            CollectUniqueNodesVisitor visitor;
            for (const auto& entry : nodes) {
                Node* parent = entry.first;
                parent->acceptAndEscalate(visitor);
            }
            return visitor.nodes();
        }
        
        NodeList collectChildren(const ParentChildrenMap& nodes) {
            NodeList result;
            for (const auto& entry : nodes) {
                const NodeList& children = entry.second;
                VectorUtils::append(result, children);
            }
            return result;
        }
        
        NodeList collectDescendants(const Model::NodeList& nodes) {
            CollectNodesVisitor visitor;
            for (const auto* node : nodes) {
                node->recurse(visitor);
            }
            return visitor.nodes();
        }

        ParentChildrenMap parentChildrenMap(const NodeList& nodes) {
            ParentChildrenMap result;
            
            for (Node* node : nodes) {
                Node* parent = node->parent();
                ensure(parent != nullptr, "parent is null");
                result[parent].push_back(node);
            }
            
            return result;
        }
    }
}
