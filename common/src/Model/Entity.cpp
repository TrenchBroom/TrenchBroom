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

#include "Entity.h"

#include "Model/Brush.h"
#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        const BBox3 Entity::DefaultBounds(8.0);

        Entity::Entity() :
        m_boundsValid(false) {}

        bool Entity::pointEntity() const {
            if (definition() == NULL)
                return !hasChildren();
            return definition()->type() == Assets::EntityDefinition::Type_PointEntity;
        }
        
        const Vec3 Entity::origin() const {
            return Vec3::parse(attribute(AttributeNames::Origin));
        }

        void Entity::setOrigin(const Vec3& origin) {
            addOrUpdateAttribute(AttributeNames::Origin, origin.rounded().asString());
        }
        
        void Entity::applyRotation(const Mat4x4& transformation) {
            EntityRotationPolicy::applyRotation(this, transformation);
        }

        class CanAddChildToEntity : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const World* world)   { setResult(false); }
            void doVisit(const Layer* layer)   { setResult(false); }
            void doVisit(const Group* group)   { setResult(true); }
            void doVisit(const Entity* entity) { setResult(true); }
            void doVisit(const Brush* brush)   { setResult(true); }
        };

        bool Entity::doCanAddChild(const Node* child) const {
            CanAddChildToEntity visitor;
            child->accept(visitor);
            return visitor.result();
        }
        
        bool Entity::doCanRemoveChild(const Node* child) const {
            return true;
        }
        
        void Entity::doParentWillChange() {
            assert(!selected());
        }

        void Entity::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }
        
        void Entity::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        void Entity::doAttributesDidChange() {
            invalidateBounds();
        }
        
        bool Entity::doCanAddOrUpdateAttribute(const AttributeName& name, const AttributeValue& value) const {
            return true;
        }
        
        bool Entity::doCanRenameAttribute(const AttributeName& name, const AttributeName& newName) const {
            return true;
        }
        
        bool Entity::doCanRemoveAttribute(const AttributeName& name) const {
            return true;
        }

        const BBox3& Entity::doGetBounds() const {
            if (!m_boundsValid)
                validateBounds();
            return m_bounds;
        }
        
        class TransformEntity : public NodeVisitor {
        private:
            const Mat4x4d& m_transformation;
            bool m_lockTextures;
            const BBox3& m_worldBounds;
        public:
            TransformEntity(const Mat4x4d& transformation, const bool lockTextures, const BBox3& worldBounds) :
            m_transformation(transformation),
            m_lockTextures(lockTextures),
            m_worldBounds(worldBounds) {}
        private:
            void doVisit(World* world)   {}
            void doVisit(Layer* layer)   {}
            void doVisit(Group* group)   {}
            void doVisit(Entity* entity) {}
            void doVisit(Brush* brush)   { brush->transform(m_transformation, m_lockTextures, m_worldBounds); }
        };

        void Entity::doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds) {
            if (hasChildren()) {
                TransformEntity visitor(transformation, lockTextures, worldBounds);
                iterate(visitor);
            } else {
                setOrigin(transformation * origin());
                applyRotation(stripTranslation(transformation));
            }
        }
        
        bool Entity::doContains(const Node* node) const {
        }
        
        bool Entity::doIntersects(const Node* node) const {
        }

        void Entity::doWasSelected() {
            familyMemberWasSelected();
        }
        
        void Entity::doWasDeselected() {
            familyMemberWasDeselected();
        }

        void Entity::invalidateBounds() {
            m_boundsValid = false;
        }
        
        void Entity::validateBounds() const {
            const Assets::EntityDefinition* def = definition();
            if (def != NULL && def->type() == Assets::EntityDefinition::Type_PointEntity) {
                m_bounds = static_cast<const Assets::PointEntityDefinition*>(def)->bounds();
                m_bounds.translate(origin());
            } else if (hasChildren()) {
                ComputeNodeBoundsVisitor visitor(DefaultBounds);
                iterate(visitor);
                m_bounds = visitor.bounds();
            } else {
                m_bounds = DefaultBounds;
                m_bounds.translate(origin());
            }
            m_boundsValid = true;
        }
    }
}
