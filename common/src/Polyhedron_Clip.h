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

#ifndef TrenchBroom_Polyhedron_Clip_h
#define TrenchBroom_Polyhedron_Clip_h

template <typename T, typename FP>
class Polyhedron<T,FP>::SplitByPlaneCriterion : public Polyhedron<T,FP>::SplittingCriterion {
private:
    const Plane<T,3>& m_plane;
public:
    SplitByPlaneCriterion(const Plane<T,3>& plane) :
    m_plane(plane) {}
private:
    bool doMatches(const Face* face) const {
        // The plane matches if it has at least one vertex that is below the plane.
        const HalfEdge* firstEdge = face->boundary().front();
        const HalfEdge* currentEdge = firstEdge;
        do {
            const Vertex* origin = currentEdge->origin();
            if (m_plane.pointStatus(origin->position()) == Math::PointStatus::PSBelow)
                return true;
            currentEdge = currentEdge->next();
        } while (currentEdge != firstEdge);
        return false;
    }
};

template <typename T, typename FP>
struct Polyhedron<T,FP>::ClipResult {
    typedef enum {
        Type_ClipUnchanged,
        Type_ClipEmpty,
        Type_ClipSuccess
    } Type;
    
    const Type type;

    ClipResult(const Type i_type) :
    type(i_type) {}
    
    bool unchanged() const { return type == Type_ClipUnchanged; }
    bool empty() const     { return type == Type_ClipEmpty; }
    bool success() const   { return type == Type_ClipSuccess; }
};

template <typename T, typename FP>
typename Polyhedron<T,FP>::ClipResult Polyhedron<T,FP>::clip(const Plane<T,3>& plane) {
    Callback c;
    return clip(plane, c);
}

template <typename T, typename FP> template <typename C>
typename Polyhedron<T,FP>::ClipResult Polyhedron<T,FP>::clip(const Plane<T,3>& plane, C& callback) {
    if (isCoplanarToAnyFace(plane, callback))
        return ClipResult(ClipResult::Type_ClipUnchanged);
    
    const ClipResult vertexResult = checkIntersects(plane);
    if (!vertexResult.success())
        return vertexResult;
    
    // Now we know that the brush will be split.
    // The basic idea is now to split all faces which are intersected by the given plane so that the polyhedron
    // can be separated into two halves such that no face has vertices on opposite sides of the plane.
    intersectWithPlane(plane, callback);
    
    // We construct a seam along those edges which are completely inside the plane and delete the half of the
    // polyhedron that is above the plane. The remaining half is an open polyhedron (one face is missing) which
    // is below the plane.
    Seam seam;
    seam.reserve(16);
    split(SplitByPlaneCriterion(plane), seam, callback);
    
    // We seal the polyhedron by creating a new face.
    weaveCap(seam, callback);
    updateBounds();
    
    assert(checkInvariant());
    return ClipResult(ClipResult::Type_ClipSuccess);
}

template <typename T, typename FP> template <typename C>
bool Polyhedron<T,FP>::isCoplanarToAnyFace(const Plane<T,3>& plane, const C& callback) const {
    typename FaceList::const_iterator it, end;
    for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
        const Face* face = *it;
        const Plane<T,3> facePlane = callback.plane(face);
        if (plane.equals(facePlane))
            return true;
    }
    return false;
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::ClipResult Polyhedron<T,FP>::checkIntersects(const Plane<T,3>& plane) const {
    size_t above = 0;
    size_t below = 0;
    size_t inside = 0;
    
    typename VertexList::const_iterator it, end;
    for (it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        const Vertex* vertex = *it;
        const Math::PointStatus::Type status = plane.pointStatus(vertex->position());
        switch (status) {
            case Math::PointStatus::PSAbove:
                ++above;
                break;
            case Math::PointStatus::PSBelow:
                ++below;
                break;
            case Math::PointStatus::PSInside:
                ++inside;
                break;
            DEFAULT_SWITCH()
        }
    }
    
    assert(above + below + inside == m_vertices.size());
    if (below + inside == m_vertices.size())
        return ClipResult(ClipResult::Type_ClipUnchanged);
    if (above + inside == m_vertices.size())
        return ClipResult(ClipResult::Type_ClipEmpty);
    return ClipResult(ClipResult::Type_ClipSuccess);
}

