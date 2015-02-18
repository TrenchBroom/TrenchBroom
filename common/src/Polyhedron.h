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

#ifndef __TrenchBroom__Polyhedron__
#define __TrenchBroom__Polyhedron__

#include "VecMath.h"
#include "DoublyLinkedList.h"

#include <cassert>
#include <deque>
#include <queue>
#include <vector>

// implements a doubly-connected edge list, see de Berg et. al - Computational Geometry (3rd. Ed.), PP. 29
// see also http://www.holmes3d.net/graphics/dcel/
// This data structure extends the one described above by keeping circular lists of all vertices, all edges, and all faces.
template <typename T>
class Polyhedron {
public:
    class Vertex;
    class Edge;
    class HalfEdge;
    class Face;

    typedef std::deque<Edge*> Seam;
private:
    typedef Vec<T,3> V;
    typedef typename Vec<T,3>::List PosList;
    
    typedef typename DoublyLinkedList<Vertex>::Link VertexLink;
    typedef typename DoublyLinkedList<Edge>::Link EdgeLink;
    typedef typename DoublyLinkedList<HalfEdge>::Link HalfEdgeLink;
    typedef typename DoublyLinkedList<Face>::Link FaceLink;
    
    typedef std::vector<Vertex*> VertexVec;
    typedef std::vector<Edge*> EdgeVec;
    typedef std::vector<Face*> FaceVec;
public:
    class VertexList : public DoublyLinkedList<Vertex> {
    private:
        VertexLink& doGetLink(Vertex* vertex) const { return vertex->m_link; }
        const VertexLink& doGetLink(const Vertex* vertex) const { return vertex->m_link; }
    };
    
    class EdgeList : public DoublyLinkedList<Edge> {
    private:
        EdgeLink& doGetLink(Edge* edge) const { return edge->m_link; }
        const EdgeLink& doGetLink(const Edge* edge) const { return edge->m_link; }
    };
    
    class HalfEdgeList : public DoublyLinkedList<HalfEdge> {
    private:
        HalfEdgeLink& doGetLink(HalfEdge* edge) const { return edge->m_link; }
        const HalfEdgeLink& doGetLink(const HalfEdge* edge) const { return edge->m_link; }
    };
    
    class FaceList : public DoublyLinkedList<Face> {
    private:
        FaceLink& doGetLink(Face* face) const { return face->m_link; }
        const FaceLink& doGetLink(const Face* face) const { return face->m_link; }
    };
    
    class SplittingCriterion {
    private:
        typedef enum {
            MatchResult_First,
            MatchResult_Second,
            MatchResult_Both,
            MatchResult_Neither
        } MatchResult;
    public:
        virtual ~SplittingCriterion() {}
    public:
        Edge* findFirstSplittingEdge(EdgeList& edges) const {
            typename EdgeList::Iterator it = edges.iterator();
            while (it.hasNext()) {
                Edge* edge = it.next();
                const MatchResult result = matches(edge);
                switch (result) {
                    case MatchResult_Second:
                        edge->flip();
                    case MatchResult_First:
                        return edge;
                    case MatchResult_Both:
                    case MatchResult_Neither:
                        break;
                    DEFAULT_SWITCH()
                }
            }
            return NULL;
        }
        
        // finds the next seam edge in counter clockwise orientation
        Edge* findNextSplittingEdge(Edge* last) const {
            assert(last != NULL);

            HalfEdge* halfEdge = last->firstEdge()->previous();
            Edge* next = halfEdge->edge();
            if (!next->fullySpecified())
                return NULL;
            
            MatchResult result = matches(next);
            while (result != MatchResult_First && result != MatchResult_Second && next != last) {
                halfEdge = halfEdge->twin()->previous();
                next = halfEdge->edge();
                if (!next->fullySpecified())
                    return NULL;
                
                result = matches(next);
            }
            
            if (result != MatchResult_First && result != MatchResult_Second)
                return NULL;
            
            if (result == MatchResult_Second)
                next->flip();
            return next;
        }
    private:
        MatchResult matches(const Edge* edge) const {
            const bool firstResult = matches(edge->firstFace());
            const bool secondResult = matches(edge->secondFace());
            if (firstResult) {
                if (secondResult)
                    return MatchResult_Both;
                return MatchResult_First;
            }
            if (secondResult)
                return MatchResult_Second;
            return MatchResult_Neither;
        }
    public:
        bool matches(const Face* face) const {
            return doMatches(face);
        }
    private:
        virtual bool doMatches(const Face* face) const = 0;
    };
    
    class SplitByVisibilityCriterion : public SplittingCriterion {
    private:
        V m_point;
    public:
        SplitByVisibilityCriterion(const V& point) :
        m_point(point) {}
    private:
        bool doMatches(const Face* face) const {
            return !face->visibleFrom(m_point);
        }
    };
    
