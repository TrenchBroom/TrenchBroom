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

#include <cassert>
#include <vector>

// implements a doubly-connected edge list, see de Berg et. al - Computational Geometry (3rd. Ed.), PP. 29
// see also http://www.holmes3d.net/graphics/dcel/
template <typename T>
class Polyhedron {
private:
    typedef Vec<T,3> V;

    template <typename Item>
    struct Link {
        Item* previous;
        Item* next;

        Link(Item* i_item) :
        previous(i_item),
        next(i_item) {
            assert(i_item != NULL);
        }
        
        bool selfLoop() const {
            return previous == next;
        }
    };
    
    template <typename Item>
    class LinkProcessor {
    private:
        Item* m_item;
    public:
        LinkProcessor(Item* item) :
        m_item(item) {
            assert(m_item != NULL);
        }
        
        virtual ~LinkProcessor() {}
        
        Item* item() const {
            return m_item;
        }
        
        LinkProcessor<Item> next() const {
            Link<Item>& link = doGetLink(m_item);
            return LinkProcessor<Item>(link.next);
        }
        
        LinkProcessor<Item> previous() const {
            Link<Item>& link = doGetLink(m_item);
            return LinkProcessor<Item>(link.previous);
        }
        
        bool contains(const Item* item) const {
            Item* curItem = m_item;
            do {
                if (curItem == item)
                    return true;
                Link<Item>& curLink = doGetLink(curItem);
                curItem = curLink.next;
            } while (curItem != m_item);
            return false;
        }
        
        void insertAfter(Item* item) {
            insertAfter(item, item);
        }
        
        void insertAfter(Item* first, Item* last) {
            Link<Item>& myLink = doGetLink(m_item);
            Item* nextItem = myLink.next;
            Link<Item>& nextLink = doGetLink(nextItem);
            Link<Item>& firstLink = doGetLink(first);
            Link<Item>& lastLink = doGetLink(last);
            
            myLink.next = first;
            firstLink.previous = m_item;
            nextLink.previous = last;
            lastLink.next = nextItem;
        }
        
        void replace(Item* first, Item* last) {
            replace(m_item, first, last);
        }
        
        void replace(Item* until, Item* first, Item* last) {
            Link<Item>& myLink = doGetLink(m_item);
            Link<Item>& untilLink = doGetLink(until);
            Link<Item>& firstLink = doGetLink(first);
            Link<Item>& lastLink = doGetLink(last);
            
            Item* pred = myLink.previous;
            Item* succ = untilLink.next;
            Link<Item>& predLink = doGetLink(pred);
            Link<Item>& succLink = doGetLink(succ);
            
            predLink.next = first;
            firstLink.previous = pred;
            succLink.previous = last;
            lastLink.next = succ;
            
            myLink.previous = until;
            untilLink.next = m_item;
        }
        
        void remove() {
            Link<Item>& myLink = doGetLink(m_item);
            
            Item* pred = myLink.previous;
            Item* succ = myLink.next;
            Link<Item>& predLink = doGetLink(pred);
            Link<Item>& succLink = doGetLink(succ);
            
            predLink.next = succ;
            succLink.previous = pred;
            myLink.previous = myLink.next = m_item;
        }
        
        bool checkLink(const size_t maxCount) const {
            size_t count = 0;
            Item* curItem = m_item;
            do {
                const Link<Item>& curLink = doGetLink(curItem);
                Item* nextItem = curLink.next;
                if (nextItem == NULL)
                    return false;
                const Link<Item>& nextLink = doGetLink(nextItem);
                if (nextLink.previous != curItem)
                    return false;
                ++count;
            } while (curItem != m_item && count < maxCount);
            return count < maxCount;
        }
        
        size_t count(const size_t maxCount) const {
            size_t count = 0;
            Item* curItem = m_item;
            do {
                assert(curItem != NULL);
                const Link<Item>& curLink = doGetLink(curItem);
                curItem = curLink.next;
                ++count;
            } while (curItem != m_item && count < maxCount);
            return count;
        }
    private:
        virtual Link<Item>& doGetLink(Item* item) const = 0;
    };
public:
    class Edge;
    class Vertex {
    private:
        V m_position;
        Link<Vertex> m_link;
        Edge* m_leaving;
        
