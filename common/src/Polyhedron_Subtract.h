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
typename Polyhedron<T,FP,VP>::SubtractResult Polyhedron<T,FP,VP>::subtract(const Polyhedron& subtrahend, const Callback& callback) const {
    Subtract subtract(*this, subtrahend, callback);
    
    List& result = subtract.result();
    Merge merge(result, callback);
    return result;
}

/**
 The subtraction algorithm performs four steps.
 
 1. Clip the subtrahend into the minuend so that it is entirely contained in the minuend. If the subtrahend becomes empty, stop.
 2. Chop the minuend by clipping it with every face of the subtrahend. This creates a set of fragments.
 3. Remove the fragment that is identical to the original subtrahend.
 4. Simplify the fragments by attempting to move all of their vertices onto vertices of the original minuend and subtrahend.
 */
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
            
            typename V::Set::const_iterator lIt = std::begin(lhs);
            typename V::Set::const_iterator rIt = std::begin(rhs);
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
    /**
     Clips the subrahend into the minuend.
    
     Returns true if the subtrahend is not empty, and false otherwise.
     */
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
    
    /**
     Chop up the minuend by clipping it with every face of the subtrahend. This creates fragments.
     */
    void chopMinuend() {
        const Face* firstFace = m_subtrahend.faces().front();
        const Face* currentFace = firstFace;
        do {
            const Plane<T,3> plane = m_callback.plane(currentFace);
            
            for (auto it = std::begin(m_fragments), end = std::end(m_fragments); it != end; ++it) {
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
    
    /**
     Remove the fragment that is identical to the subtrahend.
     */
    void removeSubtrahend() {
        const typename V::List vertices = V::asList(std::begin(m_subtrahend.vertices()), std::end(m_subtrahend.vertices()), GetVertexPosition());
        
        for (auto it = std::begin(m_fragments), end = std::end(m_fragments); it != end; ++it) {
            const Polyhedron& fragment = *it;
            if (fragment.hasVertices(vertices, 0.1)) {
                m_fragments.erase(it);
                return;
            }
        }
        assert(false);
    }
    
    /**
     Simplify the fragments by moving their vertices onto the vertices of the original minuend and subtrahend, if possible.
     
     We call the vertices of the original minuend or subtrahend the original vertices. All other vertices are called the new vertices.
     */
    void simplify() {
        moveAdditionalVertices();
        removeDuplicateFragments();
    }
    
    typedef std::tuple<V, V> VertexMove;
    typedef std::list<VertexMove> VertexMoveList;
    
    class Fragment {
    public:
        typedef std::list<Fragment> List;
    private:
        typename Polyhedron::List::iterator m_it;
    public:
        Fragment(const typename Polyhedron::List::iterator it) :
        m_it(it) {}
        
        VertexMoveList findVertexMoves(const PositionSet& excluded, const PositionSet& minuendVertices) const {
            VertexMoveList result;
            
            const Polyhedron& fragment = *m_it;
            if (fragment.polyhedron()) {
                const Vertex* firstVertex = fragment.vertices().front();
                const Vertex* currentVertex = firstVertex;
                do {
                    if (excluded.count(currentVertex->position()) == 0) {
                        // Not every move is valid. If the fragments form a concave volume, we cannot just always perform these moves.
                        
                        const Vertex* destination = findClosestIncidentMinuendVertex(currentVertex, minuendVertices);
                        if (destination != nullptr)
                            result.push_back(std::make_tuple(currentVertex->position(), destination->position()));
                    }
                    currentVertex = currentVertex->next();
                } while (currentVertex != firstVertex);
            }
            
            return result;
        }
        
        const Vertex* findClosestIncidentMinuendVertex(const Vertex* vertex, const PositionSet& minuendVertices) const {
            T closestDistance2 = std::numeric_limits<T>::max();
            const Vertex* closestMinuendVertex = nullptr;
            
            const HalfEdge* firstEdge = vertex->leaving();
            const HalfEdge* currentEdge = firstEdge;
            do {
                const Vertex* destination = currentEdge->destination();
                const V& position = destination->position();
                if (minuendVertices.count(position) != 0) {
                    const T distance2 = position.squaredDistanceTo(vertex->position());
                    if (distance2 < closestDistance2)
                        closestMinuendVertex = destination;
                }
                currentEdge = currentEdge->nextIncident();
            } while (currentEdge != firstEdge);
            return closestMinuendVertex;
        }
    };
    
    void moveAdditionalVertices() {
        const PositionSet subtrahendVertices = getVertexPositionsAsSet(m_subtrahend);
        const PositionSet minuendVertices = getVertexPositionsAsSet(m_minuend);
        const PositionSet excludedVertices = SetUtils::merge(subtrahendVertices, minuendVertices);
        
        bool progress = true;
        while (progress) {
            progress = false;
            typename Fragment::List workList = buildWorkList(excludedVertices);
            for (const Fragment& fragment : workList) {
                VertexMoveList vertexMoves = fragment.findVertexMoves(excludedVertices, minuendVertices);
                for (const VertexMove& vertexMove : vertexMoves) {
                    progress = true;
                    V from, to;
                    std::tie(from, to) = vertexMove;
                    applyVertexMove(from, to);
                }
            }
        }
    }
    
    PositionSet getVertexPositionsAsSet(const Polyhedron& fragment) {
        PositionSet result(VertexCmp(0.1));
        SetUtils::makeSet(V::asList(std::begin(fragment.vertices()), std::end(fragment.vertices()), GetVertexPosition()), result);
        return result;
    }
    
    typename Fragment::List buildWorkList(const PositionSet& excluded) {
        typename Fragment::List result;
        for (auto it = std::begin(m_fragments), end = std::end(m_fragments); it != end; ++it) {
            const Polyhedron& fragment = *it;
            
            const Vertex* firstVertex = fragment.vertices().front();
            const Vertex* currentVertex = firstVertex;
            do {
                const V& currentPos = currentVertex->position();
                if (excluded.count(currentPos) == 0) {
                    result.push_back(Fragment(it));
                    break;
                }
                currentVertex = currentVertex->next();
            } while (currentVertex != firstVertex);
        }
        return result;
    }
    
    void applyVertexMove(const V& fromPos, const V& toPos) {
        for (Polyhedron& fragment : m_fragments) {
            if (fragment.hasVertex(fromPos)) {
                const typename V::List positions = fragment.vertexPositions();
                fragment.clear();
                
                for (const V& vertex : positions) {
                    if (vertex.equals(fromPos, 0.1))
                        fragment.addPoint(toPos);
                    else
                        fragment.addPoint(vertex);
                }
            }
        }
    }

    void removeInvalidFragments() {
        auto it = std::begin(m_fragments);
        while (it != std::end(m_fragments)) {
            const Polyhedron& fragment = *it;
            if (!fragment.polyhedron()) {
                it = m_fragments.erase(it);
            } else {
                it = std::next(it);
            }
        }
    }
    
    void removeDuplicateFragments() {
        FragmentVertexSet fragmentVertices = getFragmentVertices(m_fragments);
        removeDuplicateFragments(fragmentVertices);
        m_fragments = rebuildFragments(fragmentVertices);
    }
    
    FragmentVertexSet getFragmentVertices(const Polyhedron::List& fragments) {
        FragmentVertexSet result;
        for (const Polyhedron& fragment : fragments) {
            typename V::Set vertices;
            SetUtils::makeSet(V::asList(std::begin(fragment.vertices()), std::end(fragment.vertices()), GetVertexPosition()), vertices);
            result.insert(vertices);
        }
        return result;
    }
    
    void removeDuplicateFragments(FragmentVertexSet& newFragments) const {
        FragmentVertexSet result;
        const typename FragmentVertexSet::iterator end = std::end(newFragments);
        while (!newFragments.empty()) {
            typename FragmentVertexSet::iterator lIt = std::begin(newFragments);
            typename FragmentVertexSet::iterator rIt = std::begin(newFragments); ++rIt;
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

    typename Polyhedron::List rebuildFragments(const FragmentVertexSet& newFragments) {
        Polyhedron::List result;
        for (const typename V::Set& vertices : newFragments) {
            if (vertices.size() > 3) {
                const Polyhedron fragment(vertices);
                if (fragment.polyhedron())
                    result.push_back(fragment);
            }
        }
        return result;
    }
};

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Merge {
private:
    struct NeighbourEntry {
        size_t neighbour;
        typename Face::Set neighbourSharedFaces;
        
        typedef std::set<NeighbourEntry> Set;
        
        NeighbourEntry(const size_t i_neighbour, const typename Face::Set& i_neighbourSharedFaces) :
        neighbour(i_neighbour),
        neighbourSharedFaces(i_neighbourSharedFaces) {
            ensure(!neighbourSharedFaces.empty(), "shared faces empty");
        }
        
        bool operator<(const NeighbourEntry& other) const {
            return neighbour < other.neighbour;
        }
    };
    
    // A merge group is just a set of indices into m_indices (see below). In effect, it is just a set of fragments.
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
            
            for (auto lIt = std::begin(lhs), rIt = std::begin(rhs), lEnd = std::end(lhs); lIt != lEnd; ++lIt, ++rIt) {
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
    
    // The fragments being processed.
    typename Polyhedron::List&  m_fragments;
    
    // A list mapping integral indices to iterators into the fragment list.
    typedef std::vector<typename Polyhedron::List::iterator> IndexList;
    IndexList m_indices;

    // Maps a polyhedron index (into m_indices) to the set of its mergeable neighbours.
    // Each entry in the set also stores the two shared faces between the neighbours.
    typedef std::map<size_t, typename NeighbourEntry::Set> Neighbours;
    Neighbours m_neighbours;

    // Contains a set of merge groups.
    typedef std::set<MergeGroup, MergeGroupCmp> MergeGroups;
    MergeGroups m_mergeGroups;
    
    const Callback& m_callback;
public:
    Merge(typename Polyhedron::List& fragments, const Callback& callback) :
    m_fragments(fragments),
    m_callback(callback) {
        initialize();
        merge();
    }
private:
    /**
     Initialize m_indices.
     */
    void initialize() {
        m_indices.reserve(m_fragments.size());
        for (auto it = std::begin(m_fragments), end = std::end(m_fragments); it != end; ++it)
            m_indices.push_back(it);
    }
    
    /**
     A set of fragments are mergeable if and only if their convex hull encloses exactly the volume
     of the fragments. In other words, the fragments form a convex volume themselves.
     
     If we view the fragments as nodes of a graph such that two nodes are connected
     by an edge if and only if
     
     - they share a face and
     - they are mergeable,
     
     then a merge group is a connected subgraph such that all the fragments of the graph are mergeable.
     
     The goal of the merging phase is do identify maximal non-overlapping merge groups. To achieve this,
     the following phases take place:
     
     1. Build the graph by finding all mergeable fragments and store it in m_neighbours.
     2. Find all maximal merge groups. These merge groups may overlap.
     3. Partition the merge groups into disjoint mergeable sets that do not overlap.
     4. Apply the remaining merge groups by merging their fragments.
     */
    void merge() {
        findMergeableNeighbours();
        findMergeGroups();
        partitionMergeGroups();
        applyMergeGroups();
    }
    
    /**
     Sorts faces by their vertices.
     */
    class FaceKey {
    private:
        typename V::Set m_vertices;
    public:
        FaceKey(const Face* face) {
            face->getVertexPositions(std::inserter(m_vertices, std::begin(m_vertices)));
        }
        
        bool operator<(const FaceKey& other) const {
            return compare(other) < 0;
        }
    private:
        int compare(const FaceKey& other) const {
            auto myIt = std::begin(m_vertices);
            auto otIt = std::begin(other.m_vertices);
            
            for (size_t i = 0; i < std::min(m_vertices.size(), other.m_vertices.size()); ++i) {
                const V& myVertex = *myIt++;
                const V& otVertex = *otIt++;
                const int vertexCmp = myVertex.compare(otVertex, Math::Constants<T>::almostZero());
                if (vertexCmp != 0)
                    return vertexCmp;
            }
            
            if (m_vertices.size() < other.m_vertices.size())
                return -1;
            if (m_vertices.size() > other.m_vertices.size())
                return 1;
            
            return 0;
        }
    };
    
    
    typedef std::pair<size_t, Face*> NeighbourFace;
    typedef std::vector<NeighbourFace> NeighbourFaceList;
    typedef std::map<FaceKey, NeighbourFaceList> NeighbourMap;
    
    struct SharedFaces {
        size_t index;
        typename Face::Set faces;
        
        SharedFaces(const size_t i_index, const typename Face::Set& i_faces) :
        index(i_index),
        faces(i_faces) {}
    };

    typedef std::pair<SharedFaces, SharedFaces> NeighbourPair;
    typedef std::list<NeighbourPair> NeighbourList;
    
    /**
     Finds each pair of neighbouring fragments that can be merged. Mergeable neighbours are stored in
     the m_neighbours map.
     */
    void findMergeableNeighbours() {
        for (const NeighbourPair& neighbourPair : findNeighbours()) {
            const SharedFaces&  first = neighbourPair.first;
            const SharedFaces& second = neighbourPair.second;
            
            if (mergeableNeighbours(second.faces, *m_indices[first.index])) {
                assert(mergeableNeighbours(first.faces, *m_indices[second.index]));
                m_neighbours[ first.index].insert(NeighbourEntry(second.index, second.faces));
                m_neighbours[second.index].insert(NeighbourEntry( first.index,  first.faces));
            } else {
                assert(!mergeableNeighbours(first.faces, *m_indices[second.index]));
            }
        }
    }
    
    /**
     Builds a map that maps face keys (which are essentially sets of vertices) to a list of faces,
     whereby each face is represented by a pair of its index into m_indices and the face itself.
     */
    NeighbourList findNeighbours() {
        NeighbourList result;

        for (size_t fragmentIndex = 0; fragmentIndex < m_indices.size(); ++fragmentIndex) {
            const auto fragmentIt = m_indices[fragmentIndex];
            const Polyhedron& fragment = *fragmentIt;
            const typename V::Set fragmentVertices = fragment.vertexPositionSet(0.0);
            
            for (size_t candidateIndex = fragmentIndex + 1; candidateIndex < m_indices.size(); ++candidateIndex) {
                const auto candidateIt = m_indices[candidateIndex];
                const Polyhedron& candidate = *candidateIt;
                const typename V::Set candidateVertices = candidate.vertexPositionSet(0.0);
                
                const typename V::Set sharedVertices = SetUtils::intersection(fragmentVertices, candidateVertices);
                if (sharedVertices.size() >= 3) {
                    const typename Face::Set  fragmentSharedFaces = findSharedFaces( fragment, sharedVertices);
                    const typename Face::Set candidateSharedFaces = findSharedFaces(candidate, sharedVertices);
                    
                    if (!fragmentSharedFaces.empty() && !candidateSharedFaces.empty()) {
                        result.push_back(NeighbourPair(SharedFaces( fragmentIndex,  fragmentSharedFaces),
                                                       SharedFaces(candidateIndex, candidateSharedFaces)));
                    }
                }
            }
        }
        
        return result;
    }
    
    typename Face::Set findSharedFaces(const Polyhedron& polyhedron, const typename V::Set& sharedVertices) const {
        typename Face::Set result;
        
        Face* firstFace = polyhedron.faces().front();
        Face* currentFace = firstFace;
        do {
            if (SetUtils::subset(currentFace->vertexPositionSet(0.0), sharedVertices))
                result.insert(currentFace);
            currentFace = currentFace->next();
        } while (currentFace != firstFace);
        
        return result;
    }
    
    /**
     Determines whether the polyhedron that contains the given faces and the given neighbour can be merged. This is the case
     if they form a convex volume. This can be checked by determining whether any face of the polyhedron other than the given
     ones can see any vertex of the given neighbour. If no such face exists, then the two polyhedra form a convex volume and
     can be merged.
     */
    bool mergeableNeighbours(const typename Face::Set& sharedFaces, const Polyhedron& neighbour) const {
        assert(!sharedFaces.empty());
        assert(!neighbour.empty());
        
        const Vertex* firstVertex = neighbour.vertices().front();
        
        Face* firstFace = *std::begin(sharedFaces);
        Face* currentFace = firstFace->next(); // skip the first face since we know it's shared
        do {
            if (sharedFaces.count(currentFace) == 0) {
                const Vertex* currentVertex = firstVertex;
                do {
                    if (currentFace->pointStatus(currentVertex->position()) == Math::PointStatus::PSAbove)
                        return false;
                    currentVertex = currentVertex->next();
                } while (currentVertex != firstVertex);
            }
            currentFace = currentFace->next();
        } while (currentFace != firstFace);
        
        return true;
    }
    
    /**
     Finds all maximal merge groups using the information in m_neighbours.
     */
    void findMergeGroups() {
        for (const auto& neighbourPair : m_neighbours) {
            // The fragment under consideration and its mergeable neighbours.
            const size_t fragmentIndex = neighbourPair.first;
            const typename NeighbourEntry::Set& neighbours = neighbourPair.second;
            
            for (const NeighbourEntry& entry : neighbours) {
                const size_t neighbourIndex = entry.neighbour;
                
                // Create a new merge group from the fragment and its neighbour.
                MergeGroup group;
                group.insert(fragmentIndex);
                group.insert(neighbourIndex);
                
                if (m_mergeGroups.count(group) == 0) {
                    // The merge group hasn't already been found.
                    
                    // Build its convex hull.
                    const Polyhedron convexHull = mergeGroup(group);
                    
                    // Attempt to expand the group by considering every neighbour of the fragment
                    // and by considering every neighbour of the neighbour.
                    if (!expandMergeGroup(group, convexHull, fragmentIndex) &&
                        !expandMergeGroup(group, convexHull, neighbourIndex)) {
                        // The group could not be expanded any further.
                        m_mergeGroups.insert(group);
                    }
                }
            }
        }
    }
    
    
    Polyhedron mergeGroup(const MergeGroup& group) const {
        auto it = std::begin(group);
        auto end = std::end(group);
        
        Polyhedron result = *m_indices[*it++];
        while (it != end)
            result.merge(*m_indices[*it++]);
        
        return result;
    }

    /**
     Attempts to expand the given merge group that has the given polyhedron by adding to it every mergeable
     neighbour of the fragment at the given index.
     
     For each possible expansion of the group with one of those neighbours, the expanded group is then
     again attempted to be expanded further. If it could not be expanded any further, it is stored in
     m_mergeGroups.
     */
    bool expandMergeGroup(const MergeGroup& group, const Polyhedron& polyhedron, const size_t index1) {
        const auto it = m_neighbours.find(index1);
        
        // The fragment at index1 doesn't have any mergeable neighbours.
        if (it == std::end(m_neighbours))
            return false;
        
        bool didExpand = false;
        const typename NeighbourEntry::Set& neighbours = it->second;
        
        // Iterate over all mergeable neighbours and attempt to expand the merge group further.
        for (const NeighbourEntry& entry : neighbours) {
            MergeGroup newGroup = group;
            const size_t neighbourIndex = entry.neighbour;
            
            if (newGroup.insert(neighbourIndex).second &&
                mergeableNeighbours(entry.neighbourSharedFaces, polyhedron)) {
                // The potential neighbour wasn't already in the new group, and it is actually mergeable
                // with the group.
                
                if (m_mergeGroups.count(newGroup) == 0) {
                    // The new group wasn't already discovered.
                    
                    // Create a new polyhedron that represents the convex hull of the new merge group.
                    Polyhedron newPolyhedron = polyhedron;
                    newPolyhedron.merge(*m_indices[neighbourIndex]);
                    
                    if (!expandMergeGroup(newGroup, newPolyhedron, neighbourIndex)) {
                        // The newly created merge group can't be expanded any further.
                        m_mergeGroups.insert(newGroup);
                        didExpand = true;
                    }
                }
            }
        }
        return didExpand;
    }
    
    void partitionMergeGroups() {
        MergeGroups newMergeGroups;
        typename MergeGroups::iterator mFirst, mSecond;
        while (!m_mergeGroups.empty()) {
            mFirst = std::begin(m_mergeGroups);
            mSecond = mFirst; ++mSecond;
            
            const MergeGroup& first = *mFirst;
            bool firstIsDisjoint = true;
            
            while (mSecond != std::end(m_mergeGroups)) {
                const MergeGroup& second = *mSecond;
                const MergeGroup intersection = SetUtils::intersection(first, second);
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
                        const MergeGroup firstMinusSecond = SetUtils::minus(first, intersection);
                        const MergeGroup secondMinusFirst = SetUtils::minus(second, intersection);
                        
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
        for (const MergeGroup& group : m_mergeGroups) {
            if (group.size() > 1) {
                auto gIt = std::begin(group);
                auto gEnd = std::end(group);
                
                Polyhedron& master = *m_indices[*gIt++];
                while (gIt != gEnd) {
                    auto fIt = m_indices[*gIt++];
                    master.merge(*fIt);
                    m_fragments.erase(fIt);
                }
            }
        }
    }
};

#endif /* Polyhedron_Subtract_h */
