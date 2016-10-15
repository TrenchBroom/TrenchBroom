/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "TrenchBroom.h"
#include "Polyhedron.h"
#include "Polyhedron_Misc.h"
#include "Polyhedron_Vertex.h"
#include "Polyhedron_Edge.h"
#include "Polyhedron_HalfEdge.h"
#include "Polyhedron_Face.h"
#include "Polyhedron_VertexManipulation.h"
#include "Polyhedron_ConvexHull.h"
#include "Polyhedron_Clip.h"
#include "Polyhedron_Subtract.h"
#include "Polyhedron_Intersect.h"
#include "Polyhedron_Queries.h"
#include "Polyhedron_BrushGeometryPayload.h"
#include "Polyhedron_DefaultPayload.h"

template class Polyhedron<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;
template class Polyhedron<FloatType, BrushFacePayload, BrushVertexPayload>;
