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
    const ClosestVertices closestVertices(m_vertices, subtrahend.vertices());
    
    buildInitialFragments(subtrahend, closestVertices, result, callback);
    resolveIntersections(subtrahend, result, callback);
    buildMissingFragments(subtrahend, closestVertices, result, callback);
    
    return result;
}

template <typename T, typename FP>
class Polyhedron<T,FP>::ClosestVertices {
private:
    typedef std::map<Vertex*, VertexSet> ClosestVertexMap;
    ClosestVertexMap m_closestVertices;
public:
    ClosestVertices(const VertexList& vertexList1, const VertexList& vertexList2) {
        add(vertexList1, vertexList2);
        add(vertexList2, vertexList1);
    }
    
    const VertexSet& get(Vertex* vertex) const {
        static const VertexSet EmptySet = VertexSet();
        typename ClosestVertexMap::const_iterator it = m_closestVertices.find(vertex);
        if (it == m_closestVertices.end())
            return EmptySet;
        return it->second;
    }
private:
    void add(const VertexList& vertexList1, const VertexList& vertexList2) {
        Vertex* first1 = vertexList1.front();
        Vertex* cur1 = first1;
        do {
            VertexSet closestVertices;
            T closestDistance = std::numeric_limits<T>::max();
            
            Vertex* first2 = vertexList2.front();
            Vertex* cur2 = first2;
            do {
                const T distance = cur1->position().squaredDistanceTo(cur2->position());
                if (distance < closestDistance) {
                    closestDistance = distance;
                    closestVertices.clear();
                    closestVertices.insert(cur2);
                } else if (distance == closestDistance) {
                    closestVertices.insert(cur2);
                }
                
                cur2 = cur2->next();
            } while (cur2 != first2);
            
            typename VertexSet::iterator it, end;
            for (it = closestVertices.begin(), end = closestVertices.end(); it != end; ++it)
                m_closestVertices[*it].insert(cur1);
            
            cur1 = cur1->next();
        } while (cur1 != first1);
    }
};

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
void Polyhedron<T,FP>::buildInitialFragments(const Polyhedron& subtrahend, const ClosestVertices& closestVertices, SubtractResult& result, const Callback& callback) const {
    
    Face* firstFace = subtrahend.faces().front();
    Face* currentFace = firstFace;
    do {
        const Plane<T,3> plane = callback.plane(currentFace);
        bool allOnPlane = true;
        typename V::Set allVertices;
        
        const HalfEdge* firstEdge = currentFace->boundary().front();
        const HalfEdge* currentEdge = firstEdge;
        do {
            allVertices.insert(currentEdge->origin()->position());
            
            const VertexSet& closest = closestVertices.get(currentEdge->origin());
            
            typename VertexSet::const_iterator it, end;
            for (it = closest.begin(), end = closest.end(); it != end; ++it) {
                Vertex* vertex = *it;
                const Math::PointStatus::Type status = plane.pointStatus(vertex->position());
                if (status != Math::PointStatus::PSBelow) {
                    allVertices.insert(vertex->position());
                    if (status == Math::PointStatus::PSAbove)
                        allOnPlane = false;
                }
            }
            
            currentEdge = currentEdge->next();
        } while (currentEdge != firstEdge);

        if (!allOnPlane)
            result.push_back(Polyhedron(allVertices));

        currentFace = currentFace->next();
    } while (currentFace != firstFace);
}

template <typename T, typename FP>
void Polyhedron<T,FP>::buildMissingFragments(const Polyhedron& subtrahend, const ClosestVertices& closestVertices, SubtractResult& result, const Callback& callback) const {

    Face* firstFace = m_faces.front();
    Face* currentFace = firstFace;
    do {
        if (isMissingFragment(currentFace, result)) {
            const Plane<T,3> plane = callback.plane(currentFace);
            bool allOnPlane = true;
            typename V::Set allVertices;

            const HalfEdge* firstEdge = currentFace->boundary().front();
            const HalfEdge* currentEdge = firstEdge;
            
            do {
                allVertices.insert(currentEdge->origin()->position());

                const VertexSet& lastClosest = closestVertices.get(currentEdge->previous()->origin());
                const VertexSet& currentClosest = closestVertices.get(currentEdge->origin());
                VertexSet toAdd;
                SetUtils::intersection(lastClosest, currentClosest, toAdd);
                
//                if (toAdd.empty())
//                    SetUtils::merge(lastClosest, currentClosest, toAdd);
                
                typename VertexSet::const_iterator it, end;
                for (it = toAdd.begin(), end = toAdd.end(); it != end; ++it) {
                    Vertex* vertex = *it;
                    const Math::PointStatus::Type status = plane.pointStatus(vertex->position());
                    if (status != Math::PointStatus::PSAbove) {
                        allVertices.insert(vertex->position());
                        allOnPlane &= (status == Math::PointStatus::PSBelow);
                    }
                }
                
                currentEdge = currentEdge->next();
            } while (currentEdge != firstEdge);
            
            if (!allOnPlane)
                result.push_back(Polyhedron(allVertices));
        }
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
}

template <typename T, typename FP>
bool Polyhedron<T,FP>::isMissingFragment(const Face* face, const SubtractResult& result) const {
    const typename V::List vertices = V::asList(face->boundary().begin(), face->boundary().end(), GetVertexPosition());
    typename SubtractResult::const_iterator it, end;
    for (it = result.begin(), end = result.end(); it != end; ++it) {
        const Polyhedron& polyhedron = *it;
        if (polyhedron.hasVertices(vertices))
            return false;
    }
    return true;
}

template <typename T, typename FP>
void Polyhedron<T,FP>::resolveIntersections(const Polyhedron& subtrahend, SubtractResult& result, const Callback& callback) const {
    typename SubtractResult::iterator cur, it, end;
    cur = result.begin();
    while (cur != result.end()) {
        it = cur;
        end = result.end();
        
        bool increment = true;
        while (++it != end) {
            Polyhedron intersection = cur->intersect(*it, callback);
            if (!intersection.empty()) {
                const SubtractResult firstResult = cur->subtract(intersection, callback);
                const SubtractResult secondResult = it->subtract(intersection, callback);

                result.insert(result.end(), firstResult.begin(), firstResult.end());
                result.insert(result.end(), secondResult.begin(), secondResult.end());
                result.push_back(intersection);
                
                result.erase(it);
                cur = result.erase(cur);
                increment = false;
                break;
            }
        }
        if (increment)
            ++cur;
    }

    it = result.begin();
    end = result.end();
    while (it != end) {
        Polyhedron intersection = it->intersect(subtrahend, callback);
        if (!intersection.empty()) {
            const SubtractResult firstResult = it->subtract(intersection, callback);
            result.insert(result.end(), firstResult.begin(), firstResult.end());
            it = result.erase(it);
        } else {
            ++it;
        }
    }
}

#endif /* Polyhedron_Subtract_h */
