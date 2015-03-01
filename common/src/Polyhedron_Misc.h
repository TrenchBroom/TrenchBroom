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
    typename VertexList::const_iterator it, end;
    for (it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        const Vertex* v = *it;
        positions.push_back(v->position());
    }
    return positions;
}

template <typename T>
bool Polyhedron<T>::hasVertex(const V& position) const {
    return findVertexByPosition(position) != NULL;
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
bool Polyhedron<T>::hasEdge(const V& pos1, const V& pos2) const {
    return findEdgeByPositions(pos1, pos2) != NULL;
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
bool Polyhedron<T>::hasFace(const typename V::List& positions) const {
    return findFaceByPositions(positions) != NULL;
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
    typename VertexList::const_iterator it, end;
    for (it = m_vertices.begin(), end = m_vertices.end(); it != end; ++it) {
        Vertex* vertex = *it;
        if (position.equals(vertex->position(), epsilon))
            return vertex;
    }
    return NULL;
}

template <typename T>
typename Polyhedron<T>::Edge* Polyhedron<T>::findEdgeByPositions(const V& pos1, const V& pos2, const T epsilon) const {
    typename EdgeList::const_iterator it, end;
    for (it = m_edges.begin(), end = m_edges.end(); it != end; ++it) {
        Edge* edge = *it;
        if (edge->hasPositions(pos1, pos2, epsilon))
            return edge;
    }
    return NULL;
}

template <typename T>
typename Polyhedron<T>::Face* Polyhedron<T>::findFaceByPositions(const typename V::List& positions, const T epsilon) const {
    typename FaceList::const_iterator it, end;
    for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
        Face* face = *it;
        if (face->hasPositions(positions, epsilon))
            return face;
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
    if (!checkConvex())
        return false;
    if (polyhedron() && !checkClosed())
        return false;
    if (polyhedron() && !checkNoDegenerateFaces())
        return false;
    if (polyhedron() && !checkNoCoplanarFaces())
        return false;
    return true;
}

template <typename T>
bool Polyhedron<T>::checkConvex() const {
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

template <typename T>
bool Polyhedron<T>::checkClosed() const {
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

template <typename T>
bool Polyhedron<T>::checkNoCoplanarFaces() const {
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

template <typename T>
bool Polyhedron<T>::checkNoDegenerateFaces() const {
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

#endif
