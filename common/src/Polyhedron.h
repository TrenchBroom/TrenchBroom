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
    class HalfEdge;
    class Face;
private:
    typedef Vec<T,3> Pos;
    typedef typename Vec<T,3>::List PosList;
    
    typedef typename DoublyLinkedList<Vertex>::Link VertexLink;
    typedef typename DoublyLinkedList<Edge>::Link EdgeLink;
    typedef typename DoublyLinkedList<HalfEdge>::Link HalfEdgeLink;
    typedef typename DoublyLinkedList<Face>::Link FaceLink;
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
public:
    class Vertex {
    private:
        friend class HalfEdge;
        friend class VertexList;
    private:
        Pos m_position;
        VertexLink m_link;
        HalfEdge* m_leaving;
    public:
        Vertex(const Pos& position) :
        m_position(position),
        m_link(this),
        m_leaving(NULL) {}
        
        const Pos& position() const {
            return m_position;
        }
    private:
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
    public:
        HalfEdge(Vertex* origin) :
        m_origin(origin),
        m_edge(NULL),
        m_face(NULL),
        m_link(this) {
            assert(m_origin != NULL);
        }
        
        Vertex* origin() const {
            return m_origin;
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
    private:
        void setEdge(Edge* edge) {
            m_edge = edge;
        }
        
        void setFace(Face* face) {
            m_face = face;
        }
    };
    
    class Face {
    private:
        HalfEdgeList m_boundary;
        FaceLink m_link;
        
        friend class FaceList;
    public:
        Face(const HalfEdgeList& boundary) :
        m_boundary(boundary),
        m_link(this) {
            assert(m_boundary.size() >= 3);
            
            typename HalfEdgeList::Iter it = m_boundary.iterator();
            while (it.hasNext()) {
                HalfEdge* edge = it.next();
                edge->setFace(this);
            }
        }
        
        const HalfEdgeList& boundary() const {
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
        
        m_edges.append(e1);
        m_edges.append(e2);
        m_edges.append(e3);
        m_edges.append(e4);
        m_edges.append(e5);
        m_edges.append(e6);
        
        Face* f1 = createTriangle(h1, h2, h3);
        Face* f2 = createTriangle(h4, h5, h6);
        Face* f3 = createTriangle(h7, h8, h9);
        Face* f4 = createTriangle(h10, h11, h12);

        m_faces.append(f1);
        m_faces.append(f2);
        m_faces.append(f3);
        m_faces.append(f4);
    }
    
    Face* createTriangle(HalfEdge* h1, HalfEdge* h2, HalfEdge* h3) {
        HalfEdgeList boundary;
        boundary.append(h1);
        boundary.append(h2);
        boundary.append(h3);
        
        return new Face(boundary);
    }
};

#endif /* defined(__TrenchBroom__Polyhedron__) */
