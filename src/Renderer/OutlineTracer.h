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

#ifndef __TrenchBroom__OutlineTracer__
#define __TrenchBroom__OutlineTracer__

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace Renderer {
        class OutlineTracer {
        private:
            Edge3::List m_edges;
        public:
            void addEdge(const Edge3& edge);
        private:
            void mergeEdges(Edge3& edge1, Edge3& edge2) const;
            bool isParallel(const Edge3& edge1, const Edge3& edge2) const;
            void orderByDot(Edge3& edge1, Edge3& edge2, const Vec3& dir) const;
        };
    }
}

#endif /* defined(__TrenchBroom__OutlineTracer__) */
