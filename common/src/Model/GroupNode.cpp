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

#include "GroupNode.h"

#include "Ensure.h"
#include "FloatType.h"
#include "Model/Brush.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/IssueGenerator.h"
#include "Model/LayerNode.h"
#include "Model/ModelUtils.h"
#include "Model/NodeContents.h"
#include "Model/PatchNode.h"
#include "Model/PickResult.h"
#include "Model/TagVisitor.h"
#include "Model/UpdateLinkedGroupsError.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/parallel.h>
#include <kdl/result.h>
#include <kdl/result_for_each.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/ray.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        /**
         * Recursively collect the nodes to clone + transform, starting with the children of `node`.
         * (`node` itself is skipped.)
         */
        static std::vector<const Node*> collectNodesToCloneAndTransform(const Node& node) {
            auto result = std::vector<const Node*>{};

            std::function<void(const Node*)> collectNodes = [&](const Node* n) {
                result.push_back(n);
                for (auto* child : n->children()) {
                    collectNodes(child);
                }
            };

            for (auto* child : node.children()) {
                collectNodes(child);
            }

            return result;
        }

        /**
         * Given a node, clones its children recursively and applies the given transform.
         * 
         * Returns a vector of the cloned direct children of `node`.
         */
        static kdl::result<std::vector<std::unique_ptr<Node>>, UpdateLinkedGroupsError> cloneAndTransformChildren(const Node& node, const vm::bbox3& worldBounds, const vm::mat4x4& transformation) {
            auto nodesToClone = collectNodesToCloneAndTransform(node);

            using TransformResult = kdl::result<std::pair<const Node*, NodeContents>, BrushError>;

            // In parallel, produce pairs { node pointer, transformed contents } from the nodes in `nodesToClone`
            const auto transformResults = kdl::vec_parallel_transform(nodesToClone, [&](const Node* nodeToTransform) {
                return nodeToTransform->accept(kdl::overload(
                    [] (const WorldNode*) -> TransformResult { ensure(false, "Linked group structure is valid"); },
                    [] (const LayerNode*) -> TransformResult { ensure(false, "Linked group structure is valid"); },
                    [&](const GroupNode* groupNode) -> TransformResult {
                        auto group = groupNode->group();
                        group.transform(transformation);
                        return std::make_pair(nodeToTransform, NodeContents{std::move(group)});
                    },
                    [&](const EntityNode* entityNode) -> TransformResult {
                        auto entity = entityNode->entity();
                        entity.transform(transformation);
                        return std::make_pair(nodeToTransform, NodeContents{std::move(entity)});
                    },
                    [&](const BrushNode* brushNode) -> TransformResult {
                        auto brush = brushNode->brush();
                        return brush.transform(worldBounds, transformation, true)
                            .and_then([&]() -> TransformResult {
                                return std::make_pair(nodeToTransform, NodeContents{std::move(brush)});
                            });
                    },
                    [&](const PatchNode* patchNode) -> TransformResult {
                        auto patch = patchNode->patch();
                        patch.transform(transformation);
                        return std::make_pair(nodeToTransform, NodeContents{std::move(patch)});
                    }
                ));
            });

            bool transformFailed = false;
            auto origNodeAndTransformedContents = kdl::collect_values(transformResults, kdl::overload(
                [&](const auto&) {
                    transformFailed = true;
                }
            ));

            if (transformFailed) {
                return UpdateLinkedGroupsError::TransformFailed;
            }

            // Move into map for easier lookup
            auto resultsMap = std::unordered_map<const Node*, NodeContents>{};
            for (auto& [origNode, transformedContents] : origNodeAndTransformedContents) {
                resultsMap.emplace(origNode, std::move(transformedContents));
            }
            origNodeAndTransformedContents.clear();

            // Do a recursive traversal of the input node tree again,
            // creating a matching tree structure, and move in the contents
            // we've transformed above.

            bool worldBoundsExceeded = false;
            std::function<std::unique_ptr<Node>(const Node*)> cloneAndTransformRecursive =
                [&](const Node* n) -> std::unique_ptr<Node> {
                    // First, clone `n`, and move in the new (transformed) content which was prepared for it above
                    std::unique_ptr<Node> clone = n->accept(kdl::overload(
                        [] (const WorldNode*) -> std::unique_ptr<Node> { ensure(false, "Linked group structure is valid"); },
                        [] (const LayerNode*) -> std::unique_ptr<Node> { ensure(false, "Linked group structure is valid"); },
                        [&](const GroupNode* groupNode) -> std::unique_ptr<Node> {
                            auto& group = std::get<Group>(resultsMap.at(groupNode).get());
                            return std::make_unique<GroupNode>(std::move(group));
                        },
                        [&](const EntityNode* entityNode) -> std::unique_ptr<Node> {
                            auto& entity = std::get<Entity>(resultsMap.at(entityNode).get());
                            return std::make_unique<EntityNode>(std::move(entity));
                        },
                        [&](const BrushNode* brushNode) -> std::unique_ptr<Node> {
                            auto& brush = std::get<Brush>(resultsMap.at(brushNode).get());
                            return std::make_unique<BrushNode>(std::move(brush));
                        },
                        [&](const PatchNode* patchNode) -> std::unique_ptr<Node> {
                            auto& patch = std::get<BezierPatch>(resultsMap.at(patchNode).get());
                            return std::make_unique<PatchNode>(std::move(patch));
                        }
                    ));

                    if (!worldBounds.contains(clone->logicalBounds())) {
                        worldBoundsExceeded = true;
                    }

                    // Recursively clone children of `n`
                    for (const Node* child : n->children()) {
                        std::unique_ptr<Node> childClone = cloneAndTransformRecursive(child);
                    
                        // attach it as a child of `clone`
                        clone->addChild(childClone.release());
                    }

                    return clone;
                };

            // Generate the output vector by applying `cloneAndTransformRecursive` to each child of `node`.
            auto result = kdl::vec_transform(node.children(), [&](const Node* child) -> std::unique_ptr<Node> {
                return cloneAndTransformRecursive(child);
            });

            if (worldBoundsExceeded) {
                return UpdateLinkedGroupsError::UpdateExceedsWorldBounds;
            }
            return {std::move(result)};
        }

        template <typename T>
        static void preserveGroupNames(const std::vector<T>& clonedNodes, const std::vector<Model::Node*>& correspondingNodes) {
            auto clIt = std::begin(clonedNodes);
            auto coIt = std::begin(correspondingNodes);
            while (clIt != std::end(clonedNodes) && coIt != std::end(correspondingNodes)) {
                auto& clonedNode = *clIt;
                const auto* correspondingNode = *coIt;

                clonedNode->accept(kdl::overload(
                    [] (WorldNode*) {},
                    [] (LayerNode*) {},
                    [&](GroupNode* clonedGroupNode) {
                        if (const auto* correspondingGroupNode = dynamic_cast<const GroupNode*>(correspondingNode)) {
                            auto group = clonedGroupNode->group();
                            group.setName(correspondingGroupNode->group().name());
                            clonedGroupNode->setGroup(std::move(group));
                            
                            preserveGroupNames(clonedGroupNode->children(), correspondingGroupNode->children());
                        }
                    },
                    [] (EntityNode*) {},
                    [] (BrushNode*) {},
                    [] (PatchNode*) {}
                ));

                ++clIt;
                ++coIt;
            }
        }

        static void preserveEntityProperties(EntityNode& clonedEntityNode, const EntityNode& correspondingEntityNode) {
            if (clonedEntityNode.entity().protectedProperties().empty() && 
                correspondingEntityNode.entity().protectedProperties().empty()) {
                return;
            }

            auto clonedEntity = clonedEntityNode.entity();
            const auto& correspondingEntity = correspondingEntityNode.entity();

            const auto allProtectedProperties = kdl::vec_sort_and_remove_duplicates(
                kdl::vec_concat(
                    clonedEntity.protectedProperties(),
                    correspondingEntity.protectedProperties()));

            clonedEntity.setProtectedProperties(correspondingEntity.protectedProperties());

            for (const auto& propertyKey : allProtectedProperties) {
                // this can change the order of properties
                clonedEntity.removeProperty(propertyKey);
                if (const auto* propertyValue = correspondingEntity.property(propertyKey)) {
                    clonedEntity.addOrUpdateProperty(propertyKey, *propertyValue);
                }
            }

            clonedEntityNode.setEntity(std::move(clonedEntity));
        }

        template <typename T>
        static void preserveEntityProperties(const std::vector<T>& clonedNodes, const std::vector<Node*>& correspondingNodes) {
            auto clIt = std::begin(clonedNodes);
            auto coIt = std::begin(correspondingNodes);
            while (clIt != std::end(clonedNodes) && coIt != std::end(correspondingNodes)) {
                auto& clonedNode = *clIt; // deduces either to std::unique_ptr<Node>& or Node*& depending on T
                const auto* correspondingNode = *coIt;

                clonedNode->accept(kdl::overload(
                    [] (WorldNode*) {},
                    [] (LayerNode*) {},
                    [&](GroupNode* clonedGroupNode) {
                        if (const auto* correspondingGroupNode = dynamic_cast<const GroupNode*>(correspondingNode)) {
                            preserveEntityProperties(clonedGroupNode->children(), correspondingGroupNode->children());
                        }
                    },
                    [&](EntityNode* clonedEntityNode) {
                        if (const auto* correspondingEntityNode = dynamic_cast<const EntityNode*>(correspondingNode)) {
                            preserveEntityProperties(*clonedEntityNode, *correspondingEntityNode);
                        }
                    },
                    [] (BrushNode*) {},
                    [] (PatchNode*) {}
                ));

                ++clIt;
                ++coIt;
            }
        }

        kdl::result<UpdateLinkedGroupsResult, UpdateLinkedGroupsError> updateLinkedGroups(const GroupNode& sourceGroupNode, const std::vector<Model::GroupNode*>& targetGroupNodes, const vm::bbox3& worldBounds) {
            const auto& sourceGroup = sourceGroupNode.group();
            const auto [success, invertedSourceTransformation] = vm::invert(sourceGroup.transformation());
            if (!success) {
                return UpdateLinkedGroupsError::TransformIsNotInvertible;
            }

            const auto _invertedSourceTransformation = invertedSourceTransformation;
            const auto targetGroupNodesToUpdate = kdl::vec_erase(targetGroupNodes, &sourceGroupNode);
            return kdl::for_each_result(targetGroupNodesToUpdate, [&](auto* targetGroupNode) {
                const auto transformation = targetGroupNode->group().transformation() * _invertedSourceTransformation;
                return cloneAndTransformChildren(sourceGroupNode, worldBounds, transformation)
                    .and_then([&](std::vector<std::unique_ptr<Node>>&& newChildren) -> kdl::result<std::pair<Node*, std::vector<std::unique_ptr<Node>>>, UpdateLinkedGroupsError> {
                        preserveGroupNames(newChildren, targetGroupNode->children());
                        preserveEntityProperties(newChildren, targetGroupNode->children());

                        return std::make_pair(targetGroupNode, std::move(newChildren));
                    });
            });
        }

        GroupNode::GroupNode(Group group) :
        m_group(std::move(group)),
        m_editState(EditState::Closed),
        m_boundsValid(false) {}

        const Group& GroupNode::group() const {
            return m_group;
        }

        Group GroupNode::setGroup(Group group) {
            using std::swap;
            swap(m_group, group);
            return group;
        }

        bool GroupNode::opened() const {
            return m_editState == EditState::Open;
        }

        bool GroupNode::hasOpenedDescendant() const {
            return m_editState == EditState::DescendantOpen;
        }

        bool GroupNode::closed() const {
            return m_editState == EditState::Closed;
        }

        void GroupNode::open() {
            assert(m_editState == EditState::Closed);
            setEditState(EditState::Open);
            openAncestors();
        }

        void GroupNode::close() {
            assert(m_editState == EditState::Open);
            setEditState(EditState::Closed);
            closeAncestors();
        }

        const std::optional<IdType>& GroupNode::persistentId() const {
            return m_persistentId;
        }

        void GroupNode::setPersistentId(const IdType persistentId) {
            m_persistentId = persistentId;
        }

        void GroupNode::resetPersistentId() {
            m_persistentId = std::nullopt;
        }

        void GroupNode::setEditState(const EditState editState) {
            m_editState = editState;
        }

        void GroupNode::setAncestorEditState(const EditState editState) {
            visitParent(kdl::overload(
                [=](auto&& thisLambda, WorldNode* world)   -> void { world->visitParent(thisLambda); },
                [=](auto&& thisLambda, LayerNode* layer)   -> void { layer->visitParent(thisLambda); },
                [=](auto&& thisLambda, GroupNode* group)   -> void { group->setEditState(editState); group->visitParent(thisLambda); },
                [=](auto&& thisLambda, EntityNode* entity) -> void { entity->visitParent(thisLambda); },
                [=](auto&& thisLambda, BrushNode* brush)   -> void { brush->visitParent(thisLambda); },
                [=](auto&& thisLambda, PatchNode* patch)   -> void { patch->visitParent(thisLambda); }
            ));
        }

        void GroupNode::openAncestors() {
            setAncestorEditState(EditState::DescendantOpen);
        }

        void GroupNode::closeAncestors() {
            setAncestorEditState(EditState::Closed);
        }

        const std::string& GroupNode::doGetName() const {
            return m_group.name();
        }

        const vm::bbox3& GroupNode::doGetLogicalBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_logicalBounds;
        }

        const vm::bbox3& GroupNode::doGetPhysicalBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_physicalBounds;
        }

        FloatType GroupNode::doGetProjectedArea(const vm::axis::type) const {
            return static_cast<FloatType>(0);
        }

        Node* GroupNode::doClone(const vm::bbox3& /* worldBounds */) const {
            GroupNode* group = new GroupNode(m_group);
            cloneAttributes(group);
            return group;
        }

        bool GroupNode::doCanAddChild(const Node* child) const {
            return child->accept(kdl::overload(
                [](const WorldNode*)  { return false; },
                [](const LayerNode*)  { return false; },
                [](const GroupNode*)  { return true;  },
                [](const EntityNode*) { return true;  },
                [](const BrushNode*)  { return true;  },
                [](const PatchNode*)  { return true;  }
            ));
        }

        bool GroupNode::doCanRemoveChild(const Node* /* child */) const {
            return true;
        }

        bool GroupNode::doRemoveIfEmpty() const {
            return true;
        }

        bool GroupNode::doShouldAddToSpacialIndex() const {
            return false;
        }

        void GroupNode::doChildWasAdded(Node* /* node */) {
            nodePhysicalBoundsDidChange();
        }

        void GroupNode::doChildWasRemoved(Node* /* node */) {
            nodePhysicalBoundsDidChange();
        }

        void GroupNode::doNodePhysicalBoundsDidChange() {
            invalidateBounds();
        }

        void GroupNode::doChildPhysicalBoundsDidChange() {
            invalidateBounds();
            nodePhysicalBoundsDidChange();
        }

        bool GroupNode::doSelectable() const {
            return true;
        }

        void GroupNode::doPick(const EditorContext&, const vm::ray3& /* ray */, PickResult&) {
            // For composite nodes (Groups, brush entities), pick rays don't hit the group
            // but instead just the primitives inside (brushes, point entities).
            // This avoids a potential performance trap where we'd have to exhaustively
            // test many objects if most of the map was inside groups, but it means
            // the pick results need to be postprocessed to account for groups (if desired).
            // See: https://github.com/TrenchBroom/TrenchBroom/issues/2742
        }

        void GroupNode::doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) {
            if (logicalBounds().contains(point)) {
                result.push_back(this);
            }

            for (auto* child : Node::children()) {
                child->findNodesContaining(point, result);
            }
        }

        void GroupNode::doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) {
            generator->generate(this, issues);
        }

        void GroupNode::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }

        void GroupNode::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        Node* GroupNode::doGetContainer() {
            return parent();
        }

        LayerNode* GroupNode::doGetContainingLayer() {
            return findContainingLayer(this);
        }

        GroupNode* GroupNode::doGetContainingGroup() {
            return findContainingGroup(this);
        }

        void GroupNode::invalidateBounds() {
            m_boundsValid = false;
        }

        void GroupNode::validateBounds() const {
            m_logicalBounds = computeLogicalBounds(children(), vm::bbox3(0.0));
            m_physicalBounds = computePhysicalBounds(children(), vm::bbox3(0.0));
            m_boundsValid = true;
        }

        void GroupNode::doAcceptTagVisitor(TagVisitor& visitor) {
            visitor.visit(*this);
        }

        void GroupNode::doAcceptTagVisitor(ConstTagVisitor& visitor) const {
            visitor.visit(*this);
        }
    }
}
