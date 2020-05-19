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
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushFaceSnapshot.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushSnapshot.h"
#include "Model/EntityNode.h"
#include "Model/FindContainerVisitor.h"
#include "Model/FindGroupVisitor.h"
#include "Model/FindLayerVisitor.h"
#include "Model/GroupNode.h"
#include "Model/IssueGenerator.h"
#include "Model/NodeVisitor.h"
#include "Model/PickResult.h"
#include "Model/TagVisitor.h"
#include "Model/TexCoordSystem.h"
#include "Model/WorldNode.h"
#include "Renderer/BrushRendererBrushCache.h"

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

        BrushNode::BrushNode(const vm::bbox3& worldBounds, const std::vector<BrushFace*>& faces) :
        m_brushRendererBrushCache(std::make_unique<Renderer::BrushRendererBrushCache>()),
        m_brush(this, worldBounds, faces) {}

        BrushNode::BrushNode(Brush brush) :
        m_brushRendererBrushCache(std::make_unique<Renderer::BrushRendererBrushCache>()),
        m_brush(std::move(brush)) {
            m_brush.setNode(this);
        }

        BrushNode::~BrushNode() = default;

        BrushNode* BrushNode::clone(const vm::bbox3& worldBounds) const {
            return static_cast<BrushNode*>(Node::clone(worldBounds));
        }

        NodeSnapshot* BrushNode::doTakeSnapshot() {
            return new BrushSnapshot(this);
        }

        class FindBrushOwner : public NodeVisitor, public NodeQuery<AttributableNode*> {
        private:
            void doVisit(WorldNode* world) override       { setResult(world); cancel(); }
            void doVisit(LayerNode* /* layer */) override {}
            void doVisit(GroupNode* /* group */) override {}
            void doVisit(EntityNode* entity) override     { setResult(entity); cancel(); }
            void doVisit(BrushNode* /* brush */) override {}
        };

        AttributableNode* BrushNode::entity() const {
            if (parent() == nullptr) {
                return nullptr;
            }

            FindBrushOwner visitor;
            parent()->acceptAndEscalate(visitor);
            if (!visitor.hasResult()) {
                return nullptr;
            } else {
                return visitor.result();
            }
        }

        const Brush& BrushNode::brush() const {
            return m_brush;
        }
        
        void BrushNode::setBrush(Brush brush) {
            const NotifyNodeChange nodeChange(this);
            const NotifyPhysicalBoundsChange boundsChange(this);
            m_brush = std::move(brush);
            m_brush.setNode(this);
            
            invalidateIssues();
            invalidateVertexCache();
        }

        BrushFaceSnapshot* BrushNode::takeSnapshot(BrushFace* face) {
            ensure(face != nullptr, "face must not be null");
            return new BrushFaceSnapshot(this, face);
        }

        bool BrushNode::canTransform(const vm::mat4x4& transformation, const vm::bbox3& worldBounds) const {
            return m_brush.canTransform(transformation, worldBounds);
        }

        void BrushNode::findIntegerPlanePoints(const vm::bbox3& worldBounds) {
            const NotifyNodeChange nodeChange(this);
            const NotifyPhysicalBoundsChange boundsChange(this);
            m_brush.findIntegerPlanePoints(worldBounds);
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
            return new BrushNode(m_brush);
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
            const auto hit = findFaceHit(ray);
            if (hit.face != nullptr) {
                ensure(!vm::is_nan(hit.distance), "nan hit distance");
                const auto hitPoint = vm::point_at_distance(ray, hit.distance);
                pickResult.addHit(Hit(BrushHitType, hit.distance, hitPoint, BrushFaceHandle(this, hit.face)));
            }
        }

        void BrushNode::doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) {
            if (m_brush.containsPoint(point)) {
                result.push_back(this);
            }
        }

        BrushNode::BrushFaceHit::BrushFaceHit() : face(nullptr), distance(vm::nan<FloatType>()) {}

        BrushNode::BrushFaceHit::BrushFaceHit(BrushFace* i_face, const FloatType i_distance) : face(i_face), distance(i_distance) {}

        BrushNode::BrushFaceHit BrushNode::findFaceHit(const vm::ray3& ray) const {
            if (vm::is_nan(vm::intersect_ray_bbox(ray, logicalBounds()))) {
                return BrushFaceHit();
            }

            for (auto* face : m_brush.faces()) {
                const auto distance = face->intersectWithRay(ray);
                if (!vm::is_nan(distance)) {
                    return BrushFaceHit(face, distance);
                }
            }
            return BrushFaceHit();
        }

        Node* BrushNode::doGetContainer() const {
            FindContainerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        LayerNode* BrushNode::doGetLayer() const {
            FindLayerVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        GroupNode* BrushNode::doGetGroup() const {
            FindGroupVisitor visitor;
            escalate(visitor);
            return visitor.hasResult() ? visitor.result() : nullptr;
        }

        void BrushNode::doTransform(const vm::mat4x4& transformation, const bool lockTextures, const vm::bbox3& worldBounds) {
            const NotifyNodeChange nodeChange(this);
            const NotifyPhysicalBoundsChange boundsChange(this);
            m_brush.transform(transformation, lockTextures, worldBounds);
        }

        class BrushNode::Contains : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            const Brush& m_brush;
        public:
            Contains(const Brush& brush) :
            m_brush(brush) {}
        private:
            void doVisit(const WorldNode* /* world */) override { setResult(false); }
            void doVisit(const LayerNode* /* layer */) override { setResult(false); }
            void doVisit(const GroupNode* group) override       { setResult(contains(group->logicalBounds())); }
            void doVisit(const EntityNode* entity) override     { setResult(contains(entity->logicalBounds())); }
            void doVisit(const BrushNode* brush) override       { setResult(contains(brush)); }

            bool contains(const vm::bbox3& bounds) const {
                return m_brush.contains(bounds);
            }

            bool contains(const BrushNode* brush) const {
                return m_brush.contains(brush->m_brush);
            }
        };

        bool BrushNode::doContains(const Node* node) const {
            Contains contains(m_brush);
            node->accept(contains);
            assert(contains.hasResult());
            return contains.result();
        }

        class BrushNode::Intersects : public ConstNodeVisitor, public NodeQuery<bool> {
        private:
            const Brush& m_brush;
        public:
            Intersects(const Brush& brush) :
            m_brush(brush) {}
        private:
            void doVisit(const WorldNode* /* world */) override { setResult(false); }
            void doVisit(const LayerNode* /* layer */) override { setResult(false); }
            void doVisit(const GroupNode* group) override       { setResult(intersects(group->logicalBounds())); }
            void doVisit(const EntityNode* entity) override     { setResult(intersects(entity->logicalBounds())); }
            void doVisit(const BrushNode* brush) override       { setResult(intersects(brush)); }

            bool intersects(const vm::bbox3& bounds) const {
                return m_brush.intersects(bounds);
            }

            bool intersects(const BrushNode* brush) {
                return m_brush.intersects(brush->m_brush);
            }
        };

        bool BrushNode::doIntersects(const Node* node) const {
            Intersects intersects(m_brush);
            node->accept(intersects);
            assert(intersects.hasResult());
            return intersects.result();
        }

        void BrushNode::invalidateVertexCache() {
            m_brushRendererBrushCache->invalidateVertexCache();
        }

        Renderer::BrushRendererBrushCache& BrushNode::brushRendererBrushCache() const {
            return *m_brushRendererBrushCache;
        }

        void BrushNode::initializeTags(TagManager& tagManager) {
            Taggable::initializeTags(tagManager);
            for (auto* face : m_brush.faces()) {
                face->initializeTags(tagManager);
            }
        }

        void BrushNode::clearTags() {
            for (auto* face : m_brush.faces()) {
                face->clearTags();
            }
            Taggable::clearTags();
        }

        void BrushNode::updateTags(TagManager& tagManager) {
            for (auto& face : m_brush.faces()) {
                face->updateTags(tagManager);
            }
            Taggable::updateTags(tagManager);
        }

        bool BrushNode::allFacesHaveAnyTagInMask(TagType::Type tagMask) const {
            // Possible optimization: Store the shared face tag mask in the brush and updated it when a face changes.

            TagType::Type sharedFaceTags = TagType::AnyType; // set all bits to 1
            for (const auto* face : m_brush.faces()) {
                sharedFaceTags &= face->tagMask();
            }
            return (sharedFaceTags & tagMask) != 0;
        }

        bool BrushNode::anyFaceHasAnyTag() const {
            for (const auto* face : m_brush.faces()) {
                if (face->hasAnyTag()) {
                    return true;
                }
            }
            return false;
        }

        bool BrushNode::anyFacesHaveAnyTagInMask(TagType::Type tagMask) const {
            // Possible optimization: Store the shared face tag mask in the brush and updated it when a face changes.

            for (const auto* face : m_brush.faces()) {
                if (face->hasTag(tagMask)) {
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
    }
}
