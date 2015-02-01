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
#include <vector>

// implements a doubly-connected edge list, see de Berg et. al - Computational Geometry (3rd. Ed.), PP. 29
// see also http://www.holmes3d.net/graphics/dcel/
template <typename T>
class Polyhedron {
public:
    class Vertex;
    class Edge;
    class Face;
private:
    typedef Vec<T,3> V;
    
    typedef typename DoublyLinkedList<Vertex>::Link VertexLink;
    typedef typename DoublyLinkedList<Edge>::Link EdgeLink;
    typedef typename DoublyLinkedList<Face>::Link FaceLink;
public:
    class VertexList : public DoublyLinkedList<Vertex> {
    private:
        VertexLink& doGetLink(Vertex* vertex) const { return vertex->m_link; }
        const VertexLink& doGetLink(const Vertex* vertex) const { return vertex->m_link; }
    };
    
    class BoundaryList : public DoublyLinkedList<Edge> {
    private:
        EdgeLink& doGetLink(Edge* edge) const { return edge->m_boundaryLink; }
        const EdgeLink& doGetLink(const Edge* edge) const { return edge->m_boundaryLink; }
    };
    
    class EdgeList : public DoublyLinkedList<Edge> {
    private:
        EdgeLink& doGetLink(Edge* edge) const { return edge->m_edgeLink; }
        const EdgeLink& doGetLink(const Edge* edge) const { return edge->m_edgeLink; }
    };
    
    class FaceList : public DoublyLinkedList<Face> {
    private:
        FaceLink& doGetLink(Face* face) const { return face->m_link; }
        const FaceLink& doGetLink(const Face* face) const { return face->m_link; }
    };
public:
    class Vertex {
    private:
        friend class Edge;
        friend class VertexList;
    private:
        V m_position;
        VertexLink m_link;
        Edge* m_leaving;
    public:
        Vertex(const V& position) :
        m_position(position),
        m_leaving(NULL) {}
        
        const V& position() const {
            return m_position;
        }
    private:
        void setLeaving(Edge* edge) {
            assert(edge != NULL);
            assert(edge->origin() == this);
            m_leaving = edge;
        }
    };
    
    class Edge {
    private:
        friend class BoundaryList;
        friend class EdgeList;
    private:
        Vertex* m_origin;
        Edge* m_twin;
        EdgeLink m_edgeLink;
        EdgeLink m_boundaryLink;
        Face* m_face;
    public:
        Edge(Vertex* origin) :
        m_origin(origin),
        m_twin(NULL),
        m_face(NULL) {
            assert(m_origin != NULL);
            m_origin->setLeaving(this);
        }
        
        Vertex* origin() const {
            return m_origin;
        }
        
        Vertex* destination() const {
            assert(m_twin != NULL);
            return m_twin->origin();
        }
        
        Edge* previousBoundaryEdge() const {
            return m_boundaryLink.previous();
        }
        
        Edge* nextBoundaryEdge() const {
            return m_boundaryLink.next();
        }
        
        void conjoin(Edge* twin) {
            assert(twin != NULL);
            assert(m_twin == NULL);
            assert(twin->m_twin == NULL);
            
            m_twin = twin;
            m_twin->m_twin = this;
        }
    };
    
    class Face {
    private:
        friend class FaceList;
    private:
        BoundaryList m_boundary;
        FaceLink m_link;
    public:
        Face(const BoundaryList& boundary) :
        m_boundary(boundary) {
            assert(m_boundary.size() >= 3);
        }
        
        const BoundaryList& boundary() const {
            return m_boundary;
        }
    };
    
    VertexList m_vertices;
    EdgeList m_edges;
    FaceList m_faces;
public:
    Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4) {
        initialize(p1, p2, p3, p4);
    }
    
    const VertexList& vertices() const {
        return m_vertices;
    }
    
    const EdgeList& edges() const {
        return m_edges;
    }
    
    const FaceList& faces() const {
        return m_faces;
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
        
        m_vertices.append(v1);
        m_vertices.append(v2);
        m_vertices.append(v3);
        m_vertices.append(v4);
        
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
        
        m_edges.append(e1);
        m_edges.append(e2);
        m_edges.append(e3);
        m_edges.append(e4);
        m_edges.append(e6);
        m_edges.append(e9);
        
        Face* f1 = createTriangle(e1, e2, e3);
        Face* f2 = createTriangle(e4, e5, e6);
        Face* f3 = createTriangle(e7, e8, e9);
        Face* f4 = createTriangle(e10, e11, e12);

        m_faces.append(f1);
        m_faces.append(f2);
        m_faces.append(f3);
        m_faces.append(f4);
    }
    
    Face* createTriangle(Edge* e1, Edge* e2, Edge* e3) {
        BoundaryList boundary;
        boundary.append(e1);
        boundary.append(e2);
        boundary.append(e3);
        
        return new Face(boundary);
    }
};

#endif /* defined(__TrenchBroom__Polyhedron__) */
