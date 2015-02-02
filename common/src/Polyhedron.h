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
    class LinkedEdge;
    class Face;
private:
    typedef Vec<T,3> Pos;
    typedef typename Vec<T,3>::List PosList;
    
    typedef typename DoublyLinkedList<Vertex>::Link VertexLink;
    typedef typename DoublyLinkedList<Edge>::Link BoundaryLink;
    typedef typename DoublyLinkedList<LinkedEdge>::Link EdgeLink;
    typedef typename DoublyLinkedList<Face>::Link FaceLink;
public:
    class VertexList : public DoublyLinkedList<Vertex> {
    private:
        VertexLink& doGetLink(Vertex* vertex) const { return vertex->m_link; }
        const VertexLink& doGetLink(const Vertex* vertex) const { return vertex->m_link; }
    };
    
    class BoundaryList : public DoublyLinkedList<Edge> {
    private:
        BoundaryLink& doGetLink(Edge* edge) const { return edge->m_boundaryLink; }
        const BoundaryLink& doGetLink(const Edge* edge) const { return edge->m_boundaryLink; }
    };
    
    class EdgeList : public DoublyLinkedList<LinkedEdge> {
    private:
        EdgeLink& doGetLink(LinkedEdge* edge) const { return edge->m_edgeLink; }
        const EdgeLink& doGetLink(const LinkedEdge* edge) const { return edge->m_edgeLink; }
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
        Pos m_position;
        VertexLink m_link;
        Edge* m_leaving;
    public:
        Vertex(const Pos& position) :
        m_position(position),
        m_link(this),
        m_leaving(NULL) {}
        
        const Pos& position() const {
            return m_position;
        }
    private:
        void setLeaving(Edge* edge) {
            assert(edge != NULL);
            assert(edge->origin() == this);
            m_leaving = edge;
        }
    };
    
    class UnlinkedEdge;
    
    class Edge {
    private:
        friend class BoundaryList;
        friend class LinkedEdge;
        friend class UnlinkedEdge;
    protected:
        Vertex* m_origin;
        BoundaryLink m_boundaryLink;
        Face* m_face;
    public:
        Edge(Vertex* origin) :
        m_origin(origin),
        m_boundaryLink(this),
        m_face(NULL) {
            assert(m_origin != NULL);
            m_origin->setLeaving(this);
        }
        
        virtual ~Edge() {}
        
        Vertex* origin() const {
            return m_origin;
        }
        
        Vertex* destination() const {
            assert(twin() != NULL);
            return twin()->origin();
        }
        
        Edge* twin() const {
            return doGetTwin();
        }
        
        Edge* previousBoundaryEdge() const {
            return m_boundaryLink.previous();
        }
        
        Edge* nextBoundaryEdge() const {
            return m_boundaryLink.next();
        }
        
        void conjoin(Edge* twin) {
            assert(twin != NULL);
            doConjoin(twin);
        }
    private:
        virtual void doConjoin(Edge* twin) = 0;
        virtual void doConjoin(LinkedEdge* twin) = 0;
        virtual void doConjoin(UnlinkedEdge* twin) = 0;
        virtual Edge* doGetTwin() const = 0;
    };
    
    class LinkedEdge : public Edge {
    private:
        friend class EdgeList;
        friend class UnlinkedEdge;
        UnlinkedEdge* m_twin;
        EdgeLink m_edgeLink;
    public:
        LinkedEdge(Vertex* origin) :
        Edge(origin),
        m_twin(NULL),
        m_edgeLink(this) {}
    private:
        void doConjoin(Edge* twin) {
            assert(m_twin == NULL);
            twin->doConjoin(this);
        }
        
        void doConjoin(LinkedEdge* twin) {
            assert(false);
        }
        
        void doConjoin(UnlinkedEdge* twin) {
            assert(twin->m_twin == NULL);
            m_twin = twin;
            m_twin->m_twin = this;
        }
        
        Edge* doGetTwin() const {
            return m_twin;
        }
    };
    
    class UnlinkedEdge : public Edge {
    private:
        friend class LinkedEdge;
        LinkedEdge* m_twin;
    public:
        UnlinkedEdge(Vertex* origin) :
        Edge(origin),
        m_twin(NULL) {}
    private:
        void doConjoin(Edge* twin) {
            assert(m_twin == NULL);
            twin->doConjoin(this);
        }
        
        void doConjoin(LinkedEdge* twin) {
            assert(twin->m_twin == NULL);
            m_twin = twin;
            m_twin->m_twin = this;
        }
        
        void doConjoin(UnlinkedEdge* twin) {
            assert(false);
        }
        
        Edge* doGetTwin() const {
            return m_twin;
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
        m_boundary(boundary),
        m_link(this) {
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
    Polyhedron(const Pos& p1, const Pos& p2, const Pos& p3, const Pos& p4) {
        initialize(p1, p2, p3, p4);
    }
    
    size_t vertexCount() const {
        return m_vertices.size();
    }
    
    const VertexList& vertices() const {
        return m_vertices;
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
    
    void addPoints(const PosList& points) {
    }
private:
    void initialize(const Pos& p1, const Pos& p2, const Pos& p3, const Pos& p4) {
        using std::swap;
        assert(!commonPlane(p1, p2, p3, p4));
        
        Vertex* v1 = new Vertex(p1);
        Vertex* v2 = new Vertex(p2);
        Vertex* v3 = new Vertex(p3);
        Vertex* v4 = new Vertex(p4);
        
        const Pos d1 = v4->position() - v2->position();
        const Pos d2 = v4->position() - v3->position();
        const Pos d3 = v1->position() - v4->position();
        const Pos n1 = crossed(d1, d2);
        
        if (d3.dot(n1) > 0.0)
            swap(v2, v3);
        
        m_vertices.append(v1);
        m_vertices.append(v2);
        m_vertices.append(v3);
        m_vertices.append(v4);
        
        LinkedEdge* e1 = new LinkedEdge(v2);
        LinkedEdge* e2 = new LinkedEdge(v3);
        LinkedEdge* e3 = new LinkedEdge(v4);
        LinkedEdge* e4 = new LinkedEdge(v1);
        UnlinkedEdge* e5 = new UnlinkedEdge(v3);
        LinkedEdge* e6 = new LinkedEdge(v2);
        UnlinkedEdge* e7 = new UnlinkedEdge(v1);
        UnlinkedEdge* e8 = new UnlinkedEdge(v2);
        UnlinkedEdge* e9 = new UnlinkedEdge(v4);
        LinkedEdge* e10 = new LinkedEdge(v1);
        UnlinkedEdge* e11 = new UnlinkedEdge(v4);
        UnlinkedEdge* e12 = new UnlinkedEdge(v3);
        
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
        m_edges.append(e10);
        
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
