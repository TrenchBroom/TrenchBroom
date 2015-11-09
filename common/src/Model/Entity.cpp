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

#include "Model/BoundsContainsNodeVisitor.h"
#include "Model/BoundsIntersectsNodeVisitor.h"
#include "Model/Brush.h"
#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/EntitySnapshot.h"
#include "Model/FindContainerVisitor.h"
#include "Model/FindGroupVisitor.h"
#include "Model/FindLayerVisitor.h"
#include "Model/IntersectNodeWithRayVisitor.h"
#include "Model/IssueGenerator.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"

namespace TrenchBroom {
    namespace Model {
        const Hit::HitType Entity::EntityHit = Hit::freeHitType();
        const BBox3 Entity::DefaultBounds(8.0);

        Entity::Entity() :
        AttributableNode(),
        Object(),
        m_boundsValid(false),
        m_model(NULL) {}

        bool Entity::pointEntity() const {
            if (definition() == NULL)
                return !hasChildren();
            return definition()->type() == Assets::EntityDefinition::Type_PointEntity;
        }
        
        Vec3 Entity::origin() const {
            return Vec3::parse(attribute(AttributeNames::Origin));
        }

        Mat4x4 Entity::rotation() const {
            return EntityRotationPolicy::getRotation(this);
        }

        FloatType Entity::area(Math::Axis::Type axis) const {
            const Vec3 size = bounds().size();
            switch (axis) {
                case Math::Axis::AX:
                    return size.y() * size.z();
                case Math::Axis::AY:
                    return size.x() * size.z();
                case Math::Axis::AZ:
                    return size.y() * size.z();
                default:
                    return 0.0;
            }
        }

        void Entity::setOrigin(const Vec3& origin) {
            addOrUpdateAttribute(AttributeNames::Origin, origin.rounded().asString());
        }
        
        void Entity::applyRotation(const Mat4x4& transformation) {
            EntityRotationPolicy::applyRotation(this, transformation);
        }

        Assets::ModelSpecification Entity::modelSpecification() const {
            if (m_definition == NULL || !pointEntity())
                return Assets::ModelSpecification();
            Assets::PointEntityDefinition* pointDefinition = static_cast<Assets::PointEntityDefinition*>(m_definition);
            return pointDefinition->model(m_attributes);
        }
        
        Assets::EntityModel* Entity::model() const {
            return m_model;
        }
        
        void Entity::setModel(Assets::EntityModel* model) {
            m_model = model;
        }

        const BBox3& Entity::doGetBounds() const {
            if (!m_boundsValid)
                validateBounds();
            return m_bounds;
        }
        
        Node* Entity::doClone(const BBox3& worldBounds) const {
            Entity* entity = new Entity();
            cloneAttributes(entity);
            entity->setDefinition(definition());
            entity->setAttributes(attributes());
            return entity;
        }

        NodeSnapshot* Entity::doTakeSnapshot() {
            const EntityAttribute origin(AttributeNames::Origin, attribute(AttributeNames::Origin), NULL);
            
            const AttributeName rotationName = EntityRotationPolicy::getAttribute(this);
            const EntityAttribute rotation(rotationName, attribute(rotationName), NULL);
            
            return new EntitySnapshot(this, origin, rotation);
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
        
        bool Entity::doRemoveIfEmpty() const {
            return true;
        }

        void Entity::doChildWasAdded(Node* node) {
            nodeBoundsDidChange();
        }
        
        void Entity::doChildWasRemoved(Node* node) {
            nodeBoundsDidChange();
        }

        void Entity::doNodeBoundsDidChange() {
            invalidateBounds();
        }
        
        void Entity::doChildBoundsDidChange(Node* node) {
            nodeBoundsDidChange();
        }

        bool Entity::doSelectable() const {
            return !hasChildren();
        }

        void Entity::doPick(const Ray3& ray, PickResult& pickResult) const {
            if (hasChildren()) {
                const NodeList& children = Node::children();
                NodeList::const_iterator it, end;
                for (it = children.begin(), end = children.end(); it != end; ++it) {
                    const Node* child = *it;
                    child->pick(ray, pickResult);
                }
            } else {
                const BBox3& myBounds = bounds();
                if (!myBounds.contains(ray.origin)) {
                    const FloatType distance = myBounds.intersectWithRay(ray);
                    if (!Math::isnan(distance)) {
                        const Vec3 hitPoint = ray.pointAtDistance(distance);
                        pickResult.addHit(Hit(EntityHit, distance, hitPoint, this));
                    }
                }
            }
        }
        
        FloatType Entity::doIntersectWithRay(const Ray3& ray) const {
            if (hasChildren()) {
                const BBox3& myBounds = bounds();
                if (!myBounds.contains(ray.origin) && Math::isnan(myBounds.intersectWithRay(ray)))
                    return Math::nan<FloatType>();
                
                IntersectNodeWithRayVisitor visitor(ray);
                iterate(visitor);
                if (!visitor.hasResult())
                    return Math::nan<FloatType>();
                return visitor.result();
            } else {
                const BBox3& myBounds = bounds();
                if (!myBounds.contains(ray.origin))
                    return myBounds.intersectWithRay(ray);
                return Math::nan<FloatType>();
            }
        }

        void Entity::doGenerateIssues(const IssueGenerator* generator, IssueList& issues) {
            generator->generate(this, issues);
        }
        
        void Entity::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }
        
        void Entity::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        void Entity::doAttributesDidChange() {
            nodeBoundsDidChange();
        }
        
        bool Entity::doIsAttributeNameMutable(const AttributeName& name) const {
            return true;
        }
        
        bool Entity::doIsAttributeValueMutable(const AttributeName& name) const {
            return true;
        }

        Vec3 Entity::doGetLinkSourceAnchor() const {
            return bounds().center();
        }
        
        Vec3 Entity::doGetLinkTargetAnchor() const {
            return bounds().center();
        }

        Node* Entity::doGetContainer() const {
            FindContainerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : NULL;
        }

        Layer* Entity::doGetLayer() const {
            FindLayerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : NULL;
        }
        
        Group* Entity::doGetGroup() const {
            FindGroupVisitor visitor(false);
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : NULL;
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
                const NotifyNodeChange nodeChange(this);
                TransformEntity visitor(transformation, lockTextures, worldBounds);
                iterate(visitor);
            } else {
                // node change is called by setOrigin already
                const Vec3 bottomCenter = Vec3(bounds().center().xy(), bounds().min.z());
                const Vec3 delta = bottomCenter - origin();
                const Vec3 transformedCenter = transformation * bottomCenter;
                
                setOrigin(transformedCenter - delta);
                applyRotation(stripTranslation(transformation));
            }
        }
        
        bool Entity::doContains(const Node* node) const {
            BoundsContainsNodeVisitor contains(bounds());
            node->accept(contains);
            assert(contains.hasResult());
            return contains.result();
        }
        
        bool Entity::doIntersects(const Node* node) const {
            BoundsIntersectsNodeVisitor intersects(bounds());
            node->accept(intersects);
            assert(intersects.hasResult());
            return intersects.result();
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
