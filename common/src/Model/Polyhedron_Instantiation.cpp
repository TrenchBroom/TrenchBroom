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

// clang-format off
// order of includes is important
#include "FloatType.h"
#include "Polyhedron.h"
#include "Polyhedron_Misc.h"
#include "Polyhedron_Vertex.h"
#include "Polyhedron_Edge.h"
#include "Polyhedron_HalfEdge.h"
#include "Polyhedron_Face.h"
#include "Polyhedron_ConvexHull.h"
#include "Polyhedron_Clip.h"
#include "Polyhedron_CSG.h"
#include "Polyhedron_Queries.h"
#include "Polyhedron_Checks.h"

#include "Polyhedron_BrushGeometryPayload.h"
#include "Polyhedron_DefaultPayload.h"
// clang-format on

namespace TrenchBroom {
    namespace Model {
        template struct Polyhedron_GetVertexLink<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        template struct Polyhedron_GetVertexLink<FloatType, BrushFacePayload, BrushVertexPayload>;

        template class Polyhedron_Vertex<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        template class Polyhedron_Vertex<FloatType, BrushFacePayload, BrushVertexPayload>;

        template struct Polyhedron_GetEdgeLink<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        template struct Polyhedron_GetEdgeLink<FloatType, BrushFacePayload, BrushVertexPayload>;

        template class Polyhedron_Edge<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        template class Polyhedron_Edge<FloatType, BrushFacePayload, BrushVertexPayload>;

        template struct Polyhedron_GetHalfEdgeLink<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        template struct Polyhedron_GetHalfEdgeLink<FloatType, BrushFacePayload, BrushVertexPayload>;

        template class Polyhedron_HalfEdge<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        template class Polyhedron_HalfEdge<FloatType, BrushFacePayload, BrushVertexPayload>;

        template struct Polyhedron_GetFaceLink<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        template struct Polyhedron_GetFaceLink<FloatType, BrushFacePayload, BrushVertexPayload>;

        template class Polyhedron_Face<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        template class Polyhedron_Face<FloatType, BrushFacePayload, BrushVertexPayload>;

        template class Polyhedron<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        template class Polyhedron<FloatType, BrushFacePayload, BrushVertexPayload>;
    }
}
