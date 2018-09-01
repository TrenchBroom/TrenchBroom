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

#ifndef TrenchBroom_CoordinatePlane_h
#define TrenchBroom_CoordinatePlane_h

#include "MathUtils.h"
#include "vec_decl.h"

template <typename T, size_t S>
class CoordinatePlane {
private:
    size_t m_axis;
public:
    CoordinatePlane(const size_t axis) :
    m_axis(axis) {}
    
    CoordinatePlane(const vec<T,S>& axis) :
    m_axis(axis.firstComponent()) {}
    
    vec<T,S> project(const vec<T,S>& point) const {
        vec<T,S> result = point;
        result[m_axis] = static_cast<T>(0.0);
        return result;
    }
};

template <typename T>
vec<T,3> swizzle(const vec<T,3>& point, const size_t axis) {
    assert(axis <= 3);
    switch (axis) {
        case 0:
            return vec<T,3>(point.y(), point.z(), point.x());
        case 1:
            return vec<T,3>(point.z(), point.x(), point.y());
        default:
            return point;
    }
}

template <typename T>
vec<T,3> unswizzle(const vec<T,3>& point, const size_t axis) {
    assert(axis <= 3);
    switch (axis) {
        case 0:
            return vec<T,3>(point.z(), point.x(), point.y());
        case 1:
            return vec<T,3>(point.y(), point.z(), point.x());
        default:
            return point;
    }
}

#endif
