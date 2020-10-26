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
#include "Polyhedron.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/CollectNodesVisitor.h"
#include "Model/GroupNode.h"
#include "Model/EditorContext.h"
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

        /**
         * Recursively collect brushes and entities from the given vector of node trees such that
         * the returned nodes match the given predicate. A matching brush is only returned if it
         * isn't in the given vector brushes. A node matches the given predicate if there is a brush
         * in the given vector of brushes such that the predicate evaluates to true for that pair of
         * node and brush.
         *
         * The given predicate must be a function that maps a node and a brush to true or false.
         */
        template <typename P>
        static std::vector<Node*> collectMatchingNodes(const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushes, const P& predicate) {
            auto result = std::vector<Model::Node*>{};

            const auto collectIfMatching = [&](auto* node) {
                for (const auto* brush : brushes) {
                    if (predicate(node, brush)) {
                        result.push_back(node);
                        return;
                    }
                }
            };

            for (auto* node : nodes) {
                node->acceptLambda(kdl::overload(
                    [] (auto&& thisLambda, Model::WorldNode* world) { world->visitChildren(thisLambda); },
                    [] (auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, Model::GroupNode* group) { 
                        if (group->opened()) {
                            group->visitChildren(thisLambda);
                        } else {
                            collectIfMatching(group);
                        }
                    },
                    [&](auto&& thisLambda, Model::EntityNode* entity) { 
                        if (entity->hasChildren()) {
                            entity->visitChildren(thisLambda);
                        } else {
                            collectIfMatching(entity);
                        }
                    },
                    [&](Model::BrushNode* brush)  { 
                        // if `brush` is one of the search query nodes, don't count it as touching
                        if (!kdl::vec_contains(brushes, brush)) {
                            collectIfMatching(brush);
                        }
                    }
                ));
            }

            return result;
        }

        std::vector<Node*> collectTouchingNodes(const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushes) {
            return collectMatchingNodes(nodes, brushes, [](const auto* node, const auto* brush) {
                return brush->intersects(node);
            });
        }

        std::vector<Node*> collectContainedNodes(const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushes) {
            return collectMatchingNodes(nodes, brushes, [](const auto* node, const auto* brush) {
                return brush->contains(node);
            });
        }

        std::vector<Node*> collectSelectableNodes(const std::vector<Node*>& nodes, const EditorContext& editorContext) {
            auto result = std::vector<Node*>{};

            for (auto* node : nodes) {
                node->acceptLambda(kdl::overload(
                    [&](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, GroupNode* group) {
                        if (editorContext.selectable(group)) {
                            // implies that any containing group is opened and that group itself is closed
                            // therefore we don't need to visit the group's children
                            result.push_back(group);
                        } else {
                            group->visitChildren(thisLambda);
                        }
                    },
                    [&](auto&& thisLambda, EntityNode* entity) {
                        if (editorContext.selectable(entity)) {
                            result.push_back(entity);
                        }
                        entity->visitChildren(thisLambda);
                    },
                    [&](BrushNode* brush) {
                        if (editorContext.selectable(brush)) {
                            result.push_back(brush);
                        }
                    }
                ));
            }
            
            return result;
        }

        std::vector<BrushFaceHandle> collectBrushFaces(const std::vector<Node*>& nodes) {
            auto faces = std::vector<BrushFaceHandle>{};
            for (auto* node : nodes) {
                node->acceptLambda(kdl::overload(
                    [] (auto&& thisLambda, WorldNode* world)   { world->visitChildren(thisLambda); },
                    [] (auto&& thisLambda, LayerNode* layer)   { layer->visitChildren(thisLambda); },
                    [] (auto&& thisLambda, GroupNode* group)   { group->visitChildren(thisLambda); },
                    [] (auto&& thisLambda, EntityNode* entity) { entity->visitChildren(thisLambda); },
                    [&](BrushNode* brushNode) {
                        const auto& brush = brushNode->brush();
                        for (size_t i = 0; i < brush.faceCount(); ++i) {
                            faces.emplace_back(brushNode, i);
                        }
                    }
                ));
            }
            return faces;
        }

        std::vector<BrushFaceHandle> collectSelectableBrushFaces(const std::vector<Node*>& nodes, const EditorContext& editorContext) {
            auto faces = std::vector<BrushFaceHandle>{};
            for (auto* node : nodes) {
                node->acceptLambda(kdl::overload(
                    [] (auto&& thisLambda, WorldNode* world)   { world->visitChildren(thisLambda); },
                    [] (auto&& thisLambda, LayerNode* layer)   { layer->visitChildren(thisLambda); },
                    [] (auto&& thisLambda, GroupNode* group)   { group->visitChildren(thisLambda); },
                    [] (auto&& thisLambda, EntityNode* entity) { entity->visitChildren(thisLambda); },
                    [&](BrushNode* brushNode) {
                        const auto& brush = brushNode->brush();
                        for (size_t i = 0; i < brush.faceCount(); ++i) {
                            const auto& face = brush.face(i);
                            if (editorContext.selectable(brushNode, face)) {
                                faces.emplace_back(brushNode, i);
                            }
                        }
                    }
                ));
            }
            return faces;
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

        bool boundsContainNode(const vm::bbox3& bounds, const Node* node) {
            return node->acceptLambda(kdl::overload(
                [] (const WorldNode*)         { return false; },
                [] (const LayerNode*)         { return false; },
                [&](const GroupNode* group)   { return bounds.contains(group->logicalBounds()); },
                [&](const EntityNode* entity) { return bounds.contains(entity->logicalBounds()); },
                [&](const BrushNode* brush)   { return bounds.contains(brush->logicalBounds()); }
            ));
        }

        bool boundsIntersectNode(const vm::bbox3& bounds, const Node* node) {
            return node->acceptLambda(kdl::overload(
                [] (const WorldNode*)         { return false; },
                [] (const LayerNode*)         { return false; },
                [&](const GroupNode* group)   { return bounds.contains(group->logicalBounds()); },
                [&](const EntityNode* entity) { return bounds.contains(entity->logicalBounds()); },
                [&](const BrushNode* brush)   { 
                    for (const auto* vertex : brush->brush().vertices()) {
                        if (bounds.contains(vertex->position())) {
                            return true;
                        }
                    }
                    return false;
                 }
            ));
        }
    }
}
