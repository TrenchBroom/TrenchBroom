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

#include "Assets/EntityDefinition.h"
#include "Assets/EntityModel.h"
#include "Model/BoundsContainsNodeVisitor.h"
#include "Model/BoundsIntersectsNodeVisitor.h"
#include "Model/BrushNode.h"
#include "Model/ComputeNodeBoundsVisitor.h"
#include "Model/EntityRotationPolicy.h"
#include "Model/EntitySnapshot.h"
#include "Model/FindContainerVisitor.h"
#include "Model/FindGroupVisitor.h"
#include "Model/FindLayerVisitor.h"
#include "Model/IssueGenerator.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"
#include "Model/TagVisitor.h"

#include <kdl/string_utils.h>

#include <vecmath/forward.h>
#include <vecmath/bbox.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/intersection.h>
#include <vecmath/util.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <vector>

namespace TrenchBroom {
    namespace Model {
        const HitType::Type Entity::EntityHit = HitType::freeType();
        const vm::bbox3 Entity::DefaultBounds(8.0);

        Entity::Entity() :
        AttributableNode(),
        Object(),
        m_boundsValid(false),
        m_modelFrame(nullptr) {
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
            return hasEntityDefinition() && definition()->type() == Assets::EntityDefinitionType::BrushEntity;
        }

        bool Entity::hasPointEntityDefinition() const {
            return hasEntityDefinition() && definition()->type() == Assets::EntityDefinitionType::PointEntity;
        }

        bool Entity::hasPointEntityModel() const {
            return hasPointEntityDefinition() && m_modelFrame != nullptr;
        }

        const vm::bbox3& Entity::definitionBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_definitionBounds;
        }

        const vm::vec3& Entity::origin() const {
            return m_cachedOrigin;
        }

        const vm::mat4x4& Entity::rotation() const {
            return m_cachedRotation;
        }

        const vm::mat4x4 Entity::modelTransformation() const {
            return vm::translation_matrix(origin()) * rotation();
        }

        Assets::PitchType Entity::pitchType() const {
            return (m_modelFrame != nullptr ? m_modelFrame->pitchType() : Assets::PitchType::Normal);
        }

        FloatType Entity::area(vm::axis::type axis) const {
            const vm::vec3 size = physicalBounds().size();
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
            m_cachedOrigin = vm::parse<FloatType, 3>(attribute(AttributeNames::Origin, ""), vm::vec3::zero());
            if (vm::is_nan(m_cachedOrigin)) {
                m_cachedOrigin = vm::vec3::zero();
            }
            m_cachedRotation = EntityRotationPolicy::getRotation(this);
        }

        void Entity::setOrigin(const vm::vec3& origin) {
            addOrUpdateAttribute(AttributeNames::Origin, kdl::str_to_string(vm::round(origin)));
        }

        void Entity::applyRotation(const vm::mat4x4& transformation) {
            EntityRotationPolicy::applyRotation(this, transformation);
        }

        Assets::ModelSpecification Entity::modelSpecification() const {
            if (!hasPointEntityDefinition()) {
                return Assets::ModelSpecification();
            } else {
                auto* pointDefinition = static_cast<Assets::PointEntityDefinition*>(m_definition);
                return pointDefinition->model(m_attributes);
            }
        }

