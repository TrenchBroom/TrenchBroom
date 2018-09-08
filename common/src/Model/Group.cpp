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

#include "Hit.h"
#include "Model/BoundsContainsNodeVisitor.h"
#include "Model/BoundsIntersectsNodeVisitor.h"
#include "Model/Brush.h"
#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/Entity.h"
#include "Model/FindContainerVisitor.h"
#include "Model/FindGroupVisitor.h"
#include "Model/FindLayerVisitor.h"
#include "Model/GroupSnapshot.h"
#include "Model/IntersectNodeWithRayVisitor.h"
#include "Model/IssueGenerator.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"
#include "Model/TransformObjectVisitor.h"

namespace TrenchBroom {
    namespace Model {
        const Hit::HitType Group::GroupHit = Hit::freeHitType();

        Group::Group(const String& name) :
        m_name(name),
        m_editState(Edit_Closed),
        m_boundsValid(false) {}
        
        void Group::setName(const String& name) {
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
            void doVisit(World* world) override   {}
            void doVisit(Layer* layer) override   {}
            void doVisit(Group* group) override   { group->setEditState(m_editState); }
            void doVisit(Entity* entity) override {}
            void doVisit(Brush* brush) override   {}
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

        const String& Group::doGetName() const {
            return m_name;
        }

        const vm::bbox3& Group::doGetBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_bounds;
        }
        
        Node* Group::doClone(const vm::bbox3& worldBounds) const {
            Group* group = new Group(m_name);
            cloneAttributes(group);
            return group;
        }

        NodeSnapshot* Group::doTakeSnapshot() {
            return new GroupSnapshot(this);
        }
        
        class CanAddChildToGroup : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const World* world) override   { setResult(false); }
            void doVisit(const Layer* layer) override   { setResult(false); }
            void doVisit(const Group* group) override   { setResult(true); }
            void doVisit(const Entity* entity) override { setResult(true); }
            void doVisit(const Brush* brush) override   { setResult(true); }
        };
        
        bool Group::doCanAddChild(const Node* child) const {
            CanAddChildToGroup visitor;
            child->accept(visitor);
            return visitor.result();
        }
        
        bool Group::doCanRemoveChild(const Node* child) const {
            return true;
        }

        bool Group::doRemoveIfEmpty() const {
            return true;
        }

        void Group::doChildWasAdded(Node* node) {
            nodeBoundsDidChange(bounds());
        }
        
        void Group::doChildWasRemoved(Node* node) {
            nodeBoundsDidChange(bounds());
        }

        void Group::doNodeBoundsDidChange(const vm::bbox3& oldBounds) {
            invalidateBounds();
        }

        void Group::doChildBoundsDidChange(Node* node, const vm::bbox3& oldBounds) {
            const vm::bbox3 myOldBounds = bounds();
            invalidateBounds();
            if (bounds() != myOldBounds) {
                nodeBoundsDidChange(myOldBounds);
            }
        }

        bool Group::doSelectable() const {
            return true;
        }

        void Group::doPick(const vm::ray3& ray, PickResult& pickResult) const {
            // A group can only be picked if and only if all of the following conditions are met
            // * it is closed or has no open descendant
            // * it is top level or has an open parent
            if ((!opened() && !hasOpenedDescendant()) && groupOpened()) {
                const auto distance = intersectWithRay(ray);
                if (!vm::isnan(distance)) {
                    const auto hitPoint = ray.pointAtDistance(distance);
                    pickResult.addHit(Hit(GroupHit, distance, hitPoint, this));
                }
            }
        }
        
        void Group::doFindNodesContaining(const vm::vec3& point, NodeList& result) {
            if (bounds().contains(point)) {
                result.push_back(this);
            }

            for (auto* child : Node::children()) {
                child->findNodesContaining(point, result);
            }
        }

        FloatType Group::doIntersectWithRay(const vm::ray3& ray) const {
            const auto& myBounds = bounds();
            if (!myBounds.contains(ray.origin) && vm::isnan(intersect(ray, myBounds))) {
                return vm::nan<FloatType>();
            }

            IntersectNodeWithRayVisitor visitor(ray);
            iterate(visitor);
            if (!visitor.hasResult()) {
                return vm::nan<FloatType>();
            } else {
                return visitor.result();
            }
        }
        
        void Group::doGenerateIssues(const IssueGenerator* generator, IssueList& issues) {
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
            BoundsContainsNodeVisitor contains(bounds());
            node->accept(contains);
            assert(contains.hasResult());
            return contains.result();
        }
        
        bool Group::doIntersects(const Node* node) const {
            BoundsIntersectsNodeVisitor intersects(bounds());
            node->accept(intersects);
            assert(intersects.hasResult());
            return intersects.result();
        }

        void Group::invalidateBounds() {
            m_boundsValid = false;
        }
        
        void Group::validateBounds() const {
            ComputeNodeBoundsVisitor visitor(vm::bbox3(0.0));
            iterate(visitor);
            m_bounds = visitor.bounds();
            m_boundsValid = true;
        }
    }
}
