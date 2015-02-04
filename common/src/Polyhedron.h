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
    typedef Vec<T,3> V;
    typedef typename Vec<T,3>::List PosList;
    
    typedef typename DoublyLinkedList<Vertex>::Link VertexLink;
    typedef typename DoublyLinkedList<Edge>::Link EdgeLink;
    typedef typename DoublyLinkedList<HalfEdge>::Link HalfEdgeLink;
    typedef typename DoublyLinkedList<Face>::Link FaceLink;
    
    typedef std::vector<Edge*> EdgeVec;
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
        
        Edge* findNextSplittingEdge(Edge* last) const {
            assert(last != NULL);

            HalfEdge* halfEdge = last->firstEdge()->next();
            Edge* next = halfEdge->edge();
            if (!next->fullySpecified())
                return NULL;
            
            MatchResult result = matches(next);
            while (result != MatchResult_First && result != MatchResult_Second && next != last) {
                halfEdge = halfEdge->twin()->next();
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
public:
    class Vertex {
    private:
        friend class HalfEdge;
        friend class VertexList;
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
    private:
        void flip() {
            using std::swap;
            swap(m_first, m_second);
        }
        
        Edge* split() {
            Edge* result = new Edge(m_second);
            m_first->setAsLeaving();
            m_second = NULL;
            return result;
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
            
            typename HalfEdgeList::Iterator it = m_boundary.iterator();
            while (it.hasNext()) {
                HalfEdge* edge = it.next();
                edge->setFace(this);
            }
        }
        
        ~Face() {
            m_boundary.deleteAll();
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
    private:
        Math::PointStatus::Type pointStatus(const V& point, const T epsilon = Math::Constants<T>::pointStatusEpsilon()) const {
            const V norm = normal();
            const T offset = origin().dot(norm);
            const T distance = point.dot(norm) - offset;
            if (distance > epsilon)
                return Math::PointStatus::PSAbove;
            if (distance < -epsilon)
                return Math::PointStatus::PSBelow;
            return Math::PointStatus::PSInside;
        }
    };
    
    VertexList m_vertices;
    EdgeList m_edges;
    FaceList m_faces;
public:
    Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4) {
        initialize(p1, p2, p3, p4);
    }
private:
    Polyhedron(const VertexList& vertices, const EdgeList& edges, const FaceList& faces) :
    m_vertices(vertices),
    m_edges(edges),
    m_faces(faces) {}
public:
    ~Polyhedron() {
        m_faces.deleteAll();
        m_edges.deleteAll();
        m_vertices.deleteAll();
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
    
    bool closed() const {
        return vertexCount() + faceCount() == edgeCount() + 2;
    }
    
    struct SplitResult {
        struct SeamElement {
            Edge* matched;
            Edge* unmatched;
            
            SeamElement(Edge* i_matched, Edge* i_unmatched) :
            matched(i_matched),
            unmatched(i_unmatched) {
                assert(matched != NULL);
                assert(unmatched != NULL);
            }
        };
        
        typedef std::vector<SeamElement> Seam;
        
        Polyhedron* unmatched;
        Seam seam;
        
        SplitResult(Polyhedron* i_unmatched, const Seam& i_seam) :
        unmatched(i_unmatched),
        seam(i_seam) {
            assert(unmatched != NULL);
            assert(seam.size() >= 3);
        }
        
        SplitResult() :
        unmatched(NULL) {}
        
        bool success() const {
            return unmatched != NULL;
        }
    };
    
    // This algorithm splits this polyhedron along the edges where one of the adjacant faces matches
    // the given criterion and the other does not. The algorithm runs linear in the sum of the numbers
    // of all vertices, edges, and faces.
    SplitResult split(const SplittingCriterion& criterion) {
        typedef typename SplitResult::SeamElement SeamElement;
        
        VertexList vertices;
        EdgeList edges;
        FaceList faces;
        typename SplitResult::Seam seam;
        
        Edge* splittingEdge = criterion.findFirstSplittingEdge(m_edges);
        if (splittingEdge == NULL)
            return SplitResult();
        
        // First, go along the splitting seam and split the edges into two edges with one half edge each.
        // Both the resulting edges have their only half edge as their first edge.
        do {
            assert(splittingEdge != NULL);
            
            Edge* duplicate = splittingEdge->split();
            edges.append(duplicate);
            
            seam.push_back(SeamElement(splittingEdge, duplicate));
            
            Edge* nextSplittingEdge = criterion.findNextSplittingEdge(splittingEdge);
            assert(nextSplittingEdge != splittingEdge);
            assert(nextSplittingEdge == NULL || splittingEdge->secondVertex() == nextSplittingEdge->firstVertex());
            splittingEdge = nextSplittingEdge;
        } while (splittingEdge != NULL);
        
        typename VertexList::Iterator vertexIt;
        typename EdgeList::Iterator edgeIt;
        typename FaceList::Iterator faceIt;
        
        // Now iterate all the edges we have created so far and duplicate the vertices, as these are shared
        // between the two resulting polyhedra.
        edgeIt = edges.iterator();
        while (edgeIt.hasNext()) {
            Edge* edge = edgeIt.next();
            HalfEdge* halfEdge = edge->firstEdge();
            Vertex* vertex = halfEdge->origin();
            Vertex* duplicate = new Vertex(vertex->position());
            
            // This vertex should remain in this polyhedron, so we duplicate it and set it
            // as the origin of all half edges leaving from it that belong to this polyhedron.
            // For this, we iterate over the half edges in clockwise order and go to their twins.
            // Since the vertex is shared, we will reach another edge that was previously split.
            // At that edge, the twin of the half edge's predecessor will be NULL.
            
            while (halfEdge != NULL) {
                halfEdge->setOrigin(duplicate);
                halfEdge = halfEdge->previous()->twin();
            }
            
            vertices.append(duplicate);
        }
        
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
                vertices.append(vertex);
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
                edges.append(edge);
            }
        }
        
        faceIt = m_faces.iterator();
        while (faceIt.hasNext()) {
            Face* face = faceIt.next();
            
            if (!criterion.matches(face)) {
                faceIt.remove();
                faces.append(face);
            }
        }
        
        return SplitResult(new Polyhedron(vertices, edges, faces), seam);
    }
private:
    Edge* findFirstSplittingEdge(const SplittingCriterion& criterion) {
        typename EdgeList::Iterator it = m_edges.iterator();
        while (it.hasNext()) {
            Edge* edge = it.next();
            if (edge->flipIfMatches(criterion))
                return edge;
        }
        return NULL;
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