        const vm::bbox3& Entity::modelBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_modelBounds;
        }

        const Assets::EntityModelFrame* Entity::modelFrame() const {
            return m_modelFrame;
        }

        void Entity::setModelFrame(const Assets::EntityModelFrame* modelFrame) {
            const auto oldBounds = physicalBounds();
            m_modelFrame = modelFrame;
            nodePhysicalBoundsDidChange(oldBounds);
            cacheAttributes();
        }

        const vm::bbox3& Entity::doGetLogicalBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_logicalBounds;
        }

        const vm::bbox3& Entity::doGetPhysicalBounds() const {
            if (!m_boundsValid) {
                validateBounds();
            }
            return m_physicalBounds;
        }

        Node* Entity::doClone(const vm::bbox3& /* worldBounds */) const {
            auto* entity = new Entity();
            cloneAttributes(entity);
            entity->setDefinition(definition());
            entity->setAttributes(attributes());
            entity->setModelFrame(m_modelFrame);
            return entity;
        }

        NodeSnapshot* Entity::doTakeSnapshot() {
            return new EntitySnapshot(this);
        }

        class CanAddChildToEntity : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            void doVisit(const WorldNode*)  override   { setResult(false); }
            void doVisit(const LayerNode*)  override   { setResult(false); }
            void doVisit(const GroupNode*)  override   { setResult(true); }
            void doVisit(const Entity*) override { setResult(true); }
            void doVisit(const BrushNode*)  override   { setResult(true); }
        };

        bool Entity::doCanAddChild(const Node* child) const {
            CanAddChildToEntity visitor;
            child->accept(visitor);
            return visitor.result();
        }

        bool Entity::doCanRemoveChild(const Node* /* child */) const {
            return true;
        }

        bool Entity::doRemoveIfEmpty() const {
            return true;
        }

        bool Entity::doShouldAddToSpacialIndex() const {
            return true;
        }

        void Entity::doChildWasAdded(Node* /* node */) {
            nodePhysicalBoundsDidChange(physicalBounds());
        }

        void Entity::doChildWasRemoved(Node* /* node */) {
            nodePhysicalBoundsDidChange(physicalBounds());
        }

        void Entity::doNodePhysicalBoundsDidChange() {
            invalidateBounds();
        }

        void Entity::doChildPhysicalBoundsDidChange() {
            const vm::bbox3 myOldBounds = physicalBounds();
            invalidateBounds();
            if (physicalBounds() != myOldBounds) {
                nodePhysicalBoundsDidChange(myOldBounds);
            }
        }

        bool Entity::doSelectable() const {
            return !hasChildren();
        }

        void Entity::doPick(const vm::ray3& ray, PickResult& pickResult) {
            if (!hasChildren()) {
                const vm::bbox3& myBounds = definitionBounds();
                if (!myBounds.contains(ray.origin)) {
                    const FloatType distance = vm::intersect_ray_bbox(ray, myBounds);
                    if (!vm::is_nan(distance)) {
                        const vm::vec3 hitPoint = vm::point_at_distance(ray, distance);
                        pickResult.addHit(Hit(EntityHit, distance, hitPoint, this));
                        return;
                    }
                }

                // only if the bbox hit test failed do we hit test the model
                if (m_modelFrame != nullptr) {
                    // we transform the ray into the model's space
                    const auto transform = modelTransformation();
                    const auto [invertible, inverse] = vm::invert(transform);
                    if (invertible) {
                        const auto transformedRay = vm::ray3f(ray.transform(inverse));
                        const auto distance = m_modelFrame->intersect(transformedRay);
                        if (!vm::is_nan(distance)) {
                            // transform back to world space
                            const auto transformedHitPoint = vm::vec3(point_at_distance(transformedRay, distance));
                            const auto hitPoint = transform * transformedHitPoint;
                            pickResult.addHit(Hit(EntityHit, static_cast<FloatType>(distance), hitPoint, this));
                            return;
                        }
                    }
                }
            }
        }

        void Entity::doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) {
            if (hasChildren()) {
                for (Node* child : Node::children())
                    child->findNodesContaining(point, result);
            } else {
                if (logicalBounds().contains(point))
                    result.push_back(this);
            }
        }

        void Entity::doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) {
            generator->generate(this, issues);
        }

        void Entity::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }

        void Entity::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        std::vector<Node*> Entity::nodesRequiredForViewSelection() {
            if (hasChildren()) {
                // Selecting a brush entity means selecting the children
                return children();
            } else {
                return std::vector<Node*>{this};
            }
        }

        void Entity::doAttributesDidChange(const vm::bbox3& oldBounds) {
            // update m_cachedOrigin and m_cachedRotation. Must be done first because nodePhysicalBoundsDidChange() might
            // call origin()
            cacheAttributes();

            nodePhysicalBoundsDidChange(oldBounds);

            // needs to be called again because the calculated rotation will be different after calling nodePhysicalBoundsDidChange()
            cacheAttributes();
        }

        bool Entity::doIsAttributeNameMutable(const std::string& /* name */) const {
            return true;
        }

        bool Entity::doIsAttributeValueMutable(const std::string& /* name */) const {
            return true;
        }

        vm::vec3 Entity::doGetLinkSourceAnchor() const {
            return logicalBounds().center();
        }

        vm::vec3 Entity::doGetLinkTargetAnchor() const {
            return logicalBounds().center();
        }

        Node* Entity::doGetContainer() const {
            FindContainerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        LayerNode* Entity::doGetLayer() const {
            FindLayerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        GroupNode* Entity::doGetGroup() const {
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
            void doVisit(WorldNode*) override  {}
            void doVisit(LayerNode*) override  {}
            void doVisit(GroupNode*) override  {}
            void doVisit(Entity*) override {}
            void doVisit(BrushNode* brush) override { brush->transform(m_transformation, m_lockTextures, m_worldBounds); }
        };

        void Entity::doTransform(const vm::mat4x4& transformation, const bool lockTextures, const vm::bbox3& worldBounds) {
            if (hasChildren()) {
                const NotifyNodeChange nodeChange(this);
                TransformEntity visitor(transformation, lockTextures, worldBounds);
                iterate(visitor);
            } else {
                // node change is called by setOrigin already
                const auto center = logicalBounds().center();
                const auto offset = center - origin();
                const auto transformedCenter = transformation * center;
                setOrigin(transformedCenter - offset);

                // applying rotation has side effects (e.g. normalizing "angles")
                // so only do it if there is actually some rotation.
                const auto rotation = vm::strip_translation(transformation);
                if (rotation != vm::mat4x4::identity()) {
                    applyRotation(rotation);
                }
            }
        }

        bool Entity::doContains(const Node* node) const {
            BoundsContainsNodeVisitor contains(logicalBounds());
            node->accept(contains);
            assert(contains.hasResult());
            return contains.result();
        }

        bool Entity::doIntersects(const Node* node) const {
            BoundsIntersectsNodeVisitor intersects(logicalBounds());
            node->accept(intersects);
            assert(intersects.hasResult());
            return intersects.result();
        }

        void Entity::invalidateBounds() {
            m_boundsValid = false;
        }

        void Entity::validateBounds() const {
            if (hasPointEntityDefinition()) {
                const Assets::EntityDefinition* def = definition();
                m_definitionBounds = static_cast<const Assets::PointEntityDefinition*>(def)->bounds();
                m_definitionBounds = m_definitionBounds.translate(origin());
            } else {
                m_definitionBounds = DefaultBounds.translate(origin());
            }
            if (hasPointEntityModel()) {
                m_modelBounds = vm::bbox3(m_modelFrame->bounds()).transform(modelTransformation());
            } else {
                m_modelBounds = DefaultBounds.transform(modelTransformation());
            }

            if (hasChildren()) {
                ComputeNodeBoundsVisitor visitor(BoundsType::Logical, vm::bbox3(0.0));
                iterate(visitor);
                m_logicalBounds = visitor.bounds();

                ComputeNodeBoundsVisitor physicalBoundsVisitor(BoundsType::Physical, vm::bbox3(0.0));
                iterate(physicalBoundsVisitor);
                m_physicalBounds = physicalBoundsVisitor.bounds();
            } else {
                m_logicalBounds = m_definitionBounds;
                if (hasPointEntityModel()) {
                    m_physicalBounds = vm::merge(m_definitionBounds, m_modelBounds);
                } else {
                    m_physicalBounds = m_definitionBounds;
                }
            }

            m_boundsValid = true;
        }

        void Entity::doAcceptTagVisitor(TagVisitor& visitor) {
            visitor.visit(*this);
        }

        void Entity::doAcceptTagVisitor(ConstTagVisitor& visitor) const {
            visitor.visit(*this);
        }
    }
}
