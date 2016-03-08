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

#ifndef TrenchBroom_Polyhedron_Vertex_h
#define TrenchBroom_Polyhedron_Vertex_h

template <typename T, typename FP, typename VP>
typename DoublyLinkedList<typename Polyhedron<T,FP,VP>::Vertex, typename Polyhedron<T,FP,VP>::GetVertexLink>::Link& Polyhedron<T,FP,VP>::GetVertexLink::operator()(Vertex* vertex) const {
    return vertex->m_link;
}

template <typename T, typename FP, typename VP>
const typename DoublyLinkedList<typename Polyhedron<T,FP,VP>::Vertex, typename Polyhedron<T,FP,VP>::GetVertexLink>::Link& Polyhedron<T,FP,VP>::GetVertexLink::operator()(const Vertex* vertex) const {
    return vertex->m_link;
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Vertex::Vertex(const V& position) :
m_position(position),
#ifdef _MSC_VER
// MSVC throws a warning because we're passing this to the FaceLink constructor, but it's okay because we just store the pointer there.
#pragma warning(push)
#pragma warning(disable : 4355)
m_link(this),
#pragma warning(pop)
#else
m_link(this),
#endif
m_leaving(NULL),
m_payload(VP::defaultValue()) {}

template <typename T, typename FP, typename VP>
typename VP::Type Polyhedron<T,FP,VP>::Vertex::payload() const {
    return m_payload;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Vertex::setPayload(typename VP::Type payload) {
    m_payload = payload;
}

template <typename T, typename FP, typename VP>
const typename Polyhedron<T,FP,VP>::V& Polyhedron<T,FP,VP>::Vertex::position() const {
    return m_position;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::Vertex::next() const {
    return m_link.next();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::Vertex::previous() const {
    return m_link.previous();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::Vertex::leaving() const {
    return m_leaving;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::Vertex::findConnectingEdge(const Vertex* vertex) const {
    assert(vertex != NULL);
    assert(m_leaving != NULL);
    
    HalfEdge* curEdge = m_leaving;
    do {
        if (vertex == curEdge->destination())
            return curEdge;
        curEdge = curEdge->nextIncident();
    } while (curEdge != NULL && curEdge != m_leaving);
    return NULL;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::Vertex::findColinearEdge(const HalfEdge* arriving) const {
    assert(arriving != NULL);
    assert(m_leaving != NULL);
    assert(arriving->destination() == this);
    
    HalfEdge* curEdge = m_leaving;
    do {
        if (arriving->colinear(curEdge))
            return curEdge;
        curEdge = curEdge->nextIncident();
    } while (curEdge != m_leaving);
    return NULL;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Vertex::correctPosition(const size_t decimals, const T epsilon) {
    m_position.correct(decimals, epsilon);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Vertex::setPosition(const V& position) {
    m_position = position;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Vertex::setLeaving(HalfEdge* edge) {
    assert(edge == NULL || edge->origin() == this);
    m_leaving = edge;
}

#endif
