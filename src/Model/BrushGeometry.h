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

#ifndef __TrenchBroom__BrushGeometry__
#define __TrenchBroom__BrushGeometry__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushEdge.h"

namespace TrenchBroom {
    namespace Model {
        class BrushGeometry {
        public:
            typedef enum {
                BrushIsSplit,
                BrushIsNull,
                FaceIsRedundant
            } AddFaceResultCode;

            template <typename T>
            struct Result {
                T resultCode;
                BrushFace::List addedFaces;
                BrushFace::List droppedFaces;
                
                Result(const T i_resultCode, const BrushFace::List& i_addedFaces = BrushFace::EmptyList, const BrushFace::List& i_droppedFaces = BrushFace::EmptyList) :
                resultCode(i_resultCode),
                addedFaces(i_addedFaces),
                droppedFaces(i_droppedFaces) {}
                
                void append(const Result<T>& other) {
                    addedFaces.insert(addedFaces.end(), other.addedFaces.begin(), other.addedFaces.end());
                    droppedFaces.insert(droppedFaces.end(), other.droppedFaces.begin(), other.droppedFaces.end());
                }
            };
            
            typedef Result<AddFaceResultCode> AddFaceResult;
        private:
            BrushVertex::List m_vertices;
            BrushEdge::List m_edges;
            BrushFaceGeometry::List m_sides;
        public:
            BrushGeometry(const BBox3& worldBounds);
            ~BrushGeometry();
            
            BBox3 bounds() const;
            const BrushVertex::List& vertices() const;
            const BrushEdge::List& edges() const;
            const BrushFaceGeometry::List& sides() const;

            AddFaceResult addFaces(const BrushFace::List& faces);
            AddFaceResult addFace(BrushFace::Ptr face);
        private:
            void initializeWithBounds(const BBox3& bounds);
        };
    }
}

#endif /* defined(__TrenchBroom__BrushGeometry__) */
