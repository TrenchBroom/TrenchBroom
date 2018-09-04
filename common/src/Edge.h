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

#ifndef TrenchBroom_Edge_h
#define TrenchBroom_Edge_h

#include "vec_decl.h"

#include <vector>

// TODO 2201: Rename to segment
namespace TrenchBroom {
    template <typename T, size_t S>
    class Edge {
    public:
        using Type = T;
        static const size_t Size = S;
        using float_type = Edge<float, S>;
        using List = std::vector<Edge<T,S>>;
    private:
        vec<T,S> m_start;
        vec<T,S> m_end;
    public:
        Edge() {}

        Edge(const vec<T,S>& i_start, const vec<T,S>& i_end) :
        m_start(i_start),
        m_end(i_end) {
            if (m_end < m_start)
                flip();
        }

        // Copy and move constructors
        Edge(const Edge<T,S>& other) = default;
        Edge(Edge<T,S>&& other) = default;

        // Assignment operators
        Edge<T,S>& operator=(const Edge<T,S>& other) = default;
        Edge<T,S>& operator=(Edge<T,S>&& other) = default;

        // Conversion constructors
        template <typename U>
        Edge(const Edge<U,S>& other) :
        m_start(other.start()),
        m_end(other.end()) {}

        bool operator==(const Edge<T,S>& other) const {
            return compare(*this, other, T(0.0)) == 0;
        }

        bool operator!=(const Edge<T,S>& other) const {
            return compare(*this, other, T(0.0)) != 0;
        }

        bool operator<(const Edge<T,S>& other) const {
            return compare(*this, other, T(0.0)) < 0;
        }

        bool operator<=(const Edge<T,S>& other) const {
            return compare(*this, other, T(0.0)) <= 0;
        }

        bool operator>(const Edge<T,S>& other) const {
            return compare(*this, other, T(0.0)) > 0;
        }

        bool operator>=(const Edge<T,S>& other) const {
            return compare(*this, other, T(0.0)) >= 0;
        }

        const vec<T,S>& start() const {
            return m_start;
        }

        const vec<T,S>& end() const {
            return m_end;
        }

        vec<T,S> center() const {
            return (m_start + m_end) / static_cast<T>(2.0);
        }

        vec<T,S> direction() const {
            return normalize(m_end - m_start);
        }

        vec<T,S> pointAtDistance(const T distance) const {
            return m_start + distance * direction();
        }

        static typename vec<T,S>::List asVertexList(const typename Edge<T,S>::List& edges) {
            typename vec<T,S>::List result;
            result.reserve(2 * edges.size());
            for (size_t i = 0; i < edges.size(); ++i) {
                result.push_back(edges[i].start());
                result.push_back(edges[i].end());
            }
            return result;
        }
    public:
        friend Edge<T,S> translate(const Edge<T,S>& edge, const vec<T,S>& offset) {
            return Edge<T,S>(edge.m_start + offset, edge.m_end + offset);
        }
    private:
        void flip() {
            using std::swap;
            swap(m_start, m_end);
        }
    };

    typedef Edge<double, 3> Edge3d;
    typedef Edge<float, 3> Edge3f;
    typedef Edge<double, 2> Edge2d;
    typedef Edge<float, 2> Edge2f;

    template <typename T, size_t S>
    int compare(const Edge<T,S>& lhs, const Edge<T,S>& rhs, const T epsilon = static_cast<T>(0.0)) {
        const int startCmp = compare(lhs.start(), rhs.start(), epsilon);
        if (startCmp < 0)
            return -1;
        if (startCmp > 0)
            return 1;
        return compare(lhs.end(), rhs.end(), epsilon);
    }


    template <typename T, size_t S>
    T squaredDistance(const Edge<T,S>& lhs, const Edge<T,S>& rhs) {
        const auto startDistance = squaredDistance(lhs.start(), rhs.start());
        const auto endDistance = squaredDistance(lhs.end(), rhs.end());
        return Math::max(startDistance, endDistance);
    }
}

#endif