        friend class Edge;
        friend class Polyhedron<T>;
    public:
        Vertex(const V& position) :
        m_position(position),
        m_link(this),
        m_leaving(NULL) {}
        
        const V& position() const {
            return m_position;
        }

        Vertex* next() const {
            return m_link.next;
        }
    private:
        bool chooseNewLeavingEdge() {
            assert(m_leaving != NULL);
            
            Edge* pred = m_leaving->previousBoundaryEdge();
            assert(pred != NULL);
            assert(pred != m_leaving);
            
            m_leaving = pred->m_twin;
            return m_leaving != NULL;
        }
    };
    
    class Face;
    class Edge {
    private:
        Vertex* m_origin;
        Edge* m_twin;
        Link<Edge> m_edgeLink;
        Face* m_face;
        Link<Edge> m_boundaryLink;
        
        friend class Face;
        friend class Polyhedron<T>;
    public:
        Edge(Vertex* origin) :
        m_origin(origin),
        m_twin(NULL),
        m_edgeLink(this),
        m_face(NULL),
        m_boundaryLink(this) {
            assert(m_origin != NULL);
            
            if (m_origin->m_leaving == NULL)
                m_origin->m_leaving = this;
        }
        
        Vertex* origin() const {
            return m_origin;
        }
        
        Vertex* destination() const {
            assert(m_twin != NULL);
            return m_twin->m_origin;
        }
        
        Edge* next() const {
            return m_edgeLink.next;
        }
        
        Edge* nextBoundaryEdge() const {
            return m_boundaryLink.next;
        }
        
        Edge* previousBoundaryEdge() const {
            return m_boundaryLink.previous;
        }

        Face* face() const {
            return m_face;
        }
    private:
        bool isHorizonFor(const V& point) const {
            assert(m_face != NULL);
            assert(m_twin != NULL);
            assert(m_twin->m_face != NULL);
            return m_face->isVisibleFrom(point) != m_twin->m_face->isVisibleFrom(point);
        }
    };
    
    class Face {
    private:
        Edge* m_edges;
        Link<Face> m_link;
        friend class Polyhedron<T>;
    public:
        Face(Edge* edges) :
        m_edges(edges),
        m_link(this) {
            assert(m_edges != NULL);
            Edge* edge = m_edges;
            do {
                assert(edge->m_origin != NULL);
                assert(edge->m_face == NULL);
                
                edge->m_face = this;
                edge = edge->nextBoundaryEdge();
            } while (edge != m_edges);
        }

        V orthogonal() const {
            const Edge* edge = m_edges;
            const V p1 = edge->origin()->position();
            
            edge = edge->nextBoundaryEdge();
            const V p2 = edge->origin()->position();
            
            edge = edge->nextBoundaryEdge();
            const V p3 = edge->origin()->position();
            
            return crossed(p2 - p1, p3 - p1);
        }
        
        V normal() const {
            return orthogonal().normalized();
        }
        
        Math::PointStatus::Type pointStatus(const V& point, const T epsilon = Math::Constants<T>::pointStatusEpsilon()) const {
            const V norm = normal();
            const T offset = m_edges->origin()->position().dot(norm);
            const T distance = point.dot(norm) - offset;
            if (distance > epsilon)
                return Math::PointStatus::PSAbove;
            if (distance < -epsilon)
                return Math::PointStatus::PSBelow;
            return Math::PointStatus::PSInside;
        }
        
        Face* next() const {
            return m_link.next;
        }
        
        Edge* edges() const {
            return m_edges;
        }
    private:
        bool isVisibleFrom(const V& point) const {
            return pointStatus(point) == Math::PointStatus::PSAbove;
        }
    };
    
    class VertexLinkProcessor : public LinkProcessor<Vertex> {
    public:
        VertexLinkProcessor(Vertex* vertex) :
        LinkProcessor<Vertex>(vertex) {}
    private:
        Link<Vertex>& doGetLink(Vertex* vertex) const {
            return vertex->m_link;
        }
    };
    
