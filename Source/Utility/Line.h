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

#ifndef TrenchBroom_Line_h
#define TrenchBroom_Line_h

#include "Utility/Vec.h"

namespace TrenchBroom {
    namespace VecMath {
        template <typename T>
        class Line {
        public:
            Vec<T,3> point;
            Vec<T,3> direction;

            Line() : point(Vec<T,3>::Null), direction(Vec<T,3>::Null) {}
            
            Line(const Vec<T,3>& i_point, const Vec<T,3>& i_direction) : point(i_point), direction(i_direction) {}
            
            inline const Vec<T,3> pointAtDistance(const T distance) const {
                return Vec<T,3>(point.x + direction.x * distance,
                               point.y + direction.y * distance,
                               point.z + direction.z * distance);
            }
        };
        
        typedef Line<float> Linef;
        typedef Line<double> Lined;
    }
}

#endif
