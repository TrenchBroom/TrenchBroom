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
    
    size_t edgeCount() const;
    const EdgeList& edges() const;
    
    size_t faceCount() const;
    const FaceList& faces() const;
    
    bool empty() const;
    bool point() const;
    bool edge() const;
    bool polygon() const;
    bool polyhedron() const;
    bool closed() const;
public: // Moving vertices
    typename V::List moveVertices(const typename V::List& positions, const V& delta);
private:
    struct MoveVertexResult;
    MoveVertexResult moveVertex(Vertex* vertex, const V& destination, bool allowMergeIncidentVertex);

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
    
    void weaveCap(const Seam& seam);
    void weaveCap(const Seam& seam, const V& position);
    void mergeCoplanarFaces(const Seam& seam);
private: // General purpose methods
    void initializeTetrahedron(const V& p1, const V& p2, const V& p3, const V& p4);
    bool chooseNonColinearPoints(typename V::List& positions) const;

    Vertex* findVertexByPosition(const V& position, T epsilon = Math::Constants<T>::almostZero()) const;
    
    Face* createTriangle(HalfEdge* h1, HalfEdge* h2, HalfEdge* h3) const;
    
    void clear();
    
    bool checkInvariant() const;
    bool isConvex() const;
};


template <typename T>
Polyhedron<T>::Polyhedron() {}

template <typename T>
Polyhedron<T>::Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4) {
    initializeTetrahedron(p1, p2, p3, p4);
}

template <typename T>
Polyhedron<T>::Polyhedron(const BBox<T,3>& bounds) {
    const V p1(bounds.min.x(), bounds.min.y(), bounds.min.z());
    const V p2(bounds.min.x(), bounds.min.y(), bounds.max.z());
    const V p3(bounds.min.x(), bounds.max.y(), bounds.min.z());
    const V p4(bounds.min.x(), bounds.max.y(), bounds.max.z());
    const V p5(bounds.max.x(), bounds.min.y(), bounds.min.z());
    const V p6(bounds.max.x(), bounds.min.y(), bounds.max.z());
    const V p7(bounds.max.x(), bounds.max.y(), bounds.min.z());
    const V p8(bounds.max.x(), bounds.max.y(), bounds.max.z());
    
    initializeTetrahedron(p1, p2, p3, p5);
    addPoint(p4);
    addPoint(p6);
    addPoint(p7);
    addPoint(p8);
}

template <typename T>
Polyhedron<T>::Polyhedron(typename V::List positions) {
    if (chooseNonColinearPoints(positions)) {
        initializeTetrahedron(positions[0], positions[1], positions[2], positions[3]);
        addPoints(positions.begin() + 4, positions.end());
    }
}

template <typename T>
Polyhedron<T>::Polyhedron(const VertexList& vertices, const EdgeList& edges, const FaceList& faces) :
m_vertices(vertices),
m_edges(edges),
m_faces(faces) {}

template <typename T>
Polyhedron<T>::~Polyhedron() {
    clear();
}

template <typename T>
size_t Polyhedron<T>::vertexCount() const {
    return m_vertices.size();
}

template <typename T>
const typename Polyhedron<T>::VertexList& Polyhedron<T>::vertices() const {
    return m_vertices;
}

template <typename T>
typename Polyhedron<T>::V::List Polyhedron<T>::vertexPositions() const {
    typename V::List positions;
    positions.reserve(vertexCount());
    return vertexPositiosn(positions);
}

template <typename T>
typename Polyhedron<T>::V::List& Polyhedron<T>::vertexPositions(typename V::List& positions) const {
    typename VertexList::ConstIterator it = m_vertices.iterator();
    while (it.hasNext()) {
        const Vertex* v = it.next();
        positions.push_back(v->position());
    }
    return positions;
}

template <typename T>
size_t Polyhedron<T>::edgeCount() const {
    return m_edges.size();
}

template <typename T>
const typename Polyhedron<T>::EdgeList& Polyhedron<T>::edges() const {
    return m_edges;
}

template <typename T>
size_t Polyhedron<T>::faceCount() const {
    return m_faces.size();
}

template <typename T>
const typename Polyhedron<T>::FaceList& Polyhedron<T>::faces() const {
    return m_faces;
}

template <typename T>
bool Polyhedron<T>::empty() const {
    return vertexCount() == 0;
}

template <typename T>
bool Polyhedron<T>::point() const {
    return vertexCount() == 1;
}

template <typename T>
bool Polyhedron<T>::edge() const {
    return vertexCount() == 2;
}

template <typename T>
bool Polyhedron<T>::polygon() const {
    return faceCount() == 1;
}

template <typename T>
bool Polyhedron<T>::polyhedron() const {
    return faceCount() > 3;
}

template <typename T>
bool Polyhedron<T>::closed() const {
    return vertexCount() + faceCount() == edgeCount() + 2;
}

template <typename T>
typename Polyhedron<T>::V::List Polyhedron<T>::moveVertices(const typename V::List& positions, const V& delta) {
    if (delta.null())
        return positions;
    
    typename V::List newPositions;
    for (size_t i = 0; i < positions.size(); ++i) {
        Vertex* vertex = findVertexByPosition(positions[i]);
        assert(vertex != NULL);
        
        const V destination = vertex->position() + delta;
        const MoveVertexResult result = moveVertex(vertex, destination, true);
        if (!result.deleted())
            newPositions.push_back(result.vertex->position());
    }
    
    return newPositions;
}

template <typename T>
struct Polyhedron<T>::MoveVertexResult {
    typedef enum {
        Type_VertexMoved,
        Type_VertexDeleted,
        Type_VertexUnchanged
    } Type;
    
    const Type type;
    Vertex* vertex;
    
    MoveVertexResult(const Type i_type, Vertex* i_vertex = NULL) :
    type(i_type),
    vertex(i_vertex) {
        assert(type != Type_VertexDeleted || vertex == NULL);
    }
    
    bool moved() const     { return type == Type_VertexMoved; }
    bool deleted() const   { return type == Type_VertexDeleted; }
    bool unchanged() const { return type == Type_VertexUnchanged; }
};

