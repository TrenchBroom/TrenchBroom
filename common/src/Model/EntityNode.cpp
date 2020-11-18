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

#include "EntityNode.h"

#include "Assets/EntityDefinition.h"
#include "Assets/EntityModel.h"
#include "Model/BrushNode.h"
#include "Model/EntityAttributesVariableStore.h"
#include "Model/EntityRotationPolicy.h"
#include "Model/EntitySnapshot.h"
#include "Model/IssueGenerator.h"
#include "Model/ModelUtils.h"
#include "Model/PickResult.h"
#include "Model/TagVisitor.h"

#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/string_utils.h>

#include <vecmath/forward.h>
#include <vecmath/bbox.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/intersection.h>
#include <vecmath/util.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <optional>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        const HitType::Type EntityNode::EntityHitType = HitType::freeType();
        const vm::bbox3 EntityNode::DefaultBounds(8.0);

        EntityNode::EntityNode() :
        AttributableNode(),
        Object() {}

        EntityNode::EntityNode(Entity entity) :
        AttributableNode(std::move(entity)),
        Object() {}

        bool EntityNode::hasEntityDefinition() const {
            return m_entity.definition() != nullptr;
        }

        bool EntityNode::hasBrushEntityDefinition() const {
            return hasEntityDefinition() && m_entity.definition()->type() == Assets::EntityDefinitionType::BrushEntity;
        }

        bool EntityNode::hasPointEntityDefinition() const {
            return hasEntityDefinition() && m_entity.definition()->type() == Assets::EntityDefinitionType::PointEntity;
        }

        bool EntityNode::hasPointEntityModel() const {
            return hasPointEntityDefinition() && modelFrame() != nullptr;
        }

        vm::bbox3 EntityNode::definitionBounds() const {
            if (hasPointEntityDefinition()) {
                const Assets::EntityDefinition* def = m_entity.definition();
                const auto definitionBounds = static_cast<const Assets::PointEntityDefinition*>(def)->bounds();
                return definitionBounds.translate(origin());
            } else {
                return DefaultBounds.translate(origin());
            }
        }

        const vm::vec3& EntityNode::origin() const {
            return m_entity.origin();
        }

        const vm::mat4x4& EntityNode::rotation() const {
            return m_entity.rotation();
        }

        const vm::mat4x4 EntityNode::modelTransformation() const {
            return vm::translation_matrix(origin()) * rotation();
        }

        Assets::PitchType EntityNode::pitchType() const {
            return (modelFrame() != nullptr ? modelFrame()->pitchType() : Assets::PitchType::Normal);
        }

        FloatType EntityNode::area(vm::axis::type axis) const {
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

        void EntityNode::setOrigin(const vm::vec3& origin) {
            auto entity = m_entity;
            entity.addOrUpdateAttribute(AttributeNames::Origin, kdl::str_to_string(vm::correct(origin)));
            setEntity(std::move(entity));
        }

        void EntityNode::applyRotation(const vm::mat4x4& transformation) {
            EntityRotationPolicy::applyRotation(this, transformation);
        }

        Assets::ModelSpecification EntityNode::modelSpecification() const {
            if (!hasPointEntityDefinition()) {
                return Assets::ModelSpecification();
            } else {
                const auto* pointDefinition = static_cast<const Assets::PointEntityDefinition*>(m_entity.definition());
                const auto variableStore = EntityAttributesVariableStore(m_entity);
                return pointDefinition->model(variableStore);
            }
        }

        const vm::bbox3& EntityNode::modelBounds() const {
            validateBounds();
            return m_cachedBounds->modelBounds;
        }

        const Assets::EntityModelFrame* EntityNode::modelFrame() const {
            return m_entity.model();
        }

        void EntityNode::setModelFrame(const Assets::EntityModelFrame* modelFrame) {
            const auto oldBounds = physicalBounds();
            m_entity.setModel(modelFrame);
            nodePhysicalBoundsDidChange(oldBounds);
        }

        const vm::bbox3& EntityNode::doGetLogicalBounds() const {
            validateBounds();
            return m_cachedBounds->logicalBounds;
        }

        const vm::bbox3& EntityNode::doGetPhysicalBounds() const {
            validateBounds();
            return m_cachedBounds->physicalBounds;
        }

        Node* EntityNode::doClone(const vm::bbox3& /* worldBounds */) const {
            auto* entity = new EntityNode(m_entity);
            cloneAttributes(entity);
            return entity;
        }

        NodeSnapshot* EntityNode::doTakeSnapshot() {
            return new EntitySnapshot(this);
        }

        bool EntityNode::doCanAddChild(const Node* child) const {
            return child->accept(kdl::overload(
                [](const WorldNode*)  { return false; },
                [](const LayerNode*)  { return false; },
                [](const GroupNode*)  { return false; },
                [](const EntityNode*) { return false; },
                [](const BrushNode*)  { return true;  }
            ));
        }

        bool EntityNode::doCanRemoveChild(const Node* /* child */) const {
            return true;
        }

        bool EntityNode::doRemoveIfEmpty() const {
            return true;
        }

        bool EntityNode::doShouldAddToSpacialIndex() const {
            return true;
        }

        void EntityNode::doChildWasAdded(Node* /* node */) {
            m_entity.setPointEntity(!hasChildren());
            nodePhysicalBoundsDidChange(physicalBounds());
        }

        void EntityNode::doChildWasRemoved(Node* /* node */) {
            m_entity.setPointEntity(hasChildren());
            nodePhysicalBoundsDidChange(physicalBounds());
        }

        void EntityNode::doNodePhysicalBoundsDidChange() {
            invalidateBounds();
        }

        void EntityNode::doChildPhysicalBoundsDidChange() {
            const vm::bbox3 myOldBounds = physicalBounds();
            invalidateBounds();
            if (physicalBounds() != myOldBounds) {
                nodePhysicalBoundsDidChange(myOldBounds);
            }
        }

        bool EntityNode::doSelectable() const {
            return !hasChildren();
        }

        void EntityNode::doPick(const vm::ray3& ray, PickResult& pickResult) {
            if (!hasChildren()) {
                const vm::bbox3& myBounds = definitionBounds();
                if (!myBounds.contains(ray.origin)) {
                    const FloatType distance = vm::intersect_ray_bbox(ray, myBounds);
                    if (!vm::is_nan(distance)) {
                        const vm::vec3 hitPoint = vm::point_at_distance(ray, distance);
                        pickResult.addHit(Hit(EntityHitType, distance, hitPoint, this));
                        return;
                    }
                }

                // only if the bbox hit test failed do we hit test the model
                if (modelFrame() != nullptr) {
                    // we transform the ray into the model's space
                    const auto transform = modelTransformation();
                    const auto [invertible, inverse] = vm::invert(transform);
                    if (invertible) {
                        const auto transformedRay = vm::ray3f(ray.transform(inverse));
                        const auto distance = modelFrame()->intersect(transformedRay);
                        if (!vm::is_nan(distance)) {
                            // transform back to world space
                            const auto transformedHitPoint = vm::vec3(point_at_distance(transformedRay, distance));
                            const auto hitPoint = transform * transformedHitPoint;
                            pickResult.addHit(Hit(EntityHitType, static_cast<FloatType>(distance), hitPoint, this));
                            return;
                        }
                    }
                }
            }
        }

        void EntityNode::doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) {
            if (hasChildren()) {
                for (Node* child : Node::children())
                    child->findNodesContaining(point, result);
            } else {
                if (logicalBounds().contains(point))
                    result.push_back(this);
            }
        }

        void EntityNode::doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) {
            generator->generate(this, issues);
        }

        void EntityNode::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }

        void EntityNode::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        std::vector<Node*> EntityNode::nodesRequiredForViewSelection() {
            if (hasChildren()) {
                // Selecting a brush entity means selecting the children
                return children();
            } else {
                return std::vector<Node*>{this};
            }
        }

        void EntityNode::doAttributesDidChange(const vm::bbox3& oldBounds) {
            nodePhysicalBoundsDidChange(oldBounds);
        }

        vm::vec3 EntityNode::doGetLinkSourceAnchor() const {
            return logicalBounds().center();
        }

        vm::vec3 EntityNode::doGetLinkTargetAnchor() const {
            return logicalBounds().center();
        }

        Node* EntityNode::doGetContainer() {
            return parent();
        }

        LayerNode* EntityNode::doGetLayer() {
            return findContainingLayer(this);
        }

        GroupNode* EntityNode::doGetGroup() {
            return findContainingGroup(this);
        }

        kdl::result<void, TransformError> EntityNode::doTransform(const vm::bbox3& worldBounds, const vm::mat4x4& transformation, bool lockTextures) {
            if (hasChildren()) {
                const NotifyNodeChange nodeChange(this);

                for (auto* child : children()) {
                    const auto result = child->accept(kdl::overload(
                        [] (WorldNode*)  { return kdl::result<void, TransformError>::success(); },
                        [] (LayerNode*)  { return kdl::result<void, TransformError>::success(); },
                        [] (GroupNode*)  { return kdl::result<void, TransformError>::success(); },
                        [] (EntityNode*) { return kdl::result<void, TransformError>::success(); },
                        [&](BrushNode* brush) {
                            return brush->transform(worldBounds, transformation, lockTextures);
                        }
                    ));
                    if (!result.is_success()) {
                        return result;
                    }
                }
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

            return kdl::result<void, TransformError>::success();
        }

        bool EntityNode::doContains(const Node* node) const {
            return boundsContainNode(logicalBounds(), node);
        }

        bool EntityNode::doIntersects(const Node* node) const {
            return boundsIntersectNode(logicalBounds(), node);
        }

        void EntityNode::invalidateBounds() {
            m_cachedBounds = std::nullopt;
        }

        void EntityNode::validateBounds() const {
            if (m_cachedBounds.has_value()) {
                return;
            }

            m_cachedBounds = CachedBounds{};

            if (hasPointEntityModel()) {
                m_cachedBounds->modelBounds = vm::bbox3(modelFrame()->bounds()).transform(modelTransformation());
            } else {
                m_cachedBounds->modelBounds = DefaultBounds.transform(modelTransformation());
            }

            if (hasChildren()) {
                m_cachedBounds->logicalBounds = computeLogicalBounds(children(), vm::bbox3(0.0));
                m_cachedBounds->physicalBounds = computePhysicalBounds(children(), vm::bbox3(0.0));
            } else {
                m_cachedBounds->logicalBounds = definitionBounds();
                if (hasPointEntityModel()) {
                    m_cachedBounds->physicalBounds = vm::merge(definitionBounds(), m_cachedBounds->modelBounds);
                } else {
                    m_cachedBounds->physicalBounds = definitionBounds();
                }
            }
        }

        void EntityNode::doAcceptTagVisitor(TagVisitor& visitor) {
            visitor.visit(*this);
        }

        void EntityNode::doAcceptTagVisitor(ConstTagVisitor& visitor) const {
            visitor.visit(*this);
        }
    }
}
