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
        class Entity;
        
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
            
            Entity* parent() const;
            void setParent(Entity* parent);
            
            bool select();
            bool deselect();
            
            BBox3 bounds() const;
            void pick(const Ray3& ray, PickResult& result);
            
            const BrushFaceList& faces() const;
            const BrushEdgeList& edges() const;
            BrushFaceList incidentFaces(const BrushVertex& vertex) const;
            
            void addEdges(Vertex::List& vertices) const;
            
            bool clip(const BBox3& worldBounds, BrushFace* face);
            bool canMoveBoundary(const BBox3& worldBounds, const BrushFace& face, const Vec3& delta) const;
            void moveBoundary(const BBox3& worldBounds, BrushFace& face, const Vec3& delta, const bool lockTexture);
        private:
            void rebuildGeometry(const BBox3& worldBounds, const BrushFaceList faces);
            void addFaces(const BrushFaceList& faces);
            void addFace(BrushFace* face);

            Brush(const Brush& other);
            Brush& operator=(const Brush& other);
        };
    }
}

#endif /* defined(__TrenchBroom__Brush__) */
