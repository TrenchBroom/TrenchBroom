/*
 Copyright (C) 2010 Kristian Duske

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

#include "mdl/Polyhedron.h"
#include "mdl/Polyhedron_BrushGeometryPayload.h"

namespace tb::mdl
{

using BrushGeometry = Polyhedron<double, BrushFacePayload, BrushVertexPayload>;

using BrushVertex = Polyhedron_Vertex<double, BrushFacePayload, BrushVertexPayload>;
using BrushEdge = Polyhedron_Edge<double, BrushFacePayload, BrushVertexPayload>;
using BrushHalfEdge = Polyhedron_HalfEdge<double, BrushFacePayload, BrushVertexPayload>;
using BrushFaceGeometry = Polyhedron_Face<double, BrushFacePayload, BrushVertexPayload>;

using BrushVertexList =
  Polyhedron_VertexList<double, BrushFacePayload, BrushVertexPayload>;
using BrushEdgeList = Polyhedron_EdgeList<double, BrushFacePayload, BrushVertexPayload>;
using BrushHalfEdgeList =
  Polyhedron_HalfEdgeList<double, BrushFacePayload, BrushVertexPayload>;

} // namespace tb::mdl
