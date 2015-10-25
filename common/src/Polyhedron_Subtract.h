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
typename Polyhedron<T,FP>::SubtractResult Polyhedron<T,FP>::subtract(const Polyhedron& subtrahend, const Callback& callback) const {
    Subtract subtract(*this, subtrahend, callback);
    
    List& result = subtract.result();
    Merge merge(result, callback);
    return result;
}

template <typename T, typename FP>
class Polyhedron<T,FP>::Subtract {
private:
    const Polyhedron& m_minuend;
    Polyhedron m_subtrahend;
    const Callback& m_callback;
    Polyhedron::List m_fragments;

    typedef std::map<Vec<T,3>, Vec<T,3> > ClosestVertices;
public:
    Subtract(const Polyhedron& minuend, const Polyhedron& subtrahend, const Callback& callback) :
    m_minuend(minuend),
    m_subtrahend(subtrahend),
    m_callback(callback) {
        if (clipSubtrahend()) {
            m_fragments.push_back(m_minuend);
            chopMinuend();
            removeSubtrahend();
            simplify();
        }
    }
public:
    Polyhedron::List& result() {
        return m_fragments;
    }
private:
    bool clipSubtrahend() {
        Face* first = m_minuend.faces().front();
        Face* current = first;
        do {
            const ClipResult result = m_subtrahend.clip(m_callback.plane(current));
            if (result.empty())
                return false;
            current = current->next();
        } while (current != first);
        return true;
    }
    
    void chopMinuend() {
        const Face* firstFace = m_subtrahend.faces().front();
        const Face* currentFace = firstFace;
        do {
            const Plane<T,3> plane = m_callback.plane(currentFace);
            
            typename Polyhedron::List::iterator it, end;
            for (it = m_fragments.begin(), end = m_fragments.end(); it != end; ++it) {
                Polyhedron front = *it;
                const ClipResult clipResult = front.clip(plane);
                if (clipResult.success()) {
                    it->clip(plane.flipped());
                    it = m_fragments.insert(it, front);
                    ++it;
                }
            }
            
            currentFace = currentFace->next();
        } while (currentFace != firstFace);
    }
    
    void removeSubtrahend() {
        const typename V::List vertices = V::asList(m_subtrahend.vertices().begin(), m_subtrahend.vertices().end(), GetVertexPosition());
        
        typename Polyhedron::List::iterator it, end;
        for (it = m_fragments.begin(), end = m_fragments.end(); it != end; ++it) {
            const Polyhedron& fragment = *it;
            if (fragment.hasVertices(vertices)) {
                m_fragments.erase(it);
                break;
            }
        }
    }
    
    void simplify() {
        typename V::Set exclude;
        SetUtils::makeSet(V::asList(m_subtrahend.vertices().begin(), m_subtrahend.vertices().end(), GetVertexPosition()), exclude);
        SetUtils::makeSet(V::asList(m_minuend.vertices().begin(), m_minuend.vertices().end(), GetVertexPosition()), exclude);
        
        const ClosestVertices closest = findClosestVertices();
        
        typename Polyhedron::List::iterator it = m_fragments.begin();
        typename Polyhedron::List::iterator end = m_fragments.end();
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
                it = m_fragments.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    ClosestVertices findClosestVertices() const {
        ClosestVertices result;
        
        typename Polyhedron::List::const_iterator it, end;
        for (it = m_fragments.begin(), end = m_fragments.end(); it != end; ++it) {
            const Polyhedron& fragment = *it;
            const Vertex* firstVertex = fragment.vertices().front();
            const Vertex* currentVertex = firstVertex;
            do {
                if (result.count(currentVertex->position()) == 0) {
                    const Vertex* closestMinuendVertex = m_minuend.findClosestVertex(currentVertex->position());
                    result[currentVertex->position()] = closestMinuendVertex->position();
                }
                currentVertex = currentVertex->next();
            } while (currentVertex != firstVertex);
        }
        
        return result;
    }
};

template <typename T, typename FP>
class Polyhedron<T,FP>::Merge {
private:
    struct NeighbourEntry {
        size_t neighbour;
        Face* face;
        Face* neighbourFace;

        typedef std::set<NeighbourEntry> Set;

        NeighbourEntry(const size_t i_neighbour, Face* i_face, Face* i_neighbourFace) :
        neighbour(i_neighbour),
        face(i_face),
        neighbourFace(i_neighbourFace) {
            assert(face != NULL);
            assert(neighbourFace != NULL);
        }
        
        bool operator<(const NeighbourEntry& other) const {
            return neighbour < other.neighbour;
        }
    };
    
    typedef std::vector<typename Polyhedron::List::iterator> IndexList;
    typedef std::map<size_t, typename NeighbourEntry::Set> Neighbours;
    
    typedef std::set<size_t> MergeGroup;
    
