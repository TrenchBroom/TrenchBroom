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

#ifndef TRENCHBROOM_LINE_IMPL_H
#define TRENCHBROOM_LINE_IMPL_H

#include "line_decl.h"
#include "vec_decl.h"
#include "vec_impl.h"

template <typename T, size_t S>
const typename line<T,S>::List line<T,S>::EmptyList = line<T,S>::List();

template <typename T, size_t S>
line<T,S>::line() :
point(vec<T,S>::zero),
direction(vec<T,S>::zero) {}

template <typename T, size_t S>
line<T,S>::line(const vec<T,S>& i_point, const vec<T,S>& i_direction) :
point(i_point),
direction(i_direction) {}

template <typename T, size_t S>
T line<T,S>::distance(const vec<T,S>& i_point) const {
    return dot(i_point - point, direction);
}

template <typename T, size_t S>
vec<T,S> line<T,S>::pointAtDistance(const T distance) const {
    return point + direction * distance;
}

template <typename T, size_t S>
vec<T,S> line<T,S>::project(const vec<T,S>& i_point) const {
    return pointAtDistance(distance(i_point));
}

template <typename T, size_t S>
std::ostream& operator<<(std::ostream& stream, const line<T,S>& line) {
    stream << "{ point: (" << line.point << "), direction: (" << line.direction << ") }";
    return stream;
}

#endif //TRENCHBROOM_LINE_IMPL_H
