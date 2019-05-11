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

#ifndef TrenchBroom_Polyhedron_Misc_h
#define TrenchBroom_Polyhedron_Misc_h

#include "CollectionUtils.h"

#include <vecmath/vec.h>
#include <vecmath/ray.h>
#include <vecmath/plane.h>
#include <vecmath/bbox.h>
#include <vecmath/scalar.h>
#include <vecmath/util.h>

#include <map>

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::VertexDistanceCmp {
private:
    V m_anchor;
public:
    VertexDistanceCmp(const V& anchor) :
    m_anchor(anchor) {}

    bool operator()(const Vertex* lhs, const Vertex* rhs) const {
        const T lDist = squaredDistance(m_anchor, lhs->position());
        const T rDist = squaredDistance(m_anchor, rhs->position());
        if (lDist < rDist)
            return true;
        if (lDist > rDist)
            return false;
        return lhs->position() < rhs->position();
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
void Polyhedron<T,FP,VP>::Callback::vertexWasCreated(Vertex* vertex) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::vertexWillBeDeleted(Vertex* vertex) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::vertexWasAdded(Vertex* vertex) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::vertexWillBeRemoved(Vertex* vertex) {}

template <typename T, typename FP, typename VP>
vm::plane<T,3> Polyhedron<T,FP,VP>::Callback::getPlane(const Face* face) const {
    const auto& boundary = face->boundary();
    assert(boundary.size() >= 3);

    const auto* firstEdge = boundary.front();
    const auto* curEdge = firstEdge;
    do {
        const auto* e1 = curEdge;
        const auto* e2 = e1->next();
        const auto* e3 = e2->next();

        const auto& p1 = e1->origin()->position();
        const auto& p2 = e2->origin()->position();
        const auto& p3 = e3->origin()->position();

        const auto [valid, result] = fromPoints(p2, p1, p3);
        if (valid) {
            return result;
        }

        curEdge = curEdge->next();
    } while (curEdge != firstEdge);

    // TODO: We should really throw an exception here.
    assert(false);
    return ::vm::plane<T,3>(); // Ooops!
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::faceWasCreated(Face* face) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::faceWillBeDeleted(Face* face) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::faceDidChange(Face* face) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::faceWasFlipped(Face* face) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::faceWasSplit(Face* original, Face* clone) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::Callback::facesWillBeMerged(Face* remaining, Face* toDelete) {}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron() {
    updateBounds();
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(std::initializer_list<V> positions) {
    Callback c;
    addPoints(std::begin(positions), std::end(positions), c);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(std::initializer_list<V> positions, Callback& callback) {
    addPoints(std::begin(positions), std::end(positions), callback);
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
Polyhedron<T,FP,VP>::Polyhedron(const vm::bbox<T,3>& bounds) {
    Callback c;
    setBounds(bounds, c);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const vm::bbox<T,3>& bounds, Callback& callback) {
    setBounds(bounds, callback);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const std::vector<V>& positions) {
    Callback c;
    addPoints(std::begin(positions), std::end(positions), c);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const std::vector<V>& positions, Callback& callback) {
    addPoints(std::begin(positions), std::end(positions), callback);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(const Polyhedron<T,FP,VP>& other) {
    Copy copy(other.faces(), other.edges(), other.vertices(), *this);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>::Polyhedron(Polyhedron<T,FP,VP>&& other) noexcept :
m_vertices(std::move(other.m_vertices)),
m_edges(std::move(other.m_edges)),
m_faces(std::move(other.m_faces)),
m_bounds(std::move(other.m_bounds)) {}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addPoints(const V& p1, const V& p2, const V& p3, const V& p4, Callback& callback) {
    addPoint(p1, callback);
    addPoint(p2, callback);
    addPoint(p3, callback);
    addPoint(p4, callback);
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::setBounds(const vm::bbox<T,3>& bounds, Callback& callback) {
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

    m_bounds = bounds;
}

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Copy {
private:
    using VertexMap = std::map<const Vertex*, Vertex*>;
    using VertexMapEntry = typename VertexMap::value_type;

    using HalfEdgeMap = std::map<const HalfEdge*, HalfEdge*>;
    using HalfEdgeMapEntry = typename HalfEdgeMap::value_type;

    VertexMap m_vertexMap;
    HalfEdgeMap m_halfEdgeMap;

    VertexList m_vertices;
    EdgeList m_edges;
    FaceList m_faces;
    Polyhedron& m_destination;
public:
    Copy(const FaceList& originalFaces, const EdgeList& originalEdges, const VertexList& originalVertices, Polyhedron& destination) :
    m_destination(destination) {
        copyVertices(originalVertices);
        copyFaces(originalFaces);
        copyEdges(originalEdges);
        swapContents();
    }
private:
    void copyVertices(const VertexList& originalVertices) {
        if (!originalVertices.empty()) {
            const Vertex* firstVertex = originalVertices.front();
            const Vertex* currentVertex = firstVertex;
            do {
                Vertex* copy = new Vertex(currentVertex->position());
                assertResult(MapUtils::insertOrFail(m_vertexMap, currentVertex, copy));
                m_vertices.append(copy, 1);
                currentVertex = currentVertex->next();
            } while (currentVertex != firstVertex);
        }
    }

    void copyFaces(const FaceList& originalFaces) {
        if (!originalFaces.empty()) {
            const Face* firstFace = originalFaces.front();
            const Face* currentFace = firstFace;
            do {
                copyFace(currentFace);
                currentFace = currentFace->next();
            } while (currentFace != firstFace);
        }
    }

    void copyFace(const Face* originalFace) {
        HalfEdgeList myBoundary;

        const HalfEdge* firstHalfEdge = originalFace->m_boundary.front();
        const HalfEdge* currentHalfEdge = firstHalfEdge;
        do {
            myBoundary.append(copyHalfEdge(currentHalfEdge), 1);
            currentHalfEdge = currentHalfEdge->next();
        } while (currentHalfEdge != firstHalfEdge);

        Face* copy = new Face(myBoundary);
        m_faces.append(copy, 1);
    }

    HalfEdgeList copyBoundary(const HalfEdgeList& original) {
        HalfEdgeList result;

        const HalfEdge* firstHalfEdge = original.front();
        const HalfEdge* currentHalfEdge = firstHalfEdge;
        do {
            result.append(copyHalfEdge(currentHalfEdge), 1);
            currentHalfEdge = currentHalfEdge->next();
        } while (currentHalfEdge != firstHalfEdge);

        return result;
    }

    HalfEdge* copyHalfEdge(const HalfEdge* original) {
        const Vertex* originalOrigin = original->origin();

        Vertex* myOrigin = findVertex(originalOrigin);
        HalfEdge* copy = new HalfEdge(myOrigin);
        m_halfEdgeMap.insert(std::make_pair(original, copy));
        return copy;
    }

    Vertex* findVertex(const Vertex* original) {
        typename VertexMap::iterator it = m_vertexMap.find(original);
        assert(it != std::end(m_vertexMap));
        return it->second;
    }

    void copyEdges(const EdgeList& originalEdges) {
        if (!originalEdges.empty()) {
            const Edge* firstEdge = originalEdges.front();
            const Edge* currentEdge = firstEdge;
            do {
                m_edges.append(copyEdge(currentEdge), 1);
                currentEdge = currentEdge->next();
            } while (currentEdge != firstEdge);
        }
    }

    Edge* copyEdge(const Edge* original) {
        HalfEdge* myFirst = findOrCopyHalfEdge(original->firstEdge());
        if (!original->fullySpecified())
            return new Edge(myFirst);

        HalfEdge* mySecond = findOrCopyHalfEdge(original->secondEdge());
        return new Edge(myFirst, mySecond);
    }

    HalfEdge* findOrCopyHalfEdge(const HalfEdge* original) {
        const auto insertPos = MapUtils::findInsertPos(m_halfEdgeMap, original);
        if (!insertPos.first) {
            const Vertex* originalOrigin = original->origin();
            Vertex* myOrigin = findVertex(originalOrigin);
            HalfEdge* copy = new HalfEdge(myOrigin);
            m_halfEdgeMap.insert(insertPos.second, std::make_pair(original, copy));
            return copy;
        }

        const auto it = insertPos.second;
        assert(it != std::begin(m_halfEdgeMap));
        return std::prev(it)->second;
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
Polyhedron<T,FP,VP>::~Polyhedron() {
    clear();
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP>& Polyhedron<T,FP,VP>::operator=(Polyhedron<T,FP,VP> other) {
    swap(*this, other);
    return *this;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::operator==(const Polyhedron& other) const {
    if (vertexCount() != other.vertexCount())
        return false;
    if (edgeCount() != other.edgeCount())
        return false;
    if (faceCount() != other.faceCount())
        return false;

    if (vertexCount() > 0) {
        Vertex* current = m_vertices.front();
        Vertex* first = current;
        do {
            if (!other.hasVertex(current->position(), 0.0))
                return false;
            current = current->next();
        } while (current != first);
    }

    if (edgeCount() > 0) {
        Edge* current = m_edges.front();
        Edge* first = current;
        do {
            if (!other.hasEdge(current->firstVertex()->position(), current->secondVertex()->position(), 0.0))
                return false;
            current = current->next();
        } while (current != first);
    }

    if (faceCount() > 0) {
        Face* current = m_faces.front();
        Face* first = current;
        do {
            if (!other.hasFace(current->vertexPositions(), 0.0))
                return false;
            current = current->next();
        } while (current != first);
    }

    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::operator!=(const Polyhedron& other) const {
    return !(*this == other);
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
    return findVertexByPosition(position, nullptr, epsilon) != nullptr;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::hasVertex(const std::vector<V>& positions, const T epsilon) const {
    for (const V& position : positions) {
        if (hasVertex(position, epsilon))
            return true;
    }
    return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::hasVertices(const std::vector<V>& positions, const T epsilon) const {
    if (positions.size() != vertexCount())
        return false;
    for (const V& position : positions) {
        if (!hasVertex(position, epsilon))
            return false;
    }
    return true;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::PosList Polyhedron<T,FP,VP>::vertexPositions() const {
    std::vector<V> result;
    result.reserve(vertexCount());
    getVertexPositions(std::back_inserter(result));
    return result;
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
    return findEdgeByPositions(pos1, pos2, epsilon) != nullptr;
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
bool Polyhedron<T,FP,VP>::hasFace(const std::vector<V>& positions, const T epsilon) const {
    return findFaceByPositions(positions, epsilon) != nullptr;
}

template <typename T, typename FP, typename VP>
const vm::bbox<T,3>& Polyhedron<T,FP,VP>::bounds() const {
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
Polyhedron<T,FP,VP>::FaceHit::FaceHit() : face(nullptr), distance(vm::nan<T>()) {}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::FaceHit::isMatch() const { return face != nullptr; }

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::FaceHit Polyhedron<T,FP,VP>::pickFace(const vm::ray<T,3>& ray) const {
    const auto side = polygon() ? vm::side::both : vm::side::front;
    auto* firstFace = m_faces.front();
    auto* currentFace = firstFace;
    do {
        const auto distance = currentFace->intersectWithRay(ray, side);
        if (!vm::isnan(distance)) {
            return FaceHit(currentFace, distance);
        }
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return FaceHit();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::findVertexByPosition(const V& position, const Vertex* except, const T epsilon) const {
    auto* firstVertex = m_vertices.front();
    auto* currentVertex = firstVertex;
    do {
        if (currentVertex != except && isEqual(position, currentVertex->position(), epsilon)) {
            return currentVertex;
        }
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return nullptr;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Vertex* Polyhedron<T,FP,VP>::findClosestVertex(const V& position, const T maxDistance) const {
    auto closestDistance2 = maxDistance * maxDistance;
    Vertex* closestVertex = nullptr;

    auto* firstVertex = m_vertices.front();
    auto* currentVertex = firstVertex;
    do {
        const T currentDistance2 = squaredDistance(position, currentVertex->position());
        if (currentDistance2 < closestDistance2) {
            closestDistance2 = currentDistance2;
            closestVertex = currentVertex;
        }
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return closestVertex;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::ClosestVertexSet Polyhedron<T,FP,VP>::findClosestVertices(const V& position) const {
    ClosestVertexSet result = ClosestVertexSet(VertexDistanceCmp(position));
    auto* firstVertex = m_vertices.front();
    auto* currentVertex = firstVertex;
    do {
        result.insert(currentVertex);
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return result;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::findEdgeByPositions(const V& pos1, const V& pos2, const T epsilon) const {
    auto* firstEdge = m_edges.front();
    auto* currentEdge = firstEdge;
    do {
        currentEdge = currentEdge->next();
        if (currentEdge->hasPositions(pos1, pos2, epsilon))
            return currentEdge;
    } while (currentEdge != firstEdge);
    return nullptr;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::findClosestEdge(const V& pos1, const V& pos2, const T maxDistance) const {
    auto closestDistance = maxDistance;
    Edge* closestEdge = nullptr;

    auto* firstEdge = m_edges.front();
    auto* currentEdge = firstEdge;
    do {
        const auto currentDistance = currentEdge->distanceTo(pos1, pos2);
        if (currentDistance < closestDistance) {
            closestDistance = currentDistance;
            closestEdge = currentEdge;
        }
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return closestEdge;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::findFaceByPositions(const std::vector<V>& positions, const T epsilon) const {
    auto* firstFace = m_faces.front();
    auto* currentFace = firstFace;
    do {
        if (currentFace->hasVertexPositions(positions, epsilon))
            return currentFace;
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return nullptr;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Face* Polyhedron<T,FP,VP>::findClosestFace(const std::vector<V>& positions, const T maxDistance) {
    auto closestDistance = maxDistance;
    Face* closestFace = nullptr;

    auto* firstFace = m_faces.front();
    auto* currentFace = firstFace;
    do {
        const auto currentDistance = currentFace->distanceTo(positions);
        if (currentDistance < closestDistance) {
            closestDistance = currentDistance;
            closestFace = currentFace;
        }
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return closestFace;
}

template <typename T, typename FP, typename VP> template <typename O>
void Polyhedron<T,FP,VP>::getVertexPositions(O output) const {
    auto* firstVertex = m_vertices.front();
    auto* currentVertex = firstVertex;
    do {
        output = currentVertex->position();
        ++output;
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::hasVertex(const Vertex* vertex) const {
    const auto* firstVertex = m_vertices.front();
    const auto* currentVertex = firstVertex;
    do {
        if (currentVertex == vertex) {
            return true;
        }
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::hasEdge(const Edge* edge) const {
    const auto* firstEdge = m_edges.front();
    const auto* currentEdge = firstEdge;
    do {
        if (currentEdge == edge) {
            return true;
        }
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return false;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::hasFace(const Face* face) const {
    const auto* firstFace = m_faces.front();
    const auto* currentFace = firstFace;
    do {
        if (currentFace == face) {
            return true;
        }
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
    if (!checkFaceNeighbours())
        return false;
    if (!checkOverlappingFaces())
        return false;
    if (!checkVertexLeavingEdges())
        return false;
    if (!checkEulerCharacteristic())
        return false;
    if (!checkClosed())
        return false;
    if (!checkNoDegenerateFaces())
        return false;
    if (!checkEdges())
        return false;
    /* This check leads to false positive with almost coplanar faces.
     if (polyhedron() && !checkNoCoplanarFaces())
     return false;
     */
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkEulerCharacteristic() const {
    if (!polyhedron())
        return true;

    // See https://en.m.wikipedia.org/wiki/Euler_characteristic
    return vertexCount() + faceCount() - edgeCount() == 2;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkOverlappingFaces() const {
    if (!polyhedron())
        return true;

    Face* firstFace = m_faces.front();
    Face* curFace1 = firstFace;
    do {
        Face* curFace2 = curFace1->next();
        while (curFace2 != firstFace) {
            const size_t sharedVertexCount = curFace1->countSharedVertices(curFace2);
            if (sharedVertexCount == curFace1->vertexCount() ||
                sharedVertexCount == curFace2->vertexCount()) {

                std::cout << "Face1: " << std::endl << *curFace1;
                std::cout << "Face2: " << std::endl << *curFace2;

                return false;
            }
            curFace2 = curFace2->next();
        }
        curFace1 = curFace1->next();
    } while (curFace1->next() != firstFace);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkFaceBoundaries() const {
    if (m_faces.empty())
        return true;

    Face* firstFace = m_faces.front();
    Face* currentFace = firstFace;
    do {
        HalfEdge* firstEdge = currentFace->boundary().front();
        HalfEdge* currentEdge = firstEdge;
        do {
            if (currentEdge->face() != currentFace)
                return false;
            if (currentEdge->edge() == nullptr)
                return false;
            if (!hasEdge(currentEdge->edge()))
                return false;
            if (!hasVertex(currentEdge->origin()))
                return false;

            currentEdge = currentEdge->next();
        } while (currentEdge != firstEdge);

        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkFaceNeighbours() const {
    if (!polyhedron())
        return true;

    Face* firstFace = m_faces.front();
    Face* currentFace = firstFace;
    do {
        const HalfEdgeList& boundary = currentFace->boundary();
        HalfEdge* firstEdge = boundary.front();
        HalfEdge* currentEdge = firstEdge;
        do {
            HalfEdge* twin = currentEdge->twin();
            if (twin == nullptr)
                return false;
            if (twin->face() == nullptr)
                return false;
            if (!hasFace(twin->face()))
                return false;

            currentEdge = currentEdge->next();
        } while (currentEdge != firstEdge);

        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkConvex() const {
    if (!polyhedron())
        return true;

    const Face* firstFace = m_faces.front();
    const Face* currentFace = firstFace;
    do {
        const Vertex* firstVertex = m_vertices.front();
        const Vertex* currentVertex = firstVertex;
        do {
            if (currentFace->pointStatus(currentVertex->position()) == vm::point_status::above)
                return false;
            currentVertex = currentVertex->next();
        } while (currentVertex != firstVertex);
        currentFace = currentFace->next();
    } while (currentFace != firstFace);

    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkClosed() const {
    if (!polyhedron())
        return true;

    const Edge* firstEdge = m_edges.front();
    const Edge* currentEdge = firstEdge;
    do {
        if (!currentEdge->fullySpecified())
            return false;

        const Face* firstFace = currentEdge->firstFace();
        const Face* secondFace = currentEdge->secondFace();

        if (!m_faces.contains(firstFace))
            return false;
        if (!m_faces.contains(secondFace))
            return false;
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);

    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkNoCoplanarFaces() const {
    if (!polyhedron())
        return true;

    const Edge* firstEdge = m_edges.front();
    const Edge* currentEdge = firstEdge;
    do {
        const Face* firstFace = currentEdge->firstFace();
        const Face* secondFace = currentEdge->secondFace();

        if (firstFace == secondFace)
            return false;
        if (firstFace->coplanar(secondFace))
            return false;
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);

    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkNoDegenerateFaces() const {
    if (!polyhedron())
        return true;

    const Face* firstFace = m_faces.front();
    const Face* currentFace = firstFace;
    do {
        if (currentFace->vertexCount() < 3)
            return false;

        const HalfEdge* firstHalfEdge = currentFace->boundary().front();
        const HalfEdge* currentHalfEdge = firstHalfEdge;
        do {
            const Edge* edge = currentHalfEdge->edge();
            if (edge == nullptr || !edge->fullySpecified())
                return false;
            currentHalfEdge = currentHalfEdge->next();
        } while (currentHalfEdge != firstHalfEdge);

        currentFace = currentFace->next();
    } while (currentFace != firstFace);

    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkVertexLeavingEdges() const {
    if (empty() || point())
        return true;

    const Vertex* firstVertex = m_vertices.front();
    const Vertex* currentVertex = firstVertex;
    do {
        const HalfEdge* leaving = currentVertex->leaving();
        if (leaving == nullptr)
            return false;
        if (leaving->origin() != currentVertex)
            return false;
        if (!point()) {
            const Edge* edge = leaving->edge();
            if (edge == nullptr)
                return false;
            if (!hasEdge(edge))
                return false;
            if (polyhedron() && !edge->fullySpecified())
                return false;
        }
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkEdges() const {
    if (!polyhedron())
        return true;

    Edge* firstEdge = m_edges.front();
    Edge* currentEdge = firstEdge;
    do {
        if (!currentEdge->fullySpecified())
            return false;
        Face* firstFace = currentEdge->firstFace();
        if (firstFace == nullptr)
            return false;
        if (!m_faces.contains(firstFace))
            return false;

        Face* secondFace = currentEdge->secondFace();
        if (secondFace == nullptr)
            return false;
        if (!m_faces.contains(secondFace))
            return false;

        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkEdgeLengths(const T minLength) const {
    if (m_edges.empty())
        return true;

    const T minLength2 = minLength * minLength;

    const Edge* firstEdge = m_edges.front();
    const Edge* currentEdge = firstEdge;
    do {
        const T length2 = squaredLength(currentEdge->vector());
        if (length2 < minLength2)
            return false;
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkLeavingEdges(const Vertex* v) const {
    ensure(v != nullptr, "v is null");
    const HalfEdge* firstEdge = v->leaving();
    ensure(firstEdge != nullptr, "firstEdge is null");
    const HalfEdge* curEdge = firstEdge;

    do {
        const HalfEdge* nextEdge = curEdge->nextIncident();
        do {
            if (curEdge->destination() == nextEdge->destination())
                return false;
            nextEdge = nextEdge->nextIncident();
        } while (nextEdge != firstEdge);

        curEdge = curEdge->nextIncident();
    } while (curEdge->nextIncident() != firstEdge);

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

    updateBounds();
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::healEdges(const T minLength) {
    Callback callback;
    return healEdges(callback, minLength);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::healEdges(Callback& callback, const T minLength) {
    const T minLength2 = minLength * minLength;

    /*
     We have to iterate over all edges while the list of edges is being modified, so we cannot use the usual
     do / while iteration. Instead, we count the number of edges we have examined - but since one or more edges
     can be removed in every iteration, we have to correct that number for the decrease in the total number of
     edges of the brush - this is where sizeDelta comes in. If no edges has been removed, it comes out as -1.
     If one edge has been removed, it comes out as 0, if two edges have been removed, it comes out as +1, and so on.

     Since sizeDelta is subtracted from the number of examined edges, it corrects exactly for the change in the number
     of edges of the brush.
     */

    long examined = 0;
    Edge* currentEdge = m_edges.front();
    while (examined < static_cast<long>(m_edges.size()) && polyhedron()) {
        const size_t oldSize = m_edges.size();

        const T length2 = vm::squaredLength(currentEdge->vector());
        if (length2 < minLength2) {
            currentEdge = removeEdge(currentEdge, callback);
        } else {
            currentEdge = currentEdge->next();
        }

        const size_t newSize = m_edges.size();
        const long sizeDelta = static_cast<long>(oldSize - newSize) - 1;
        examined -= sizeDelta;
    }

    assert(!polyhedron() || checkEdgeLengths(minLength));

    updateBounds();

    return polyhedron();
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::removeEdge(Edge* edge, Callback& callback) {
    // First, transfer all edges from the second to the first vertex of the given edge.
    // This results in the edge being a loop and the second vertex to be orphaned.
    auto* firstVertex = edge->firstVertex();
    auto* secondVertex = edge->secondVertex();
    while (secondVertex->leaving() != nullptr) {
        auto* leaving = secondVertex->leaving();
        auto* newLeaving = leaving->previous()->twin();
        leaving->setOrigin(firstVertex);
        if (newLeaving->origin() == secondVertex) {
            secondVertex->setLeaving(newLeaving);
        } else {
            secondVertex->setLeaving(nullptr);
        }
    }

    // Remove the edge's first edge from its first face and delete the face if it degenerates
    {
        auto* firstFace = edge->firstFace();
        auto* firstEdge = edge->firstEdge();
        auto* nextEdge = firstEdge->next();

        firstVertex->setLeaving(firstEdge->previous()->twin());
        firstFace->removeFromBoundary(firstEdge);
        nextEdge->setOrigin(firstVertex);
        delete firstEdge;

        if (firstFace->vertexCount() == 2) {
            removeDegenerateFace(firstFace, callback);
        }
    }

    // Remove the edges's second edge from its second face and delete the face if it degenerates
    {
        auto* secondFace = edge->secondFace();
        auto* secondEdge = edge->secondEdge();

        secondFace->removeFromBoundary(secondEdge);
        delete secondEdge;

        if (secondFace->vertexCount() == 2) {
            removeDegenerateFace(secondFace, callback);
        }
    }

    callback.vertexWillBeDeleted(secondVertex);
    m_vertices.remove(secondVertex);
    delete secondVertex;

    auto* result = edge->next();
    m_edges.remove(edge);
    delete edge;

    // Merge faces that may have become coplanar
    {
        auto* firstEdge = firstVertex->leaving();
        auto* currentEdge = firstEdge;
        do {
            auto* nextEdge = currentEdge->nextIncident();
            auto* currentFace = firstEdge->face();
            auto* neighbour = firstEdge->twin()->face();
            if (currentFace->coplanar(neighbour)) {
                result = mergeNeighbours(currentEdge, result, callback);
            }
            currentEdge = nextEdge;
        } while (currentEdge != firstEdge);
    }

    return result;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::removeDegenerateFace(Face* face, Callback& callback) {
    assert(face->vertexCount() == 2);

    // The boundary of the face to remove consists of two half edges:
    auto* halfEdge1 = face->boundary().front();
    auto* halfEdge2 = halfEdge1->next();
    assert(halfEdge2->next() == halfEdge1);
    assert(halfEdge1->previous() == halfEdge2);

    // The face has two vertices:
    auto* vertex1 = halfEdge1->origin();
    auto* vertex2 = halfEdge2->origin();

    // Make sure we don't delete the vertices' leaving edges:
    vertex1->setLeaving(halfEdge2->twin());
    vertex2->setLeaving(halfEdge1->twin());

    assert(vertex1->leaving() != halfEdge1);
    assert(vertex1->leaving() != halfEdge2);
    assert(vertex2->leaving() != halfEdge1);
    assert(vertex2->leaving() != halfEdge2);

    // These two edges will be merged into one:
    auto* edge1 = halfEdge1->edge();
    auto* edge2 = halfEdge2->edge();

    // The twins of the two half edges of the degenerate face will become twins now.
    auto* halfEdge1Twin = halfEdge1->twin();
    auto* halfEdge2Twin = halfEdge2->twin();

    // We will keep edge1 and delete edge2.
    // Make sure that halfEdge1's twin is the first edge of edge1:
    edge1->makeFirstEdge(halfEdge1Twin);

    // Now replace halfEdge2 by new halfEdge2Twin:
    assert(halfEdge2Twin->edge() == edge2);
    halfEdge2Twin->unsetEdge();
    edge1->unsetSecondEdge(); // unsets halfEdge1, leaving halfEdge1Twin as the first half edge of edge1
    edge1->setSecondEdge(halfEdge2Twin); // replace halfEdge1 with halfEdge2Twin

    // Now edge1 should be correct:
    assert(edge1->firstEdge() == halfEdge1Twin);
    assert(edge1->secondEdge() == halfEdge2Twin);

    // Delete the now obsolete edge.
    // The constructor doesn't do anything, so no further cleanup is necessary.
    m_edges.remove(edge2);
    delete edge2;

    // Delete the degenerate face. This also deletes its boundary of halfEdge1 and halfEdge2.
    callback.faceWillBeDeleted(face);
    m_faces.remove(face);
    delete face;
}

// Merges the face incident to the given edge (called border) with its neighbour, i.e.
// the face incident to the given face's twin. The face incident to the border is deleted
// while the neighbour consumes the boundary of the incident face.
// Also handles the case where the border is longer than just one edge.
// The given valid edge is to remain valid, that is, if it is deleted, its successor is to be
// returned. The returned edge will therefore be the given valid edge, or its first successor that
// wasn't deleted by this function.
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::Edge* Polyhedron<T,FP,VP>::mergeNeighbours(HalfEdge* borderFirst, Edge* validEdge, Callback& callback) {
    Face* face = borderFirst->face();
    Face* neighbour = borderFirst->twin()->face();

    while (borderFirst->previous()->face() == face &&
           borderFirst->previous()->twin()->face() == neighbour) {
        borderFirst = borderFirst->previous();
    }

    HalfEdge* twinLast = borderFirst->twin();
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
        Vertex* origin = cur->origin();

        if (edge == validEdge) {
            validEdge = validEdge->next();
        }

        m_edges.remove(edge);
        delete edge;

        delete cur;
        delete twin;

        if (cur != borderFirst) {
            callback.vertexWillBeDeleted(origin);
            m_vertices.remove(origin);
            delete origin;
        }

        cur = next;
    } while (cur != borderFirst);

    callback.facesWillBeMerged(neighbour, face);
    m_faces.remove(face);
    delete face;

    return validEdge;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::updateBounds() {
    if (m_vertices.size() == 0) {
        m_bounds.min = m_bounds.max = vm::vec<T,3>::NaN;
    } else {
        Vertex* first = m_vertices.front();
        Vertex* current = first;
        m_bounds.min = m_bounds.max = current->position();

        current = current->next();
        while (current != first) {
            m_bounds = vm::merge(m_bounds, current->position());
            current = current->next();
        }
    }
}

#endif
