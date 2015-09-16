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

template <typename T, typename FP>
class Polyhedron<T,FP>::Seam {
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
private:
    bool checkEdge(Edge* edge) const {
        if (m_edges.empty())
            return true;
        Edge* last = m_edges.back();
        return last->firstVertex() == edge->secondVertex();
    }
};

template <typename T, typename FP> template <typename I>
void Polyhedron<T,FP>::addPoints(I cur, I end) {
    Callback c;
    while (cur != end)
        addPoint(*cur++, c);
}

template <typename T, typename FP> template <typename I, typename C>
void Polyhedron<T,FP>::addPoints(I cur, I end, C& callback) {
    while (cur != end)
        addPoint(*cur++, callback);
}

template <typename T, typename FP>
void Polyhedron<T,FP>::addPoint(const V& position) {
    Callback c;
    addPoint(position, c);
}

template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::addPoint(const V& position, C& callback) {
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
            addFurtherPoint(position, callback);
            m_bounds.mergeWith(position);
            break;
    }
    assert(checkInvariant());
}

// Adds the given point to an empty polyhedron.
template <typename T, typename FP>
void Polyhedron<T,FP>::addFirstPoint(const V& position) {
    assert(empty());
    m_vertices.append(new Vertex(position), 1);
}

// Adds the given point to a polyhedron that contains one point.
template <typename T, typename FP>
void Polyhedron<T,FP>::addSecondPoint(const V& position) {
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
template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::addThirdPoint(const V& position, C& callback) {
    assert(edge());
    
    typename VertexList::iterator it = m_vertices.begin();
    const Vertex* v1 = *it++;
    const Vertex* v2 = *it++;
    
    if (linearlyDependent(v1->position(), v2->position(), position))
        addPointToEdge(position);
    else
        addPointToPolygon(position, callback);
}

// Adds a colinear third point to a polyhedron that contains one edge.
template <typename T, typename FP>
void Polyhedron<T,FP>::addPointToEdge(const V& position) {
    assert(edge());
    
    typename VertexList::iterator it = m_vertices.begin();
    Vertex* v1 = *it++;
    Vertex* v2 = *it++;
    assert(linearlyDependent(v1->position(), v2->position(), position));
    
    if (!position.containedWithinSegment(v1->position(), v2->position()))
        v2->setPosition(position);
}

// Adds the given point to a polyhedron that is either a polygon or a polyhedron.
template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::addFurtherPoint(const V& position, C& callback) {
    if (faceCount() == 1)
        addFurtherPointToPolygon(position, callback);
    else
        addFurtherPointToPolyhedron(position, callback);
}

//Adds the given point to a polygon. The result is either a differen polygon if the
// given point is coplanar to the already existing polygon, or a polyhedron if the
// given point is not coplanar.
template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::addFurtherPointToPolygon(const V& position, C& callback) {
    Face* face = *m_faces.begin();
    const Math::PointStatus::Type status = face->pointStatus(position);
    switch (status) {
        case Math::PointStatus::PSInside:
            addPointToPolygon(position, callback);
            break;
        case Math::PointStatus::PSAbove:
            face->flip();
        case Math::PointStatus::PSBelow:
            makePolyhedron(position, callback);
            break;
    }
}

// Adds the given coplanar point to a polyhedron that is a polygon or an edge.
template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::addPointToPolygon(const V& position, C& callback) {
    typename V::List positions;
    positions.reserve(vertexCount() + 1);
    vertexPositions(positions);
    positions.push_back(position);
    
    positions = convexHull2D<T>(positions);
    clear();
    makePolygon(positions, callback);
}

// Creates a new polygon from the given set of coplanar points. Assumes that
// this polyhedron is empty and that the given point list contains at least three
// non-colinear points.
template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::makePolygon(const typename V::List& positions, C& callback) {
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
template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::makePolyhedron(const V& position, C& callback) {
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

    addPointToPolyhedron(position, seam, callback);
}

// Adds the given point to this polyhedron.
template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::addFurtherPointToPolyhedron(const V& position, C& callback) {
    assert(polyhedron());
    const Seam seam = createSeam(SplitByVisibilityCriterion(position));
    split(seam, callback);
    if (!seam.empty())
        addPointToPolyhedron(position, seam, callback);
}

// Adds the given point to this polyhedron by weaving a cap over the given seam.
// Assumes that this polyhedron has been split by the given seam.
template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::addPointToPolyhedron(const V& position, const Seam& seam, C& callback) {
    assert(!seam.empty());
    Vertex* newVertex = weaveCap(seam, position, callback);
    cleanupAfterVertexMove(newVertex, callback);
    assert(checkInvariant() && closed());
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::Seam Polyhedron<T,FP>::createSeam(const SplittingCriterion& criterion) {
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
template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::split(const Seam& seam, C& callback) {
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
    
    assert(checkConvex());
}

template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::deleteFaces(HalfEdge* first, FaceSet& visitedFaces, VertexList& verticesToDelete, C& callback) {
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
template <typename T, typename FP> template <typename C>
void Polyhedron<T,FP>::weaveCap(const Seam& seam, C& callback) {
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
}

// Weaves a new cap onto the given seam edges. The new cap will form a triangle fan (actually a cone) with a new vertex
// at the location of the given point being shared by all the newly created triangles.
template <typename T, typename FP> template <typename C>
typename Polyhedron<T,FP>::Vertex* Polyhedron<T,FP>::weaveCap(const Seam& seam, const V& position, C& callback) {
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

template <typename T, typename FP> template <typename C>
typename Polyhedron<T,FP>::Face* Polyhedron<T,FP>::createCapTriangle(HalfEdge* h1, HalfEdge* h2, HalfEdge* h3, C& callback) const {
    HalfEdgeList boundary;
    boundary.append(h1, 1);
    boundary.append(h2, 1);
    boundary.append(h3, 1);
    
    Face* f = new Face(boundary);
    callback.faceWasCreated(f);
    return f;
}

template <typename T, typename FP>
class Polyhedron<T,FP>::SplittingCriterion {
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

template <typename T, typename FP>
class Polyhedron<T,FP>::SplitByVisibilityCriterion : public Polyhedron<T,FP>::SplittingCriterion {
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