    class SplitByNormalCriterion : public SplittingCriterion {
    private:
        V m_normal;
    public:
        SplitByNormalCriterion(const V& normal) :
        m_normal(normal) {}
    private:
        bool doMatches(const Face* face) const {
            return !face->normal().equals(m_normal);
        }
    };
public:
    class Vertex {
    private:
        friend class HalfEdge;
        friend class VertexList;
        friend class Polyhedron<T>;
    private:
        V m_position;
        VertexLink m_link;
        HalfEdge* m_leaving;
    public:
        Vertex(const V& position) :
        m_position(position),
        m_link(this),
        m_leaving(NULL) {}
        
        const V& position() const {
            return m_position;
        }
        
        HalfEdge* leaving() const {
            return m_leaving;
        }
    private:
        void setPosition(const V& position) {
            m_position = position;
        }
        
        void setLeaving(HalfEdge* edge) {
            assert(edge != NULL);
            assert(edge->origin() == this);
            m_leaving = edge;
        }
    };
    
    class Edge {
    private:
        HalfEdge* m_first;
        HalfEdge* m_second;
        EdgeLink m_link;
        
        friend class EdgeList;
        friend class Polyhedron<T>;
    public:
        Edge(HalfEdge* first, HalfEdge* second) :
        m_first(first),
        m_second(second),
        m_link(this) {
            assert(m_first != NULL);
            assert(m_second != NULL);
            m_first->setEdge(this);
            m_second->setEdge(this);
        }
    private:
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
    
    class HalfEdge {
    private:
        Vertex* m_origin;
        Edge* m_edge;
        Face* m_face;
        HalfEdgeLink m_link;

        friend class Edge;
        friend class HalfEdgeList;
        friend class Face;
        friend class Polyhedron<T>;
    public:
        HalfEdge(Vertex* origin) :
        m_origin(origin),
        m_edge(NULL),
        m_face(NULL),
        m_link(this) {
            assert(m_origin != NULL);
            setAsLeaving();
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
            return destination() - origin();
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
        
        void unlink() {
            m_link.unlink();
        }
    };
    
    class Face {
    private:
        HalfEdgeList m_boundary;
        FaceLink m_link;
        
        friend class FaceList;
        friend class Polyhedron<T>;
    public:
        Face(const HalfEdgeList& boundary) :
        m_boundary(boundary),
        m_link(this) {
            assert(m_boundary.size() >= 3);
            
            typename HalfEdgeList::Iterator it = m_boundary.iterator();
            while (it.hasNext()) {
                HalfEdge* edge = it.next();
                edge->setFace(this);
            }
        }
        
        ~Face() {
            m_boundary.deleteAll();
        }
        
        size_t vertexCount() const {
            return m_boundary.size();
        }
        
        const HalfEdgeList& boundary() const {
            return m_boundary;
        }
        
        V origin() const {
            typename HalfEdgeList::ConstIterator it = m_boundary.iterator();
            const HalfEdge* edge = it.next();
            return edge->origin()->position();
        }
        
        V normal() const {
            typename HalfEdgeList::ConstIterator it = m_boundary.iterator();
            const HalfEdge* edge = it.next();
            const V p1 = edge->origin()->position();
            
            edge = it.next();
            const V p2 = edge->origin()->position();
            
            edge = it.next();
            const V p3 = edge->origin()->position();
            
            return crossed(p2 - p1, p3 - p1).normalized();
        }
        
        bool visibleFrom(const V& point) const {
            return pointStatus(point) == Math::PointStatus::PSAbove;
        }

        bool coplanar(const Face* other) const {
            assert(other != NULL);
            return normal().equals(other->normal(), Math::Constants<T>::colinearEpsilon());
        }
        
        bool isDegenerateTriangle(const T epsilon = Math::Constants<T>::almostZero()) const {
            if (vertexCount() != 3)
                return false;
            
            typename HalfEdgeList::ConstIterator it = m_boundary.iterator();
            const HalfEdge* edge = it.next();
            const T l1 = edge->squaredLength();
            
            edge = it.next();
            const T l2 = edge->squaredLength();
            
            edge = it.next();
            const T l3 = edge->squaredLength();
            
            const T epsilon2 = epsilon * epsilon;
            if (l1 > l2) {
                if (l1 > l3)
                    return Math::eq(l1, l2 + l3, epsilon2);
            } else {
                if (l2 > l3)
                    return Math::eq(l2, l1 + l3, epsilon2);
            }
            return Math::eq(l3, l1 + l2, epsilon2);
        }
    private:
        Math::PointStatus::Type pointStatus(const V& point, const T epsilon = Math::Constants<T>::pointStatusEpsilon()) const {
            const V norm = normal();
            const T distance = (point - origin()).dot(norm);
            if (distance > epsilon)
                return Math::PointStatus::PSAbove;
            if (distance < -epsilon)
                return Math::PointStatus::PSBelow;
            return Math::PointStatus::PSInside;
        }
        
