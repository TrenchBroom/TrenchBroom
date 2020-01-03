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

#ifndef Polyhedron_Instantiation_h
#define Polyhedron_Instantiation_h

#include "FloatType.h"
#include "Polyhedron.h"
#include "Polyhedron_BrushGeometryPayload.h"
#include "Polyhedron_DefaultPayload.h"

namespace TrenchBroom {
    namespace Model {
        extern template class Polyhedron_Vertex<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        extern template class Polyhedron_Vertex<FloatType, BrushFacePayload, BrushVertexPayload>;

        extern template class Polyhedron_Edge<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        extern template class Polyhedron_Edge<FloatType, BrushFacePayload, BrushVertexPayload>;

        extern template class Polyhedron_HalfEdge<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        extern template class Polyhedron_HalfEdge<FloatType, BrushFacePayload, BrushVertexPayload>;

        extern template class Polyhedron_Face<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        extern template class Polyhedron_Face<FloatType, BrushFacePayload, BrushVertexPayload>;

        extern template class Polyhedron<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
        extern template class Polyhedron<FloatType, BrushFacePayload, BrushVertexPayload>;
    }
}

#endif
