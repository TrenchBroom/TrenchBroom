/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
            void doVisit(World* world)   {}
            void doVisit(Layer* layer)   {}
            void doVisit(Group* group)   { group->setEditState(m_editState); }
            void doVisit(Entity* entity) {}
            void doVisit(Brush* brush)   {}
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

        const BBox3& Group::doGetBounds() const {
            if (!m_boundsValid)
                validateBounds();
            return m_bounds;
        }
        
        Node* Group::doClone(const BBox3& worldBounds) const {
            Group* group = new Group(m_name);
            cloneAttributes(group);
            return group;
        }

        NodeSnapshot* Group::doTakeSnapshot() {
            return new GroupSnapshot(this);
        }
        
        class CanAddChildToGroup : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const World* world)   { setResult(false); }
            void doVisit(const Layer* layer)   { setResult(false); }
            void doVisit(const Group* group)   { setResult(true); }
            void doVisit(const Entity* entity) { setResult(true); }
            void doVisit(const Brush* brush)   { setResult(true); }
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
            nodeBoundsDidChange();
        }
        
        void Group::doChildWasRemoved(Node* node) {
            nodeBoundsDidChange();
        }

        void Group::doNodeBoundsDidChange() {
            invalidateBounds();
        }
        
        void Group::doChildBoundsDidChange(Node* node) {
            nodeBoundsDidChange();
        }

        bool Group::doShouldPropagateDescendantEvents() const {
            return false;
        }

        bool Group::doSelectable() const {
            return true;
        }

        void Group::doPick(const Ray3& ray, PickResult& pickResult) const {
            if (!opened() && !hasOpenedDescendant()) {
                const FloatType distance = intersectWithRay(ray);
                if (!Math::isnan(distance)) {
                    const Vec3 hitPoint = ray.pointAtDistance(distance);
                    pickResult.addHit(Hit(GroupHit, distance, hitPoint, this));
                }
            }
            
            const NodeList& children = Node::children();
            NodeList::const_iterator it, end;
            for (it = children.begin(), end = children.end(); it != end; ++it) {
                const Node* child = *it;
                child->pick(ray, pickResult);
            }
        }
        
        FloatType Group::doIntersectWithRay(const Ray3& ray) const {
            const BBox3& myBounds = bounds();
            if (!myBounds.contains(ray.origin) && Math::isnan(myBounds.intersectWithRay(ray)))
                return Math::nan<FloatType>();
            
            IntersectNodeWithRayVisitor visitor(ray);
            iterate(visitor);
            if (!visitor.hasResult())
                return Math::nan<FloatType>();
            return visitor.result();
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
            return visitor.hasResult() ? visitor.result() : NULL;
        }

        Layer* Group::doGetLayer() const {
            FindLayerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : NULL;
        }
        
        Group* Group::doGetGroup() const {
            FindGroupVisitor visitor(false);
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : NULL;
        }

        void Group::doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) {
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
            ComputeNodeBoundsVisitor visitor(BBox3(0.0));
            iterate(visitor);
            m_bounds = visitor.bounds();
            m_boundsValid = true;
        }
    }
}
