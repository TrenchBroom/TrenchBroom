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

#include "Relation.h"

#include <algorithm>
#include <iterator>
#include <numeric>

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::SubtractResult Polyhedron<T,FP,VP>::subtract(const Polyhedron& subtrahend) const {
    Callback c;
    return subtract(subtrahend, c);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::SubtractResult Polyhedron<T,FP,VP>::subtract(const Polyhedron& subtrahend, const Callback& callback) const {
    List result;
    
    Subtract subtract(*this, subtrahend, result, callback);
    Simplify simplify(subtract);
    Merge merge(result, callback);
    
    // addMissingFragments(fragments, *this, subtrahend, callback);
    
    return result;
}

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Subtract {
private:
    const Polyhedron& m_minuend;
    Polyhedron m_subtrahend;
    List& m_fragments;
    const Callback& m_callback;
    
    friend class Simplify;
public:
    Subtract(const Polyhedron& minuend, const Polyhedron& subtrahend, List& fragments, const Callback& callback) :
    m_minuend(minuend),
    m_subtrahend(subtrahend),
    m_fragments(fragments),
    m_callback(callback) {
        if (clipSubtrahend()) {
            m_fragments.push_back(m_minuend);
            chopMinuend();
            removeSubtrahend();
        }
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
    
    void removeSubtrahend() {
        const typename V::List vertices = V::asList(m_subtrahend.vertices().begin(), m_subtrahend.vertices().end(), GetVertexPosition());
        
        for (auto it = std::begin(m_fragments), end = std::end(m_fragments); it != end; ++it) {
            const Polyhedron& fragment = *it;
            if (fragment.hasVertices(vertices, 0.1)) {
                m_fragments.erase(it);
                return;
            }
        }
        assert(false);
    }
};

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Simplify {
private:
    const Polyhedron& m_minuend;
    const Polyhedron& m_subtrahend;
    List& m_fragments;
    const Callback& m_callback;

    typedef typename List::iterator PolyIt;
    
    struct VertexInfo {
        Vertex* vertex;
        PolyIt fragment;
        
        VertexInfo(Vertex* i_vertex, PolyIt i_fragment) :
        vertex(i_vertex),
        fragment(i_fragment) {
            assert(vertex != nullptr);
        }
    };
    
    typedef std::list<VertexInfo> VertexInfoList;
    typedef std::map<V, VertexInfoList, typename V::LexicographicOrder> VertexGraph;
    typedef typename VertexGraph::iterator GraphIt;
    
    VertexGraph m_graph;

    struct VertexMove {
        GraphIt sourceInfo;
        const Vertex* targetVertex;
        
        VertexMove(GraphIt i_sourceInfo, const Vertex* i_targetVertex) :
        sourceInfo(i_sourceInfo),
        targetVertex(i_targetVertex) {
            assert(targetVertex != nullptr);
        }

        const V& sourcePosition() const {
            return sourceInfo->first;
        }
        
        const V& targetPosition() const {
            return targetVertex->position();
        }
        
        const VertexInfoList& incidentFragments() const {
            return sourceInfo->second;
        }
    };
    
    typedef std::list<VertexMove> VertexMoveList;
public:
    Simplify(Subtract& subtract) :
    m_minuend(subtract.m_minuend),
    m_subtrahend(subtract.m_subtrahend),
    m_fragments(subtract.m_fragments),
    m_callback(subtract.m_callback) {
        initializeGraph();
        simplify();
    }
private:
    void initializeGraph() {
        for (auto it = std::begin(m_fragments), end = std::end(m_fragments); it != end; ++it) {
            addFragmentToGraph(it);
        }
    }
    
    void addFragmentToGraph(PolyIt fragment) {
        for (Vertex* vertex : fragment->vertices()) {
            const V& position = vertex->position();
            m_graph[position].emplace_back(vertex, fragment);
        }
    }
    
    void removeFragmentFromGraph(PolyIt fragment) {
        for (Vertex* vertex : fragment->vertices()) {
            const V& position = vertex->position();
            VertexInfoList& infos = m_graph[position];
            auto it = std::find_if(std::begin(infos), std::end(infos), [fragment](const VertexInfo& info){
                return info.fragment == fragment;
            });

            if (it != std::end(infos))
                infos.erase(it);
        }
    }
    
    void simplify() {
        const typename V::Set excluded = findExcludedVertices();

        while (true) {
            const VertexMoveList moves = findVertexMoves(excluded);
            for (const VertexMove& move : moves) {
                if (applyVertexMove(move))
                    continue;
            }
            break;
        }
        
        // Prune fragments.
        m_fragments.erase(std::remove_if(std::begin(m_fragments), std::end(m_fragments), [](const Polyhedron& fragment) {
            return !fragment.polyhedron();
        }), std::end(m_fragments));
    }
    
    typename V::Set findExcludedVertices() {
        typename V::Set result;
        SetUtils::merge(result, m_minuend.vertexPositionSet());
        SetUtils::merge(result, m_subtrahend.vertexPositionSet());
        return result;
    }
    
    VertexMoveList findVertexMoves(const typename V::Set& excluded) {
        VertexMoveList entries;
        
        for (GraphIt it = std::begin(m_graph), end = std::end(m_graph); it != end; ++it) {
            const V& position = it->first;
            if (excluded.count(position) == 0) {
                // The vertex is a fragment vertex that does not coincide with a minuend or subtrahend vertex.
                const VertexInfoList& infos = it->second;
                const Vertex* minuendVertex = findClosestIncidentMinuendVertex(position, infos);
                if (minuendVertex != nullptr) {
                    entries.emplace_back(it, minuendVertex);
                }
            }
        }
        
        // Sort by the number of incident fragments in descending order.
        entries.sort([](const VertexMove& lhs, const VertexMove& rhs) {
            const VertexInfoList& lhsInfos = lhs.sourceInfo->second;
            const VertexInfoList& rhsInfos = rhs.sourceInfo->second;
            return lhsInfos.size() > rhsInfos.size();
        });

        return entries;
    }
    
    const Vertex* findClosestIncidentMinuendVertex(const V& position, const VertexInfoList& infos) {
        const Vertex* closestMinuendVertex = nullptr;
        T closestDistance2 = std::numeric_limits<T>::max();
        
        for (const VertexInfo& info : infos) {
            const Vertex* vertex = info.vertex;
            const HalfEdge* firstEdge = vertex->leaving();
            const HalfEdge* currentEdge = firstEdge;
            do {
                const Vertex* neighbour = currentEdge->destination();
                const Vertex* minuendVertex = m_minuend.findVertexByPosition(neighbour->position());
                if (minuendVertex != nullptr) {
                    const T distance2 = position.squaredDistanceTo(minuendVertex->position());
                    if (distance2 < closestDistance2) {
                        closestDistance2 = distance2;
                        closestMinuendVertex = minuendVertex;
                    }
                }
                
                currentEdge = currentEdge->nextIncident();
            } while (currentEdge != firstEdge);
        }
        
        return closestMinuendVertex;
    }
    
    bool applyVertexMove(const VertexMove& move) {
        const V& sourcePosition = move.sourcePosition();
        const VertexInfoList& infos = move.incidentFragments();
        const V& targetPosition = move.targetPosition();

        typedef std::pair<PolyIt, Polyhedron> UpdatedFragment;
        typedef std::list<UpdatedFragment> UpdatedFragmentList;
        
        UpdatedFragmentList updatedFragments;
        
        for (const VertexInfo& info : infos) {
            PolyIt fragmentIt = info.fragment;
            Polyhedron fragment = *fragmentIt; // Copy is intentional.
            fragment.removeVertexByPosition(sourcePosition);
            fragment.addPoint(targetPosition);
            
            if (fragment.polyhedron() && fragment.intersects(m_subtrahend))
                return false;
            
            updatedFragments.push_back(std::make_pair(fragmentIt, fragment));
        }
        
        m_graph.erase(sourcePosition);
        
        for (const UpdatedFragment& updatedFragment : updatedFragments) {
            PolyIt it = updatedFragment.first;
            removeFragmentFromGraph(it);

            const Polyhedron& fragment = updatedFragment.second;
            *it = fragment;

            addFragmentToGraph(it);
        }
        
        return true;
    }
};

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Merge {
private:
    typedef std::vector<typename Polyhedron::List::iterator> IndexList;
    typedef std::multimap<size_t, size_t> Neighbours;
    
    typedef std::set<size_t> MergeGroup;
    typedef std::set<MergeGroup> MergeGroups;
    
    typename Polyhedron::List&  m_fragments;
    const Callback& m_callback;
    IndexList m_indices;
    
    // Maps a polyhedron index (into m_indices) to the set of its mergeable neighbours.
    // Each entry in the set also stores the two shared faces between the neighbours.
    Neighbours m_neighbours;
    MergeGroups m_mergeGroups;
public:
    Merge(typename Polyhedron::List& fragments, const Callback& callback) :
    m_fragments(fragments),
    m_callback(callback) {
        initializeIndices();
        initializeNeighbours();
        merge();
    }
private:
    void initializeIndices() {
        for (auto it = std::begin(m_fragments), end = std::end(m_fragments); it != end; ++it)
            m_indices.push_back(it);
    }
    
    typedef std::vector<typename V::Set> VertexSets;

    /**
     Finds each pair of neighbouring fragments that can be merged. Mergeable neighbours are stored in
     the m_neighbours map.
     */
    void initializeNeighbours() {
        const VertexSets vertexSets = buildFragmentVertexSets();
        
        for (size_t i = 0; i < m_indices.size(); ++i) {
            const typename V::Set& f1Vertices = vertexSets[i];
            for (size_t j = i + 1; j < m_indices.size(); ++j) {
                const typename V::Set& f2Vertices = vertexSets[j];
                const typename V::Set& sharedVertices = SetUtils::intersection(f1Vertices, f2Vertices);
                
                if (sharedVertices.size() > 2) {
                    // We have two neighbours. Check if they are mergeable.
                    const Polyhedron& f1 = *m_indices[i];
                    const Polyhedron& f2 = *m_indices[j];
                    
                    if (mergeableNeighbours(f1, f2, sharedVertices)) {
                        m_neighbours.insert(std::make_pair(i, j));
                        m_neighbours.insert(std::make_pair(j, i));
                    }
                }
            }
        }
        
    }
    
    VertexSets buildFragmentVertexSets() {
        VertexSets result;
        result.reserve(m_fragments.size());
        
        for (auto fragmentIt : m_indices) {
            const Polyhedron& fragment = *fragmentIt;
            result.push_back(fragment.vertexPositionSet());
        }
        
        return result;
    }

    /*
     Checks whether two neighbouring fragments are mergeable. Assumes that the given fragments share at least
     three vertices. The given fragments are mergeable if, for every non-shared vertex of fragment1 (fragment2), 
     every visible face of fragment2 (fragment1) has only shared vertices.
     */
    bool mergeableNeighbours(const Polyhedron& fragment1, const Polyhedron& fragment2, const typename V::Set& sharedVertices) const {
        assert(sharedVertices.size() > 2);
        
        return (onlySharedFacesVisible(fragment1.vertices(), fragment2.faces(), sharedVertices) &&
                onlySharedFacesVisible(fragment2.vertices(), fragment1.faces(), sharedVertices));
    }
    
    bool onlySharedFacesVisible(const VertexList& vertices, const FaceList& faces, const typename V::Set& sharedVertices) const {
        for (const Vertex* vertex : vertices) {
            for (const Face* face : faces) {
                if (face->pointStatus(vertex->position()) == Math::PointStatus::PSAbove) {
                    if (!SetUtils::subset(face->vertexPositionSet(), sharedVertices))
                        return false;
                }
            }
        }
        
        return true;
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
        findMergeGroups();
        partitionMergeGroups();
        applyMergeGroups();
    }
    
    
    /**
     Finds all maximal merge groups using the information in m_neighbours.
     */
    void findMergeGroups() {
        for (const auto neighbourPair : m_neighbours) {
            // The fragment under consideration and its mergeable neighbours.
            const size_t fragmentIndex = neighbourPair.first;
            const size_t neighbourIndex = neighbourPair.second;
            
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
    
    /**
     Attempts to expand the given merge group that has the given convex hull by adding to it every mergeable
     neighbour of the fragment at the given index.
     
     For each possible expansion of the group with one of those neighbours, the expanded group is then
     again attempted to be expanded further. If it could not be expanded any further, it is stored in
     m_mergeGroups.
     */
    bool expandMergeGroup(const MergeGroup& group, const Polyhedron& hull, const size_t index1) {
        const auto range = m_neighbours.equal_range(index1);
              auto it = range.first;
        const auto end = range.second;
        
        // The fragment at index1 doesn't have any mergeable neighbours.
        if (it == end)
            return false;
        
        const typename V::Set hullVertices = hull.vertexPositionSet();
        
        bool didExpand = false;
        while (it != end) {
            MergeGroup newGroup = group;
            
            const size_t neighbourIndex = it->second;
            const Polyhedron& neighbour = *m_indices[neighbourIndex];
            
            const typename V::Set sharedVertices = SetUtils::intersection(hullVertices, neighbour.vertexPositionSet());
            
            if (newGroup.insert(neighbourIndex).second &&
                sharedVertices.size() > 2 &&
                mergeableNeighbours(hull, neighbour, sharedVertices)) {
                // The potential neighbour wasn't already in the new group, and it is actually mergeable
                // with the group.
                
                if (m_mergeGroups.count(newGroup) == 0) {
                    // The new group wasn't already discovered.
                    
                    // Create a new polyhedron that represents the convex hull of the new merge group.
                    Polyhedron newHull = hull;
                    newHull.merge(*m_indices[neighbourIndex]);
                    
                    if (!expandMergeGroup(newGroup, newHull, neighbourIndex)) {
                        // The newly created merge group can't be expanded any further.
                        m_mergeGroups.insert(newGroup);
                        didExpand = true;
                    }
                }
            }
            
            ++it;
        }
        return didExpand;
    }
    
    Polyhedron mergeGroup(const MergeGroup& group) const {
        MergeGroup::const_iterator it = std::begin(group);
        MergeGroup::const_iterator end = std::end(group);
        
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


template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addMissingFragments(Polyhedron::List& fragments, const Polyhedron& minuend, const Polyhedron& subtrahend, const Callback& callback) {
    const FaceSet uncoveredFaces = findUncoveredFaces(fragments, minuend, subtrahend, callback);

    const auto cmp = [](const Face* lhs, const Face* rhs) {
        HalfEdge* lhsFirst = lhs->boundary().front();
        HalfEdge* lhsCur = lhsFirst;
        do {
            Edge* lhsEdge = lhsCur->edge();
            Edge* rhsEdge = rhs->findEdge(lhsEdge->firstVertex()->position(), lhsEdge->secondVertex()->position());
            if (rhsEdge != nullptr)
                return true;
            lhsCur = lhsCur->next();
        } while (lhsCur != lhsFirst);
        
        return false;
    };
    
    const auto eqClasses = CollectionUtils::equivalenceClasses(uncoveredFaces, cmp);

    for (const std::list<Face*>& eqClass : eqClasses) {
        typename V::Set vertices;
        typename V::Set visibleMinuendVertices = minuend.vertexPositionSet();
        for (const Face* face : eqClass) {
            SetUtils::merge(vertices, face->vertexPositionSet());
            
            const Plane<T,3>& boundary = callback.plane(face);
            CollectionUtils::eraseIf(visibleMinuendVertices, [&boundary](const V& vertex) {
                return boundary.pointStatus(vertex) != Math::PointStatus::PSAbove;
            });
        }
        
        SetUtils::merge(vertices, visibleMinuendVertices);
        fragments.push_back(Polyhedron(vertices));
    }
}

/*
 Find all faces of the given fragments which are not covered. A face is covered if either of the following conditions hold:
 * its boundary plane coincides with the boundary plane of any face of the minuend
 * the inverse of its boundary plane coincides with the boundary plane of any face of the subtrahend
 * it shares all of its vertices with another fragment.
 */
template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::FaceSet Polyhedron<T,FP,VP>::findUncoveredFaces(const Polyhedron::List& fragments, const Polyhedron& minuend, const Polyhedron& subtrahend, const Callback& callback) {
    const typename Plane<T,3>::Set ignoredPlanes = findIgnoredPlanes(minuend, subtrahend, callback);

    FaceSet result;
    for (const Polyhedron& fragment : fragments) {
        Face* firstFace = fragment.faces().front();
        Face* curFace = firstFace;
        do {
            const Plane<T,3> boundary = callback.plane(curFace);
            if (ignoredPlanes.count(boundary) == 0) {
                if (!isCoveredByFragment(curFace, fragments, fragment)) {
                    result.insert(curFace);
                }
            }
            curFace = curFace->next();
        } while (curFace != firstFace);
    }
    
    return result;
}

template <typename T, typename FP, typename VP>
typename Plane<T,3>::Set Polyhedron<T,FP,VP>::findIgnoredPlanes(const Polyhedron& minuend, const Polyhedron& subtrahend, const Callback& callback) {
    typename Plane<T,3>::Set result;
    addIgnoredPlanes(minuend, false, callback, result);
    addIgnoredPlanes(subtrahend, true, callback, result);
    return result;
}

template <typename T, typename FP, typename VP>
void Polyhedron<T,FP,VP>::addIgnoredPlanes(const Polyhedron& polyhedron, bool flip, const Callback& callback, typename Plane<T,3>::Set& result) {
    Face* firstFace = polyhedron.faces().front();
    Face* curFace = firstFace;
    do {
        const Plane<T,3> boundary = callback.plane(curFace);
        if (flip)
            result.insert(boundary.flipped());
        else
            result.insert(boundary);
        curFace = curFace->next();
    } while (curFace != firstFace);
}

template <typename T, typename FP, typename VP>
bool Polyhedron<T,FP,VP>::isCoveredByFragment(const Face* face, const Polyhedron::List& fragments, const Polyhedron& ignore) {
    const typename V::Set& faceVertices = face->vertexPositionSet();
    for (const Polyhedron& fragment : fragments) {
        if (&fragment == &ignore)
            continue;
        
        const typename V::Set& fragmentVertices = fragment.vertexPositionSet();
        const typename V::Set intersection = SetUtils::intersection(faceVertices, fragmentVertices);
        
        if (intersection.size() == faceVertices.size())
            return true;
        
        // assert(intersection.size() <= 2);
    }
    return false;
}


#endif /* Polyhedron_Subtract_h */
