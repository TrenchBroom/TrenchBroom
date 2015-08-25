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

template <typename T>
Polyhedron<T>::Polyhedron() {}

template <typename T>
Polyhedron<T>::Polyhedron(const V& p1, const V& p2, const V& p3, const V& p4) {
    addPoint(p1);
    addPoint(p2);
    addPoint(p3);
    addPoint(p4);
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

    addPoint(p1);
    addPoint(p2);
    addPoint(p3);
    addPoint(p4);
    addPoint(p5);
    addPoint(p6);
    addPoint(p7);
    addPoint(p8);
}

template <typename T>
Polyhedron<T>::Polyhedron(typename V::List positions) {
    addPoints(positions.begin(), positions.end());
}

template <typename T>
class Polyhedron<T>::Copy {
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
    
    ~Copy() {
        m_faces.deleteAll();
        m_edges.deleteAll();
        m_vertices.deleteAll();
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
        
        Face* copy = m_destination.createFace(myBoundary);
        m_faces.append(copy, 1);
    }
    
    HalfEdge* copyHalfEdge(const HalfEdge* original) {
        const Vertex* originalOrigin = original->origin();
        
        Vertex* myOrigin = findOrCopyVertex(originalOrigin);
        HalfEdge* copy = m_destination.createHalfEdge(myOrigin);
        m_halfEdgeMap.insert(std::make_pair(original, copy));
        return copy;
    }

    Vertex* findOrCopyVertex(const Vertex* original) {
        typedef std::pair<bool, typename VertexMap::iterator> InsertPos;
        
        InsertPos insertPos = MapUtils::findInsertPos(m_vertexMap, original);
        if (!insertPos.first) {
            Vertex* copy = m_destination.createVertex(original->position());
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
            return m_destination.createEdge(myFirst);

        HalfEdge* mySecond = findHalfEdge(original->secondEdge());
        return m_destination.createEdge(myFirst, mySecond);
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
    }
};

template <typename T>
Polyhedron<T>::Polyhedron(const Polyhedron<T>& other) {
    Copy copy(other.faces(), other.edges(), *this);
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
typename Polyhedron<T>::Vertex* Polyhedron<T>::createVertex(const V& position) const {
    return new Vertex(position);
}

template <typename T>
typename Polyhedron<T>::Edge* Polyhedron<T>::createEdge(HalfEdge* first, HalfEdge* second) const {
    return new Edge(first, second);
}

template <typename T>
typename Polyhedron<T>::HalfEdge* Polyhedron<T>::createHalfEdge(Vertex* origin) const {
    return new HalfEdge(origin);
}

template <typename T>
typename Polyhedron<T>::Face* Polyhedron<T>::createFace(const HalfEdgeList& boundary) const {
    return new Face(boundary);
}

template <typename T>
typename Polyhedron<T>::Face* Polyhedron<T>::cloneFace(const Face* original, const HalfEdgeList& boundary) const {
    return new Face(boundary);
}

template <typename T>
Polyhedron<T>& Polyhedron<T>::operator=(Polyhedron<T> other) {
    swap(*this, other);
    return *this;
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
void Polyhedron<T>::clear() {
    m_faces.deleteAll();
    m_edges.deleteAll();
    m_vertices.deleteAll();
}

template <typename T>
struct Polyhedron<T>::FaceHit {
    Face* face;
    T distance;
    
    FaceHit(Face* i_face, const T i_distance) : face(i_face), distance(i_distance) {}
    FaceHit() : face(NULL), distance(Math::nan<T>()) {}
    bool isMatch() const { return face != NULL; }
};

template <typename T>
typename Polyhedron<T>::FaceHit Polyhedron<T>::pickFace(const Ray<T,3>& ray) const {
    const Math::Side side = polygon() ? Math::Side_Both : Math::Side_Front;
    typename FaceList::const_iterator it, end;
    for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
        Face* face = *it;
        const T distance = face->intersectWithRay(ray, side);
        if (!Math::isnan(distance))
            return FaceHit(face, distance);
    }
    return FaceHit();
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
