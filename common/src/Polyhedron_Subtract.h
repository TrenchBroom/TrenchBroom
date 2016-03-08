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

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::SubtractResult Polyhedron<T,FP,VP>::subtract(const Polyhedron& subtrahend) const {
    Callback c;
    return subtract(subtrahend, c);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::SubtractResult Polyhedron<T,FP,VP>::subtract(const Polyhedron& subtrahend, const Callback& callback) const {
    Subtract subtract(*this, subtrahend, callback);
    
    List& result = subtract.result();
    Merge merge(result, callback);
    return result;
}

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Subtract {
private:
    const Polyhedron& m_minuend;
    Polyhedron m_subtrahend;
    const Callback& m_callback;
    typename Polyhedron::List m_fragments;
    
    typedef typename V::LexicographicOrder VertexCmp;
    typedef std::set<V, VertexCmp> PositionSet;
    typedef std::map<V, V, VertexCmp> ClosestVertices;
    typedef std::map<V, V, VertexCmp> MoveableVertices;
    
    class VertexSetCmp {
    public:
        bool operator()(const typename V::Set& lhs, const typename V::Set& rhs) const {
            return compare(lhs, rhs) < 0;
        }
    private:
        int compare(const typename V::Set& lhs, const typename V::Set& rhs) const {
            if (lhs.size() < rhs.size())
                return -1;
            if (lhs.size() > rhs.size())
                return 1;
            
            typename V::Set::const_iterator lIt = lhs.begin();
            typename V::Set::const_iterator rIt = rhs.begin();
            for (size_t i = 0; i < lhs.size(); ++i) {
                const V& lPos = *lIt++;
                const V& rPos = *rIt++;
                
                const int cmp = lPos.compare(rPos);
                if (cmp != 0)
                    return cmp;
            }
            return 0;
        }
    };
    
    typedef std::set<typename V::Set, VertexSetCmp> FragmentVertexSet;
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
    typename Polyhedron::List& result() {
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
            if (fragment.hasVertices(vertices, 0.1)) {
                m_fragments.erase(it);
                return;
            }
        }
        assert(false);
    }
    
    void simplify() {
        FragmentVertexSet newFragments = buildNewFragments();
        removeDuplicateFragments(newFragments);
        rebuildFragments(newFragments);
    }
    
    FragmentVertexSet buildNewFragments() {
        FragmentVertexSet result;
        
        const ClosestVertices closest = findClosestVertices();
        if (closest.empty())
            return findFragmentVertices();
        
        typename Polyhedron::List::iterator it, end;
        for (it = m_fragments.begin(), end = m_fragments.end(); it != end; ++it) {
            Polyhedron& fragment = *it;
            typename V::Set newFragmentVertices;
            
            const Vertex* firstVertex = fragment.vertices().front();
            const Vertex* currentVertex = firstVertex;
            
            do {
                const V& currentPosition = currentVertex->position();
                typename ClosestVertices::const_iterator clIt = closest.find(currentPosition);
                if (clIt != closest.end()) {
                    const V& targetPosition = clIt->second;
                    newFragmentVertices.insert(targetPosition);
                } else {
                    newFragmentVertices.insert(currentPosition);
                }
                currentVertex = currentVertex->next();
            } while (currentVertex != firstVertex);
            
            result.insert(newFragmentVertices);
        }
        
        return result;
    }
    
    ClosestVertices findClosestVertices() {
        MoveableVertices moveableVertices = findMoveableVertices();
        FragmentVertexSet fragmentVertices = findFragmentVertices();
        return findClosestVertices(moveableVertices, fragmentVertices);
    }
    
    MoveableVertices findMoveableVertices() const {
        const PositionSet exclude = findExcludedVertices();
        MoveableVertices result(VertexCmp(0.1));
        
        typename Polyhedron::List::const_iterator it, end;
        for (it = m_fragments.begin(), end = m_fragments.end(); it != end; ++it) {
            const Polyhedron& fragment = *it;
            findMoveableVertices(fragment, exclude, result);
        }
        
        return result;
    }
    
    PositionSet findExcludedVertices() const {
        PositionSet result(VertexCmp(0.1));
        SetUtils::makeSet(V::asList(m_subtrahend.vertices().begin(), m_subtrahend.vertices().end(), GetVertexPosition()), result);
        SetUtils::makeSet(V::asList(m_minuend.vertices().begin(), m_minuend.vertices().end(), GetVertexPosition()), result);
        return result;
    }
    
    void findMoveableVertices(const Polyhedron& fragment, const PositionSet& exclude, MoveableVertices& result) const {
        const Vertex* firstVertex = fragment.vertices().front();
        const Vertex* currentVertex = firstVertex;
        do {
            const V& currentPosition = currentVertex->position();
            if (exclude.count(currentPosition) == 0 && result.count(currentPosition) == 0)
                result.insert(std::make_pair(currentPosition, m_minuend.findClosestVertex(currentPosition)->position()));
            currentVertex = currentVertex->next();
        } while (currentVertex != firstVertex);
    }
    
    FragmentVertexSet findFragmentVertices() const {
        FragmentVertexSet result;
        
        typename Polyhedron::List::const_iterator it, end;
        for (it = m_fragments.begin(), end = m_fragments.end(); it != end; ++it) {
            const Polyhedron& fragment = *it;
            typename V::Set vertices(VertexCmp(0.1));
            
            const Vertex* firstVertex = fragment.vertices().front();
            const Vertex* currentVertex = firstVertex;
            do {
                vertices.insert(currentVertex->position());
                currentVertex = currentVertex->next();
            } while (currentVertex != firstVertex);
            
            result.insert(vertices);
        }
        
        return result;
    }
    
    ClosestVertices findClosestVertices(const MoveableVertices& vertices, FragmentVertexSet& fragments) {
        ClosestVertices result;
        
        typename MoveableVertices::const_iterator vIt, vEnd;
        for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt) {
            const V& vertexPosition = vIt->first;
            const V& targetPosition = vIt->second;
            
            if (applyVertexMove(vertexPosition, targetPosition, fragments))
                result[vertexPosition] = targetPosition;
        }
        
        return result;
    }
    
    bool applyVertexMove(const V& vertexPosition, const V& targetPosition, FragmentVertexSet& fragments) {
        FragmentVertexSet newFragments;
        typename FragmentVertexSet::const_iterator fIt,fEnd;
        for (fIt = fragments.begin(), fEnd = fragments.end(); fIt != fEnd; ++fIt) {
            typename V::Set newVertices = *fIt;
            
            if (newVertices.erase(vertexPosition) > 0) {
                newVertices.insert(targetPosition);
                
                Polyhedron newFragment(newVertices);
                if (newFragment.polyhedron()) {
                    if (newFragment.intersects(m_subtrahend))
                        return false;
                }
            }
            newFragments.insert(newVertices);
        }
        
        using std::swap;
        swap(fragments, newFragments);
        
        return true;
    }
    
    bool containsIntersectingFragments(const List& fragments) const {
        typename List::const_iterator first, second, end;
        for (first = fragments.begin(), end = fragments.end(); first != end; ++first) {
            for (second = first, std::advance(second, 1); second != end; ++second) {
                if (first->intersects(*second))
                    return true;
            }
        }
        return false;
    }
    
    void removeDuplicateFragments(FragmentVertexSet& newFragments) const {
        FragmentVertexSet result;
        const typename FragmentVertexSet::iterator end = newFragments.end();
        while (!newFragments.empty()) {
            typename FragmentVertexSet::iterator lIt = newFragments.begin();
            typename FragmentVertexSet::iterator rIt = newFragments.begin(); ++rIt;
            while (lIt != end && rIt != end) {
                if (SetUtils::subset(*lIt, *rIt)) {
                    newFragments.erase(lIt);
                    lIt = end;
                } else if (SetUtils::subset(*rIt, *lIt)) {
                    rIt = SetUtils::erase(newFragments, rIt);
                } else {
                    ++rIt;
                }
            }
            if (lIt != end) {
                result.insert(*lIt);
                newFragments.erase(lIt);
            }
        }
        
        using std::swap;
        swap(newFragments, result);
    }
    
    void rebuildFragments(const FragmentVertexSet& newFragments) {
        m_fragments.clear();
        
        typename FragmentVertexSet::const_iterator it, end;
        for (it = newFragments.begin(), end = newFragments.end(); it != end; ++it) {
            const typename V::Set& vertices = *it;
            if (vertices.size() > 3) {
                const Polyhedron fragment(vertices);
                if (fragment.polyhedron())
                    m_fragments.push_back(fragment);
            }
        }
    }
};

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Partition {
private:
    List& m_fragments;
public:
    Partition(List& fragments) :
    m_fragments(fragments) {}
    
};

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Merge {
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
    
    typename Polyhedron::List&  m_fragments;
    const Callback& m_callback;
    IndexList m_indices;
    Neighbours m_neighbours;
    MergeGroups m_mergeGroups;
public:
    Merge(typename Polyhedron::List& fragments, const Callback& callback) :
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
        typename V::Set m_vertices;
    public:
        FaceKey(const Face* face) {
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
                
                if (mergeableNeighbours(secondFace, *m_indices[firstIndex])) {
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
                const FaceKey key(currentFace);
                NeighbourFaceList& neighbours = result[key];
                assert(neighbours.size() < 2);
                neighbours.push_back(std::make_pair(index, currentFace));
                currentFace = currentFace->next();
            } while (currentFace != firstFace);
        }
        
        return result;
    }
    
    bool mergeableNeighbours(const Face* sharedFace, const Polyhedron& neighbour) const {
        // The two polyhedra which share the given faces can be merged if no vertex of one polyhedron is visible by any
        // other face of the other polyhedron other than the shared face.
        
        const Vertex* firstVertex = neighbour.vertices().front();
        
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
                
                MergeGroup group;
                group.insert(index1);
                group.insert(index2);
                
                const Polyhedron polyhedron = mergeGroup(group);
                
                if (m_mergeGroups.count(group) == 0 &&
                    !expandMergeGroup(group, polyhedron, index1) &&
                    !expandMergeGroup(group, polyhedron, index2))
                    m_mergeGroups.insert(group);
            }
        }
    }
    
    bool expandMergeGroup(const MergeGroup& group, const Polyhedron& polyhedron, const size_t index1) {
        const typename Neighbours::const_iterator nIt = m_neighbours.find(index1);
        if (nIt == m_neighbours.end())
            return false;
        
        bool didExpand = false;
        const typename NeighbourEntry::Set& entries = nIt->second;
        typename NeighbourEntry::Set::const_iterator eIt, eEnd;
        for (eIt = entries.begin(), eEnd = entries.end(); eIt != eEnd; ++eIt) {
            MergeGroup newGroup = group;
            
            const NeighbourEntry& entry = *eIt;
            const size_t index2 = entry.neighbour;
            const Face* neighbourFace = entry.neighbourFace;
            
            if (mergeableNeighbours(neighbourFace, polyhedron) && newGroup.insert(index2).second) {
                Polyhedron newPolyhedron = polyhedron;
                newPolyhedron.merge(*m_indices[index2]);
                
                if (m_mergeGroups.count(newGroup) == 0 &&
                    !expandMergeGroup(newGroup, newPolyhedron, index2)) {
                    m_mergeGroups.insert(newGroup);
                    didExpand = true;
                }
            }
        }
        return didExpand;
    }
    
    Polyhedron mergeGroup(const MergeGroup& group) const {
        MergeGroup::const_iterator it = group.begin();
        MergeGroup::const_iterator end = group.end();
        
        Polyhedron result = *m_indices[*it];
        ++it;
        while (it != end) {
            result.merge(*m_indices[*it]);
            ++it;
        }
        
        return result;
    }
    
    void partitionMergeGroups() {
        MergeGroups newMergeGroups;
        typename MergeGroups::iterator mFirst, mSecond;
        while (!m_mergeGroups.empty()) {
            mFirst = m_mergeGroups.begin();
            mSecond = mFirst; ++mSecond;
            
            const MergeGroup& first = *mFirst;
            bool firstIsDisjoint = true;
            
            while (mSecond != m_mergeGroups.end()) {
                const MergeGroup& second = *mSecond;
                MergeGroup intersection;
                
                SetUtils::intersection(first, second, intersection);
                if (!intersection.empty()) {
                    firstIsDisjoint = false;
                    if (first.size() == intersection.size()) {
                        // both sets are identical or first is a subset of second, erase first and break
                        m_mergeGroups.erase(mFirst);
                    } else if (second.size() == intersection.size()) {
                        // second is a subset of first, erase second and break
                        m_mergeGroups.erase(mSecond);
                    } else {
                        // the groups must be partitioned properly
                        MergeGroup firstMinusSecond;
                        MergeGroup secondMinusFirst;
                        SetUtils::minus(first, intersection, firstMinusSecond);
                        SetUtils::minus(second, intersection, secondMinusFirst);
                        
                        // erase both first and second
                        m_mergeGroups.erase(mFirst);
                        m_mergeGroups.erase(mSecond);
                        
                        // insert the new merge groups and break
                        m_mergeGroups.insert(intersection);
                        m_mergeGroups.insert(firstMinusSecond);
                        m_mergeGroups.insert(secondMinusFirst);
                    }
                    break;
                } else {
                    ++mSecond;
                }
            }
            
            if (firstIsDisjoint) {
                newMergeGroups.insert(first);
                m_mergeGroups.erase(mFirst);
            }
        }
        
        using std::swap;
        std::swap(m_mergeGroups, newMergeGroups);
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
