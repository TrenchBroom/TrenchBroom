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
    };
    
    class Face;
    class Edge {
    private:
        Vertex* m_origin;
        Edge* m_twin;
        Link<Edge> m_edgeLink;
        Face* m_face;
        Link<Edge> m_faceLink;
        
        friend class Face;
        friend class Polyhedron<T>;
    public:
        Edge(Vertex* origin) :
        m_origin(origin),
        m_twin(NULL),
        m_edgeLink(this),
        m_face(NULL),
        m_faceLink(this) {
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
        
        Edge* nextFaceEdge() const {
            return m_faceLink.next;
        }
        
        Edge* previousFaceEdge() const {
            return m_faceLink.previous;
        }

        Face* face() const {
            return m_face;
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
                assert(edge->m_twin != NULL);
                assert(edge->m_face == NULL);
                
                edge->m_face = this;
                edge = edge->nextFaceEdge();
            } while (edge != m_edges);
        }

        V orthogonal() const {
            const Edge* edge = m_edges;
            const V p1 = edge->origin()->position();
            
            edge = nextEdge(edge);
            const V p2 = edge->origin()->position();
            
            edge = nextEdge(edge);
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
        
        Edge* nextEdge(const Edge* edge) const {
            assert(edge != NULL);
            assert(edge->m_face == this);
            return edge->m_faceLink.next;
        }
    };
private:
    Edge* m_edges;
public:
    Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4) :
    m_edges(NULL) {
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
        
        insertEdge(e1, e2);
        insertEdge(e2, e3);
        insertEdge(e3, e4);
        insertEdge(e4, e6);
        insertEdge(e6, e9);

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
    
    ~Polyhedron() {
        deleteFaces();
        deleteVertices();
        deleteEdges();
    }
    
private:
    void conjoinTwins(Edge* e1, Edge* e2) {
        assert(e1 != NULL);
        assert(e1->m_twin == NULL);
        assert(e2 != NULL);
        assert(e2->m_twin == NULL);
        assert(e1->m_face == NULL || e1->m_face != e2->m_face);

        e1->m_twin = e2;
        e2->m_twin = e1;
    }
    
    void insertVertex(Vertex* v1, Vertex* v2) {
        Vertex* v3 = v1->next();
        insertBetween(v1->m_link, v1, v2->m_link, v2, v3->m_link, v3);
    }
    
    void insertEdge(Edge* e1, Edge* e2) {
        Edge* e3 = e1->next();
        insertBetween(e1->m_edgeLink, e1, e2->m_edgeLink, e2, e3->m_edgeLink, e3);
    }

    void linkTriangle(Edge* e1, Edge* e2, Edge* e3) {
        insertBoundary(e1, e2);
        insertBoundary(e2, e3);
    }
    
    void insertBoundary(Edge* e1, Edge* e2) {
        Edge* e3 = e1->nextFaceEdge();
        insertBetween(e1->m_faceLink, e1, e2->m_faceLink, e2, e3->m_faceLink, e3);
    }
    
    void insertFace(Face* f1, Face* f2) {
        Face* f3 = f1->next();
        insertBetween(f1->m_link, f1, f2->m_link, f2, f3->m_link, f3);
    }
    
    template <typename Item>
    void insertBetween(Link<Item>& predLink, Item* predItem, Link<Item>& insertLink, Item* insertItem, Link<Item>& succLink, Item* succItem) {
        insertBetween(predLink, predItem, insertLink, insertItem, insertLink, insertItem, succLink, succItem);
    }
    
    template <typename Item>
    void insertBetween(Link<Item>& predLink, Item* predItem, Link<Item>& insertFromLink, Item* insertFromItem, Link<Item>& insertToLink, Item* insertToItem, Link<Item>& succLink, Item* succItem) {
        predLink.next = insertFromItem;
        insertFromLink.previous = predItem;
        insertToLink.next = succItem;
        succLink.previous = insertToItem;
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
            delete edge;
            delete edge->m_twin;
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
};

#endif /* defined(__TrenchBroom__Polyhedron__) */
