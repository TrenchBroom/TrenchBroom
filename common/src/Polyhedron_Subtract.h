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
    
    const FaceVertexMap map = findFaceVertices(subtrahend, callback);
    
    SubtractResult result;
    result.reserve(map.size());
    
    typename FaceVertexMap::const_iterator it, end;
    for (it = map.begin(), end = map.end(); it != end; ++it) {
        const Face* subtrahendFace = it->first;
        const VertexSet& myVertices = it->second;
        typename V::List vertices;
        vertices.reserve(subtrahendFace->boundary().size() + myVertices.size());
        
        const HalfEdge* firstEdge = subtrahendFace->boundary().front();
        const HalfEdge* currentEdge = firstEdge;
        do {
            vertices.push_back(currentEdge->origin()->position());
            currentEdge = currentEdge->next();
        } while (currentEdge != firstEdge);

        V::toList(myVertices.begin(), myVertices.end(), GetVertexPosition(), vertices);
        result.push_back(vertices);
    }
    
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
typename Polyhedron<T,FP>::FaceVertexMap Polyhedron<T,FP>::findFaceVertices(const Polyhedron& subtrahend, const Callback& callback) const {
    
    FaceVertexMap result;
    
    // maps each of my vertices to those faces of the subtrahend to which it is closest
    const VertexFaceMap closestFaces = findClosestFaces(subtrahend, callback);
    
    Face* firstFace = subtrahend.faces().front();
    Face* currentFace = firstFace;
    do {
        const Plane3 plane = callback.plane(currentFace);
        
        bool hasVertexAbove = false;
        VertexSet vertices;
        Vertex* firstVertex = m_vertices.front();
        Vertex* currentVertex = firstVertex;
        do {
            typename VertexFaceMap::const_iterator it = closestFaces.find(currentVertex);
            assert(it != closestFaces.end());
            
            const FaceSet& faces = it->second;
            if (faces.count(currentFace) > 0) {
                const Math::PointStatus::Type status = plane.pointStatus(currentVertex->position());
                switch (status) {
                    case Math::PointStatus::PSAbove:
                        hasVertexAbove = true;
                    case Math::PointStatus::PSInside:
                        vertices.insert(currentVertex);
                        break;
                    default:
                        break;
                }
            }
            currentVertex = currentVertex->next();
        } while (currentVertex != firstVertex);
        
        if (hasVertexAbove)
            result.insert(std::make_pair(currentFace, vertices));
        
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    
    return result;
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::VertexFaceMap Polyhedron<T,FP>::findClosestFaces(const Polyhedron& subtrahend, const Callback& callback) const {
    VertexFaceMap result;
    Vertex* firstVertex = m_vertices.front();
    Vertex* currentVertex = firstVertex;
    do {
        T closestDistance = std::numeric_limits<T>::max();
        FaceSet faces;
        
        Face* firstFace = subtrahend.faces().front();
        Face* currentFace = firstFace;
        do {
            const Plane3 plane = callback.plane(currentFace);
            if (plane.pointStatus(currentVertex->position()) != Math::PointStatus::PSBelow) {
                const T distance = faceVertexDistance(currentFace, currentVertex);
                if (Math::lt(distance, closestDistance)) {
                    closestDistance = distance;
                    faces.clear();
                    faces.insert(currentFace);
                } else if (Math::eq(distance, closestDistance)) {
                    faces.insert(currentFace);
                }
            }
            currentFace = currentFace->next();
        } while (currentFace != firstFace);
        
        result.insert(std::make_pair(currentVertex, faces));
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
        const T distance = currentVertex->position().distanceTo(vertex->position());
        closestDistance = Math::min(distance, closestDistance);
        currentEdge = currentEdge->next();
    } while (currentEdge != firstEdge);
    return closestDistance;
}

#endif /* Polyhedron_Subtract_h */
