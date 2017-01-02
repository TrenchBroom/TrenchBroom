/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CollectionUtils.h"
#include "Vec.h"

#include <algorithm>
#include <cassert>
#include <vector>

namespace TrenchBroom {
    template <typename T, size_t S>
    class Polygon {
    public:
        typedef std::vector<Polygon<T,S> > List;
    private:
        typename Vec<T,S>::List m_vertices;
    public:
        Polygon() {}
        
        Polygon(const typename Vec<T,S>::List& i_vertices) :
        m_vertices(i_vertices) {
            orderVertices(m_vertices);
        }
        
        Polygon(typename Vec<T,S>::List& i_vertices) {
            using std::swap;
            swap(m_vertices, i_vertices);
            orderVertices(m_vertices);
        }
        
        bool operator==(const Polygon<T,S>& rhs) const {
            return VectorUtils::equals(m_vertices, rhs.m_vertices);
        }
        
        bool operator!=(const Polygon<T,S>& rhs) const {
            return !(*this == rhs);
        }
        
        bool operator<(const Polygon<T,S>& rhs) const {
            return compare(rhs) < 0;
        }
        
        int compare(const Polygon<T,S>& other) const {
            if (m_vertices.size() < other.m_vertices.size())
                return -1;
            if (m_vertices.size() > other.m_vertices.size())
                return 1;

            const size_t count = std::min(m_vertices.size(), other.m_vertices.size());
            for (size_t i = 0; i < count; ++i) {
                const int cmp = m_vertices[i].compare(other.m_vertices[i]);
                if (cmp < 0)
                    return -1;
                if (cmp > 0)
                    return 1;
            }
            return 0;
        }
        
        bool contains(const Vec<T,S>& vertex) const {
            return std::find(std::begin(m_vertices), std::end(m_vertices), vertex) != std::end(m_vertices);
        }
        
        size_t vertexCount() const {
            return m_vertices.size();
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
            for (size_t i = 0; i < polygons.size(); ++i)
                result.insert(std::end(result), std::begin(polygons[i].m_vertices), std::end(polygons[i].m_vertices));
            return result;
        }
        
    private:
        void orderVertices(typename Vec<T,S>::List& vertices) {
            if (vertices.size() < 2)
                return;
            
            typedef typename Vec<T,S>::List::iterator Iter;
            Iter it = std::begin(vertices);
            Iter end = std::end(vertices);
            Iter smallest = it++;
            
            while (it != end) {
                if (*it < *smallest)
                    smallest = it;
                ++it;
            }

            std::rotate(std::begin(vertices), smallest, std::end(vertices));
        }
    };
    
    typedef Polygon<float,2> Polygon2f;
    typedef Polygon<double,2> Polygon2d;
    typedef Polygon<float,3> Polygon3f;
    typedef Polygon<double,3> Polygon3d;
}

#endif
