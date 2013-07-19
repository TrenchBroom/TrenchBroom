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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__Brush__
#define __TrenchBroom__Brush__

#include "TrenchBroom.h"
#include "VecMath.h"
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
        
        class Brush : public Object {
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
            ~Brush();
            
            Entity* parent() const;
            void setParent(Entity* parent);
            
            bool select();
            bool deselect();
            
            BBox3 bounds() const;
            void pick(const Ray3& ray, PickResult& result);
            
            const BrushFaceList& faces() const;
            const BrushEdge::List& edges() const;

            template <class Operator, class Filter>
            void eachBrushFace(const Operator& op, const Filter& filter) {
                BrushFaceList::const_iterator it, end;
                for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                    BrushFace* face = *it;
                    if (filter(this, face))
                        op(this, face);
                }
            }
            
            template <class Operator, class Filter>
            void eachBrushFace(Operator& op, const Filter& filter) {
                BrushFaceList::const_iterator it, end;
                for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                    BrushFace* face = *it;
                    if (filter(this, face))
                        op(this, face);
                }
            }
            
            void addEdges(Vertex::List& vertices) const;
        private:
            void rebuildGeometry(const BBox3& worldBounds, const BrushFaceList& faces);
            void addFaces(const BrushFaceList& faces);
            void addFace(BrushFace* face);
        };
    }
}

#endif /* defined(__TrenchBroom__Brush__) */
