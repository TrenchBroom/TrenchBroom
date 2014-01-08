/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
#include "Allocator.h"
#include "SharedPointer.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/ModelTypes.h"
#include "Model/Object.h"
#include "Model/Picker.h"
#include "Renderer/VertexSpec.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushGeometry;
        struct BrushAlgorithmResult;
        class Entity;
        class Brush;

        class BrushSnapshot {
        private:
            struct FacesHolder {
                typedef std::tr1::shared_ptr<FacesHolder> Ptr;
                BrushFaceList faces;
                ~FacesHolder();
            };
            
            Brush* m_brush;
            FacesHolder::Ptr m_holder;
        public:
            BrushSnapshot(Brush& brush);
            void restore(const BBox3& worldBounds);
        };
        
        class Brush : public Object, public Allocator<Brush> {
        public:
            typedef Renderer::VertexSpecs::P3 VertexSpec;
            typedef VertexSpec::Vertex Vertex;

            static const Hit::HitType BrushHit;
        private:
            Entity* m_parent;
            BrushFaceList m_faces;
            BrushGeometry* m_geometry;
        public:
            Brush(const BBox3& worldBounds, const BrushFaceList& faces);
            Brush(const BBox3& worldBounds, const Brush& other);
            ~Brush();
            
            Brush* clone(const BBox3& worldBounds) const;
            BrushSnapshot takeSnapshot();
            
            Entity* parent() const;
            void setParent(Entity* parent);
            
            void select();
            void deselect();
            
            BBox3 bounds() const;
            void pick(const Ray3& ray, PickResult& result);
            
            const BrushFaceList& faces() const;
            const BrushEdgeList& edges() const;
            const BrushVertexList& vertices() const;
            BrushFaceList incidentFaces(const BrushVertex& vertex) const;
            
            void addEdges(Vertex::List& vertices) const;
            
            bool clip(const BBox3& worldBounds, BrushFace* face);
            bool canMoveBoundary(const BBox3& worldBounds, const BrushFace& face, const Vec3& delta) const;
            void moveBoundary(const BBox3& worldBounds, BrushFace& face, const Vec3& delta, const bool lockTexture);
            
            bool canMoveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta);
            Vec3::List moveVertices(const BBox3& worldBounds, const Vec3::List& vertexPositions, const Vec3& delta);
            
            void snapPlanePointsToInteger(const BBox3& worldBounds);
            void findIntegerPlanePoints(const BBox3& worldBounds);
        private:
            void doTransform(const Mat4x4& transformation, const bool lockTextures, const BBox3& worldBounds);
            bool doContains(const Object& object) const;
            bool doContains(const Entity& entity) const;
            bool doContains(const Brush& brush) const;
            bool doContainedBy(const Object& object) const;
            bool doContainedBy(const Entity& entity) const;
            bool doContainedBy(const Brush& brush) const;
            bool doIntersects(const Object& object) const;
            bool doIntersects(const Entity& entity) const;
            bool doIntersects(const Brush& brush) const;
            void doVisit(ObjectVisitor& visitor);
            Object* doClone(const BBox3& worldBounds) const;
        private:
            bool containsPoint(const Vec3& point) const;

            friend class BrushSnapshot;
            void restoreFaces(const BBox3& worldBounds, const BrushFaceList& faces);
            
            void processBrushAlgorithmResult(const BBox3& worldBounds, const BrushAlgorithmResult& result);
            void rebuildGeometry(const BBox3& worldBounds);
            void addFaces(const BrushFaceList& faces);
            void addFace(BrushFace* face);
            void detachFaces(const BrushFaceList& faces);
            
            Brush(const Brush& other);
            Brush& operator=(const Brush& other);
        };
    }
}

#endif /* defined(__TrenchBroom__Brush__) */
