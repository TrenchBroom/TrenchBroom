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
#include "mat_decl.h"
#include "mat_impl.h"

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
vec<T,S> line<T,S>::getOrigin() const {
    return point;
}

template <typename T, size_t S>
vec<T,S> line<T,S>::getDirection() const {
    return direction;
}

template <typename T, size_t S>
line<T,S> line<T,S>::transform(const mat<T,S,S>& transform) const {
    const auto newPoint = point * transform;
    const auto newDirection = point * stripTranslation(transform);
}

template <typename T, size_t S>
line<T,S> line<T,S>::makeCanonical() const {
    // choose the point such that its support vector is orthogonal to
    // the direction of this line
    const auto distance = point.dot(direction);
    const auto newPoint = (point - distance * direction);

    // make sure the first nonzero component of the direction is positive
    auto newDirection = direction;
    for (size_t i = 0; i < S; ++i) {
        if (direction[i] != 0.0) {
            if (direction[i] < 0.0) {
                newDirection = -newDirection;
            }
            break;
        }
    }

    return line<T,S>(newPoint, newDirection);
}

template <typename T, size_t S>
bool operator==(const line<T,S>& lhs, const line<T,S>& rhs) {
    const auto lhsC = lhs.makeCanonical();
    const auto rhsC = rhs.makeCanonical();
    return lhsC.point == rhsC.point && lhsC.direction == rhsC.direction;
}

template <typename T, size_t S>
bool operator!=(const line<T,S>& lhs, const line<T,S>& rhs) {
    const auto lhsC = lhs.makeCanonical();
    const auto rhsC = rhs.makeCanonical();
    return lhsC.point != rhsC.point || lhsC.direction != rhsC.direction;
}

template <typename T, size_t S>
std::ostream& operator<<(std::ostream& stream, const line<T,S>& line) {
    stream << "{ point: (" << line.point << "), direction: (" << line.direction << ") }";
    return stream;
}

#endif //TRENCHBROOM_LINE_IMPL_H
