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

#ifndef TrenchBroom_Polyhedron_ConvexHull_h
#define TrenchBroom_Polyhedron_ConvexHull_h

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Seam {
private:
    typedef std::vector<Edge*> SeamList;
    SeamList m_edges;
public:
    typedef typename SeamList::iterator iterator;
    typedef typename SeamList::const_iterator const_iterator;
public:
    Seam() {
        m_edges.reserve(16);
    }
    
    void push_back(Edge* edge) {
        assert(edge != NULL);
        assert(checkEdge(edge));
        m_edges.push_back(edge);
    }
    
    bool empty() const {
        return m_edges.empty();
    }
    
    size_t size() const {
        return m_edges.size();
    }
    
    Edge* front() const {
        return m_edges.front();
    }
    
    Edge* back() const {
        return m_edges.back();
    }
    
    iterator begin() {
        return m_edges.begin();
    }
    
    iterator end() {
        return m_edges.end();
    }
    
    const_iterator begin() const {
        return m_edges.begin();
    }

    const_iterator end() const {
        return m_edges.end();
    }
    
    void clear() {
        m_edges.clear();
    }
private:
    bool checkEdge(Edge* edge) const {
        if (m_edges.empty())
            return true;
        Edge* last = m_edges.back();
        return last->firstVertex() == edge->secondVertex();
    }
};

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addPoints(const typename V::List& points) {
    addPoints(points.begin(), points.end());
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addPoints(const typename V::List& points, Callback& callback) {
    addPoints(points.begin(), points.end(), callback);
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
void Polyhedron<T,FP,VP>::addPoint(const V& position) {
    Callback c;
    addPoint(position, c);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addPoint(const V& position, Callback& callback) {
    assert(checkInvariant());
    switch (vertexCount()) {
        case 0:
            addFirstPoint(position);
            m_bounds.min = m_bounds.max = position;
            break;
        case 1:
            addSecondPoint(position);
            m_bounds.mergeWith(position);
            break;
        case 2:
            addThirdPoint(position, callback);
            m_bounds.mergeWith(position);
            break;
        default:
            if (addFurtherPoint(position, callback))
                m_bounds.mergeWith(position);
            break;
    }
    assert(checkInvariant());
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
void Polyhedron<T,FP,VP>::addFirstPoint(const V& position) {
    assert(empty());
    m_vertices.append(new Vertex(position), 1);
}

// Adds the given point to a polyhedron that contains one point.
template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addSecondPoint(const V& position) {
    assert(point());
    
    Vertex* onlyVertex = *m_vertices.begin();
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
template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addThirdPoint(const V& position, Callback& callback) {
    assert(edge());
    
    Vertex* v1 = m_vertices.front();
    Vertex* v2 = v1->next();
    
    if (linearlyDependent(v1->position(), v2->position(), position))
        addPointToEdge(position);
    else
        addPointToPolygon(position, callback);
}

// Adds a colinear third point to a polyhedron that contains one edge.
template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addPointToEdge(const V& position) {
    assert(edge());
    
    Vertex* v1 = m_vertices.front();
    Vertex* v2 = v1->next();
    assert(linearlyDependent(v1->position(), v2->position(), position));
    
    if (!position.containedWithinSegment(v1->position(), v2->position()))
        v2->setPosition(position);
}

// Adds the given point to a polyhedron that is either a polygon or a polyhedron.
template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::addFurtherPoint(const V& position, Callback& callback) {
    if (faceCount() == 1) {
        return addFurtherPointToPolygon(position, callback);
    } else {
        addFurtherPointToPolyhedron(position, callback);
        return true;
    }
}

//Adds the given point to a polygon. The result is either a differen polygon if the
// given point is coplanar to the already existing polygon, or a polyhedron if the
// given point is not coplanar.
template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::addFurtherPointToPolygon(const V& position, Callback& callback) {
    Face* face = *m_faces.begin();
    const Math::PointStatus::Type status = face->pointStatus(position);
    switch (status) {
        case Math::PointStatus::PSInside:
            addPointToPolygon(position, callback);
            return true;
        case Math::PointStatus::PSAbove:
            face->flip();
        case Math::PointStatus::PSBelow:
            return makePolyhedron(position, callback);
    }
    // will never be reached
    return true;
}

// Adds the given coplanar point to a polyhedron that is a polygon or an edge.
template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addPointToPolygon(const V& position, Callback& callback) {
    typename V::List positions;
    positions.reserve(vertexCount() + 1);
    V::toList(m_vertices.begin(), m_vertices.end(), GetVertexPosition(), positions);
    positions.push_back(position);
    
    positions = convexHull2D<T>(positions);
    clear();
    makePolygon(positions, callback);
}

// Creates a new polygon from the given set of coplanar points. Assumes that
// this polyhedron is empty and that the given point list contains at least three
// non-colinear points.
template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::makePolygon(const typename V::List& positions, Callback& callback) {
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
    callback.faceWasCreated(f);
    m_faces.append(f, 1);
}

// Converts a coplanar polyhedron into a non-coplanar one by adding the given
// point, which is assumed to be non-coplanar to the points in this polyhedron.
template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::makePolyhedron(const V& position, Callback& callback) {
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
void Polyhedron<T,FP,VP>::addFurtherPointToPolyhedron(const V& position, Callback& callback) {
    assert(polyhedron());
    const Seam seam = createSeam(SplitByVisibilityCriterion(position));
    if (!seam.empty()) {
        split(seam, callback);
        addPointToPolyhedron(position, seam, callback);
    }
}

// Adds the given point to this polyhedron by weaving a cap over the given seam.
// Assumes that this polyhedron has been split by the given seam.
template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::addPointToPolyhedron(const V& position, const Seam& seam, Callback& callback) {
    assert(!seam.empty());

    Face* remainingFace = m_faces.front();
    typename V::List vertices;
    vertices.reserve(remainingFace->vertexCount());
    remainingFace->getVertexPositions(std::back_inserter(vertices));

    Vertex* newVertex = weaveCap(seam, position, callback);
    cleanupAfterVertexMove(newVertex, callback);
    if (faceCount() < 4) {
        // If this polyhedron was a polygon, and the added point was too close to it, the cleanup
        // may merge some of the newly added faces, and the result is an invalid polyhedron.
        // We recreate the original polygon.
        
        clear();
        addPoints(vertices.begin(), vertices.end(), callback);
        
        return false;
    }
    return true;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Seam Polyhedron<T,FP,VP>::createSeam(const SplittingCriterion& criterion) {
    Seam seam;
    
    Edge* first = criterion.findFirstSplittingEdge(m_edges);
    if (first != NULL) {
        Edge* current = first;
        do {
            assert(current != NULL);
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
    
    // First, unset the second half edge of every seam edge.
    // Thereby remember the second half edge of the first seam edge.
    HalfEdge* first = seam.front()->secondEdge();
    typename Seam::const_iterator it, end;
    for (it = seam.begin(), end = seam.end(); it != end; ++it) {
        Edge* edge = *it;
        edge->setFirstAsLeaving();
        edge->unsetSecondEdge();
    }
    
    // Now we must delete all the faces, edges, and vertices which are above the seam.
    // The first half edge we remembered above is our entry point into that portion of the polyhedron.
    // We must remember which faces we have already visited to stop the recursion.
    FaceSet faceSet;
    VertexList verticesToDelete;
    deleteFaces(first, faceSet, verticesToDelete, callback);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::deleteFaces(HalfEdge* first, FaceSet& visitedFaces, VertexList& verticesToDelete, Callback& callback) {
    Face* face = first->face();
    if (!visitedFaces.insert(face).second)
        return;
    
    HalfEdge* current = first;
    do {
        Edge* edge = current->edge();
        if (edge != NULL) {
            if (edge->fullySpecified())
                deleteFaces(edge->twin(current), visitedFaces, verticesToDelete, callback);
            
            if (edge->fullySpecified()) {
                edge->makeSecondEdge(current);
                edge->unsetSecondEdge();
            } else {
                current->setEdge(NULL);
                m_edges.remove(edge);
                delete edge;
            }
        }
        Vertex* origin = current->origin();
        if (origin->leaving() == current) {
            m_vertices.remove(origin);
            verticesToDelete.append(origin, 1);
        }
        current = current->next();
    } while (current != first);
    
    callback.faceWillBeDeleted(face);
    m_faces.remove(face);
    delete face;
}

// Weaves a new cap onto the given seam edges. The new cap will be a single polygon, so we assume that all seam vertices lie
// on a plane.
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::weaveCap(const Seam& seam, Callback& callback) {
    assert(seam.size() >= 3);

    HalfEdgeList boundary;
    typename Seam::const_iterator it, end;
    for (it = seam.begin(), end = seam.end(); it != end; ++it) {
        Edge* currentEdge = *it;
        assert(!currentEdge->fullySpecified());
        
        Vertex* origin = currentEdge->secondVertex();
        HalfEdge* boundaryEdge = new HalfEdge(origin);
        boundary.append(boundaryEdge, 1);
        currentEdge->setSecondEdge(boundaryEdge);
    }

    Face* face = new Face(boundary);
    callback.faceWasCreated(face);
    m_faces.append(face, 1);
    return face;
}

// Weaves a new cap onto the given seam edges. The new cap will form a triangle fan (actually a cone) with a new vertex
// at the location of the given point being shared by all the newly created triangles.
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::weaveCap(const Seam& seam, const V& position, Callback& callback) {
    assert(seam.size() >= 3);
    
    Vertex* top = new Vertex(position);
    
    HalfEdge* first = NULL;
    HalfEdge* last = NULL;
    
    typename Seam::const_iterator it, end;
    for (it = seam.begin(), end = seam.end(); it != end; ++it) {
        Edge* edge = *it;
        assert(!edge->fullySpecified());
        
        Vertex* v1 = edge->secondVertex();
        Vertex* v2 = edge->firstVertex();
        
        HalfEdge* h1 = new HalfEdge(top);
        HalfEdge* h2 = new HalfEdge(v1);
        HalfEdge* h3 = new HalfEdge(v2);
        
        m_faces.append(createCapTriangle(h1, h2, h3, callback), 1);
        
        if (last != NULL)
            m_edges.append(new Edge(h1, last), 1);
        edge->setSecondEdge(h2);
        
        if (first == NULL)
            first = h1;
        last = h3;
    }
    
    m_edges.append(new Edge(first, last), 1);
    m_vertices.append(top, 1);
    
    return top;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::createCapTriangle(HalfEdge* h1, HalfEdge* h2, HalfEdge* h3, Callback& callback) const {
    HalfEdgeList boundary;
    boundary.append(h1, 1);
    boundary.append(h2, 1);
    boundary.append(h3, 1);
    
    Face* f = new Face(boundary);
    callback.faceWasCreated(f);
    return f;
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
        typename EdgeList::iterator it, end;
        for (it = edges.begin(), end = edges.end(); it != end; ++it) {
            Edge* edge = *it;
            const MatchResult result = matches(edge);
            switch (result) {
                case MatchResult_Second:
                    edge->flip();
                case MatchResult_First:
                    return edge;
                case MatchResult_Both:
                case MatchResult_Neither:
                    break;
                switchDefault()
            }
        }
        return NULL;
    }
    
    // finds the next seam edge in counter clockwise orientation
    Edge* findNextSplittingEdge(Edge* last) const {
        assert(last != NULL);
        
        HalfEdge* halfEdge = last->firstEdge()->previous();
        Edge* next = halfEdge->edge();
        
        MatchResult result = matches(next);
        while (result != MatchResult_First && result != MatchResult_Second && next != last) {
            halfEdge = halfEdge->twin()->previous();
            next = halfEdge->edge();
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

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::SplitByVisibilityCriterion : public Polyhedron<T,FP,VP>::SplittingCriterion {
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

#endif
