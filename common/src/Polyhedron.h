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

#ifndef TrenchBroom_Polyhedron_h
#define TrenchBroom_Polyhedron_h

#include "VecMath.h"
#include "DoublyLinkedList.h"

#include <cassert>
#include <deque>
#include <queue>
#include <vector>

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
public:
    class VertexList : public DoublyLinkedList<Vertex> {
    private:
        VertexLink& doGetLink(Vertex* vertex) const;
        const VertexLink& doGetLink(const Vertex* vertex) const;
    };
    
    class EdgeList : public DoublyLinkedList<Edge> {
    private:
        EdgeLink& doGetLink(Edge* edge) const;
        const EdgeLink& doGetLink(const Edge* edge) const;
    };
    
    class HalfEdgeList : public DoublyLinkedList<HalfEdge> {
    private:
        HalfEdgeLink& doGetLink(HalfEdge* edge) const;
        const HalfEdgeLink& doGetLink(const HalfEdge* edge) const;
    };
    
    class FaceList : public DoublyLinkedList<Face> {
    private:
        FaceLink& doGetLink(Face* face) const;
        const FaceLink& doGetLink(const Face* face) const;
    };
private:
    VertexList m_vertices;
    EdgeList m_edges;
    FaceList m_faces;
public: // Constructors
    Polyhedron();
    Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4);
    Polyhedron(const BBox<T,3>& bounds);
    Polyhedron(typename V::List positions);
private:
    Polyhedron(const VertexList& vertices, const EdgeList& edges, const FaceList& faces);
public: // Destructor
    ~Polyhedron();
public: // Accessors
    size_t vertexCount() const;
    const VertexList& vertices() const;
    typename V::List vertexPositions() const;
    typename V::List& vertexPositions(typename V::List& positions) const;
    bool hasVertex(const V& position) const;
    
    size_t edgeCount() const;
    const EdgeList& edges() const;
    bool hasEdge(const V& pos1, const V& pos2) const;
    
    size_t faceCount() const;
    const FaceList& faces() const;
    bool hasFace(const typename V::List& positions) const;
    
    bool empty() const;
    bool point() const;
    bool edge() const;
    bool polygon() const;
    bool polyhedron() const;
    bool closed() const;
private: // General purpose methods
    void initializeTetrahedron(const V& p1, const V& p2, const V& p3, const V& p4);
    bool chooseNonColinearPoints(typename V::List& positions) const;
    
    Vertex* findVertexByPosition(const V& position, T epsilon = Math::Constants<T>::almostZero()) const;
    Edge* findEdgeByPositions(const V& pos1, const V& pos2, T epsilon = Math::Constants<T>::almostZero()) const;
    Face* findFaceByPositions(const typename V::List& positions, T epsilon = Math::Constants<T>::almostZero()) const;
    
    Face* createTriangle(HalfEdge* h1, HalfEdge* h2, HalfEdge* h3) const;
    
    void clear();
    
    bool checkInvariant() const;
    bool checkConvex() const;
    bool checkClosed() const;
    bool checkNoCoplanarFaces() const;
    bool checkNoDegenerateFaces() const;
public: // Moving vertices
    typename V::List moveVertices(const typename V::List& positions, const V& delta);
private:
    struct MoveVertexResult;
    MoveVertexResult moveVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex);
    MoveVertexResult movePointVertex(Vertex* vertex, const V& destination);
    MoveVertexResult moveEdgeVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex);
    MoveVertexResult movePolygonVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex);
    MoveVertexResult movePolyhedronVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex);

    void splitIncidentFaces(Vertex* vertex, const V& destination);
    void chopFace(Face* face, HalfEdge* halfEdge);
    void splitFace(Face* face, HalfEdge* halfEdge);
    
    T computeNextMergePoint(Vertex* vertex, const V& origin, const V& destination, T lastFrac) const;
    T computeNextMergePointForIncidentNeighbour(HalfEdge* edge, const V& origin, const V& destination, T lastFrac) const;
    T computeNextMergePointForOppositeNeighbour(HalfEdge* edge, const V& origin, const V& destination, T lastFrac) const;
    T computeNextMergePointForPlane(const V& origin, const V& destination, const Plane<T,3>& plane, T lastFrac) const;
    
    void mergeVertices(HalfEdge* connectingEdge);

    Vertex* cleanupAfterVertexMove(Vertex* vertex);
    
    void mergeLeavingEdges(Vertex* vertex);
    void mergeNeighboursOfColinearEdges(HalfEdge* edge1, HalfEdge* edge2);
    
    Vertex* mergeIncomingAndLeavingEdges(Vertex* vertex);
    void mergeColinearEdges(HalfEdge* edge1, HalfEdge* edge2);

    Vertex* mergeIncidentFaces(Vertex* vertex);
    void mergeNeighbours(HalfEdge* borderFirst);
public: // Convex hull and adding points
    template <typename I>
    void addPoints(I cur, I end) { while (cur != end) addPoint(*cur++); }
    void addPoint(const V& position);
private:
    typedef std::deque<Edge*> Seam;

    void addFirstPoint(const V& position);
    void addSecondPoint(const V& position);
    
    void addThirdPoint(const V& position);
    void addPointToEdge(const V& position);
    
    void addFurtherPoint(const V& position);
    void addFurtherPointToPolygon(const V& position);
    void addPointToPolygon(const V& position);
    void makePolygon(const typename V::List& positions);
    void makePolyhedron(const V& position);
    
    void addFurtherPointToPolyhedron(const V& position);
    void addPointToPolyhedron(const V& position, const Seam& seam);
    
    class SplittingCriterion;
    class SplitByVisibilityCriterion;
    class SplitByNormalCriterion;
    
    Seam split(const SplittingCriterion& criterion);
    
    Vertex* weaveCap(const Seam& seam, const V& position);
};


#include "Polyhedron_Misc.h"
#include "Polyhedron_Vertex.h"
#include "Polyhedron_Edge.h"
#include "Polyhedron_HalfEdge.h"
#include "Polyhedron_Face.h"
#include "Polyhedron_VertexManipulation.h"
#include "Polyhedron_ConvexHull.h"

#endif
