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

#ifndef TrenchBroom_Polyhedron_Misc_h
#define TrenchBroom_Polyhedron_Misc_h

#include <map>

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::VertexDistanceCmp {
private:
    V m_anchor;
public:
    VertexDistanceCmp(const V& anchor) :
    m_anchor(anchor) {}
    
    bool operator()(const Vertex* lhs, const Vertex* rhs) const {
        const T lDist = m_anchor.squaredDistanceTo(lhs->position());
        const T rDist = m_anchor.squaredDistanceTo(rhs->position());
        if (lDist < rDist)
            return true;
        if (lDist > rDist)
            return false;
        return lhs->position().compare(rhs->position()) < 0;
    }
};


template <typename T, typename FP, typename VP>
const typename Polyhedron<T,FP,VP>::V& Polyhedron<T,FP,VP>::GetVertexPosition::operator()(const Vertex* vertex) const {
    return vertex->position();
}

template <typename T, typename FP, typename VP>
const typename Polyhedron<T,FP,VP>::V& Polyhedron<T,FP,VP>::GetVertexPosition::operator()(const HalfEdge* halfEdge) const {
    return halfEdge->origin()->position();
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Callback::~Callback() {}

template <typename T, typename FP, typename VP>
Plane<T,3> Polyhedron<T,FP,VP>::Callback::plane(const Face* face) const {
    const HalfEdgeList& boundary = face->boundary();
    assert(boundary.size() >= 3);
    
    const HalfEdge* e1 = boundary.front();
    const HalfEdge* e2 = e1->next();
    const HalfEdge* e3 = e2->next();
    
    const V& p1 = e1->origin()->position();
    const V& p2 = e2->origin()->position();
    const V& p3 = e3->origin()->position();

    Plane<T,3> plane;
    assertResult(setPlanePoints(plane, p2, p1, p3));
    return plane;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::faceWasCreated(Face* face) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::faceWillBeDeleted(Face* face) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::faceDidChange(Face* face) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::faceWasSplit(Face* original, Face* clone) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::facesWillBeMerged(Face* remaining, Face* toDelete) {}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron() {
    updateBounds();
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4) {
    Callback c;
    addPoints(p1, p2, p3, p4, c);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4, Callback& callback) {
    addPoints(p1, p2, p3, p4, callback);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const BBox<T,3>& bounds) {
    Callback c;
    setBounds(bounds, c);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const BBox<T,3>& bounds, Callback& callback) {
    setBounds(bounds, callback);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const typename V::List& positions) {
    Callback c;
    addPoints(positions.begin(), positions.end(), c);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const typename V::List& positions, Callback& callback) {
    addPoints(positions.begin(), positions.end(), callback);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const typename V::Set& positions) {
    Callback c;
    addPoints(positions.begin(), positions.end(), c);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const typename V::Set& positions, Callback& callback) {
    addPoints(positions.begin(), positions.end(), callback);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addPoints(const V& p1, const V& p2, const V& p3, const V& p4, Callback& callback) {
    addPoint(p1, callback);
    addPoint(p2, callback);
    addPoint(p3, callback);
    addPoint(p4, callback);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::setBounds(const BBox<T,3>& bounds, Callback& callback) {
    if (bounds.min == bounds.max) {
        addPoint(bounds.min);
        return;
    }
    
    // Explicitely create the polyhedron for better performance when building brushes.
    
    const V p1(bounds.min.x(), bounds.min.y(), bounds.min.z());
    const V p2(bounds.min.x(), bounds.min.y(), bounds.max.z());
    const V p3(bounds.min.x(), bounds.max.y(), bounds.min.z());
    const V p4(bounds.min.x(), bounds.max.y(), bounds.max.z());
    const V p5(bounds.max.x(), bounds.min.y(), bounds.min.z());
    const V p6(bounds.max.x(), bounds.min.y(), bounds.max.z());
    const V p7(bounds.max.x(), bounds.max.y(), bounds.min.z());
    const V p8(bounds.max.x(), bounds.max.y(), bounds.max.z());
    
    Vertex* v1 = new Vertex(p1);
    Vertex* v2 = new Vertex(p2);
    Vertex* v3 = new Vertex(p3);
    Vertex* v4 = new Vertex(p4);
    Vertex* v5 = new Vertex(p5);
    Vertex* v6 = new Vertex(p6);
    Vertex* v7 = new Vertex(p7);
    Vertex* v8 = new Vertex(p8);

    m_vertices.append(v1, 1);
    m_vertices.append(v2, 1);
    m_vertices.append(v3, 1);
    m_vertices.append(v4, 1);
    m_vertices.append(v5, 1);
    m_vertices.append(v6, 1);
    m_vertices.append(v7, 1);
    m_vertices.append(v8, 1);
    
    // Front face
    HalfEdge* f1h1 = new HalfEdge(v1);
    HalfEdge* f1h2 = new HalfEdge(v5);
    HalfEdge* f1h3 = new HalfEdge(v6);
    HalfEdge* f1h4 = new HalfEdge(v2);
    HalfEdgeList f1b;
    f1b.append(f1h1, 1);
    f1b.append(f1h2, 1);
    f1b.append(f1h3, 1);
    f1b.append(f1h4, 1);
    m_faces.append(new Face(f1b), 1);
    
    // Left face
    HalfEdge* f2h1 = new HalfEdge(v1);
    HalfEdge* f2h2 = new HalfEdge(v2);
    HalfEdge* f2h3 = new HalfEdge(v4);
    HalfEdge* f2h4 = new HalfEdge(v3);
    HalfEdgeList f2b;
    f2b.append(f2h1, 1);
    f2b.append(f2h2, 1);
    f2b.append(f2h3, 1);
    f2b.append(f2h4, 1);
    m_faces.append(new Face(f2b), 1);
    
    // Bottom face
    HalfEdge* f3h1 = new HalfEdge(v1);
    HalfEdge* f3h2 = new HalfEdge(v3);
    HalfEdge* f3h3 = new HalfEdge(v7);
    HalfEdge* f3h4 = new HalfEdge(v5);
    HalfEdgeList f3b;
    f3b.append(f3h1, 1);
    f3b.append(f3h2, 1);
    f3b.append(f3h3, 1);
    f3b.append(f3h4, 1);
    m_faces.append(new Face(f3b), 1);
    
    // Top face
    HalfEdge* f4h1 = new HalfEdge(v2);
    HalfEdge* f4h2 = new HalfEdge(v6);
    HalfEdge* f4h3 = new HalfEdge(v8);
    HalfEdge* f4h4 = new HalfEdge(v4);
    HalfEdgeList f4b;
    f4b.append(f4h1, 1);
    f4b.append(f4h2, 1);
    f4b.append(f4h3, 1);
    f4b.append(f4h4, 1);
    m_faces.append(new Face(f4b), 1);
    
    // Back face
    HalfEdge* f5h1 = new HalfEdge(v3);
    HalfEdge* f5h2 = new HalfEdge(v4);
    HalfEdge* f5h3 = new HalfEdge(v8);
    HalfEdge* f5h4 = new HalfEdge(v7);
    HalfEdgeList f5b;
    f5b.append(f5h1, 1);
    f5b.append(f5h2, 1);
    f5b.append(f5h3, 1);
    f5b.append(f5h4, 1);
    m_faces.append(new Face(f5b), 1);
    
    // Right face
    HalfEdge* f6h1 = new HalfEdge(v5);
    HalfEdge* f6h2 = new HalfEdge(v7);
    HalfEdge* f6h3 = new HalfEdge(v8);
    HalfEdge* f6h4 = new HalfEdge(v6);
    HalfEdgeList f6b;
    f6b.append(f6h1, 1);
    f6b.append(f6h2, 1);
    f6b.append(f6h3, 1);
    f6b.append(f6h4, 1);
    m_faces.append(new Face(f6b), 1);
    
    m_edges.append(new Edge(f1h4, f2h1), 1); // v1, v2
    m_edges.append(new Edge(f2h4, f3h1), 1); // v1, v3
    m_edges.append(new Edge(f1h1, f3h4), 1); // v1, v5
    m_edges.append(new Edge(f2h2, f4h4), 1); // v2, v4
    m_edges.append(new Edge(f4h1, f1h3), 1); // v2, v6
    m_edges.append(new Edge(f2h3, f5h1), 1); // v3, v4
    m_edges.append(new Edge(f3h2, f5h4), 1); // v3, v7
    m_edges.append(new Edge(f4h3, f5h2), 1); // v4, v8
    m_edges.append(new Edge(f1h2, f6h4), 1); // v5, v6
    m_edges.append(new Edge(f6h1, f3h3), 1); // v5, v7
    m_edges.append(new Edge(f6h3, f4h2), 1); // v6, v8
    m_edges.append(new Edge(f6h2, f5h3), 1); // v7, v8
}

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Copy {
private:
    typedef std::map<const Vertex*, Vertex*> VertexMap;
    typedef typename VertexMap::value_type VertexMapEntry;

    typedef std::map<const HalfEdge*, HalfEdge*> HalfEdgeMap;
    typedef typename HalfEdgeMap::value_type HalfEdgeMapEntry;

    VertexMap m_vertexMap;
    HalfEdgeMap m_halfEdgeMap;
    
    VertexList m_vertices;
    EdgeList m_edges;
    FaceList m_faces;
    Polyhedron& m_destination;
public:
    Copy(const FaceList& originalFaces, const EdgeList& originalEdges, Polyhedron& destination) :
    m_destination(destination) {
        copyFaces(originalFaces);
        copyEdges(originalEdges);
        swapContents();
    }
private:
    void copyFaces(const FaceList& originalFaces) {
        typename FaceList::const_iterator fIt, fEnd;
        for (fIt = originalFaces.begin(), fEnd = originalFaces.end(); fIt != fEnd; ++fIt) {
            const Face* originalFace = *fIt;
            copyFace(originalFace);
        }
    }
    
    void copyFace(const Face* originalFace) {
        HalfEdgeList myBoundary;

        typename HalfEdgeList::const_iterator hIt, hEnd;
        for (hIt = originalFace->m_boundary.begin(), hEnd = originalFace->m_boundary.end(); hIt != hEnd; ++hIt) {
            const HalfEdge* originalHalfEdge = *hIt;
            myBoundary.append(copyHalfEdge(originalHalfEdge), 1);
        }
        
        Face* copy = new Face(myBoundary);
        m_faces.append(copy, 1);
    }
    
    HalfEdge* copyHalfEdge(const HalfEdge* original) {
        const Vertex* originalOrigin = original->origin();
        
        Vertex* myOrigin = findOrCopyVertex(originalOrigin);
        HalfEdge* copy = new HalfEdge(myOrigin);
        m_halfEdgeMap.insert(std::make_pair(original, copy));
        return copy;
    }

    Vertex* findOrCopyVertex(const Vertex* original) {
        typedef std::pair<bool, typename VertexMap::iterator> InsertPos;
        
        InsertPos insertPos = MapUtils::findInsertPos(m_vertexMap, original);
        if (!insertPos.first) {
            Vertex* copy = new Vertex(original->position());
            m_vertices.append(copy, 1);
            m_vertexMap.insert(insertPos.second, std::make_pair(original, copy));
            return copy;
        }
        return insertPos.second->second;
    }
    
    void copyEdges(const EdgeList& originalEdges) {
        typename EdgeList::const_iterator eIt, eEnd;
        for (eIt = originalEdges.begin(), eEnd = originalEdges.end(); eIt != eEnd; ++eIt) {
            const Edge* originalEdge = *eIt;
            Edge* copy = copyEdge(originalEdge);
            m_edges.append(copy, 1);
        }
    }
    
    Edge* copyEdge(const Edge* original) const {
        HalfEdge* myFirst = findHalfEdge(original->firstEdge());
        if (!original->fullySpecified())
            return new Edge(myFirst);

        HalfEdge* mySecond = findHalfEdge(original->secondEdge());
        return new Edge(myFirst, mySecond);
    }
    
    HalfEdge* findHalfEdge(const HalfEdge* original) const {
        HalfEdge* result = MapUtils::find(m_halfEdgeMap, original, static_cast<HalfEdge*>(NULL));
        assert(result != NULL);
        return result;
    }

    void swapContents() {
        using std::swap;
        swap(m_vertices, m_destination.m_vertices);
        swap(m_edges, m_destination.m_edges);
        swap(m_faces, m_destination.m_faces);
        m_destination.updateBounds();
    }
};

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const Polyhedron<T,FP,VP>& other) {
    Copy copy(other.faces(), other.edges(), *this);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::~Polyhedron() {
    clear();
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>& Polyhedron<T,FP,VP>::operator=(Polyhedron<T,FP,VP> other) {
    swap(*this, other);
    return *this;
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::vertexCount() const {
    return m_vertices.size();
}

template <typename T, typename FP, typename VP>
const typename Polyhedron<T,FP,VP>::VertexList& Polyhedron<T,FP,VP>::vertices() const {
    return m_vertices;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::hasVertex(const V& position, const T epsilon) const {
    return findVertexByPosition(position, epsilon) != NULL;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::hasVertices(const typename V::List& positions, const T epsilon) const {
    if (positions.size() != vertexCount())
        return false;
    typename V::List::const_iterator it, end;
    for (it = positions.begin(), end = positions.end(); it != end; ++it) {
        if (!hasVertex(*it, epsilon))
            return false;
    }
    return true;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::printVertices() const {
    const Vertex* firstVertex = m_vertices.front();
    const Vertex* currentVertex = firstVertex;
    do {
        std::cout << currentVertex->position().asString() << std::endl;
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
}


template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::edgeCount() const {
    return m_edges.size();
}

template <typename T, typename FP, typename VP>
const typename Polyhedron<T,FP,VP>::EdgeList& Polyhedron<T,FP,VP>::edges() const {
    return m_edges;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::hasEdge(const V& pos1, const V& pos2, const T epsilon) const {
    return findEdgeByPositions(pos1, pos2, epsilon) != NULL;
}

template <typename T, typename FP, typename VP>
size_t Polyhedron<T,FP,VP>::faceCount() const {
    return m_faces.size();
}

template <typename T, typename FP, typename VP>
const typename Polyhedron<T,FP,VP>::FaceList& Polyhedron<T,FP,VP>::faces() const {
    return m_faces;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::hasFace(const typename V::List& positions, const T epsilon) const {
    return findFaceByPositions(positions, epsilon) != NULL;
}

template <typename T, typename FP, typename VP>
const BBox<T,3>& Polyhedron<T,FP,VP>::bounds() const {
    return m_bounds;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::empty() const {
    return vertexCount() == 0;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::point() const {
    return vertexCount() == 1;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::edge() const {
    return vertexCount() == 2;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::polygon() const {
    return faceCount() == 1;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::polyhedron() const {
    return faceCount() > 3;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::closed() const {
    return vertexCount() + faceCount() == edgeCount() + 2;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::clear() {
    m_faces.clear();
    m_edges.clear();
    m_vertices.clear();
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::FaceHit::FaceHit(Face* i_face, const T i_distance) : face(i_face), distance(i_distance) {}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::FaceHit::FaceHit() : face(NULL), distance(Math::nan<T>()) {}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::FaceHit::isMatch() const { return face != NULL; }

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::FaceHit Polyhedron<T,FP,VP>::pickFace(const Ray<T,3>& ray) const {
    const Math::Side side = polygon() ? Math::Side_Both : Math::Side_Front;
    Face* firstFace = m_faces.front();
    Face* currentFace = firstFace;
    do {
        const T distance = currentFace->intersectWithRay(ray, side);
        if (!Math::isnan(distance))
            return FaceHit(currentFace, distance);
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return FaceHit();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::findVertexByPosition(const V& position, const T epsilon) const {
    Vertex* firstVertex = m_vertices.front();
    Vertex* currentVertex = firstVertex;
    do {
        if (position.equals(currentVertex->position(), epsilon))
            return currentVertex;
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return NULL;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::findClosestVertex(const V& position) const {
    T closestDistance = std::numeric_limits<T>::max();
    Vertex* closestVertex = NULL;
    
    Vertex* firstVertex = m_vertices.front();
    Vertex* currentVertex = firstVertex;
    do {
        const T distance = position.squaredDistanceTo(currentVertex->position());
        if (distance < closestDistance) {
            closestDistance = distance;
            closestVertex = currentVertex;
        }
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return closestVertex;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::ClosestVertexSet Polyhedron<T,FP,VP>::findClosestVertices(const V& position) const {
    ClosestVertexSet result = ClosestVertexSet(VertexDistanceCmp(position));
    Vertex* firstVertex = m_vertices.front();
    Vertex* currentVertex = firstVertex;
    do {
        result.insert(currentVertex);
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return result;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::findEdgeByPositions(const V& pos1, const V& pos2, const T epsilon) const {
    Edge* firstEdge = m_edges.front();
    Edge* currentEdge = firstEdge;
    do {
        currentEdge = currentEdge->next();
        if (currentEdge->hasPositions(pos1, pos2, epsilon))
            return currentEdge;
    } while (currentEdge != firstEdge);
    return NULL;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::findFaceByPositions(const typename V::List& positions, const T epsilon) const {
    Face* firstFace = m_faces.front();
    Face* currentFace = firstFace;
    do {
        if (currentFace->hasVertexPositions(positions, epsilon))
            return currentFace;
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return NULL;
}

template <typename T, typename FP, typename VP> template <typename O>
void Polyhedron<T,FP,VP>::getVertexPositions(O output) const {
    Vertex* firstVertex = m_vertices.front();
    Vertex* currentVertex = firstVertex;
    do {
        output = currentVertex->position();
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::hasVertex(const Vertex* vertex) const {
    const Vertex* firstVertex = m_vertices.front();
    const Vertex* currentVertex = firstVertex;
    do {
        if (currentVertex == vertex)
            return true;
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::hasEdge(const Edge* edge) const {
    const Edge* firstEdge = m_edges.front();
    const Edge* currentEdge = firstEdge;
    do {
        if (currentEdge == edge)
            return true;
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::hasFace(const Face* face) const {
    const Face* firstFace = m_faces.front();
    const Face* currentFace = firstFace;
    do {
        if (currentFace == face)
            return true;
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkInvariant() const {
    /*
    if (!checkConvex())
        return false;
     */
    if (!checkFaceBoundaries())
        return false;
    if (polyhedron() && !checkClosed())
        return false;
    if (polyhedron() && !checkNoDegenerateFaces())
        return false;
    if (polyhedron() && !checkVertexLeavingEdges())
        return false;
    if (polyhedron() && !checkEdges())
        return false;
    /* This check leads to false positive with almost coplanar faces.
    if (polyhedron() && !checkNoCoplanarFaces())
        return false;
     */
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkFaceBoundaries() const {
    if (m_faces.empty())
        return true;
    Face* first = m_faces.front();
    Face* current = first;
    do {
        if (!current->checkBoundary())
            return false;
        current = current->next();
    } while (current != first);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkConvex() const {
    typename FaceList::const_iterator fIt, fEnd;
    for (fIt = m_faces.begin(), fEnd = m_faces.end(); fIt != fEnd; ++fIt) {
        const Face* face = *fIt;
        typename VertexList::const_iterator vIt, vEnd;
        for (vIt = m_vertices.begin(), vEnd = m_vertices.end(); vIt != vEnd; ++vIt) {
            const Vertex* vertex = *vIt;
            if (face->pointStatus(vertex->position()) == Math::PointStatus::PSAbove)
                return false;
        }
    }
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkClosed() const {
    typename EdgeList::const_iterator eIt, eEnd;
    for (eIt = m_edges.begin(), eEnd = m_edges.end(); eIt != eEnd; ++eIt) {
        const Edge* edge = *eIt;
        if (!edge->fullySpecified())
            return false;
        
        const Face* firstFace = edge->firstFace();
        const Face* secondFace = edge->secondFace();
        
        if (!m_faces.contains(firstFace))
            return false;
        if (!m_faces.contains(secondFace))
            return false;
    }
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkNoCoplanarFaces() const {
    typename EdgeList::const_iterator eIt, eEnd;
    for (eIt = m_edges.begin(), eEnd = m_edges.end(); eIt != eEnd; ++eIt) {
        const Edge* edge = *eIt;
        const Face* firstFace = edge->firstFace();
        const Face* secondFace = edge->secondFace();
        
        if (firstFace == secondFace)
            return false;
        if (firstFace->coplanar(secondFace))
            return false;
    }
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkNoDegenerateFaces() const {
    typename FaceList::const_iterator fIt, fEnd;
    for (fIt = m_faces.begin(), fEnd = m_faces.end(); fIt != fEnd; ++fIt) {
        const Face* face = *fIt;
        if (face->vertexCount() < 3)
            return false;
        
        const HalfEdgeList& boundary = face->boundary();
        typename HalfEdgeList::const_iterator hIt, hEnd;
        for (hIt = boundary.begin(), hEnd = boundary.end(); hIt != hEnd; ++hIt) {
            const HalfEdge* halfEdge = *hIt;
            const Edge* edge = halfEdge->edge();
            
            if (edge == NULL || !edge->fullySpecified())
                return false;
        }
    }
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkVertexLeavingEdges() const {
    const Vertex* firstVertex = m_vertices.front();
    const Vertex* currentVertex = firstVertex;
    do {
        const HalfEdge* leaving = currentVertex->leaving();
        if (leaving == NULL)
            return false;
        if (leaving->origin() != currentVertex)
            return false;
        const Edge* edge = leaving->edge();
        if (edge == NULL)
            return false;
        if (!edge->fullySpecified())
            return false;
        if (!hasEdge(edge))
            return false;
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkEdges() const {
    if (m_edges.empty())
        return true;
    
    Edge* firstEdge = m_edges.front();
    Edge* currentEdge = firstEdge;
    do {
        if (!currentEdge->fullySpecified())
            return false;
        Face* firstFace = currentEdge->firstFace();
        if (firstFace == NULL)
            return false;
        if (!m_faces.contains(firstFace))
            return false;
        
        Face* secondFace = currentEdge->secondFace();
        if (secondFace == NULL)
            return false;
        if (!m_faces.contains(secondFace))
            return false;
        
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return true;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::correctVertexPositions(const size_t decimals, const T epsilon) {
    Vertex* firstVertex = m_vertices.front();
    Vertex* currentVertex = firstVertex;
    do {
        currentVertex->correctPosition(decimals, epsilon);
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::updateBounds() {
    if (m_vertices.size() == 0) {
        m_bounds.min = m_bounds.max = Vec<T,3>::NaN;
    } else {
        Vertex* first = m_vertices.front();
        Vertex* current = first;
        m_bounds.min = m_bounds.max = current->position();
        
        current = current->next();
        while (current != first) {
            m_bounds.mergeWith(current->position());
            current = current->next();
        }
    }
}

#endif
