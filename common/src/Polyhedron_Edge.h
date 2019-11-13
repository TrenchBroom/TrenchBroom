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

#ifndef TrenchBroom_Polyhedron_Edge_h
#define TrenchBroom_Polyhedron_Edge_h

#include <vecmath/vec.h>
#include <vecmath/plane.h>
#include <vecmath/segment.h>
#include <vecmath/distance.h>
#include <vecmath/scalar.h>

template <typename T, typename FP, typename VP>
intrusive_circular_link<typename Polyhedron<T,FP,VP>::Edge>& Polyhedron<T,FP,VP>::GetEdgeLink::operator()(Edge* edge) const {
    return edge->m_link;
}

template <typename T, typename FP, typename VP>
const intrusive_circular_link<typename Polyhedron<T,FP,VP>::Edge>& Polyhedron<T,FP,VP>::GetEdgeLink::operator()(const Edge* edge) const {
    return edge->m_link;
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Edge::Edge(HalfEdge* first, HalfEdge* second) :
m_first(first),
m_second(second),
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
    ensure(m_first != nullptr, "first is null");
    m_first->setEdge(this);
    if (m_second != nullptr) {
        m_second->setEdge(this);
    }
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::Edge::firstVertex() const {
    ensure(m_first != nullptr, "first is null");
    return m_first->origin();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::Edge::secondVertex() const {
    ensure(m_first != nullptr, "first is null");
    if (m_second != nullptr) {
        return m_second->origin();
    } else {
        return m_first->next()->origin();
    }
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::Edge::otherVertex(Vertex* vertex) const {
    ensure(vertex != nullptr, "vertex is null");
    assert(vertex == firstVertex() || vertex == secondVertex());
    if (vertex == firstVertex()) {
        return secondVertex();
    } else {
        return firstVertex();
    }
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::Edge::firstEdge() const {
    ensure(m_first != nullptr, "first is null");
    return m_first;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::Edge::secondEdge() const {
    ensure(m_second != nullptr, "second is null");
    return m_second;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::Edge::twin(const HalfEdge* halfEdge) const {
    ensure(halfEdge != nullptr, "halfEdge is null");
    assert(halfEdge == m_first || halfEdge == m_second);
    if (halfEdge == m_first) {
        return m_second;
    } else {
        return m_first;
    }
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::V Polyhedron<T,FP,VP>::Edge::vector() const {
    return secondVertex()->position() - firstVertex()->position();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::V Polyhedron<T,FP,VP>::Edge::center() const {
    assert(fullySpecified());
    return (m_first->origin()->position() + m_second->origin()->position()) / static_cast<T>(2.0);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::Edge::firstFace() const {
    ensure(m_first != nullptr, "first is null");
    return m_first->face();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::Edge::secondFace() const {
    ensure(m_second != nullptr, "second is null");
    return m_second->face();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::Edge::commonVertex(const Edge* other) const {
    ensure(other != nullptr, "other is null");
    if (other->hasVertex(firstVertex())) {
        return firstVertex();
    } else if (other->hasVertex(secondVertex())) {
        return secondVertex();
    } else {
        return nullptr;
    }
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Edge::hasVertex(const Vertex* vertex) const {
    return firstVertex() == vertex || secondVertex() == vertex;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Edge::hasPosition(const V& position, const T epsilon) const {
    return (vm::is_equal( firstVertex()->position(), position, epsilon) ||
            vm::is_equal(secondVertex()->position(), position, epsilon));
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Edge::hasPositions(const V& position1, const V& position2, const T epsilon) const {
    return ((vm::is_equal( firstVertex()->position(), position1, epsilon) &&
             vm::is_equal(secondVertex()->position(), position2, epsilon)) ||
            (vm::is_equal( firstVertex()->position(), position2, epsilon) &&
             vm::is_equal(secondVertex()->position(), position1, epsilon))
            );
}

template <typename T, typename FP, typename VP>
T Polyhedron<T,FP,VP>::Edge::distanceTo(const V& position1, const V& position2) const {
    const T pos1Distance = vm::min(vm::squared_distance(firstVertex()->position(), position1), vm::squared_distance(secondVertex()->position(), position1));
    const T pos2Distance = vm::min(vm::squared_distance(firstVertex()->position(), position2), vm::squared_distance(secondVertex()->position(), position2));
    return std::max(pos1Distance, pos2Distance);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Edge::orphaned() const {
    return m_first == nullptr && m_second == nullptr;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Edge::fullySpecified() const {
    ensure(m_first != nullptr, "first is null");
    return m_second != nullptr;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Edge::contains(const V& point, const T maxDistance) const {
    return vm::distance(vm::segment<T,3>(firstVertex()->position(), secondVertex()->position()), point).distance < maxDistance;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::Edge::next() const {
    return m_link.next();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::Edge::previous() const {
    return m_link.previous();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::Edge::split(const vm::plane<T,3>& plane) {
    // Assumes that the start and the end vertex of this edge are on opposite sides of
    // the given plane (precondition).
    // The caller must ensure this.

    const V& startPos = firstVertex()->position();
    const V& endPos = secondVertex()->position();

    const T startDist = plane.point_distance(startPos);
    const T endDist = plane.point_distance(endPos);

    // Check what's implied by the precondition:
    assert(vm::abs(startDist) > vm::constants<T>::point_status_epsilon());
    assert(vm::abs(endDist)   > vm::constants<T>::point_status_epsilon());
    assert(vm::sign(startDist) != vm::sign(endDist));
    assert(startDist != endDist); // implied by the above

    const T dot = startDist / (startDist - endDist);

    // 1. startDist and endDist have opposite signs, therefore dot cannot be negative
    // 2. |startDist - endDist| > 0 (due to precondition), therefore dot > 0
    // 3. |x-y| > x if x and y have different signs, therefore x / (x-y) < 1
    assert(dot > T(0.0) && dot < T(1.0));

    const V position = startPos + dot * (endPos - startPos);
    return insertVertex(position);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::Edge::insertVertex(const V& position) {
    /*
     before:

     |----------this edge---------|
     |                            |
     ------------old1st----------->
     <-----------old2nd------------

     after:

     |-this edge--|  |--new edge--|
     |            |  |            |
     ----old1st--->  ----new1st--->
     <---new2nd----  ----old2nd----
                   /\
               new vertex

     */

    // create new vertices and new half edges originating from it
    // the caller is responsible for storing the newly created vertex!
    Vertex* newVertex = new Vertex(position);
    HalfEdge* newFirstEdge = new HalfEdge(newVertex);
    HalfEdge* oldFirstEdge = firstEdge();
    HalfEdge* newSecondEdge = new HalfEdge(newVertex);
    HalfEdge* oldSecondEdge = secondEdge();

    // insert the new half edges into the corresponding faces
    firstFace()->insertIntoBoundaryAfter(oldFirstEdge, HalfEdgeList({ newFirstEdge }));
    secondFace()->insertIntoBoundaryAfter(oldSecondEdge, HalfEdgeList({ newSecondEdge }));

    // make old1st the leaving edge of its origin vertex
    setFirstAsLeaving();

    // unset old2nd from this edge
    unsetSecondEdge();

    // and replace it with new2nd
    setSecondEdge(newSecondEdge);

    return new Edge(newFirstEdge, oldSecondEdge);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Edge::flip() {
    using std::swap;
    swap(m_first, m_second);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Edge::makeFirstEdge(HalfEdge* edge) {
    ensure(edge != nullptr, "edge is null");
    assert(m_first == edge || m_second == edge);
    if (edge != m_first) {
        flip();
    }
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Edge::makeSecondEdge(HalfEdge* edge) {
    ensure(edge != nullptr, "edge is null");
    assert(m_first == edge || m_second == edge);
    if (edge != m_second) {
        flip();
    }
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Edge::setFirstAsLeaving() {
    ensure(m_first != nullptr, "first is null");
    m_first->setAsLeaving();
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Edge::unsetSecondEdge() {
    ensure(m_second != nullptr, "second is null");
    m_second->unsetEdge();
    m_second = nullptr;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Edge::setSecondEdge(HalfEdge* second) {
    ensure(second != nullptr, "second is null");
    assert(m_second == nullptr);
    assert(second->edge() == nullptr);
    m_second = second;
    m_second->setEdge(this);
}

#endif
