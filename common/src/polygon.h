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

#ifndef TrenchBroom_Polygon_h
#define TrenchBroom_Polygon_h

#include "Algorithms.h"
#include "CollectionUtils.h"
#include "VecMath.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

template <typename T, size_t S>
class polygon {
public:
    using Type = T;
    static const size_t Size = S;
    using List = std::vector<polygon<T,S>>;
    using float_type = polygon<float, S>;
private:
    typename vec<T,S>::List m_vertices;
public:
    /**
     * Creates a new empty polygon.
     */
    polygon() {}

    /**
     * Creates a new polygon with the given vertices. The given points are assumed to form a convex polygon.
     *
     *  @param i_vertices the vertices
     */
    polygon(std::initializer_list<vec<T,S>> i_vertices) :
    m_vertices(i_vertices) {
        CollectionUtils::rotateMinToFront(m_vertices);
    }

    /**
     * Creates a new polygon with the given vertices. The given points are assumed to form a convex polygon.
     *
     * @param i_vertices the vertices
     */
    polygon(const typename vec<T,S>::List& i_vertices) :
    m_vertices(i_vertices) {
        CollectionUtils::rotateMinToFront(m_vertices);
    }

    /**
     * Creates a new polygon with the given vertices. The given points are assumed to form a convex polygon.
     *
     * @param i_vertices the vertices
     */
    polygon(typename vec<T,S>::List&& i_vertices) :
    m_vertices(i_vertices) {
        CollectionUtils::rotateMinToFront(m_vertices);
    }

    // Copy and move constructors
    polygon(const polygon<T,S>& other) = default;
    polygon(polygon<T,S>&& other) noexcept = default;

    // Assignment operators
    polygon<T,S>& operator=(const polygon<T,S>& other) = default;
    polygon<T,S>& operator=(polygon<T,S>&& other) noexcept = default;

    /**
     * Creates a new polygon by copying the values from the given polygon. If the given polygon has a different component
     * type, the values are converted using static_cast.
     *
     * @tparam U the component type of the given polygon
     * @param other the polygon to copy the values from
     */
    template <typename U>
    polygon(const polygon<U,S>& other) {
        m_vertices.reserve(other.vertexCount());
        for (const auto& vertex : other.vertices()) {
            m_vertices.push_back(vec<T,S>(vertex));
        }
    }
public:
    bool hasVertex(const vec<T,S>& vertex) const {
        return std::find(std::begin(m_vertices), std::end(m_vertices), vertex) != std::end(m_vertices);
    }

    bool contains(const vec<T,S>& point, const vec<T,3>& normal) const {
        return polygonContainsPoint(point, normal, std::begin(m_vertices), std::end(m_vertices));
    }

    size_t vertexCount() const {
        return m_vertices.size();
    }

    typename vec<T,3>::List::const_iterator begin() const {
        return std::begin(m_vertices);
    }

    typename vec<T,3>::List::const_iterator end() const {
        return std::end(m_vertices);
    }

    const typename vec<T,S>::List& vertices() const {
        return m_vertices;
    }

    vec<T,S> center() const {
        assert(!m_vertices.empty());
        vec<T,S> center = m_vertices[0];
        for (size_t i = 1; i < m_vertices.size(); ++i) {
            center = center + m_vertices[i];
        }
        return center / static_cast<T>(m_vertices.size());
    }

    static typename vec<T,S>::List asVertexList(const typename polygon<T,S>::List& polygons) {
        typename vec<T,S>::List result;
        for (const auto& polygon : polygons) {
            VectorUtils::append(result, polygon.m_vertices);
        }
        return result;
    }

    polygon<T,S> inverted() const {
        polygon<T,S> result(*this);
        return result.invert();
    }

    polygon<T,S>& invert() {
        if (m_vertices.size() > 1) {
            std::reverse(std::next(std::begin(m_vertices)), std::end(m_vertices));
        }
        return *this;
    }

