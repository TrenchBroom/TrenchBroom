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

#ifndef Polyhedron_Subtract_h
#define Polyhedron_Subtract_h

#include <iterator>

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::SubtractResult Polyhedron<T,FP,VP>::subtract(const Polyhedron& subtrahend) const {
    Callback c;
    return subtract(subtrahend, c);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::SubtractResult Polyhedron<T,FP,VP>::subtract(Polyhedron subtrahend, Callback& callback) const {
    
    if (!subtrahend.clip(*this, callback).success())
        return SubtractResult();
    
    List fragments = createInitialFragments(subtrahend, callback);
    partitionFragments(fragments);
    
    // Create missing fragments
    
    // Create result
    SubtractResult result;
    for (const Polyhedron& fragment : fragments) {
        if (fragment.polyhedron())
            result.push_back(fragment);
    }
    
    return result;
    
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::List Polyhedron<T,FP,VP>::createInitialFragments(const Polyhedron& subtrahend, const Callback& callback) const {
    List result;
    
    Face* firstFace = subtrahend.faces().front();
    Face* curFace = firstFace;
    do {
        typename V::Set vertices = curFace->vertexPositionSet();
        const Plane<T,3> boundary = callback.plane(curFace);
        
        Vertex* firstVertex = m_vertices.front();
        Vertex* curVertex = firstVertex;
        do {
            const V& position = curVertex->position();
            if (boundary.pointStatus(position) != Math::PointStatus::PSBelow)
                vertices.insert(position);
            
            curVertex = curVertex->next();
        } while (curVertex != firstVertex);
        
        result.push_back(Polyhedron(vertices));
        
        curFace = curFace->next();
    } while (curFace != firstFace);
    
    return result;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::partitionFragments(List& fragments) {
    auto it1 = std::begin(fragments);
    while (it1 != std::end(fragments)) {
        const typename V::Set f1Vertices = it1->vertexPositionSet();
        
        auto it2 = std::next(it1);
        bool incIt1 = true;
        while (it2 != std::end(fragments)) {
            const typename V::Set f2Vertices = it2->vertexPositionSet();
            const typename V::Set sharedVertices = SetUtils::intersection(f1Vertices, f2Vertices);
            
            if (sharedVertices.size() < 4) {
                ++it2;
                continue;
            }
            
            if (sharedVertices.size() == it1->vertexCount()) {
                it1 = fragments.erase(it1);
                incIt1 = false;
                break;
            }
            
            if (sharedVertices.size() == it2->vertexCount()) {
                it2 = fragments.erase(it2);
                continue;
            }
            
            const Polyhedron sharedVolume(sharedVertices);
            if (!sharedVolume.polyhedron()) {
                ++it2;
                continue;
            }
            
            // The two fragments overlap. Remove the overlapping areas and add a new fragment for the overlap.
            removeFragmentOverlap(*it1, sharedVertices);
            removeFragmentOverlap(*it2, sharedVertices);
            fragments.push_back(sharedVolume);
            
            ++it2;
        }
        
        if (incIt1)
            ++it1;
    }
}

/*
 Attempts to remove the vertices in the given set from the given fragment. A vertex is only actually removed if each
 of its adjacent vertices is also in the given set.
 */
template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::removeFragmentOverlap(Polyhedron& fragment, const typename V::Set& vertices) {
    for (const auto& position : vertices) {
        auto* vertex = fragment.findVertexByPosition(position);
        assert(vertex != nullptr);
        
        if (checkAllNeighboursInSet(vertex, vertices))
            fragment.removeVertex(vertex);
    }
}

/*
 Checks whether the given vertex is connected only to vertices in the given set.
 */
template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::checkAllNeighboursInSet(const Vertex* vertex, const typename V::Set& vertices) {
    typename Polyhedron::HalfEdge* firstEdge = vertex->leaving();
    typename Polyhedron::HalfEdge* curEdge = firstEdge;
    do {
        const typename Polyhedron::Vertex* destination = curEdge->destination();
        if (vertices.count(destination->position()) == 0)
            return false;
        curEdge = curEdge->nextIncident();
    } while (curEdge != firstEdge);
    
    return true;
}

#endif /* Polyhedron_Subtract_h */
