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

#include "BrushNode.h"

#include "Exceptions.h"
#include "FloatType.h"
#include "Polyhedron.h"
#include "Polyhedron_Matcher.h"
#include "Model/Brush.h"
#include "Model/BrushError.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushGeometry.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/IssueGenerator.h"
#include "Model/LayerNode.h"
#include "Model/ModelUtils.h"
#include "Model/PickResult.h"
#include "Model/TagVisitor.h"
#include "Model/TexCoordSystem.h"
#include "Model/WorldNode.h"
#include "Renderer/BrushRendererBrushCache.h"

#include <kdl/overload.h>
#include <kdl/result.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/intersection.h>
#include <vecmath/vec.h>
#include <vecmath/vec_ext.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/segment.h>
#include <vecmath/polygon.h>
#include <vecmath/util.h>

#include <algorithm> // for std::remove
#include <iterator>
#include <set>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        const HitType::Type BrushNode::BrushHitType = HitType::freeType();

        BrushNode::BrushNode(Brush brush) :
        m_brushRendererBrushCache(std::make_unique<Renderer::BrushRendererBrushCache>()),
        m_brush(std::move(brush)) {
            updateSelectedFaceCount();
        }

        BrushNode::~BrushNode() = default;

        BrushNode* BrushNode::clone(const vm::bbox3& worldBounds) const {
            return static_cast<BrushNode*>(Node::clone(worldBounds));
        }

        const AttributableNode* BrushNode::entity() const {
            return visitParent(kdl::overload(
                [](const WorldNode* world)                    -> const AttributableNode* { return world; },
                [](const EntityNode* entity)                  -> const AttributableNode* { return entity; },
                [](auto&& thisLambda, const LayerNode* layer) -> const AttributableNode* { return layer->visitParent(thisLambda).value_or(nullptr); },
                [](auto&& thisLambda, const GroupNode* group) -> const AttributableNode* { return group->visitParent(thisLambda).value_or(nullptr); },
                [](auto&& thisLambda, const BrushNode* brush) -> const AttributableNode* { return brush->visitParent(thisLambda).value_or(nullptr); }
            )).value_or(nullptr);
        }

        AttributableNode* BrushNode::entity() {
            return const_cast<AttributableNode*>(const_cast<const BrushNode*>(this)->entity());
        }

        const Brush& BrushNode::brush() const {
            return m_brush;
        }
        
        Brush BrushNode::setBrush(Brush brush) {
            const NotifyNodeChange nodeChange(this);
            const NotifyPhysicalBoundsChange boundsChange(this);

            using std::swap;
            swap(m_brush, brush);
            
            updateSelectedFaceCount();
            invalidateIssues();
            invalidateVertexCache();

            return brush;
        }

        bool BrushNode::hasSelectedFaces() const {
            return m_selectedFaceCount > 0u;
        }

        void BrushNode::selectFace(const size_t faceIndex) {
            m_brush.face(faceIndex).select();
            ++m_selectedFaceCount;
        }
        
        void BrushNode::deselectFace(const size_t faceIndex) {
            m_brush.face(faceIndex).deselect();
            --m_selectedFaceCount;
        }

        void BrushNode::updateFaceTags(const size_t faceIndex, TagManager& tagManager) {
            m_brush.face(faceIndex).updateTags(tagManager);
        }

        void BrushNode::setFaceTexture(const size_t faceIndex, Assets::Texture* texture) {
            m_brush.face(faceIndex).setTexture(texture);
            
            invalidateIssues();
            invalidateVertexCache();
        }

        void BrushNode::updateSelectedFaceCount() {
            m_selectedFaceCount = 0u;
            for (const BrushFace& face : m_brush.faces()) {
                if (face.selected()) {
                    ++m_selectedFaceCount;
                }
            }
        }

        const std::string& BrushNode::doGetName() const {
            static const std::string name("brush");
            return name;
        }

        const vm::bbox3& BrushNode::doGetLogicalBounds() const {
            return m_brush.bounds();
        }

        const vm::bbox3& BrushNode::doGetPhysicalBounds() const {
            return logicalBounds();
        }

        Node* BrushNode::doClone(const vm::bbox3& /* worldBounds */) const {
            auto* result = new BrushNode(m_brush);
            cloneAttributes(result);
            return result;
        }

        bool BrushNode::doCanAddChild(const Node* /* child */) const {
            return false;
        }

        bool BrushNode::doCanRemoveChild(const Node* /* child */) const {
            return false;
        }

        bool BrushNode::doRemoveIfEmpty() const {
            return false;
        }

        bool BrushNode::doShouldAddToSpacialIndex() const {
            return true;
        }

        bool BrushNode::doSelectable() const {
            return true;
        }

        void BrushNode::doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) {
            generator->generate(this, issues);
        }

        void BrushNode::doAccept(NodeVisitor& visitor) {
            visitor.visit(this);
        }

        void BrushNode::doAccept(ConstNodeVisitor& visitor) const {
            visitor.visit(this);
        }

        void BrushNode::doPick(const vm::ray3& ray, PickResult& pickResult) {
            if (const auto hit = findFaceHit(ray)) {
                const auto [distance, faceIndex] = *hit;
                ensure(!vm::is_nan(distance), "nan hit distance");
                const auto hitPoint = vm::point_at_distance(ray, distance);
                pickResult.addHit(Hit(BrushHitType, distance, hitPoint, BrushFaceHandle(this, faceIndex)));
            }
        }

        void BrushNode::doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) {
            if (m_brush.containsPoint(point)) {
                result.push_back(this);
            }
        }

        std::optional<std::tuple<FloatType, size_t>> BrushNode::findFaceHit(const vm::ray3& ray) const {
            if (!vm::is_nan(vm::intersect_ray_bbox(ray, logicalBounds()))) {
                for (size_t i = 0u; i < m_brush.faceCount(); ++i) {
                    const auto& face = m_brush.face(i);
                    const auto distance = face.intersectWithRay(ray);
                    if (!vm::is_nan(distance)) {
                        return std::make_tuple(distance, i);
                    }
                }
            }
            return std::nullopt;
        }

        Node* BrushNode::doGetContainer() {
            return parent();
        }

        LayerNode* BrushNode::doGetLayer() {
            return findContainingLayer(this);
        }

        GroupNode* BrushNode::doGetGroup() {
            return findContainingGroup(this);
        }

        kdl::result<void, TransformError> BrushNode::doTransform(const vm::bbox3& worldBounds, const vm::mat4x4& transformation, bool lockTextures) {
            const NotifyNodeChange nodeChange(this);
            const NotifyPhysicalBoundsChange boundsChange(this);

            return m_brush.transform(worldBounds, transformation, lockTextures)
                .visit(kdl::overload(
                    [&](Brush&& brush) {
                        m_brush = std::move(brush);
                        invalidateIssues();
                        invalidateVertexCache();

                        return kdl::result<void, TransformError>::success();
                    },
                    [](const BrushError e) {
                        return kdl::result<void, TransformError>::error(TransformError{kdl::str_to_string(e)});
                    }
                ));
        }

        bool BrushNode::doContains(const Node* node) const {
            return node->accept(kdl::overload(
                [](const WorldNode*)          { return false; },
                [](const LayerNode*)          { return false; },
                [&](const GroupNode* group)   { return m_brush.contains(group->logicalBounds()); },
                [&](const EntityNode* entity) { return m_brush.contains(entity->logicalBounds()); },
                [&](const BrushNode* brush)   { return m_brush.contains(brush->brush()); }
            ));
        }

        bool BrushNode::doIntersects(const Node* node) const {
            return node->accept(kdl::overload(
                [](const WorldNode*)          { return false; },
                [](const LayerNode*)          { return false; },
                [&](const GroupNode* group)   { return m_brush.intersects(group->logicalBounds()); },
                [&](const EntityNode* entity) { return m_brush.intersects(entity->logicalBounds()); },
                [&](const BrushNode* brush)   { return m_brush.intersects(brush->brush()); }
            ));
        }

        void BrushNode::invalidateVertexCache() {
            m_brushRendererBrushCache->invalidateVertexCache();
        }

        Renderer::BrushRendererBrushCache& BrushNode::brushRendererBrushCache() const {
            return *m_brushRendererBrushCache;
        }

        void BrushNode::initializeTags(TagManager& tagManager) {
            Taggable::initializeTags(tagManager);
            for (auto& face : m_brush.faces()) {
                face.initializeTags(tagManager);
            }
        }

        void BrushNode::clearTags() {
            for (auto& face : m_brush.faces()) {
                face.clearTags();
            }
            Taggable::clearTags();
        }

        void BrushNode::updateTags(TagManager& tagManager) {
            for (auto& face : m_brush.faces()) {
                face.updateTags(tagManager);
            }
            Taggable::updateTags(tagManager);
        }

        bool BrushNode::allFacesHaveAnyTagInMask(TagType::Type tagMask) const {
            // Possible optimization: Store the shared face tag mask in the brush and updated it when a face changes.

            TagType::Type sharedFaceTags = TagType::AnyType; // set all bits to 1
            for (const auto& face : m_brush.faces()) {
                sharedFaceTags &= face.tagMask();
            }
            return (sharedFaceTags & tagMask) != 0;
        }

        bool BrushNode::anyFaceHasAnyTag() const {
            for (const auto& face : m_brush.faces()) {
                if (face.hasAnyTag()) {
                    return true;
                }
            }
            return false;
        }

        bool BrushNode::anyFacesHaveAnyTagInMask(TagType::Type tagMask) const {
            // Possible optimization: Store the shared face tag mask in the brush and updated it when a face changes.

            for (const auto& face : m_brush.faces()) {
                if (face.hasTag(tagMask)) {
                    return true;
                }
            }
            return false;
        }

        void BrushNode::doAcceptTagVisitor(TagVisitor& visitor) {
            visitor.visit(*this);
        }

        void BrushNode::doAcceptTagVisitor(ConstTagVisitor& visitor) const {
            visitor.visit(*this);
        }

        bool operator==(const BrushNode& lhs, const BrushNode& rhs) {
            return lhs.brush() == rhs.brush();
        }

        bool operator!=(const BrushNode& lhs, const BrushNode& rhs) {
            return !(lhs == rhs);
        }
    }
}
