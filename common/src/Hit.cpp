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

#include "Hit.h"
#include "HitFilter.h"

#include <algorithm>
#include <limits>

namespace TrenchBroom {
    const Hit::HitType Hit::NoType = 0;
    const Hit::HitType Hit::AnyType = 0xFFFFFFFF;
    
    Hit::HitType Hit::freeHitType() {
        static HitType currentType = 1;
        const HitType result = currentType;
        currentType = currentType << 1;
        return result;
    }
    
    const Hit Hit::NoHit = Hit(NoType, 0.0, Vec3::Null, false);
    
    bool Hit::isMatch() const {
        return m_type != NoType;
    }
    
    Hit::HitType Hit::type() const {
        return m_type;
    }
    
    bool Hit::hasType(const HitType typeMask) const {
        return (m_type & typeMask) != 0;
    }
    
    FloatType Hit::distance() const {
        return m_distance;
    }
    
    const Vec3& Hit::hitPoint() const {
        return m_hitPoint;
    }
    
    FloatType Hit::error() const {
        return m_error;
    }

    CompareHits::~CompareHits() {}
    
    int CompareHits::compare(const Hit& lhs, const Hit& rhs) const {
        return doCompare(lhs, rhs);
    }

    int CompareHitsByDistance::doCompare(const Hit& lhs, const Hit& rhs) const {
        if (lhs.distance() < rhs.distance())
            return -1;
        if (lhs.distance() > rhs.distance())
            return 1;
        return 0;
    }

    class Hits::CompareWrapper {
    private:
        const CompareHits* m_compare;
    public:
        CompareWrapper(const CompareHits* compare) : m_compare(compare) {}
        bool operator()(const Hit& lhs, const Hit& rhs) const { return m_compare->compare(lhs, rhs) < 0; }
    };
    
    Hits::Hits() {}

    bool Hits::empty() const {
        return m_hits.empty();
    }
    
    size_t Hits::size() const {
        return m_hits.size();
    }

    void Hits::addHit(const Hit& hit) {
        assert(m_compare != NULL);
        List::iterator pos = std::upper_bound(m_hits.begin(), m_hits.end(), hit, CompareWrapper(m_compare.get()));
        m_hits.insert(pos, hit);
    }
    
    const Hit& Hits::findFirst(const Hit::HitType type, const bool ignoreOccluders) const {
        return findFirst(TypedHitFilter(type), ignoreOccluders);
    }
    
    const Hit& Hits::findFirst(const Hit::HitType type, const Hit::HitType ignoreOccluderMask) const {
        return findFirst(TypedHitFilter(type), ignoreOccluderMask);
    }
    
    const Hit& Hits::findFirst(const Hit::HitType type, const HitFilter& ignoreFilter) const {
        return findFirst(TypedHitFilter(type), ignoreFilter);
    }
    
    const Hit& Hits::findFirst(const HitFilter& filter, const bool ignoreOccluders) const {
        return findFirst(filter, ignoreOccluders ? Hit::AnyType : Hit::NoType);
    }
    
    const Hit& Hits::findFirst(const HitFilter& filter, const Hit::HitType ignoreOccluderMask) const {
        return findFirst(filter, TypedHitFilter(ignoreOccluderMask));
    }
    
    const Hit& Hits::findFirst(const HitFilter& include, const HitFilter& exclude) const {
        if (!m_hits.empty()) {
            List::const_iterator it = m_hits.begin();
            List::const_iterator end = m_hits.end();
            List::const_iterator bestMatch = m_hits.end();
            FloatType bestMatchError = std::numeric_limits<FloatType>::max();
            FloatType bestOccluderError = std::numeric_limits<FloatType>::max();
            
            bool containsOccluder = false;
            while (it != end && !containsOccluder) {
                const FloatType distance = it->distance();
                do {
                    const Hit& hit = *it;
                    if (include(hit)) {
                        if (hit.error() < bestMatchError) {
                            bestMatch = it;
                            bestMatchError = hit.error();
                        }
                    } else if (!exclude(hit)) {
                        bestOccluderError = std::min(bestOccluderError, hit.error());
                        containsOccluder = true;
                    }
                    ++it;
                } while (it != end && Math::eq(it->distance(), distance));
            }
            
            if (bestMatch != m_hits.end() && bestMatchError <= bestOccluderError)
                return *bestMatch;
        }
        return Hit::NoHit;
    }
    
    const Hits::List& Hits::all() const {
        return m_hits;
    }
    
    Hits::List Hits::filter(const Hit::HitType type) const {
        return filter(TypedHitFilter(type));
    }

    Hits::List Hits::filter(const HitFilter& include) const {
        Hits::List result;
        Hits::List::const_iterator it, end;
        for (it = m_hits.begin(), end = m_hits.end(); it != end; ++it) {
            const Hit& hit = *it;
            if (include(hit))
                result.push_back(hit);
        }
        return result;
    }

    Hits hitsByDistance() {
        return Hits((CompareHitsByDistance()));
    }
}
