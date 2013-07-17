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
#include "Model/Object.h"
#include "Model/Picker.h"
#include "Renderer/VertexSpec.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushGeometry;
        class Entity;
        
        class Brush : public Object, public std::tr1::enable_shared_from_this<Brush> {
        public:
            typedef std::tr1::shared_ptr<Brush> Ptr;
            typedef std::vector<Brush::Ptr> List;
            static const List EmptyList;
            typedef Renderer::VertexSpecs::P3 VertexSpec;
            typedef VertexSpec::Vertex Vertex;

            static const Hit::HitType BrushHit;
        private:
            Entity* m_parent;
            BrushFace::List m_faces;
            BrushGeometry* m_geometry;
        public:
            static Brush::Ptr newBrush(const BBox3& worldBounds, const BrushFace::List& faces);
            ~Brush();
            
            Entity* parent() const;
            void setParent(Entity* parent);
            
            bool select();
            bool deselect();
            
            BBox3 bounds() const;
            void pick(const Ray3& ray, PickResult& result);
            
            const BrushFace::List& faces() const;
            const BrushEdge::List& edges() const;

            template <class Operator, class Filter>
            void eachBrushFace(const Operator& op, const Filter& filter) {
                Ptr thisPtr = sharedFromThis();
                BrushFace::List::const_iterator it, end;
                for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                    BrushFace::Ptr face = *it;
                    if (filter(thisPtr, face))
                        op(thisPtr, face);
                }
            }
            
            template <class Operator, class Filter>
            void eachBrushFace(Operator& op, const Filter& filter) {
                Ptr thisPtr = sharedFromThis();
                BrushFace::List::const_iterator it, end;
                for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                    BrushFace::Ptr face = *it;
                    if (filter(thisPtr, face))
                        op(thisPtr, face);
                }
            }
            
            void addEdges(Vertex::List& vertices) const;
        private:
            Brush(const BBox3& worldBounds, const BrushFace::List& faces);
            Ptr sharedFromThis();
            void rebuildGeometry(const BBox3& worldBounds, const BrushFace::List& faces);
            void addFaces(const BrushFace::List& faces);
            void addFace(BrushFace::Ptr face);
            void removeAllFaces();
        };
    }
}

#endif /* defined(__TrenchBroom__Brush__) */
