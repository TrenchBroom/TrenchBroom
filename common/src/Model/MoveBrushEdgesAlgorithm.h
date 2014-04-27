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

#ifndef __TrenchBroom__MoveBrushEdgesAlgorithm__
#define __TrenchBroom__MoveBrushEdgesAlgorithm__

#include "Model/MoveBrushVertexAlgorithm.h"

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/BrushGeometry.h"

namespace TrenchBroom {
    namespace Model {
        class MoveBrushEdgesAlgorithm : public MoveBrushVertexAlgorithm<MoveEdgesResult> {
        private:
            const BBox3& m_worldBounds;
            const Edge3::List& m_edges;
            const Vec3& m_delta;
        public:
            MoveBrushEdgesAlgorithm(BrushGeometry& geometry, const BBox3& worldBounds, const Edge3::List& edges, const Vec3& delta);
        private:
            bool doCanExecute(BrushGeometry& geometry);
            MoveEdgesResult doExecute(BrushGeometry& geometry);
        };
    }
}

#endif /* defined(__TrenchBroom__MoveBrushEdgesAlgorithm__) */
