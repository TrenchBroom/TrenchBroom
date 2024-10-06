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

// clang-format off
// order of includes is important
#include "Polyhedron.h"
#include "Polyhedron_Misc.h" // IWYU pragma: keep
#include "Polyhedron_Vertex.h" // IWYU pragma: keep
#include "Polyhedron_Edge.h" // IWYU pragma: keep
#include "Polyhedron_HalfEdge.h" // IWYU pragma: keep
#include "Polyhedron_Face.h" // IWYU pragma: keep
#include "Polyhedron_ConvexHull.h" // IWYU pragma: keep
#include "Polyhedron_Clip.h" // IWYU pragma: keep
#include "Polyhedron_CSG.h" // IWYU pragma: keep
#include "Polyhedron_Queries.h" // IWYU pragma: keep
#include "Polyhedron_Checks.h"

#include "Polyhedron_BrushGeometryPayload.h"
#include "Polyhedron_DefaultPayload.h"
// clang-format on

namespace tb::mdl
{

template struct Polyhedron_GetVertexLink<
  double,
  DefaultPolyhedronPayload,
  DefaultPolyhedronPayload>;
template struct Polyhedron_GetVertexLink<double, BrushFacePayload, BrushVertexPayload>;

template class Polyhedron_Vertex<
  double,
  DefaultPolyhedronPayload,
  DefaultPolyhedronPayload>;
template class Polyhedron_Vertex<double, BrushFacePayload, BrushVertexPayload>;

template struct Polyhedron_GetEdgeLink<
  double,
  DefaultPolyhedronPayload,
  DefaultPolyhedronPayload>;
template struct Polyhedron_GetEdgeLink<double, BrushFacePayload, BrushVertexPayload>;

template class Polyhedron_Edge<
  double,
  DefaultPolyhedronPayload,
  DefaultPolyhedronPayload>;
template class Polyhedron_Edge<double, BrushFacePayload, BrushVertexPayload>;

template struct Polyhedron_GetHalfEdgeLink<
  double,
  DefaultPolyhedronPayload,
  DefaultPolyhedronPayload>;
template struct Polyhedron_GetHalfEdgeLink<double, BrushFacePayload, BrushVertexPayload>;

template class Polyhedron_HalfEdge<
  double,
  DefaultPolyhedronPayload,
  DefaultPolyhedronPayload>;
template class Polyhedron_HalfEdge<double, BrushFacePayload, BrushVertexPayload>;

template struct Polyhedron_GetFaceLink<
  double,
  DefaultPolyhedronPayload,
  DefaultPolyhedronPayload>;
template struct Polyhedron_GetFaceLink<double, BrushFacePayload, BrushVertexPayload>;

template class Polyhedron_Face<
  double,
  DefaultPolyhedronPayload,
  DefaultPolyhedronPayload>;
template class Polyhedron_Face<double, BrushFacePayload, BrushVertexPayload>;

template class Polyhedron<double, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
template class Polyhedron<double, BrushFacePayload, BrushVertexPayload>;

} // namespace tb::mdl
