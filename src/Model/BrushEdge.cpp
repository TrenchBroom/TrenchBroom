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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "BrushEdge.h"

#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        BrushEdge::BrushEdge(BrushVertex* start, BrushVertex* end) :
        m_start(start),
        m_end(end),
        m_left(NULL),
        m_right(NULL),
        m_mark(New) {
            assert(start != end);
        }
        
        BrushEdge::~BrushEdge() {
            m_start = NULL;
            m_end = NULL;
            m_left = NULL;
            m_right = NULL;
        }
        
        const BrushVertex* BrushEdge::start() const {
            return m_start;
        }
        
        BrushVertex* BrushEdge::start() {
            return m_start;
        }
        
        const BrushVertex* BrushEdge::end() const {
            return m_end;
        }
        
        BrushVertex* BrushEdge::end() {
            return m_end;
        }
        
        const BrushFaceGeometry* BrushEdge::left() const {
            return m_left;
        }
        
        BrushFaceGeometry* BrushEdge::left() {
            return m_left;
        }
        
        const BrushFaceGeometry* BrushEdge::right() const {
            return m_right;
        }
        
        BrushFaceGeometry* BrushEdge::right() {
            return m_right;
        }
        
        BrushEdge::Mark BrushEdge::mark() const {
            return m_mark;
        }
        
        const BrushFace* BrushEdge::leftFace() const {
            if (m_left == NULL)
                return NULL;
            return m_left->face();
        }

        BrushFace* BrushEdge::leftFace() {
            if (m_left == NULL)
                return NULL;
            return m_left->face();
        }
        
        const BrushFace* BrushEdge::rightFace() const {
            if (m_right == NULL)
                return NULL;
            return m_right->face();
        }
        
        BrushFace* BrushEdge::rightFace() {
            if (m_right == NULL)
                return NULL;
            return m_right->face();
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
            for (size_t i = 0; i < 3; ++i) {
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

        void BrushEdge::setLeftNull() {
            m_left = NULL;
        }
        
        void BrushEdge::setRightNull() {
            m_right = NULL;
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

        bool BrushEdge::contains(const Vec3& point, const FloatType maxDistance) const {
            const Vec3 edgeVec = vector();
            const Vec3 edgeDir = edgeVec.normalized();
            const FloatType dot = (point - m_start->position()).dot(edgeDir);
            
            // determine the closest point on the edge
            Vec3 closestPoint;
            if (dot < 0.0)
                closestPoint = m_start->position();
            else if ((dot * dot) > edgeVec.squaredLength())
                closestPoint = m_end->position();
            else
                closestPoint = m_start->position() + edgeDir * dot;
            
            const FloatType distance2 = (point - closestPoint).squaredLength();
            return distance2 <= (maxDistance * maxDistance);
        }

        Vec3 BrushEdge::vector() const {
            return m_end - m_start;
        }
    }
}
