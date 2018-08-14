/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef Polyhedron_Queries_h
#define Polyhedron_Queries_h

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::contains(const V& point, const Callback& callback) const {
    if (!polyhedron())
        return false;
    
    if (!bounds().contains(point))
        return false;
    
    const Face* firstFace = m_faces.front();
    const Face* currentFace = firstFace;
    do {
        const Plane<T,3> plane = callback.plane(currentFace);
        if (plane.pointStatus(point) == Math::PointStatus::PSAbove)
            return false;
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::contains(const Polyhedron& other, const Callback& callback) const {
    if (!polyhedron())
        return false;
    
    if (!bounds().contains(other.bounds()))
        return false;

    const Vertex* theirFirst = other.vertices().front();
    const Vertex* theirCurrent = theirFirst;
    do {
        if (!contains(theirCurrent->position()))
            return false;
        theirCurrent = theirCurrent->next();
    } while (theirCurrent != theirFirst);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::intersects(const Polyhedron& other, const Callback& callback) const {
    if (!bounds().intersects(other.bounds()))
        return false;

    if (empty() || other.empty())
        return false;
    
    if (point()) {
        if (other.point()) {
            return pointIntersectsPoint(*this, other, callback);
        } else if (other.edge()) {
            return pointIntersectsEdge(*this, other, callback);
        } else if (other.polygon()) {
            return pointIntersectsPolygon(*this, other, callback);
        } else {
            return pointIntersectsPolyhedron(*this, other, callback);
        }
    } else if (edge()) {
        if (other.point()) {
            return edgeIntersectsPoint(*this, other, callback);
        } else if (other.edge()) {
            return edgeIntersectsEdge(*this, other, callback);
        } else if (other.polygon()) {
            return edgeIntersectsPolygon(*this, other, callback);
        } else {
            return edgeIntersectsPolyhedron(*this, other, callback);
        }
    } else if (polygon()) {
        if (other.point()) {
            return polygonIntersectsPoint(*this, other, callback);
        } else if (other.edge()) {
            return polygonIntersectsEdge(*this, other, callback);
        } else if (other.polygon()) {
            return polygonIntersectsPolygon(*this, other, callback);
        } else {
            return polygonIntersectsPolyhedron(*this, other, callback);
        }
    } else {
        if (other.point()) {
            return polyhedronIntersectsPoint(*this, other, callback);
        } else if (other.edge()) {
            return polyhedronIntersectsEdge(*this, other, callback);
        } else if (other.polygon()) {
            return polyhedronIntersectsPolygon(*this, other, callback);
        } else {
            return polyhedronIntersectsPolyhedron(*this, other, callback);
        }
    }
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::pointIntersectsPoint(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    assert(lhs.point());
    assert(rhs.point());
    
    const V& lhsPos = lhs.m_vertices.front()->position();
    const V& rhsPos = rhs.m_vertices.front()->position();
    return lhsPos == rhsPos;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::pointIntersectsEdge(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    assert(lhs.point());
    assert(rhs.edge());
    
    const V& lhsPos = lhs.m_vertices.front()->position();
    const Edge* rhsEdge = rhs.m_edges.front();
    const V& rhsStart = rhsEdge->firstVertex()->position();
    const V& rhsEnd = rhsEdge->secondVertex()->position();

    return lhsPos.containedWithinSegment(rhsStart, rhsEnd);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::pointIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    assert(lhs.point());
    assert(rhs.polygon());
    
    const V& lhsPos = lhs.m_vertices.front()->position();
    const Face* rhsFace = rhs.m_faces.front();
    const V rhsNormal = callback.plane(rhsFace).normal;
    const HalfEdgeList& rhsBoundary = rhsFace->boundary();
    
    return polygonContainsPoint(lhsPos, rhsNormal, std::begin(rhsBoundary), std::end(rhsBoundary), GetVertexPosition());
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::pointIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    assert(lhs.point());
    assert(rhs.polyhedron());
    
    const V& lhsPos = lhs.m_vertices.front()->position();
    return rhs.contains(lhsPos, callback);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::edgeIntersectsPoint(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    return pointIntersectsEdge(rhs, lhs, callback);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::edgeIntersectsEdge(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    assert(lhs.edge());
    assert(rhs.edge());

    const Edge* lhsEdge = lhs.m_edges.front();
    const V& lhsStart = lhsEdge->firstVertex()->position();
    const V& lhsEnd = lhsEdge->secondVertex()->position();

    const Edge* rhsEdge = rhs.m_edges.front();
    if (rhsEdge->hasPosition(lhsStart) || rhsEdge->hasPosition(lhsEnd))
        return true;
    
    const V& rhsStart = rhsEdge->firstVertex()->position();
    const V& rhsEnd = rhsEdge->secondVertex()->position();
    
    const Ray<T,3> lhsRay(lhsStart, normalize(lhsEnd - lhsStart));
    const typename Ray<T,3>::LineDistance dist = lhsRay.squaredDistanceToSegment(rhsStart, rhsEnd);

    const T rayLen = lhsRay.distanceToPointOnRay(lhsEnd);
    
    if (dist.parallel) {
        if (dist.colinear()) {
            const T rhsStartDist = lhsRay.distanceToPointOnRay(rhsStart);
            const T rhsEndDist   = lhsRay.distanceToPointOnRay(rhsEnd);
            
            return (Math::between(rhsStartDist, 0.0, rayLen) ||  // lhs constains rhs start
                    Math::between(rhsEndDist,   0.0, rayLen) ||  // lhs contains rhs end
                    (rhsStartDist > 0.0) != (rhsEndDist > 0.0)); // rhs contains lhs
        }
        return false;
    }

    static const T epsilon2 = Math::Constants<T>::almostZero() * Math::Constants<T>::almostZero();
    return dist.distance < epsilon2 && dist.rayDistance <= rayLen;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::edgeIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    assert(lhs.edge());
    assert(rhs.polygon());
    
    const Edge* lhsEdge = lhs.m_edges.front();
    const Face* rhsFace = rhs.m_faces.front();
    
    return edgeIntersectsFace(lhsEdge, rhsFace);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::edgeIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    assert(lhs.edge());
    assert(rhs.polyhedron());
    
    const Edge* lhsEdge = lhs.m_edges.front();
    const V& lhsStart = lhsEdge->firstVertex()->position();
    const V& lhsEnd = lhsEdge->secondVertex()->position();

    const Ray<T,3> lhsRay(lhsStart, normalize(lhsEnd - lhsStart));
    const T rayLen = dot(lhsEnd - lhsStart, lhsRay.direction);
    
    bool frontHit = false;
    bool backHit  = false;
    
    Face* firstFace = rhs.m_faces.front();
    Face* currentFace = firstFace;
    do {
        const typename Face::RayIntersection result = currentFace->intersectWithRay(lhsRay);
        if (result.front()) {
            if (result.distance() <= rayLen)
                return true;
            frontHit = true;
        } else if (result.back()) {
            if (result.distance() <= rayLen)
                return true;
            backHit = true;
        }
        
        currentFace = currentFace->next();
    } while (currentFace != firstFace);

    return backHit && !frontHit;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::edgeIntersectsFace(const Edge* lhsEdge, const Face* rhsFace) {
    const V& lhsStart = lhsEdge->firstVertex()->position();
    const V& lhsEnd = lhsEdge->secondVertex()->position();
    const Ray<T,3> lhsRay(lhsStart, normalize(lhsEnd - lhsStart));
    
    const T dist = rhsFace->intersectWithRay(lhsRay, Math::Side_Both);
    if (Math::isnan(dist)) {
        const V& edgeDir = lhsRay.direction;
        const V faceNorm = rhsFace->normal();
        if (Math::zero(dot(faceNorm, edgeDir))) {
            // ray and face are parallel, intersect with edges

            static const T MaxDistance = Math::Constants<T>::almostZero() * Math::Constants<T>::almostZero();
            
            const HalfEdge* rhsFirstEdge = rhsFace->boundary().front();
            const HalfEdge* rhsCurEdge = rhsFirstEdge;
            do {
                const V& start = rhsCurEdge->origin()->position();
                const V& end   = rhsCurEdge->destination()->position();
                if (lhsRay.squaredDistanceToSegment(start, end).distance <= MaxDistance)
                    return true;
                rhsCurEdge = rhsCurEdge->next();
            } while (rhsCurEdge != rhsFirstEdge);
        }
        return false;
    }
    
    const T rayLen = dot(lhsEnd - lhsStart, lhsRay.direction);
    return dist <= rayLen;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::polygonIntersectsPoint(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    return pointIntersectsPolygon(rhs, lhs, callback);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::polygonIntersectsEdge(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback){
    return edgeIntersectsPolygon(rhs, lhs, callback);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::polygonIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    assert(lhs.polygon());
    assert(rhs.polygon());

    Face* lhsFace = lhs.faces().front();
    Face* rhsFace = rhs.faces().front();

    return faceIntersectsFace(lhsFace, rhsFace);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::polygonIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    assert(lhs.polygon());
    assert(rhs.polyhedron());

    Face* lhsFace = lhs.faces().front();
    Face* firstRhsFace = rhs.faces().front();
    Face* curRhsFace = firstRhsFace;
    
    do {
        if (faceIntersectsFace(lhsFace, curRhsFace))
            return true;
        
        curRhsFace = curRhsFace->next();
    } while (curRhsFace != firstRhsFace);

    Vertex* vertex = lhs.vertices().front();
    return rhs.contains(vertex->position());
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::faceIntersectsFace(const Face* lhsFace, const Face* rhsFace) {
    const HalfEdgeList& lhsBoundary = lhsFace->boundary();
    const HalfEdgeList& rhsBoundary = rhsFace->boundary();
    
    HalfEdge* firstLhsEdge = lhsBoundary.front();
    HalfEdge* curLhsEdge = firstLhsEdge;
    do {
        if (edgeIntersectsFace(curLhsEdge->edge(), rhsFace))
            return true;
        
        curLhsEdge = curLhsEdge->next();
    } while (curLhsEdge != firstLhsEdge);
    
    Vertex* lhsVertex = lhsBoundary.front()->origin();
    Vertex* rhsVertex = rhsBoundary.front()->origin();
    
    if (polygonContainsPoint(lhsVertex->position(), std::begin(rhsBoundary), std::end(rhsBoundary), GetVertexPosition()))
        return true;
    
    if (polygonContainsPoint(rhsVertex->position(), std::begin(lhsBoundary), std::end(lhsBoundary), GetVertexPosition()))
        return true;
    
    return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::polyhedronIntersectsPoint(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    return pointIntersectsPolyhedron(rhs, lhs, callback);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::polyhedronIntersectsEdge(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    return edgeIntersectsPolyhedron(rhs, lhs, callback);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::polyhedronIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    return polygonIntersectsPolyhedron(rhs, lhs, callback);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::polyhedronIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    assert(lhs.polyhedron());
    assert(rhs.polyhedron());
    
    // separating axis theorem
    // http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
    
    if (separate(lhs.m_faces.front(), rhs.vertices().front(), callback))
        return false;
    if (separate(rhs.faces().front(), lhs.m_vertices.front(), callback))
        return false;
    
    const Edge* lhsFirstEdge = lhs.m_edges.front();
    const Edge* lhsCurEdge = lhsFirstEdge;
    const Edge* rhsFirstEdge = rhs.m_edges.front();
    do {
        const V lhsEdgeVec = lhsCurEdge->vector();
        const V& lhsEdgeOrigin = lhsCurEdge->firstVertex()->position();
        
        const Edge* rhsCurrentEdge = rhsFirstEdge;
        do {
            const V rhsEdgeVec = rhsCurrentEdge->vector();
            const V direction = cross(lhsEdgeVec, rhsEdgeVec);
            
            if (!isNull(direction)) {
                const Plane<T,3> plane(lhsEdgeOrigin, direction);
                
                const Math::PointStatus::Type lhsStatus = pointStatus(plane, lhs.m_vertices.front());
                if (lhsStatus != Math::PointStatus::PSInside) {
                    const Math::PointStatus::Type rhsStatus = pointStatus(plane, rhs.m_vertices.front());
                    if (rhsStatus != Math::PointStatus::PSInside) {
                        if (lhsStatus != rhsStatus)
                            return false;
                    }
                }
            }
            
            rhsCurrentEdge = rhsCurrentEdge->next();
        } while (rhsCurrentEdge != rhsFirstEdge);
        lhsCurEdge = lhsCurEdge->next();
    } while (lhsCurEdge != lhsFirstEdge);
    
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::separate(const Face* firstFace, const Vertex* firstVertex, const Callback& callback) {
    const Face* currentFace = firstFace;
    do {
        const Plane<T,3> plane = callback.plane(currentFace);
        if (pointStatus(plane, firstVertex) == Math::PointStatus::PSAbove)
            return true;
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return false;
}

template <typename T, typename FP, typename VP>
Math::PointStatus::Type Polyhedron<T,FP,VP>::pointStatus(const Plane<T,3>& plane, const Vertex* firstVertex) {
    size_t above = 0;
    size_t below = 0;
    const Vertex* currentVertex = firstVertex;
    do {
        const Math::PointStatus::Type status = plane.pointStatus(currentVertex->position());
        if (status == Math::PointStatus::PSAbove)
            ++above;
        else if (status == Math::PointStatus::PSBelow)
            ++below;
        if (above > 0 && below > 0)
            return Math::PointStatus::PSInside;
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return above > 0 ? Math::PointStatus::PSAbove : Math::PointStatus::PSBelow;
}

#endif /* Polyhedron_Queries_h */
