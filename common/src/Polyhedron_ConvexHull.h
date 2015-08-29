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

template <typename T> template <typename I>
void Polyhedron<T>::addPoints(I cur, I end) {
    Callback c;
    while (cur != end)
        addPoint(*cur++, c);
}

template <typename T> template <typename I, typename C>
void Polyhedron<T>::addPoints(I cur, I end, C& callback) {
    while (cur != end)
        addPoint(*cur++, callback);
}

template <typename T>
void Polyhedron<T>::addPoint(const V& position) {
    Callback c;
    addPoint(position, c);
}

template <typename T> template <typename C>
void Polyhedron<T>::addPoint(const V& position, C& callback) {
    assert(checkInvariant());
    switch (vertexCount()) {
        case 0:
            addFirstPoint(position);
            break;
        case 1:
            addSecondPoint(position);
            break;
        case 2:
            addThirdPoint(position, callback);
            break;
        default:
            addFurtherPoint(position, callback);
            break;
    }
    assert(checkInvariant());
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
template <typename T> template <typename C>
void Polyhedron<T>::addThirdPoint(const V& position, C& callback) {
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
template <typename T>
void Polyhedron<T>::addPointToEdge(const V& position) {
    assert(edge());
    
    typename VertexList::iterator it = m_vertices.begin();
    Vertex* v1 = *it++;
    Vertex* v2 = *it++;
    assert(linearlyDependent(v1->position(), v2->position(), position));
    
    if (!position.containedWithinSegment(v1->position(), v2->position()))
        v2->setPosition(position);
}

// Adds the given point to a polyhedron that is either a polygon or a polyhedron.
template <typename T> template <typename C>
void Polyhedron<T>::addFurtherPoint(const V& position, C& callback) {
    if (faceCount() == 1)
        addFurtherPointToPolygon(position, callback);
    else
        addFurtherPointToPolyhedron(position, callback);
}

//Adds the given point to a polygon. The result is either a differen polygon if the
// given point is coplanar to the already existing polygon, or a polyhedron if the
// given point is not coplanar.
template <typename T> template <typename C>
void Polyhedron<T>::addFurtherPointToPolygon(const V& position, C& callback) {
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
template <typename T> template <typename C>
void Polyhedron<T>::addPointToPolygon(const V& position, C& callback) {
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
template <typename T> template <typename C>
void Polyhedron<T>::makePolygon(const typename V::List& positions, C& callback) {
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
template <typename T> template <typename C>
void Polyhedron<T>::makePolyhedron(const V& position, C& callback) {
    assert(polygon());
    
    Seam seam;
    Face* face = *m_faces.begin();
    const HalfEdgeList& boundary = face->boundary();
    typename HalfEdgeList::const_iterator hIt, hEnd;
    for (hIt = boundary.begin(), hEnd = boundary.end(); hIt != hEnd; ++hIt) {
        const HalfEdge* h = *hIt;
        Edge* e = h->edge();
        seam.push_front(e); // ensure that the seam is in CCW order
    }
    
    addPointToPolyhedron(position, seam, callback);
}

// Adds the given point to this polyhedron.
template <typename T> template <typename C>
void Polyhedron<T>::addFurtherPointToPolyhedron(const V& position, C& callback) {
    assert(polyhedron());
    const Seam seam = split(SplitByVisibilityCriterion(position), callback);
    if (!seam.empty())
        addPointToPolyhedron(position, seam, callback);
}

// Adds the given point to this polyhedron by weaving a cap over the given seam.
// Assumes that this polyhedron has been split by the given seam.
template <typename T> template <typename C>
void Polyhedron<T>::addPointToPolyhedron(const V& position, const Seam& seam, C& callback) {
    assert(!seam.empty());
    Vertex* newVertex = weaveCap(seam, position, callback);
    cleanupAfterVertexMove(newVertex, callback);
    assert(checkInvariant() && closed());
}

// Splits this polyhedron along the edges where one of the adjacent faces matches the given
// criterion and the other adjacent face does not. It deletes all faces, edges, and vertices
// which do not match the given criterion, and returns the delimiting edges as the seam.
// The delimiting edges all have the remaining half edge as their first edge, and the second
// edge, which has been deleted by the split, is NULL.
template <typename T> template <typename C>
typename Polyhedron<T>::Seam Polyhedron<T>::split(const SplittingCriterion& criterion, C& callback) {
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
    
    typename VertexList::iterator vertexIt;
    typename EdgeList::iterator edgeIt;
    typename FaceList::iterator faceIt;
    
    // Now handle the remaining faces, edge, and vertices by sorting them into the correct polyhedra.
    vertexIt = m_vertices.begin();
    while (vertexIt != m_vertices.end()) {
        Vertex* vertex = *vertexIt;
        HalfEdge* edge = vertex->leaving();
        assert(edge != NULL);
        
        // As we have already handled the shared vertices, it holds that for each remaining vertex,
        // either all adjacent faces match or not. There are no mixed vertices anymore at this point.
        
        Face* face = edge->face();
        assert(face != NULL);
        
        if (!criterion.matches(face)) {
            vertexIt = m_vertices.erase(vertexIt);
            vertices.append(vertex, 1);
        } else {
            ++vertexIt;
        }
    }
    
    edgeIt = m_edges.begin();
    while (edgeIt != m_edges.end()) {
        Edge* edge = *edgeIt;
        Face* face = edge->firstFace();
        assert(face != NULL);
        
        // There are no mixed edges at this point anymore either, and all remaining edges have at least
        // one edge, and that is their first edge.
        
        if (!criterion.matches(face)) {
            assert(edge->secondFace() == NULL || !criterion.matches(edge->secondFace()));
            edgeIt = m_edges.erase(edgeIt);
            edges.append(edge, 1);
        } else {
            ++edgeIt;
        }
    }
    
    faceIt = m_faces.begin();
    while (faceIt != m_faces.end()) {
        Face* face = *faceIt;
        
        if (!criterion.matches(face)) {
            faceIt = m_faces.erase(faceIt);
            faces.append(face, 1);
            callback.faceWillBeDeleted(face);
        } else {
            ++faceIt;
        }
    }
    
    faces.deleteAll();
    edges.deleteAll();
    vertices.deleteAll();
    
    assert(checkConvex());
    
    return seam;
}


// Weaves a new cap onto the given seam edges. The new cap will form a triangle fan (actually a cone) with a new vertex
// at the location of the given point being shared by all the newly created triangles.
template <typename T> template <typename C>
typename Polyhedron<T>::Vertex* Polyhedron<T>::weaveCap(const Seam& seam, const V& position, C& callback) {
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

template <typename T> template <typename C>
typename Polyhedron<T>::Face* Polyhedron<T>::createCapTriangle(HalfEdge* h1, HalfEdge* h2, HalfEdge* h3, C& callback) const {
    HalfEdgeList boundary;
    boundary.append(h1, 1);
    boundary.append(h2, 1);
    boundary.append(h3, 1);
    
    Face* f = new Face(boundary);
    callback.faceWasCreated(f);
    return f;
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

#endif
