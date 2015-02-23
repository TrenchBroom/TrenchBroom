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

template <typename T>
typename Polyhedron<T>::HalfEdgeLink& Polyhedron<T>::HalfEdgeList::doGetLink(HalfEdge* edge) const {
    return edge->m_link;
}

template <typename T>
const typename Polyhedron<T>::HalfEdgeLink& Polyhedron<T>::HalfEdgeList::doGetLink(const HalfEdge* edge) const {
    return edge->m_link;
}

template <typename T>
class Polyhedron<T>::HalfEdge {
private:
    friend class Edge;
    friend class HalfEdgeList;
    friend class Face;
    friend class Polyhedron<T>;
private:
    Vertex* m_origin;
    Edge* m_edge;
    Face* m_face;
    HalfEdgeLink m_link;
private:
    HalfEdge(Vertex* origin) :
    m_origin(origin),
    m_edge(NULL),
    m_face(NULL),
    m_link(this) {
        assert(m_origin != NULL);
        setAsLeaving();
    }
public:
    ~HalfEdge() {
        if (m_origin->leaving() == this)
            m_origin->setLeaving(NULL);
    }
    
    Vertex* origin() const {
        return m_origin;
    }
    
    Vertex* destination() const {
        return next()->origin();
    }
    
    T length() const {
        return vector().length();
    }
    
    T squaredLength() const {
        return vector().squaredLength();
    }
    
    V vector() const {
        return destination()->position() - origin()->position();
    }
    
    Edge* edge() const {
        return m_edge;
    }
    
    Face* face() const {
        return m_face;
    }
    
    HalfEdge* next() const {
        return m_link.next();
    }
    
    HalfEdge* previous() const {
        return m_link.previous();
    }
    
    HalfEdge* twin() const {
        assert(m_edge != NULL);
        return m_edge->twin(this);
    }
    
    HalfEdge* previousIncident() const {
        return twin()->next();
    }
    
    HalfEdge* nextIncident() const {
        return previous()->twin();
    }
private:
    bool isLeavingEdge() const {
        return m_origin->leaving() == this;
    }
    
    bool colinear(const HalfEdge* other) const {
        const V dir = vector().normalized();
        const V otherDir = other->vector().normalized();
        return Math::eq(otherDir.dot(dir), 1.0);
    }
    
    void setOrigin(Vertex* origin) {
        assert(origin != NULL);
        m_origin = origin;
        setAsLeaving();
    }
    
    void setEdge(Edge* edge) {
        m_edge = edge;
    }
    
    void setFace(Face* face) {
        m_face = face;
    }
    
    void setAsLeaving() {
        m_origin->setLeaving(this);
    }
};

#endif