    struct MergeGroupCmp {
    public:
        bool operator()(const MergeGroup& lhs, const MergeGroup& rhs) const {
            return compare(lhs, rhs) < 0;
        }
    private:
        int compare(const MergeGroup& lhs, const MergeGroup& rhs) const {
            if (lhs.size() < rhs.size())
                return -1;
            if (lhs.size() > rhs.size())
                return 1;
            
            typename MergeGroup::const_iterator lIt, lEnd, rIt;
            for (lIt = lhs.begin(), rIt = rhs.begin(), lEnd = lhs.end(); lIt != lEnd; ++lIt, ++rIt) {
                const size_t lIndex = *lIt;
                const size_t rIndex = *rIt;
                if (lIndex < rIndex)
                    return -1;
                if (lIndex > rIndex)
                    return 1;
            }
            
            return 0;
        }
    };
    
    typedef std::set<MergeGroup, MergeGroupCmp> MergeGroups;
    
    Polyhedron::List&  m_fragments;
    const Callback& m_callback;
    IndexList m_indices;
    Neighbours m_neighbours;
    MergeGroups m_mergeGroups;
public:
    Merge(Polyhedron::List& fragments, const Callback& callback) :
    m_fragments(fragments),
    m_callback(callback) {
        initialize();
        merge();
    }
private:
    void initialize() {
        typename Polyhedron::List::iterator it, end;
        for (it = m_fragments.begin(), end = m_fragments.end(); it != end; ++it)
            m_indices.push_back(it);
    }
    
    void merge() {
        findMergeableNeighbours();
        findMergeGroups();
        partitionMergeGroups();
        applyMergeGroups();
    }
    
    class FaceKey {
    private:
        Plane<T,3> m_plane;
        typename V::Set m_vertices;
    public:
        FaceKey(const Face* face, const Callback& callback) {
            m_plane = callback.plane(face);
            if (m_plane.distance == 0.0)
                m_plane.normal.makeAbsolute();
            else if (m_plane.distance < 0.0)
                m_plane.flip();
            
            const HalfEdge* first = face->boundary().front();
            const HalfEdge* current = first;
            do {
                m_vertices.insert(current->origin()->position());
                current = current->next();
            } while (current != first);
        }
        
        bool operator<(const FaceKey& other) const {
            return compare(other) < 0;
        }
    private:
        int compare(const FaceKey& other) const {
            const int planeCmp = m_plane.compare(other.m_plane);
            if (planeCmp != 0)
                return planeCmp;
            if (m_vertices.size() < other.m_vertices.size())
                return -1;
            if (m_vertices.size() > other.m_vertices.size())
                return 1;
            
            typename V::Set::const_iterator myIt = m_vertices.begin();
            typename V::Set::const_iterator otIt = other.m_vertices.begin();
            
            for (size_t i = 0; i < m_vertices.size(); ++i) {
                const V& myVertex = *myIt++;
                const V& otVertex = *otIt++;
                const int vertexCmp = myVertex.compare(otVertex);
                if (vertexCmp != 0)
                    return vertexCmp;
            }
            
            return 0;
        }
    };
    

    typedef std::pair<size_t, Face*> NeighbourFace;
    typedef std::vector<NeighbourFace> NeighbourFaceList;
    typedef std::map<FaceKey, NeighbourFaceList> NeighbourMap;

    void findMergeableNeighbours() {
        const NeighbourMap neighbourMap = findNeighbours();
        typename NeighbourMap::const_iterator nIt, nEnd;
        for (nIt = neighbourMap.begin(), nEnd = neighbourMap.end(); nIt != nEnd; ++nIt) {
            const NeighbourFaceList& neighbourFaces = nIt->second;
            assert(neighbourFaces.size() <= 2);
            
            if (neighbourFaces.size() == 2) {
                const NeighbourFace& first  = neighbourFaces[0];
                const NeighbourFace& second = neighbourFaces[1];

                const size_t firstIndex = first.first;
                const size_t secondIndex = second.first;
                Face* firstFace = first.second;
                Face* secondFace = second.second;
                
                if (mergeableNeighbours(secondFace, firstIndex)) {
                    m_neighbours[ firstIndex].insert(NeighbourEntry(secondIndex,  firstFace, secondFace));
                    m_neighbours[secondIndex].insert(NeighbourEntry( firstIndex, secondFace,  firstFace));
                }
            }
        }
    }
    
    NeighbourMap findNeighbours() {
        NeighbourMap result;
        
        for (size_t index = 0; index < m_indices.size(); ++index) {
            const typename Polyhedron::List::iterator fIt = m_indices[index];
            const Polyhedron& fragment = *fIt;
            Face* firstFace = fragment.faces().front();
            Face* currentFace = firstFace;
            do {
                const FaceKey key(currentFace, m_callback);
                NeighbourFaceList& neighbours = result[key];
                neighbours.push_back(std::make_pair(index, currentFace));
                currentFace = currentFace->next();
            } while (currentFace != firstFace);
        }
        
        return result;
    }
    
