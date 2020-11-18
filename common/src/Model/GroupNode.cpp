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

#include "FloatType.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupSnapshot.h"
#include "Model/IssueGenerator.h"
#include "Model/LayerNode.h"
#include "Model/ModelUtils.h"
#include "Model/PickResult.h"
#include "Model/TagVisitor.h"
#include "Model/WorldNode.h"

#include <kdl/result.h>

#include <vecmath/ray.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        GroupNode::GroupNode(const std::string& name) :
        m_editState(Edit_Closed),
        m_boundsValid(false) {
            setName(name);
        }

        void GroupNode::setName(const std::string& name) {
            auto entity = m_entity;
            entity.addOrUpdateAttribute(AttributeNames::GroupName, name);
            setEntity(std::move(entity));
        }

        bool GroupNode::opened() const {
            return m_editState == Edit_Open;
        }

        bool GroupNode::hasOpenedDescendant() const {
            return m_editState == Edit_DescendantOpen;
        }

        bool GroupNode::closed() const {
            return m_editState == Edit_Closed;
        }

        void GroupNode::open() {
            assert(m_editState == Edit_Closed);
            setEditState(Edit_Open);
            openAncestors();
        }

        void GroupNode::close() {
            assert(m_editState == Edit_Open);
            setEditState(Edit_Closed);
            closeAncestors();
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
                [=](auto&& thisLambda, BrushNode* brush)   -> void { brush->visitParent(thisLambda); }
            ));
        }

        void GroupNode::openAncestors() {
            setAncestorEditState(Edit_DescendantOpen);
        }

        void GroupNode::closeAncestors() {
            setAncestorEditState(Edit_Closed);
        }

        const std::string& GroupNode::doGetName() const {
            static const auto NoName = std::string("");
            const auto* value = entity().attribute(AttributeNames::GroupName);
            return value ? *value : NoName;
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

        Node* GroupNode::doClone(const vm::bbox3& /* worldBounds */) const {
            GroupNode* group = new GroupNode(doGetName());
            cloneAttributes(group);
            return group;
        }

        NodeSnapshot* GroupNode::doTakeSnapshot() {
            return new GroupSnapshot(this);
        }

        bool GroupNode::doCanAddChild(const Node* child) const {
            return child->accept(kdl::overload(
                [](const WorldNode*)  { return false; },
                [](const LayerNode*)  { return false; },
                [](const GroupNode*)  { return true;  },
                [](const EntityNode*) { return true;  },
                [](const BrushNode*)  { return true;  }
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
            nodePhysicalBoundsDidChange(physicalBounds());
        }

        void GroupNode::doChildWasRemoved(Node* /* node */) {
            nodePhysicalBoundsDidChange(physicalBounds());
        }

        void GroupNode::doNodePhysicalBoundsDidChange() {
            invalidateBounds();
        }

        void GroupNode::doChildPhysicalBoundsDidChange() {
            const vm::bbox3 myOldBounds = physicalBounds();
            invalidateBounds();
            if (physicalBounds() != myOldBounds) {
                nodePhysicalBoundsDidChange(myOldBounds);
            }
        }

        bool GroupNode::doSelectable() const {
            return true;
        }

        void GroupNode::doPick(const vm::ray3& /* ray */, PickResult&) {
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

        void GroupNode::doAttributesDidChange(const vm::bbox3& /* oldBounds */) {
        }

        vm::vec3 GroupNode::doGetLinkSourceAnchor() const {
            return vm::vec3::zero();
        }

        vm::vec3 GroupNode::doGetLinkTargetAnchor() const {
            return vm::vec3::zero();
        }

        Node* GroupNode::doGetContainer() {
            return parent();
        }

        LayerNode* GroupNode::doGetLayer() {
            return findContainingLayer(this);
        }

        GroupNode* GroupNode::doGetGroup() {
            return findContainingGroup(this);
        }

        kdl::result<void, TransformError> GroupNode::doTransform(const vm::bbox3& worldBounds, const vm::mat4x4& transformation, const bool lockTextures) {
            for (auto* child : children()) {
                auto result = child->accept(kdl::overload(
                    [](Model::WorldNode*) {
                        return kdl::result<void, Model::TransformError>::success();
                    },
                    [](Model::LayerNode*) {
                        return kdl::result<void, Model::TransformError>::success();
                    },
                    [&](Model::GroupNode* group) {
                        return group->transform(worldBounds, transformation, lockTextures);
                    },
                    [&](Model::EntityNode* entity) {
                        return entity->transform(worldBounds, transformation, lockTextures);
                    },
                    [&](Model::BrushNode* brush) {
                        return brush->transform(worldBounds, transformation, lockTextures);
                    }
                ));
                if (result.is_error()) {
                    return result;
                }
            }

            return kdl::result<void, TransformError>::success();
        }

        bool GroupNode::doContains(const Node* node) const {
            return boundsContainNode(logicalBounds(), node);
        }

        bool GroupNode::doIntersects(const Node* node) const {
            return boundsIntersectNode(logicalBounds(), node);
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
