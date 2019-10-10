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

#include <vecmath/segment.h>
#include <vecmath/ray.h>
#include <vecmath/plane.h>
#include <vecmath/constants.h>
#include <vecmath/util.h>
#include <vecmath/distance.h>

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::contains(const V& point, const Callback& callback) const {
    if (!polyhedron())
        return false;

    if (!bounds().contains(point))
        return false;

    const Face* firstFace = m_faces.front();
    const Face* currentFace = firstFace;
    do {
        const vm::plane<T,3> plane = callback.getPlane(currentFace);
        if (plane.point_status(point) == vm::plane_status::above) {
            return false;
        }
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::contains(const Polyhedron& other, const Callback& callback) const {
    if (!polyhedron()) {
        return false;
    }

    if (!bounds().contains(other.bounds())) {
        return false;
    }

    const Vertex* theirFirst = other.vertices().front();
    const Vertex* theirCurrent = theirFirst;
    do {
        if (!contains(theirCurrent->position())) {
            return false;
        }
        theirCurrent = theirCurrent->next();
    } while (theirCurrent != theirFirst);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::intersects(const Polyhedron& other, const Callback& callback) const {
    if (!bounds().intersects(other.bounds())) {
        return false;
    }

    if (empty() || other.empty()) {
        return false;
    }

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

    return vm::segment<T,3>(rhsStart, rhsEnd).contains(lhsPos, vm::constants<T>::almost_zero());
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::pointIntersectsPolygon(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    assert(lhs.point());
    assert(rhs.polygon());

    const V& lhsPos = lhs.m_vertices.front()->position();
    const Face* rhsFace = rhs.m_faces.front();
    const V rhsNormal = callback.getPlane(rhsFace).normal;
    const HalfEdgeList& rhsBoundary = rhsFace->boundary();

    return vm::polygon_contains_point(lhsPos, rhsNormal, std::begin(rhsBoundary), std::end(rhsBoundary), GetVertexPosition());
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

    const auto* lhsEdge = lhs.m_edges.front();
    const auto& lhsStart = lhsEdge->firstVertex()->position();
    const auto& lhsEnd = lhsEdge->secondVertex()->position();

    const auto* rhsEdge = rhs.m_edges.front();
    if (rhsEdge->hasPosition(lhsStart) || rhsEdge->hasPosition(lhsEnd)) {
        return true;
    }

    const auto& rhsStart = rhsEdge->firstVertex()->position();
    const auto& rhsEnd = rhsEdge->secondVertex()->position();

    const auto lhsRay = vm::ray<T,3>(lhsStart, normalize(lhsEnd - lhsStart));
    const auto dist = vm::squared_distance(lhsRay, vm::segment<T,3>(rhsStart, rhsEnd));
    const auto rayLen = vm::distance_to_projected_point(lhsRay, lhsEnd);

    if (dist.parallel) {
        if (dist.is_colinear()) {
            const auto rhsStartDist = vm::distance_to_projected_point(lhsRay, rhsStart);
            const auto rhsEndDist   = vm::distance_to_projected_point(lhsRay, rhsEnd);

            return (vm::contains(rhsStartDist, 0.0, rayLen) ||  // lhs constains rhs start
                    vm::contains(rhsEndDist, 0.0, rayLen) ||  // lhs contains rhs end
                    (rhsStartDist > 0.0) != (rhsEndDist > 0.0)); // rhs contains lhs
        } else {
            return false;
        }
    }

    static const auto epsilon2 = vm::constants<T>::almost_zero() * vm::constants<T>::almost_zero();
    return dist.distance < epsilon2 && dist.position1 <= rayLen;
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

    const auto* lhsEdge = lhs.m_edges.front();
    const auto& lhsStart = lhsEdge->firstVertex()->position();
    const auto& lhsEnd = lhsEdge->secondVertex()->position();

    const auto lhsRay = vm::ray<T,3>(lhsStart, normalize(lhsEnd - lhsStart));
    const auto rayLen = dot(lhsEnd - lhsStart, lhsRay.direction);

    auto frontHit = false;
    auto backHit  = false;

    auto* firstFace = rhs.m_faces.front();
    auto* currentFace = firstFace;
    do {
        const auto result = currentFace->intersectWithRay(lhsRay);
        if (result.front()) {
            if (result.distance() <= rayLen) {
                return true;
            }
            frontHit = true;
        } else if (result.back()) {
            if (result.distance() <= rayLen) {
                return true;
            }
            backHit = true;
        }

        currentFace = currentFace->next();
    } while (currentFace != firstFace);

    return backHit && !frontHit;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::edgeIntersectsFace(const Edge* lhsEdge, const Face* rhsFace) {
    const auto& lhsStart = lhsEdge->firstVertex()->position();
    const auto& lhsEnd = lhsEdge->secondVertex()->position();
    const auto lhsRay = vm::ray<T,3>(lhsStart, normalize(lhsEnd - lhsStart));

    const auto dist = rhsFace->intersectWithRay(lhsRay, vm::side::both);
    if (vm::is_nan(dist)) {
        const auto& edgeDir = lhsRay.direction;
        const auto faceNorm = rhsFace->normal();
        if (vm::is_zero(dot(faceNorm, edgeDir), vm::constants<T>::almost_zero())) {
            // ray and face are parallel, intersect with edges

            static const auto MaxDistance = vm::constants<T>::almost_zero() * vm::constants<T>::almost_zero();

            const auto* rhsFirstEdge = rhsFace->boundary().front();
            const auto* rhsCurEdge = rhsFirstEdge;
            do {
                const auto& start = rhsCurEdge->origin()->position();
                const auto& end   = rhsCurEdge->destination()->position();
                if (vm::distance(lhsRay, vm::segment<T,3>(start, end)).distance <= MaxDistance) {
                    return true;
                }
                rhsCurEdge = rhsCurEdge->next();
            } while (rhsCurEdge != rhsFirstEdge);
        } else {
            return false;
        }
    }

    const auto rayLen = dot(lhsEnd - lhsStart, lhsRay.direction);
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

    auto* lhsFace = lhs.faces().front();
    auto* rhsFace = rhs.faces().front();

    return faceIntersectsFace(lhsFace, rhsFace);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::polygonIntersectsPolyhedron(const Polyhedron& lhs, const Polyhedron& rhs, const Callback& callback) {
    assert(lhs.polygon());
    assert(rhs.polyhedron());

    auto* lhsFace = lhs.faces().front();
    auto* firstRhsFace = rhs.faces().front();
    auto* curRhsFace = firstRhsFace;

    do {
        if (faceIntersectsFace(lhsFace, curRhsFace)) {
            return true;
        }

        curRhsFace = curRhsFace->next();
    } while (curRhsFace != firstRhsFace);

    auto* vertex = lhs.vertices().front();
    return rhs.contains(vertex->position());
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::faceIntersectsFace(const Face* lhsFace, const Face* rhsFace) {
    const auto& lhsBoundary = lhsFace->boundary();
    const auto& rhsBoundary = rhsFace->boundary();

    auto* firstLhsEdge = lhsBoundary.front();
    auto* curLhsEdge = firstLhsEdge;
    do {
        if (edgeIntersectsFace(curLhsEdge->edge(), rhsFace)) {
            return true;
        }

        curLhsEdge = curLhsEdge->next();
    } while (curLhsEdge != firstLhsEdge);

    auto* lhsVertex = lhsBoundary.front()->origin();
    auto* rhsVertex = rhsBoundary.front()->origin();

    return (
        vm::polygon_contains_point(lhsVertex->position(), std::begin(rhsBoundary), std::end(rhsBoundary), GetVertexPosition()) ||
        vm::polygon_contains_point(rhsVertex->position(), std::begin(lhsBoundary), std::end(lhsBoundary), GetVertexPosition()));
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

    if (separate(lhs.m_faces.front(), rhs.vertices().front(), callback)) {
        return false;
    }
    if (separate(rhs.faces().front(), lhs.m_vertices.front(), callback)) {
        return false;
    }

    const auto* lhsFirstEdge = lhs.m_edges.front();
    const auto* lhsCurEdge = lhsFirstEdge;
    const auto* rhsFirstEdge = rhs.m_edges.front();
    do {
        const auto  lhsEdgeVec = lhsCurEdge->vector();
        const auto& lhsEdgeOrigin = lhsCurEdge->firstVertex()->position();

        const auto* rhsCurrentEdge = rhsFirstEdge;
        do {
            const auto rhsEdgeVec = rhsCurrentEdge->vector();
            const auto direction = cross(lhsEdgeVec, rhsEdgeVec);

            if (!vm::is_zero(direction, vm::constants<T>::almost_zero())) {
                const auto plane = vm::plane<T,3>(lhsEdgeOrigin, direction);

                const auto lhsStatus = pointStatus(plane, lhs.m_vertices.front());
                if (lhsStatus != vm::plane_status::inside) {
                    const auto rhsStatus = pointStatus(plane, rhs.m_vertices.front());
                    if (rhsStatus != vm::plane_status::inside) {
                        if (lhsStatus != rhsStatus) {
                            return false;
                        }
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
    const auto* currentFace = firstFace;
    do {
        const auto plane = callback.getPlane(currentFace);
        if (pointStatus(plane, firstVertex) == vm::plane_status::above) {
            return true;
        }
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return false;
}

template <typename T, typename FP, typename VP>
vm::plane_status Polyhedron<T,FP,VP>::pointStatus(const vm::plane<T,3>& plane, const Vertex* firstVertex) {
    size_t above = 0;
    size_t below = 0;
    const auto* currentVertex = firstVertex;
    do {
        const auto status = plane.point_status(currentVertex->position());
        if (status == vm::plane_status::above)
            ++above;
        else if (status == vm::plane_status::below)
            ++below;
        if (above > 0 && below > 0)
            return vm::plane_status::inside;
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return above > 0 ? vm::plane_status::above : vm::plane_status::below;
}

#endif /* Polyhedron_Queries_h */