    bool mergeableNeighbours(const Face* sharedFace, const size_t neighbourIndex) const {
        // The two polyhedra which share the given faces can be merged if no vertex of one polyhedron is visible by any
        // other face of the other polyhedron other than the shared face.
        
        const typename Polyhedron::List::iterator pIt = m_indices[neighbourIndex];
        const Polyhedron& polyhedron = *pIt;

        const Vertex* firstVertex = polyhedron.vertices().front();
        
        const Face* currentFace = sharedFace->next(); // skip the shared face
        do {
            const Vertex* currentVertex = firstVertex;
            do {
                if (currentFace->visibleFrom(currentVertex->position()))
                    return false;
                currentVertex = currentVertex->next();
            } while (currentVertex != firstVertex);
            currentFace = currentFace->next();
        } while (currentFace != sharedFace);
        return true;
    }
    
    void findMergeGroups() {
        typename Neighbours::const_iterator nIt, nEnd;
        for (nIt = m_neighbours.begin(), nEnd = m_neighbours.end(); nIt != nEnd; ++nIt) {
            const size_t index1 = nIt->first;
            const typename NeighbourEntry::Set& entries = nIt->second;
            
            typename NeighbourEntry::Set::const_iterator eIt, eEnd;
            for (eIt = entries.begin(), eEnd = entries.end(); eIt != eEnd; ++eIt) {
                const NeighbourEntry& entry = *eIt;
                
                const size_t index2 = entry.neighbour;
                const Face* face1 = entry.face;
                const Face* face2 = entry.neighbourFace;
                
                MergeGroup group;
                group.insert(index1);
                group.insert(index2);
                
                if (m_mergeGroups.insert(group).second) {
                    expandMergeGroup(group, index1, face2);
                    expandMergeGroup(group, index2, face1);
                }
            }
        }
    }
    
    void expandMergeGroup(const MergeGroup& group, const size_t index1, const Face* face) {
        const typename Neighbours::const_iterator nIt = m_neighbours.find(index1);
        if (nIt != m_neighbours.end()) {
            MergeGroup newGroup = group;
            
            const typename NeighbourEntry::Set& entries = nIt->second;
            typename NeighbourEntry::Set::const_iterator eIt, eEnd;
            for (eIt = entries.begin(), eEnd = entries.end(); eIt != eEnd; ++eIt) {
                const NeighbourEntry& entry = *eIt;
                const size_t index2 = entry.neighbour;
                if (mergeableNeighbours(face, index2) && newGroup.insert(index2).second) {
                    m_mergeGroups.insert(newGroup);
                    expandMergeGroup(newGroup, index2, face);
                }
            }
        }
    }
    
    void partitionMergeGroups() {
        typename MergeGroups::iterator mIt = m_mergeGroups.begin();
        typename MergeGroups::iterator mCur, mEnd, tmp;
        while (mIt != m_mergeGroups.end()) {
            const MergeGroup& first = *mIt;

            mCur = mIt; mCur++;
            mEnd = m_mergeGroups.end();
            while (mCur != mEnd) {
                const MergeGroup& second = *mCur;
                MergeGroup intersection;
                MergeGroup firstMinusSecond;
                MergeGroup secondMinusFirst;
                
                SetUtils::intersection(first, second, intersection);
                if (!intersection.empty()) {
                    if (first.size() == intersection.size()) {
                        // both sets are identical or first is a subset of second, erase first and break
                        mIt = SetUtils::erase(m_mergeGroups, mIt);
                        --mIt; // will be increased at the end of the outer while loop!
                        break;
                    } else if (second.size() == intersection.size()) {
                        // second is a subset of first, erase second
                        mCur = SetUtils::erase(m_mergeGroups, mCur);
                    } else {
                        // the groups must be partitioned properly
                        SetUtils::minus(first, intersection, firstMinusSecond);
                        SetUtils::minus(second, intersection, secondMinusFirst);
                        
                        // erase both first and second
                        SetUtils::erase(m_mergeGroups, mCur);
                        mIt = SetUtils::erase(m_mergeGroups, mIt);
                        --mIt; // will be increased at the end of the outer while loop!
                        
                        // insert the new merge groups and break
                        m_mergeGroups.insert(intersection);
                        m_mergeGroups.insert(firstMinusSecond);
                        m_mergeGroups.insert(secondMinusFirst);
                        break;
                    }
                } else {
                    ++mCur;
                }
            }
            ++mIt;
        }
    }
    
    void applyMergeGroups() {
        typename MergeGroups::const_iterator mIt, mEnd;
        typename MergeGroup::const_iterator gIt, gEnd;
        typename Polyhedron::List::iterator fIt;
        
        for (mIt = m_mergeGroups.begin(), mEnd = m_mergeGroups.end(); mIt != mEnd; ++mIt) {
            const MergeGroup& group = *mIt;
            if (group.size() > 1) {
                gIt = group.begin();
                gEnd = group.end();
                
                Polyhedron& master = *m_indices[*gIt++];
                while (gIt != gEnd) {
                    fIt = m_indices[*gIt++];
                    master.merge(*fIt);
                    m_fragments.erase(fIt);
                }
            }
        }
    }
};

#endif /* Polyhedron_Subtract_h */