        void flip() {
            m_boundary.reverse();
        }
        
        void replaceBoundary(HalfEdge* edge, HalfEdge* with) {
            replaceBoundary(edge, edge, with);
        }
        
        void replaceBoundary(HalfEdge* from, HalfEdge* to, HalfEdge* with) {
            assert(from != NULL);
            assert(to != NULL);
            assert(with != NULL);
            assert(from->face() == this);
            assert(to->face() == this);
            assert(with->face() == NULL);
            
            const size_t removeCount = countAndSetFace(from, to->next(), NULL);
            const size_t insertCount = countAndSetFace(with, with, this);
            m_boundary.replace(from, to, removeCount, with, insertCount);
        }

        size_t countAndSetFace(HalfEdge* from, HalfEdge* until, Face* face) {
            size_t count = 0;
            HalfEdge* cur = from;
            do {
                cur->setFace(face);
                cur = cur->next();
                ++count;
            } while (cur != until);
            return count;
        }
    };
    
    VertexList m_vertices;
    EdgeList m_edges;
    FaceList m_faces;
public:
    Polyhedron() {}
    
    Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4) {
        initialize(p1, p2, p3, p4);
    }

    Polyhedron(const BBox<T,3>& bounds) {
        const V p1(bounds.min.x(), bounds.min.y(), bounds.min.z());
        const V p2(bounds.min.x(), bounds.min.y(), bounds.max.z());
        const V p3(bounds.min.x(), bounds.max.y(), bounds.min.z());
        const V p4(bounds.min.x(), bounds.max.y(), bounds.max.z());
        const V p5(bounds.max.x(), bounds.min.y(), bounds.min.z());
        const V p6(bounds.max.x(), bounds.min.y(), bounds.max.z());
        const V p7(bounds.max.x(), bounds.max.y(), bounds.min.z());
        const V p8(bounds.max.x(), bounds.max.y(), bounds.max.z());
        
        initialize(p1, p2, p3, p5);
        addPoint(p4);
        addPoint(p6);
        addPoint(p7);
        addPoint(p8);
    }
    
    Polyhedron(typename V::List points) {
        if (chooseInitialPoints(points)) {
            initialize(points[0], points[1], points[2], points[3]);
            addPoints(points.begin() + 4, points.end());
        }
    }
private:
    Polyhedron(const VertexList& vertices, const EdgeList& edges, const FaceList& faces) :
    m_vertices(vertices),
    m_edges(edges),
    m_faces(faces) {}
public:
    ~Polyhedron() {
        clear();
    }
    
    size_t vertexCount() const {
        return m_vertices.size();
    }
    
    const VertexList& vertices() const {
        return m_vertices;
    }
    
    typename V::List vertexPositions() const {
        typename V::List positions;
        positions.reserve(vertexCount());
        return vertexPositiosn(positions);
    }
    
    typename V::List& vertexPositions(typename V::List& positions) const {
        typename VertexList::ConstIterator it = m_vertices.iterator();
        while (it.hasNext()) {
            const Vertex* v = it.next();
            positions.push_back(v->position());
        }
        return positions;
    }
    
    size_t edgeCount() const {
        return m_edges.size();
    }
    
    const EdgeList& edges() const {
        return m_edges;
    }
    
    size_t faceCount() const {
        return m_faces.size();
    }
    
    const FaceList& faces() const {
        return m_faces;
    }
    
    bool empty() const {
        return vertexCount() == 0;
    }
    
    bool point() const {
        return vertexCount() == 1;
    }
    
    bool edge() const {
        return vertexCount() == 2;
    }
    
    bool polygon() const {
        return faceCount() == 1;
    }
    
    bool polyhedron() const {
        return faceCount() > 3;
    }
    
    bool closed() const {
        return vertexCount() + faceCount() == edgeCount() + 2;
    }
    
    Vertex* findVertexByPosition(const V& position, const T epsilon = Math::Constants<T>::almostZero()) const {
        typename VertexList::ConstIterator it = m_vertices.iterator();
        while (it.hasNext()) {
            Vertex* vertex = it.next();
            if (position.equals(vertex->position(), epsilon))
                return vertex;
        }
        return NULL;
    }
private:
    struct MoveVertexResult;
public: // moving vertices
    typename V::List moveVertices(const typename V::List& positions, const V& delta) {
        if (delta.null())
            return positions;
        
        typename V::List newPositions;
        for (size_t i = 0; i < positions.size(); ++i) {
            Vertex* vertex = findVertexByPosition(positions[i]);
            assert(vertex != NULL);
            
            const V destination = vertex->position() + delta;
            const MoveVertexResult result = moveVertex(vertex, destination);
            if (result.moved())
                newPositions.push_back(result.vertex->position());
        }
        
        return newPositions;
    }
private:
    struct MoveVertexResult {
        typedef enum {
            Type_VertexMoved,
            Type_VertexDeleted,
            Type_VertexUnchanged
        } Type;
        
