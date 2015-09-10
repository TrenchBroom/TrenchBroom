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

template <typename T, typename FP>
class Polyhedron<T,FP>::GetEdgeLink {
public:
    typename DoublyLinkedList<Edge, GetEdgeLink>::Link& operator()(Edge* edge) const {
        return edge->m_link;
    }
    
    const typename DoublyLinkedList<Edge, GetEdgeLink>::Link& operator()(const Edge* edge) const {
        return edge->m_link;
    }
};

template <typename T, typename FP>
class Polyhedron<T,FP>::Edge : public Allocator<Edge> {
private:
    friend class Polyhedron<T,FP>;
private:
    HalfEdge* m_first;
    HalfEdge* m_second;
    EdgeLink m_link;
private:
    Edge(HalfEdge* first, HalfEdge* second = NULL) :
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
    
    Vertex* otherVertex(Vertex* vertex) const {
        assert(vertex != NULL);
        assert(vertex == firstVertex() || vertex == secondVertex());
        if (vertex == firstVertex())
            return secondVertex();
        return firstVertex();
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
    
    V vector() const {
        return secondVertex()->position() - firstVertex()->position();
    }
    
    V center() const {
        assert(fullySpecified());
        return (m_first->origin()->position() + m_second->origin()->position()) / static_cast<T>(2.0);
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
    
    bool hasPosition(const V& position, const T epsilon = Math::Constants<T>::almostZero()) const {
        return ( firstVertex()->position().equals(position, epsilon) ||
                secondVertex()->position().equals(position, epsilon));
    }
    
    bool hasPositions(const V& position1, const V& position2, const T epsilon = Math::Constants<T>::almostZero()) const {
        return (( firstVertex()->position().equals(position1, epsilon) &&
                 secondVertex()->position().equals(position2, epsilon)) ||
                ( firstVertex()->position().equals(position2, epsilon) &&
                 secondVertex()->position().equals(position1, epsilon))
                );
    }
    
    bool fullySpecified() const {
        assert(m_first != NULL);
        return m_second != NULL;
    }
    
    bool contains(const V& point, const T maxDistance = Math::Constants<T>::almostZero()) const {
        return point.distanceToSegment(firstVertex()->position(), secondVertex()->position()).distance < maxDistance;
    }

    Edge* next() const {
        return m_link.next();
    }
    
    Edge* previous() const {
        return m_link.previous();
    }
private:
    Edge* split(const Plane<T,3>& plane) {
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
        
        // cheat a little bit?, just like QBSP
        position.correct();
        return insertVertex(position);
    }

    Edge* splitAtCenter() {
        return insertVertex(center());
    }

    Edge* insertVertex(const V& position) {
        Vertex* newVertex = new Vertex(position);
        HalfEdge* newFirstEdge = new HalfEdge(newVertex);
        HalfEdge* oldFirstEdge = firstEdge();
        HalfEdge* newSecondEdge = new HalfEdge(newVertex);
        HalfEdge* oldSecondEdge = secondEdge();
        
        newFirstEdge->setFace(firstFace());
        newSecondEdge->setFace(secondFace());
        
        firstFace()->insertIntoBoundaryAfter(oldFirstEdge, newFirstEdge);
        secondFace()->insertIntoBoundaryAfter(oldSecondEdge, newSecondEdge);
        
        unsetSecondEdge();
        setSecondEdge(newSecondEdge);
        
        Edge* newEdge = new Edge(newFirstEdge, oldSecondEdge);
        return newEdge;
    }
    
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
