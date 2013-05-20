/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "IntersectBrushGeometryWithFace.h"

#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        IntersectBrushGeometryWithFace::IntersectBrushGeometryWithFace(BrushGeometry& geometry, BrushFacePtr face) :
        BrushGeometryAlgorithm(geometry),
        m_face(face) {}
        
        IntersectBrushGeometryResult IntersectBrushGeometryWithFace::doExecute(BrushGeometry& geometry) {
            if (isFaceIdenticalWithAnySide(geometry))
                return Redundant;
            const IntersectBrushGeometryResult outside = isFaceOutsideOfGeometry(geometry);
            if (outside != Split)
                return outside;
            
        }
        
        bool IntersectBrushGeometryWithFace::isFaceIdenticalWithAnySide(BrushGeometry& geometry) {
            const BrushFaceGeometryList& sides = geometry.sides();
            BrushFaceGeometryList::const_iterator it, end;
            for (it = sides.begin(), end = sides.end(); it != end; ++it) {
                BrushFaceGeometry& side = **it;
                if (side.face() != NULL) {
                    if (m_face->arePointsOnPlane(side.face()->boundary()))
                        return true;
                }
            }
            return false;
        }
        
        IntersectBrushGeometryResult IntersectBrushGeometryWithFace::isFaceOutsideOfGeometry(BrushGeometry& geometry) {
            size_t drop = 0;
            size_t keep = 0;
            size_t undecided = 0;
            const BrushVertexList& vertices = geometry.vertices();
            BrushVertexList::const_iterator it, end;
            for (it = vertices.begin(), end = vertices.end(); it != end; ++it) {
                BrushVertex& vertex = **it;
                const Plane3& boundary = m_face->boundary();
                const PointStatus::Type status = boundary.pointStatus(vertex.position());
                switch (status) {
                    case PointStatus::PSAbove:
                        drop++;
                        break;
                    case PointStatus::PSBelow:
                        keep++;
                        break;
                    default:
                        undecided++;
                        break;
                }
            }
            
            assert(drop + keep + undecided == vertices.size());
            if (drop + undecided == vertices.size())
                return Null;
            if (keep + undecided == vertices.size())
                return Redundant;
            return Split;
        }
    }
}
