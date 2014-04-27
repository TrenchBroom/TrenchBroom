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

#ifndef __TrenchBroom__SplitBrushEdgeAlgorithm__
#define __TrenchBroom__SplitBrushEdgeAlgorithm__

#include "Model/MoveBrushVertexAlgorithm.h"

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/BrushGeometry.h"

namespace TrenchBroom {
    namespace Model {
        class SplitBrushEdgeAlgorithm : public MoveBrushVertexAlgorithm<SplitResult> {
        private:
            const BBox3& m_worldBounds;
            const Edge3 m_edge;
            const Vec3& m_delta;
        public:
            SplitBrushEdgeAlgorithm(BrushGeometry& geometry, const BBox3& worldBounds, const Edge3& edge, const Vec3& delta);
        private:
            bool doCanExecute(BrushGeometry& geometry);
            SplitResult doExecute(BrushGeometry& geometry);
            BrushVertex* splitEdge(BrushGeometry& geometry, BrushEdge* edge);
        };
    }
}

#endif /* defined(__TrenchBroom__SplitBrushEdgeAlgorithm__) */
