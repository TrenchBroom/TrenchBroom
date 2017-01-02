/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

namespace TrenchBroom {
    namespace Model {
        struct BrushAlgorithmResult;
        class BrushContentTypeBuilder;
        class ModelFactory;
        class PickResult;
        
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
            class CanMoveBoundaryCallback;
            class CanMoveBoundary;
            class MoveVerticesCallback;
            typedef MoveVerticesCallback RemoveVertexCallback;
            class QueryCallback;
            class FaceMatchingCallback;
        public:
            typedef ConstProjectingSequence<BrushVertexList, ProjectToVertex> VertexList;
            typedef ConstProjectingSequence<BrushEdgeList, ProjectToEdge> EdgeList;
        private:
            BrushFaceArray m_faces;
            BrushGeometry* m_geometry;
            
            const BrushContentTypeBuilder* m_contentTypeBuilder;
            mutable BrushContentType::FlagType m_contentType;
            mutable bool m_transparent;
            mutable bool m_contentTypeValid;
        public:
            Brush(const BBox3& worldBounds, const BrushFaceArray& faces);
            ~Brush();
        private:
            void cleanup();
        public:
            Brush* clone(const BBox3& worldBounds) const;
            
            AttributableNode* entity() const;
        public: // face management:
            BrushFace* findFace(const Vec3& normal) const;
            BrushFace* findFace(const Plane3& boundary) const;
            BrushFace* findFace(const Polygon3& vertices) const;
            BrushFace* findFace(const Polygon3::List& candidates) const;
            
            size_t faceCount() const;
            const BrushFaceArray& faces() const;
            void setFaces(const BBox3& worldBounds, const BrushFaceArray& faces);
            
            bool fullySpecified() const;
            
            void faceDidChange();
        private:
            void addFaces(const BrushFaceArray& faces);
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
                BrushFaceArray::iterator rem = std::end(m_faces);
                while (cur != end) {
                    rem = doRemoveFace(std::begin(m_faces), rem, *cur);
                    ++cur;
                }
                
                m_faces.erase(rem, std::end(m_faces));
            }
            
            void removeFace(BrushFace* face);
            BrushFaceArray::iterator doRemoveFace(BrushFaceArray::iterator begin, BrushFaceArray::iterator end, BrushFace* face);
            
            void detachFaces(const BrushFaceArray& faces);
            void detachFace(BrushFace* face);
        public: // clone face attributes from matching faces of other brushes
            void cloneFaceAttributesFrom(const BrushArray& brushes);
            void cloneFaceAttributesFrom(const Brush* brush);
            void cloneInvertedFaceAttributesFrom(const BrushArray& brushes);
            void cloneInvertedFaceAttributesFrom(const Brush* brush);
        public: // clipping
            bool clip(const BBox3& worldBounds, BrushFace* face);
        public: // move face along normal
            bool canMoveBoundary(const BBox3& worldBounds, const BrushFace* face, const Vec3& delta) const;
            void moveBoundary(const BBox3& worldBounds, BrushFace* face, const Vec3& delta, const bool lockTexture);
        public:
            // geometry access
            size_t vertexCount() const;
            VertexList vertices() const;
            
            bool hasVertex(const Vec3& position) const;
            bool hasVertices(const Vec3::List positions) const;
            bool hasEdge(const Edge3& edge) const;
            bool hasEdges(const Edge3::List& edges) const;
            bool hasFace(const Polygon3& face) const;
            bool hasFaces(const Polygon3::List& faces) const;
            
            bool hasFace(const Vec3& p1, const Vec3& p2, const Vec3& p3) const;
            bool hasFace(const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& p4) const;
            bool hasFace(const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& p4, const Vec3& p5) const;
            
            size_t edgeCount() const;
            EdgeList edges() const;
            
            bool containsPoint(const Vec3& point) const;
            
            BrushFaceArray incidentFaces(const BrushVertex* vertex) const;
            
