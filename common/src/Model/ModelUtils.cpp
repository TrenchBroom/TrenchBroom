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

#include "ModelUtils.h"

namespace TrenchBroom {
    namespace Model {
        Model::NodeArray collectParents(const Model::NodeArray& nodes) {
            return collectParents(std::begin(nodes), std::end(nodes));
        }
        
        Model::NodeArray collectParents(const Model::ParentChildrenMap& nodes) {
            Model::CollectUniqueNodesVisitor visitor;
            for (const auto& entry : nodes) {
                Model::Node* parent = entry.first;
                parent->acceptAndEscalate(visitor);
            }
            return visitor.nodes();
        }
        
        Model::NodeArray collectChildren(const Model::ParentChildrenMap& nodes) {
            Model::NodeArray result;
            for (const auto& entry : nodes) {
                const Model::NodeArray& children = entry.second;
                VectorUtils::append(result, children);
            }
            return result;
        }
        
        Model::ParentChildrenMap parentChildrenMap(const Model::NodeArray& nodes) {
            Model::ParentChildrenMap result;
            
            for (Model::Node* node : nodes) {
                Model::Node* parent = node->parent();
                ensure(parent != NULL, "parent is null");
                result[parent].push_back(node);
            }
            
            return result;
        }
    }
}
