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

#ifndef Polyhedron_Subtract_h
#define Polyhedron_Subtract_h

template <typename T, typename FP>
typename Polyhedron<T,FP>::SubtractResult Polyhedron<T,FP>::subtract(const Polyhedron& subtrahend) const {
    Callback c;
    return subtract(subtrahend, c);
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::SubtractResult Polyhedron<T,FP>::subtract(Polyhedron subtrahend, const Callback& callback) const {
    if (!clipSubtrahend(subtrahend, callback))
        return SubtractResult(0);
    
    SubtractResult result;
    
    buildInitialFragments(subtrahend, result, callback);
    buildMissingFragments(subtrahend, result, callback);
    resolveIntersections(result, callback);
    
    return result;
}

template <typename T, typename FP>
bool Polyhedron<T,FP>::clipSubtrahend(Polyhedron& subtrahend, const Callback& callback) const {
    Face* first = m_faces.front();
    Face* current = first;
    do {
        const ClipResult result = subtrahend.clip(callback.plane(current));
        if (result.empty())
            return false;
        current = current->next();
    } while (current != first);
    return true;
}

template <typename T, typename FP>
void Polyhedron<T,FP>::buildInitialFragments(const Polyhedron& subtrahend, SubtractResult& result, const Callback& callback) const {
    const VertexFaceMap vertexProximity = findClosestFaces(subtrahend.faces(), m_vertices, Math::PointStatus::PSBelow, callback);
    
    Face* firstFace = subtrahend.faces().front();
    Face* currentFace = firstFace;
    do {
        const typename V::Set vertices = findFaceVertices(currentFace, m_vertices, Math::PointStatus::PSBelow, vertexProximity, callback);
        if (!vertices.empty())
            result.push_back(Polyhedron(vertices));
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
}

template <typename T, typename FP>
void Polyhedron<T,FP>::buildMissingFragments(const Polyhedron& subtrahend, SubtractResult& result, const Callback& callback) const {

}

template <typename T, typename FP>
typename Polyhedron<T,FP>::V::Set Polyhedron<T,FP>::findFaceVertices(Face* face, const VertexList& vertices, Math::PointStatus::Type skipStatus, const VertexFaceMap& vertexProximity, const Callback& callback) const {
    const Plane3 plane = callback.plane(face);
    
    bool hasVertexAbove = false;
    typename V::Set allVertices;
    Vertex* firstVertex = vertices.front();
    Vertex* currentVertex = firstVertex;
    do {
        typename VertexFaceMap::const_iterator it = vertexProximity.find(currentVertex);
        assert(it != vertexProximity.end());
        
        const FaceSet& closestFaces = it->second;
        if (closestFaces.count(face) > 0) {
            const Math::PointStatus::Type status = plane.pointStatus(currentVertex->position());
            if (status != skipStatus) {
                if (status != Math::PointStatus::PSInside)
                    hasVertexAbove = true;
                allVertices.insert(currentVertex->position());
            }
        }
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    
    if (!hasVertexAbove)
        return V::EmptySet;
    
    const HalfEdge* firstEdge = face->boundary().front();
    const HalfEdge* currentEdge = firstEdge;
    do {
        allVertices.insert(currentEdge->origin()->position());
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    
    return allVertices;
}


template <typename T, typename FP>
typename Polyhedron<T,FP>::VertexFaceMap Polyhedron<T,FP>::findClosestFaces(const FaceList& faces, const VertexList& vertices, const Math::PointStatus::Type skipStatus, const Callback& callback) const {
    VertexFaceMap result;
    Vertex* firstVertex = vertices.front();
    Vertex* currentVertex = firstVertex;
    do {
        T closestDistance = std::numeric_limits<T>::max();
        FaceSet closestFaces;
        
        Face* firstFace = faces.front();
        Face* currentFace = firstFace;
        do {
            const Plane3 plane = callback.plane(currentFace);
            if (plane.pointStatus(currentVertex->position()) != skipStatus) {
                const T distance = faceVertexDistance(currentFace, currentVertex);
                if (Math::lt(distance, closestDistance)) {
                    closestDistance = distance;
                    closestFaces.clear();
                    closestFaces.insert(currentFace);
                } else if (Math::eq(distance, closestDistance)) {
                    closestFaces.insert(currentFace);
                }
            }
            currentFace = currentFace->next();
        } while (currentFace != firstFace);
        
        result.insert(std::make_pair(currentVertex, closestFaces));
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    
    return result;
}

template <typename T, typename FP>
T Polyhedron<T,FP>::faceVertexDistance(const Face* face, const Vertex* vertex) const {
    T closestDistance = std::numeric_limits<T>::max();
    
    const HalfEdge* firstEdge = face->boundary().front();
    const HalfEdge* currentEdge = firstEdge;
    do {
        const Vertex* currentVertex = currentEdge->origin();
        const T distance = currentVertex->position().squaredDistanceTo(vertex->position());
        closestDistance = Math::min(distance, closestDistance);
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return closestDistance;
}

template <typename T, typename FP>
void Polyhedron<T,FP>::resolveIntersections(SubtractResult& result, const Callback& callback) const {
    typename SubtractResult::iterator cur = result.begin();
    while (cur != result.end()) {
        typename SubtractResult::iterator it = cur;
        typename SubtractResult::iterator end = result.end();
        
        bool increment = true;
        while (++it != end) {
            Polyhedron intersection = cur->intersect(*it, callback);
            if (!intersection.empty()) {
                const SubtractResult firstResult = cur->subtract(intersection, callback);
                const SubtractResult secondResult = it->subtract(intersection, callback);

                result.insert(result.end(), firstResult.begin(), firstResult.end());
                result.insert(result.end(), secondResult.begin(), secondResult.end());
                result.push_back(intersection);
                
                result.erase(it);
                cur = result.erase(cur);
                increment = false;
                break;
            }
        }
        if (increment)
            ++cur;
    }
}

#endif /* Polyhedron_Subtract_h */
