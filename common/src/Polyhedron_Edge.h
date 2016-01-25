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

#ifndef TrenchBroom_Polyhedron_Edge_h
#define TrenchBroom_Polyhedron_Edge_h

template <typename T, typename FP, typename VP>
typename DoublyLinkedList<typename Polyhedron<T,FP,VP>::Edge, typename Polyhedron<T,FP,VP>::GetEdgeLink>::Link& Polyhedron<T,FP,VP>::GetEdgeLink::operator()(Edge* edge) const {
    return edge->m_link;
}

template <typename T, typename FP, typename VP>
const typename DoublyLinkedList<typename Polyhedron<T,FP,VP>::Edge, typename Polyhedron<T,FP,VP>::GetEdgeLink>::Link& Polyhedron<T,FP,VP>::GetEdgeLink::operator()(const Edge* edge) const {
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
    assert(m_first != NULL);
    m_first->setEdge(this);
    if (m_second != NULL)
        m_second->setEdge(this);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::Edge::firstVertex() const {
    assert(m_first != NULL);
    return m_first->origin();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::Edge::secondVertex() const {
    assert(m_first != NULL);
    if (m_second != NULL)
        return m_second->origin();
    return m_first->next()->origin();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::Edge::otherVertex(Vertex* vertex) const {
    assert(vertex != NULL);
    assert(vertex == firstVertex() || vertex == secondVertex());
    if (vertex == firstVertex())
        return secondVertex();
    return firstVertex();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::Edge::firstEdge() const {
    assert(m_first != NULL);
    return m_first;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::Edge::secondEdge() const {
    assert(m_second != NULL);
    return m_second;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::Edge::twin(const HalfEdge* halfEdge) const {
    assert(halfEdge != NULL);
    assert(halfEdge == m_first || halfEdge == m_second);
    if (halfEdge == m_first)
        return m_second;
    return m_first;
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
    assert(m_first != NULL);
    return m_first->face();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::Edge::secondFace() const {
    assert(m_second != NULL);
    return m_second->face();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::Edge::commonVertex(const Edge* other) const {
    assert(other != NULL);
    if (other->hasVertex(firstVertex()))
        return firstVertex();
    if (other->hasVertex(secondVertex()))
        return secondVertex();
    return NULL;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Edge::hasVertex(const Vertex* vertex) const {
    return firstVertex() == vertex || secondVertex() == vertex;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Edge::hasPosition(const V& position, const T epsilon) const {
    return ( firstVertex()->position().equals(position, epsilon) ||
            secondVertex()->position().equals(position, epsilon));
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Edge::hasPositions(const V& position1, const V& position2, const T epsilon) const {
    return (( firstVertex()->position().equals(position1, epsilon) &&
             secondVertex()->position().equals(position2, epsilon)) ||
            ( firstVertex()->position().equals(position2, epsilon) &&
             secondVertex()->position().equals(position1, epsilon))
            );
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Edge::orphaned() const {
    return m_first == NULL && m_second == NULL;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Edge::fullySpecified() const {
    assert(m_first != NULL);
    return m_second != NULL;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::Edge::contains(const V& point, const T maxDistance) const {
    return point.distanceToSegment(firstVertex()->position(), secondVertex()->position()).distance < maxDistance;
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
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::Edge::split(const Plane<T,3>& plane) {
    // Do exactly what QBSP is doing:
    const T startDist = plane.pointDistance(firstVertex()->position());
    const T endDist = plane.pointDistance(secondVertex()->position());
    
    assert(startDist != endDist);
    const T dot = startDist / (startDist - endDist);
    
    const V& startPos = firstVertex()->position();
    const V& endPos = secondVertex()->position();
    V position;
    for (size_t i = 0; i < 3; ++i) {
        if (plane.normal[i] == 1.0)
            position[i] = plane.distance;
        else if (plane.normal[i] == -1.0)
            position[i] = -plane.distance;
        else
            position[i] = startPos[i] + dot * (endPos[i] - startPos[i]);
    }
    
    return insertVertex(position);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::Edge::splitAtCenter() {
    return insertVertex(center());
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::Edge::insertVertex(const V& position) {
    Vertex* newVertex = new Vertex(position);
    HalfEdge* newFirstEdge = new HalfEdge(newVertex);
    HalfEdge* oldFirstEdge = firstEdge();
    HalfEdge* newSecondEdge = new HalfEdge(newVertex);
    HalfEdge* oldSecondEdge = secondEdge();
    
    newFirstEdge->setFace(firstFace());
    newSecondEdge->setFace(secondFace());
    
    firstFace()->insertIntoBoundaryAfter(oldFirstEdge, newFirstEdge);
    secondFace()->insertIntoBoundaryAfter(oldSecondEdge, newSecondEdge);
    
    setFirstAsLeaving();
    unsetSecondEdge();
    setSecondEdge(newSecondEdge);
    
    Edge* newEdge = new Edge(newFirstEdge, oldSecondEdge);
    return newEdge;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Edge::flip() {
    using std::swap;
    swap(m_first, m_second);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Edge::makeFirstEdge(HalfEdge* edge) {
    assert(edge != NULL);
    assert(m_first == edge || m_second == edge);
    if (edge != m_first)
        flip();
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Edge::makeSecondEdge(HalfEdge* edge) {
    assert(edge != NULL);
    assert(m_first == edge || m_second == edge);
    if (edge != m_second)
        flip();
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Edge::setFirstAsLeaving() {
    assert(m_first != NULL);
    m_first->setAsLeaving();
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Edge::unsetSecondEdge() {
    m_second->setEdge(NULL);
    m_second = NULL;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Edge::setSecondEdge(HalfEdge* second) {
    assert(second != NULL);
    assert(m_second == NULL);
    assert(second->edge() == NULL);
    m_second = second;
    m_second->setEdge(this);
}

#endif