template <typename T>
typename Polyhedron<T>::MoveVertexResult Polyhedron<T>::moveVertex(Vertex* vertex, const V& destination, const bool allowMergeIncidentVertex) {
    assert(vertex != NULL);
    assert(vertex->position() != destination);
    assert(checkInvariant());
    
    // The idea of this algorithm can be summarized as follows:
    // First, break up all faces incident to the given vertex so that they become triangles.
    // Second, examine the (straight) path along which the given vertex will travel when moved to the
    // given location. Determine the closest point on this path where one of the incident triangles
    // becomes coplanar with one (or more) of its neighbours. This point is given by the value of
    // curFrac, which is between 0.0 and 1.0, 0.0 being the original vertex position and 1.0 the destination.
    // Third, determine whether the vertex will come to rest on another vertex. If that is the case, and
    // allowMergeIncidentVertices is true, then merge those vertices if possible and conclude the operation.
    // Otherwise, proceed with the fourth step: Move the vertex to the point we just computed, and merge the
    // incident faces with their neighbours.
    // Continue this until the vertex is either deleted during the merging phase, or until the vertex arrives
    // at its destination position.
    
    T lastFrac = 0.0;
    const V origin = vertex->position();
    while (!vertex->position().equals(destination, 0.0)) {
        splitIncidentFaces(vertex, destination);
        
        const T curFrac = computeNextMergePoint(vertex, origin, destination, lastFrac);
        if (curFrac < 0.0)
            return MoveVertexResult(MoveVertexResult::Type_VertexUnchanged, vertex);
        
        assert(curFrac > lastFrac);
        lastFrac = curFrac;
        
        const V newPosition = origin + lastFrac * (destination - origin);
        Vertex* occupant = findVertexByPosition(newPosition);
        if (occupant != NULL) {
            HalfEdge* connectingEdge = vertex->findConnectingEdge(occupant);
            if (!allowMergeIncidentVertex || connectingEdge == NULL) {
                mergeIncidentFaces(vertex);
                const MoveVertexResult result = moveVertex(vertex, origin, false);
                assert(result.moved());
                return MoveVertexResult(MoveVertexResult::Type_VertexUnchanged);
            }
            mergeVertices(connectingEdge);
        }
        
        vertex->setPosition(newPosition);
        vertex = cleanupAfterVertexMove(vertex);
        if (vertex == NULL)
            return MoveVertexResult(MoveVertexResult::Type_VertexDeleted);
    }
    
    return MoveVertexResult(MoveVertexResult::Type_VertexMoved, vertex);
}

// Splits the faces incident to the given vertex so that the polyhedron cannot become
// convex if the vertex is moved to the given destination. Depending on the orientation of
// an incident face and the vector from the given vertex's position to the destination,
// this includes chopping off triangles of some incident faces, and splitting other faces
// into triangle fans.
template <typename T>
void Polyhedron<T>::splitIncidentFaces(Vertex* vertex, const V& destination) {
    HalfEdge* firstEdge = vertex->leaving();
    HalfEdge* curEdge = firstEdge;
    
    do {
        HalfEdge* nextEdge = curEdge->nextIncident();
        Face* face = curEdge->face();
        if (face->vertexCount() > 3) {
            const Math::PointStatus::Type status = face->pointStatus(destination);
            if (status == Math::PointStatus::PSBelow)
                chopFace(face, curEdge);
            else
                splitFace(face, curEdge);
        }
        curEdge = nextEdge;
    } while (curEdge != firstEdge);
}

// Creates a new face by chopping off one triangle of the given face. The triangle
// will have the destination of the given edge, the origin of the given edge, and
// the origin of the given edge's predecessor as its vertices.
template <typename T>
void Polyhedron<T>::chopFace(Face* face, HalfEdge* halfEdge) {
    assert(face->vertexCount() > 3);
    
    HalfEdge* next = halfEdge;
    HalfEdge* previous = next->previous();
    
    HalfEdge* newEdge1 = new HalfEdge(previous->origin());
    HalfEdge* newEdge2 = new HalfEdge(next->destination());
    
    face->replaceBoundary(previous, next, newEdge1);
    
    assert(next->next() == previous);
    assert(previous->previous() == next);
    
    HalfEdgeList boundary;
    boundary.append(previous, 2);
    boundary.append(newEdge2, 1);
    
    m_faces.append(new Face(boundary), 1);
    m_edges.append(new Edge(newEdge1, newEdge2), 1);
}

/*
 Splits the given face into triangles by adding new edges from the origin of the given edge
 to every other non-adjacent vertex in the given face.
 ______     ______
 |    |     |   /|
 |    |     |  / |
 |    |     | /  |
 |    |     |/   |
 --h-->     --h-->
 
*/
template <typename T>
void Polyhedron<T>::splitFace(Face* face, HalfEdge* halfEdge) {
    HalfEdge* next = halfEdge;
    while (face->vertexCount() > 3) {
        HalfEdge* previous = next->previous();
        chopFace(face, previous);
    }
}

template <typename T>
T Polyhedron<T>::computeNextMergePoint(Vertex* vertex, const V& origin, const V& destination, T lastFrac) const {
    HalfEdge* firstEdge = vertex->leaving();
    HalfEdge* curEdge = firstEdge;
    
    T minFrac = 1.0;
    do {
        minFrac = std::min(minFrac, computeNextMergePointForIncidentNeighbour(curEdge, origin, destination, lastFrac));
        minFrac = std::min(minFrac, computeNextMergePointForOppositeNeighbour(curEdge, origin, destination, lastFrac));
        curEdge = curEdge->nextIncident();
    } while (curEdge != firstEdge);
    return minFrac;
}

