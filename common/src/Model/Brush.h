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

#ifndef TrenchBroom_Brush
#define TrenchBroom_Brush

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Hit.h"
#include "ProjectingSequence.h"
#include "Polyhedron_Matcher.h"
#include "Model/BrushContentType.h"
#include "Model/BrushGeometry.h"
#include "Model/Node.h"
#include "Model/Object.h"
#include "Renderer/BrushRendererBrushCache.h"
#include "Renderer/VertexListBuilder.h"
#include "Renderer/TexturedIndexArrayMap.h"
#include "Renderer/IndexArrayMapBuilder.h"
#include "Renderer/TexturedIndexArrayBuilder.h"

#include <vector>
#include <Renderer/BrushRendererBrushCache.h>

#include <set>

namespace TrenchBroom {
    namespace Model {
        struct BrushAlgorithmResult;
        class BrushContentTypeBuilder;
        class ModelFactory;
        class PickResult;
        class BrushRendererBrushCache;
        
        class Brush : public Node, public Object {
        private:
            friend class SetTempFaceLinks;
        public:
            static const Hit::HitType BrushHit;
        private:
            struct ProjectToVertex : public ProjectingSequenceProjector<BrushVertex*, BrushVertex*> {
                static BrushVertex*& project(BrushVertex*& vertex);
            };
            
            struct ProjectToEdge : public ProjectingSequenceProjector<BrushEdge*, BrushEdge*> {
                static BrushEdge*& project(BrushEdge*& edge);
            };
            
            class AddFaceToGeometryCallback;
            class HealEdgesCallback;
            class AddFacesToGeometry;
            class MoveVerticesCallback;
            typedef MoveVerticesCallback RemoveVertexCallback;
            class QueryCallback;
            class FaceMatchingCallback;
            
            using VertexSet = std::set<vm::vec3>;
        public:
            typedef ConstProjectingSequence<BrushVertexList, ProjectToVertex> VertexList;
            typedef ConstProjectingSequence<BrushEdgeList, ProjectToEdge> EdgeList;

        private:
            BrushFaceList m_faces;
            BrushGeometry* m_geometry;
            
            const BrushContentTypeBuilder* m_contentTypeBuilder;
            mutable BrushContentType::FlagType m_contentType;
            mutable bool m_transparent;
            mutable bool m_contentTypeValid;
            mutable Renderer::BrushRendererBrushCache m_brushRendererBrushCache;
        public:
            Brush(const vm::bbox3& worldBounds, const BrushFaceList& faces);
            ~Brush() override;
        private:
            void cleanup();
        public:
            Brush* clone(const vm::bbox3& worldBounds) const;
            
            AttributableNode* entity() const;
        public: // face management:
            BrushFace* findFace(const vm::vec3& normal) const;
            BrushFace* findFace(const vm::plane3& boundary) const;
            BrushFace* findFace(const vm::polygon3& vertices) const;
            BrushFace* findFace(const vm::polygon3::List& candidates) const;
            
            size_t faceCount() const;
            const BrushFaceList& faces() const;
            void setFaces(const vm::bbox3& worldBounds, const BrushFaceList& faces);

            bool closed() const;
            bool fullySpecified() const;
            
            void faceDidChange();
        private:
            void addFaces(const BrushFaceList& faces);
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
                BrushFaceList::iterator rem = std::end(m_faces);
                while (cur != end) {
                    rem = doRemoveFace(std::begin(m_faces), rem, *cur);
                    ++cur;
                }
                
                m_faces.erase(rem, std::end(m_faces));
            }
            
            void removeFace(BrushFace* face);
            BrushFaceList::iterator doRemoveFace(BrushFaceList::iterator begin, BrushFaceList::iterator end, BrushFace* face);
            
            void detachFaces(const BrushFaceList& faces);
            void detachFace(BrushFace* face);
        public: // clone face attributes from matching faces of other brushes
            void cloneFaceAttributesFrom(const BrushList& brushes);
            void cloneFaceAttributesFrom(const Brush* brush);
            void cloneInvertedFaceAttributesFrom(const BrushList& brushes);
            void cloneInvertedFaceAttributesFrom(const Brush* brush);
        public: // clipping
            bool clip(const vm::bbox3& worldBounds, BrushFace* face);
        public: // move face along normal
            bool canMoveBoundary(const vm::bbox3& worldBounds, const BrushFace* face, const vm::vec3& delta) const;
            void moveBoundary(const vm::bbox3& worldBounds, BrushFace* face, const vm::vec3& delta, const bool lockTexture);
            bool canExpand(const vm::bbox3& worldBounds, const FloatType delta, const bool lockTexture) const;
            /**
             * Moves all faces by `delta` units along their normals; negative values shrink the brush.
             * Returns true if the brush is valid after the modification, false if the brush is invalid.
             */
            bool expand(const vm::bbox3& worldBounds, const FloatType delta, const bool lockTexture);
        public:
            // geometry access
            size_t vertexCount() const;
            VertexList vertices() const;
            const vm::vec3::List vertexPositions() const;
            vm::vec3 findClosestVertexPosition(const vm::vec3& position) const;

