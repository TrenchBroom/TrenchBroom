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

#ifndef TRENCHBROOM_POLYGON_DECL_H
#define TRENCHBROOM_POLYGON_DECL_H

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <vector>

namespace vm {
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
        polygon() = default;

        /**
         * Creates a new polygon with the given vertices. The given points are assumed to form a convex polygon.
         *
         *  @param i_vertices the vertices
         */
        polygon(std::initializer_list<vec<T,S>> i_vertices) :
        m_vertices(i_vertices) {
            rotateMinToFront();
        }

        /**
         * Creates a new polygon with the given vertices. The given points are assumed to form a convex polygon.
         *
         * @param i_vertices the vertices
         */
        explicit polygon(const typename vec<T,S>::List& i_vertices) :
        m_vertices(i_vertices) {
            rotateMinToFront();
        }

        /**
         * Creates a new polygon with the given vertices. The given points are assumed to form a convex polygon.
         *
         * @param i_vertices the vertices
         */
        explicit polygon(typename vec<T,S>::List&& i_vertices) :
        m_vertices(i_vertices) {
            rotateMinToFront();
        }
    private:
        void rotateMinToFront() {
            if (!m_vertices.empty()) {
                const auto begin = std::begin(m_vertices);
                const auto end = std::end(m_vertices);
                const auto min = std::min_element(begin, end);
                std::rotate(begin, min, end);
            }
        }
    public:
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
        explicit polygon(const polygon<U,S>& other) {
            m_vertices.reserve(other.vertexCount());
            for (const auto& vertex : other.vertices()) {
                m_vertices.push_back(vec<T,S>(vertex));
            }
        }

        /**
         * Checks whether this polygon has a vertex with the given coordinates.
         *
         * @param vertex the position to check
         * @return bool if this polygon has a vertex at the given position and false otherwise
         */
        bool hasVertex(const vec<T,S>& vertex) const {
            return std::find(std::begin(m_vertices), std::end(m_vertices), vertex) != std::end(m_vertices);
        }

        /**
         * Returns the number of vertices of this polygon.
         *
         * @return the number of vertices
         */
        size_t vertexCount() const {
            return m_vertices.size();
        }

        /**
         * Returns an iterator to the beginning of the vertices.
         *
         * @return an iterator to the beginning of the vertices
         */
        typename vec<T,3>::List::const_iterator begin() const {
            return std::begin(m_vertices);
        }

        /**
         * Returns an iterator to the end of the vertices.
         *
         * @return an iterator to the end of the vertices
         */
        typename vec<T,3>::List::const_iterator end() const {
            return std::end(m_vertices);
        }

        /**
         * Returns the vertices of this polygon.
         *
         * @return the vertices
         */
        const typename vec<T,S>::List& vertices() const {
            return m_vertices;
        }

        /**
         * Computes the center of this polygon.
         *
         * @return the center of this polygon
         */
        vec<T,S> center() const {
            return std::accumulate(std::begin(m_vertices), std::end(m_vertices), vec<T,S>::zero) / static_cast<T>(m_vertices.size());
        }

        /**
         * Inverts this polygon by reversing its vertices.
         *
         * @return the inverted polygon
         */
        polygon<T,S> invert() const{
            auto vertices = m_vertices;
            if (vertices.size() > 1) {
                std::reverse(std::next(std::begin(vertices)), std::end(vertices));
            }
            return polygon<T,S>(vertices);
        }

        /**
         * Translates this polygon by the given offset.
         *
         * @param offset the offset by which to translate
         * @return the translated polygon
         */
        polygon<T,S> translate(const vec<T,S>& offset) const {
            return polygon<T,S>(m_vertices + offset);
        }

        /**
         * Transforms this polygon using the given transformation matrix.
         *
         * @param mat the transformation to apply
         * @return the transformed polygon
         */
        polygon<T,S> transform(const mat<T,S+1,S+1>& mat) const {
            return polygon<T,S>(mat * vertices());
        }

        /**
         * Adds the vertices of the given range of polygons to the given output iterator.
         *
         * @tparam I the range iterator type
         * @tparam O the output iterator type
         * @param cur the range start
         * @param end the range end
         * @param out the output iterator
         */
        template <typename I, typename O>
        static void getVertices(I cur, I end, O out) {
            while (cur != end) {
                const auto& polygon = *cur;
                for (const auto& vertex : polygon) {
                    out = vertex;
                    ++out;
                }
                ++cur;
            }
        }
    };

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
}

#endif
