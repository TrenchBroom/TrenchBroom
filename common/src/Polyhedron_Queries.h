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

#ifndef Polyhedron_Queries_h
#define Polyhedron_Queries_h

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::contains(const V& point, const Callback& callback) const {
    if (!bounds().contains(point))
        return false;
    
    const Face* firstFace = m_faces.front();
    const Face* currentFace = firstFace;
    do {
        const Plane<T,3> plane = callback.plane(currentFace);
        if (plane.pointStatus(point) == Math::PointStatus::PSAbove)
            return false;
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::contains(const Polyhedron& other, const Callback& callback) const {
    if (!bounds().contains(other.bounds()))
        return false;
    const Vertex* theirFirst = other.vertices().front();
    const Vertex* theirCurrent = theirFirst;
    do {
        if (!contains(theirCurrent->position()))
            return false;
        theirCurrent = theirCurrent->next();
    } while (theirCurrent != theirFirst);
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::intersects(const Polyhedron& other, const Callback& callback) const {
    if (!bounds().intersects(other.bounds()))
        return false;

    // separating axis theorem
    // http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
    
    if (separate(m_faces.front(), other.vertices().front(), callback))
        return false;
    if (separate(other.faces().front(), m_vertices.front(), callback))
        return false;
    
    const Edge* myFirstEdge = m_edges.front();
    const Edge* myCurrentEdge = myFirstEdge;
    const Edge* theirFirstEdge = other.edges().front();
    do {
        const V myEdgeVec = myCurrentEdge->vector();
        const V& origin = myCurrentEdge->firstVertex()->position();
        
        const Edge* theirCurrentEdge = theirFirstEdge;
        do {
            const V theirEdgeVec = theirCurrentEdge->vector();
            const V direction = crossed(myEdgeVec, theirEdgeVec);
            const Plane<T,3> plane(origin, direction);
            
            const Math::PointStatus::Type myStatus = pointStatus(plane, m_vertices.front());
            if (myStatus != Math::PointStatus::PSInside) {
                const Math::PointStatus::Type theirStatus = pointStatus(plane, other.vertices().front());
                if (theirStatus != Math::PointStatus::PSInside) {
                    if (myStatus != theirStatus)
                        return false;
                }
            }
            
            theirCurrentEdge = theirCurrentEdge->next();
        } while (theirCurrentEdge != theirFirstEdge);
        myCurrentEdge = myCurrentEdge->next();
    } while (myCurrentEdge != myFirstEdge);
    
    return true;
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::separate(const Face* firstFace, const Vertex* firstVertex, const Callback& callback) const {
    const Face* currentFace = firstFace;
    do {
        const Plane<T,3> plane = callback.plane(currentFace);
        if (pointStatus(plane, firstVertex) == Math::PointStatus::PSAbove)
            return true;
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return false;
}

template <typename T, typename FP, typename VP>
Math::PointStatus::Type Polyhedron<T,FP,VP>::pointStatus(const Plane<T,3>& plane, const Vertex* firstVertex) const {
    size_t above = 0;
    size_t below = 0;
    const Vertex* currentVertex = firstVertex;
    do {
        const Math::PointStatus::Type status = plane.pointStatus(currentVertex->position());
        if (status == Math::PointStatus::PSAbove)
            ++above;
        else if (status == Math::PointStatus::PSBelow)
            ++below;
        if (above > 0 && below > 0)
            return Math::PointStatus::PSInside;
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return above > 0 ? Math::PointStatus::PSAbove : Math::PointStatus::PSBelow;
}

#endif /* Polyhedron_Queries_h */