    class EdgeLinkProcessor : public LinkProcessor<Edge> {
    public:
        EdgeLinkProcessor(Edge* edge) :
        LinkProcessor<Edge>(edge) {}
    private:
        Link<Edge>& doGetLink(Edge* edge) const {
            return edge->m_edgeLink;
        }
    };
    
    class BoundaryLinkProcessor : public LinkProcessor<Edge> {
    public:
        BoundaryLinkProcessor(Edge* edge) :
        LinkProcessor<Edge>(edge) {}
    private:
        Link<Edge>& doGetLink(Edge* edge) const {
            return edge->m_boundaryLink;
        }
    };
    
    class FaceLinkProcessor : public LinkProcessor<Face> {
    public:
        FaceLinkProcessor(Face* face) :
        LinkProcessor<Face>(face) {}
    private:
        Link<Face>& doGetLink(Face* face) const {
            return face->m_link;
        }
    };
private:
    typedef std::vector<Edge*> EdgeList;
    typedef std::vector<Face*> FaceList;
    
    Edge* m_edges;
public:
    Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4) :
    m_edges(NULL) {
        initialize(p1, p2, p3, p4);
    }
    
    Polyhedron(typename V::List points) :
    m_edges(NULL) {
        assert(points.size() >= 4);

        using std::swap;
        
        // first, choose a third point that is not colinear to the first two
        size_t index = 2;
        while (index < points.size() && V::colinear(points[0], points[1], points[index]))
            ++index;
        if (index < points.size()) {
            if (index != 2)
                swap(points[2], points[index]);
            
            // now choose a fourth point such that it doesn't lie on the plane defined by the first three
            index = 3;
            while (index < points.size() && commonPlane(points[0], points[1], points[2], points[index]))
                ++index;
            if (index == points.size()) {
                if (index != 3)
                    swap(points[3], points[index]);
                
                initialize(points[0], points[1], points[2], points[3]);
                addPoints(points.begin() + 4, points.end());
            }
        }
    }
    
    ~Polyhedron() {
        deleteFaces();
        deleteVertices();
        deleteEdges();
    }
    
    Vertex* vertices() const {
        return m_edges->m_origin;
    }
    
    Edge* edges() const {
        return m_edges;
    }
    
    Face* faces() const {
        return m_edges->m_face;
    }
