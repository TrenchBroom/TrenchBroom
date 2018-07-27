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
#include "Vec.h"

#include <algorithm>
#include <cassert>
#include <vector>

namespace TrenchBroom {
    template <typename T, size_t S>
    class Polygon {
    public:
        using Type = T;
        static const size_t Size = S;
        using List = std::vector<Polygon<T,S>>;
        using FloatType = Polygon<float, S>;
    private:
        typename Vec<T,S>::List m_vertices;
    public:
        Polygon() {}
        
        Polygon(std::initializer_list<Vec<T,S>> i_vertices) :
        m_vertices(i_vertices) {
            CollectionUtils::rotateMinToFront(m_vertices);
        }
        
        Polygon(const typename Vec<T,S>::List& i_vertices) :
        m_vertices(i_vertices) {
            CollectionUtils::rotateMinToFront(m_vertices);
        }
        
        Polygon(typename Vec<T,S>::List& i_vertices) {
            using std::swap;
            swap(m_vertices, i_vertices);
            CollectionUtils::rotateMinToFront(m_vertices);
        }
        
        template <typename TT, size_t SS>
        Polygon(const Polygon<TT,SS>& other) {
            m_vertices.reserve(other.vertexCount());
            for (const auto& vertex : other)
                m_vertices.push_back(Vec<T,S>(vertex));
        }

        bool operator==(const Polygon<T,S>& rhs) const {
            return compare(rhs) == 0;
        }
        
        bool operator!=(const Polygon<T,S>& rhs) const {
            return compare(rhs) != 0;
        }
        
        bool operator<(const Polygon<T,S>& rhs) const {
            return compare(rhs) < 0;
        }

        int compareSnapped(const Polygon<T,S>& other, const T precision) const {
            if (m_vertices.size() < other.m_vertices.size())
                return -1;
            if (m_vertices.size() > other.m_vertices.size())
                return 1;

            const auto count = m_vertices.size();
            return doCompareSnapped(other, 0, count, precision);
        }

        int compare(const Polygon<T,S>& other, const T epsilon = static_cast<T>(0.0)) const {
            if (m_vertices.size() < other.m_vertices.size())
                return -1;
            if (m_vertices.size() > other.m_vertices.size())
                return 1;

            const auto count = m_vertices.size();
            return doCompare(other, 0, count, epsilon);
        }

        int compareUnoriented(const Polygon<T,S>& other, const T epsilon = static_cast<T>(0.0)) const {
            if (m_vertices.size() < other.m_vertices.size())
                return -1;
            if (m_vertices.size() > other.m_vertices.size())
                return 1;

            const auto count = m_vertices.size();
            if (count == 0) {
                return 0;
            }

            // Compare first:
            const auto cmp0 = m_vertices[0].compare(other.m_vertices[0]);
            if (cmp0 < 0) {
                return -1;
            } else if (cmp0 > 0) {
                return +1;
            }

            if (count == 1) {
                return 0;
            }

            // First vertices are identical. Now compare my second with other's second.
            auto cmp1 = m_vertices[1].compare(other.m_vertices[1]);
            if (cmp1 == 0) {
                // The second vertices are also identical, so we just do a forward compare.
                return doCompare(other, 2, count, epsilon);
            } else {
                // The second vertices are not identical, so we attemp a backward compare.
                size_t i = 1;
                while (i < count) {
                    const auto j = count - i;
                    const auto cmp = m_vertices[i].compare(other.m_vertices[j]);
                    if (cmp != 0) {
                        // Backward compare failed, so make a forward compare
                        return doCompare(other, 2, count, epsilon);
                    }
                    ++i;
                }
                return 0;
            }
        }
    private:
        int doCompare(const Polygon<T,S>& other, size_t i, const size_t count, const T epsilon) const {
            while (i < count) {
                const auto cmp = m_vertices[i].compare(other.m_vertices[i], epsilon);
                if (cmp < 0) {
                    return -1;
                } else if (cmp > 0) {
                    return +1;
                }
                ++i;
            }

            return 0;
        }

        int doCompareSnapped(const Polygon<T,S>& other, size_t i, const size_t count, const T precision) const {
            while (i < count) {
                const auto cmp = m_vertices[i].compareSnapped(other.m_vertices[i], precision);
                if (cmp < 0) {
                    return -1;
                } else if (cmp > 0) {
                    return +1;
                }
                ++i;
            }

            return 0;
        }
    public:
        bool hasVertex(const Vec<T,S>& vertex) const {
            return std::find(std::begin(m_vertices), std::end(m_vertices), vertex) != std::end(m_vertices);
        }
        
        bool contains(const Vec<T,S>& point, const Vec<T,3>& normal) const {
            return polygonContainsPoint(point, normal, std::begin(m_vertices), std::end(m_vertices));
        }
        
        size_t vertexCount() const {
            return m_vertices.size();
        }
        
        typename Vec<T,3>::List::const_iterator begin() const {
            return std::begin(m_vertices);
        }
        
        typename Vec<T,3>::List::const_iterator end() const {
            return std::end(m_vertices);
        }
        
        const typename Vec<T,S>::List& vertices() const {
            return m_vertices;
        }
        
        Vec<T,S> center() const {
            assert(!m_vertices.empty());
            Vec<T,S> center = m_vertices[0];
            for (size_t i = 1; i < m_vertices.size(); ++i)
                center += m_vertices[i];
            return center / static_cast<T>(m_vertices.size());
        }
        
        static typename Vec<T,S>::List asVertexList(const typename Polygon<T,S>::List& polygons) {
            typename Vec<T,S>::List result;
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
    public:
        friend Polygon<T,S> translate(const Polygon<T,S>& polygon, const Vec<T,S>& offset) {
            return Polygon<T,S>(polygon.vertices() + offset);
        }
    };
    
    typedef Polygon<float,2> Polygon2f;
    typedef Polygon<double,2> Polygon2d;
    typedef Polygon<float,3> Polygon3f;
    typedef Polygon<double,3> Polygon3d;
}

#endif
