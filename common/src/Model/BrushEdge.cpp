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

#include "BrushEdge.h"

#include "Model/BrushFaceGeometry.h"
#include "Model/BrushVertex.h"

namespace TrenchBroom {
    namespace Model {
        BrushEdge::BrushEdge(BrushVertex* i_start, BrushVertex* i_end) :
        start(i_start),
        end(i_end),
        left(NULL),
        right(NULL),
        mark(Mark_New) {
            assert(start != end);
        }
        
        BrushEdge::BrushEdge(BrushVertex* i_start, BrushVertex* i_end, BrushFaceGeometry* i_left, BrushFaceGeometry* i_right) :
        start(i_start),
        end(i_end),
        left(i_left),
        right(i_right),
        mark(Mark_New) {
            assert(start != end);
        }

        BrushEdge::~BrushEdge() {
            start = NULL;
            end = NULL;
            left = NULL;
            right = NULL;
        }
        
        const BrushFace* BrushEdge::leftFace() const {
            if (left == NULL)
                return NULL;
            return left->face;
        }

        BrushFace* BrushEdge::leftFace() {
            if (left == NULL)
                return NULL;
            return left->face;
        }
        
        const BrushFace* BrushEdge::rightFace() const {
            if (right == NULL)
                return NULL;
            return right->face;
        }
        
        BrushFace* BrushEdge::rightFace() {
            if (right == NULL)
                return NULL;
            return right->face;
        }
        
        void BrushEdge::updateMark() {
            size_t drop = 0;
            size_t keep = 0;
            size_t undecided = 0;
            
            const BrushVertex::Mark startMark = start->mark;
            const BrushVertex::Mark endMark = end->mark;
            
            if (startMark == BrushVertex::Mark_Drop)
                drop++;
            else if (startMark == BrushVertex::Mark_Keep)
                keep++;
            else if (startMark == BrushVertex::Mark_Undecided)
                undecided++;
            if (endMark == BrushVertex::Mark_Drop)
                drop++;
            else if (endMark == BrushVertex::Mark_Keep)
                keep++;
            else if (endMark == BrushVertex::Mark_Undecided)
                undecided++;
            assert(drop + keep + undecided == 2);
            
            if (drop == 1 && keep == 1)
                mark = BrushEdge::Mark_Split;
            else if (drop > 0)
                mark = BrushEdge::Mark_Drop;
            else if (keep > 0)
                mark = BrushEdge::Mark_Keep;
            else
                mark = BrushEdge::Mark_Undecided;
        }
        
        BrushVertex* BrushEdge::split(const Plane3& plane) {
            assert(mark == BrushEdge::Mark_Split);
            
            // Do exactly what QBSP is doing:
            const FloatType startDist = plane.pointDistance(start->position);
            const FloatType endDist = plane.pointDistance(end->position);
            
            assert(startDist != endDist);
            const FloatType dot = startDist / (startDist - endDist);

            const Vec3& startPos = start->position;
            const Vec3& endPos = end->position;
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
            if (start->mark == BrushVertex::Mark_Drop)
                start = newVertex;
            else
                end = newVertex;
            return newVertex;
        }

        void BrushEdge::flip() {
            using std::swap;
            swap(left, right);
            swap(start, end);
        }

        void BrushEdge::setLeftNull() {
            left = NULL;
        }
        
        void BrushEdge::setRightNull() {
            right = NULL;
        }

        void BrushEdge::replaceSide(BrushFaceGeometry* oldSide, BrushFaceGeometry* newSide) {
            assert(left == oldSide || right == oldSide);
            if (left == oldSide)
                left = newSide;
            else
                right = newSide;
        }

        const BrushVertex* BrushEdge::startVertex(const BrushFaceGeometry* side) const {
            if (side == right)
                return start;
            if (side == left)
                return end;
            return NULL;
        }
        
        BrushVertex* BrushEdge::startVertex(const BrushFaceGeometry* side) {
            if (side == right)
                return start;
            if (side == left)
                return end;
            return NULL;
        }
        
        const BrushVertex* BrushEdge::endVertex(const BrushFaceGeometry* side) const {
            if (side == right)
                return end;
            if (side == left)
                return start;
            return NULL;
        }
        
        BrushVertex* BrushEdge::endVertex(const BrushFaceGeometry* side) {
            if (side == right)
                return end;
            if (side == left)
                return start;
            return NULL;
        }

        bool BrushEdge::hasPositions(const Vec3& position1, const Vec3& position2) const {
            if (start == NULL || end == NULL)
                return false;
            return ((start->position == position1 && end->position == position2) ||
                    (start->position == position2 && end->position == position1));
                    
        }

        bool BrushEdge::isIncidentWith(const BrushEdge* edge) const {
            return start == edge->start || start == edge->end || end == edge->start || end == edge->end;
        }

        bool BrushEdge::connects(const BrushVertex* vertex1, BrushVertex* vertex2) const {
            return ((vertex1 == start && vertex2 == end) ||
                    (vertex2 == start && vertex1 == end));
        }

        bool BrushEdge::contains(const Vec3& point, const FloatType maxDistance) const {
            return distanceTo(point).distance <= maxDistance;
        }

        Vec3::EdgeDistance BrushEdge::distanceTo(const Vec3& point) const {
            return point.distanceToSegment(start->position, end->position);
        }

        Vec3 BrushEdge::vector() const {
            return end->position - start->position;
        }

        Vec3 BrushEdge::center() const {
            return (start->position + end->position) / 2.0;
        }

        Edge3 BrushEdge::edgeInfo() const {
            return Edge3(start->position, end->position);
        }

        BrushEdgeList::iterator findBrushEdge(BrushEdgeList& edges, const Vec3& position1, const Vec3& position2) {
            BrushEdgeList::iterator it = edges.begin();
            const BrushEdgeList::iterator end = edges.end();
            while (it != end) {
                const BrushEdge& edge = **it;
                if (edge.hasPositions(position1, position2))
                    return it;
                ++it;
            }
            return end;
        }

        BrushEdgeList::const_iterator findBrushEdge(const BrushEdgeList& edges, const Vec3& position1, const Vec3& position2) {
            BrushEdgeList::const_iterator it = edges.begin();
            const BrushEdgeList::const_iterator end = edges.end();
            while (it != end) {
                const BrushEdge& edge = **it;
                if (edge.hasPositions(position1, position2))
                    return it;
                ++it;
            }
            return end;
        }
    }
}
