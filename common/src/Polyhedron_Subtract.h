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
    
    FragmentList fragments = createInitialFragments(subtrahend, callback);
    Fragment::partitionFragments(fragments);
    
    // Create missing fragments
    
    // Create result
    SubtractResult result;
    for (const Fragment& fragment : fragments) {
        const Polyhedron& polyhedron = fragment.polyhedron();
        if (polyhedron.polyhedron())
            result.push_back(polyhedron);
    }
    
    return result;
    
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::FragmentList Polyhedron<T,FP,VP>::createInitialFragments(const Polyhedron& subtrahend, const Callback& callback) const {
    FragmentList result;
    
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
        
        result.push_back(Fragment(vertices));
        
        curFace = curFace->next();
    } while (curFace != firstFace);
    
    return result;
}

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Fragment {
private:
    typedef typename V::Set VertexSet;
    typedef std::set<Plane<T,3>> PlaneSet;
    
    VertexSet m_vertices;
public:
    struct SubsetCmp {
        bool operator()(const Fragment& lhs, const Fragment& rhs) const {
            return SetUtils::subset(lhs.m_vertices, rhs.m_vertices);
        }
    };
public:
    template <typename C>
    Fragment(const C& vertices) :
    m_vertices(vertices) {}
    
    bool operator<(const Fragment& other) const {
        return m_vertices < other.m_vertices;
    }
    
    void removeVertex(const V& position) {
        m_vertices.erase(position);
    }
    
    size_t vertexCount() const {
        return m_vertices.size();
    }
    
    Polyhedron polyhedron() const {
        return Polyhedron(m_vertices);
    }
    
    VertexSet intersectVertices(const Fragment& other) const {
        return SetUtils::intersection(m_vertices, other.m_vertices);
    }
    
    static void partitionFragments(FragmentList& fragments) {
        auto it1 = std::begin(fragments);
        while (it1 != std::end(fragments)) {
            auto it2 = std::next(it1);
            bool incIt1 = true;
            while (it2 != std::end(fragments)) {
                const VertexSet sharedVertices = it1->intersectVertices(*it2);
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
                
                const Polyhedron<T,FP,VP> sharedVolume(sharedVertices);
                if (!sharedVolume.polyhedron()) {
                    ++it2;
                    continue;
                }
                
                // The two fragments overlap. Remove the overlapping areas and add a new fragment for the overlap.
                removeOverlap(*it1, sharedVertices);
                removeOverlap(*it2, sharedVertices);
                fragments.push_back(Fragment(sharedVertices));
                
                ++it2;
            }
            
            if (incIt1)
                ++it1;
        }
    }
    
    static void removeOverlap(Fragment& fragment, const VertexSet& sharedVertices) {
        const Polyhedron& polyhedron = fragment.polyhedron();
        for (const V& position : sharedVertices) {
            const Vertex* vertex = polyhedron.findVertexByPosition(position);
            assert(vertex != nullptr);
            
            if (fullyShared(vertex, sharedVertices))
                fragment.removeVertex(position);
        }
    }
    
    /*
     Checks whether the given vertex is connected only to vertices in the given set.
     */
    static bool fullyShared(const Vertex* vertex, const VertexSet& sharedVertices) {
        HalfEdge* firstEdge = vertex->leaving();
        HalfEdge* curEdge = firstEdge;
        do {
            const Vertex* destination = curEdge->destination();
            if (sharedVertices.count(destination->position()) == 0)
                return false;
            curEdge = curEdge->nextIncident();
        } while (curEdge != firstEdge);
        
        return true;
    }
};

#endif /* Polyhedron_Subtract_h */
