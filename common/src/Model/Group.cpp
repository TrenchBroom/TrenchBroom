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

#include "Model/Brush.h"
#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/Entity.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        Group::Group(const String& name) :
        m_name(name),
        m_boundsValid(false) {}
        
        const String& Group::name() const {
            return m_name;
        }
        
        void Group::setName(const String& name) {
            m_name = name;
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
        
        void Group::doParentWillChange() {
            assert(!selected());
        }

        void Group::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }
        
        void Group::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        const BBox3& Group::doGetBounds() const {
            if (!m_boundsValid)
                validateBounds();
            return m_bounds;
        }

        class TransformGroup : public NodeVisitor {
        private:
            const Mat4x4d& m_transformation;
            bool m_lockTextures;
            const BBox3& m_worldBounds;
        public:
            TransformGroup(const Mat4x4d& transformation, const bool lockTextures, const BBox3& worldBounds) :
            m_transformation(transformation),
            m_lockTextures(lockTextures),
            m_worldBounds(worldBounds) {}
        private:
            void doVisit(World* world)   {}
            void doVisit(Layer* layer)   {}
            void doVisit(Group* group)   {  group->transform(m_transformation, m_lockTextures, m_worldBounds); }
            void doVisit(Entity* entity) { entity->transform(m_transformation, m_lockTextures, m_worldBounds); }
            void doVisit(Brush* brush)   {  brush->transform(m_transformation, m_lockTextures, m_worldBounds); }
        };
        
        void Group::doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) {
            TransformGroup visitor(transformation, lockTextures, worldBounds);
            iterate(visitor);
        }
        
        bool Group::doContains(const Node* node) const {
        }
        
        bool Group::doIntersects(const Node* node) const {
        }

        void Group::doWasSelected() {
            familyMemberWasSelected();
        }
        
        void Group::doWasDeselected() {
            familyMemberWasDeselected();
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
