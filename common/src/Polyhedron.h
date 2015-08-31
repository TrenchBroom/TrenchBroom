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

template <typename T, typename FP>
class Polyhedron {
private:
    template <typename P> class FaceT;
public:
    class Vertex;
    class Edge;
    class HalfEdge;
    typedef FaceT<FP> Face;
private:
    typedef Vec<T,3> V;
    typedef typename Vec<T,3>::List PosList;
    
    class GetVertexLink;
    class GetEdgeLink;
    class GetHalfEdgeLink;
    class GetFaceLink;

    typedef typename DoublyLinkedList<Vertex, GetVertexLink>::Link VertexLink;
    typedef typename DoublyLinkedList<Edge, GetEdgeLink>::Link EdgeLink;
    typedef typename DoublyLinkedList<HalfEdge, GetHalfEdgeLink>::Link HalfEdgeLink;
    typedef typename DoublyLinkedList<Face, GetFaceLink>::Link FaceLink;
public:
    typedef DoublyLinkedList<Vertex, GetVertexLink> VertexList;
    typedef DoublyLinkedList<Edge, GetEdgeLink> EdgeList;
    typedef DoublyLinkedList<HalfEdge, GetHalfEdgeLink> HalfEdgeList;
    typedef DoublyLinkedList<Face, GetFaceLink> FaceList;
public:
    class Callback {
    public:
        virtual ~Callback();
    public: // factory methods
        virtual Plane<T,3> plane(const Face* face) const;
        virtual void faceWasCreated(Face* face);
        virtual void faceWillBeDeleted(Face* face);
        virtual void faceDidChange(Face* face);
        virtual void faceWasSplit(Face* original, Face* clone);
        virtual void facesWillBeMerged(Face* remaining, Face* toDelete);
    };
private:
    VertexList m_vertices;
    EdgeList m_edges;
    FaceList m_faces;
public: // Constructors
    Polyhedron();
    
    Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4);
    template <typename C> Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4, C& callback);

    Polyhedron(const BBox<T,3>& bounds);
    template <typename C> Polyhedron(const BBox<T,3>& bounds, C& callback);
    
    Polyhedron(typename V::List positions);
    template <typename C> Polyhedron(typename V::List positions, C& callback);

    Polyhedron(const Polyhedron<T,FP>& other);
private: // Constructor helpers
    template <typename C> void addPoints(const V& p1, const V& p2, const V& p3, const V& p4, C& callback);
    template <typename C> void setBounds(const BBox<T,3>& bounds, C& callback);
private: // Copy constructor
    class Copy;
    Polyhedron(const VertexList& vertices, const EdgeList& edges, const FaceList& faces);
public: // Destructor
    virtual ~Polyhedron();
public: // operators
    Polyhedron<T,FP>& operator=(Polyhedron<T,FP> other);
public: // swap function
    friend void swap(Polyhedron<T,FP>& first, Polyhedron<T,FP>& second) {
        using std::swap;
        swap(first.m_vertices, second.m_vertices);
        swap(first.m_edges, second.m_edges);
        swap(first.m_faces, second.m_faces);
    }
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

    void clear();
    
    struct FaceHit;
    FaceHit pickFace(const Ray<T,3>& ray) const;
private: // General purpose methods
    Vertex* findVertexByPosition(const V& position, T epsilon = Math::Constants<T>::almostZero()) const;
    Edge* findEdgeByPositions(const V& pos1, const V& pos2, T epsilon = Math::Constants<T>::almostZero()) const;
    Face* findFaceByPositions(const typename V::List& positions, T epsilon = Math::Constants<T>::almostZero()) const;
    
    bool checkInvariant() const;
    bool checkConvex() const;
    bool checkClosed() const;
    bool checkNoCoplanarFaces() const;
    bool checkNoDegenerateFaces() const;
public: // Moving vertices
    typename V::List moveVertices(const typename V::List& positions, const V& delta);
    template <typename C> typename V::List moveVertices(const typename V::List& positions, const V& delta, C& callback);
