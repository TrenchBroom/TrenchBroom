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

#ifndef TrenchBroom_Polyhedron_HalfEdge_h
#define TrenchBroom_Polyhedron_HalfEdge_h

template <typename T, typename FP, typename VP>
typename DoublyLinkedList<typename Polyhedron<T,FP,VP>::HalfEdge, typename Polyhedron<T,FP,VP>::GetHalfEdgeLink>::Link& Polyhedron<T,FP,VP>::GetHalfEdgeLink::operator()(HalfEdge* halfEdge) const {
    return halfEdge->m_link;
}

template <typename T, typename FP, typename VP>
const typename DoublyLinkedList<typename Polyhedron<T,FP,VP>::HalfEdge, typename Polyhedron<T,FP,VP>::GetHalfEdgeLink>::Link& Polyhedron<T,FP,VP>::GetHalfEdgeLink::operator()(const HalfEdge* halfEdge) const {
    return halfEdge->m_link;
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::HalfEdge::HalfEdge(Vertex* origin) :
m_origin(origin),
m_edge(NULL),
m_face(NULL),
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
    assert(m_origin != NULL);
    setAsLeaving();
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::HalfEdge::~HalfEdge() {
    if (m_origin->leaving() == this)
        m_origin->setLeaving(NULL);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::HalfEdge::origin() const {
    return m_origin;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::HalfEdge::destination() const {
    return next()->origin();
}

template <typename T, typename FP, typename VP>
T Polyhedron<T,FP,VP>::HalfEdge::length() const {
    return vector().length();
}

template <typename T, typename FP, typename VP>
T Polyhedron<T,FP,VP>::HalfEdge::squaredLength() const {
    return vector().squaredLength();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::V Polyhedron<T,FP,VP>::HalfEdge::vector() const {
    return destination()->position() - origin()->position();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::HalfEdge::edge() const {
    return m_edge;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::HalfEdge::face() const {
    return m_face;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::HalfEdge::next() const {
    return m_link.next();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::HalfEdge::previous() const {
    return m_link.previous();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::HalfEdge::twin() const {
    assert(m_edge != NULL);
    return m_edge->twin(this);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::HalfEdge::previousIncident() const {
    return twin()->next();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::HalfEdge* Polyhedron<T,FP,VP>::HalfEdge::nextIncident() const {
    return previous()->twin();
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::HalfEdge::hasOrigins(const typename V::List& positions, const T epsilon) const {
    const HalfEdge* edge = this;
    for (size_t i = 0; i < positions.size(); ++i) {
        if (!edge->origin()->position().equals(positions[i], epsilon))
            return false;
        edge = edge->next();
    }
    return true;
}

template <typename T, typename FP, typename VP>
String Polyhedron<T,FP,VP>::HalfEdge::asString() const {
    StringStream str;
    origin()->position().write(str);
    str << " --> ";
    if (destination() != NULL)
        destination()->position().write(str);
    else
        str << "NULL";
    return str.str();
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::HalfEdge::isLeavingEdge() const {
    return m_origin->leaving() == this;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::HalfEdge::colinear(const HalfEdge* other) const {
    const V dir = vector().normalized();
    const V otherDir = other->vector().normalized();
    return dir.colinearTo(otherDir);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::HalfEdge::setOrigin(Vertex* origin) {
    assert(origin != NULL);
    m_origin = origin;
    setAsLeaving();
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::HalfEdge::setEdge(Edge* edge) {
    m_edge = edge;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::HalfEdge::setFace(Face* face) {
    m_face = face;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::HalfEdge::setAsLeaving() {
    m_origin->setLeaving(this);
}

#endif