template <typename T>
T Polyhedron<T>::computeNextMergePointForIncidentNeighbour(HalfEdge* edge, const V& origin, const V& destination, T lastFrac) const {
    /*
     We consider the plane made up by the points p1, p2 and p3 of side and next. If the movement
     of the vertex were to go through this plane, the brush would become convex, which we must prevent.
     
     v----p3
     |\ n |
     | \  |
     |  \ |
     | s \|
     p1---p2
     
     */
    
    HalfEdge* nextEdge = edge->nextIncident();
    HalfEdge* nextNextEdge = nextEdge->nextIncident();
    
    assert(edge->face()->vertexCount() == 3);
    assert(nextEdge->face()->vertexCount() == 3);
    
    Vertex* v1 = edge->destination();
    Vertex* v2 = nextEdge->destination();
    Vertex* v3 = nextNextEdge->destination();
    
    const V& p1 = v1->position();
    const V& p2 = v2->position();
    const V& p3 = v3->position();
    
    Plane<T,3> plane;
    if (!setPlanePoints(plane, p1, p2, p3)) {
        // The points are colinear and we cannot determine the move distance - this is an error!
        return -1.0;
    }
    
    return computeNextMergePointForPlane(origin, destination, plane, lastFrac);
}

template <typename T>
T Polyhedron<T>::computeNextMergePointForOppositeNeighbour(HalfEdge* edge, const V& origin, const V& destination, T lastFrac) const {
    /*
     We consider the boundary plane of the one neighbour to the side which is not incident to the
     moved vertex. This neighbour is not necessarily a triangle, but that does not matter.
     
             p3------
            /    n  |
           /     e  |
     v----p2     i  |
     |\ s |      g  |
     | \  |      h  |
     |  \ |      b  |
     |   \|      o  |
     -----p1     u  |
           \     r  |
            ---------
     */
    
    HalfEdge* myBorder = edge->next();
    HalfEdge* theirBorder = myBorder->twin();
    
    Vertex* v1 = edge->destination();
    Vertex* v2 = theirBorder->origin();
    Vertex* v3 = theirBorder->previous()->origin();
    
    const V& p1 = v1->position();
    const V& p2 = v2->position();
    const V& p3 = v3->position();
    
    Plane<T,3> plane;
    if (!setPlanePoints(plane, p1, p2, p3)) {
        // The points are colinear and we cannot determine the move distance - this is an error!
        return -1.0;
    }
    
    return computeNextMergePointForPlane(origin, destination, plane, lastFrac);
}

template <typename T>
T Polyhedron<T>::computeNextMergePointForPlane(const V& origin, const V& destination, const Plane<T,3>& plane, T lastFrac) const {
    const T origDot = origin.dot(plane.normal) - plane.distance;
    const T destDot = destination.dot(plane.normal) - plane.distance;
    
    // We only consider this a merge point if the origin does not lie (almost) within the plane.
    if (std::abs(origDot) >= 0.001) {
        // Does the vertex travel through the plane?
        if ((origDot > 0.0) != (destDot > 0.0)) {
            // The distance must grow because we use the original vertex origin.
            const T frac = std::abs(origDot) / (std::abs(origDot) + std::abs(destDot));
            if (frac > lastFrac)
                return frac;
        }
    }
    return 1.0;
}

// Merges the origin and destination vertex of the given edge into one vertex, thereby
// deleting the edge, the destination vertex, and the faces incident to the connecting edge
// and its twin.
// Assumes that these incident faces are triangles.
template <typename T>
void Polyhedron<T>::mergeVertices(HalfEdge* connectingEdge) {
    HalfEdge* oppositeEdge = connectingEdge->twin();
    
    Vertex* origin = connectingEdge->origin();
    Vertex* destination = oppositeEdge->origin();
    
    // First we merge the triangles that will become invalid by the merge to their neighbours.
    // We assume they are both triangles.
    assert(connectingEdge->face()->vertexCount() == 3);
    assert(oppositeEdge->face()->vertexCount() == 3);
    mergeNeighbours(connectingEdge->previous());
    mergeNeighbours(oppositeEdge->next());
    
    // Now we delete the destination of the connecting edge.
    // First we have to change the origin of all edges originating
    // at the destination to the origin of the connecting edge.
    // We also have to delete the connecting edge and its twin from the incident faces.
    
    destination->setLeaving(connectingEdge->next());
    
    HalfEdge* firstEdge = destination->leaving();
    HalfEdge* curEdge = firstEdge;
    do {
        HalfEdge* next = curEdge->nextIncident();
        curEdge->setOrigin(origin);
        curEdge = next;
    } while (curEdge != firstEdge);
    
    Face* leftFace = connectingEdge->face();
    leftFace->removeFromBoundary(connectingEdge);
    
    Face* rightFace = oppositeEdge->face();
    rightFace->removeFromBoundary(oppositeEdge);
    
    Edge* edge = connectingEdge->edge();
    m_edges.remove(edge);
    delete edge;
    
    delete connectingEdge;
    delete oppositeEdge;
    
    m_vertices.remove(destination);
    delete destination;
}

// Merges all leaving edges of the given vertex with their successors if they have become colinear.
// Also merges all faces incident to the given vertex with their coplanar neighbours.
// Finally merges all arriving edges of the given vertex with any colinear leaving edges.
// Returns the given vertex or NULL if it was deleted.
template <typename T>
typename Polyhedron<T>::Vertex* Polyhedron<T>::cleanupAfterVertexMove(Vertex* vertex) {
    mergeLeavingEdges(vertex);
    vertex = mergeIncidentFaces(vertex);
    if (vertex != NULL)
        vertex = mergeIncomingAndLeavingEdges(vertex);
    return vertex;
}

// Merges all leaving edges of the given vertex with their successors if they have become colinear.
template <typename T>
void Polyhedron<T>::mergeLeavingEdges(Vertex* vertex) {
    HalfEdge* curEdge = vertex->leaving();
    do {
        Vertex* destination = curEdge->destination();
        HalfEdge* colinearEdge = destination->findColinearEdge(curEdge);
        if (colinearEdge != NULL) {
            mergeNeighboursOfColinearEdges(curEdge, colinearEdge);
            mergeColinearEdges(curEdge, colinearEdge);
        }
        curEdge = curEdge->nextIncident();
    } while (curEdge != vertex->leaving());
}

// Merges the neighbours of the given successive colinear edges with their coplanar neighbours.
template <typename T>
void Polyhedron<T>::mergeNeighboursOfColinearEdges(HalfEdge* edge1, HalfEdge* edge2) {
    assert(edge1->destination() == edge2->origin());
    
    if (edge1->next() == edge2) // the left side is a degenerate triangle now
        mergeNeighbours(edge1->previous());
    else
        mergeNeighbours(edge1->next());
    if (edge1->twin()->previous() == edge2->twin())
        mergeNeighbours(edge1->twin()->next());
    else
        mergeNeighbours(edge1->twin()->previous());
}