        const Type type;
        Vertex* vertex;
        
        MoveVertexResult(const Type i_type, Vertex* i_vertex = NULL) :
        type(i_type),
        vertex(i_vertex) {
            assert(type != Type_VertexDeleted || vertex == NULL);
        }
        
        bool moved() const     { return type == Type_VertexMoved; }
        bool deleted() const   { return type == Type_VertexDeleted; }
        bool unchanged() const { return type == Type_VertexUnchanged; }
    };
    
    MoveVertexResult moveVertex(Vertex* vertex, const V& destination) {
        assert(vertex != NULL);
        assert(vertex->position() != destination);
        assert(checkInvariant());

        T lastFrac = 0.0;
        const V origin = vertex->position();
        while (!vertex->position().equals(destination, 0.0)) {
            splitIncidentFaces(vertex, destination);
            
            const T curFrac = computeMinFrac(vertex, origin, destination, lastFrac);
            if (curFrac < 0.0)
                return MoveVertexResult(MoveVertexResult::Type_VertexUnchanged, vertex);
            
            assert(curFrac > lastFrac);
            lastFrac = curFrac;
            
            // We can now safely move the vertex to its new position without the brush becoming convex.
            vertex->setPosition(origin + lastFrac * (destination - origin));
            vertex = cleanupAfterVertexMove(vertex);
            if (vertex == NULL)
                return MoveVertexResult(MoveVertexResult::Type_VertexDeleted);
        }
        
        return MoveVertexResult(MoveVertexResult::Type_VertexMoved, vertex);
    }
private:
    void splitIncidentFaces(Vertex* vertex, const V& destination) {
        HalfEdge* firstEdge = vertex->leaving();
        HalfEdge* curEdge = firstEdge;
        
        do {
            HalfEdge* nextEdge = curEdge->nextIncident();
            Face* face = curEdge->face();
            if (face->vertexCount() > 3) {
                const Math::PointStatus::Type status = face->pointStatus(destination);
                if (status == Math::PointStatus::PSBelow)
                    chopFace(face, curEdge);
                else
                    splitFace(face, curEdge);
            }
            curEdge = nextEdge;
        } while (curEdge != firstEdge);
    }
    
    void chopFace(Face* face, HalfEdge* halfEdge) {
        assert(face->vertexCount() > 3);
        
        HalfEdge* next = halfEdge;
        HalfEdge* previous = next->previous();
        
        HalfEdge* newEdge1 = new HalfEdge(previous->origin());
        HalfEdge* newEdge2 = new HalfEdge(next->destination());
        
        face->replaceBoundary(previous, next, newEdge1);
        
        assert(next->next() == previous);
        assert(previous->previous() == next);

        previous->unlink();
        next->unlink();
        
        m_faces.append(createTriangle(previous, next, newEdge2), 1);
        m_edges.append(new Edge(newEdge1, newEdge2), 1);
    }
    
    void splitFace(Face* face, HalfEdge* halfEdge) {
        HalfEdge* next = halfEdge;
        while (face->vertexCount() > 3) {
            HalfEdge* previous = next->previous();
            chopFace(face, previous);
        }
    }
    
    T computeMinFrac(Vertex* vertex, const V& origin, const V& destination, const T lastFrac) const {
        HalfEdge* firstEdge = vertex->leaving();
        HalfEdge* curEdge = firstEdge;

        T minFrac = 1.0;
        do {
            minFrac = std::min(minFrac, computeMinFracForIncidentNeighbour(curEdge, origin, destination, lastFrac));
            minFrac = std::min(minFrac, computeMinFracForOppositeNeighbour(curEdge, origin, destination, lastFrac));
            curEdge = curEdge->nextIncident();
        } while (curEdge != firstEdge);
        return minFrac;
    }
    
    T computeMinFracForIncidentNeighbour(HalfEdge* edge, const V& origin, const V& destination, const T lastFrac) const {
        /*
         We consider the plane made up by the points p1, p2 and p3 of side and next. If the movement
         of the vertex were to go through this plane, the brush would become convex, which we must prevent.
         
         v----p1
         |\ s |
         | \  |
         |  \ |
         | n \|
         p3----p2
         
         */

        HalfEdge* nextEdge = edge->nextIncident();
        Face* face = edge->face();
        Face* neighbour = nextEdge->face();
        
        assert(face->vertexCount() == 3);
        assert(neighbour->vertexCount() == 3);
        
        Vertex* v1 = edge->destination();
        Vertex* v2 = edge->origin();
        Vertex* v3 = nextEdge->next()->destination();

        const V& p1 = v1->position();
        const V& p2 = v2->position();
        const V& p3 = v3->position();
        
        Plane<T,3> plane;
        if (!setPlanePoints(plane, p1, p2, p3)) {
            // The points are colinear and we cannot determine the move distance - this is an error!
            return -1.0;
        }
        
        return computeMinFrac(origin, destination, plane, lastFrac);
    }
    
