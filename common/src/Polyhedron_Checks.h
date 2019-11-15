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
            if (currentEdge->face() != currentFace) {
                return false;
            }
            if (currentEdge->edge() == nullptr) {
                return false;
            }
            if (!m_edges.contains(currentEdge->edge())) {
                return false;
            }
            if (!m_vertices.contains(currentEdge->origin())) {
                return false;
            }

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
            if (twin == nullptr) {
                return false;
            }
            if (twin->face() == nullptr) {
                return false;
            }
            if (!m_faces.contains(twin->face())) {
                return false;
            }
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
            if (currentFace->pointStatus(currentVertex->position()) == vm::plane_status::above) {
                return false;
            }
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
        if (leaving == nullptr) {
            return false;
        }
        if (leaving->origin() != currentVertex) {
            return false;
        }
        if (!point()) {
            const Edge* edge = leaving->edge();
            if (edge == nullptr) {
                return false;
            }
            if (!m_edges.contains(edge)) {
                return false;
            }
            if (polyhedron() && !edge->fullySpecified()) {
                return false;
            }
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
        const T length2 = vm::squared_length(currentEdge->vector());
        if (length2 < minLength2) {
            return false;
        }
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkLeavingEdges(const Vertex* v) const {
    assert(v != nullptr);
    const HalfEdge* firstEdge = v->leaving();
    assert(firstEdge != nullptr);
    const HalfEdge* curEdge = firstEdge;

    do {
        const HalfEdge* nextEdge = curEdge->nextIncident();
        do {
            if (curEdge->destination() == nextEdge->destination()) {
                return false;
            }
            nextEdge = nextEdge->nextIncident();
        } while (nextEdge != firstEdge);

        curEdge = curEdge->nextIncident();
    } while (curEdge->nextIncident() != firstEdge);

    return true;
}
