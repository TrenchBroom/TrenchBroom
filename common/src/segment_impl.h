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

#ifndef TRENCHBROOM_SEGMENT_IMPL_H
#define TRENCHBROOM_SEGMENT_IMPL_H

#include "segment_decl.h"
#include "vec_decl.h"
#include "vec_impl.h"
#include "mat_decl.h"
#include "mat_impl.h"

#include <cstddef>

template <typename T, size_t S>
segment<T,S>::segment() {}

template <typename T, size_t S>
segment<T,S>::segment(const vec<T,S>& p1, const vec<T,S>& p2) :
m_start(p1),
m_end(p2) {
    if (m_end < m_start) {
        flip();
    }
}

template <typename T, size_t S>
vec<T,S> segment<T,S>::getOrigin() const {
    return m_start;
}

template <typename T, size_t S>
vec<T,S> segment<T,S>::getDirection() const {
    return direction();
}

template <typename T, size_t S>
segment<T,S> segment<T,S>::transform(const mat<T,S,S>& transform) const {
    return segment<T,S>(m_start * transform, m_end * transform);
}

template <typename T, size_t S>
const vec<T,S>& segment<T,S>::start() const {
    return m_start;
}

template <typename T, size_t S>
const vec<T,S>& segment<T,S>::end() const {
    return m_end;
}

template <typename T, size_t S>
vec<T,S> segment<T,S>::center() const {
    return (m_start + m_end) / static_cast<T>(2.0);
}

template <typename T, size_t S>
vec<T,S> segment<T,S>::direction() const {
    return normalize(m_end - m_start);
}

template <typename T, size_t S>
int compare(const segment<T,S>& lhs, const segment<T,S>& rhs, const T epsilon) {
    const int startCmp = compare(lhs.start(), rhs.start(), epsilon);
    if (startCmp < 0) {
        return -1;
    } else if (startCmp > 0) {
        return 1;
    } else {
        return compare(lhs.end(), rhs.end(), epsilon);
    }
}

template <typename T, size_t S>
bool operator==(const segment<T,S>& lhs, const segment<T,S>& rhs) {
    return compare(lhs, rhs, T(0.0)) == 0;
}

template <typename T, size_t S>
bool operator!=(const segment<T,S>& lhs, const segment<T,S>& rhs) {
    return compare(lhs, rhs, T(0.0)) != 0;
}

template <typename T, size_t S>
bool operator<(const segment<T,S>& lhs, const segment<T,S>& rhs) {
    return compare(lhs, rhs, T(0.0)) < 0;
}

template <typename T, size_t S>
bool operator<=(const segment<T,S>& lhs, const segment<T,S>& rhs) {
    return compare(lhs, rhs, T(0.0)) <= 0;
}

template <typename T, size_t S>
bool operator>(const segment<T,S>& lhs, const segment<T,S>& rhs) {
    return compare(lhs, rhs, T(0.0)) > 0;
}

template <typename T, size_t S>
bool operator>=(const segment<T,S>& lhs, const segment<T,S>& rhs) {
    return compare(lhs, rhs, T(0.0)) >= 0;
}

template <typename T, size_t S>
segment<T,S> translate(const segment<T,S>& s, const vec<T,S>& offset) {
    return segment<T,S>(s.start() + offset, s.end() + offset);
}

#endif //TRENCHBROOM_SEGMENT_IMPL_H
