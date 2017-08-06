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
    return subtract.result();
}

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Subtract {
private:
    const Polyhedron& m_minuend;
    Polyhedron m_subtrahend;
    const Callback& m_callback;
    List m_fragments;
    
    typedef std::list<Plane<T,3>> PlaneList;
    typedef typename PlaneList::const_iterator PlaneIt;
public:
    Subtract(const Polyhedron& minuend, const Polyhedron& subtrahend, const Callback& callback) :
    m_minuend(minuend),
    m_subtrahend(subtrahend),
    m_callback(callback) {
        if (clipSubtrahend()) {
            subtract();
        }
    }
    
    const List result() {
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

    void subtract() {
        const PlaneList planes = sortPlanes(findSubtrahendPlanes());
        
        assert(m_fragments.empty());
        doSubtract(List{m_minuend}, std::begin(planes), std::end(planes));
    }
    
    PlaneList findSubtrahendPlanes() const {
        PlaneList result;
        
        const Face* firstFace = m_subtrahend.faces().front();
        const Face* currentFace = firstFace;
        do {
            const Plane<T,3> plane = m_callback.plane(currentFace);
            result.push_back(plane);
            currentFace = currentFace->next();
        } while (currentFace != firstFace);
        
        return result;
    }
    
    static PlaneList sortPlanes(PlaneList planes) {
        auto it = std::begin(planes);
        it = sortPlanes(it, std::end(planes), Vec<T,3>::PosX);
        it = sortPlanes(it, std::end(planes), Vec<T,3>::PosY);
        it = sortPlanes(it, std::end(planes), Vec<T,3>::PosZ);
        
        return planes;
    }

    static typename PlaneList::iterator sortPlanes(typename PlaneList::iterator begin, typename PlaneList::iterator end, Vec<T,3> axis) {
        if (begin == end)
            return end;
        
        auto it = begin;
        while (it != end) {
            auto next = selectPlanes(it, end, axis);
            if (next == it)
                break; // no further progress
            it = next;
        }
        
        return it;
    }
    
    static typename PlaneList::iterator selectPlanes(typename PlaneList::iterator begin, typename PlaneList::iterator end, Vec<T,3> axis) {
        assert(begin != end);
        
        auto bestIt = end;
        for (auto it = begin; it != end; ++it) {
            const T bestDot = bestIt != end ? Math::abs(bestIt->normal.dot(axis)) : 0.0;
            const T curDot  = Math::abs(it->normal.dot(axis));
            
            if (curDot > bestDot)
                    bestIt = it;
        }
        
        if (bestIt == end)
            return end;

        if (Math::abs(bestIt->normal.dot(axis)) < 0.5)
            return begin;
        
        assert(bestIt != end);
        axis = -bestIt->normal;
        std::iter_swap(begin++, bestIt);
        
        bestIt = end;
        for (auto it = begin; it != end; ++it) {
            const T bestDot = bestIt != end ? bestIt->normal.dot(axis) : 0.0;
            const T curDot  = it->normal.dot(axis);
            
            if (curDot > bestDot)
                bestIt = it;
        }
        
        if (bestIt != end)
            std::iter_swap(begin++, bestIt);
        return begin;
    }
    
    static Plane<T,3> extractFirstPlane(PlaneList& planes) {
        auto bestIt = std::end(planes);
        T bestDot = 0.0;
        for (auto it = std::begin(planes), end = std::end(planes); it != end; ++it) {
            const Plane<T,3>& plane = *it;
            const T dot = Math::abs(plane.normal.dot(Vec<T,3>::PosX));
            if (dot < bestDot) {
                bestDot = dot;
                bestIt = it;
            }
        }
        
        assert(bestIt != std::end(planes));
        planes.erase(bestIt);
        return *bestIt;
    }
    
    void doSubtract(const List& fragments, PlaneIt curPlaneIt, PlaneIt endPlaneIt) {
        if (fragments.empty() || curPlaneIt == endPlaneIt) {
            // no more fragments to process or all of `minutendFragments`
            // are now behind all of subtrahendPlanes so they can be discarded.
            return;
        }
        
        const Plane<T,3> curPlane = *curPlaneIt;
        const Plane<T,3> curPlaneInv = curPlane.flipped();
        
        // clip the list of minutendFragments into a list of those in front of the
        // currentPlane, and those behind
        List backFragments;
        
        for (const Polyhedron& fragment : fragments) {
            // the front fragments go directly into the result set.
            Polyhedron<T,FP,VP> fragmentInFront = fragment;
            const auto frontClipResult = fragmentInFront.clip(curPlaneInv);
            
            if (!frontClipResult.empty()) // Polyhedron::clip() keeps the part behind the plane.
                m_fragments.push_back(fragmentInFront);
            
            // back fragments need to be clipped by the rest of the subtrahend planes
            Polyhedron<T,FP,VP> fragmentBehind = fragment;
            const auto backClipResult = fragmentBehind.clip(curPlane);
            if (!backClipResult.empty())
                backFragments.push_back(fragmentBehind);
        }
        
        // recursively process the back fragments.
        doSubtract(backFragments, std::next(curPlaneIt), endPlaneIt);
    }
};

#endif /* Polyhedron_Subtract_h */
