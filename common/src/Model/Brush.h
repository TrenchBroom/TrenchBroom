/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef __TrenchBroom__Brush__
#define __TrenchBroom__Brush__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Hit.h"
#include "Model/BrushContentType.h"
#include "Model/BrushGeometryTypes.h"
#include "Model/Node.h"
#include "Model/Object.h"

namespace TrenchBroom {
    namespace Model {
        struct BrushAlgorithmResult;
        class BrushContentTypeBuilder;
        class BrushGeometry;
        
        class Brush : public Node, public Object {
        public:
            static const Hit::HitType BrushHit;
        private:
            BrushFaceList m_faces;
            BrushGeometry* m_geometry;
            
            const BrushContentTypeBuilder* m_contentTypeBuilder;
            mutable BrushContentType::FlagType m_contentType;
            mutable bool m_transparent;
            mutable bool m_contentTypeValid;
            
            class Contains;
            class Intersects;
        public:
            Brush(const BBox3& worldBounds, const BrushFaceList& faces);
            ~Brush();
            
            Brush* clone(const BBox3& worldBounds) const;
            
            Attributable* entity() const;
        public: // face management:
            const BrushFaceList& faces() const;
            void setFaces(const BBox3& worldBounds, const BrushFaceList& faces);

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
                BrushFaceList::iterator rem = m_faces.end();
                while (cur != end) {
                    rem = doRemoveFace(m_faces.begin(), rem, *cur);
                    ++cur;
                }
                
                m_faces.erase(rem, m_faces.end());
            }
            
            void removeFace(BrushFace* face);
            BrushFaceList::iterator doRemoveFace(BrushFaceList::iterator begin, BrushFaceList::iterator end, BrushFace* face);
            
            void detachFaces(const BrushFaceList& faces);
            void detachFace(BrushFace* face);
        public: // clipping
            bool clip(const BBox3& worldBounds, BrushFace* face);
        public: // move face along normal
            bool canMoveBoundary(const BBox3& worldBounds, const BrushFace* face, const Vec3& delta) const;
            void moveBoundary(const BBox3& worldBounds, BrushFace* face, const Vec3& delta, const bool lockTexture);
        public:
            // geometry access
            size_t vertexCount() const;
            const BrushVertexList& vertices() const;
            
            size_t edgeCount() const;
            const BrushEdgeList& edges() const;
            
            bool containsPoint(const Vec3& point) const;
            
            // vertex operations
            bool canMoveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta);
            Vec3::List moveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta);

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
            void processBrushAlgorithmResult(const BBox3& worldBounds, const BrushAlgorithmResult& result);
            void invalidateFaces();
        private: // brush geometry
            void rebuildGeometry(const BBox3& worldBounds);
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
            Node* doClone(const BBox3& worldBounds) const;
            NodeSnapshot* doTakeSnapshot();
            
            bool doCanAddChild(const Node* child) const;
            bool doCanRemoveChild(const Node* child) const;
            bool doRemoveIfEmpty() const;

            bool doSelectable() const;
            void doAccept(NodeVisitor& visitor);
            void doAccept(ConstNodeVisitor& visitor) const;
        private: // implement Object interface
            const BBox3& doGetBounds() const;
            void doPick(const Ray3& ray, Hits& hits) const;

            Node* doGetContainer() const;
            Layer* doGetLayer() const;
            Group* doGetGroup() const;
            
            void doTransform(const Mat4x4& transformation, bool lockTextures, const BBox3& worldBounds);
            bool doContains(const Node* node) const;
            bool doIntersects(const Node* node) const;
        };
    }
}

#endif /* defined(__TrenchBroom__Brush__) */
