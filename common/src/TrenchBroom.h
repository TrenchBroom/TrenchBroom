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
using bbox3 = bbox<FloatType, 3>;
using bbox2 = bbox<FloatType, 2>;
using vec3 = vec<FloatType, 3>;
using vec2 = vec<FloatType, 2>;
using plane3 = plane<FloatType, 3>;
using quat3 = quat<FloatType>;
using mat4x4 = mat<FloatType, 4, 4>;
using line3 = line<FloatType, 3>;
using ray3 = ray<FloatType, 3>;
using segment3 = segment<FloatType, 3>;
using polygon3 = polygon<FloatType, 3>;

#include "Polyhedron_Instantiation.h"

template<typename T, typename FP, typename VB> class Polyhedron;
using Polyhedron3 = Polyhedron<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload>;


namespace Math {
    using C = Constants<FloatType>;
}

#endif
