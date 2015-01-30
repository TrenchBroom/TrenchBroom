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
    struct LinkItem {
        Link<Item>& link;
        Item* item;

        LinkItem(Link<Item>& i_link, Item* i_item) :
        link(i_link),
        item(i_item) {
            assert(item != NULL);
        }
        
        bool selfLoop() const {
            assert(!link.selfLoop() || (link.next == item && link.previous == item));
            return link.selfLoop();
        }
        
        bool predecessorOf(const LinkItem& succ) const {
            return succ.successorOf(*this);
        }
        
        bool successorOf(const LinkItem& pred) const {
            assert((pred.link.next == item) == (link.previous == pred.item));
            return pred.link.next == item;
        }
    };
    
    template <typename Item>
    static void insertBetween(LinkItem<Item>& pred, LinkItem<Item>& toInsert, LinkItem<Item>& succ) {
        assert(succ.successorOf(pred));
        assert(toInsert.selfLoop());
        
        pred.link.next = toInsert.item;
        toInsert.link.previous = pred.item;
        succ.link.previous = toInsert.item;
        toInsert.link.next = succ.item;
    }
public:
    class Edge;
    class Vertex {
    private:
        V m_position;
        Link<Vertex> m_link;
        Edge* m_leaving;
        
        friend class Edge;
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
        
        void insertAfter(Vertex* next) {
            assert(next != NULL);
            
            LinkItem<Vertex> pred(m_link, this);
            LinkItem<Vertex> toInsert(next->m_link, next);
            LinkItem<Vertex> succ(m_link.next->m_link, m_link.next);
            insertBetween(pred, toInsert, succ);
        }
        
        void deleteAll() {
            Vertex* vertex = m_link.next;
            while (vertex != this) {
                Vertex* next = vertex->m_link.next;
                delete vertex;
                vertex = next;
            }
            
            delete this;
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
        
        void insertEdgeLinkAfter(Edge* next) {
            assert(m_twin != NULL);
            assert(next != NULL);
            assert(next->m_twin != NULL);
            
            doInsertEdgeLinkAfter(next);
            m_twin->doInsertEdgeLinkAfter(next->m_twin);
        }
        
        void insertFaceLinkAfter(Edge* next) {
            assert(next != NULL);
            
            LinkItem<Edge> pred(m_faceLink, this);
            LinkItem<Edge> toInsert(next->m_faceLink, next);
            LinkItem<Edge> succ(m_faceLink.next->m_faceLink, m_faceLink.next);
            insertBetween(pred, toInsert, succ);
        }
        
        void conjoin(Edge* twin) {
            assert(twin != NULL);
            assert(twin->m_twin == NULL);
            assert(m_twin == NULL);
            
            m_twin = twin;
            m_twin->m_twin = this;
        }
        
        void deleteAll() {
            Edge* edge = m_edgeLink.next;
            while (edge != this) {
                Edge* next = edge->m_edgeLink.next;
                delete edge->m_twin;
                delete edge;
                edge = next;
            }
            
            delete this;
        }
    private:
        void doInsertEdgeLinkAfter(Edge* next) {
            assert(next != NULL);
            
            LinkItem<Edge> pred(m_edgeLink, this);
            LinkItem<Edge> toInsert(next->m_edgeLink, next);
            LinkItem<Edge> succ(m_edgeLink.next->m_edgeLink, m_edgeLink.next);
            insertBetween(pred, toInsert, succ);
        }
        
    };
    
    class Face {
    private:
        Edge* m_edges;
        Link<Face> m_link;
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

        V normal() const {
            const Edge* edge = m_edges;
            const V p1 = edge->origin()->position();
            
            edge = nextEdge(edge);
            const V p2 = edge->origin()->position();
            
            edge = nextEdge(edge);
            const V p3 = edge->origin()->position();
            
            return crossed(p2 - p1, p3 - p1).normalized();
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
        
        void insertAfter(Face* next) {
            assert(next != NULL);
            
            LinkItem<Face> pred(m_link, this);
            LinkItem<Face> toInsert(next->m_link, next);
            LinkItem<Face> succ(m_link.next->m_link, m_link.next);
            insertBetween(pred, toInsert, succ);
        }
        
        void deleteAll() {
            Face* face = m_link.next;
            while (face != this) {
                Face* next = face->m_link.next;
                delete face;
                face = next;
            }
            
            delete this;
        }
    };
private:
    Vertex* m_vertices;
    size_t m_vertexCount;
    
    Edge* m_edges;
    size_t m_edgeCount;
    
    Face* m_faces;
    size_t m_faceCount;
public:
    Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4) :
    m_vertices(NULL),
    m_vertexCount(0),
    m_edges(NULL),
    m_edgeCount(0),
    m_faces(NULL),
    m_faceCount(0) {
        using std::swap;
        assert(!commonPlane(p1, p2, p3, p4));
        
        Vertex* v1 = new Vertex(p1);
        Vertex* v2 = new Vertex(p2);
        Vertex* v3 = new Vertex(p3);
        Vertex* v4 = new Vertex(p4);
        
        v1->insertAfter(v2);
        v2->insertAfter(v3);
        v3->insertAfter(v4);
        
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
        
        e1->conjoin(e5);
        e2->conjoin(e11);
        e3->conjoin(e8);
        e4->conjoin(e12);
        e6->conjoin(e7);
        e9->conjoin(e10);
        
        e1->insertEdgeLinkAfter(e2);
        e2->insertEdgeLinkAfter(e3);
        e3->insertEdgeLinkAfter(e4);
        e4->insertEdgeLinkAfter(e6);
        e6->insertEdgeLinkAfter(e9);
        
        e1->insertFaceLinkAfter(e2);
        e2->insertFaceLinkAfter(e3);

        e4->insertFaceLinkAfter(e5);
        e5->insertFaceLinkAfter(e6);

        e7->insertFaceLinkAfter(e8);
        e8->insertFaceLinkAfter(e9);
        
        e10->insertFaceLinkAfter(e11);
        e11->insertFaceLinkAfter(e12);
        
        Face* f1 = new Face(e1);
        Face* f2 = new Face(e4);
        Face* f3 = new Face(e7);
        Face* f4 = new Face(e10);
        
        f1->insertAfter(f2);
        f2->insertAfter(f3);
        f3->insertAfter(f4);
        
        m_vertexCount = 4;
        m_vertices = v1;
        
        m_edgeCount = 6;
        m_edges = e1;
        
        m_faceCount = 4;
        m_faces = f1;
    }
    
    size_t vertexCount() const {
        return m_vertexCount;
    }
    
    Vertex* vertices() const {
        return m_vertices;
    }
    
    size_t edgeCount() const {
        return m_edgeCount;
    }
    
    Edge* edges() const {
        return m_edges;
    }
    
    size_t faceCount() const {
        return m_faceCount;
    }
    
    Face* faces() const {
        return m_faces;
    }
    
    bool containsPoint(const V& point, const T epsilon = Math::Constants<T>::pointStatusEpsilon()) const {
        const Face* face = m_faces;
        for (size_t i = 0; i < m_faceCount; ++i) {
            if (face->pointStatus(point) == Math::PointStatus::PSAbove)
                return false;
            face = face->next();
        }
        return true;
    }
    
    ~Polyhedron() {
        m_faces->deleteAll();
        m_edges->deleteAll();
        m_vertices->deleteAll();
    }
    
    
};

#endif /* defined(__TrenchBroom__Polyhedron__) */
