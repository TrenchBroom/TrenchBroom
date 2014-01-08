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

#include "Picker.h"

#include "Model/Pickable.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
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

        HitFilter::~HitFilter() {}
        
        PickResult::FirstHit::FirstHit(const bool i_matches, const Hit& i_hit) :
        matches(i_matches),
        hit(i_hit) {}

        bool PickResult::CompareHits::operator() (const Hit& left, const Hit& right) const {
            return left.distance() < right.distance();
        }
        
        PickResult::FirstHit PickResult::firstHit(const HitFilter& filter, const bool ignoreOccluders) const {
            if (!m_hits.empty()) {
                Hit::List::const_iterator it = m_hits.begin();
                Hit::List::const_iterator end = m_hits.end();
                if (!ignoreOccluders) {
                    const Hit& first = *it;
                    if (filter.matches(first))
                        return FirstHit(true, first);
                    
                    const FloatType closest = (it++)->distance();
                    while (it != end) {
                        const Hit& hit = *it;
                        if (hit.distance() > closest)
                            break;
                        if (filter.matches(hit))
                            return FirstHit(true, hit);
                        ++it;
                    }
                } else {
                    while (it != end) {
                        const Hit& hit = *it;
                        if (filter.matches(hit))
                            return FirstHit(true, hit);
                        ++it;
                    }
                }
            }
            return FirstHit(false, Hit::NoHit);
        }
        
        Hit::List PickResult::hits(const HitFilter& filter) const {
            Hit::List result;
            Hit::List::const_iterator it, end;
            for (it = m_hits.begin(), end = m_hits.end(); it != end; ++it) {
                const Hit& hit = *it;
                if (filter.matches(hit))
                    result.push_back(hit);
            }
            return result;
        }
        
        Hit::List PickResult::allHits() const {
            return m_hits;
        }

        void PickResult::addHit(const Hit& hit) {
            m_hits.push_back(hit);
        }
        
        void PickResult::sortHits() {
            std::sort(m_hits.begin(), m_hits.end(), CompareHits());
        }

        Picker::Picker(const BBox<FloatType, 3>& worldBounds) :
        m_octree(worldBounds, static_cast<FloatType>(64.0f)) {}

        void Picker::addObject(Pickable* object) {
            m_octree.addObject(object->bounds(), object);
        }
        
        void Picker::removeObject(Pickable* object) {
            m_octree.removeObject(object->bounds(), object);
        }
        
        PickResult Picker::pick(const Ray3& ray) {
            PickResult result;
            const PickableList candidates = m_octree.findObjects(ray);
            PickableList::const_iterator it, end;
            for (it = candidates.begin(), end = candidates.end(); it != end; ++it) {
                Pickable* object = *it;
                object->pick(ray, result);
            }
            result.sortHits();
            return result;
        }
    }
}
