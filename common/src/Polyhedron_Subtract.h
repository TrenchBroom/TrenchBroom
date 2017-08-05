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
        subtract();
    }
    
    const List result() {
        return m_fragments;
    }
private:
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
    
    static PlaneList sortPlanes(const PlaneList& planes) {
        static const T epsilon = Math::Constants<T>::angleEpsilon();
        
        PlaneList x, y, z, yz, xz, xy, other;
        
        for (const auto &plane : planes) {
            if (Math::abs(Math::abs(plane.normal.x()) - 1.0) < epsilon) {
                x.push_back(plane);
            } else if (Math::abs(Math::abs(plane.normal.y()) - 1.0) < epsilon) {
                y.push_back(plane);
            } else if (Math::abs(Math::abs(plane.normal.z()) - 1.0) < epsilon) {
                z.push_back(plane);
            } else if (Math::abs(plane.normal.x()) < epsilon) {
                yz.push_back(plane);
            } else if (Math::abs(plane.normal.y()) < epsilon) {
                xz.push_back(plane);
            } else if (Math::abs(plane.normal.z()) < epsilon) {
                xy.push_back(plane);
            } else {
                other.push_back(plane);
            }
        }
        
        PlaneList result;
        ListUtils::append(result, x);
        ListUtils::append(result, y);
        ListUtils::append(result, z);
        ListUtils::append(result, yz);
        ListUtils::append(result, xz);
        ListUtils::append(result, xy);
        ListUtils::append(result, other);
        assert(result.size() == planes.size());
        return result;
    }
    
    void doSubtract(const List& fragments, PlaneIt curPlaneIt, PlaneIt endPlaneIt) {
        if (curPlaneIt == endPlaneIt) {
            // all of `minutendFragments` are now behind all of subtrahendPlanes
            // so they can be discarded.
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
