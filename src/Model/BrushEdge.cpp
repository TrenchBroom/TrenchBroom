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

#include "BrushEdge.h"

#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        BrushEdge::BrushEdge(BrushVertex* start, BrushVertex* end) :
        m_start(start),
        m_end(end),
        m_left(NULL),
        m_right(NULL),
        m_mark(New) {}
        
        BrushEdge::~BrushEdge() {
            m_start = NULL;
            m_end = NULL;
            m_left = NULL;
            m_right = NULL;
        }
        
        void BrushEdge::updateMark() {
            size_t drop = 0;
            size_t keep = 0;
            size_t undecided = 0;
            
            const BrushVertex::Mark startMark = m_start->mark();
            const BrushVertex::Mark endMark = m_end->mark();
            
            if (startMark == BrushVertex::Drop)
                drop++;
            else if (startMark == BrushVertex::Keep)
                keep++;
            else if (startMark == BrushVertex::Undecided)
                undecided++;
            if (endMark == BrushVertex::Drop)
                drop++;
            else if (endMark == BrushVertex::Keep)
                keep++;
            else if (endMark == BrushVertex::Undecided)
                undecided++;
            assert(drop + keep + undecided == 2);
            
            if (drop == 1 && keep == 1)
                m_mark = BrushEdge::Split;
            else if (drop > 0)
                m_mark = BrushEdge::Drop;
            else if (keep > 0)
                m_mark = BrushEdge::Keep;
            else
                m_mark = BrushEdge::Undecided;
        }
        
        BrushVertex* BrushEdge::split(const Plane3& plane) {
            assert(mark() == BrushEdge::Split);
            
            // Do exactly what QBSP is doing:
            const FloatType startDist = plane.pointDistance(m_start->position());
            const FloatType endDist = plane.pointDistance(m_end->position());
            
            assert(startDist != endDist);
            const FloatType dot = startDist / (startDist - endDist);

            const Vec3& startPos = m_start->position();
            const Vec3& endPos = m_end->position();
            Vec3 position;
            for (size_t i = 0; i < 3; i++) {
                if (plane.normal[i] == 1.0)
                    position[i] = plane.distance;
                else if (plane.normal[i] == -1.0)
                    position[i] = -plane.distance;
                else
                    position[i] = startPos[i] + dot * (endPos[i] - startPos[i]);
            }
            
            // cheat a little bit?, just like QBSP
            position.correct();
            
            BrushVertex* newVertex = new BrushVertex(position);
            if (m_start->mark() == BrushVertex::Drop)
                m_start = newVertex;
            else
                m_end = newVertex;
            return newVertex;
        }

        void BrushEdge::flip() {
            std::swap(m_left, m_right);
            std::swap(m_start, m_end);
        }

        const BrushVertex* BrushEdge::start(const BrushFaceGeometry* side) const {
            if (side == m_right)
                return m_start;
            if (side == m_left)
                return m_end;
            return NULL;
        }
        
        BrushVertex* BrushEdge::start(const BrushFaceGeometry* side) {
            if (side == m_right)
                return m_start;
            if (side == m_left)
                return m_end;
            return NULL;
        }
        
        const BrushVertex* BrushEdge::end(const BrushFaceGeometry* side) const {
            if (side == m_right)
                return m_end;
            if (side == m_left)
                return m_start;
            return NULL;
        }
        
        BrushVertex* BrushEdge::end(const BrushFaceGeometry* side) {
            if (side == m_right)
                return m_end;
            if (side == m_left)
                return m_start;
            return NULL;
        }

        bool BrushEdge::hasPositions(const Vec3& position1, const Vec3& position2) const {
            if (m_start == NULL || m_end == NULL)
                return false;
            return (m_start->position() == position1 && m_end->position() == position2) || (m_start->position() == position2 && m_end->position() == position1);
                    
        }
    }
}
