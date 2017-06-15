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

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::SubtractResult Polyhedron<T,FP,VP>::subtract(const Polyhedron& subtrahend) const {
    Callback c;
    return subtract(subtrahend, c);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::SubtractResult Polyhedron<T,FP,VP>::subtract(Polyhedron subtrahend, Callback& callback) const {
    
    if (!subtrahend.clip(*this, callback).success())
        return SubtractResult();
    
    // Create maximal fragments
    const FragmentSet maximalFragments = createMaximalFragments(subtrahend, callback);
    
    // Make disjoint
    // Create result
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::FragmentSet Polyhedron<T,FP,VP>::createMaximalFragments(const Polyhedron& subtrahend, const Callback& callback) const {
    return maximizeFragments(createInitialFragments(subtrahend, callback), callback);
}


template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::FragmentSet Polyhedron<T,FP,VP>::createInitialFragments(const Polyhedron& subtrahend, const Callback& callback) const {
    FragmentSet result;
    
    Face* firstFace = subtrahend.faces().front();
    Face* curFace = firstFace;
    do {
        const auto powerSet = SetUtils::powerSet(curFace->vertexSet());
        for (const auto& vertices : powerSet)
            result.insert(Fragment(vertices, callback));
        
        curFace = curFace->next();
    } while (curFace != firstFace);
    
    return result;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::FragmentSet Polyhedron<T,FP,VP>::maximizeFragments(const FragmentSet& fragments, const Callback& callback) const {
    FragmentSet result;
}

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Fragment {
private:
    typedef typename V::Set VertexSet;
    typedef std::set<Plane<T,3>> PlaneSet;
    
    VertexSet m_vertices;
    PlaneSet m_planes;
public:
    template <typename C>
    Fragment(const C& vertices, const Callback& callback) {
        addVertices(vertices, callback);
    }
    
    template <typename C>
    void addVertices(const C& vertices, const Callback& callback) {
        for (const Vertex* vertex : vertices)
            addVertex(vertex, callback);
    }
    
    void addVertex(const Vertex* vertex, const Callback& callback) {
        m_vertices.insert(vertex->position());
        m_planes = SetUtils::intersection(m_planes, computePlanes(vertex, callback));
    }
    
    bool operator<(const Fragment& other) const {
        return m_vertices < other.m_vertices;
    }
private:
    PlaneSet computePlanes(const Vertex* vertex, const Callback& callback) const {
        PlaneSet result;
        
        const HalfEdge* firstEdge = vertex->leaving();
        const HalfEdge* curEdge = firstEdge;
        do {
            const Face* face = curEdge->face();
            result.insert(callback.plane(face));
            curEdge = curEdge->nextIncident();
        } while (curEdge != firstEdge);
        
        return result;
    }
};

#endif /* Polyhedron_Subtract_h */
