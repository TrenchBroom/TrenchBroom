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
        
        using BrushGeometry = Polyhedron<FloatType, BrushFacePayload, BrushVertexPayload>;
        
        using BrushVertex = BrushGeometry::Vertex;
        using BrushEdge = BrushGeometry::Edge;
        using BrushHalfEdge = BrushGeometry::HalfEdge;
        using BrushFaceGeometry = BrushGeometry::Face;

        using BrushVertexList = BrushGeometry::VertexList;
        using BrushEdgeList = BrushGeometry::EdgeList;
        using BrushHalfEdgeList = BrushGeometry::HalfEdgeList;
    }
}

#endif
