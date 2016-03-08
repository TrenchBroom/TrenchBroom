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

#ifndef TrenchBroom_BrushGeometry_h
#define TrenchBroom_BrushGeometry_h

#include "TrenchBroom.h"
#include "Polyhedron.h"
#include "Polyhedron_BrushGeometryPayload.h"
#include "Polyhedron_DefaultPayload.h"

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class BrushFace;
        
        typedef Polyhedron<FloatType, BrushFacePayload, BrushVertexPayload> BrushGeometry;
        
        void restoreFaceLinks(BrushGeometry* geometry);
        void restoreFaceLinks(BrushGeometry& geometry);
        
        class SetTempFaceLinks {
        private:
            Brush* m_brush;
        public:
            SetTempFaceLinks(Brush* brush, BrushGeometry& tempGeometry);
            ~SetTempFaceLinks();
        };
        
        typedef BrushGeometry::Vertex BrushVertex;
        typedef BrushGeometry::Edge BrushEdge;
        typedef BrushGeometry::HalfEdge BrushHalfEdge;
        typedef BrushGeometry::Face BrushFaceGeometry;

        typedef BrushGeometry::VertexList BrushVertexList;
        typedef BrushGeometry::EdgeList BrushEdgeList;
        typedef BrushGeometry::HalfEdgeList BrushHalfEdgeList;
    }
}

#endif
