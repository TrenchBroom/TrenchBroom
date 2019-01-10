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

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/bbox.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/intersection.h>
#include <vecmath/util.h>

namespace TrenchBroom {
    namespace Model {
        const Hit::HitType Entity::EntityHit = Hit::freeHitType();
        const vm::bbox3 Entity::DefaultBounds(8.0);

        Entity::Entity() :
        AttributableNode(),
        Object(),
        m_boundsValid(false) {
            cacheAttributes();
        }

        bool Entity::brushEntity() const {
            return hasChildren();
        }

        bool Entity::pointEntity() const {
            return !brushEntity();
        }
        
        bool Entity::hasEntityDefinition() const {
            return m_definition != nullptr;
        }

        bool Entity::hasBrushEntityDefinition() const {
            return hasEntityDefinition() && definition()->type() == Assets::EntityDefinition::Type_BrushEntity;
        }

        bool Entity::hasPointEntityDefinition() const {
            return hasEntityDefinition() && definition()->type() == Assets::EntityDefinition::Type_PointEntity;
        }
        
        bool Entity::hasPointEntityModel() const {
            return hasPointEntityDefinition();
        }
        
        const vm::vec3& Entity::origin() const {
            return m_cachedOrigin;
        }

        const vm::mat4x4& Entity::rotation() const {
            return m_cachedRotation;
        }

        FloatType Entity::area(vm::axis::type axis) const {
            const vm::vec3 size = bounds().size();
            switch (axis) {
                case vm::axis::x:
                    return size.y() * size.z();
                case vm::axis::y:
                    return size.x() * size.z();
                case vm::axis::z:
                    return size.y() * size.z();
                default:
                    return 0.0;
            }
        }

        void Entity::cacheAttributes() {
            m_cachedOrigin = vm::vec3::parse(attribute(AttributeNames::Origin, ""));
            m_cachedRotation = EntityRotationPolicy::getRotation(this);
        }

        void Entity::setOrigin(const vm::vec3& origin) {
            addOrUpdateAttribute(AttributeNames::Origin, StringUtils::toString(round(origin)));
        }
        
        void Entity::applyRotation(const vm::mat4x4& transformation) {
            EntityRotationPolicy::applyRotation(this, transformation);
        }

        Assets::ModelSpecification Entity::modelSpecification() const {
            if (!hasPointEntityModel())
                return Assets::ModelSpecification();
            Assets::PointEntityDefinition* pointDefinition = static_cast<Assets::PointEntityDefinition*>(m_definition);
            return pointDefinition->model(m_attributes);
        }

        const vm::bbox3& Entity::doGetBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_bounds;
        }
        
        Node* Entity::doClone(const vm::bbox3& worldBounds) const {
            auto* entity = new Entity();
            cloneAttributes(entity);
            entity->setDefinition(definition());
            entity->setAttributes(attributes());
            return entity;
        }

