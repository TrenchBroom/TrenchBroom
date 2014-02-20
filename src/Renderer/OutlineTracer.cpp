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

namespace TrenchBroom {
    namespace Renderer {
        void OutlineTracer::addEdge(const Edge3& edge) {
        }

        void OutlineTracer::mergeEdges(Edge3& edge1, Edge3& edge2) const {
            if (!isParallel(edge1, edge2))
                return;
            
            const Vec3 dir = edge1.direction();
            orderByDot(edge1, edge2, dir);
            
            const FloatType e1s = edge1.start.dot(dir);
            const FloatType e1e = edge1.end.dot(dir);
            const FloatType e2s = edge2.start.dot(dir);
            const FloatType e2e = edge2.end.dot(dir);
            assert(e1s < e1e && e1e <= e2s && e2s < e2e);
            
            if (e1e < e2s) {
                // ----
                //       ----
                // ----  ----
                // no overlap - keep both
            } else if (e1e == e2s) {
                // ----
                //     --
                // ------
                // touch - extend 1, discard 2
            } else if (e2s < e1e) {
                if (e1e < e2e) {
                    // ----
                    //   ----
                    // --  --
                    // overlap over end, shorten both
                } else if (e1e == e2e) {
                    // ----
                    //   --
                    // --
                    // overlap until end, shorten 1, discard 2
                } else {
                    assert(e2e < e1e);
                    // ----
                    //  --
                    // -  -
                    // overlap in middle, shorten both
                }
            } else {
                assert(e1s == e2s);
                if (e2e < e1e) {
                    // ----
                    // --
                    //   --
                    // overlap from start, shorten 1, discard 2
                } else {
                    assert(e1e == e2e);
                    // ----
                    // ----
                    //
                    // both are equal, discard both
                }
            }
        }
    }
}
