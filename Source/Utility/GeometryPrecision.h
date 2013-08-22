/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_GeometryPrecision_h
#define TrenchBroom_GeometryPrecision_h

#include "Utility/VecMath.h"

namespace TrenchBroom {
    namespace VecMath {
        // use this type for all geometry computations
        typedef double GeomPrec;
        
        typedef Vec<GeomPrec,2> Vec2g;
        typedef Vec<GeomPrec,3> Vec3g;
        typedef Vec<GeomPrec,4> Vec4g;
        typedef BBox<GeomPrec> BBoxg;
        typedef Ray<GeomPrec> Rayg;
        typedef Line<GeomPrec> Lineg;
        typedef Plane<GeomPrec> Planeg;
        typedef Quat<GeomPrec> Quatg;
        typedef Mat<GeomPrec,2,2> Mat2g;
        typedef Mat<GeomPrec,3,3> Mat3g;
        typedef Mat<GeomPrec,4,4> Mat4g;
    }
}

#endif
