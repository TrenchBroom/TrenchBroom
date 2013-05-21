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

#ifndef __TrenchBroom__BrushFaceGeometry__
#define __TrenchBroom__BrushFaceGeometry__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Model/BrushGeometryTypes.h"
#include "Model/BrushFaceTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        
        class BrushFaceGeometry {
        public:
            typedef enum {
                Keep,
                Drop,
                Split
            } Mark;
        private:
            BrushVertexList m_vertices;
            BrushEdgeList m_edges;
            BrushFacePtr m_face;
        public:
            inline BrushFacePtr face() const {
                return m_face;
            }
            
            inline void setFace(BrushFacePtr face) {
                m_face = face;
            }
            
            inline const BrushVertexList& vertices() const {
                return m_vertices;
            }
            
            inline const BrushEdgeList& edges() const {
                return m_edges;
            }
            
            const Mark mark() const;
            BrushEdge* splitUsingEdgeMarks();
            BrushEdge* findUndecidedEdge() const;
            
            void addForwardEdge(BrushEdge* edge);
            void addForwardEdges(const BrushEdgeList& edges);
            void addBackwardEdge(BrushEdge* edge);
            void addBackwardEdges(const BrushEdgeList& edges);
            
            bool isClosed() const;
            bool hasVertexPositions(const Vec3::List& positions) const;
        private:
            void replaceEdgesWithBackwardEdge(const BrushEdgeList::iterator it1, const BrushEdgeList::iterator it2, BrushEdge* edge);
            void updateVerticesFromEdges();
        };
        
        inline BrushFaceGeometryList::iterator findBrushFaceGeometry(BrushFaceGeometryList& faceGeometries, const Vec3::List& positions) {
            BrushFaceGeometryList::iterator it = faceGeometries.begin();
            const BrushFaceGeometryList::iterator end = faceGeometries.end();
            while (it != end) {
                const BrushFaceGeometry& faceGeometry = **it;
                if (faceGeometry.hasVertexPositions(positions))
                    return it;
                ++it;
            }
            return end;
        }
        
        inline BrushFaceGeometryList::const_iterator findBrushFaceGeometry(const BrushFaceGeometryList& faceGeometries, const Vec3::List& positions) {
            BrushFaceGeometryList::const_iterator it = faceGeometries.begin();
            const BrushFaceGeometryList::const_iterator end = faceGeometries.end();
            while (it != end) {
                const BrushFaceGeometry& faceGeometry = **it;
                if (faceGeometry.hasVertexPositions(positions))
                    return it;
                ++it;
            }
            return end;
        }
    }
}

#endif /* defined(__TrenchBroom__BrushFaceGeometry__) */