// Merges all arriving edges of the given vertex with any colinear leaving edges.
// Returns the given vertex or NULL if it was deleted.
template <typename T>
typename Polyhedron<T>::Vertex* Polyhedron<T>::mergeIncomingAndLeavingEdges(Vertex* vertex) {
    HalfEdge* arriving = vertex->leaving()->twin();
    do {
        HalfEdge* colinearLeaving = vertex->findColinearEdge(arriving);
        if (colinearLeaving != NULL) {
            mergeColinearEdges(arriving, colinearLeaving);
            return NULL;
        }
        arriving = arriving->next()->twin();
    } while (arriving != vertex->leaving()->twin());
    return vertex;
}

// Merges the given successive colinear edges. As a result, the origin of the second
// given edge, the second given edge itself and its twin are removed. At the same time,
// the first given edge replaces the second given edge, and the first given edge's twin
// replaces the twin of the second given edge.
// Assumes that the incident faces have already been merged.
template <typename T>
void Polyhedron<T>::mergeColinearEdges(HalfEdge* edge1, HalfEdge* edge2) {
    assert(edge1->destination() == edge2->origin());
    assert(edge1->face() == edge2->face());
    assert(edge1->twin()->face() == edge2->twin()->face());
    
    Vertex* vertex = edge2->origin();
    Vertex* succ = edge2->destination();
    
    edge1->twin()->setOrigin(succ);
    succ->setLeaving(edge1->twin());
    
    HalfEdge* toRemove1 = edge2;
    HalfEdge* toRemove2 = toRemove1->twin();
    
    Face* left = toRemove1->face();
    left->removeFromBoundary(toRemove1);
    
    Face* right = toRemove2->face();
    right->removeFromBoundary(toRemove2);
    
    Edge* edge = toRemove1->edge();
    m_edges.remove(edge);
    delete edge;
    
    delete toRemove1;
    delete toRemove2;
    
    m_vertices.remove(vertex);
    delete vertex;
}

// Merges all faces incident to the given vertex with their coplanar neighbours.
// Returns the given vertex or NULL if the given vertex was deleted.
template <typename T>
typename Polyhedron<T>::Vertex* Polyhedron<T>::mergeIncidentFaces(Vertex* vertex) {
    size_t incidentFaceCount = 0;
    bool allCoplanar = true;
    
    // First determine whether all incident faces have become coplanar. While doing this,
    // also remember an edge such that its incident face and its inner neighbour are not
    // coplanar.
    HalfEdge* firstEdge = vertex->leaving();
    HalfEdge* curEdge = firstEdge;
    do {
        Face* face = curEdge->face();
        HalfEdge* innerBorder = curEdge->previous();
        Face* innerNeighbour = innerBorder->twin()->face();
        
        if (!face->coplanar(innerNeighbour)) {
            allCoplanar = false;
            firstEdge = curEdge;
        }
        curEdge = innerBorder->twin();
        ++incidentFaceCount;
    } while (allCoplanar && curEdge != firstEdge);
    
    if (allCoplanar) {
        HalfEdgeList boundary;
        
        // Now we iterate using the incident face count because we can't rely
        // on the curEdge's twin while we're deleting the edges we encounter.
        curEdge = firstEdge;
        for (size_t i = 0; i < incidentFaceCount; ++i) {
            Face* face = curEdge->face();
            Edge* edge = curEdge->edge();
            
            HalfEdge* twin = curEdge->twin();
            HalfEdge* outerBorder = curEdge->next();
            
            // Don't increment past the last edge because its twin will already have been deleted.
            if (i < incidentFaceCount - 1)
                curEdge = curEdge->nextIncident();
            
            twin->origin()->setLeaving(outerBorder);
            
            face->removeFromBoundary(outerBorder);
            boundary.append(outerBorder, 1);
            
            m_faces.remove(face);
            m_edges.remove(edge);
            
            delete face;
            delete edge;
        }
        
        Face* face = new Face(boundary);
        m_faces.append(face, 1);
        
        m_vertices.remove(vertex);
        delete vertex;
        vertex = NULL;
    } else {
        // Due to how we have chosen the first edge, we know that the first two
        // incident faces are not coplanar and thus won't be merged. This is important
        // because otherwise, the loop would immediately terminate after one iteration
        // since curEdge is not incremented when two inner neighbours are merged.
        curEdge = firstEdge;
        do {
            Face* face = curEdge->face();
            
            HalfEdge* outerBorder = curEdge->next();
            Face* outerNeighbour = outerBorder->twin()->face();
            
            HalfEdge* innerBorder = curEdge->previous();
            Face* innerNeighbour = innerBorder->twin()->face();
            
            if (face->coplanar(outerNeighbour)) {
                mergeNeighbours(outerBorder);
                curEdge = curEdge->nextIncident();
            } else if (face->coplanar(innerNeighbour)) {
                // Ensure that we don't remove the first edge, otherwise we'll loop endlessly.
                if (innerBorder->twin() == firstEdge)
                    firstEdge = firstEdge->nextIncident();
                mergeNeighbours(innerBorder);
            } else {
                curEdge = curEdge->nextIncident();
            }
        } while (curEdge != firstEdge);
    }
    return vertex;
}