    T computeMinFracForOppositeNeighbour(HalfEdge* edge, const V& origin, const V& destination, const T lastFrac) const {
        /*
         We consider the boundary plane of the one neighbour to the side which is not incident to the
         moved vertex. This neighbour is not necessarily a triangle, but that does not matter.
         
                 p3------
                /    n  |
               /     e  |
         v----p2     i  |
         |\ s |      g  |
         | \  |      h  |
         |  \ |      b  |
         |   \|      o  |
         -----p1     u  |
               \     r  |
                ---------
         */
        
        HalfEdge* myBorder = edge->next();
        HalfEdge* theirBorder = myBorder->twin();
        
        Vertex* v1 = edge->destination();
        Vertex* v2 = theirBorder->origin();
        Vertex* v3 = theirBorder->previous()->origin();
        
        const V& p1 = v1->position();
        const V& p2 = v2->position();
        const V& p3 = v3->position();

        Plane<T,3> plane;
        if (!setPlanePoints(plane, p1, p2, p3)) {
            // The points are colinear and we cannot determine the move distance - this is an error!
            return -1.0;
        }
        
        return computeMinFrac(origin, destination, plane, lastFrac);
    }
    
    T computeMinFrac(const V& origin, const V& destination, const Plane<T,3>& plane, const T lastFrac) const {
        const T origDot = origin.dot(plane.normal) - plane.distance;
        const T destDot = destination.dot(plane.normal) - plane.distance;
        
        // Let's allow for some error.
        if (std::abs(origDot) >= 0.001 || std::abs(destDot) >= 0.001) {
            // Does the vertex lie above the plane?
            if (origDot > 0.0) {
                // Does it actually travel into the plane?
                if (destDot <= 0.0) {
                    // The distance must grow because we use the original vertex origin.
                    const T frac = std::abs(origDot) / (std::abs(origDot) + std::abs(destDot));
                    if (frac > lastFrac)
                        return frac;
                }
            }
        }
        return 1.0;
    }
    
    Vertex* cleanupAfterVertexMove(Vertex* vertex) {
        // If any of the incident sides have become degenerate triangles, that is, the length of the longest side
        // is the sum of the shorter two, then delete those triangles.
        
        vertex = mergeIncidentFaces(vertex);
        if (vertex != NULL) {
            // Merge all colinear edges. This might also delete the vertex, so be careful.
        }
        
        return vertex;
    }

    Vertex* mergeIncidentFaces(Vertex* vertex) {
        HalfEdge* firstEdge = vertex->leaving();
        HalfEdge* curEdge = firstEdge;
    
        bool allInnerMerged = true;
        do {
            Face* face = curEdge->face();
            
            HalfEdge* outerBorder = curEdge->next();
            Face* outerNeighbour = outerBorder->twin()->face();
            
            HalfEdge* innerBorder = curEdge->previous();
            Face* innerNeighbour = innerBorder->twin()->face();
            
            if (face->coplanar(outerNeighbour)) {
                allInnerMerged = false;
                mergeNeighbours(outerBorder);
                curEdge = curEdge->previous()->twin();
            } else if (face->coplanar(innerNeighbour)) {
                mergeNeighbours(innerBorder);
            } else {
                allInnerMerged = false;
                curEdge = curEdge->previous()->twin();
            }
        } while (curEdge != firstEdge);
        
        if (allInnerMerged) {
            m_vertices.remove(vertex);
            return NULL;
        }
        
        return vertex;
    }
    
    
                                             
    void mergeNeighbours(HalfEdge* border) {
        HalfEdge* twin = border->twin();
        Face* face = border->face();
        Face* neighbour = twin->face();
        
        using std::swap;
        HalfEdgeList boundary;
        std::swap(boundary, face->m_boundary);
        
        boundary.remove(border);
        neighbour->replaceBoundary(twin, border);
        
        Edge* edge = border->edge();
        m_edges.remove(edge);
        
        m_faces.remove(face);
        
        delete border;
        delete twin;
        delete edge;
        delete face;
    }
    
    bool coplanarWithOuterNeighbour(HalfEdge* edge) const {
        Face* face = edge->face();
        Face* outerNeighbour = edge->next()->twin()->face();
        return face->normal().equals(outerNeighbour->normal());
    }
public: // adding points and convex hull computations
    template <typename I>
    void addPoints(I cur, I end) {
        while (cur != end) {
            const V& point = *cur;
            addPoint(point);
            ++cur;
        }
    }
    
