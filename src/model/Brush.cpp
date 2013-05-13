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

#include "Brush.h"

#include "CollectionUtils.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        Brush::Brush(const BrushFaceList& faces) :
        m_faces(faces) {
            buildGeometry();
        }
        
        Brush::~Brush() {
            VectorUtils::clearAndDelete(m_faces);
            clearAndDeleteGeometry();
        }

        const BrushFaceList& Brush::faces() const {
            return m_faces;
        }

        void Brush::buildGeometry(const BBox3& bounds) {
            
        }

        void Brush::clearAndDeleteGeometry() {
            BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                BrushFace& face = **it;
                face.setEdges(EmptyBrushEdgeList);
            }
            VectorUtils::clearAndDelete(m_edges);
            VectorUtils::clearAndDelete(m_vertices);
        }
    }
}