private:
    void initialize(const V& p1, const V& p2, const V& p3, const V& p4) {
        using std::swap;
        assert(!commonPlane(p1, p2, p3, p4));
        
        Vertex* v1 = new Vertex(p1);
        Vertex* v2 = new Vertex(p2);
        Vertex* v3 = new Vertex(p3);
        Vertex* v4 = new Vertex(p4);
        
        insertVertex(v1, v2);
        insertVertex(v2, v3);
        insertVertex(v3, v4);
        
        const V d1 = v4->position() - v2->position();
        const V d2 = v4->position() - v3->position();
        const V d3 = v1->position() - v4->position();
        const V n1 = crossed(d1, d2);
        
        if (d3.dot(n1) > 0.0)
            swap(v2, v3);
        
        Edge* e1 = new Edge(v2);
        Edge* e2 = new Edge(v3);
        Edge* e3 = new Edge(v4);
        Edge* e4 = new Edge(v1);
        Edge* e5 = new Edge(v3);
        Edge* e6 = new Edge(v2);
        Edge* e7 = new Edge(v1);
        Edge* e8 = new Edge(v2);
        Edge* e9 = new Edge(v4);
        Edge* e10 = new Edge(v1);
        Edge* e11 = new Edge(v4);
        Edge* e12 = new Edge(v3);
        
        conjoinTwins(e1, e5);
        conjoinTwins(e2, e11);
        conjoinTwins(e3, e8);
        conjoinTwins(e4, e12);
        conjoinTwins(e6, e7);
        conjoinTwins(e9, e10);
        
        linkTriangle(e1, e2, e3);
        linkTriangle(e4, e5, e6);
        linkTriangle(e7, e8, e9);
        linkTriangle(e10, e11, e12);
        
        Face* f1 = new Face(e1);
        Face* f2 = new Face(e4);
        Face* f3 = new Face(e7);
        Face* f4 = new Face(e10);
        
        insertFace(f1, f2);
        insertFace(f2, f3);
        insertFace(f3, f4);
        
        m_edges = e1;
        
        assert(checkConsistency());
    }
    
    class Conflicts {
    private:
        typedef std::map<V, FaceList> VertexConflicts;
        typedef std::map<Face*, typename V::List> FaceConflicts;
        
        VertexConflicts m_vertexConflicts;
        FaceConflicts m_faceConflicts;
    public:
        template <typename I>
        Conflicts(I curPoint, I endPoint, Face* faces) {
            assert(faces != NULL);
            insertAllConflicts(curPoint, endPoint, faces);
        }
        
        bool empty() const {
            assert(m_vertexConflicts.empty() == m_faceConflicts.empty());
            return m_vertexConflicts.empty();
        }
        
        bool hasConflicts(const V& point) const {
            return m_vertexConflicts.find(point) != m_vertexConflicts.end();
        }
        
        bool haveConflicts(const FaceList& faces) const {
            typename FaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Face* face = *it;
                if (hasConflicts(face))
                    return true;
            }
            return false;
        }
        
        bool hasConflicts(Face* face) const {
            return m_faceConflicts.find(face) != m_faceConflicts.end();
        }
        
        const FaceList& vertexConflicts(const V& point) const {
            static const FaceList Empty(0);
            typename VertexConflicts::const_iterator it = m_vertexConflicts.find(point);
            if (it == m_vertexConflicts.end())
                return Empty;
            return it->second;
        }
        
        const typename V::List& faceConflicts(Face* face) const {
            static const typename V::List Empty(0);
            typename FaceConflicts::const_iterator it = m_faceConflicts.find(face);
            if (it == m_faceConflicts.end())
                return Empty;
            return it->second;
        }
        
        void insertConflicts(Face* face, Face* oldFace1, Face* oldFace2) {
            insertConflicts(faceConflicts(oldFace1), face);
            insertConflicts(faceConflicts(oldFace2), face);
        }
        
        void removeConflicts(const V& point) {
            typename VertexConflicts::iterator vIt = m_vertexConflicts.find(point);
            if (vIt != m_vertexConflicts.end()) {
                const FaceList& faces = vIt->second;
                typename FaceList::const_iterator fIt, fEnd;
                for (fIt = faces.begin(), fEnd = faces.end(); fIt != fEnd; ++fIt) {
                    Face* face = *fIt;
                    removeFaceConflict(face, point);
                }
                
                m_vertexConflicts.erase(vIt);
            }
        }
        
        void removeConflicts(const FaceList& faces) {
            typename FaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Face* face = *it;
                removeConflicts(face);
            }
        }
        
        void removeConflicts(Face* face) {
            typename FaceConflicts::iterator it = m_faceConflicts.find(face);
            if (it != m_faceConflicts.end()) {
                const typename V::List& vertices = it->second;
                typename V::List::const_iterator vIt, vEnd;
                for (vIt = vertices.begin(), vIt = vertices.end(); vIt != vEnd; ++vIt) {
                    const V& point = *vIt;
                    removePointConflicts(point, face);
                }
                
                m_faceConflicts.erase(it);
            }
        }
    private:
        template <typename I>
        void insertAllConflicts(I curPoint, I endPoint, Face* faces) {
            Face* face = faces;
            do {
                insertConflicts(curPoint, endPoint, face);
                face = face->next();
            } while (face != faces);
        }
        
        void insertConflicts(const typename V::List& points, Face* face) {
            insertConflicts(points.begin(), points.end(), face);
        }
        
        template <typename I>
        void insertConflicts(I curPoint, I endPoint, Face* face) {
            while (curPoint != endPoint) {
                const V& point = *curPoint;
                if (face->isVisibleFrom(point))
                    insertConflict(point, face);
                ++curPoint;
            }
        }
        
        void insertConflict(const V& point, Face* face) {
            m_vertexConflicts[point].push_back(face);
            m_faceConflicts[face].push_back(point);
        }
        
        void removePointConflicts(const V& point, Face* face) {
            typename VertexConflicts::iterator vIt = m_vertexConflicts.find(point);
            assert(vIt != m_vertexConflicts.end());
            
            FaceList& faces = vIt->second;
            CHECK_BOOL(VectorUtils::erase(faces, face));
            
            if (faces.empty())
                m_vertexConflicts.erase(vIt);
        }
        
        void removeFaceConflict(Face* face, const V& point) {
            typename FaceConflicts::iterator cIt = m_faceConflicts.find(face);
            assert(cIt != m_faceConflicts.end());
            
            typename V::List& vertices = cIt->second;
            CHECK_BOOL(VectorUtils::erase(vertices, point));
            
            if (vertices.empty())
                m_faceConflicts.erase(cIt);
        }
    };
