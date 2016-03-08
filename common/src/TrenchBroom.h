#/*
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

#ifndef TrenchBroom_TrenchBroom_h
#define TrenchBroom_TrenchBroom_h

#include "VecMath.h"
#include "Polyhedron_DefaultPayload.h"

typedef double FloatType;
typedef BBox<FloatType, 2> BBox2;
typedef BBox<FloatType, 3> BBox3;
typedef Vec<FloatType, 3> Vec3;
typedef Vec<FloatType, 2> Vec2;
typedef Plane<FloatType, 3> Plane3;
typedef Quat<FloatType> Quat3;
typedef Mat<FloatType, 4, 4> Mat4x4;
typedef Mat<FloatType, 3, 3> Mat3x3;
typedef Mat<FloatType, 2, 2> Mat2x2;
typedef Line<FloatType, 3> Line3;
typedef Ray<FloatType, 3> Ray3;
typedef CoordinatePlane<FloatType, 3> CoordinatePlane3;

template<typename T, typename FP, typename VB> class Polyhedron;
typedef Polyhedron<FloatType, DefaultPolyhedronPayload, DefaultPolyhedronPayload> Polyhedron3;

namespace TrenchBroom {
    typedef Edge<FloatType, 3> Edge3;
    typedef Polygon<FloatType, 3> Polygon3;
}

namespace Math {
    typedef Constants<FloatType> C;
}

template <typename T>
void safeDelete(T*& p) {
    delete p;
    p = NULL;
}

#endif
