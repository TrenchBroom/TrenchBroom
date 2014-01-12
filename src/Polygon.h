/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Vec.h"

#include <algorithm>
#include <cassert>
#include <vector>

template <typename T, size_t S>
struct Polygon {
    typedef std::vector<Polygon<T,S> > List;
    
    typename Vec<T,S>::List vertices;
    
    Polygon() {}
    
    Polygon(const typename Vec<T,S>::List& i_vertices) :
    vertices(i_vertices) {}
    
    Polygon(typename Vec<T,S>::List& i_vertices) {
        using std::swap;
        swap(vertices, i_vertices);
    }
    
    bool operator==(const Polygon<T,S>& rhs) const {
        if (vertices.size() != rhs.vertices.size())
            return false;
        
        const size_t count = vertices.size();
        for (size_t i = 0; i < count; ++i) {
            bool equal = true;
            for (size_t j = 0; j < count && equal; ++j)
                equal = vertices[(i+j) % count] == rhs.vertices[j];
            if (equal)
                return true;
        }
        return false;
    }
    
    Vec<T,S> center() const {
        assert(!vertices.empty());
        Vec<T,S> center = vertices[0];
        for (size_t i = 1; i < vertices.size(); ++i)
            center += vertices[i];
        return center / static_cast<T>(vertices.size());
    }
    
    static typename Vec<T,S>::List asVertexList(const Polygon<T,S>::List& polygons) {
        typename Vec<T,S>::List result;
        for (size_t i = 0; i < polygons.size(); ++i)
            result.insert(result.end(), polygons[i].vertices.begin(), polygons[i].vertices.end());
        return result;
    }
};

#endif
