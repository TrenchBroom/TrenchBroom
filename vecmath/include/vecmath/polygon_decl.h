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

#include "forward.h"
#include "vec_decl.h"

#include <cstddef>

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
        polygon();

        /**
         * Creates a new polygon with the given vertices. The given points are assumed to form a convex polygon.
         *
         *  @param i_vertices the vertices
         */
        polygon(std::initializer_list<vec<T,S>> i_vertices);

        /**
         * Creates a new polygon with the given vertices. The given points are assumed to form a convex polygon.
         *
         * @param i_vertices the vertices
         */
        polygon(const typename vec<T,S>::List& i_vertices);

        /**
         * Creates a new polygon with the given vertices. The given points are assumed to form a convex polygon.
         *
         * @param i_vertices the vertices
         */
        polygon(typename vec<T,S>::List&& i_vertices);

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
        bool hasVertex(const vec<T,S>& vertex) const;

        /**
         * Checks whether this polygon contains the given point.
         *
         * @param point the point to check
         * @param normal the normal of this polygon
         * @return true if this polygon contains the given point, and false otherwise
         */
        bool contains(const vec<T,S>& point, const vec<T,3>& normal) const;

        /**
         * Returns the number of vertices of this polygon.
         *
         * @return the number of vertices
         */
        size_t vertexCount() const;

        /**
         * Returns an iterator to the beginning of the vertices.
         *
         * @return an iterator to the beginning of the vertices
         */
        typename vec<T,3>::List::const_iterator begin() const;

        /**
         * Returns an iterator to the end of the vertices.
         *
         * @return an iterator to the end of the vertices
         */
        typename vec<T,3>::List::const_iterator end() const;

        /**
         * Returns the vertices of this polygon.
         *
         * @return the vertices
         */
        const typename vec<T,S>::List& vertices() const;

        /**
         * Computes the center of this polygon.
         *
         * @return the center of this polygon
         */
        vec<T,S> center() const;

        /**
         * Inverts this polygon by reversing its vertices.
         *
         * @return the inverted polygon
         */
        polygon<T,S> invert() const;

        /**
         * Translates this polygon by the given offset.
         *
         * @param offset the offset by which to translate
         * @return the translated polygon
         */
        polygon<T,S> translate(const vec<T,S>& offset) const;

        /**
         * Transforms this polygon using the given transformation matrix.
         *
         * @param mat the transformation to apply
         * @return the transformed polygon
         */
        polygon<T,S> transform(const mat<T,S+1,S+1>& mat) const;

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
    int compare(const polygon<T,S>& lhs, const polygon<T,S>& rhs, T epsilon = static_cast<T>(0.0));

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
    bool operator==(const polygon<T,S>& lhs, const polygon<T,S>& rhs);

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
    bool operator!=(const polygon<T,S>& lhs, const polygon<T,S>& rhs);

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
    bool operator<(const polygon<T,S>& lhs, const polygon<T,S>& rhs);

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
    bool operator<=(const polygon<T,S>& lhs, const polygon<T,S>& rhs);

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
    bool operator>(const polygon<T,S>& lhs, const polygon<T,S>& rhs);

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
    bool operator>=(const polygon<T,S>& lhs, const polygon<T,S>& rhs);

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
    int compareUnoriented(const polygon<T,S>& lhs, const polygon<T,S>& rhs, T epsilon = static_cast<T>(0.0));
}

#endif
