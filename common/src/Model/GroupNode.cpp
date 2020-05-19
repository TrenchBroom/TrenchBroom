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
#include "Model/BoundsContainsNodeVisitor.h"
#include "Model/BoundsIntersectsNodeVisitor.h"
#include "Model/BrushNode.h"
#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/EntityNode.h"
#include "Model/FindContainerVisitor.h"
#include "Model/FindGroupVisitor.h"
#include "Model/FindLayerVisitor.h"
#include "Model/GroupSnapshot.h"
#include "Model/IssueGenerator.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"
#include "Model/TransformObjectVisitor.h"
#include "Model/TagVisitor.h"

#include <vecmath/ray.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        GroupNode::GroupNode(const std::string& name) :
        m_name(name),
        m_editState(Edit_Closed),
        m_boundsValid(false) {}

        void GroupNode::setName(const std::string& name) {
            m_name = name;
        }

        bool GroupNode::opened() const {
            return m_editState == Edit_Open;
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

        class GroupNode::SetEditStateVisitor : public NodeVisitor {
        private:
            EditState m_editState;
        public:
            SetEditStateVisitor(const EditState editState) : m_editState(editState) {}
        private:
            void doVisit(WorldNode*) override       {}
            void doVisit(LayerNode*) override       {}
            void doVisit(GroupNode* group) override { group->setEditState(m_editState); }
            void doVisit(EntityNode*) override      {}
            void doVisit(BrushNode*) override       {}
        };

        void GroupNode::openAncestors() {
            SetEditStateVisitor visitor(Edit_DescendantOpen);
            escalate(visitor);
        }

        void GroupNode::closeAncestors() {
            SetEditStateVisitor visitor(Edit_Closed);
            escalate(visitor);
        }

        bool GroupNode::hasOpenedDescendant() const {
            return m_editState == Edit_DescendantOpen;
        }

        const std::string& GroupNode::doGetName() const {
            return m_name;
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
            GroupNode* group = new GroupNode(m_name);
            cloneAttributes(group);
            return group;
        }

        NodeSnapshot* GroupNode::doTakeSnapshot() {
            return new GroupSnapshot(this);
        }

        class CanAddChildToGroup : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const WorldNode*) override  { setResult(false); }
            void doVisit(const LayerNode*) override  { setResult(false); }
            void doVisit(const GroupNode*) override  { setResult(true); }
            void doVisit(const EntityNode*) override { setResult(true); }
            void doVisit(const BrushNode*) override  { setResult(true); }
        };

        bool GroupNode::doCanAddChild(const Node* child) const {
            CanAddChildToGroup visitor;
            child->accept(visitor);
            return visitor.result();
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
            // See: https://github.com/kduske/TrenchBroom/issues/2742
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

        Node* GroupNode::doGetContainer() const {
            FindContainerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        LayerNode* GroupNode::doGetLayer() const {
            FindLayerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        GroupNode* GroupNode::doGetGroup() const {
            FindGroupVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        void GroupNode::doTransform(const vm::mat4x4& transformation, const bool lockTextures, const vm::bbox3& worldBounds) {
            TransformObjectVisitor visitor(transformation, lockTextures, worldBounds);
            iterate(visitor);
        }

        bool GroupNode::doContains(const Node* node) const {
            BoundsContainsNodeVisitor contains(logicalBounds());
            node->accept(contains);
            assert(contains.hasResult());
            return contains.result();
        }

        bool GroupNode::doIntersects(const Node* node) const {
            BoundsIntersectsNodeVisitor intersects(logicalBounds());
            node->accept(intersects);
            assert(intersects.hasResult());
            return intersects.result();
        }

        void GroupNode::invalidateBounds() {
            m_boundsValid = false;
        }

        void GroupNode::validateBounds() const {
            ComputeNodeBoundsVisitor visitor(BoundsType::Logical, vm::bbox3(0.0));
            iterate(visitor);
            m_logicalBounds = visitor.bounds();

            ComputeNodeBoundsVisitor physicalBoundsVisitor(BoundsType::Physical, vm::bbox3(0.0));
            iterate(physicalBoundsVisitor);
            m_physicalBounds = physicalBoundsVisitor.bounds();

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
