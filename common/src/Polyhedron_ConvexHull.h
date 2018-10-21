/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_Polyhedron_ConvexHull_h
#define TrenchBroom_Polyhedron_ConvexHull_h

#include <vecmath/segment.h>
#include <vecmath/plane.h>
#include <vecmath/bbox.h>
#include <vecmath/constants.h>
#include <vecmath/util.h>

#include <list>

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Seam {
public:
    typedef std::list<Edge*> List;
private:
    List m_edges;
public:
    typedef typename List::iterator iterator;
    typedef typename List::const_iterator const_iterator;
public:
    void push_back(Edge* edge) {
        ensure(edge != nullptr, "edge is null");
        assert(empty() || edge != last());
        assert(checkEdge(edge));
        m_edges.push_back(edge);
    }
    
    void replace(typename List::iterator first, typename List::iterator end, Edge* replacement) {
        m_edges.erase(first, end);
        m_edges.insert(end, replacement);
        assert(check());
    }

    template <typename C>
    bool shift(const C& criterion) {
        size_t i = 0;
        while (i < m_edges.size()) {
            if (criterion(static_cast<const Seam&>(*this)))
                return true;
            shift();
            ++i;
        }
        return false;
    }
    
    void shift() {
        assert(!m_edges.empty());
        iterator pos = std::end(m_edges);
        iterator first = std::begin(m_edges);
        iterator last = first;
        std::advance(last, 1);
        
        m_edges.splice(pos, m_edges, first, last);
        assert(check());
    }

    bool empty() const {
        return m_edges.empty();
    }
    
    size_t size() const {
        return m_edges.size();
    }
    
    Edge* first() const {
        assert(!empty());
        return m_edges.front();
    }
    
    Edge* second() const {
        assert(size() > 1);
        const_iterator it = std::begin(m_edges);
        std::advance(it, 1);
        return *it;
    }
    
    Edge* last() const {
        assert(!empty());
        return m_edges.back();
    }
    
    iterator begin() {
        return std::begin(m_edges);
    }
    
    iterator end() {
        return std::end(m_edges);
    }
    
    const_iterator begin() const {
        return std::begin(m_edges);
    }

    const_iterator end() const {
        return std::end(m_edges);
    }
    
    void clear() {
        m_edges.clear();
    }
    
    void print() const {
        for (const Edge* edge : m_edges) {
            std::cout << "(" << edge->secondVertex()->position() << ") -> (" << edge->firstVertex()->position() << ")" << std::endl;
        }
    }
    
    bool hasMultipleLoops() const {
        assert(size() > 2);
        
        VertexSet visitedVertices;
        for (const Edge* edge : m_edges) {
            if (!visitedVertices.insert(edge->secondVertex()).second)
                return true;
        }
        return false;
    }
private:
    // Check whether the given edge is connected to the last edge
    // of the current seam with a vertex.
    bool checkEdge(Edge* edge) const {
        if (m_edges.empty())
            return true;
        Edge* last = m_edges.back();
        if (last->firstVertex() == edge->secondVertex())
            return true;
        return false;
    }

    
    bool check() const {
        assert(size() > 2);
        
        const Edge* last = m_edges.back();
        for (const Edge* edge : m_edges) {
            if (last->firstVertex() != edge->secondVertex())
                return false;
            
            last = edge;
        }
        return true;
    }
};

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addPoints(const std::vector<V>& points) {
    addPoints(std::begin(points), std::end(points));
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addPoints(const std::vector<V>& points, Callback& callback) {
    addPoints(std::begin(points), std::end(points), callback);
}

template <typename T, typename FP, typename VP> template <typename I>
void Polyhedron<T,FP,VP>::addPoints(I cur, I end) {
    Callback c;
    while (cur != end)
        addPoint(*cur++, c);
}

template <typename T, typename FP, typename VP> template <typename I>
void Polyhedron<T,FP,VP>::addPoints(I cur, I end, Callback& callback) {
    while (cur != end)
        addPoint(*cur++, callback);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addPoint(const V& position) {
    Callback c;
    return addPoint(position, c);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addPoint(const V& position, Callback& callback) {
    assert(checkInvariant());
    Vertex* result = nullptr;
    switch (vertexCount()) {
        case 0:
            result = addFirstPoint(position, callback);
            m_bounds.min = m_bounds.max = position;
            break;
        case 1:
            result = addSecondPoint(position, callback);
            m_bounds = vm::merge(m_bounds, position);
            break;
        case 2:
            result = addThirdPoint(position, callback);
            m_bounds = vm::merge(m_bounds, position);
            break;
        default:
            result = addFurtherPoint(position, callback);
            if (result != nullptr) {
                m_bounds = vm::merge(m_bounds, position);
            }
            break;
    }
    assert(checkInvariant());
    if (result != nullptr) {
        callback.vertexWasAdded(result);
    }
    return result;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::removeVertex(Vertex* vertex) {
    ensure(vertex != nullptr, "vertex is null");
    Callback c;
    removeVertex(vertex, c);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::removeVertex(Vertex* vertex, Callback& callback) {
    ensure(vertex != nullptr, "vertex is null");
    assert(findVertexByPosition(vertex->position()) == vertex);
    assert(checkInvariant());
    
    callback.vertexWillBeRemoved(vertex);
    
    if (point())
        removeSingleVertex(vertex, callback);
    else if (edge())
        removeVertexFromEdge(vertex, callback);
    else if (polygon())
        removeVertexFromPolygon(vertex, callback);
    else
        removeVertexFromPolyhedron(vertex, callback);
    
    assert(checkInvariant());
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::removeVertexByPosition(const V& position) {
    Vertex* vertex = findVertexByPosition(position);
    ensure(vertex != nullptr, "couldn't find vertex to remove");
    removeVertex(vertex);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::merge(const Polyhedron& other) {
    Callback c;
    merge(other, c);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::merge(const Polyhedron& other, Callback& callback) {
    if (!other.empty()) {
        const Vertex* firstVertex = other.vertices().front();
        const Vertex* currentVertex = firstVertex;
        do {
            addPoint(currentVertex->position(), callback);
            currentVertex = currentVertex->next();
        } while (currentVertex != firstVertex);
    }
}

// Adds the given point to an empty polyhedron.
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addFirstPoint(const V& position, Callback& callback) {
    assert(empty());
    Vertex* newVertex = new Vertex(position);
    m_vertices.append(newVertex, 1);
    callback.vertexWasCreated(newVertex);
    return newVertex;
}

// Adds the given point to a polyhedron that contains one point.
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addSecondPoint(const V& position, Callback& callback) {
    assert(point());
    
    Vertex* onlyVertex = *std::begin(m_vertices);
    if (position != onlyVertex->position()) {
        Vertex* newVertex = new Vertex(position);
        m_vertices.append(newVertex, 1);
        callback.vertexWasCreated(newVertex);
        
        HalfEdge* halfEdge1 = new HalfEdge(onlyVertex);
        HalfEdge* halfEdge2 = new HalfEdge(newVertex);
        Edge* edge = new Edge(halfEdge1, halfEdge2);
        m_edges.append(edge, 1);
        return newVertex;
    } else {
        return nullptr;
    }
}

// Adds the given point to a polyhedron that contains one edge.
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addThirdPoint(const V& position, Callback& callback) {
    assert(edge());
    
    Vertex* v1 = m_vertices.front();
    Vertex* v2 = v1->next();
    
    if (colinear(v1->position(), v2->position(), position)) {
        return addColinearThirdPoint(position, callback);
    } else {
        return addNonColinearThirdPoint(position, callback);
    }
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addColinearThirdPoint(const V& position, Callback& callback) {
    assert(edge());
    
    auto* v1 = m_vertices.front();
    auto* v2 = v1->next();
    assert(colinear(v1->position(), v2->position(), position));

    if (vm::segment<T,3>(v1->position(), v2->position()).contains(position, vm::constants<T>::almostZero())) {
        return nullptr;
    }

    if (vm::segment<T,3>(position, v2->position()).contains(v1->position(), vm::constants<T>::almostZero())) {
        v1->setPosition(position);
        return v1;
    }

    assert((vm::segment<T,3>(position, v1->position()).contains(v2->position(), vm::constants<T>::almostZero())));
    v2->setPosition(position);
    return v2;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addNonColinearThirdPoint(const V& position, Callback& callback) {
    assert(edge());

    Vertex* v1 = m_vertices.front();
    Vertex* v2 = v1->next();
    assert(!colinear(v1->position(), v2->position(), position));
    
    HalfEdge* h1 = v1->leaving();
    HalfEdge* h2 = v2->leaving();
    assert(h1->next() == h1);
    assert(h1->previous() == h1);
    assert(h2->next() == h2);
    assert(h2->previous() == h2);
    
    Vertex* v3 = new Vertex(position);
    HalfEdge* h3 = new HalfEdge(v3);
    
    Edge* e1 = m_edges.front();
    e1->makeFirstEdge(h1);
    e1->unsetSecondEdge();
    
    HalfEdgeList boundary;
    boundary.append(h1, 1);
    boundary.append(h2, 1);
    boundary.append(h3, 1);
    
    Face* face = new Face(boundary);
    
    Edge* e2 = new Edge(h2);
    Edge* e3 = new Edge(h3);
    
    m_vertices.append(v3, 1);
    m_edges.append(e2, 1);
    m_edges.append(e3, 1);
    m_faces.append(face, 1);
    
    callback.vertexWasCreated(v1);
    callback.faceWasCreated(face);
    
    return v3;
}


// Adds the given point to a polyhedron that is either a polygon or a polyhedron.
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addFurtherPoint(const V& position, Callback& callback) {
    if (faceCount() == 1)
        return addFurtherPointToPolygon(position, callback);
    else
        return addFurtherPointToPolyhedron(position, callback);
}

//Adds the given point to a polygon. The result is either a differen polygon if the
// given point is coplanar to the already existing polygon, or a polyhedron if the
// given point is not coplanar.
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addFurtherPointToPolygon(const V& position, Callback& callback) {
    Face* face = m_faces.front();
    const vm::point_status status = face->pointStatus(position);
    switch (status) {
        case vm::point_status::inside:
            return addPointToPolygon(position, callback);
        case vm::point_status::above:
            face->flip();
            callback.faceWasFlipped(face);
            switchFallthrough();
        case vm::point_status::below:
            return makePolyhedron(position, callback);
    }
    // will never be reached
    return nullptr;
}

// Adds the given coplanar point to a polyhedron that is a polygon.
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addPointToPolygon(const V& position, Callback& callback) {
    assert(polygon());
    
    Face* face = m_faces.front();
    vm::plane<T,3> facePlane = callback.getPlane(face);
    
    HalfEdge* firstVisibleEdge = nullptr;
    HalfEdge* lastVisibleEdge = nullptr;
    
    HalfEdge* firstEdge = face->boundary().front();
    HalfEdge* curEdge = firstEdge;
    do {
        HalfEdge* prevEdge = curEdge->previous();
        HalfEdge* nextEdge = curEdge->next();
        const vm::point_status prevStatus = prevEdge->pointStatus(facePlane.normal, position);
        const vm::point_status  curStatus =  curEdge->pointStatus(facePlane.normal, position);
        const vm::point_status nextStatus = nextEdge->pointStatus(facePlane.normal, position);
        
        // If the current edge contains the point, it will not be added anyway.
        if (curStatus == vm::point_status::inside &&
            vm::segment<T,3>(curEdge->origin()->position(), curEdge->destination()->position()).contains(position, vm::constants<T>::almostZero())) {
            return nullptr;
        }

        if (prevStatus == vm::point_status::below &&  curStatus != vm::point_status::below) {
            firstVisibleEdge = curEdge;
        }

        if ( curStatus != vm::point_status::below && nextStatus == vm::point_status::below) {
            lastVisibleEdge = curEdge;
        }

        curEdge = curEdge->next();
    } while (curEdge != firstEdge && (firstVisibleEdge == nullptr || lastVisibleEdge == nullptr));
    
    // Is the point contained in the polygon?
    if (firstVisibleEdge == nullptr || lastVisibleEdge == nullptr) {
        return nullptr;
    }

    // Now we know which edges are visible from the point. These will have to be replaced with two new edges.
    Vertex* newVertex = new Vertex(position);
    HalfEdge* h1 = new HalfEdge(firstVisibleEdge->origin());
    HalfEdge* h2 = new HalfEdge(newVertex);
    
    face->insertIntoBoundaryAfter(lastVisibleEdge, h1);
    face->insertIntoBoundaryAfter(h1, h2);
    face->removeFromBoundary(firstVisibleEdge, lastVisibleEdge);

    h1->setAsLeaving();
    
    Edge* e1 = new Edge(h1);
    Edge* e2 = new Edge(h2);

    // Delete the visible half edges, the vertices and edges.
    firstEdge = firstVisibleEdge;
    curEdge = firstEdge;
    do {
        HalfEdge* nextEdge = curEdge->next();
        
        Edge* edge = curEdge->edge();
        m_edges.remove(edge);
        delete edge;
        
        if (curEdge != firstEdge) {
            Vertex* vertex = curEdge->origin();
            callback.vertexWillBeDeleted(vertex);
            m_vertices.remove(vertex);
            delete vertex;
        }
        
        curEdge = nextEdge;
    } while (curEdge != firstEdge);
    
    m_edges.append(e1, 1);
    m_edges.append(e2, 1);
    m_vertices.append(newVertex, 1);
    callback.vertexWasCreated(newVertex);
    
    return newVertex;
}

// Creates a new polygon from the given set of coplanar points. Assumes that
// this polyhedron is empty and that the given point list contains at least three
// non-colinear points.
template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::makePolygon(const std::vector<V>& positions, Callback& callback) {
    assert(empty());
    assert(positions.size() > 2);
    
    HalfEdgeList boundary;
    for (size_t i = 0; i < positions.size(); ++i) {
        const V& p = positions[i];
        Vertex* v = new Vertex(p);
        HalfEdge* h = new HalfEdge(v);
        Edge* e = new Edge(h);
        
        m_vertices.append(v, 1);
        callback.vertexWasCreated(v);
        
        boundary.append(h, 1);
        m_edges.append(e, 1);
    }
    
    Face* f = new Face(boundary);
    callback.faceWasCreated(f);
    m_faces.append(f, 1);
}

// Converts a coplanar polyhedron into a non-coplanar one by adding the given
// point, which is assumed to be non-coplanar to the points in this polyhedron.
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::makePolyhedron(const V& position, Callback& callback) {
    assert(polygon());
    
    Seam seam;
    Face* face = m_faces.front();
    const HalfEdgeList& boundary = face->boundary();
    
    HalfEdge* first = boundary.front();
    HalfEdge* current = first;
    do {
        seam.push_back(current->edge());
        current = current->previous(); // The seam must be CCW, so we have to iterate in reverse order in this case.
    } while (current != first);

    return addPointToPolyhedron(position, seam, callback);
}

// Adds the given point to this polyhedron.
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addFurtherPointToPolyhedron(const V& position, Callback& callback) {
    assert(polyhedron());
    if (contains(position, callback))
        return nullptr;
    
    const Seam seam = createSeam(SplitByVisibilityCriterion(position));
    
    // If no correct seam could be created, we assume that the vertex was inside the polyhedron.
    // If the seam has multiple loops, this indicates that the point to be added is very close to
    // another vertex and no correct seam can be computed due to imprecision. In that case, we just
    // assume that the vertex is inside the polyhedron and skip it.
    if (seam.empty() || seam.hasMultipleLoops())
        return nullptr;
    
    assert(checkFaceBoundaries());
    split(seam, callback);
    assert(checkFaceBoundaries());

    return addPointToPolyhedron(position, seam, callback);
}

// Adds the given point to this polyhedron by weaving a cap over the given seam.
// Assumes that this polyhedron has been split by the given seam.
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::addPointToPolyhedron(const V& position, const Seam& seam, Callback& callback) {
    assert(seam.size() >= 3);
    assert(!seam.hasMultipleLoops());

    Vertex* newVertex = weave(seam, position, callback);
    assert(polyhedron());
    return newVertex;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::removeSingleVertex(Vertex* vertex, Callback& callback) {
    assert(point());
    
    callback.vertexWillBeDeleted(vertex);
    m_vertices.remove(vertex);
    delete vertex;
    
    assert(empty());
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::removeVertexFromEdge(Vertex* vertex, Callback& callback) {
    assert(edge());
    
    HalfEdge* halfEdge = vertex->leaving();
    Edge* edge = halfEdge->edge();
    
    delete halfEdge;
    
    m_edges.remove(edge);
    delete edge;
    
    callback.vertexWillBeDeleted(vertex);
    m_vertices.remove(vertex);
    delete vertex;
    
    assert(point());
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::removeVertexFromPolygon(Vertex* vertex, Callback& callback) {
    assert(polygon());
    
    if (vertexCount() == 3)
        removeThirdVertexFromPolygon(vertex, callback);
    else
        removeFurtherVertexFromPolygon(vertex, callback);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::removeThirdVertexFromPolygon(Vertex* vertex, Callback& callback) {
    assert(vertexCount() == 3);
    
    HalfEdge* removedHalfEdge = vertex->leaving();
    HalfEdge* firstRemainingHalfEdge = removedHalfEdge->next();
    HalfEdge* secondRemainingHalfEdge = firstRemainingHalfEdge->next();
    
    Edge* remainingEdge = firstRemainingHalfEdge->edge();
    Edge* firstRemovedEdge = removedHalfEdge->edge();
    Edge* secondRemovedEdge = secondRemainingHalfEdge->edge();
    
    Face* face = removedHalfEdge->face();
    callback.faceWillBeDeleted(face);

    // Remove in two steps so that the remaining half edges form two loops.
    face->removeFromBoundary(firstRemainingHalfEdge, firstRemainingHalfEdge);
    face->removeFromBoundary(secondRemainingHalfEdge, secondRemainingHalfEdge);
    
    m_faces.remove(face);
    delete face;

    secondRemainingHalfEdge->unsetEdge();
    remainingEdge->makeFirstEdge(firstRemainingHalfEdge);
    remainingEdge->setSecondEdge(secondRemainingHalfEdge);
    
    m_edges.remove(firstRemovedEdge);
    m_edges.remove(secondRemovedEdge);
    delete firstRemovedEdge;
    delete secondRemovedEdge;
    
    callback.vertexWillBeDeleted(vertex);
    m_vertices.remove(vertex);
    delete vertex;
    
    assert(edge());
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::removeFurtherVertexFromPolygon(Vertex* vertex, Callback& callback) {
    assert(polygon() && vertexCount() > 3);
    
    HalfEdge* outgoingHalfEdge = vertex->leaving();
    Edge* outgoingEdge = outgoingHalfEdge->edge();
    
    Face* face = outgoingHalfEdge->face();
    face->removeFromBoundary(outgoingHalfEdge);
    
    m_edges.remove(outgoingEdge);
    delete outgoingEdge;
    delete outgoingHalfEdge;
    
    callback.vertexWillBeDeleted(vertex);
    m_vertices.remove(vertex);
    delete vertex;
    
    assert(polygon());
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::removeVertexFromPolyhedron(Vertex* vertex, Callback& callback) {
    assert(polyhedron());
    
    const Seam seam = createSeam(SplitByConnectivityCriterion(vertex));
    split(seam, callback);

    std::cout << "Printing all faces..." << std::endl;
    for (const auto* face : m_faces) {
        std::cout << "Face: " << std::endl << *face;
    }
    std::cout << "Done." << std::endl;

    if (faceCount() > 1)
        sealWithMultiplePolygons(seam, callback);
    updateBounds();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Seam Polyhedron<T,FP,VP>::createSeam(const SplittingCriterion& criterion) {
    Seam seam;
    
    Edge* first = criterion.findFirstSplittingEdge(m_edges);
    if (first != nullptr) {
        Edge* current = first;
        do {
            ensure(current != nullptr, "current is null");
            seam.push_back(current);
            current = criterion.findNextSplittingEdge(current);
        } while (current != first);
    }

    // The resulting seam contains the edges where one face satisfies the given criterion while the other does not.
    // The edges are in counter clockwise order and consecutive, and they form a loop. They are oriented such that
    // the first face matches the criterion and the second face does not.
    return seam;
}

// Splits this polyhedron along the given seam and removes all faces, edges and vertices which are "above" the seam.
template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::split(const Seam& seam, Callback& callback) {
    assert(seam.size() >= 3);
#ifndef NDEBUG
    if (seam.hasMultipleLoops()) {
        seam.print();
    }
    assert(!seam.hasMultipleLoops());
#endif
    
    // First, unset the second half edge of every seam edge.
    // Thereby remember the second half edge of the first seam edge.
    // Note that all seam edges are oriented such that their second half edge belongs
    // to the portion of the polyhedron that must be removed.
    HalfEdge* first = seam.first()->secondEdge();
    for (Edge* edge : seam) {
        // Set the first edge as the leaving edge. Since the first one will remain
        // in the polyhedron, we can use this as an indicator whether or not to
        // delete a vertex in the call to deleteFaces.
        edge->setFirstAsLeaving();
        edge->unsetSecondEdge();
    }
    
    // Now we must delete all the faces, edges, and vertices which are above the seam.
    // Since we opened the seam, that is, we unset the 2nd half edge of each seam edge,
    // which belongs to the portion of the polyhedron that will be deleted, the deletion
    // will not touch the faces that should remain in the polyhedron. Additionally, the
    // seam edges will also not be deleted.
    // The first half edge we remembered above is our entry point into that portion of the polyhedron.
    // We must remember which faces we have already visited to stop the recursion.
    FaceSet visitedFaces;
    VertexList verticesToDelete; // Will automatically delete the vertices when it falls out of scope
    deleteFaces(first, visitedFaces, verticesToDelete, callback);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::deleteFaces(HalfEdge* first, FaceSet& visitedFaces, VertexList& verticesToDelete, Callback& callback) {
    Face* face = first->face();
    
    // Have we already visited this face?
    if (!visitedFaces.insert(face).second)
        return;

    // Callback must be called now when the face is still fully intact.
    callback.faceWillBeDeleted(face);

    HalfEdge* current = first;
    do {
        Edge* edge = current->edge();
        if (edge != nullptr) {
            // This indicates that the current half edge was not part of the seam before
            // the seam was opened, i.e., it may have a neighbour that should also be deleted.
            
            // If the current edge has a neighbour, we can go ahead and delete it.
            // Once the function returns, the neighbour is definitely deleted unless
            // we are in a recursive call where that neighbour is being deleted by one
            // of our callers. In that case, the call to deleteFaces returned immediately.
            if (edge->fullySpecified()) {
                deleteFaces(edge->twin(current), visitedFaces, verticesToDelete, callback);
            }

            if (edge->fullySpecified()) {
                // This indicates that we are in a recursive call and that the neighbour across
                // the current edge is going to be deleted by one of our callers. We open the
                // edge and unset it so that it is not considered again later.
                edge->makeSecondEdge(current);
                edge->unsetSecondEdge();
            } else {
                // This indicates that the neighbour across the current edges has already been deleted
                // or that it will be deleted by one of our callers.
                // This means that we can safely unset the edge and delete it.
                current->unsetEdge();
                m_edges.remove(edge);
                delete edge;
            }
        }
        
        Vertex* origin = current->origin();
        if (origin->leaving() == current) {
            // We expect that the vertices on the seam have had a remaining edge
            // set as their leaving edge before the call to this function.
            callback.vertexWillBeDeleted(origin);
            m_vertices.remove(origin);
            verticesToDelete.append(origin, 1);
        }
        current = current->next();
    } while (current != first);
    
    m_faces.remove(face);
    delete face;
}

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::ShiftSeamForSealing {
public:
    bool operator()(const Seam& seam) const {
        const auto* first = seam.first();
        const auto* second = seam.second();
        
        if (first->firstFace() == second->firstFace()) {
            return false;
        }

        const auto* v1 = first->firstVertex();
        const auto* v2 = first->secondVertex();
        const auto* v3 = second->firstVertex();

        const auto [valid, plane] = fromPoints(v1->position(), v2->position(), v3->position());
        if (!valid) {
            return false;
        }

        const auto* last = seam.last();
        const auto* v4 = last->secondVertex();
        if (plane.pointStatus(v4->position()) != vm::point_status::below) {
            return false;
        }

        return checkRemainingPoints(plane, seam);
    }
private:
    bool checkRemainingPoints(const vm::plane<T,3>& plane, const Seam& seam) const {
        if (seam.size() < 5)
            return true;
        
        typename Seam::const_iterator it = std::begin(seam);
        typename Seam::const_iterator end = std::end(seam);
        
        std::advance(it, 2);
        std::advance(end, -1);
        
        while (it != end) {
            const Edge* edge = *it;
            const Vertex* vertex = edge->firstVertex();
            if (plane.pointStatus(vertex->position()) == vm::point_status::above)
                return false;
            ++it;
        }
        return true;
    }
};

/**
 Weaves a new cap onto the given seam edges. The new cap will be a single polygon, so we assume that all seam vertices lie
 on a plane.
 */
template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::sealWithSinglePolygon(const Seam& seam, Callback& callback) {
    assert(seam.size() >= 3);
    assert(!seam.hasMultipleLoops());
    
    HalfEdgeList boundary;
    for (Edge* seamEdge : seam) {
        assert(!seamEdge->fullySpecified());
        
        Vertex* origin = seamEdge->secondVertex();
        HalfEdge* boundaryEdge = new HalfEdge(origin);
        boundary.append(boundaryEdge, 1);
        seamEdge->setSecondEdge(boundaryEdge);
    }
    
    Face* face = new Face(boundary);
    callback.faceWasCreated(face);
    m_faces.append(face, 1);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::sealWithMultiplePolygons(Seam seam, Callback& callback) {
    assert(seam.size() >= 3);
    assert(!seam.hasMultipleLoops());
    
    if (seam.size() == 3) {
        sealWithSinglePolygon(seam, callback);
        return;
    }

    std::cout << "Printing new faces..." << std::endl;

    while (!seam.empty()) {
        assert(seam.size() >= 3);
        
        if (seam.size() > 3) {
            seam.shift(ShiftSeamForSealing());
        }

        HalfEdgeList boundary;

        auto firstIt = std::begin(seam);
        auto endIt = firstIt;
        auto* firstEdge = *endIt;
        ++endIt;
        
        auto* secondEdge = *endIt;
        ++endIt;
        
        auto* firstBoundaryEdge = new HalfEdge(firstEdge->secondVertex());
        auto* secondBoundaryEdge = new HalfEdge(secondEdge->secondVertex());
        
        boundary.append(firstBoundaryEdge, 1);
        boundary.append(secondBoundaryEdge, 1);
        
        firstEdge->setSecondEdge(firstBoundaryEdge);
        secondEdge->setSecondEdge(secondBoundaryEdge);

        // try to add more points as long as they all lie on the same plane
        // as the first three points
        
        auto* v1 = firstEdge->firstVertex();
        auto* v2 = firstEdge->secondVertex();
        auto* v3 = secondEdge->firstVertex();

        const auto [valid, plane] = fromPoints(v1->position(), v2->position(), v3->position());
        assert(valid); unused(valid);

        auto* lastVertex = v3;
        while (endIt != std::end(seam) && plane.pointStatus((*endIt)->firstVertex()->position()) == vm::point_status::inside) {
            auto* curEdge = *endIt;
            ++endIt;
            
            auto* curBoundaryEdge = new HalfEdge(curEdge->secondVertex());
            boundary.append(curBoundaryEdge, 1);
            curEdge->setSecondEdge(curBoundaryEdge);
            
            lastVertex = curEdge->firstVertex();
        }
        
        if (endIt != std::end(seam)) {
            auto* lastBoundaryEdge = new HalfEdge(lastVertex);
            boundary.append(lastBoundaryEdge, 1);

            auto* newEdge = new Edge(lastBoundaryEdge);
            m_edges.append(newEdge, 1);
            seam.replace(firstIt, endIt, newEdge);
        } else {
            seam.clear();
        }
        
        auto* newFace = new Face(boundary);
        std::cout << "New Face:" << std::endl << *newFace;
        callback.faceWasCreated(newFace);
        m_faces.append(newFace, 1);
    }

    std::cout << "Done." << std::endl;
}



template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::ShiftSeamForWeaving {
private:
    const V m_position;
public:
    ShiftSeamForWeaving(const V& position) : m_position(position) {}
public:
    bool operator()(const Seam& seam) const {
        const auto* last = seam.last();
        const auto* first = seam.first();
        
        const auto* v1 = last->firstVertex();
        const auto* v2 = last->secondVertex();
        const auto* v3 = first->firstVertex();
        assert(v3 != v1);
        assert(v3 != v2);

        const auto [valid, lastPlane] = fromPoints(m_position, v1->position(), v2->position());
        assert(valid); unused(valid);

        const auto status = lastPlane.pointStatus(v3->position());
        return status == vm::point_status::below;
    }
};

/**
 Weaves a new cap onto the given seam edges. The new cap will form a triangle fan (actually a cone) with a new vertex
 at the location of the given point being shared by all the newly created triangles.
 */
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::weave(Seam seam, const V& position, Callback& callback) {
    assert(seam.size() >= 3);
    assert(!seam.hasMultipleLoops());
    assertResult(seam.shift(ShiftSeamForWeaving(position)));
    
    auto* top = new Vertex(position);
    
    HalfEdge* first = nullptr;
    HalfEdge* last = nullptr;
    
    typename Seam::const_iterator it = std::begin(seam);
    while (it != std::end(seam)) {
        auto* edge = *it++;
        
        assert(!edge->fullySpecified());
        auto* v1 = edge->secondVertex();
        auto* v2 = edge->firstVertex();

        auto* h1 = new HalfEdge(top);
        auto* h2 = new HalfEdge(v1);
        auto* h3 = new HalfEdge(v2);
        auto* h = h3;
        
        HalfEdgeList boundary;
        boundary.append(h1, 1);
        boundary.append(h2, 1);
        boundary.append(h3, 1);
        edge->setSecondEdge(h2);
        
        if (it != std::end(seam)) {
            const auto [valid, plane] = fromPoints(top->position(), v2->position(), v1->position());
            assert(valid); unused(valid);

            auto* next = *it;
            
            // TODO use same coplanarity check as in Face::coplanar(const Face*) const ?
            while (it != std::end(seam) && plane.pointStatus(next->firstVertex()->position()) == vm::point_status::inside) {
                next->setSecondEdge(h);

                auto* v = next->firstVertex();
                h = new HalfEdge(v);
                boundary.append(h, 1);
                
				if (++it != std::end(seam)) {
                    next = *it;
                }
            }
        }
        
        Face* newFace = new Face(boundary);
        callback.faceWasCreated(newFace);
        m_faces.append(newFace, 1);
        
        if (last != nullptr)
            m_edges.append(new Edge(h1, last), 1);
        
        if (first == nullptr)
            first = h1;
        last = h;
    }

    assert(first->face() != last->face());
    m_edges.append(new Edge(first, last), 1);
    
    m_vertices.append(top, 1);
    callback.vertexWasCreated(top);
    
    return top;
}

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::SplittingCriterion {
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
        for (Edge* edge : edges) {
            const MatchResult result = matches(edge);
            switch (result) {
                case MatchResult_Second:
                    edge->flip();
                    switchFallthrough();
                case MatchResult_First:
                    return edge;
                case MatchResult_Both:
                case MatchResult_Neither:
                    break;
                switchDefault()
            }
        }
        return nullptr;
    }
    
    // finds the next seam edge in counter clockwise orientation
    Edge* findNextSplittingEdge(Edge* last) const {
        ensure(last != nullptr, "last is null");
        
        HalfEdge* halfEdge = last->firstEdge()->previous();
        Edge* next = halfEdge->edge();
        
        MatchResult result = matches(next);
        while (result != MatchResult_First && result != MatchResult_Second && next != last) {
            halfEdge = halfEdge->twin()->previous();
            next = halfEdge->edge();
            result = matches(next);
        }
        
        if (result != MatchResult_First && result != MatchResult_Second)
            return nullptr;
        
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

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::SplitByConnectivityCriterion : public Polyhedron<T,FP,VP>::SplittingCriterion {
private:
    const Vertex* m_vertex;
public:
    SplitByConnectivityCriterion(const Vertex* vertex) :
    m_vertex(vertex) {}
private:
    bool doMatches(const Face* face) const override {
        return !m_vertex->incident(face);
    }
};



template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::SplitByVisibilityCriterion : public Polyhedron<T,FP,VP>::SplittingCriterion {
private:
    V m_point;
public:
    SplitByVisibilityCriterion(const V& point) :
    m_point(point) {}
private:
    bool doMatches(const Face* face) const override {
        return face->pointStatus(m_point) == vm::point_status::below;
    }
};

#endif
