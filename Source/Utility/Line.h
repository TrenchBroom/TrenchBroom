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

#include "Utility/Vec3.h"

namespace TrenchBroom {
    namespace VecMath {
        template <typename T>
        class Line {
        public:
            Vec3<T> point;
            Vec3<T> direction;

            Line() : point(Vec3<T>::Null), direction(Vec3<T>::Null) {}
            
            Line(const Vec3<T>& i_point, const Vec3<T>& i_direction) : point(i_point), direction(i_direction) {}
            
            inline const Vec3<T> pointAtDistance(const T distance) const {
                return Vec3<T>(point.x + direction.x * distance,
                               point.y + direction.y * distance,
                               point.z + direction.z * distance);
            }
        };
        
        typedef Line<float> Linef;
    }
}

#endif