public:
    void addPoints(const typename V::List& points) {
        addPoints(points.begin(), points.end());
    }
    
    template <typename I>
    void addPoints(I curPoint, I endPoint) {
        Conflicts conflicts(curPoint, endPoint, faces());
        if (!conflicts.empty()) {
            while (curPoint != endPoint) {
                const V& point = *curPoint;
                
                // we take a copy of the visible faces on purpose so that the list doesn't change when we remove the conflicts later
                const FaceList visibleFaces = conflicts.vertexConflicts(point);
                if (visibleFaces.empty())
                    continue;
                
                // find the horizon
                EdgeList horizonEdges = findHorizon(point);
                const size_t horizonCount = horizonEdges.size();
                assert(horizonCount > 0);
                
                // create a new vertex
                Vertex* v1 = new Vertex(point);
                insertVertex(vertices(), v1);
                assert(checkConsistency());
                
                // add new faces
                Edge* lastE1 = NULL;
                for (size_t i = 0; i < horizonCount; ++i) {
                    Edge* horizonEdge = horizonEdges[i];
                    Edge* nextHorizonEdge = horizonEdges[Math::succ(i, horizonCount)];
                    
                    assert(horizonEdge->destination() == nextHorizonEdge->origin());
                    assert(horizonEdge->m_face == nextHorizonEdge->m_face);
                    
                    if (m_edges == horizonEdge || m_edges == horizonEdge->m_twin)
                        m_edges = nextHorizonEdge;
                    
                    Face* f1 = horizonEdge->m_face;
                    Face* f2 = horizonEdge->m_twin->m_face;
                    
                    Vertex* v2 = horizonEdge->destination();
                    Vertex* v3 = horizonEdge->origin();
                    
                    Face* face = createTriangle(v1, v2, v3);
                    Edge* e1 = face->edges();
                    Edge* e2 = e1->nextBoundaryEdge();
                    Edge* e3 = e2->nextBoundaryEdge();
                    
                    separateTwins(horizonEdge);
                    conjoinTwins(e2, horizonEdge);
                    if (lastE1 != NULL)
                        conjoinTwins(e3, lastE1);
                    
                    if (mergeFaceWithNeighbourIfNeccessary(face, horizonEdge->m_twin)) {
                        delete face;
                    } else {
                        insertFace(faces(), face);
                        conflicts.insertConflicts(face, f1, f2);
                    }
                    
                    lastE1 = e1;
                }
                
                // find e3 of the first created face:
                Edge* firstE2 = horizonEdges.front()->m_twin;
                Edge* firstE3 = firstE2->nextBoundaryEdge();
                assert(firstE3->m_twin == NULL);
                conjoinTwins(firstE3, lastE1);
                assert(checkConsistency());

                conflicts.removeConflicts(point);
                conflicts.removeConflicts(visibleFaces);
                
                assert(!conflicts.hasConflicts(point));
                assert(!conflicts.haveConflicts(visibleFaces));
                
                // ensure that our edge pointer does not go stale when the faces are deleted
                m_edges = firstE3;
                deleteFaces(visibleFaces);
                assert(checkConsistency());
                
                ++curPoint;
            }
        }
    }