private:
    struct MoveVertexResult;
    template <typename C> MoveVertexResult moveVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex, C& callback);
    MoveVertexResult movePointVertex(Vertex* vertex, const V& destination);
    MoveVertexResult moveEdgeVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex);
    MoveVertexResult movePolygonVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex);
    template <typename C> MoveVertexResult movePolyhedronVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex, C& callback);

    template <typename C> void splitIncidentFaces(Vertex* vertex, const V& destination, C& callback);
    template <typename C> void chopFace(Face* face, HalfEdge* halfEdge, C& callback);
    template <typename C> void splitFace(Face* face, HalfEdge* halfEdge, C& callback);
    
    T computeNextMergePoint(Vertex* vertex, const V& origin, const V& destination, T lastFrac) const;
    T computeNextMergePointForIncidentNeighbour(HalfEdge* edge, const V& origin, const V& destination, T lastFrac) const;
    T computeNextMergePointForOppositeNeighbour(HalfEdge* edge, const V& origin, const V& destination, T lastFrac) const;
    T computeNextMergePointForPlane(const V& origin, const V& destination, const Plane<T,3>& plane, T lastFrac) const;
    
    template <typename C> void mergeVertices(HalfEdge* connectingEdge, C& callback);

    template <typename C> Vertex* cleanupAfterVertexMove(Vertex* vertex, C& callback);
    
    template <typename C> void mergeLeavingEdges(Vertex* vertex, C& callback);
    template <typename C> void mergeNeighboursOfColinearEdges(HalfEdge* edge1, HalfEdge* edge2, C& callback);
    
    Vertex* mergeIncomingAndLeavingEdges(Vertex* vertex);
    void mergeColinearEdges(HalfEdge* edge1, HalfEdge* edge2);

    template <typename C> Vertex* mergeIncidentFaces(Vertex* vertex, C& callback);
    template <typename C> void mergeNeighbours(HalfEdge* borderFirst, C& callback);
public: // Convex hull and adding points
    template <typename I> void addPoints(I cur, I end);
    template <typename I, typename C> void addPoints(I cur, I end, C& callback);
    void addPoint(const V& position);
    template <typename C> void addPoint(const V& position, C& callback);
private:
    typedef std::deque<Edge*> Seam;

    void addFirstPoint(const V& position);
    void addSecondPoint(const V& position);
    
    template <typename C> void addThirdPoint(const V& position, C& callback);
    void addPointToEdge(const V& position);
    
    template <typename C> void addFurtherPoint(const V& position, C& callback);
    template <typename C> void addFurtherPointToPolygon(const V& position, C& callback);
    template <typename C> void addPointToPolygon(const V& position, C& callback);
    template <typename C> void makePolygon(const typename V::List& positions, C& callback);
    template <typename C> void makePolyhedron(const V& position, C& callback);
    
    template <typename C> void addFurtherPointToPolyhedron(const V& position, C& callback);
    template <typename C> void addPointToPolyhedron(const V& position, const Seam& seam, C& callback);
    
    class SplittingCriterion;
    class SplitByVisibilityCriterion;
    class SplitByNormalCriterion;
    
    template <typename C> Seam split(const SplittingCriterion& criterion, C& callback);
    
    template <typename C> void weaveCap(const Seam& seam, C& callback);
    template <typename C> Vertex* weaveCap(const Seam& seam, const V& position, C& callback);
    template <typename C> Face* createCapTriangle(HalfEdge* h1, HalfEdge* h2, HalfEdge* h3, C& callback) const;
private: // Clipping
    class SplitByPlaneCriterion;
public:
    struct ClipResult;
    ClipResult clip(const Plane<T,3>& plane);
    template <typename C> ClipResult clip(const Plane<T,3>& plane, C& callback);
private:
    template <typename C> bool isCoplanarToAnyFace(const Plane<T,3>& plane, const C& callback) const;
    ClipResult checkIntersects(const Plane<T,3>& plane) const;

    template <typename C> void intersectWithPlane(const Plane<T,3>& plane, C& callback);
    HalfEdge* findInitialIntersectingEdge(const Plane<T,3>& plane) const;
    template <typename C> HalfEdge* intersectWithPlane(HalfEdge* firstBoundaryEdge, const Plane<T,3>& plane, C& callback);
    template <typename C> void intersectWithPlane(HalfEdge* remainingFirst, HalfEdge* deletedFirst, C& callback);
    HalfEdge* findNextIntersectingEdge(HalfEdge* searchFrom, const Plane<T,3>& plane) const;
};


#include "Polyhedron_Misc.h"
#include "Polyhedron_Clip.h"
#include "Polyhedron_Vertex.h"
#include "Polyhedron_Edge.h"
#include "Polyhedron_HalfEdge.h"
#include "Polyhedron_Face.h"
#include "Polyhedron_VertexManipulation.h"
#include "Polyhedron_ConvexHull.h"

#endif
