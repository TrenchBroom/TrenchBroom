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

#include "Vec.h"

template <typename T, size_t S>
class Line {
public:
    Vec<T,S> point;
    Vec<T,S> direction;

    Line() :
    point(Vec<T,S>::Null),
    direction(Vec<T,S>::Null) {}
    
    Line(const Vec<T,S>& i_point, const Vec<T,S>& i_direction) :
    point(i_point),
    direction(i_direction) {}
    
    inline const Vec<T,S> pointAtDistance(const T distance) const {
        return point + direction * distance;
    }
};

typedef Line<float,3> Line3f;
typedef Line<double,3> Line3d;

#endif
