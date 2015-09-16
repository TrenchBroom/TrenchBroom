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

template <typename T, typename FP>
typename DoublyLinkedList<typename Polyhedron<T,FP>::HalfEdge, typename Polyhedron<T,FP>::GetHalfEdgeLink>::Link& Polyhedron<T,FP>::GetHalfEdgeLink::operator()(HalfEdge* halfEdge) const {
    return halfEdge->m_link;
}

template <typename T, typename FP>
const typename DoublyLinkedList<typename Polyhedron<T,FP>::HalfEdge, typename Polyhedron<T,FP>::GetHalfEdgeLink>::Link& Polyhedron<T,FP>::GetHalfEdgeLink::operator()(const HalfEdge* halfEdge) const {
    return halfEdge->m_link;
}

template <typename T, typename FP>
Polyhedron<T,FP>::HalfEdge::HalfEdge(Vertex* origin) :
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

template <typename T, typename FP>
Polyhedron<T,FP>::HalfEdge::~HalfEdge() {
    if (m_origin->leaving() == this)
        m_origin->setLeaving(NULL);
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::Vertex* Polyhedron<T,FP>::HalfEdge::origin() const {
    return m_origin;
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::Vertex* Polyhedron<T,FP>::HalfEdge::destination() const {
    return next()->origin();
}

template <typename T, typename FP>
T Polyhedron<T,FP>::HalfEdge::length() const {
    return vector().length();
}

template <typename T, typename FP>
T Polyhedron<T,FP>::HalfEdge::squaredLength() const {
    return vector().squaredLength();
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::V Polyhedron<T,FP>::HalfEdge::vector() const {
    return destination()->position() - origin()->position();
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::Edge* Polyhedron<T,FP>::HalfEdge::edge() const {
    return m_edge;
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::Face* Polyhedron<T,FP>::HalfEdge::face() const {
    return m_face;
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::HalfEdge* Polyhedron<T,FP>::HalfEdge::next() const {
    return m_link.next();
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::HalfEdge* Polyhedron<T,FP>::HalfEdge::previous() const {
    return m_link.previous();
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::HalfEdge* Polyhedron<T,FP>::HalfEdge::twin() const {
    assert(m_edge != NULL);
    return m_edge->twin(this);
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::HalfEdge* Polyhedron<T,FP>::HalfEdge::previousIncident() const {
    return twin()->next();
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::HalfEdge* Polyhedron<T,FP>::HalfEdge::nextIncident() const {
    return previous()->twin();
}

template <typename T, typename FP>
bool Polyhedron<T,FP>::HalfEdge::hasOrigins(const typename V::List& positions, const T epsilon) const {
    const HalfEdge* edge = this;
    for (size_t i = 0; i < positions.size(); ++i) {
        if (edge->origin()->position() != positions[i])
            return false;
        edge = edge->next();
    }
    return true;
}

template <typename T, typename FP>
String Polyhedron<T,FP>::HalfEdge::asString() const {
    StringStream str;
    origin()->position().write(str);
    str << " --> ";
    if (destination() != NULL)
        destination()->position().write(str);
    else
        str << "NULL";
    return str.str();
}

template <typename T, typename FP>
bool Polyhedron<T,FP>::HalfEdge::isLeavingEdge() const {
    return m_origin->leaving() == this;
}

template <typename T, typename FP>
bool Polyhedron<T,FP>::HalfEdge::colinear(const HalfEdge* other) const {
    const V dir = vector().normalized();
    const V otherDir = other->vector().normalized();
    return Math::eq(otherDir.dot(dir), static_cast<T>(1.0));
}

template <typename T, typename FP>
void Polyhedron<T,FP>::HalfEdge::setOrigin(Vertex* origin) {
    assert(origin != NULL);
    m_origin = origin;
    setAsLeaving();
}

template <typename T, typename FP>
void Polyhedron<T,FP>::HalfEdge::setEdge(Edge* edge) {
    m_edge = edge;
}

template <typename T, typename FP>
void Polyhedron<T,FP>::HalfEdge::setFace(Face* face) {
    m_face = face;
}

template <typename T, typename FP>
void Polyhedron<T,FP>::HalfEdge::setAsLeaving() {
    m_origin->setLeaving(this);
}

#endif