            // vertex operations
            bool canMoveVertices(const BBox3& worldBounds, const Vec3::List& vertices, const Vec3& delta) const;
            Vec3::List moveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta);

            bool canAddVertex(const BBox3& worldBounds, const Vec3& position) const;
            BrushVertex* addVertex(const BBox3& worldBounds, const Vec3& position);
            
            bool canRemoveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions) const;
            void removeVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions);
            
            bool canSnapVertices(const BBox3& worldBounds, size_t snapTo);
            void snapVertices(const BBox3& worldBounds, size_t snapTo);

            // edge operations
            bool canMoveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta);
            Edge3::List moveEdges(const BBox3& worldBounds, const Edge3::List& edgePositions, const Vec3& delta);
            bool canSplitEdge(const BBox3& worldBounds, const Edge3& edgePosition, const Vec3& delta);
            Vec3 splitEdge(const BBox3& worldBounds, const Edge3& edgePosition, const Vec3& delta);
            
            // face operations
            bool canMoveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta);
            Polygon3::List moveFaces(const BBox3& worldBounds, const Polygon3::List& facePositions, const Vec3& delta);
            bool canSplitFace(const BBox3& worldBounds, const Polygon3& facePosition, const Vec3& delta);
            Vec3 splitFace(const BBox3& worldBounds, const Polygon3& facePosition, const Vec3& delta);
        private:
            bool doCanMoveVertices(const BBox3& worldBounds, const Vec3::List& vertices, Vec3 delta, bool allowVertexRemoval) const;
            void doSetNewGeometry(const BBox3& worldBounds, const PolyhedronMatcher<BrushGeometry>& matcher, BrushGeometry& newGeometry);
        public:
            // CSG operations
            BrushArray subtract(const ModelFactory& factory, const BBox3& worldBounds, const String& defaultTextureName, const Brush* subtrahend) const;
            void intersect(const BBox3& worldBounds, const Brush* brush);
        private:
            Brush* createBrush(const ModelFactory& factory, const BBox3& worldBounds, const String& defaultTextureName, const BrushGeometry& geometry, const Brush* subtrahend) const;
        private:
            void updateFacesFromGeometry(const BBox3& worldBounds);
            void updatePointsFromVertices(const BBox3& worldBounds);
        public: // brush geometry
            void rebuildGeometry(const BBox3& worldBounds);
            void findIntegerPlanePoints(const BBox3& worldBounds);
        private:
            bool checkGeometry() const;
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
            const String& doGetName() const;
            const BBox3& doGetBounds() const;
            
            Node* doClone(const BBox3& worldBounds) const;
            NodeSnapshot* doTakeSnapshot();
            
            bool doCanAddChild(const Node* child) const;
            bool doCanRemoveChild(const Node* child) const;
            bool doRemoveIfEmpty() const;

            void doParentDidChange();

            bool doSelectable() const;

            void doGenerateIssues(const IssueGenerator* generator, IssueArray& issues);
            void doAccept(NodeVisitor& visitor);
            void doAccept(ConstNodeVisitor& visitor) const;
        private: // implement Object interface
            void doPick(const Ray3& ray, PickResult& pickResult) const;
            void doFindNodesContaining(const Vec3& point, NodeArray& result);
            FloatType doIntersectWithRay(const Ray3& ray) const;

            struct BrushFaceHit {
                BrushFace* face;
                FloatType distance;
				BrushFaceHit();
                BrushFaceHit(BrushFace* i_face, FloatType i_distance);
            };

            BrushFaceHit findFaceHit(const Ray3& ray) const;
            
            Node* doGetContainer() const;
            Layer* doGetLayer() const;
            Group* doGetGroup() const;
            
            void doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds);

            class Contains;
            bool doContains(const Node* node) const;
            
            class Intersects;
            bool doIntersects(const Node* node) const;
        private:
            Brush(const Brush&);
            Brush& operator=(const Brush&);
        };
    }
}

#endif /* defined(TrenchBroom_Brush) */