// Merges the face incident to the given edge (called border) with its neighbour, i.e.
// the face incident to the given face's twin. The face incident to the border is deleted
// while the neighbour consumes the boundary of the incident face.
// Also handles the case where the border is longer than just one edge, but we assume that
// the given edge is the first edge of the border.
template <typename T>
void Polyhedron<T>::mergeNeighbours(HalfEdge* borderFirst) {
    HalfEdge* twinLast = borderFirst->twin();
    
    Face* face = borderFirst->face();
    Face* neighbour = twinLast->face();
    
    HalfEdge* borderLast = borderFirst;
    while (borderLast->next()->face() == face &&
           borderLast->next()->twin()->face() == neighbour) {
        borderLast = borderLast->next();
    }
    
    HalfEdge* twinFirst = borderLast->twin();
    
    borderFirst->origin()->setLeaving(twinLast->next());
    twinFirst->origin()->setLeaving(borderLast->next());
    
    HalfEdge* remainingFirst = borderLast->next();
    HalfEdge* remainingLast = borderFirst->previous();
    
    face->removeFromBoundary(borderFirst, borderLast);
    face->removeFromBoundary(remainingFirst, remainingLast);
    
    neighbour->replaceBoundary(twinFirst, twinLast, remainingFirst);
    
    HalfEdge* cur = borderFirst;
    do {
        Edge* edge = cur->edge();
        HalfEdge* next = cur->next();
        HalfEdge* twin = cur->twin();
        
        if (cur != borderFirst) {
            Vertex* origin = cur->origin();
            m_vertices.remove(origin);
            delete origin;
        }
        
        m_edges.remove(edge);
        delete edge;
        
        delete cur;
        delete twin;
        cur = next;
    } while (cur != borderFirst);
    
    m_faces.remove(face);
    delete face;
}

template <typename T>
void Polyhedron<T>::addPoint(const V& position) {
    assert(checkInvariant());
    switch (vertexCount()) {
        case 0:
            addFirstPoint(position);
            break;
        case 1:
            addSecondPoint(position);
            break;
        case 2:
            addThirdPoint(position);
            break;
        default:
            addFurtherPoint(position);
            break;
    }
}

// Adds the given point to an empty polyhedron.
template <typename T>
void Polyhedron<T>::addFirstPoint(const V& position) {
    assert(empty());
    m_vertices.append(new Vertex(position), 1);
}

// Adds the given point to a polyhedron that contains one point.
template <typename T>
void Polyhedron<T>::addSecondPoint(const V& position) {
    assert(point());
    
    Vertex* onlyVertex = m_vertices.iterator().next();
    if (position != onlyVertex->position()) {
        Vertex* newVertex = new Vertex(position);
        m_vertices.append(newVertex, 1);
        
        HalfEdge* halfEdge1 = new HalfEdge(onlyVertex);
        HalfEdge* halfEdge2 = new HalfEdge(newVertex);
        Edge* edge = new Edge(halfEdge1, halfEdge2);
        m_edges.append(edge, 1);
    }
}

// Adds the given point to a polyhedron that contains one edge.
template <typename T>
void Polyhedron<T>::addThirdPoint(const V& position) {
    assert(edge());
    
    typename VertexList::Iterator it = m_vertices.iterator();
    const Vertex* v1 = it.next();
    const Vertex* v2 = it.next();
    
    if (linearlyDependent(v1->position(), v2->position(), position))
        addPointToEdge(position);
    else
        addPointToPolygon(position);
}

// Adds a colinear third point to a polyhedron that contains one edge.
template <typename T>
void Polyhedron<T>::addPointToEdge(const V& position) {
    assert(edge());
    
    typename VertexList::Iterator it = m_vertices.iterator();
    Vertex* v1 = it.next();
    Vertex* v2 = it.next();
    assert(linearlyDependent(v1->position(), v2->position(), position));
    
    if (!position.containedWithinSegment(v1->position(), v2->position()))
        v2->setPosition(position);
}

// Adds the given point to a polyhedron that is either a polygon or a polyhedron.
template <typename T>
void Polyhedron<T>::addFurtherPoint(const V& position) {
    if (faceCount() == 1)
        addFurtherPointToPolygon(position);
    else
        addFurtherPointToPolyhedron(position);
}

//Adds the given point to a polygon. The result is either a differen polygon if the
// given point is coplanar to the already existing polygon, or a polyhedron if the
// given point is not coplanar.
template <typename T>
void Polyhedron<T>::addFurtherPointToPolygon(const V& position) {
    Face* face = m_faces.iterator().next();
    const Math::PointStatus::Type status = face->pointStatus(position);
    switch (status) {
        case Math::PointStatus::PSInside:
            addPointToPolygon(position);
            break;
        case Math::PointStatus::PSAbove:
            face->flip();
        case Math::PointStatus::PSBelow:
            makePolyhedron(position);
            break;
    }
}

// Adds the given coplanar point to a polyhedron that is a polygon or an edge.
template <typename T>
void Polyhedron<T>::addPointToPolygon(const V& position) {
    typename V::List positions;
    positions.reserve(vertexCount() + 1);
    vertexPositions(positions);
    positions.push_back(position);
    
    positions = convexHull2D<T>(positions);
    clear();
    makePolygon(positions);
}

// Creates a new polygon from the given set of coplanar points. Assumes that
// this polyhedron is empty and that the given point list contains at least three
// non-colinear points.
template <typename T>
void Polyhedron<T>::makePolygon(const typename V::List& positions) {
    assert(empty());
    assert(positions.size() > 2);
    
    HalfEdgeList boundary;
    for (size_t i = 0; i < positions.size(); ++i) {
        const V& p = positions[i];
        Vertex* v = new Vertex(p);
        HalfEdge* h = new HalfEdge(v);
        Edge* e = new Edge(h);
        
        m_vertices.append(v, 1);
        boundary.append(h, 1);
        m_edges.append(e, 1);
    }
    
    Face* f = new Face(boundary);
    m_faces.append(f, 1);
}

// Converts a coplanar polyhedron into a non-coplanar one by adding the given
// point, which is assumed to be non-coplanar to the points in this polyhedron.
template <typename T>
void Polyhedron<T>::makePolyhedron(const V& position) {
    assert(polygon());
    
    Seam seam;
    Face* face = m_faces.iterator().next();
    const HalfEdgeList& boundary = face->boundary();
    typename HalfEdgeList::ConstIterator it = boundary.iterator();
    while (it.hasNext()) {
        const HalfEdge* h = it.next();
        Edge* e = h->edge();
        seam.push_front(e); // ensure that the seam is in CCW order
    }
    
    addPointToPolyhedron(position, seam);
}

// Adds the given point to this polyhedron.
template <typename T>
void Polyhedron<T>::addFurtherPointToPolyhedron(const V& position) {
    assert(polyhedron());
    const Seam seam = split(SplitByVisibilityCriterion(position));
    if (!seam.empty())
        addPointToPolyhedron(position, seam);
}