    void addPoint(const V& point) {
        assert(checkInvariant());
        switch (vertexCount()) {
            case 0:
                addFirstPoint(point);
                break;
            case 1:
                addSecondPoint(point);
                break;
            case 2:
                addThirdPoint(point);
                break;
            default:
                addFurtherPoint(point);
                break;
        }
    }
private:
    void addFirstPoint(const V& point) {
        assert(vertexCount() == 0);
        assert(edgeCount() == 0);
        assert(faceCount() == 0);

        m_vertices.append(new Vertex(point), 1);
    }
    
    void addSecondPoint(const V& point) {
        assert(vertexCount() == 1);
        assert(edgeCount() == 0);
        assert(faceCount() == 0);
        
        Vertex* onlyVertex = m_vertices.iterator().next();
        if (point != onlyVertex->position()) {
            Vertex* newVertex = new Vertex(point);
            m_vertices.append(newVertex, 1);
            
            HalfEdge* halfEdge1 = new HalfEdge(onlyVertex);
            HalfEdge* halfEdge2 = new HalfEdge(newVertex);
            Edge* edge = new Edge(halfEdge1, halfEdge2);
            m_edges.append(edge, 1);
        }
    }
    
    void addThirdPoint(const V& point) {
        assert(vertexCount() == 2);
        assert(edgeCount() == 1);
        assert(faceCount() == 0);
        
        typename VertexList::Iterator it = m_vertices.iterator();
        const Vertex* v1 = it.next();
        const Vertex* v2 = it.next();
        
        if (linearlyDependent(v1->position(), v2->position(), point))
            addPointToEdge(point);
        else
            addPointToPolygon(point);
    }
    
    void addFurtherPoint(const V& point) {
        if (faceCount() == 1)
            addFurtherPointToPolygon(point);
        else
            addFurtherPointToPolyhedron(point);
    }
    
    void addFurtherPointToPolygon(const V& point) {
        Face* face = m_faces.iterator().next();
        const Math::PointStatus::Type status = face->pointStatus(point);
        switch (status) {
            case Math::PointStatus::PSInside:
                addPointToPolygon(point);
                break;
            case Math::PointStatus::PSAbove:
                face->flip();
            case Math::PointStatus::PSBelow:
                makePolyhedron(point);
                break;
        }
    }
    
    void addPointToEdge(const V& point) {
        typename VertexList::Iterator it = m_vertices.iterator();
        Vertex* v1 = it.next();
        Vertex* v2 = it.next();
        assert(linearlyDependent(v1->position(), v2->position(), point));
        
        if (!point.containedWithinSegment(v1->position(), v2->position()))
            v2->setPosition(point);
    }
    
    void addPointToPolygon(const V& point) {
        typename V::List points;
        points.reserve(vertexCount() + 1);
        vertexPositions(points);
        points.push_back(point);
        
        points = convexHull2D<T>(points);
        clear();
        makePolygon(points);
    }
    
    void makePolygon(const typename V::List& points) {
        assert(vertexCount() == 0);
        assert(edgeCount() == 0);
        assert(faceCount() == 0);
        assert(points.size() > 2);

        HalfEdgeList boundary;
        for (size_t i = 0; i < points.size(); ++i) {
            const V& p = points[i];
            Vertex* v = new Vertex(p);
            HalfEdge* h = new HalfEdge(v);
            Edge* e = new Edge(h);

            m_vertices.append(v, 1);
            boundary.append(h, 1);
            m_edges.append(e, 1);
        }
        
        Face* f = new Face(boundary);
        m_faces.append(f, 1);
    }
    
    void makePolyhedron(const V& point) {
        Seam seam;
        
        Face* face = m_faces.iterator().next();
        const HalfEdgeList& boundary = face->boundary();
        typename HalfEdgeList::ConstIterator it = boundary.iterator();
        while (it.hasNext()) {
            const HalfEdge* h = it.next();
            Edge* e = h->edge();
            seam.push_front(e); // ensure that the seam is in CCW order
        }
        
        addPointToPolyhedron(point, seam);
    }
    
    void addFurtherPointToPolyhedron(const V& point) {
        const Seam seam = split(SplitByVisibilityCriterion(point));
        if (!seam.empty()) {
            weaveCap(seam, point);
            mergeCoplanarFaces(seam);
            assert(checkInvariant());
        }
    }
    
    void addPointToPolyhedron(const V& point, const Seam& seam) {
        assert(!seam.empty());
        weaveCap(seam, point);
        mergeCoplanarFaces(seam);
        assert(checkInvariant());
    }
    
