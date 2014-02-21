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

#include "OutlineTracer.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {
        void OutlineTracer::addEdge(const Edge3& edge) {
        }

        void OutlineTracer::mergeEdges(Edge3& edge1, Edge3& edge2) const {
            using std::swap;
            
            if (!isParallel(edge1, edge2))
                return;
            
            const Vec3 dir = (edge1.end - edge1.start).normalized();
            
            const FloatType s1 = edge1.start.dot(dir);
            const FloatType e1 = edge1.end.dot(dir);
            /* should never happen and is checked by assertion!
            if (s1 > e1) {
                swap(edge1.start, edge1.end);
                swap(s1, e1);
            }
             */
            
            FloatType s2 = edge2.start.dot(dir);
            FloatType e2 = edge2.end.dot(dir);
            if (s2 > e2) {
                swap(edge2.start, edge2.end);
                swap(s2, e2);
            }
            
            // now the vertices of each edge should be ordered in relation to edge1's direction:
            assert(s1 < e1 && s2 < e2);
            
            // now we must handle all 13 different cases of how the edges might be arranged:
            if (e2 > e1) {
                if (s2 > e1) {
                    // --
                    //     --
                } else if (s2 == e1) {
                    // --
                    //   --
                } else if (s1 < s2) {
                    // ---
                    //   --
                } else if (s2 == s1) {
                    // --
                    // ---
                } else {
                    assert(s1 < s1);
                    //  --
                    // ----
                }
            } else if (e2 == e1) {
                if (s1 < s2) {
                    // ---
                    //  --
                } else if (s1 == s2) {
                    // ---
                    // ---
                } else {
                    assert(s2 < s1);
                    //  --
                    // ---
                }
            } else if (e2 == s1) {
                //   --
                // --
            } else if (e2 < s1) {
                //     --
                // --
            } else {
                assert(s1 < e2 && e2 < e1);
                if (s1 < s2) {
                    // ----
                    //  --
                } else if (s1 == s2) {
                    // ----
                    // --
                } else {
                    assert(s2 < s1);
                    //  ----
                    // ---
                }
            }
        }
    }
}