// Adds the given point to this polyhedron by weaving a cap over the given seam.
// Assumes that this polyhedron has been split by the given seam.
template <typename T>
void Polyhedron<T>::addPointToPolyhedron(const V& position, const Seam& seam) {
    assert(!seam.empty());
    weaveCap(seam, position);
    mergeCoplanarFaces(seam);
    assert(checkInvariant());
}

// Splits this polyhedron along the edges where one of the adjacent faces matches the given
// criterion and the other adjacent face does not. It deletes all faces, edges, and vertices
// which do not match the given criterion, and returns the delimiting edges as the seam.
// The delimiting edges all have the remaining half edge as their first edge, and the second
// edge, which has been deleted by the split, is NULL.
template <typename T>
typename Polyhedron<T>::Seam Polyhedron<T>::split(const SplittingCriterion& criterion) {
    VertexList vertices;
    EdgeList edges;
    FaceList faces;
    Seam seam;
    
    Edge* splittingEdge = criterion.findFirstSplittingEdge(m_edges);
    if (splittingEdge == NULL)
        return Seam(0);
    
    // First, go along the splitting seam and split the edges into two edges with one half edge each.
    // Both the resulting edges have their only half edge as their first edge.
    do {
        assert(splittingEdge != NULL);
        Edge* nextSplittingEdge = criterion.findNextSplittingEdge(splittingEdge);
        assert(nextSplittingEdge != splittingEdge);
        assert(nextSplittingEdge == NULL || splittingEdge->firstVertex() == nextSplittingEdge->secondVertex());
        
        splittingEdge->unsetSecondEdge();
        seam.push_back(splittingEdge);
        
        splittingEdge = nextSplittingEdge;
    } while (splittingEdge != NULL);
    
    typename VertexList::Iterator vertexIt;
    typename EdgeList::Iterator edgeIt;
    typename FaceList::Iterator faceIt;
    
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
            vertices.append(vertex, 1);
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
            edges.append(edge, 1);
        }
    }
    
    faceIt = m_faces.iterator();
    while (faceIt.hasNext()) {
        Face* face = faceIt.next();
        
        if (!criterion.matches(face)) {
            faceIt.remove();
            faces.append(face, 1);
        }
    }
    
    faces.deleteAll();
    edges.deleteAll();
    vertices.deleteAll();
    
    assert(isConvex());
    
    return seam;
}

// Weaves a new cap onto the given seam edges. The vertices of the seam are assumed to be
// coplanar, because the new cap is just one face.
template <typename T>
void Polyhedron<T>::weaveCap(const Seam& seam) {
    assert(seam.size() >= 3);
    
    HalfEdgeList halfEdges;
    for (size_t i = 0; i < seam.size(); ++i) {
        Edge* edge = seam[i];
        assert(!edge->fullySpecified());
        
        HalfEdge* halfEdge = new HalfEdge(edge->secondVertex());
        edge->setSecondEdge(halfEdge);
        halfEdges.append(halfEdge, 1);
    }
    
    Face* face = new Face(halfEdges);
    m_faces.append(face, 1);
    
    assert(isConvex());
}

// Weaves a new cap onto the given seam edges. The new cap will form a triangle fan (actually a cone) with a new vertex
// at the location of the given point being shared by all the newly created triangles.
template <typename T>
void Polyhedron<T>::weaveCap(const Seam& seam, const V& position) {
    assert(seam.size() >= 3);
    
    Vertex* top = new Vertex(position);
    
    HalfEdge* first = NULL;
    HalfEdge* last = NULL;
    for (size_t i = 0; i < seam.size(); ++i) {
        Edge* edge = seam[i];
        assert(!edge->fullySpecified());
        
        Vertex* v1 = edge->secondVertex();
        Vertex* v2 = edge->firstVertex();
        
        HalfEdge* h1 = new HalfEdge(top);
        HalfEdge* h2 = new HalfEdge(v1);
        HalfEdge* h3 = new HalfEdge(v2);
        
        m_faces.append(createTriangle(h1, h2, h3), 1);
        
        if (last != NULL)
            m_edges.append(new Edge(h1, last), 1);
        edge->setSecondEdge(h2);
        
        if (first == NULL)
            first = h1;
        last = h3;
    }
    
    m_edges.append(new Edge(first, last), 1);
    m_vertices.append(top, 1);
    
    assert(isConvex());
}

// Merges all coplanar incident faces of the given seam edges.
template <typename T>
void Polyhedron<T>::mergeCoplanarFaces(const Seam& seam) {
    std::queue<Edge*> queue(seam);
    while (!queue.empty()) {
        Edge* first = queue.front(); queue.pop();
        assert(first->fullySpecified());
        
        const V firstNorm = first->firstFace()->normal();
        if (firstNorm.equals(first->secondFace()->normal())) {
            // fast forward through all seam edges which have both of their incident faces coplanar to the first face
            if (!queue.empty()) {
                Edge* cur = queue.front();
                while (cur != NULL && cur->firstFace()->normal().equals(firstNorm)) {
                    assert(cur->fullySpecified());
                    assert(cur->firstFace()->normal().equals(firstNorm) == cur->secondFace()->normal().equals(firstNorm));
                    queue.pop();
                    cur = !queue.empty() ? queue.front() : NULL;
                }
            }
            
            // now remove all coplanar faces and replace them with a new cap
            const Seam mergeSeam = split(SplitByNormalCriterion(firstNorm));
            weaveCap(mergeSeam);
        }
    }
    
    assert(isConvex());
}

template <typename T>
void Polyhedron<T>::initializeTetrahedron(const V& p1, const V& p2, const V& p3, const V& p4) {
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
    
    m_vertices.append(v1, 1);
    m_vertices.append(v2, 1);
    m_vertices.append(v3, 1);
    m_vertices.append(v4, 1);
    
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
    
    m_edges.append(e1, 1);
    m_edges.append(e2, 1);
    m_edges.append(e3, 1);
    m_edges.append(e4, 1);
    m_edges.append(e5, 1);
    m_edges.append(e6, 1);
    
    Face* f1 = createTriangle(h1, h2, h3);
    Face* f2 = createTriangle(h4, h5, h6);
    Face* f3 = createTriangle(h7, h8, h9);
    Face* f4 = createTriangle(h10, h11, h12);
    
    m_faces.append(f1, 1);
    m_faces.append(f2, 1);
    m_faces.append(f3, 1);
    m_faces.append(f4, 1);
    
    assert(checkInvariant());
}

