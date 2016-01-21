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

#ifndef TrenchBroom_Polyhedron_Face_h
#define TrenchBroom_Polyhedron_Face_h

#include <iterator>

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
    updateBoundaryFaces(this);
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
void Polyhedron<T,FP,VP>::Face::printBoundary() const {
    const HalfEdge* firstEdge = m_boundary.front();
    const HalfEdge* currentEdge = firstEdge;
    do {
        std::cout << currentEdge->asString() << std::endl;
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::V Polyhedron<T,FP,VP>::Face::origin() const {
    const HalfEdge* edge = *m_boundary.begin();
    return edge->origin()->position();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::V::List Polyhedron<T,FP,VP>::Face::vertexPositions() const {
    typename V::List positions(0);
    positions.reserve(vertexCount());
    getVertexPositions(std::back_inserter(positions));
    return positions;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Face::hasVertexPosition(const V& position, const T epsilon) const {
    const HalfEdge* firstEdge = m_boundary.front();
    const HalfEdge* currentEdge = firstEdge;
    do {
        if (currentEdge->origin()->position().equals(position, epsilon))
            return true;
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Face::hasVertexPositions(const typename V::List& positions, const T epsilon) const {
    if (positions.size() != vertexCount())
        return false;
    
    const HalfEdge* firstEdge = m_boundary.front();
    const HalfEdge* currentEdge = firstEdge;
    do {
        if (currentEdge->hasOrigins(positions, epsilon))
            return true;
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return false;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::V Polyhedron<T,FP,VP>::Face::normal() const {
    const HalfEdge* first = m_boundary.front();
    const HalfEdge* current = first;
    V cross;
    do {
        const V& p1 = current->origin()->position();
        const V& p2 = current->next()->origin()->position();
        const V& p3 = current->next()->next()->origin()->position();
        cross = crossed(p2 - p1, p3 - p1);
        if (!cross.null())
            return cross.normalized();
        current = current->next();
    } while (first != current);
    return cross;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::V Polyhedron<T,FP,VP>::Face::center() const {
    return V::center(m_boundary.begin(), m_boundary.end(), GetVertexPosition());
}

template <typename T, typename FP, typename VP>
T Polyhedron<T,FP,VP>::Face::intersectWithRay(const Ray<T,3>& ray, const Math::Side side) const {
    const Plane<T,3> plane(origin(), normal());
    const T dot = plane.normal.dot(ray.direction);
    if (Math::zero(dot))
        return Math::nan<T>();
    if (side != Math::Side_Both) {
        if (side == Math::Side_Front) {
            if (dot > 0.0)
                return Math::nan<T>();
        } else if (dot < 0.0) { // and side == Math::Side_Back
            return Math::nan<T>();
        }
    }
    
    return intersectPolygonWithRay(ray, plane, m_boundary.begin(), m_boundary.end(), GetVertexPosition());
}

template <typename T, typename FP, typename VP> template <typename O>
void Polyhedron<T,FP,VP>::Face::getVertexPositions(O output) const {
    HalfEdge* firstEdge = m_boundary.front();
    HalfEdge* currentEdge = firstEdge;
    do {
        output = currentEdge->origin()->position();
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Face::visibleFrom(const V& point) const {
    return pointStatus(point) == Math::PointStatus::PSAbove;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Face::coplanar(const Face* other) const {
    assert(other != NULL);
    if (!normal().colinearTo(other->normal()))
        return false;

    const Plane3 myPlane(m_boundary.front()->origin()->position(), normal());
    if (!other->verticesOnPlane(myPlane))
        return false;
    
    const Plane3 otherPlane(other->boundary().front()->origin()->position(), other->normal());
    return verticesOnPlane(otherPlane);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Face::verticesOnPlane(const Plane<T,3>& plane) const {
    HalfEdge* firstEdge = m_boundary.front();
    HalfEdge* currentEdge = firstEdge;
    do {
        const Vertex* vertex = currentEdge->origin();
        if (plane.pointStatus(vertex->position()) != Math::PointStatus::PSInside)
            return false;
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return true;
}

template <typename T, typename FP, typename VP>
Math::PointStatus::Type Polyhedron<T,FP,VP>::Face::pointStatus(const V& point, const T epsilon) const {
    const V norm = normal();
    const T distance = (point - origin()).dot(norm);
    if (distance > epsilon)
        return Math::PointStatus::PSAbove;
    if (distance < -epsilon)
        return Math::PointStatus::PSBelow;
    return Math::PointStatus::PSInside;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::flip() {
    m_boundary.reverse();
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::insertIntoBoundaryBefore(HalfEdge* before, HalfEdge* edge) {
    assert(before != NULL);
    assert(edge != NULL);
    assert(before->face() == this);
    assert(edge->face() == this);
    
    m_boundary.insertBefore(before, edge, 1);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::insertIntoBoundaryAfter(HalfEdge* after, HalfEdge* edge) {
    assert(after != NULL);
    assert(edge != NULL);
    assert(after->face() == this);
    assert(edge->face() == this);
    
    m_boundary.insertAfter(after, edge, 1);
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::Face::removeFromBoundary(HalfEdge* from, HalfEdge* to) {
    assert(from != NULL);
    assert(to != NULL);
    assert(from->face() == this);
    assert(to->face() == this);
    
    const size_t removeCount = countAndSetFace(from, to->next(), NULL);
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
    assert(from != NULL);
    assert(to != NULL);
    assert(with != NULL);
    assert(from->face() == this);
    assert(to->face() == this);
    assert(with->face() == NULL);
    
    const size_t removeCount = countAndSetFace(from, to->next(), NULL);
    const size_t insertCount = countAndSetFace(with, with, this);
    m_boundary.replace(from, to, removeCount, with, insertCount);
    return removeCount;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::replaceEntireBoundary(HalfEdgeList& newBoundary) {
    using std::swap;
    
    updateBoundaryFaces(NULL);
    swap(m_boundary, newBoundary);
    updateBoundaryFaces(this);
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::Face::countAndSetFace(HalfEdge* from, HalfEdge* until, Face* face) {
    size_t count = 0;
    HalfEdge* cur = from;
    do {
        cur->setFace(face);
        cur = cur->next();
        ++count;
    } while (cur != until);
    return count;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::updateBoundaryFaces(Face* face) {
    HalfEdge* first = m_boundary.front();
    HalfEdge* current = first;
    do {
        current->setFace(face);
        current = current->next();
    } while (current != first);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::removeBoundaryFromEdges() {
    HalfEdge* first = m_boundary.front();
    HalfEdge* current = first;
    do {
        Edge* edge = current->edge();
        if (edge != NULL) {
            edge->makeSecondEdge(current);
            edge->unsetSecondEdge();
        }
        current = current->next();
    } while (current != first);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Face::setLeavingEdges() {
    HalfEdge* first = m_boundary.front();
    HalfEdge* current = first;
    do {
        current->setAsLeaving();
        current = current->next();
    } while (current != first);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Face::checkBoundary() const {
    HalfEdge* first = m_boundary.front();
    HalfEdge* current = first;
    do {
        if (current->face() != this)
            return false;
        current = current->next();
    } while (current != first);
    return true;
}

#endif