        NodeSnapshot* Entity::doTakeSnapshot() {
            const EntityAttribute origin(AttributeNames::Origin, attribute(AttributeNames::Origin), nullptr);
            
            const AttributeName rotationName = EntityRotationPolicy::getAttribute(this);
            const EntityAttribute rotation(rotationName, attribute(rotationName), nullptr);
            
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

        bool Entity::doAddToNodeTree() const {
            return true;
        }

        void Entity::doChildWasAdded(Node* node) {
            nodeBoundsDidChange(bounds());
        }
        
        void Entity::doChildWasRemoved(Node* node) {
            nodeBoundsDidChange(bounds());
        }

        void Entity::doNodeBoundsDidChange(const vm::bbox3& oldBounds) {
            invalidateBounds();
        }
        
        void Entity::doChildBoundsDidChange(Node* node, const vm::bbox3& oldBounds) {
            const vm::bbox3 myOldBounds = bounds();
            invalidateBounds();
            if (bounds() != myOldBounds) {
                nodeBoundsDidChange(myOldBounds);
            }
        }

        bool Entity::doSelectable() const {
            return !hasChildren();
        }

        void Entity::doPick(const vm::ray3& ray, PickResult& pickResult) const {
            if (!hasChildren()) {
                const vm::bbox3& myBounds = bounds();
                if (!myBounds.contains(ray.origin)) {
                    const FloatType distance = intersect(ray, myBounds);
                    if (!vm::isnan(distance)) {
                        const vm::vec3 hitPoint = ray.pointAtDistance(distance);
                        pickResult.addHit(Hit(EntityHit, distance, hitPoint, this));
                    }
                }
            }
        }
        
        void Entity::doFindNodesContaining(const vm::vec3& point, NodeList& result) {
            if (hasChildren()) {
                for (Node* child : Node::children())
                    child->findNodesContaining(point, result);
            } else {
                if (bounds().contains(point))
                    result.push_back(this);
            }
        }

        FloatType Entity::doIntersectWithRay(const vm::ray3& ray) const {
            if (hasChildren()) {
                const vm::bbox3& myBounds = bounds();
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
            } else {
                const auto& myBounds = bounds();
                if (!myBounds.contains(ray.origin)) {
                    return intersect(ray, myBounds);
                } else {
                    return vm::nan<FloatType>();
                }
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

        NodeList Entity::nodesRequiredForViewSelection() {
            if (hasChildren()) {
                // Selecting a brush entity means selecting the children
                return children();
            } else {
                return NodeList{this};
            }
        }
        
        void Entity::doAttributesDidChange(const vm::bbox3& oldBounds) {
            // update m_cachedOrigin and m_cachedRotation. Must be done first because nodeBoundsDidChange() might
            // call origin()
            cacheAttributes();

            nodeBoundsDidChange(oldBounds);

            // needs to be called again because the calculated rotation will be different after calling nodeBoundsDidChange()
            cacheAttributes();
        }
        
        bool Entity::doIsAttributeNameMutable(const AttributeName& name) const {
            return true;
        }
        
        bool Entity::doIsAttributeValueMutable(const AttributeName& name) const {
            return true;
        }

        vm::vec3 Entity::doGetLinkSourceAnchor() const {
            return bounds().center();
        }
        
        vm::vec3 Entity::doGetLinkTargetAnchor() const {
            return bounds().center();
        }

        Node* Entity::doGetContainer() const {
            FindContainerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        Layer* Entity::doGetLayer() const {
            FindLayerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }
        
        Group* Entity::doGetGroup() const {
            FindGroupVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        class TransformEntity : public NodeVisitor {
        private:
            const vm::mat4x4& m_transformation;
            bool m_lockTextures;
            const vm::bbox3& m_worldBounds;
        public:
            TransformEntity(const vm::mat4x4& transformation, const bool lockTextures, const vm::bbox3& worldBounds) :
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

        void Entity::doTransform(const vm::mat4x4& transformation, const bool lockTextures, const vm::bbox3& worldBounds) {
            if (hasChildren()) {
                const NotifyNodeChange nodeChange(this);
                TransformEntity visitor(transformation, lockTextures, worldBounds);
                iterate(visitor);
            } else {
                // node change is called by setOrigin already
                const auto center = bounds().center();
                const auto offset = center - origin();
                const auto transformedCenter = transformation * center;
                setOrigin(transformedCenter - offset);
                
                // applying rotation has side effects (e.g. normalizing "angles")
                // so only do it if there is actually some rotation.
                const auto rotation = vm::stripTranslation(transformation);
                if (rotation != vm::mat4x4::identity) {
                    applyRotation(rotation);
                }
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
            } else if (def != nullptr && def->type() == Assets::EntityDefinition::Type_PointEntity) {
                m_bounds = static_cast<const Assets::PointEntityDefinition*>(def)->bounds();
                m_bounds = m_bounds.translate(origin());
            } else {
                m_bounds = DefaultBounds;
                m_bounds = m_bounds.translate(origin());
            }
            m_boundsValid = true;
        }
    }
}
