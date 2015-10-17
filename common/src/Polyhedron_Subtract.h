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
    result.push_back(*this);
    
    chopMinuend(subtrahend, result, callback);
    removeSubtrahend(subtrahend, result);
    simplifySubtractResult(subtrahend, result, callback);
    
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
void Polyhedron<T,FP>::chopMinuend(const Polyhedron& subtrahend, SubtractResult& result, const Callback& callback) const {
    const Face* firstFace = subtrahend.faces().front();
    const Face* currentFace = firstFace;
    do {
        const Plane<T,3> plane = callback.plane(currentFace);
        
        typename SubtractResult::iterator it, end;
        for (it = result.begin(), end = result.end(); it != end; ++it) {
            Polyhedron front = *it;
            const ClipResult clipResult = front.clip(plane);
            if (clipResult.success()) {
                it->clip(plane.flipped());
                it = result.insert(it, front);
                ++it;
            }
        }
        
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
}

template <typename T, typename FP>
void Polyhedron<T,FP>::removeSubtrahend(const Polyhedron& subtrahend, SubtractResult& result) const {
    const typename V::List vertices = V::asList(subtrahend.vertices().begin(), subtrahend.vertices().end(), GetVertexPosition());
    
    typename SubtractResult::iterator it, end;
    for (it = result.begin(), end = result.end(); it != end; ++it) {
        const Polyhedron& fragment = *it;
        if (fragment.hasVertices(vertices)) {
            result.erase(it);
            break;
        }
    }
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::ClosestVertices Polyhedron<T,FP>::findClosestVertices(const SubtractResult& result) const {
    ClosestVertices closestVertices;
    
    typename SubtractResult::const_iterator it, end;
    for (it = result.begin(), end = result.end(); it != end; ++it) {
        const Polyhedron& fragment = *it;
        const Vertex* firstVertex = fragment.vertices().front();
        const Vertex* currentVertex = firstVertex;
        do {
            if (closestVertices.count(currentVertex->position()) == 0) {
                const Vertex* closestMinuendVertex = findClosestVertex(currentVertex->position());
                closestVertices[currentVertex->position()] = closestMinuendVertex->position();
            }
            currentVertex = currentVertex->next();
        } while (currentVertex != firstVertex);
    }
    
    return closestVertices;
}

template <typename T, typename FP>
void Polyhedron<T,FP>::simplifySubtractResult(const Polyhedron& subtrahend, SubtractResult& result, const Callback& callback) const {
    typename V::Set exclude;
    SetUtils::makeSet(V::asList(subtrahend.vertices().begin(), subtrahend.vertices().end(), GetVertexPosition()), exclude);
    SetUtils::makeSet(V::asList(m_vertices.begin(), m_vertices.end(), GetVertexPosition()), exclude);
    
    const ClosestVertices closest = findClosestVertices(result);
    
    typename SubtractResult::iterator it = result.begin();
    typename SubtractResult::iterator end = result.end();
    while (it != end) {
        Polyhedron& fragment = *it;
        typename V::Set newFragmentVertices;
        
        const Vertex* firstVertex = fragment.vertices().front();
        const Vertex* currentVertex = firstVertex;
        
        do {
            const V& currentPosition = currentVertex->position();
            if (exclude.count(currentPosition) == 0) {
                typename ClosestVertices::const_iterator clIt = closest.find(currentPosition);
                assert(clIt != closest.end());
                const V& targetPosition = clIt->second;
                newFragmentVertices.insert(targetPosition);
            } else {
                newFragmentVertices.insert(currentPosition);
            }
            currentVertex = currentVertex->next();
        } while (currentVertex != firstVertex);

        fragment = Polyhedron(newFragmentVertices);
        if (!fragment.polyhedron()) {
            it = result.erase(it);
        } else {
            ++it;
        }
    }
}

template <typename T, typename FP>
typename Polyhedron<T,FP>::Vertex* Polyhedron<T,FP>::findMovableVertex(const Polyhedron& fragment, const typename V::Set& exclude) const {
    
    Vertex* firstVertex = fragment.vertices().front();
    Vertex* currentVertex = firstVertex;
    do {
        if (exclude.count(currentVertex->position()) == 0)
            return currentVertex;
        currentVertex = currentVertex->next();
    } while (currentVertex != firstVertex);
    return NULL;
}


#endif /* Polyhedron_Subtract_h */
