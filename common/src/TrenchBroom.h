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

#ifndef TrenchBroom_TrenchBroom_h
#define TrenchBroom_TrenchBroom_h

#include "VecMath.h"

#include "Polyhedron.h"
#include "Polyhedron_BrushGeometryPayload.h"
#include "Polyhedron_DefaultPayload.h"

using FloatType = double;
using BBox3 = BBox<FloatType, 3>;
using BBox2 = BBox<FloatType, 2>;
using vec3 = vec<FloatType, 3>;
using vec2 = vec<FloatType, 2>;
using Plane3 = Plane<FloatType, 3>;
using Quat3 = Quat<FloatType>;
using mat4x4 = mat<FloatType, 4, 4>;
using Line3 = Line<FloatType, 3>;
using Ray3 = Ray<FloatType, 3>;

#include "Polyhedron_Instantiation.h"

template<typename T, typename FP, typename VB> class Polyhedron;
using Polyhedron3 = Polyhedron<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;

namespace TrenchBroom {
    using Edge3 = Edge<FloatType, 3>;
    using Polygon3 = Polygon<FloatType, 3>;
}

namespace Math {
    using C = Constants<FloatType>;
}

#endif
