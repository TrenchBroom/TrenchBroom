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

#ifndef TRENCHBROOM_POLYGON_IMPL_H
#define TRENCHBROOM_POLYGON_IMPL_H

#include "polygon_decl.h"

#include "Algorithms.h"
#include "CollectionUtils.h"

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <vector>

namespace vm {
    template <typename T, size_t S>
    polygon<T,S>::polygon() = default;

    template <typename T, size_t S>
    polygon<T,S>::polygon(std::initializer_list<vec<T,S>> i_vertices) :
    m_vertices(i_vertices) {
        CollectionUtils::rotateMinToFront(m_vertices);
    }

    template <typename T, size_t S>
    polygon<T,S>::polygon(const typename vec<T,S>::List& i_vertices) :
    m_vertices(i_vertices) {
        CollectionUtils::rotateMinToFront(m_vertices);
    }

    template <typename T, size_t S>
    polygon<T,S>::polygon(typename vec<T,S>::List&& i_vertices) :
    m_vertices(i_vertices) {
        CollectionUtils::rotateMinToFront(m_vertices);
    }

    template <typename T, size_t S>
    bool polygon<T,S>::hasVertex(const vec<T,S>& vertex) const {
        return std::find(std::begin(m_vertices), std::end(m_vertices), vertex) != std::end(m_vertices);
    }

    template <typename T, size_t S>
    bool polygon<T,S>::contains(const vec<T,S>& point, const vec<T,3>& normal) const {
        return polygonContainsPoint(point, normal, std::begin(m_vertices), std::end(m_vertices));
    }

    template <typename T, size_t S>
    size_t polygon<T,S>::vertexCount() const {
        return m_vertices.size();
    }

    template <typename T, size_t S>
    typename vec<T,3>::List::const_iterator polygon<T,S>::begin() const {
        return std::begin(m_vertices);
    }

    template <typename T, size_t S>
    typename vec<T,3>::List::const_iterator polygon<T,S>::end() const {
        return std::end(m_vertices);
    }

    template <typename T, size_t S>
    const typename vec<T,S>::List& polygon<T,S>::   vertices() const {
        return m_vertices;
    }

    template <typename T, size_t S>
    vec<T,S> polygon<T,S>::center() const {
        return std::accumulate(std::begin(m_vertices), std::end(m_vertices), vec<T,S>::zero) / static_cast<T>(m_vertices.size());
    }

    template <typename T, size_t S>
    polygon<T,S> polygon<T,S>::invert() const {
        auto vertices = m_vertices;
        if (vertices.size() > 1) {
            std::reverse(std::next(std::begin(vertices)), std::end(vertices));
        }
        return polygon<T,S>(vertices);
    }

    template <typename T, size_t S>
    polygon<T,S> polygon<T,S>::translate(const vec<T,S>& offset) const {
        return polygon<T,S>(m_vertices + offset);
    }

    template <typename T, size_t S>
    polygon<T,S> polygon<T,S>::transform(const mat<T,S+1,S+1>& mat) const {
        return polygon<T,S>(mat * vertices());
    }

    template <typename T, size_t S>
    int compare(const polygon<T,S>& lhs, const polygon<T,S>& rhs, const T epsilon) {
        const auto& lhsVerts = lhs.vertices();
        const auto& rhsVerts = rhs.vertices();

        if (lhsVerts.size() < rhsVerts.size()) {
            return -1;
        } else if (lhsVerts.size() > rhsVerts.size()) {
            return 1;
        } else {
            return compare(std::begin(lhsVerts), std::end(lhsVerts),
                           std::begin(rhsVerts), std::end(rhsVerts), epsilon);
        }
    }

    template <typename T, size_t S>
    bool operator==(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
        return compare(lhs, rhs, T(0.0)) == 0;
    }

    template <typename T, size_t S>
    bool operator!=(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
        return compare(lhs, rhs, T(0.0)) != 0;
    }

    template <typename T, size_t S>
    bool operator<(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
        return compare(lhs, rhs, T(0.0)) < 0;
    }

    template <typename T, size_t S>
    bool operator<=(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
        return compare(lhs, rhs, T(0.0)) <= 0;
    }

    template <typename T, size_t S>
    bool operator>(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
        return compare(lhs, rhs, T(0.0)) > 0;
    }

    template <typename T, size_t S>
    bool operator>=(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
        return compare(lhs, rhs, T(0.0)) >= 0;
    }

    template <typename T, size_t S>
    int compareUnoriented(const polygon<T,S>& lhs, const polygon<T,S>& rhs, const T epsilon) {
        const auto& lhsVerts = lhs.vertices();
        const auto& rhsVerts = rhs.vertices();

        if (lhsVerts.size() < rhsVerts.size()) {
            return -1;
        } else if (lhsVerts.size() > rhsVerts.size()) {
            return 1;
        } else {
            const auto count = lhsVerts.size();
            if (count == 0) {
                return 0;
            }

            // Compare first:
            const auto cmp0 = compare(lhsVerts[0], rhsVerts[0], epsilon);
            if (cmp0 < 0) {
                return -1;
            } else if (cmp0 > 0) {
                return +1;
            }

            if (count == 1) {
                return 0;
            }

            // First vertices are identical. Now compare my second with other's second.
            auto cmp1 = compare(lhsVerts[1], rhsVerts[1], epsilon);
            if (cmp1 == 0) {
                // The second vertices are also identical, so we just do a forward compare.
                return compare(std::next(std::begin(lhsVerts), 2), std::end(lhsVerts),
                               std::next(std::begin(rhsVerts), 2), std::end(rhsVerts), epsilon);
            } else {
                // The second vertices are not identical, so we attemp a backward compare.
                size_t i = 1;
                while (i < count) {
                    const auto j = count - i;
                    const auto cmp = compare(lhsVerts[i], rhsVerts[j], epsilon);
                    if (cmp != 0) {
                        // Backward compare failed, so make a forward compare
                        return compare(std::next(std::begin(lhsVerts), 2), std::end(lhsVerts),
                                       std::next(std::begin(rhsVerts), 2), std::end(rhsVerts), epsilon);
                    }
                    ++i;
                }
                return 0;
            }
        }
    }
}

#endif //TRENCHBROOM_POLYGON_IMPL_H
