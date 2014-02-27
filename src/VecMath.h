/*
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

#ifndef TrenchBroom_VecMath_h
#define TrenchBroom_VecMath_h

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include "BBox.h"
#include "Line.h"
#include "Mat.h"
#include "Plane.h"
#include "Quat.h"
#include "Vec.h"
#include "CoordinatePlane.h"
#include "Edge.h"
#include "Polygon.h"

template <typename TT>
const TT intersectLineWithTriangle(const Vec<TT,3>& O, const Vec<TT,3>& D, const Vec<TT,3>& V0, const Vec<TT,3>& V1, const Vec<TT,3>& V2) {
    // see http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
    
    const Vec<TT,3>  E1 = V1 - V0;
    const Vec<TT,3>  E2 = V2 - V0;
    const Vec<TT,3>  P  = crossed(D, E2);
    const TT         a  = P.dot(E1);
    if (Math::zero(a))
        return Math::nan<TT>();
    
    const Vec<TT,3>  T  = O - V0;
    const Vec<TT,3>  Q  = crossed(T, E1);
    
    const TT t = Q.dot(E2) / a;
    const TT u = P.dot(T) / a;
    if (Math::neg(u))
        return Math::nan<TT>();
    
    const TT v = Q.dot(D) / a;
    if (Math::neg(v))
        return Math::nan<TT>();
    
    if (Math::gt(u+v, static_cast<TT>(1.0)))
        return Math::nan<TT>();
    
    return t;
}

template <typename TT>
const TT intersectRayWithTriangle(const Vec<TT,3>& O, const Vec<TT,3>& D, const Vec<TT,3>& V0, const Vec<TT,3>& V1, const Vec<TT,3>& V2) {
    const TT distance = intersectLineWithTriangle(O, D, V0, V1, V2);
    if (Math::neg(distance))
        return Math::nan<TT>();
    return distance;
}

#endif