    // This algorithm splits this polyhedron along the edges where one of the adjacant faces matches
    // the given criterion and the other does not. The algorithm runs linear in the sum of the numbers
    // of all vertices, edges, and faces. The returned seam is oriented in counter-clockwise order.
    Seam split(const SplittingCriterion& criterion) {
        VertexList vertices;
        EdgeList edges;
        FaceList faces;
        Seam seam;
        
        Edge* splittingEdge = criterion.findFirstSplittingEdge(m_edges);
        if (splittingEdge == NULL)
            return Seam(0);
        
        // First, go along the splitting seam and split the edges into two edges with one half edge each.
        // Both the resulting edges have their only half edge as their first edge.
        do {
            assert(splittingEdge != NULL);
            Edge* nextSplittingEdge = criterion.findNextSplittingEdge(splittingEdge);
            assert(nextSplittingEdge != splittingEdge);
            assert(nextSplittingEdge == NULL || splittingEdge->firstVertex() == nextSplittingEdge->secondVertex());

            splittingEdge->unsetSecondEdge();
            seam.push_back(splittingEdge);
            
            splittingEdge = nextSplittingEdge;
        } while (splittingEdge != NULL);
        
        typename VertexList::Iterator vertexIt;
        typename EdgeList::Iterator edgeIt;
        typename FaceList::Iterator faceIt;
        
        // Now handle the remaining faces, edge, and vertices by sorting them into the correct polyhedra.
        vertexIt = m_vertices.iterator();
        while (vertexIt.hasNext()) {
            Vertex* vertex = vertexIt.next();
            HalfEdge* edge = vertex->leaving();
            assert(edge != NULL);
            
            // As we have already handled the shared vertices, it holds that for each remaining vertex,
            // either all adjacent faces match or not. There are no mixed vertices anymore at this point.
            
            Face* face = edge->face();
            assert(face != NULL);
            
            if (!criterion.matches(face)) {
                vertexIt.remove();
                vertices.append(vertex, 1);
            }
        }
        
        edgeIt = m_edges.iterator();
        while (edgeIt.hasNext()) {
            Edge* edge = edgeIt.next();
            Face* face = edge->firstFace();
            assert(face != NULL);
            
            // There are no mixed edges at this point anymore either, and all remaining edges have at least
            // one edge, and that is their first edge.
            
            if (!criterion.matches(face)) {
                assert(edge->secondFace() == NULL || !criterion.matches(edge->secondFace()));
                edgeIt.remove();
                edges.append(edge, 1);
            }
        }
        
        faceIt = m_faces.iterator();
        while (faceIt.hasNext()) {
            Face* face = faceIt.next();
            
            if (!criterion.matches(face)) {
                faceIt.remove();
                faces.append(face, 1);
            }
        }
        
        faces.deleteAll();
        edges.deleteAll();
        vertices.deleteAll();
        
        assert(isConvex());
        
        return seam;
    }
    
    void weaveCap(const Seam& seam, const V& point) {
        assert(seam.size() >= 3);
        
        Vertex* top = new Vertex(point);
        
        HalfEdge* first = NULL;
        HalfEdge* last = NULL;
        for (size_t i = 0; i < seam.size(); ++i) {
            Edge* edge = seam[i];
            assert(!edge->fullySpecified());
            
            Vertex* v1 = edge->secondVertex();
            Vertex* v2 = edge->firstVertex();
            
            HalfEdge* h1 = new HalfEdge(top);
            HalfEdge* h2 = new HalfEdge(v1);
            HalfEdge* h3 = new HalfEdge(v2);
            
            m_faces.append(createTriangle(h1, h2, h3), 1);
            
            if (last != NULL)
                m_edges.append(new Edge(h1, last), 1);
            edge->setSecondEdge(h2);
            
            if (first == NULL)
                first = h1;
            last = h3;
        }
        
        m_edges.append(new Edge(first, last), 1);
        m_vertices.append(top, 1);

        assert(isConvex());
    }
    
    void mergeCoplanarFaces(const Seam& seam) {
        std::queue<Edge*> queue(seam);
        while (!queue.empty()) {
            Edge* first = queue.front(); queue.pop();
            assert(first->fullySpecified());

            const V firstNorm = first->firstFace()->normal();
            if (firstNorm.equals(first->secondFace()->normal())) {
                // fast forward through all seam edges which have both of their incident faces coplanar to the first face
                if (!queue.empty()) {
                    Edge* cur = queue.front();
                    while (cur != NULL && cur->firstFace()->normal().equals(firstNorm)) {
                        assert(cur->fullySpecified());
                        assert(cur->firstFace()->normal().equals(firstNorm) == cur->secondFace()->normal().equals(firstNorm));
                        queue.pop();
                        cur = !queue.empty() ? queue.front() : NULL;
                    }
                }

                // now remove all coplanar faces and replace them with a new cap
                const Seam mergeSeam = split(SplitByNormalCriterion(firstNorm));
                weaveCap(mergeSeam);
            }
        }

        assert(isConvex());
    }

