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

#pragma once

#include "FloatType.h"

#include "Model/Polyhedron_BrushGeometryPayload.h"
#include "Model/Polyhedron_DefaultPayload.h"
#include "Model/Polyhedron_Forward.h"

namespace TrenchBroom {
    namespace Model {
        using BrushGeometry = Polyhedron<FloatType, BrushFacePayload, BrushVertexPayload>;

        using BrushVertex = Polyhedron_Vertex<FloatType, BrushFacePayload, BrushVertexPayload>;
        using BrushEdge = Polyhedron_Edge<FloatType, BrushFacePayload, BrushVertexPayload>;
        using BrushHalfEdge = Polyhedron_HalfEdge<FloatType, BrushFacePayload, BrushVertexPayload>;
        using BrushFaceGeometry = Polyhedron_Face<FloatType, BrushFacePayload, BrushVertexPayload>;

        using BrushVertexList = Polyhedron_VertexList<FloatType, BrushFacePayload, BrushVertexPayload>;
        using BrushEdgeList = Polyhedron_EdgeList<FloatType, BrushFacePayload, BrushVertexPayload>;
        using BrushHalfEdgeList = Polyhedron_HalfEdgeList<FloatType, BrushFacePayload, BrushVertexPayload>;
    }
}

#endif
