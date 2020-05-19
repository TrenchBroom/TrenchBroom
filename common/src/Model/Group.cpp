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

#include "Group.h"

#include "FloatType.h"
#include "Model/BoundsContainsNodeVisitor.h"
#include "Model/BoundsIntersectsNodeVisitor.h"
#include "Model/BrushNode.h"
#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/Entity.h"
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
        Group::Group(const std::string& name) :
        m_name(name),
        m_editState(Edit_Closed),
        m_boundsValid(false) {}

        void Group::setName(const std::string& name) {
            m_name = name;
        }

        bool Group::opened() const {
            return m_editState == Edit_Open;
        }

        void Group::open() {
            assert(m_editState == Edit_Closed);
            setEditState(Edit_Open);
            openAncestors();
        }

        void Group::close() {
            assert(m_editState == Edit_Open);
            setEditState(Edit_Closed);
            closeAncestors();
        }

        void Group::setEditState(const EditState editState) {
            m_editState = editState;
        }

        class Group::SetEditStateVisitor : public NodeVisitor {
        private:
            EditState m_editState;
        public:
            SetEditStateVisitor(const EditState editState) : m_editState(editState) {}
        private:
            void doVisit(World*) override       {}
            void doVisit(Layer*) override       {}
            void doVisit(Group* group) override { group->setEditState(m_editState); }
            void doVisit(Entity*) override      {}
            void doVisit(BrushNode*) override       {}
        };

        void Group::openAncestors() {
            SetEditStateVisitor visitor(Edit_DescendantOpen);
            escalate(visitor);
        }

        void Group::closeAncestors() {
            SetEditStateVisitor visitor(Edit_Closed);
            escalate(visitor);
        }

        bool Group::hasOpenedDescendant() const {
            return m_editState == Edit_DescendantOpen;
        }

        const std::string& Group::doGetName() const {
            return m_name;
        }

        const vm::bbox3& Group::doGetLogicalBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_logicalBounds;
        }

        const vm::bbox3& Group::doGetPhysicalBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_physicalBounds;
        }

        Node* Group::doClone(const vm::bbox3& /* worldBounds */) const {
            Group* group = new Group(m_name);
            cloneAttributes(group);
            return group;
        }

        NodeSnapshot* Group::doTakeSnapshot() {
            return new GroupSnapshot(this);
        }

        class CanAddChildToGroup : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const World*) override  { setResult(false); }
            void doVisit(const Layer*) override  { setResult(false); }
            void doVisit(const Group*) override  { setResult(true); }
            void doVisit(const Entity*) override { setResult(true); }
            void doVisit(const BrushNode*) override  { setResult(true); }
        };

        bool Group::doCanAddChild(const Node* child) const {
            CanAddChildToGroup visitor;
            child->accept(visitor);
            return visitor.result();
        }

        bool Group::doCanRemoveChild(const Node* /* child */) const {
            return true;
        }

        bool Group::doRemoveIfEmpty() const {
            return true;
        }

        bool Group::doShouldAddToSpacialIndex() const {
            return false;
        }

        void Group::doChildWasAdded(Node* /* node */) {
            nodePhysicalBoundsDidChange(physicalBounds());
        }

        void Group::doChildWasRemoved(Node* /* node */) {
            nodePhysicalBoundsDidChange(physicalBounds());
        }

        void Group::doNodePhysicalBoundsDidChange() {
            invalidateBounds();
        }

        void Group::doChildPhysicalBoundsDidChange() {
            const vm::bbox3 myOldBounds = physicalBounds();
            invalidateBounds();
            if (physicalBounds() != myOldBounds) {
                nodePhysicalBoundsDidChange(myOldBounds);
            }
        }

        bool Group::doSelectable() const {
            return true;
        }

        void Group::doPick(const vm::ray3& /* ray */, PickResult&) {
            // For composite nodes (Groups, brush entities), pick rays don't hit the group
            // but instead just the primitives inside (brushes, point entities).
            // This avoids a potential performance trap where we'd have to exhaustively
            // test many objects if most of the map was inside groups, but it means
            // the pick results need to be postprocessed to account for groups (if desired).
            // See: https://github.com/kduske/TrenchBroom/issues/2742
        }

        void Group::doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) {
            if (logicalBounds().contains(point)) {
                result.push_back(this);
            }

            for (auto* child : Node::children()) {
                child->findNodesContaining(point, result);
            }
        }

        void Group::doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) {
            generator->generate(this, issues);
        }

        void Group::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }

        void Group::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        Node* Group::doGetContainer() const {
            FindContainerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        Layer* Group::doGetLayer() const {
            FindLayerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        Group* Group::doGetGroup() const {
            FindGroupVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        void Group::doTransform(const vm::mat4x4& transformation, const bool lockTextures, const vm::bbox3& worldBounds) {
            TransformObjectVisitor visitor(transformation, lockTextures, worldBounds);
            iterate(visitor);
        }

        bool Group::doContains(const Node* node) const {
            BoundsContainsNodeVisitor contains(logicalBounds());
            node->accept(contains);
            assert(contains.hasResult());
            return contains.result();
        }

        bool Group::doIntersects(const Node* node) const {
            BoundsIntersectsNodeVisitor intersects(logicalBounds());
            node->accept(intersects);
            assert(intersects.hasResult());
            return intersects.result();
        }

        void Group::invalidateBounds() {
            m_boundsValid = false;
        }

        void Group::validateBounds() const {
            ComputeNodeBoundsVisitor visitor(BoundsType::Logical, vm::bbox3(0.0));
            iterate(visitor);
            m_logicalBounds = visitor.bounds();

            ComputeNodeBoundsVisitor physicalBoundsVisitor(BoundsType::Physical, vm::bbox3(0.0));
            iterate(physicalBoundsVisitor);
            m_physicalBounds = physicalBoundsVisitor.bounds();

            m_boundsValid = true;
        }

        void Group::doAcceptTagVisitor(TagVisitor& visitor) {
            visitor.visit(*this);
        }

        void Group::doAcceptTagVisitor(ConstTagVisitor& visitor) const {
            visitor.visit(*this);
        }
    }
}
