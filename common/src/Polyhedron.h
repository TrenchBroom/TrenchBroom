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
    
    class Edge;
    class Vertex {
    private:
        V m_position;
        Edge* m_leaving;
        
        friend class Edge;
    public:
        Vertex(const V& position) :
        m_position(position),
        m_leaving(NULL) {}
        
        const V& position() const {
            return m_position;
        }
    };
    
    class Face;
    class Edge {
    private:
        Vertex* m_origin;
        Edge* m_twin;
        Edge* m_next;
        Edge* m_previous;
        Face* m_face;
    public:
        Edge(Vertex* origin) :
        m_origin(origin),
        m_twin(NULL),
        m_next(NULL),
        m_previous(NULL),
        m_face(NULL) {
            assert(m_origin != NULL);
            
            if (m_origin->m_leaving == NULL)
                m_origin->m_leaving = this;
        }
        
        void insertAfter(Edge* next) {
            assert(next != NULL);
            next->m_next = m_next;
            next->m_previous = this;
            if (m_next != NULL)
                m_next->m_previous = next;
            m_next = next;
        }

        void close(Edge* next) {
            assert(next != NULL);
            assert(next->m_next != NULL);
            assert(next->m_previous == NULL);
            assert(m_next == NULL);
            assert(m_previous != NULL);
            
            next->m_previous = this;
            m_next = next;
        }
        
        void conjoin(Edge* twin) {
            assert(twin != NULL);
            assert(twin->m_twin == NULL);
            assert(m_twin == NULL);
            
            m_twin = twin;
            m_twin->m_twin = this;
        }
    };
    
    class Face {
    private:
        Edge* m_edges;
    public:
        Face(Edge* edges) :
        m_edges(edges) {
            assert(m_edges != NULL);
        }
    };

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
        
        const V d1 = v4->position() - v2->position();
        const V d2 = v4->position() - v3->position();
        const V d3 = v1->position() - v4->position();
        const V n1 = crossed(d1, d2);
        
        if (d3.dot(n1) > 0.0)
            swap(v2, v3);
        
        Edge* e1 = new Edge(v2);
        Edge* e2 = new Edge(v3);
        Edge* e3 = new Edge(v4);
        
        e1->insertAfter(e2);
        e2->insertAfter(e3);
        e3->close(e1);
        
        Face* f1 = new Face(e1);
        
        
        Edge* e4 = new Edge(v1);
        Edge* e5 = new Edge(v3);
        Edge* e6 = new Edge(v2);
        
        e4->insertAfter(e5);
        e5->insertAfter(e6);
        e6->close(e4);
        
        e1->conjoin(e5);
        
        Face* f2 = new Face(e4);
        
        
        Edge* e7 = new Edge(v1);
        Edge* e8 = new Edge(v2);
        Edge* e9 = new Edge(v4);
        
        e7->insertAfter(e8);
        e8->insertAfter(e9);
        e9->close(e7);
        
        e6->conjoin(e7);
        e3->conjoin(e8);
        
        Face* f3 = new Face(e7);
        
        
        Edge* e10 = new Edge(v1);
        Edge* e11 = new Edge(v4);
        Edge* e12 = new Edge(v3);
        
        e10->insertAfter(e11);
        e11->insertAfter(e12);
        e12->close(e10);
        
        e9->conjoin(e10);
        e2->conjoin(e11);
        e4->conjoin(e12);
        
        Face* f4 = new Face(e9);
        
        m_edges = e1;
    }
};

#endif /* defined(__TrenchBroom__Polyhedron__) */
