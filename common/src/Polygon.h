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
#include "vec_type.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

namespace TrenchBroom {
    template <typename T, size_t S>
    class Polygon {
    public:
        using Type = T;
        static const size_t Size = S;
        using List = std::vector<Polygon<T,S>>;
        using float_type = Polygon<float, S>;
    private:
        typename vec<T,S>::List m_vertices;
    public:
        Polygon() {}

        Polygon(std::initializer_list<vec<T,S>> i_vertices) :
        m_vertices(i_vertices) {
            CollectionUtils::rotateMinToFront(m_vertices);
        }

        Polygon(const typename vec<T,S>::List& i_vertices) :
        m_vertices(i_vertices) {
            CollectionUtils::rotateMinToFront(m_vertices);
        }

        Polygon(typename vec<T,S>::List& i_vertices) {
            using std::swap;
            swap(m_vertices, i_vertices);
            CollectionUtils::rotateMinToFront(m_vertices);
        }

        // Copy and move constructors
        Polygon(const Polygon<T,S>& other) = default;
        Polygon(Polygon<T,S>&& other) = default;

        // Assignment operators
        Polygon<T,S>& operator=(const Polygon<T,S>& other) = default;
        Polygon<T,S>& operator=(Polygon<T,S>&& other) = default;

        // Conversion constructors
        template <typename U>
        Polygon(const Polygon<U,S>& other) {
            m_vertices.reserve(other.vertexCount());
            for (const auto& vertex : other.vertices()) {
                m_vertices.push_back(vec<T,S>(vertex));
            }
        }

        bool operator==(const Polygon<T,S>& rhs) const {
            return compare(*this, rhs, T(0.0)) == 0;
        }
        
        bool operator!=(const Polygon<T,S>& rhs) const {
            return compare(*this, rhs, T(0.0)) != 0;
        }
        
        bool operator<(const Polygon<T,S>& rhs) const {
            return compare(*this, rhs, T(0.0)) < 0;
        }

        bool operator<=(const Polygon<T,S>& rhs) const {
            return compare(*this, rhs, T(0.0)) <= 0;
        }

        bool operator>(const Polygon<T,S>& rhs) const {
            return compare(*this, rhs, T(0.0)) > 0;
        }

        bool operator>=(const Polygon<T,S>& rhs) const {
            return compare(*this, rhs, T(0.0)) >= 0;
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
            for (size_t i = 1; i < m_vertices.size(); ++i)
                center += m_vertices[i];
            return center / static_cast<T>(m_vertices.size());
        }

        static typename vec<T,S>::List asVertexList(const typename Polygon<T,S>::List& polygons) {
            typename vec<T,S>::List result;
            for (const auto& polygon : polygons) {
                VectorUtils::append(result, polygon.m_vertices);
            }
            return result;
        }

        Polygon<T,S> inverted() const {
            Polygon<T,S> result(*this);
            return result.invert();
        }

        Polygon<T,S>& invert() {
            if (m_vertices.size() > 1) {
                std::reverse(std::next(std::begin(m_vertices)), std::end(m_vertices));
            }
            return *this;
        }
        
        Polygon<T,S> transformed(const mat<T,S+1,S+1>& mat) const {
            return Polygon<T,S>(mat * vertices());
        }
    public:
        friend Polygon<T,S> translate(const Polygon<T,S>& polygon, const vec<T,S>& offset) {
            return Polygon<T,S>(polygon.vertices() + offset);
        }
    };

    typedef Polygon<float,2> Polygon2f;
    typedef Polygon<double,2> Polygon2d;
    typedef Polygon<float,3> Polygon3f;
    typedef Polygon<double,3> Polygon3d;

    template <typename T, size_t S>
    int compare(const Polygon<T,S>& lhs, const Polygon<T,S>& rhs, const T epsilon = static_cast<T>(0.0)) {
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
    int compareUnoriented(const Polygon<T,S>& lhs, const Polygon<T,S>& rhs, const T epsilon = static_cast<T>(0.0)) {
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
    T squaredDistance(const Polygon<T,S>& lhs, const Polygon<T,S>& rhs) {
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
}

#endif
