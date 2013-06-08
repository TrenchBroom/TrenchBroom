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
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        
        class BrushFaceGeometry {
        public:
            typedef std::vector<BrushFaceGeometry*> List;
            static const List EmptyList;

            typedef enum {
                Keep,
                Drop,
                Split
            } Mark;
        private:
            BrushVertex::List m_vertices;
            BrushEdge::List m_edges;
            BrushFace::Ptr m_face;
        public:
            inline BrushFace::Ptr face() const {
                return m_face;
            }
            
            inline void setFace(BrushFace::Ptr face) {
                m_face = face;
            }
            
            inline const BrushVertex::List& vertices() const {
                return m_vertices;
            }
            
            inline const BrushEdge::List& edges() const {
                return m_edges;
            }
            
            const Mark mark() const;
            BrushEdge* splitUsingEdgeMarks();
            BrushEdge* findUndecidedEdge() const;
            
            void addForwardEdge(BrushEdge* edge);
            void addForwardEdges(const BrushEdge::List& edges);
            void addBackwardEdge(BrushEdge* edge);
            void addBackwardEdges(const BrushEdge::List& edges);
            
            bool isClosed() const;
            bool hasVertexPositions(const Vec3::List& positions) const;
        private:
            void replaceEdgesWithBackwardEdge(const BrushEdge::List::iterator it1, const BrushEdge::List::iterator it2, BrushEdge* edge);
            void updateVerticesFromEdges();
        };
        
        inline BrushFaceGeometry::List::iterator findBrushFaceGeometry(BrushFaceGeometry::List& faceGeometries, const Vec3::List& positions) {
            BrushFaceGeometry::List::iterator it = faceGeometries.begin();
            const BrushFaceGeometry::List::iterator end = faceGeometries.end();
            while (it != end) {
                const BrushFaceGeometry& faceGeometry = **it;
                if (faceGeometry.hasVertexPositions(positions))
                    return it;
                ++it;
            }
            return end;
        }
        
        inline BrushFaceGeometry::List::const_iterator findBrushFaceGeometry(const BrushFaceGeometry::List& faceGeometries, const Vec3::List& positions) {
            BrushFaceGeometry::List::const_iterator it = faceGeometries.begin();
            const BrushFaceGeometry::List::const_iterator end = faceGeometries.end();
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
