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
        m_boundsValid(false) {}

        bool Entity::brushEntity() const {
            return !pointEntity();
        }

        bool Entity::pointEntity() const {
            if (!hasChildren())
                return true;
            if (definition() == NULL)
                return !hasChildren();
            return definition()->type() == Assets::EntityDefinition::Type_PointEntity;
        }
        
        bool Entity::hasBrushEntityDefinition() const {
            return m_definition != NULL && definition()->type() == Assets::EntityDefinition::Type_BrushEntity;
        }

        bool Entity::hasPointEntityDefinition() const {
            return m_definition != NULL && definition()->type() == Assets::EntityDefinition::Type_PointEntity;
        }
        
        bool Entity::hasPointEntityModel() const {
            return hasPointEntityDefinition();
        }
        
        Vec3 Entity::origin() const {
            return Vec3::parse(attribute(AttributeNames::Origin, ""));
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
            if (!hasPointEntityModel())
                return Assets::ModelSpecification();
            Assets::PointEntityDefinition* pointDefinition = static_cast<Assets::PointEntityDefinition*>(m_definition);
            return pointDefinition->model(m_attributes);
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
            void doVisit(const World* world) override   { setResult(false); }
            void doVisit(const Layer* layer) override   { setResult(false); }
            void doVisit(const Group* group) override   { setResult(true); }
            void doVisit(const Entity* entity) override { setResult(true); }
            void doVisit(const Brush* brush) override   { setResult(true); }
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
                for (const Node* child : Node::children())
                    child->pick(ray, pickResult);
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
        
        void Entity::doFindNodesContaining(const Vec3& point, NodeList& result) {
            if (hasChildren()) {
                for (Node* child : Node::children())
                    child->findNodesContaining(point, result);
            } else {
                if (bounds().contains(point))
                    result.push_back(this);
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
            void doVisit(World* world) override   {}
            void doVisit(Layer* layer) override   {}
            void doVisit(Group* group) override   {}
            void doVisit(Entity* entity) override {}
            void doVisit(Brush* brush) override   { brush->transform(m_transformation, m_lockTextures, m_worldBounds); }
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
                
                // applying rotation has side effects (e.g. normalizing "angles")
                // so only do it if there is actually some rotation.
                const Mat4x4 rotation = stripTranslation(transformation);
                if (!rotation.equals(Mat4x4::Identity))
                	applyRotation(rotation);
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
            if (hasChildren()) {
                ComputeNodeBoundsVisitor visitor(DefaultBounds);
                iterate(visitor);
                m_bounds = visitor.bounds();
            } else if (def != NULL && def->type() == Assets::EntityDefinition::Type_PointEntity) {
                m_bounds = static_cast<const Assets::PointEntityDefinition*>(def)->bounds();
                m_bounds.translate(origin());
            } else {
                m_bounds = DefaultBounds;
                m_bounds.translate(origin());
            }
            m_boundsValid = true;
        }
    }
}
