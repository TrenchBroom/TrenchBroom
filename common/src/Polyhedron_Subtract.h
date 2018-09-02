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
    
    typedef std::list<plane<T,3>> PlaneList;
    typedef typename PlaneList::const_iterator PlaneIt;
public:
    Subtract(const Polyhedron& minuend, const Polyhedron& subtrahend, const Callback& callback) :
    m_minuend(minuend),
    m_subtrahend(subtrahend),
    m_callback(callback) {
        if (clipSubtrahend()) {
            subtract();
        } else {
            // minuend and subtrahend are disjoint
            m_fragments = { minuend };
        }
    }
    
    const List result() {
        return m_fragments;
    }
private:
    /**
     * Clips away the parts of m_subtrahend which are not intersecting m_minuend
     * (and therefore aren't useful for the subtraction).
     * This is an optimization that might result in better quality subtractions.
     *
     * If the entire subtrahend is clipped away (i.e. the minuend and subtrahend are disjoint), returns false.
     * Otherwise, returns true.
     */
    bool clipSubtrahend() {
        Face* first = m_minuend.faces().front();
        Face* current = first;
        do {
            const ClipResult result = m_subtrahend.clip(m_callback.getPlane(current));
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
    
    auto findSubtrahendPlanes() const {
        PlaneList result;
        
        const Face* firstFace = m_subtrahend.faces().front();
        const Face* currentFace = firstFace;
        do {
            const plane<T,3> plane = m_callback.getPlane(currentFace);
            result.push_back(plane);
            currentFace = currentFace->next();
        } while (currentFace != firstFace);
        
        return result;
    }
    
    static PlaneList sortPlanes(PlaneList planes) {
        using VList = typename V::List;

        auto it = std::begin(planes);
        it = sortPlanes(it, std::end(planes), VList({ V::pos_x, V::pos_y, V::pos_z }));
        it = sortPlanes(it, std::end(planes), VList({ V::pos_y, V::pos_x, V::pos_z }));
        it = sortPlanes(it, std::end(planes), VList({ V::pos_z, V::pos_x, V::pos_y }));
        
        return planes;
    }

    static typename PlaneList::iterator sortPlanes(typename PlaneList::iterator begin, typename PlaneList::iterator end, const typename vec<T,3>::List& axes) {
        if (begin == end)
            return end;
        
        auto it = begin;
        while (it != end) {
            auto next = selectPlanes(it, end, axes);
            if (next == it || next == end)
                break; // no further progress
            it = next;
        }
        
        return it;
    }
    
    static typename PlaneList::iterator selectPlanes(typename PlaneList::iterator begin, typename PlaneList::iterator end, const typename vec<T,3>::List& axes) {
        assert(begin != end);
        assert(!axes.empty());
        
        vec<T,3> axis = axes.front();
        auto bestIt = end;
        for (auto it = begin; it != end; ++it) {
            auto newBestIt = selectPlane(it, bestIt, end, axis);
            
            // Resolve ambiguities if necessary.
            for (auto axIt = std::next(std::begin(axes)), axEnd = std::end(axes); newBestIt == end && axIt != axEnd; ++axIt) {
                const vec<T,3>& altAxis = *axIt;
                newBestIt = selectPlane(it, bestIt, end, altAxis);
                if (newBestIt != end)
                    break;
            }
            
            if (newBestIt != end)
                bestIt = newBestIt;
        }
        
        if (bestIt == end)
            return end;

        if (Math::abs(dot(bestIt->normal, axis)) < 0.5)
            return begin;
        
        assert(bestIt != end);
        axis = -bestIt->normal;
        std::iter_swap(begin++, bestIt);
        
        bestIt = end;
        for (auto it = begin; it != end; ++it) {
            const T bestDot = bestIt != end ? dot(bestIt->normal, axis) : 0.0;
            const T curDot  = dot(it->normal, axis);
            
            if (curDot > bestDot)
                bestIt = it;
            if (bestDot == 1.0)
                break;
        }
        
        if (bestIt != end)
            std::iter_swap(begin++, bestIt);
        return begin;
    }
    
    static typename PlaneList::iterator selectPlane(typename PlaneList::iterator curIt, typename PlaneList::iterator bestIt, typename PlaneList::iterator end, const vec<T,3>& axis) {
        const T curDot = dot(curIt->normal, axis);
        if (curDot == 0.0)
            return bestIt;
        if (curDot == 1.0)
            return curIt;

        const T bestDot = bestIt != end ? dot(bestIt->normal, axis) : 0.0;
        if (Math::abs(curDot) > Math::abs(bestDot))
            return curIt;
        
        if (Math::abs(curDot) == Math::abs(bestDot)) {
            // Resolve ambiguities.
            
            assert(bestIt != end); // Because curDot != 0.0, the same is true for bestDot!
            if (bestDot < 0.0 && curDot > 0.0) {
                // Prefer best matches pointing towards the direction of the axis, not the opposite.
                return curIt;
            }
            
            // Could not resolve ambiguities. Caller should try other axes.
            return end;
        }
        
        // Math::abs(curDot) < Math::abs(bestDot)
        return bestIt;
    }
    
    void doSubtract(const List& fragments, PlaneIt curPlaneIt, PlaneIt endPlaneIt) {
        if (fragments.empty() || curPlaneIt == endPlaneIt) {
            // no more fragments to process or all of `minutendFragments`
            // are now behind all of subtrahendPlanes so they can be discarded.
            return;
        }
        
        const auto curPlane = *curPlaneIt;
        const auto curPlaneInv = curPlane.flip();
        
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
