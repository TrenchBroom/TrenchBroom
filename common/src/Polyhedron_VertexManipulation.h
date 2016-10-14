/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TrenchBroom_Polyhedron_VertexManipulation_h
#define TrenchBroom_Polyhedron_VertexManipulation_h

#include <algorithm>

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::MoveVerticesResult::MoveVerticesResult() {}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::MoveVerticesResult::MoveVerticesResult(const typename V::List& i_movedVertices) :
movedVertices(i_movedVertices) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::MoveVerticesResult::add(const MoveVertexResult& result) {
    switch (result.type) {
        case MoveVertexResult::Type_VertexMoved:
            movedVertices.push_back(result.originalPosition);
            newVertexPositions.push_back(result.vertex->position());
            break;
        case MoveVertexResult::Type_VertexDeleted:
            deletedVertices.push_back(result.originalPosition);
            break;
        case MoveVertexResult::Type_VertexUnchanged:
            unchangedVertices.push_back(result.originalPosition);
            break;
        switchDefault()
    }
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::MoveVerticesResult::addUnknown(const V& position) {
    unknownVertices.push_back(position);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::MoveVerticesResult::allVerticesMoved() const {
    return !hasDeletedVertices() && !hasUnchangedVertices() && unknownVertices.empty() && !newVertexPositions.empty();
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::MoveVerticesResult::hasDeletedVertices() const {
    return !deletedVertices.empty();
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::MoveVerticesResult::hasUnchangedVertices() const {
    return !unchangedVertices.empty();
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::MoveVerticesResult::hasUnknownVertices() const {
    return !unknownVertices.empty();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::MoveVerticesResult Polyhedron<T,FP,VP>::moveVertices(const typename V::List& positions, const V& delta, bool allowMergeIncidentVertices) {
    Callback c;
    return moveVertices(positions, delta, allowMergeIncidentVertices, c);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::MoveVerticesResult Polyhedron<T,FP,VP>::moveVertices(typename V::List positions, const V& delta, bool allowMergeIncidentVertices, Callback& callback) {
    assert(checkInvariant());
    if (delta.null())
        return MoveVerticesResult(positions);
    const MoveVerticesResult result = doMoveVertices(positions, delta, allowMergeIncidentVertices, callback);
    assert(checkInvariant());
    return result;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::MoveVerticesResult Polyhedron<T,FP,VP>::splitEdge(const V& v1, const V& v2, const V& delta) {
    Callback c;
    return splitEdge(v1, v2, delta, c);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::MoveVerticesResult Polyhedron<T,FP,VP>::splitEdge(const V& v1, const V& v2, const V& delta, Callback& callback) {
    assert(checkInvariant());
    
    if (delta.null())
        return MoveVerticesResult();
    
    const SplitResult splitResult = splitEdge(v1, v2, callback);
    if (!splitResult.success)
        return MoveVerticesResult();
    
    const MoveVerticesResult moveResult = doMoveVertices(typename V::List(1, splitResult.vertexPosition), delta, false, callback);
    assert(checkInvariant());
    return moveResult;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::MoveVerticesResult Polyhedron<T,FP,VP>::splitFace(const typename V::List& vertexPositions, const V& delta) {
    Callback c;
    return splitFace(vertexPositions, delta, c);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::MoveVerticesResult Polyhedron<T,FP,VP>::splitFace(const typename V::List& vertexPositions, const V& delta, Callback& callback) {
    assert(checkInvariant());
    
    if (delta.null())
        return MoveVerticesResult();
    
    const SplitResult splitResult = splitFace(vertexPositions, callback);
    if (!splitResult.success)
        return MoveVerticesResult();
    
    const MoveVerticesResult moveResult = doMoveVertices(typename V::List(1, splitResult.vertexPosition), delta, false, callback);
    assert(checkInvariant());
    return moveResult;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::MoveVerticesResult Polyhedron<T,FP,VP>::doMoveVertices(typename V::List positions, const V& delta, const bool allowMergeIncidentVertices, Callback& callback) {
    std::sort(positions.begin(), positions.end(), typename V::InverseDotOrder(delta));
    MoveVerticesResult totalResult;
    
    for (size_t i = 0; i < positions.size(); ++i) {
        // Possibly be more precise here?
        Vertex* vertex = findVertexByPosition(positions[i]);
        if (vertex == NULL) {
            totalResult.addUnknown(positions[i]);
        } else {
            const V destination = vertex->position() + delta;
            const MoveVertexResult currentResult = moveVertex(vertex, destination, allowMergeIncidentVertices, callback);
            totalResult.add(currentResult);
        }
    }
    
    updateBounds();
    return totalResult;
}

template <typename T, typename FP, typename VP>
struct Polyhedron<T,FP,VP>::SplitResult {
    bool success;
    V vertexPosition;
    
    SplitResult(bool i_success, const V& i_vertexPosition = V::Null) :
    success(i_success),
    vertexPosition(i_vertexPosition) {}
};

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::SplitResult Polyhedron<T,FP,VP>::splitEdge(const V& v1, const V& v2, Callback& callback) {
    Edge* edge = findEdgeByPositions(v1, v2);
    if (edge == NULL)
        return SplitResult(false);
    
    Edge* newEdge = edge->splitAtCenter();
    m_edges.append(newEdge, 1);
    
    Vertex* newVertex = newEdge->firstVertex();
    m_vertices.append(newVertex, 1);
    callback.vertexWasCreated(newVertex);
    
    return SplitResult(true, newVertex->position());
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::SplitResult Polyhedron<T,FP,VP>::splitFace(const typename V::List& vertexPositions, Callback& callback) {
    Face* face = findFaceByPositions(vertexPositions);
    if (face == NULL)
        return SplitResult(false);
    
    Vertex* newVertex = new Vertex(face->center());
    m_vertices.append(newVertex, 1);
    callback.vertexWasCreated(newVertex);
    
    const size_t vertexCount = face->vertexCount();
    HalfEdge* current = face->boundary().front();
    
    // First, create a new triangle that cuts into the face. The face will be convex until the following loop finishes.
    {
        HalfEdge* next = current->next();
        
        HalfEdge* fromCenter = new HalfEdge(newVertex);
        HalfEdge* fromCenterTwin = new HalfEdge(current->origin());
        HalfEdge* toCenter = new HalfEdge(current->destination());
        HalfEdge* toCenterTwin = new HalfEdge(newVertex);
        
        HalfEdgeList boundaryReplacement;
        boundaryReplacement.append(fromCenterTwin, 1);
        boundaryReplacement.append(toCenterTwin, 1);
        face->replaceBoundary(current, current, fromCenterTwin);
        
        HalfEdgeList newFaceBoundary;
        newFaceBoundary.append(fromCenter, 1);
        newFaceBoundary.append(current, 1);
        newFaceBoundary.append(toCenter, 1);
        
        Face* newFace = new Face(newFaceBoundary);
        m_faces.append(newFace, 1);
        m_edges.append(new Edge(fromCenter, fromCenterTwin), 1);
        m_edges.append(new Edge(toCenter, toCenterTwin), 1);
        
        callback.faceWasSplit(face, newFace);
        current = next;
    }
    
    // Now just chop off more triangles until only triangles remain.
    for (size_t i = 0; i < vertexCount - 2; ++i) {
        HalfEdge* next = current->next();
        chopFace(face, current, callback);
        current = next;
    }
    
    return SplitResult(true, newVertex->position());
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
template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::splitFace(Face* face, HalfEdge* halfEdge, Callback& callback) {
    HalfEdge* next = halfEdge;
    while (face->vertexCount() > 3) {
        HalfEdge* previous = next->previous();
        chopFace(face, previous, callback);
    }
}

// Creates a new face by chopping off one triangle of the given face. The triangle
// will have the destination of the given edge, the origin of the given edge, and
// the origin of the given edge's predecessor as its vertices.
template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::chopFace(Face* face, HalfEdge* halfEdge, Callback& callback) {
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
    
    Face* newFace = new Face(boundary);
    callback.faceWasSplit(face, newFace);
    
    m_faces.append(newFace, 1);
    m_edges.append(new Edge(newEdge1, newEdge2), 1);
}

template <typename T, typename FP, typename VP>
struct Polyhedron<T,FP,VP>::MoveVertexResult {
    typedef enum {
        Type_VertexMoved,
        Type_VertexDeleted,
        Type_VertexUnchanged
    } Type;
    
    const Type type;
    V originalPosition;
    Vertex* vertex;
    
    MoveVertexResult(const Type i_type, const V& i_originalPosition, Vertex* i_vertex = NULL) :
    type(i_type),
    originalPosition(i_originalPosition),
    vertex(i_vertex) {
        assert(type != Type_VertexDeleted || vertex == NULL);
    }
    
    bool moved() const     { return type == Type_VertexMoved; }
    bool deleted() const   { return type == Type_VertexDeleted; }
    bool unchanged() const { return type == Type_VertexUnchanged; }
};

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::MoveVertexResult Polyhedron<T,FP,VP>::moveVertex(Vertex* vertex, const V& destination, const bool allowMergeIncidentVertices, Callback& callback) {
    ensure(vertex != NULL, "vertex is null");
    if (vertex->position() == destination)
        return MoveVertexResult(MoveVertexResult::Type_VertexUnchanged, vertex->position());
    
    if (point())
        return movePointVertex(vertex, destination, callback);
    else if (edge())
        return moveEdgeVertex(vertex, destination, allowMergeIncidentVertices, callback);
    else if (polygon())
        return movePolygonVertex(vertex, destination, allowMergeIncidentVertices, callback);
    else
        return movePolyhedronVertex(vertex, destination, allowMergeIncidentVertices, callback);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::MoveVertexResult Polyhedron<T,FP,VP>::movePointVertex(Vertex* vertex, const V& destination, Callback& callback) {
    const V originalPosition(vertex->position());
    vertex->setPosition(destination);
    return MoveVertexResult(MoveVertexResult::Type_VertexMoved, originalPosition, vertex);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::MoveVertexResult Polyhedron<T,FP,VP>::moveEdgeVertex(Vertex* vertex, const V& destination, const bool allowMergeIncidentVertices, Callback& callback) {
    const V originalPosition(vertex->position());
    Edge* edge = *m_edges.begin();
    Vertex* other = edge->otherVertex(vertex);
    if (other->position().equals(destination)) {
        if (!allowMergeIncidentVertices)
            return MoveVertexResult(MoveVertexResult::Type_VertexUnchanged, originalPosition, vertex);
        
        m_edges.remove(edge);
        delete edge;
        
        callback.vertexWillBeDeleted(vertex);
        m_vertices.remove(vertex);
        delete vertex;
        
        return MoveVertexResult(MoveVertexResult::Type_VertexMoved, originalPosition, other);
    } else {
        return movePointVertex(vertex, destination, callback);
    }
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::MoveVertexResult Polyhedron<T,FP,VP>::movePolygonVertex(Vertex* vertex, const V& destination, const bool allowMergeIncidentVertices, Callback& callback) {
    const V originalPosition(vertex->position());
    Face* face = *m_faces.begin();
    if (face->pointStatus(destination) != Math::PointStatus::PSInside)
        return MoveVertexResult(MoveVertexResult::Type_VertexUnchanged, originalPosition, vertex);
    
    Vertex* occupant = findVertexByPosition(destination);
    if (occupant != NULL && occupant != vertex) {
        HalfEdge* connectingEdge = vertex->findConnectingEdge(occupant);
        if (connectingEdge == NULL)
            connectingEdge = occupant->findConnectingEdge(vertex);
        if (!allowMergeIncidentVertices || connectingEdge == NULL)
            return MoveVertexResult(MoveVertexResult::Type_VertexUnchanged, originalPosition, vertex);

        Vertex* origin = connectingEdge->origin();
        Vertex* destVertex = connectingEdge->destination();
        
        Edge* edge = connectingEdge->edge();
        face->removeFromBoundary(connectingEdge);
        
        m_edges.remove(edge);
        delete edge;
        delete connectingEdge;
        
        callback.vertexWillBeDeleted(origin);
        m_vertices.remove(origin);
        delete origin;
        
        return MoveVertexResult(MoveVertexResult::Type_VertexMoved, originalPosition, destVertex);
    } else {
        return movePointVertex(vertex, destination, callback);
    }
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::MoveVertexResult Polyhedron<T,FP,VP>::movePolyhedronVertex(Vertex* vertex, const V& destination, const bool allowMergeIncidentVertices, Callback& callback) {
    
    const V originalPosition = vertex->position();
    if (!validPolyhedronVertexMove(vertex, destination))
        return MoveVertexResult(MoveVertexResult::Type_VertexUnchanged, originalPosition, vertex);
    
    Vertex* occupant = findVertexByPosition(destination);
    if (occupant != NULL) {
        if (!allowMergeIncidentVertices || vertex->findConnectingEdge(occupant) == NULL)
            return MoveVertexResult(MoveVertexResult::Type_VertexUnchanged, originalPosition, vertex);
    }
    
    removeVertex(vertex, callback);
    if (occupant != NULL)
        return MoveVertexResult(MoveVertexResult::Type_VertexMoved, originalPosition, occupant);

    if (!validPolyhedronVertexMoveDestination(originalPosition, destination))
        return MoveVertexResult(MoveVertexResult::Type_VertexDeleted, originalPosition);
    
    Vertex* newVertex = addPoint(destination, callback);
    if (newVertex == NULL)
        return MoveVertexResult(MoveVertexResult::Type_VertexDeleted, originalPosition);
    return MoveVertexResult(MoveVertexResult::Type_VertexMoved, originalPosition, newVertex);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::validPolyhedronVertexMove(Vertex* vertex, const V& destination) const {
    // This is the potentially remaining face if the given vertex were to be removed.
    Face* face = vertex->leaving()->next()->twin()->face();
    if (face->vertexCount() != vertexCount() - 1)
        return true;
    
    // If the given vertex is removed, then this polyhedron turns into a polygon.
    // Now check whether the vertex is moved to a position inside the plane of the remaining face.
    return face->pointStatus(destination) != Math::PointStatus::PSInside;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::validPolyhedronVertexMoveDestination(const V& origin, const V& destination) const {
    // Check whether the vertex at origin would travel through any of the faces which are now visible from the original position.
    // Assumes that the vertex has already been removed. Possible optimization: Get the newly woven faces when the vertex
    // was removed.
    
    Face* firstFace = m_faces.front();
    Face* currentFace = firstFace;
    do {
        if (currentFace->pointStatus(origin) == Math::PointStatus::PSAbove &&
            currentFace->pointStatus(destination) == Math::PointStatus::PSBelow) {
            const Ray<T,3> ray(origin, (destination - origin).normalized());
            const T distance = currentFace->intersectWithRay(ray, Math::Side_Front);
            if (!Math::isnan(distance)) {
                const T distance2 = distance * distance;
                if (distance2 <= ray.squaredDistanceToPoint(destination).rayDistance)
                    return false;
            }
        }
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return true;
}


#endif
