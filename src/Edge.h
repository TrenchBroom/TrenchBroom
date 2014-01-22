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

#ifndef TrenchBroom_Edge_h
#define TrenchBroom_Edge_h

#include "Vec.h"

#include <vector>

namespace TrenchBroom {
    template <typename T, size_t S>
    struct Edge {
        typedef std::vector<Edge<T,S> > List;
        
        Vec<T,S> start;
        Vec<T,S> end;
        
        Edge() {}
        
        Edge(const Vec<T,S>& i_start, const Vec<T,S>& i_end) :
        start(i_start),
        end(i_end) {}
        
        bool operator==(const Edge<T,S>& other) const {
            return ((start == other.start && end == other.end) ||
                    (end == other.start && start == other.end));
        }
        
        Vec<T,S> center() const {
            return (start + end) / static_cast<T>(2.0);
        }
        
        static typename Vec<T,S>::List asVertexList(const typename Edge<T,S>::List& edges) {
            typename Vec<T,S>::List result(2 * edges.size());
            for (size_t i = 0; i < edges.size(); ++i) {
                result[2*i+0] = edges[i].start;
                result[2*i+1] = edges[i].end;
            }
            return result;
        }
    };
}

#endif
