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

#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        const BBox3 Entity::DefaultBounds(8.0);

        Entity::Entity() :
        m_boundsValid(false) {}

        class CanAddChildToEntity : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const World* world)   { setResult(false); }
            void doVisit(const Layer* layer)   { setResult(false); }
            void doVisit(const Group* group)   { setResult(true); }
            void doVisit(const Entity* entity) { setResult(true); }
            void doVisit(const Brush* brush)   { setResult(true); }
        };

        bool Entity::doCanAddChild(Node* child) const {
            CanAddChildToEntity visitor;
            child->accept(visitor);
            return visitor.result();
        }
        
        bool Entity::doCanRemoveChild(Node* child) const {
            return true;
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
        }
        
        bool Entity::doCanRenameAttribute(const AttributeName& name, const AttributeName& newName) const {
        }
        
        bool Entity::doCanRemoveAttribute(const AttributeName& name) const {
        }

        const BBox3& Entity::doGetBounds() const {
            if (!m_boundsValid)
                validateBounds();
            return m_bounds;
        }
        
        void Entity::doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds) {
        }
        
        bool Entity::doContains(const Node* node) const {
        }
        
        bool Entity::doIntersects(const Node* node) const {
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
