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

#include "Vec.h"

#include <vector>

template <typename T, size_t S>
class Edge {
public:
    typedef Edge<float, S> FloatType;
    typedef std::vector<Edge<T,S> > List;
private:
    Vec<T,S> m_start;
    Vec<T,S> m_end;
public:
    Edge() {}

    Edge(const Vec<T,S>& i_start, const Vec<T,S>& i_end) :
    m_start(i_start),
    m_end(i_end) {
        if (m_end < m_start)
            flip();
    }

    template <typename TT, size_t SS>
    Edge(const Edge<TT,SS>& other) :
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

    T squaredDistanceTo(const Edge<T,S>& other) const {
        const T startDistance = squaredDistance(m_start, other.m_start);
        const T endDistance = squaredDistance(m_end, other.m_end);
        return Math::max(startDistance, endDistance);
    }

    const Vec<T,S>& start() const {
        return m_start;
    }

    const Vec<T,S>& end() const {
        return m_end;
    }

    Vec<T,S> center() const {
        return (m_start + m_end) / static_cast<T>(2.0);
    }

    Vec<T,S> direction() const {
        return (m_end - m_start).normalized();
    }

    Vec<T,S> pointAtDistance(const T distance) const {
        return m_start + distance * direction();
    }

    static typename Vec<T,S>::List asVertexList(const typename Edge<T,S>::List& edges) {
        typename Vec<T,S>::List result(2 * edges.size());
        for (size_t i = 0; i < edges.size(); ++i) {
            result[2*i+0] = edges[i].start();
            result[2*i+1] = edges[i].end();
        }
        return result;
    }
public:
    friend Edge<T,S> translate(const Edge<T,S>& edge, const Vec<T,S>& offset) {
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

#endif