    void weaveCap(const Seam& seam) {
        assert(seam.size() >= 3);
        
        HalfEdgeList halfEdges;
        for (size_t i = 0; i < seam.size(); ++i) {
            Edge* edge = seam[i];
            assert(!edge->fullySpecified());
            
            HalfEdge* halfEdge = new HalfEdge(edge->secondVertex());
            edge->setSecondEdge(halfEdge);
            halfEdges.append(halfEdge, 1);
        }

        Face* face = new Face(halfEdges);
        m_faces.append(face, 1);

        assert(isConvex());
    }
private:
    void initialize(const V& p1, const V& p2, const V& p3, const V& p4) {
        using std::swap;
        assert(!commonPlane(p1, p2, p3, p4));
        
        Vertex* v1 = new Vertex(p1);
        Vertex* v2 = new Vertex(p2);
        Vertex* v3 = new Vertex(p3);
        Vertex* v4 = new Vertex(p4);
        
        const V d1 = v4->position() - v2->position();
        const V d2 = v4->position() - v3->position();
        const V d3 = v1->position() - v4->position();
        const V n1 = crossed(d1, d2);
        
        if (d3.dot(n1) > 0.0)
            swap(v2, v3);
        
        m_vertices.append(v1, 1);
        m_vertices.append(v2, 1);
        m_vertices.append(v3, 1);
        m_vertices.append(v4, 1);
        
        HalfEdge* h1 = new HalfEdge(v2);
        HalfEdge* h2 = new HalfEdge(v3);
        HalfEdge* h3 = new HalfEdge(v4);
        HalfEdge* h4 = new HalfEdge(v1);
        HalfEdge* h5 = new HalfEdge(v3);
        HalfEdge* h6 = new HalfEdge(v2);
        HalfEdge* h7 = new HalfEdge(v1);
        HalfEdge* h8 = new HalfEdge(v2);
        HalfEdge* h9 = new HalfEdge(v4);
        HalfEdge* h10 = new HalfEdge(v1);
        HalfEdge* h11 = new HalfEdge(v4);
        HalfEdge* h12 = new HalfEdge(v3);
        
        Edge* e1 = new Edge(h1, h5);
        Edge* e2 = new Edge(h2, h11);
        Edge* e3 = new Edge(h3, h8);
        Edge* e4 = new Edge(h4, h12);
        Edge* e5 = new Edge(h6, h7);
        Edge* e6 = new Edge(h9, h10);
        
        m_edges.append(e1, 1);
        m_edges.append(e2, 1);
        m_edges.append(e3, 1);
        m_edges.append(e4, 1);
        m_edges.append(e5, 1);
        m_edges.append(e6, 1);
        
        Face* f1 = createTriangle(h1, h2, h3);
        Face* f2 = createTriangle(h4, h5, h6);
        Face* f3 = createTriangle(h7, h8, h9);
        Face* f4 = createTriangle(h10, h11, h12);

        m_faces.append(f1, 1);
        m_faces.append(f2, 1);
        m_faces.append(f3, 1);
        m_faces.append(f4, 1);
        
        assert(checkInvariant());
    }
    
    bool chooseInitialPoints(typename V::List& points) {
        using std::swap;
        
        // first, choose a third point that is not colinear to the first two
        size_t index = 2;
        while (index < points.size() && V::colinear(points[0], points[1], points[index]))
            ++index;
        if (index == points.size())
            return false;
        if (index != 2)
            swap(points[2], points[index]);
        
        // now choose a fourth point such that it doesn't lie on the plane defined by the first three
        index = 3;
        while (index < points.size() && commonPlane(points[0], points[1], points[2], points[index]))
            ++index;
        if (index == points.size())
            return false;
        if (index != 3)
            swap(points[3], points[index]);
        return true;
    }
    
    Face* createTriangle(HalfEdge* h1, HalfEdge* h2, HalfEdge* h3) {
        HalfEdgeList boundary;
        boundary.append(h1, 1);
        boundary.append(h2, 1);
        boundary.append(h3, 1);
        
        return new Face(boundary);
    }
    
    void clear() {
        m_faces.deleteAll();
        m_edges.deleteAll();
        m_vertices.deleteAll();
    }
    
    bool checkInvariant() const {
        if (!isConvex())
            return false;
        return true;
    }
    
    bool isConvex() const {
        typename FaceList::ConstIterator fIt = m_faces.iterator();
        while (fIt.hasNext()) {
            const Face* face = fIt.next();
            typename VertexList::ConstIterator vIt = m_vertices.iterator();
            while (vIt.hasNext()) {
                const Vertex* vertex = vIt.next();
                if (face->pointStatus(vertex->position()) == Math::PointStatus::PSAbove)
                    return false;
            }
        }
        return true;
    }
};

#endif /* defined(__TrenchBroom__Polyhedron__) */
