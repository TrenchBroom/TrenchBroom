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

#include "Ensure.h"
#include "Model/BrushNode.h"
#include "Model/CollectNodesVisitor.h"
#include "Model/GroupNode.h"
#include "Model/EntityNode.h"

#include <kdl/overload.h>
#include <kdl/vector_utils.h>

#include <vector>

namespace TrenchBroom {
    namespace Model {
        std::vector<Node*> collectParents(const std::vector<Node*>& nodes) {
            return collectParents(std::begin(nodes), std::end(nodes));
        }

        std::vector<Node*> collectParents(const std::map<Node*, std::vector<Node*>>& nodes) {
            CollectUniqueNodesVisitor visitor;
            for (const auto& entry : nodes) {
                Node* parent = entry.first;
                parent->acceptAndEscalate(visitor);
            }
            return visitor.nodes();
        }

        std::vector<Node*> collectChildren(const std::map<Node*, std::vector<Node*>>& nodes) {
            std::vector<Node*> result;
            for (const auto& entry : nodes) {
                const std::vector<Node*>& children = entry.second;
                kdl::vec_append(result, children);
            }
            return result;
        }

        std::vector<Node*> collectDescendants(const std::vector<Node*>& nodes) {
            CollectNodesVisitor visitor;
            for (auto* node : nodes) {
                node->recurse(visitor);
            }
            return visitor.nodes();
        }

        std::map<Node*, std::vector<Node*>> parentChildrenMap(const std::vector<Node*>& nodes) {
            std::map<Node*, std::vector<Node*>> result;

            for (Node* node : nodes) {
                Node* parent = node->parent();
                ensure(parent != nullptr, "parent is null");
                result[parent].push_back(node);
            }

            return result;
        }

        vm::bbox3 computeLogicalBounds(const std::vector<Node*>& nodes, const vm::bbox3& defaultBounds) {
            vm::bbox3::builder builder;
            Node::visitAll(nodes, kdl::overload(
                [] (const WorldNode*)         {},
                [] (const LayerNode*)         {},
                [&](const GroupNode* group)   { builder.add(group->logicalBounds()); },
                [&](const EntityNode* entity) { builder.add(entity->logicalBounds()); },
                [&](const BrushNode* brush)   { builder.add(brush->logicalBounds()); }
            ));
            return builder.initialized() ? builder.bounds() : defaultBounds;
        }

        vm::bbox3 computePhysicalBounds(const std::vector<Node*>& nodes, const vm::bbox3& defaultBounds) {
            vm::bbox3::builder builder;
            Node::visitAll(nodes, kdl::overload(
                [] (const WorldNode*)         {},
                [] (const LayerNode*)         {},
                [&](const GroupNode* group)   { builder.add(group->physicalBounds()); },
                [&](const EntityNode* entity) { builder.add(entity->physicalBounds()); },
                [&](const BrushNode* brush)   { builder.add(brush->physicalBounds()); }
            ));
            return builder.initialized() ? builder.bounds() : defaultBounds;
        }
    }
}