template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::intersectWithPlane(const Plane<T,3>& plane, C& callback) {
    // First, we find a half edge that is intersected by the given plane.
    HalfEdge* initialEdge = findInitialIntersectingEdge(plane);
    assert(initialEdge != NULL);

    // Now we split the face to which this initial half edge belongs. The call returns the newly inserted edge
    // that connectes the (possibly newly inserted) vertices which are now within the plane.
    HalfEdge* currentEdge = intersectWithPlane(initialEdge, plane, callback);
    
    // The destination of that edge is the first vertex which we encountered (or inserted) which is inside the plane.
    // This is where our algorithm must stop. When we encounter that vertex again, we have completed the intersection
    // and the polyhedron can now be split in two by the given plane.
    Vertex* stopVertex = currentEdge->destination();
    do {
        // First we find the next face that is either split by the plane or which has an edge completely in the plane.
        currentEdge = findNextIntersectingEdge(currentEdge, plane);
        assert(currentEdge != NULL);
        
        // Now we split that face. Again, the returned edge connects the two (possibly inserted) vertices of that
        // face which are now inside the plane.
        currentEdge = intersectWithPlane(currentEdge, plane, callback);
    } while (currentEdge->destination() != stopVertex);
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::HalfEdge* Polyhedron<T,FP>::findInitialIntersectingEdge(const Plane<T,3>& plane) const {
    Edge* firstEdge = m_edges.front();
    Edge* currentEdge = firstEdge;
    do {
        HalfEdge* halfEdge = currentEdge->firstEdge();
        const Math::PointStatus::Type os = plane.pointStatus(halfEdge->origin()->position());
        const Math::PointStatus::Type ds = plane.pointStatus(halfEdge->destination()->position());
        if ((os == Math::PointStatus::PSInside && ds == Math::PointStatus::PSInside) ||
            (os == Math::PointStatus::PSInside && ds == Math::PointStatus::PSAbove) ||
            (os == Math::PointStatus::PSBelow  && ds == Math::PointStatus::PSAbove))
            return halfEdge;
        
        if (
            (os == Math::PointStatus::PSAbove  && ds == Math::PointStatus::PSInside) ||
            (os == Math::PointStatus::PSAbove  && ds == Math::PointStatus::PSBelow))
            return halfEdge->twin();
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return NULL;
}

template <typename T, typename FP> template <typename C>
typename Polyhedron<T,FP>::HalfEdge* Polyhedron<T,FP>::intersectWithPlane(HalfEdge* firstBoundaryEdge, const Plane<T,3>& plane, C& callback) {
    
    // Starting at the given edge, we search the boundary of the current face until we find an edge that is either split in two by the given plane
    // or where its origin is inside it. In the first case, we split the found edge by inserting a vertex at the position where
    // the plane intersects the edge. We remember the half edge starting at the newly inserted vertex as the seam origin or destination, depending
    // on whether it's the first or second such edge we have found. In the second case (the edge's origin is inside the plane), we just store the
    // half edge as either the seam origin or destination.
    // In the end, we have two vertices, identified by half edges belonging to the currently treated face, which lie inside the plane. If these two
    // vertices aren't already connected by an edge, we split the current face in two by inserting a new edge from the origin to the destination vertex.
    // Finally we must decide where to continue our search, that is, we find a face that is incident to the destination vertex such that it is split
    // by the given plane. We return the half edge of that face's boundary which starts in the destination vertex so that the search can continue there.
    
    HalfEdge* seamOrigin = NULL;
    HalfEdge* seamDestination = NULL;
    
    HalfEdge* currentBoundaryEdge = firstBoundaryEdge;
    do {
        const Math::PointStatus::Type os = plane.pointStatus(currentBoundaryEdge->origin()->position());
        const Math::PointStatus::Type ds = plane.pointStatus(currentBoundaryEdge->destination()->position());
        
        if (os == Math::PointStatus::PSInside) {
            if (seamOrigin == NULL)
                seamOrigin = currentBoundaryEdge;
            else
                seamDestination = currentBoundaryEdge;
            currentBoundaryEdge = currentBoundaryEdge->next();
        } else if ((os == Math::PointStatus::PSBelow && ds == Math::PointStatus::PSAbove) ||
                   (os == Math::PointStatus::PSAbove && ds == Math::PointStatus::PSBelow)) {
            // We have to split the edge and insert a new vertex, which will become the origin or destination of the new seam edge.
            Edge* currentEdge = currentBoundaryEdge->edge();
            Edge* newEdge = currentEdge->split(plane);
            m_edges.append(newEdge, 1);
            
            currentBoundaryEdge = currentBoundaryEdge->next();
            Vertex* newVertex = currentBoundaryEdge->origin();
            m_vertices.append(newVertex, 1);

            // The newly inserted vertex will be reexamined in the next loop iteration as it is now contained within the plane.
        } else {
            currentBoundaryEdge = currentBoundaryEdge->next();
        }
    } while (seamDestination == NULL && currentBoundaryEdge != firstBoundaryEdge);
    assert(seamOrigin != NULL);
    
    // The plane only touches one vertex of the face.
    if (seamDestination == NULL)
        return seamOrigin->previous();
    
    if (seamDestination->next() == seamOrigin) {
        std::swap(seamOrigin, seamDestination);
    } else if (seamOrigin->next() != seamDestination) {
        // If the origin and the destination are not already connected by an edge, we must split the current face and insert an edge
        // between them.
        // The newly created faces are supposed to be above the given plane, so we have to consider whether the destination of the
        // seam origin edge is above or below the plane.
        const Math::PointStatus::Type os = plane.pointStatus(seamOrigin->destination()->position());
        assert(os != Math::PointStatus::PSInside);
        if (os == Math::PointStatus::PSBelow)
            intersectWithPlane(seamOrigin, seamDestination, callback);
        else
            intersectWithPlane(seamDestination, seamOrigin, callback);
    }

    return seamDestination->previous();
}

template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::intersectWithPlane(HalfEdge* oldBoundaryFirst, HalfEdge* newBoundaryFirst, C& callback) {
    HalfEdge* newBoundaryLast = oldBoundaryFirst->previous();
    
    HalfEdge* oldBoundarySplitter = new HalfEdge(newBoundaryFirst->origin());
    HalfEdge* newBoundarySplitter = new HalfEdge(oldBoundaryFirst->origin());
    
    Face* oldFace = oldBoundaryFirst->face();
    newBoundarySplitter->setFace(oldFace);
    
    oldFace->insertIntoBoundaryAfter(newBoundaryLast, newBoundarySplitter);
    const size_t newBoundaryCount = oldFace->replaceBoundary(newBoundaryFirst, newBoundarySplitter, oldBoundarySplitter);
    
    HalfEdgeList newBoundary;
    newBoundary.append(newBoundaryFirst, newBoundaryCount);

    Face* newFace = new Face(newBoundary);
    Edge* newEdge = new Edge(oldBoundarySplitter, newBoundarySplitter);
    
    m_edges.append(newEdge, 1);
    m_faces.append(newFace, 1);
    
    callback.faceWasSplit(oldFace, newFace);
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::HalfEdge* Polyhedron<T,FP>::findNextIntersectingEdge(HalfEdge* searchFrom, const Plane<T,3>& plane) const {
    HalfEdge* currentEdge = searchFrom->next()->twin()->next();
    const HalfEdge* stopEdge = searchFrom->next();
    do {
        // Select two vertices that form a triangle (of an adjacent face) together with seamDestination's origin vertex.
        // If either of the two vertices is inside the plane or if they lie on different sides of it, then we have found
        // the next face to handle.
        
        Vertex* cd = currentEdge->destination();
        Vertex* po = currentEdge->previous()->origin();
        const Math::PointStatus::Type cds = plane.pointStatus(cd->position());
        const Math::PointStatus::Type pos = plane.pointStatus(po->position());
        
        if ((cds == Math::PointStatus::PSInside) ||
            (cds == Math::PointStatus::PSBelow && pos == Math::PointStatus::PSAbove) ||
            (cds == Math::PointStatus::PSAbove && pos == Math::PointStatus::PSBelow))
            return currentEdge;
        
        currentEdge = currentEdge->twin()->next();
    } while (currentEdge != stopEdge);
    return NULL;
}

#endif