template <typename T>
bool Polyhedron<T>::chooseNonColinearPoints(typename V::List& positions) const {
    using std::swap;
    
    // first, choose a third point that is not colinear to the first two
    size_t index = 2;
    while (index < positions.size() && V::colinear(positions[0], positions[1], positions[index]))
        ++index;
    if (index == positions.size())
        return false;
    if (index != 2)
        swap(positions[2], positions[index]);
    
    // now choose a fourth point such that it doesn't lie on the plane defined by the first three
    index = 3;
    while (index < positions.size() && commonPlane(positions[0], positions[1], positions[2], positions[index]))
        ++index;
    if (index == positions.size())
        return false;
    if (index != 3)
        swap(positions[3], positions[index]);
    return true;
}

template <typename T>
typename Polyhedron<T>::Vertex* Polyhedron<T>::findVertexByPosition(const V& position, const T epsilon) const {
    typename VertexList::ConstIterator it = m_vertices.iterator();
    while (it.hasNext()) {
        Vertex* vertex = it.next();
        if (position.equals(vertex->position(), epsilon))
            return vertex;
    }
    return NULL;
}

template <typename T>
typename Polyhedron<T>::Face* Polyhedron<T>::createTriangle(HalfEdge* h1, HalfEdge* h2, HalfEdge* h3) const {
    HalfEdgeList boundary;
    boundary.append(h1, 1);
    boundary.append(h2, 1);
    boundary.append(h3, 1);
    
    return new Face(boundary);
}

template <typename T>
void Polyhedron<T>::clear() {
    m_faces.deleteAll();
    m_edges.deleteAll();
    m_vertices.deleteAll();
}

template <typename T>
bool Polyhedron<T>::checkInvariant() const {
    if (!isConvex())
        return false;
    return true;
}

template <typename T>
bool Polyhedron<T>::isConvex() const {
    typename FaceList::ConstIterator fIt = m_faces.iterator();
    while (fIt.hasNext()) {
        const Face* face = fIt.next();
        typename VertexList::ConstIterator vIt = m_vertices.iterator();
        while (vIt.hasNext()) {
            const Vertex* vertex = vIt.next();
            if (face->pointStatus(vertex->position()) == Math::PointStatus::PSAbove)
                return false;
        }
    }
    return true;
}



template <typename T>
class Polyhedron<T>::SplittingCriterion {
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
    
