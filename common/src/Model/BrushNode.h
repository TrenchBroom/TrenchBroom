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

#ifndef TrenchBroom_BrushNode
#define TrenchBroom_BrushNode

#include "FloatType.h"
#include "Macros.h"
#include "Model/BrushGeometry.h"
#include "Model/HitType.h"
#include "Model/Node.h"
#include "Model/Object.h"
#include "Model/TagType.h"

#include <vecmath/forward.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class BrushRendererBrushCache;
    }

    namespace Model {
        class Brush;
        class BrushFace;
        class BrushFaceSnapshot;
        class GroupNode;
        class LayerNode;

        class ModelFactory;

        class BrushNode : public Node, public Object {
        public:
            static const HitType::Type BrushHitType;
        public:
            using VertexList = BrushVertexList;
            using EdgeList = BrushEdgeList;
        private:
            mutable std::unique_ptr<Renderer::BrushRendererBrushCache> m_brushRendererBrushCache; // unique_ptr for breaking header dependencies
            std::unique_ptr<Brush> m_brush; // must be destroyed before the brush renderer cache
        public:
            BrushNode(const vm::bbox3& worldBounds, const std::vector<BrushFace*>& faces);
            explicit BrushNode(std::unique_ptr<Brush> brush);
            ~BrushNode() override;
        public:
            BrushNode* clone(const vm::bbox3& worldBounds) const;

            AttributableNode* entity() const;
        public: // face management:
            BrushFace* findFace(const std::string& textureName) const;
            BrushFace* findFace(const vm::vec3& normal) const;
            BrushFace* findFace(const vm::plane3& boundary) const;
            BrushFace* findFace(const vm::polygon3& vertices, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            BrushFace* findFace(const std::vector<vm::polygon3>& candidates, FloatType epsilon = static_cast<FloatType>(0.0)) const;

            size_t faceCount() const;
            const std::vector<BrushFace*>& faces() const;
            void setFaces(const vm::bbox3& worldBounds, const std::vector<BrushFace*>& faces);

            using Node::takeSnapshot;
            BrushFaceSnapshot* takeSnapshot(BrushFace* face);

            bool closed() const;
            bool fullySpecified() const;
        public: // clone face attributes from matching faces of other brushes
            void cloneFaceAttributesFrom(const std::vector<BrushNode*>& brushes);
            void cloneFaceAttributesFrom(const BrushNode* brush);
            void cloneInvertedFaceAttributesFrom(const std::vector<BrushNode*>& brushes);
            void cloneInvertedFaceAttributesFrom(const BrushNode* brush);
        public: // clipping
            bool clip(const vm::bbox3& worldBounds, BrushFace* face);
        public: // move face along normal
            bool canMoveBoundary(const vm::bbox3& worldBounds, const BrushFace* face, const vm::vec3& delta) const;
            void moveBoundary(const vm::bbox3& worldBounds, BrushFace* face, const vm::vec3& delta, bool lockTexture);
            bool canExpand(const vm::bbox3& worldBounds, FloatType delta, bool lockTexture) const;
            /**
             * Moves all faces by `delta` units along their normals; negative values shrink the brush.
             * Returns true if the brush is valid after the modification, false if the brush is invalid.
             */
            bool expand(const vm::bbox3& worldBounds, FloatType delta, bool lockTexture);
        public:
            // geometry access
            size_t vertexCount() const;
            const VertexList& vertices() const;
            std::vector<vm::vec3> vertexPositions() const;
            vm::vec3 findClosestVertexPosition(const vm::vec3& position) const;

            bool hasVertex(const vm::vec3& position, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasEdge(const vm::segment3& edge, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasFace(const vm::polygon3& face, FloatType epsilon = static_cast<FloatType>(0.0)) const;

            bool hasFace(const vm::vec3& p1, const vm::vec3& p2, const vm::vec3& p3, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasFace(const vm::vec3& p1, const vm::vec3& p2, const vm::vec3& p3, const vm::vec3& p4, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasFace(const vm::vec3& p1, const vm::vec3& p2, const vm::vec3& p3, const vm::vec3& p4, const vm::vec3& p5, FloatType epsilon = static_cast<FloatType>(0.0)) const;

            size_t edgeCount() const;
            const EdgeList& edges() const;
            bool containsPoint(const vm::vec3& point) const;

            std::vector<BrushFace*> incidentFaces(const BrushVertex* vertex) const;

            // vertex operations
            bool canMoveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertices, const vm::vec3& delta) const;
            std::vector<vm::vec3> moveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions, const vm::vec3& delta, bool uvLock = false);

            bool canAddVertex(const vm::bbox3& worldBounds, const vm::vec3& position) const;
            BrushVertex* addVertex(const vm::bbox3& worldBounds, const vm::vec3& position);

            bool canRemoveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions) const;
            void removeVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions);

            bool canSnapVertices(const vm::bbox3& worldBounds, FloatType snapTo);
            void snapVertices(const vm::bbox3& worldBounds, FloatType snapTo, bool uvLock = false);

            // edge operations
            bool canMoveEdges(const vm::bbox3& worldBounds, const std::vector<vm::segment3>& edgePositions, const vm::vec3& delta) const;
            std::vector<vm::segment3> moveEdges(const vm::bbox3& worldBounds, const std::vector<vm::segment3>& edgePositions, const vm::vec3& delta, bool uvLock = false);

            // face operations
            bool canMoveFaces(const vm::bbox3& worldBounds, const std::vector<vm::polygon3>& facePositions, const vm::vec3& delta) const;
            std::vector<vm::polygon3> moveFaces(const vm::bbox3& worldBounds, const std::vector<vm::polygon3>& facePositions, const vm::vec3& delta, bool uvLock = false);
        public:
            // CSG operations
            /**
             * Subtracts the given subtrahends from `this`, returning the result but without modifying `this`.
             *
             * @param subtrahends brushes to subtract from `this`. The passed-in brushes are not modified.
             * @return the subtraction result
             */
            std::vector<BrushNode*> subtract(const ModelFactory& factory, const vm::bbox3& worldBounds, const std::string& defaultTextureName, const std::vector<BrushNode*>& subtrahends) const;
            std::vector<BrushNode*> subtract(const ModelFactory& factory, const vm::bbox3& worldBounds, const std::string& defaultTextureName, BrushNode* subtrahend) const;
            void intersect(const vm::bbox3& worldBounds, const BrushNode* brush);

            // transformation
            bool canTransform(const vm::mat4x4& transformation, const vm::bbox3& worldBounds) const;
        public:
            void findIntegerPlanePoints(const vm::bbox3& worldBounds);
        private: // implement Node interface
            const std::string& doGetName() const override;
            const vm::bbox3& doGetLogicalBounds() const override;
            const vm::bbox3& doGetPhysicalBounds() const override;

            Node* doClone(const vm::bbox3& worldBounds) const override;
            NodeSnapshot* doTakeSnapshot() override;

            bool doCanAddChild(const Node* child) const override;
            bool doCanRemoveChild(const Node* child) const override;
            bool doRemoveIfEmpty() const override;

            bool doShouldAddToSpacialIndex() const override;

            bool doSelectable() const override;

            void doGenerateIssues(const IssueGenerator* generator, std::vector<Issue*>& issues) override;
            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;
        private: // implement Object interface
            void doPick(const vm::ray3& ray, PickResult& pickResult) override;
            void doFindNodesContaining(const vm::vec3& point, std::vector<Node*>& result) override;

            struct BrushFaceHit {
                BrushFace* face;
                FloatType distance;
                BrushFaceHit();
                BrushFaceHit(BrushFace* i_face, FloatType i_distance);
            };

            BrushFaceHit findFaceHit(const vm::ray3& ray) const;

            Node* doGetContainer() const override;
            LayerNode* doGetLayer() const override;
            GroupNode* doGetGroup() const override;

            void doTransform(const vm::mat4x4& transformation, bool lockTextures, const vm::bbox3& worldBounds) override;

            class Contains;
            bool doContains(const Node* node) const override;

            class Intersects;
            bool doIntersects(const Node* node) const override;
        public: // renderer cache
            /**
             * Only exposed to be called by BrushFace
             */
            void invalidateVertexCache();
            Renderer::BrushRendererBrushCache& brushRendererBrushCache() const;
        private: // implement Taggable interface
        public:
            void initializeTags(TagManager& tagManager) override;
            void clearTags() override;
            void updateTags(TagManager& tagManager) override;

            /**
             * Indicates whether all of the faces of this brush have any of the given tags.
             *
             * @param tagMask the tags to check
             * @return true whether all faces of this brush have any of the given tags
             */
            bool allFacesHaveAnyTagInMask(TagType::Type tagMask) const;

            /**
             * Indicates whether any of the faces of this brush have any tags.
             *
             * @return true whether any faces of this brush have any tags
             */
            bool anyFaceHasAnyTag() const;

            /**
             * Indicates whether any of the faces of this brush have any of the given tags.
             *
             * @param tagMask the tags to check
             * @return true whether any faces of this brush have any of the given tags
             */
            bool anyFacesHaveAnyTagInMask(TagType::Type tagMask) const;
        private:
            void doAcceptTagVisitor(TagVisitor& visitor) override;
            void doAcceptTagVisitor(ConstTagVisitor& visitor) const override;
        private:
            deleteCopyAndMove(BrushNode)
        };
    }
}

#endif /* defined(TrenchBroom_BrushNode) */