private:
    Face* createTriangle(Vertex* v1, Vertex* v2, Vertex* v3) {
        Edge* e1 = new Edge(v1);
        Edge* e2 = new Edge(v2);
        Edge* e3 = new Edge(v3);

        linkTriangle(e1, e2, e3);
        return new Face(e1);
    }

    EdgeList findHorizon(const V& point) const {
        EdgeList result;
        Edge* first = findFirstHorizonEdge(point);
        if (first == NULL)
            return result;
        
        result.push_back(first);
        
        Edge* edge = findNextHorizonEdge(first, point);
        while (edge != first) {
            assert(edge != NULL);
            assert(edge->m_twin != NULL);
            result.push_back(edge);
            edge = findNextHorizonEdge(edge, point);
        }
        
        return result;
    }
    
    Edge* findFirstHorizonEdge(const V& point) const {
        Edge* edge = m_edges;
        do {
            assert(edge->m_twin != NULL);
            Face* face = edge->m_face;
            Face* neighbour = edge->m_twin->m_face;
            
            if (face->isVisibleFrom(point)) {
                if (!neighbour->isVisibleFrom(point))
                    return edge->m_twin;
            } else if (neighbour->isVisibleFrom(point))
                return edge;

            edge = edge->next();
        } while (edge != m_edges);
        
        return NULL;
    }
    
    Edge* findNextHorizonEdge(Edge* edge, const V& point) const {
        Edge* succ = edge->nextBoundaryEdge();
        while (succ != NULL && !succ->isHorizonFor(point))
            succ = succ->m_twin->nextBoundaryEdge();
        return succ;
    }

    bool mergeFaceWithNeighbourIfNeccessary(Face* face, Edge* boundary) {
        assert(boundary->m_face == face);
        
        Edge* boundaryTwin = boundary->m_twin;
        assert(boundaryTwin != NULL);
        
        Face* neighbour = boundaryTwin->m_face;
        if (!face->normal().equals(neighbour->normal()))
            return false;

        replaceBoundary(boundaryTwin, boundary->nextBoundaryEdge(), boundary->previousBoundaryEdge());
        removeEdge(boundary);
        
        delete boundary;
        delete boundaryTwin;
        
        return true;
    }
    
    void conjoinTwins(Edge* e1, Edge* e2) {
        assert(e1 != NULL);
        assert(e2 != NULL);
        assert(e1->m_twin == NULL);
        assert(e2->m_twin == NULL);

        e1->m_twin = e2;
        e2->m_twin = e1;
        
        if (m_edges == NULL)
            m_edges = e1;
        else
            insertEdge(m_edges, e1);
    }
    
    void separateTwins(Edge* edge) {
        assert(edge != NULL);
        assert(edge->m_twin != NULL);
        assert(edge->m_twin->m_twin == edge);
        
        removeEdge(edge);
        
        edge->m_twin->m_twin = NULL;
        edge->m_twin = NULL;
    }
    
    void insertVertex(Vertex* v1, Vertex* v2) {
        VertexLinkProcessor processor(v1);
        processor.insertAfter(v2);
    }
    
    void insertEdge(Edge* e1, Edge* e2) {
        assert(e1 != NULL);
        assert(e2 != NULL);
        assert(e1->m_twin != NULL);
        assert(e2->m_twin != NULL);
        
        EdgeLinkProcessor edgeProcessor(e1);
        edgeProcessor.insertAfter(e2);

        EdgeLinkProcessor twinProcessor(e1->m_twin);
        twinProcessor.insertAfter(e2->m_twin);
    }
    
    void removeEdge(Edge* edge) {
        assert(edge != NULL);
        assert(m_edges != edge);
        assert(m_edges != edge->m_twin);
        
        EdgeLinkProcessor edgeProcessor(edge);
        edgeProcessor.remove();

        if (edge->m_twin != NULL) {
            EdgeLinkProcessor twinProcessor(edge->m_twin);
            twinProcessor.remove();
        }
    }

    void linkTriangle(Edge* e1, Edge* e2, Edge* e3) {
        insertBoundary(e1, e2);
        insertBoundary(e2, e3);
    }
    
    void insertBoundary(Edge* e1, Edge* e2) {
        BoundaryLinkProcessor processor(e1);
        processor.insertAfter(e2);
    }
    
    void replaceBoundary(Edge* toReplace, Edge* firstReplacement, Edge* lastReplacement) {
        BoundaryLinkProcessor processor(toReplace);
        processor.replace(firstReplacement, lastReplacement);
    }
    
    void insertFace(Face* f1, Face* f2) {
        FaceLinkProcessor processor(f1);
        processor.insertAfter(f2);
    }
    
    void removeVertex(Vertex* vertex) {
        VertexLinkProcessor processor(vertex);
        processor.remove();
    }
    
    void deleteFaces(const FaceList& faces) {
        typename FaceList::const_iterator it, end;
        for (it = faces.begin(), end = faces.end(); it != end; ++it) {
            Face* face = *it;
            deleteFace(face);
        }
    }
    
    void deleteFace(Face* face) {
        Edge* boundary = face->edges();
        Edge* edge = boundary;
        do {
            Vertex* origin = edge->origin();
            if (origin->m_leaving == edge) {
                if (!origin->chooseNewLeavingEdge()) {
                    removeVertex(origin);
                    delete origin;
                }
            }

            if (edge->m_twin != NULL)
                separateTwins(edge);
            
            Edge* next = edge->nextBoundaryEdge();
            removeEdge(edge);
            delete edge;
            edge = next;
        } while (edge != boundary);

        FaceLinkProcessor processor(face);
        processor.remove();
        
        delete face;
    }
    
    void deleteFaces() {
        assert(m_edges != NULL);
        Face* face = m_edges->face();
        do {
            Face* next = face->next();
            delete face;
            face = next;
        } while (face != m_edges->face());
    }
    
    void deleteEdges() {
        assert(m_edges != NULL);
        Edge* edge = m_edges;
        do {
            Edge* next = edge->next();
            delete edge->m_twin;
            delete edge;
            edge = next;
        } while (edge != m_edges);
        m_edges = NULL;
    }
    
    void deleteVertices() {
        assert(m_edges != NULL);
        Vertex* vertex = m_edges->origin();
        do {
            Vertex* next = vertex->next();
            delete vertex;
            vertex = next;
        } while (vertex != m_edges->origin());
    }
    
    bool checkConsistency() const {
        if (!checkVertexLink())
            return false;
        if (!checkEdgeLinks())
            return false;
        if (!checkFaceLink())
            return false;
        if (!checkTwins())
            return false;
        return true;
    }
    
    bool checkVertexLink() const {
        static const size_t MaxVertexCount = 100;
        
        VertexLinkProcessor processor(vertices());
        return processor.checkLink(MaxVertexCount);
    }
    
    bool checkEdgeLinks() const {
        static const size_t MaxEdgeCount = 100;
        
        if (edges()->m_twin == NULL)
            return false;
        
        EdgeLinkProcessor processor(edges());
        EdgeLinkProcessor twinProcessor(edges()->m_twin);
        
        if (!processor.checkLink(MaxEdgeCount))
            return false;
        if (!twinProcessor.checkLink(MaxEdgeCount))
            return false;
        
        const size_t edgeCount = processor.count(MaxEdgeCount);
        const size_t twinCount = processor.count(MaxEdgeCount);
        if (edgeCount != twinCount)
            return false;
        return true;
    }
    
    bool checkTwins() const {
        Edge* edges = m_edges;
        Edge* twins = m_edges->m_twin;
        
        EdgeLinkProcessor edgeProcessor(edges);
        EdgeLinkProcessor twinProcessor(twins);
        
        Edge* edge = edges;
        do {
            Edge* twin = edge->m_twin;
            if (twin == NULL)
                return false;
            if (twin == edge)
                return false;
            if (edgeProcessor.contains(twin))
                return false;
            if (twinProcessor.contains(edge))
                return false;
        } while (edge != edges);
        return true;
    }
    
    bool checkFaceLink() const {
        static const size_t MaxFaceCount = 100;
        
        FaceLinkProcessor processor(faces());
        return processor.checkLink(MaxFaceCount);
    }
};

#endif /* defined(__TrenchBroom__Polyhedron__) */
