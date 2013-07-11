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
#include "Model/BrushFace.h"
#include "Model/Object.h"
#include "Renderer/VertexSpec.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushGeometry;
        
        class Brush : public Object {
        public:
            typedef std::tr1::shared_ptr<Brush> Ptr;
            typedef std::vector<Brush::Ptr> List;
            static const List EmptyList;
            typedef Renderer::VertexSpecs::P3 VertexSpec;
            typedef VertexSpec::Vertex Vertex;
        private:
            BrushFace::List m_faces;
            BrushGeometry* m_geometry;

            Brush(const BBox3& worldBounds, const BrushFace::List& faces);
        public:
            static Brush::Ptr newBrush(const BBox3& worldBounds, const BrushFace::List& faces);
            ~Brush();
            
            const BrushFace::List& faces() const;

            template <class Operator, class Filter>
            void eachBrushFace(Operator& op, Filter& filter) {
                BrushFace::List::const_iterator it, end;
                for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                    BrushFace::Ptr face = *it;
                    if (filter(face))
                        op(face);
                }
            }
            
            void addEdges(Vertex::List& vertices) const;
        private:
            void rebuildGeometry(const BBox3& worldBounds);
        };
    }
}

#endif /* defined(__TrenchBroom__Brush__) */