            bool hasVertex(const vm::vec3& position, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasVertices(const vm::vec3::List positions, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasEdge(const vm::segment3& edge, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasEdges(const vm::segment3::List& edges, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasFace(const vm::polygon3& face, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasFaces(const vm::polygon3::List& faces, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            
            bool hasFace(const vm::vec3& p1, const vm::vec3& p2, const vm::vec3& p3, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasFace(const vm::vec3& p1, const vm::vec3& p2, const vm::vec3& p3, const vm::vec3& p4, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            bool hasFace(const vm::vec3& p1, const vm::vec3& p2, const vm::vec3& p3, const vm::vec3& p4, const vm::vec3& p5, FloatType epsilon = static_cast<FloatType>(0.0)) const;
            
            size_t edgeCount() const;
            EdgeList edges() const;
            bool containsPoint(const vm::vec3& point) const;
            
            BrushFaceList incidentFaces(const BrushVertex* vertex) const;
            
            // vertex operations
            bool canMoveVertices(const vm::bbox3& worldBounds, const vm::vec3::List& vertices, const vm::vec3& delta) const;
            vm::vec3::List moveVertices(const vm::bbox3& worldBounds, const vm::vec3::List& vertexPositions, const vm::vec3& delta);

            bool canAddVertex(const vm::bbox3& worldBounds, const vm::vec3& position) const;
            BrushVertex* addVertex(const vm::bbox3& worldBounds, const vm::vec3& position);
            
            bool canRemoveVertices(const vm::bbox3& worldBounds, const vm::vec3::List& vertexPositions) const;
            void removeVertices(const vm::bbox3& worldBounds, const vm::vec3::List& vertexPositions);
            
            bool canSnapVertices(const vm::bbox3& worldBounds, FloatType snapTo);
            void snapVertices(const vm::bbox3& worldBounds, FloatType snapTo);

            // edge operations
            bool canMoveEdges(const vm::bbox3& worldBounds, const vm::segment3::List& edgePositions, const vm::vec3& delta) const;
            vm::segment3::List moveEdges(const vm::bbox3& worldBounds, const vm::segment3::List& edgePositions, const vm::vec3& delta);

            // face operations
            bool canMoveFaces(const vm::bbox3& worldBounds, const vm::polygon3::List& facePositions, const vm::vec3& delta) const;
            vm::polygon3::List moveFaces(const vm::bbox3& worldBounds, const vm::polygon3::List& facePositions, const vm::vec3& delta);
        private:
            struct CanMoveVerticesResult {
            public:
                bool success;
                BrushGeometry geometry;
                
            private:
                CanMoveVerticesResult(bool s, const BrushGeometry& g);
                
            public:
                static CanMoveVerticesResult rejectVertexMove();
                static CanMoveVerticesResult acceptVertexMove(const BrushGeometry& result);
            };
            
            CanMoveVerticesResult doCanMoveVertices(const vm::bbox3& worldBounds, const vm::vec3::List& vertexPositions, vm::vec3 delta, bool allowVertexRemoval) const;
            void doMoveVertices(const vm::bbox3& worldBounds, const vm::vec3::List& vertexPositions, const vm::vec3& delta);
            void doSetNewGeometry(const vm::bbox3& worldBounds, const PolyhedronMatcher<BrushGeometry>& matcher, BrushGeometry& newGeometry);
            
            static VertexSet createVertexSet(const vm::vec3::List& vertices = vm::vec3::EmptyList);
        public:
            // CSG operations
            BrushList subtract(const ModelFactory& factory, const vm::bbox3& worldBounds, const String& defaultTextureName, const Brush* subtrahend) const;
            void intersect(const vm::bbox3& worldBounds, const Brush* brush);

            // transformation
            bool canTransform(const vm::mat4x4& transformation, const vm::bbox3& worldBounds) const;
        private:
            Brush* createBrush(const ModelFactory& factory, const vm::bbox3& worldBounds, const String& defaultTextureName, const BrushGeometry& geometry, const Brush* subtrahend) const;
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
        public: // content type
            bool transparent() const;
            bool hasContentType(const BrushContentType& contentType) const;
            bool hasContentType(BrushContentType::FlagType contentTypeMask) const;
            void setContentTypeBuilder(const BrushContentTypeBuilder* contentTypeBuilder);
        private:
            BrushContentType::FlagType contentTypeFlags() const;
            void invalidateContentType();
            void validateContentType() const;
        private: // implement Node interface
            const String& doGetName() const override;
            const vm::bbox3& doGetBounds() const override;
            
            Node* doClone(const vm::bbox3& worldBounds) const override;
            NodeSnapshot* doTakeSnapshot() override;
            
            bool doCanAddChild(const Node* child) const override;
            bool doCanRemoveChild(const Node* child) const override;
            bool doRemoveIfEmpty() const override;

            void doParentDidChange() override;

            bool doSelectable() const override;

            void doGenerateIssues(const IssueGenerator* generator, IssueList& issues) override;
            void doAccept(NodeVisitor& visitor) override;
            void doAccept(ConstNodeVisitor& visitor) const override;
        private: // implement Object interface
            void doPick(const vm::ray3& ray, PickResult& pickResult) const override;
            void doFindNodesContaining(const vm::vec3& point, NodeList& result) override;
            FloatType doIntersectWithRay(const vm::ray3& ray) const override;

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
        private:
            Brush(const Brush&);
            Brush& operator=(const Brush&);
            
        public: // renderer cache
            /**
             * Only exposed to be called by BrushFace
             */
            void invalidateVertexCache();
            Renderer::BrushRendererBrushCache& brushRendererBrushCache() const;
        };
    }
}

#endif /* defined(TrenchBroom_Brush) */