    polygon<T,S> transformed(const mat<T,S+1,S+1>& mat) const {
        return polygon<T,S>(mat * vertices());
    }
public:
    friend polygon<T,S> translate(const polygon<T,S>& p, const vec<T,S>& offset) {
        return polygon<T,S>(p.vertices() + offset);
    }
};

using polygon2f = polygon<float,2>;
using polygon2d = polygon<double,2>;
using polygon3f = polygon<float,3>;
using polygon3d = polygon<double,3>;

/**
 * Compares the given polygons under the assumption that the first vertex of each polygon is the smallest of all
 * vertices of that polygon. A polygon is considered less than another polygon if is has fewer vertices, and vice
 * versa. If both polygons have the same number of vertices, the vertices are compared lexicographically until a pair
 * of vertices is found which are not identical. The result of comparing these vertices is returned. If no such pair
 * exists, 0 is returned.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @param epsilon an epsilon value
 * @return -1 if the first polygon is less than the second polygon, +1 in the opposite case, and 0 otherwise
 */
template <typename T, size_t S>
int compare(const polygon<T,S>& lhs, const polygon<T,S>& rhs, const T epsilon = static_cast<T>(0.0)) {
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

/**
 * Checks whether the first given polygon is identical to the second polygon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @return true if the polygons are identical and false otherwise
 */
template <typename T, size_t S>
bool operator==(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
    return compare(lhs, rhs, T(0.0)) == 0;
}

/**
 * Checks whether the first given polygon is identical to the second polygon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @return false if the polygons are identical and true otherwise
 */
template <typename T, size_t S>
bool operator!=(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
    return compare(lhs, rhs, T(0.0)) != 0;
}

/**
 * Checks whether the first given polygon is less than the second polygon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @return true if the first polygon is less than the second polygon and false otherwise
 */
template <typename T, size_t S>
bool operator<(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
    return compare(lhs, rhs, T(0.0)) < 0;
}

/**
 * Checks whether the first given polygon is less than or equal to the second polygon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @return true if the first polygon is less than or equal to the second polygon and false otherwise
 */
 template <typename T, size_t S>
bool operator<=(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
    return compare(lhs, rhs, T(0.0)) <= 0;
}

/**
 * Checks whether the first given polygon is greater than the second polygon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @return true if the first polygon is greater than the second polygon and false otherwise
 */
template <typename T, size_t S>
bool operator>(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
    return compare(lhs, rhs, T(0.0)) > 0;
}

/**
 * Checks whether the first given polygon is greater than or equal to the second polygon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @return true if the first polygon is greater than or equal to the second polygon and false otherwise
 */
template <typename T, size_t S>
bool operator>=(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
    return compare(lhs, rhs, T(0.0)) >= 0;
}

/**
 * Compares the given polygons under the assumption that the first vertex of each polygon is the smallest of all
 * vertices of that polygon. A polygon is considered less than another polygon if is has fewer vertices, and vice
 * versa. If both polygons have the same number of vertices, the vertices are compared lexicographically until a pair
 * of vertices is found which are not identical. The result of comparing these vertices is returned. If no such pair
 * exists, 0 is returned.
 *
 * In this comparison, the order of the vertices is relaxed. Two polygons can be identical even if the order of the
 * vertices is reversed.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @param epsilon an epsilon value
 * @return -1 if the first polygon is less than the second polygon, +1 in the opposite case, and 0 otherwise
 */
template <typename T, size_t S>
int compareUnoriented(const polygon<T,S>& lhs, const polygon<T,S>& rhs, const T epsilon = static_cast<T>(0.0)) {
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

template <typename T, size_t S>
T squaredDistance(const polygon<T,S>& lhs, const polygon<T,S>& rhs) {
    const auto& lhsVertices = lhs.vertices();
    const auto& rhsVertices = rhs.vertices();

    if (lhsVertices.size() != rhsVertices.size()) {
        return std::numeric_limits<T>::max();
    }

    auto maxDistance = static_cast<T>(0.0);
    for (size_t i = 0; i < lhsVertices.size(); ++i) {
        maxDistance = std::max(maxDistance, squaredDistance(lhsVertices[i], rhsVertices[i]));
    }

    return maxDistance;
}

#endif
