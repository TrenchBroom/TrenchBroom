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

template <typename T>
typename Polyhedron<T>::EdgeLink& Polyhedron<T>::EdgeList::doGetLink(Edge* edge) const {
    return edge->m_link;
}

template <typename T>
const typename Polyhedron<T>::EdgeLink& Polyhedron<T>::EdgeList::doGetLink(const Edge* edge) const {
    return edge->m_link;
}

template <typename T>
class Polyhedron<T>::Edge {
private:
    friend class EdgeList;
    friend class Polyhedron<T>;
private:
    HalfEdge* m_first;
    HalfEdge* m_second;
    EdgeLink m_link;
private:
    Edge(HalfEdge* first, HalfEdge* second) :
    m_first(first),
    m_second(second),
    m_link(this) {
        assert(m_first != NULL);
        assert(m_second != NULL);
        m_first->setEdge(this);
        m_second->setEdge(this);
    }
    
    Edge(HalfEdge* first) :
    m_first(first),
    m_second(NULL),
    m_link(this) {
        assert(m_first != NULL);
        m_first->setEdge(this);
    }
public:
    Vertex* firstVertex() const {
        assert(m_first != NULL);
        return m_first->origin();
    }
    
    Vertex* secondVertex() const {
        assert(m_first != NULL);
        if (m_second != NULL)
            return m_second->origin();
        return m_first->next()->origin();
    }
    
    HalfEdge* firstEdge() const {
        assert(m_first != NULL);
        return m_first;
    }
    
    HalfEdge* secondEdge() const {
        assert(m_second != NULL);
        return m_second;
    }
    
    HalfEdge* twin(const HalfEdge* halfEdge) const {
        assert(halfEdge != NULL);
        assert(halfEdge == m_first || halfEdge == m_second);
        if (halfEdge == m_first)
            return m_second;
        return m_first;
    }
    
    Face* firstFace() const {
        assert(m_first != NULL);
        return m_first->face();
    }
    
    Face* secondFace() const {
        assert(m_second != NULL);
        return m_second->face();
    }
    
    Vertex* commonVertex(const Edge* other) const {
        assert(other != NULL);
        if (other->hasVertex(firstVertex()))
            return firstVertex();
        if (other->hasVertex(secondVertex()))
            return secondVertex();
        return NULL;
    }
    
    bool hasVertex(const Vertex* vertex) const {
        return firstVertex() == vertex || secondVertex() == vertex;
    }
    
    bool fullySpecified() const {
        assert(m_first != NULL);
        return m_second != NULL;
    }
    
    bool contains(const V& point, const T maxDistance = Math::Constants<T>::almostZero()) const {
        return point.distanceToSegment(firstVertex()->position(), secondVertex()->position() < maxDistance);
    }
private:
    void flip() {
        using std::swap;
        swap(m_first, m_second);
    }
    
    void unsetSecondEdge() {
        m_first->setAsLeaving();
        m_second->setEdge(NULL);
        m_second = NULL;
    }
    
    void setSecondEdge(HalfEdge* second) {
        assert(second != NULL);
        assert(m_second == NULL);
        assert(second->edge() == NULL);
        m_second = second;
        m_second->setEdge(this);
    }
};

#endif
