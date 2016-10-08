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
        Model::NodeList collectParents(const Model::NodeList& nodes) {
            return collectParents(nodes.begin(), nodes.end());
        }
        
        Model::NodeList collectParents(const Model::ParentChildrenMap& nodes) {
            Model::CollectUniqueNodesVisitor visitor;
            Model::ParentChildrenMap::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* parent = it->first;
                parent->acceptAndEscalate(visitor);
            }
            return visitor.nodes();
        }
        
        Model::NodeList collectChildren(const Model::ParentChildrenMap& nodes) {
            Model::NodeList result;
            Model::ParentChildrenMap::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                const Model::NodeList& children = it->second;
                VectorUtils::append(result, children);
            }
            return result;
        }
        
        Model::ParentChildrenMap parentChildrenMap(const Model::NodeList& nodes) {
            Model::ParentChildrenMap result;
            
            Model::NodeList::const_iterator it, end;
            for (it = nodes.begin(), end = nodes.end(); it != end; ++it) {
                Model::Node* node = *it;
                Model::Node* parent = node->parent();
                ensure(parent != NULL, "parent is null");
                result[parent].push_back(node);
            }
            
            return result;
        }
    }
}
