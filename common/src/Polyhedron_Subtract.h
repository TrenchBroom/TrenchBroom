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
    // Merge merge(result, callback);
    return result;
}

template <typename T, typename FP>
class Polyhedron<T,FP>::Subtract {
private:
    const Polyhedron& m_minuend;
    Polyhedron m_subtrahend;
    const Callback& m_callback;
    typename Polyhedron::List m_fragments;

    typedef typename V::LexicographicOrder VertexCmp;
    typedef std::set<V, VertexCmp> ExcludedVertices;
    typedef std::map<V, V, VertexCmp> ClosestVertices;
    
    typedef std::vector<typename Polyhedron::List::iterator> IterList;
    typedef std::map<V, IterList, VertexCmp> MoveableVertices;
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
    
    typedef std::set<typename V::Set, VertexSetCmp> NewFragments;

    void simplify() {
        NewFragments newFragments = buildNewFragments();
        if (!newFragments.empty()) {
            removeDuplicateFragments(newFragments);
            rebuildFragments(newFragments);
        }
    }
    
    NewFragments buildNewFragments() {
        NewFragments result;
        
        typedef std::set<V, VertexCmp> ExcludeSet;
        ExcludeSet exclude(VertexCmp(0.1));
        // typename V::Set exclude;
        SetUtils::makeSet(V::asList(m_subtrahend.vertices().begin(), m_subtrahend.vertices().end(), GetVertexPosition()), exclude);
        SetUtils::makeSet(V::asList(m_minuend.vertices().begin(), m_minuend.vertices().end(), GetVertexPosition()), exclude);
        
        const ClosestVertices closest = findClosestVertices();
        if (closest.empty())
            return result;
        
        typename Polyhedron::List::iterator it, end;
        for (it = m_fragments.begin(), end = m_fragments.end(); it != end; ++it) {
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
            
            result.insert(newFragmentVertices);
        }
        
        return result;
    }
    
    ClosestVertices findClosestVertices() {
        ClosestVertices result(VertexCmp(0.1));

        const MoveableVertices moveableVertices = findMoveableVertices();
        typename MoveableVertices::const_iterator mIt, mEnd;
        for (mIt = moveableVertices.begin(), mEnd = moveableVertices.end(); mIt != mEnd; ++mIt) {
            const V& vertexPosition = mIt->first;
            const IterList& fragments = mIt->second;
            V targetPosition;
            if (!selectTargetPosition(vertexPosition, fragments, targetPosition))
                return ClosestVertices(VertexCmp(0.1));
            result[vertexPosition] = targetPosition;
        }
        
        return result;
    }
    
    MoveableVertices findMoveableVertices() {
        const ExcludedVertices exclude = findExcludedVertices();
        MoveableVertices result(VertexCmp(0.1));
        typename Polyhedron::List::iterator it, end;
        for (it = m_fragments.begin(), end = m_fragments.end(); it != end; ++it) {
            const Polyhedron& fragment = *it;
            const Vertex* firstVertex = fragment.vertices().front();
            const Vertex* currentVertex = firstVertex;
            do {
                const V& currentPosition = currentVertex->position();
                if (exclude.count(currentPosition) == 0)
                    result[currentPosition].push_back(it);
                currentVertex = currentVertex->next();
            } while (currentVertex != firstVertex);
        }
        return result;
    }
    
    ExcludedVertices findExcludedVertices() const {
        ExcludedVertices result(VertexCmp(0.1));
        SetUtils::makeSet(V::asList(m_subtrahend.vertices().begin(), m_subtrahend.vertices().end(), GetVertexPosition()), result);
        SetUtils::makeSet(V::asList(m_minuend.vertices().begin(), m_minuend.vertices().end(), GetVertexPosition()), result);
        return result;
    }

    bool selectTargetPosition(const V& originalPosition, const IterList& incidentFragments, V& targetPosition) const {
        const Polyhedron::ClosestVertexSet closestVertices = m_minuend.findClosestVertices(originalPosition);
        
        typename Polyhedron::ClosestVertexSet::const_iterator it, end;
        for (it = closestVertices.begin(), end = closestVertices.end(); it != end; ++it) {
            targetPosition = (*it)->position();
            if (checkValidTargetPosition(originalPosition, targetPosition, incidentFragments))
                return true;
        }
        return false;
    }
    
    bool checkValidTargetPosition(const V& originalPosition, const V& targetPosition, const IterList& incidentFragments) const {
        typename IterList::const_iterator it, end;
        for (it = incidentFragments.begin(), end = incidentFragments.end(); it != end; ++it) {
            const Polyhedron& fragment = **it;
            if (!checkValidTargetPosition(originalPosition, targetPosition, fragment))
                return false;
        }
        return true;
    }
    
    bool checkValidTargetPosition(const V& originalPosition, const V& targetPosition, const Polyhedron& incidentFragment) const {
        const VertexSet subtrahendVertices = findSubtrahendVertices(incidentFragment);
        if (subtrahendVertices.empty())
            return true;
        
        const FaceSet incidentFaces = findCommonIncidentFaces(subtrahendVertices);
        
        typename FaceSet::const_iterator it, end;
        for (it = incidentFaces.begin(), end = incidentFaces.end(); it != end; ++it) {
            const Face* incidentFace = *it;
            const Plane<T,3> plane = m_callback.plane(incidentFace);
            if (plane.pointStatus(originalPosition) != Math::PointStatus::PSBelow &&
                plane.pointStatus(targetPosition) == Math::PointStatus::PSBelow)
                return false;
        }
        return true;
    }
    
    VertexSet findSubtrahendVertices(const Polyhedron& fragment) const {
        VertexSet result;
        
        Vertex* firstFragmentVertex = fragment.vertices().front();
        Vertex* currentFragmentVertex = firstFragmentVertex;
        do {
            if (m_subtrahend.hasVertex(currentFragmentVertex->position(), 0.1))
                result.insert(currentFragmentVertex);
            currentFragmentVertex = currentFragmentVertex->next();
        } while (currentFragmentVertex != firstFragmentVertex);
        
        return result;
    }
    
    FaceSet findCommonIncidentFaces(const VertexSet& vertices) const {
        typename VertexSet::iterator it = vertices.begin();
        typename VertexSet::iterator end = vertices.end();
        
        FaceSet result = incidentFaces(*it);
        while (++it != end) {
            FaceSet temp;
            SetUtils::intersection(result, incidentFaces(*it), temp);
            
            using std::swap;
            swap(result, temp);
        }
        return result;
    }
    
    FaceSet incidentFaces(const Vertex* vertex) const {
        FaceSet result;
        
        HalfEdge* firstEdge = vertex->leaving();
        HalfEdge* currentEdge = firstEdge;
        do {
            result.insert(currentEdge->face());
            currentEdge = currentEdge->nextIncident();
        } while (currentEdge != firstEdge);
        return result;
    }
    
    void removeDuplicateFragments(NewFragments& newFragments) const {
        NewFragments result;
        const typename NewFragments::iterator end = newFragments.end();
        while (!newFragments.empty()) {
            typename NewFragments::iterator lIt = newFragments.begin();
            typename NewFragments::iterator rIt = newFragments.begin(); ++rIt;
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
    
    void rebuildFragments(const NewFragments& newFragments) {
        m_fragments.clear();
        
        typename NewFragments::const_iterator it, end;
        for (it = newFragments.begin(), end = newFragments.end(); it != end; ++it) {
            const typename V::Set& vertices = *it;
            const Polyhedron fragment(vertices);
            if (fragment.polyhedron())
                m_fragments.push_back(fragment);
        }
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