    // finds the next seam edge in counter clockwise orientation
    Edge* findNextSplittingEdge(Edge* last) const {
        assert(last != NULL);
        
        HalfEdge* halfEdge = last->firstEdge()->previous();
        Edge* next = halfEdge->edge();
        if (!next->fullySpecified())
            return NULL;
        
        MatchResult result = matches(next);
        while (result != MatchResult_First && result != MatchResult_Second && next != last) {
            halfEdge = halfEdge->twin()->previous();
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

template <typename T>
class Polyhedron<T>::SplitByVisibilityCriterion : public Polyhedron<T>::SplittingCriterion {
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

template <typename T>
class Polyhedron<T>::SplitByNormalCriterion : public SplittingCriterion {
private:
    V m_normal;
public:
    SplitByNormalCriterion(const V& normal) :
    m_normal(normal) {}
private:
    bool doMatches(const Face* face) const {
        return !face->normal().equals(m_normal);
    }
};



template <typename T>
typename Polyhedron<T>::VertexLink& Polyhedron<T>::VertexList::doGetLink(Vertex* vertex) const {
    return vertex->m_link;
}

template <typename T>
const typename Polyhedron<T>::VertexLink& Polyhedron<T>::VertexList::doGetLink(const Vertex* vertex) const {
    return vertex->m_link;
}

template <typename T>
typename Polyhedron<T>::EdgeLink& Polyhedron<T>::EdgeList::doGetLink(Edge* edge) const {
    return edge->m_link;
}

template <typename T>
const typename Polyhedron<T>::EdgeLink& Polyhedron<T>::EdgeList::doGetLink(const Edge* edge) const {
    return edge->m_link;
}

template <typename T>
typename Polyhedron<T>::HalfEdgeLink& Polyhedron<T>::HalfEdgeList::doGetLink(HalfEdge* edge) const {
    return edge->m_link;
}

template <typename T>
const typename Polyhedron<T>::HalfEdgeLink& Polyhedron<T>::HalfEdgeList::doGetLink(const HalfEdge* edge) const {
    return edge->m_link;
}

template <typename T>
typename Polyhedron<T>::FaceLink& Polyhedron<T>::FaceList::doGetLink(Face* face) const {
    return face->m_link;
}

template <typename T>
const typename Polyhedron<T>::FaceLink& Polyhedron<T>::FaceList::doGetLink(const Face* face) const {
    return face->m_link;
}

template <typename T>
class Polyhedron<T>::Vertex {
private:
    friend class HalfEdge;
    friend class VertexList;
    friend class Polyhedron<T>;
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
private:
    HalfEdge* leaving() const {
        return m_leaving;
    }
    
    HalfEdge* findConnectingEdge(const Vertex* vertex) const {
        assert(vertex != NULL);
        assert(m_leaving != NULL);
        
        HalfEdge* curEdge = m_leaving;
        do {
            if (vertex == curEdge->destination())
                return curEdge;
            curEdge = curEdge->nextIncident();
        } while (curEdge != m_leaving);
        return NULL;
    }
    
    HalfEdge* findColinearEdge(const HalfEdge* arriving) const {
        assert(arriving != NULL);
        assert(m_leaving != NULL);
        assert(arriving->destination() == this);
        
        HalfEdge* curEdge = m_leaving;
        do {
            if (arriving->colinear(curEdge))
                return curEdge;
            curEdge = curEdge->nextIncident();
        } while (curEdge != m_leaving);
        return NULL;
    }

    void setPosition(const V& position) {
        m_position = position;
    }
    
    void setLeaving(HalfEdge* edge) {
        assert(edge == NULL || edge->origin() == this);
        m_leaving = edge;
    }
};

template <typename T>
class Polyhedron<T>::Edge {
private:
    friend class EdgeList;
    friend class Polyhedron<T>;
private:
    HalfEdge* m_first;
    HalfEdge* m_second;
    EdgeLink m_link;
private:
    Edge(HalfEdge* first, HalfEdge* second) :
    m_first(first),
    m_second(second),
    m_link(this) {
        assert(m_first != NULL);
        assert(m_second != NULL);
        m_first->setEdge(this);
        m_second->setEdge(this);
    }

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
    
    bool contains(const V& point, const T maxDistance = Math::Constants<T>::almostZero()) const {
        return point.distanceToSegment(firstVertex()->position(), secondVertex()->position() < maxDistance);
    }
private:
    void flip() {
        using std::swap;
        swap(m_first, m_second);
    }
    
    void unsetSecondEdge() {
        m_first->setAsLeaving();
        m_second->setEdge(NULL);
        m_second = NULL;
    }
    
    void setSecondEdge(HalfEdge* second) {
        assert(second != NULL);
        assert(m_second == NULL);
        assert(second->edge() == NULL);
        m_second = second;
        m_second->setEdge(this);
    }
};

template <typename T>
class Polyhedron<T>::HalfEdge {
private:
    friend class Edge;
    friend class HalfEdgeList;
    friend class Face;
    friend class Polyhedron<T>;
private:
    Vertex* m_origin;
    Edge* m_edge;
    Face* m_face;
    HalfEdgeLink m_link;
private:
    HalfEdge(Vertex* origin) :
    m_origin(origin),
    m_edge(NULL),
    m_face(NULL),
    m_link(this) {
        assert(m_origin != NULL);
        setAsLeaving();
    }
public:
    ~HalfEdge() {
        if (m_origin->leaving() == this)
            m_origin->setLeaving(NULL);
    }
    
    Vertex* origin() const {
        return m_origin;
    }
    
    Vertex* destination() const {
        return next()->origin();
    }
    
    T length() const {
        return vector().length();
    }
    
    T squaredLength() const {
        return vector().squaredLength();
    }
    
    V vector() const {
        return destination()->position() - origin()->position();
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
    
    HalfEdge* previousIncident() const {
        return twin()->next();
    }
    
    HalfEdge* nextIncident() const {
        return previous()->twin();
    }
private:
    bool isLeavingEdge() const {
        return m_origin->leaving() == this;
    }

    bool colinear(const HalfEdge* other) const {
        const V dir = vector().normalized();
        const V otherDir = other->vector().normalized();
        return Math::eq(otherDir.dot(dir), 1.0);
    }

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

template <typename T>
class Polyhedron<T>::Face {
private:
    friend class FaceList;
    friend class Polyhedron<T>;
private:
    HalfEdgeList m_boundary;
    FaceLink m_link;
private:
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
public:
    ~Face() {
        m_boundary.deleteAll();
    }
    
    size_t vertexCount() const {
        return m_boundary.size();
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
private:
    bool visibleFrom(const V& point) const {
        return pointStatus(point) == Math::PointStatus::PSAbove;
    }
    
    bool coplanar(const Face* other) const {
        assert(other != NULL);
        return normal().equals(other->normal(), Math::Constants<T>::colinearEpsilon());
    }
    
    bool isDegenerateTriangle(const T epsilon = Math::Constants<T>::almostZero()) const {
        if (vertexCount() != 3)
            return false;
        
        typename HalfEdgeList::ConstIterator it = m_boundary.iterator();
        const HalfEdge* edge = it.next();
        const T l1 = edge->squaredLength();
        
        edge = it.next();
        const T l2 = edge->squaredLength();
        
        edge = it.next();
        const T l3 = edge->squaredLength();
        
        const T epsilon2 = epsilon * epsilon;
        if (l1 > l2) {
            if (l1 > l3)
                return Math::eq(l1, l2 + l3, epsilon2);
        } else {
            if (l2 > l3)
                return Math::eq(l2, l1 + l3, epsilon2);
        }
        return Math::eq(l3, l1 + l2, epsilon2);
    }

    Math::PointStatus::Type pointStatus(const V& point, const T epsilon = Math::Constants<T>::pointStatusEpsilon()) const {
        const V norm = normal();
        const T distance = (point - origin()).dot(norm);
        if (distance > epsilon)
            return Math::PointStatus::PSAbove;
        if (distance < -epsilon)
            return Math::PointStatus::PSBelow;
        return Math::PointStatus::PSInside;
    }
    
    void flip() {
        m_boundary.reverse();
    }
    
    void removeFromBoundary(HalfEdge* edge) {
        removeFromBoundary(edge, edge);
    }
    
    void removeFromBoundary(HalfEdge* from, HalfEdge* to) {
        assert(from != NULL);
        assert(to != NULL);
        assert(from->face() == this);
        assert(to->face() == this);
        
        const size_t removeCount = countAndSetFace(from, to->next(), NULL);
        m_boundary.remove(from, to, removeCount);
    }
    
    void replaceBoundary(HalfEdge* edge, HalfEdge* with) {
        replaceBoundary(edge, edge, with);
    }
    
    void replaceBoundary(HalfEdge* from, HalfEdge* to, HalfEdge* with) {
        assert(from != NULL);
        assert(to != NULL);
        assert(with != NULL);
        assert(from->face() == this);
        assert(to->face() == this);
        assert(with->face() == NULL);
        
        const size_t removeCount = countAndSetFace(from, to->next(), NULL);
        const size_t insertCount = countAndSetFace(with, with, this);
        m_boundary.replace(from, to, removeCount, with, insertCount);
    }
    
    size_t countAndSetFace(HalfEdge* from, HalfEdge* until, Face* face) {
        size_t count = 0;
        HalfEdge* cur = from;
        do {
            cur->setFace(face);
            cur = cur->next();
            ++count;
        } while (cur != until);
        return count;
    }
};

#endif
