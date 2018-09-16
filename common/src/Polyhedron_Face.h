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

#ifndef TrenchBroom_Polyhedron_Face_h
#define TrenchBroom_Polyhedron_Face_h

#include <vecmath/intersection.h>

#include <vecmath/vec.h>
#include <vecmath/ray.h>
#include <vecmath/plane.h>
#include <vecmath/constants.h>
#include <vecmath/scalar.h>
#include <vecmath/util.h>

template <typename T, typename FP, typename VP>
typename DoublyLinkedList<typename Polyhedron<T,FP,VP>::Face, typename Polyhedron<T,FP,VP>::GetFaceLink>::Link& Polyhedron<T,FP,VP>::GetFaceLink::operator()(Face* face) const {
    return face->m_link;
}

template <typename T, typename FP, typename VP>
const typename DoublyLinkedList<typename Polyhedron<T,FP,VP>::Face, typename Polyhedron<T,FP,VP>::GetFaceLink>::Link& Polyhedron<T,FP,VP>::GetFaceLink::operator()(const Face* face) const {
    return face->m_link;
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Face::Face(HalfEdgeList& boundary) :
m_payload(FP::defaultValue()),
#ifdef _MSC_VER
// MSVC throws a warning because we're passing this to the FaceLink constructor, but it's okay because we just store the pointer there.
#pragma warning(push)
#pragma warning(disable : 4355)
m_link(this)
#pragma warning(pop)
#else
m_link(this)
#endif
{
    using std::swap;
    swap(m_boundary, boundary);
    
    assert(m_boundary.size() >= 3);
    setBoundaryFaces();
}

template <typename T, typename FP, typename VP>
typename FP::Type Polyhedron<T,FP,VP>::Face::payload() const {
    return m_payload;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::setPayload(typename FP::Type payload) {
    m_payload = payload;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::Face::next() const {
    return m_link.next();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::Face::previous() const {
    return m_link.previous();
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::Face::vertexCount() const {
    return m_boundary.size();
}

template <typename T, typename FP, typename VP>
const typename Polyhedron<T,FP,VP>::HalfEdgeList& Polyhedron<T,FP,VP>::Face::boundary() const {
    return m_boundary;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::Face::findHalfEdge(const typename Polyhedron<T,FP,VP>::V& origin, const T epsilon) const {
    auto* firstEdge = m_boundary.front();
    auto* currentEdge = firstEdge;
    do {
        if (isEqual(currentEdge->origin()->position(), origin, epsilon)) {
            return currentEdge;
        }
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return currentEdge;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::Face::findHalfEdge(const Vertex* origin) const {
    ensure(origin != nullptr, "origin is null");
    auto* firstEdge = m_boundary.front();
    auto* currentEdge = firstEdge;
    do {
        if (currentEdge->origin() == origin) {
            return currentEdge;
        }
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return currentEdge;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::Face::findEdge(const V& first, const V& second, const T epsilon) const {
    auto* halfEdge = findHalfEdge(first, epsilon);
    if (halfEdge == nullptr) {
        return nullptr;
    }

    if (isEqual(halfEdge->destination()->position(), second, epsilon)) {
        return halfEdge->edge();
    }

    halfEdge = halfEdge->previous();
    if (isEqual(halfEdge->origin()->position(), second, epsilon)) {
        return halfEdge->edge();
    }

    return nullptr;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::printBoundary() const {
    const auto* firstEdge = m_boundary.front();
    const auto* currentEdge = firstEdge;
    do {
        std::cout << currentEdge->asString() << std::endl;
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::V Polyhedron<T,FP,VP>::Face::origin() const {
    const auto* edge = *std::begin(m_boundary);
    return edge->origin()->position();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::PosList Polyhedron<T,FP,VP>::Face::vertexPositions() const {
    std::vector<V> result(0);
    result.reserve(vertexCount());
    getVertexPositions(std::back_inserter(result));
    return result;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Face::hasVertexPosition(const V& position, const T epsilon) const {
    const auto* firstEdge = m_boundary.front();
    const auto* currentEdge = firstEdge;
    do {
        if (isEqual(currentEdge->origin()->position(), position, epsilon)) {
            return true;
        }
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Face::hasVertexPositions(const std::vector<V>& positions, const T epsilon) const {
    if (positions.size() != vertexCount()) {
        return false;
    }

    const auto* firstEdge = m_boundary.front();
    const auto* currentEdge = firstEdge;
    do {
        if (currentEdge->hasOrigins(positions, epsilon)) {
            return true;
        }
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return false;
}

template <typename T, typename FP, typename VP>
T Polyhedron<T,FP,VP>::Face::distanceTo(const std::vector<V>& positions, const T maxDistance) const {
    if (positions.size() != vertexCount()) {
        return maxDistance;
    }

    T closestDistance = maxDistance;

    // Find the boundary edge with the origin closest to the first position.
    const HalfEdge* startEdge = nullptr;

    const auto* firstEdge = m_boundary.front();
    const auto* currentEdge = firstEdge;
    do {
        const T currentDistance = distance(currentEdge->origin()->position(), positions.front());
        if (currentDistance < closestDistance) {
            closestDistance = currentDistance;
            startEdge = currentEdge;
        }
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);

    // No vertex is within maxDistance of the first of the given positions.
    if (startEdge == nullptr) {
        return maxDistance;
    }

    // now find the maximum distance of all points
    firstEdge = startEdge;
    currentEdge = firstEdge->next();
    auto posIt = std::next(std::begin(positions));
    do {
        const auto& position = *posIt;
        ++posIt;

        closestDistance = std::max(closestDistance, distance(currentEdge->origin()->position(), position));
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return closestDistance;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::V Polyhedron<T,FP,VP>::Face::normal() const {
    const auto* first = m_boundary.front();
    const auto* current = first;
    V normal;
    do {
        const auto& p1 = current->origin()->position();
        const auto& p2 = current->next()->origin()->position();
        const auto& p3 = current->next()->next()->origin()->position();
        normal = cross(p2 - p1, p3 - p1);
        if (!isZero(normal, vm::constants<T>::almostZero())) {
            return normalize(normal);
        }
        current = current->next();
    } while (first != current);
    return normal;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::V Polyhedron<T,FP,VP>::Face::center() const {
    return vm::average(std::begin(m_boundary), std::end(m_boundary), GetVertexPosition());
}

template <typename T, typename FP, typename VP>
T Polyhedron<T,FP,VP>::Face::intersectWithRay(const vm::ray<T,3>& ray, const vm::side side) const {
    const RayIntersection result = intersectWithRay(ray);
    if (result.none()) {
        return result.distance();
    }

    switch (side) {
        case vm::side::front:
            return result.front() ? result.distance() : vm::nan<T>();
        case vm::side::back:
            return result.back() ? result.distance() : vm::nan<T>();
        case vm::side::both:
            return result.distance();
        switchDefault();
    }
}

template <typename T, typename FP, typename VP>
vm::point_status Polyhedron<T,FP,VP>::Face::pointStatus(const V& point, const T epsilon) const {
    const auto norm = normal();
    const auto distance = vm::dot(point - origin(), norm);
    if (distance > epsilon) {
        return vm::point_status::above;
    } else if (distance < -epsilon) {
        return vm::point_status::below;
    } else {
        return vm::point_status::inside;
    }
}

template <typename T, typename FP, typename VP> template <typename O>
void Polyhedron<T,FP,VP>::Face::getVertexPositions(O output) const {
    auto* firstEdge = m_boundary.front();
    auto* currentEdge = firstEdge;
    do {
        output = currentEdge->origin()->position();
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex::Set Polyhedron<T,FP,VP>::Face::vertexSet() const {
    typename Vertex::Set result;
    auto* firstEdge = m_boundary.front();
    auto* currentEdge = firstEdge;
    do {
        result.insert(currentEdge->origin());
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return result;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Face::coplanar(const Face* other) const {
    ensure(other != nullptr, "other is null");

    // Test if the normals are colinear by checking their enclosed angle.
    if (1.0 - dot(normal(), other->normal()) >= vm::constants<T>::colinearEpsilon()) {
        return false;
    }

    const vm::plane<T,3> myPlane(m_boundary.front()->origin()->position(), normal());
    if (!other->verticesOnPlane(myPlane)) {
        return false;
    }

    const vm::plane<T,3> otherPlane(other->boundary().front()->origin()->position(), other->normal());
    return verticesOnPlane(otherPlane);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Face::verticesOnPlane(const vm::plane<T,3>& plane) const {
    auto* firstEdge = m_boundary.front();
    auto* currentEdge = firstEdge;
    do {
        const auto* vertex = currentEdge->origin();
        if (plane.pointStatus(vertex->position()) != vm::point_status::inside) {
            return false;
        }
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return true;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::flip() {
    m_boundary.reverse();
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::insertIntoBoundaryBefore(HalfEdge* before, HalfEdge* edge) {
    ensure(before != nullptr, "before is null");
    ensure(edge != nullptr, "edge is null");
    assert(before->face() == this);
    assert(edge->face() == nullptr);
    
    edge->setFace(this);
    m_boundary.insertBefore(before, edge, 1);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::insertIntoBoundaryAfter(HalfEdge* after, HalfEdge* edge) {
    ensure(after != nullptr, "after is null");
    ensure(edge != nullptr, "edge is null");
    assert(after->face() == this);
    assert(edge->face() == nullptr);
    
    edge->setFace(this);
    m_boundary.insertAfter(after, edge, 1);
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::Face::removeFromBoundary(HalfEdge* from, HalfEdge* to) {
    ensure(from != nullptr, "from is null");
    ensure(to != nullptr, "to is null");
    assert(from->face() == this);
    assert(to->face() == this);
    
    const auto removeCount = countAndUnsetFace(from, to->next());
    m_boundary.remove(from, to, removeCount);
    return removeCount;
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::Face::removeFromBoundary(HalfEdge* edge) {
    removeFromBoundary(edge, edge);
    return 1;
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::Face::replaceBoundary(HalfEdge* edge, HalfEdge* with) {
    return replaceBoundary(edge, edge, with);
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::Face::replaceBoundary(HalfEdge* from, HalfEdge* to, HalfEdge* with) {
    ensure(from != nullptr, "from is null");
    ensure(to != nullptr, "to is null");
    ensure(with != nullptr, "with is null");
    assert(from->face() == this);
    assert(to->face() == this);
    assert(with->face() == nullptr);
    
    const auto removeCount = countAndUnsetFace(from, to->next());
    const auto insertCount = countAndSetFace(with, with, this);
    m_boundary.replace(from, to, removeCount, with, insertCount);
    return removeCount;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::replaceEntireBoundary(HalfEdgeList& newBoundary) {
    using std::swap;
    
    unsetBoundaryFaces();
    swap(m_boundary, newBoundary);
    setBoundaryFaces();
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::Face::countAndSetFace(HalfEdge* from, HalfEdge* until, Face* face) {
    size_t count = 0;
    auto* cur = from;
    do {
        cur->setFace(face);
        cur = cur->next();
        ++count;
    } while (cur != until);
    return count;
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::Face::countAndUnsetFace(HalfEdge* from, HalfEdge* until) {
    size_t count = 0;
    auto* cur = from;
    do {
        cur->unsetFace();
        cur = cur->next();
        ++count;
    } while (cur != until);
    return count;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::setBoundaryFaces() {
    auto* first = m_boundary.front();
    auto* current = first;
    do {
        current->setFace(this);
        current = current->next();
    } while (current != first);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::unsetBoundaryFaces() {
    auto* first = m_boundary.front();
    auto* current = first;
    do {
        current->unsetFace();
        current = current->next();
    } while (current != first);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::removeBoundaryFromEdges() {
    auto* first = m_boundary.front();
    auto* current = first;
    do {
        auto* edge = current->edge();
        if (edge != nullptr) {
            edge->makeSecondEdge(current);
            edge->unsetSecondEdge();
        }
        current = current->next();
    } while (current != first);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::setLeavingEdges() {
    auto* first = m_boundary.front();
    auto* current = first;
    do {
        current->setAsLeaving();
        current = current->next();
    } while (current != first);
}


template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Face::RayIntersection {
private:
    typedef enum {
        Type_Front = 1,
        Type_Back  = 2,
        Type_None  = 3
    } Type;
    
    Type m_type;
    T m_distance;
    
    RayIntersection(const Type type, const T distance) :
    m_type(type),
    m_distance(distance) {
        assert(!vm::isNan(m_distance) || m_type == Type_None);
    }
public:
    static RayIntersection Front(const T distance) {
        return RayIntersection(Type_Front, distance);
    }

    static RayIntersection Back(const T distance) {
        return RayIntersection(Type_Back, distance);
    }

    static RayIntersection None() {
        return RayIntersection(Type_None, vm::nan<T>());
    }
    
    bool front() const {
        return m_type == Type_Front;
    }
    
    bool back() const {
        return m_type == Type_Back;
    }
    
    bool none() const {
        return m_type == Type_None;
    }
    
    T distance() const {
        return m_distance;
    }
};

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Face::RayIntersection Polyhedron<T,FP,VP>::Face::intersectWithRay(const vm::ray<T,3>& ray) const {
    const vm::plane<T,3> plane(origin(), normal());
    const auto cos = dot(plane.normal, ray.direction);
    
    if (vm::isZero(cos, vm::constants<T>::almostZero())) {
        return RayIntersection::None();
    }

    const auto distance = vm::intersect(ray, plane, std::begin(m_boundary), std::end(m_boundary), GetVertexPosition());
    if (vm::isNan(distance)) {
        return RayIntersection::None();
    } else if (cos < 0.0) {
        return RayIntersection::Front(distance);
    } else {
        return RayIntersection::Back(distance);
    }
}


template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::Face::countSharedVertices(const Face* other) const {
    ensure(other != nullptr, "other is null");
    assert(other != this);

    const auto intersection = SetUtils::intersection(vertexSet(), other->vertexSet());
    return intersection.size();
}

#endif
