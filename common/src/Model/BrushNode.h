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
        class ModelFactory;
        template <typename P> class PolyhedronMatcher;

        struct BrushAlgorithmResult;

        class BrushNode : public Node, public Object {
        private:
            friend class SetTempFaceLinks;
        public:
            static const HitType::Type BrushHit;
        private:
            class AddFaceToGeometryCallback;
            class HealEdgesCallback;
            class AddFacesToGeometry;
            class MoveVerticesCallback;
            using RemoveVertexCallback = MoveVerticesCallback;
            class QueryCallback;
        public:
            using VertexList = BrushVertexList;
            using EdgeList = BrushEdgeList;
        private:
            std::vector<BrushFace*> m_faces;
            BrushGeometry* m_geometry;

            mutable bool m_transparent;
            mutable std::unique_ptr<Renderer::BrushRendererBrushCache> m_brushRendererBrushCache; // unique_ptr for breaking header dependencies
        public:
            BrushNode(const vm::bbox3& worldBounds, const std::vector<BrushFace*>& faces);
            ~BrushNode() override;
        private:
            void cleanup();
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

            bool closed() const;
            bool fullySpecified() const;

            void faceDidChange();
        private:
            void addFaces(const std::vector<BrushFace*>& faces);
            template <typename I>
            void addFaces(I cur, I end, size_t count) {
                m_faces.reserve(m_faces.size() + count);
                while (cur != end) {
                    addFace(*cur);
                    ++cur;
                }
            }
            void addFace(BrushFace* face);

            template <typename I>
            void removeFaces(I cur, I end) {
                auto rem = std::end(m_faces);
                while (cur != end) {
                    rem = doRemoveFace(std::begin(m_faces), rem, *cur);
                    ++cur;
                }

                m_faces.erase(rem, std::end(m_faces));
            }

            void removeFace(BrushFace* face);
            std::vector<BrushFace*>::iterator doRemoveFace(std::vector<BrushFace*>::iterator begin, std::vector<BrushFace*>::iterator end, BrushFace* face);

            void detachFaces(const std::vector<BrushFace*>& faces);
            void detachFace(BrushFace* face);
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
            const std::vector<vm::vec3> vertexPositions() const;
            vm::vec3 findClosestVertexPosition(const vm::vec3& position) const;

            bool hasVertex(const vm::vec3& position, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasVertices(const std::vector<vm::vec3>& positions, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasEdge(const vm::segment3& edge, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasEdges(const std::vector<vm::segment3>& edges, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasFace(const vm::polygon3& face, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasFaces(const std::vector<vm::polygon3>& faces, FloatType epsilon = static_cast<FloatType>(0.0)) const;

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
        private:
            struct CanMoveVerticesResult {
            public:
                bool success;
                std::unique_ptr<BrushGeometry> geometry;
            private:
                CanMoveVerticesResult(bool s, BrushGeometry&& g);
            public:
                static CanMoveVerticesResult rejectVertexMove();
                static CanMoveVerticesResult acceptVertexMove(BrushGeometry&& result);
            };

            CanMoveVerticesResult doCanMoveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions, vm::vec3 delta, bool allowVertexRemoval) const;
            void doMoveVertices(const vm::bbox3& worldBounds, const std::vector<vm::vec3>& vertexPositions, const vm::vec3& delta, bool lockTexture);
            /**
             * Tries to find 3 vertices in `left` and `right` that are related according to the PolyhedronMatcher, and
             * generates an affine transform for them which can then be used to implement UV lock.
             *
             * @param matcher a polyhedron matcher which is used to identify related vertices
             * @param left the face of the left polyhedron
             * @param right the face of the right polyhedron
             * @return {true, transform} if a transform could be found, otherwise {false, unspecified}
             */
            static std::tuple<bool, vm::mat4x4> findTransformForUVLock(const PolyhedronMatcher<BrushGeometry>& matcher, BrushFaceGeometry* left, BrushFaceGeometry* right);
            /**
             * Helper function to apply UV lock to the face `right`.
             *
             * It's assumed that `left` and `right` have already been identified as "matching" faces for a vertex move
             * where `left` is a face from the polyhedron before vertex manipulation, and `right` is from the newly
             * modified brush.
             *
             * This function tries to pick 3 vertices from `left` and `right` to generate a transform
             * (using findTransformForUVLock), and updates the texturing of `right` using that transform applied to `left`.
             * If it can't perform UV lock, `right` remains unmodified.
             *
             * This is only meant to be called in the matcher callback in Brush::doSetNewGeometry
             *
             * @param matcher a polyhedron matcher which is used to identify related vertices
             * @param left the face of the left polyhedron
             * @param right the face of the right polyhedron
             */
            void applyUVLock(const PolyhedronMatcher<BrushGeometry>& matcher, BrushFaceGeometry* left, BrushFaceGeometry* right);
            void doSetNewGeometry(const vm::bbox3& worldBounds, const PolyhedronMatcher<BrushGeometry>& matcher, const BrushGeometry& newGeometry, bool uvLock = false);
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
        private:
            /**
             * Final step of CSG subtraction; takes the geometry that is the result of the subtraction, and turns it
             * into a Brush by copying texturing from `this` (for un-clipped faces) or the brushes in `subtrahends`
             * (for clipped faces).
             *
             * @param factory the model factory
             * @param worldBounds the world bounds
             * @param defaultTextureName default texture name
             * @param geometry the geometry for the newly created brush
             * @param subtrahends used as a source of texture alignment only
             * @return the newly created brush
             */
            BrushNode* createBrush(const ModelFactory& factory, const vm::bbox3& worldBounds, const std::string& defaultTextureName, const BrushGeometry& geometry, const std::vector<BrushNode*>& subtrahends) const;
        private:
            void updateFacesFromGeometry(const vm::bbox3& worldBounds, const BrushGeometry& geometry);
            void updatePointsFromVertices(const vm::bbox3& worldBounds);
        public: // brush geometry
            void rebuildGeometry(const vm::bbox3& worldBounds);
        private:
            void buildGeometry(const vm::bbox3& worldBounds);
            void deleteGeometry();
            bool checkGeometry() const;
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
            Layer* doGetLayer() const override;
            Group* doGetGroup() const override;

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
