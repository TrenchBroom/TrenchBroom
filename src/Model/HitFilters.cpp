/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "HitFilters.h"

#include "Model/Filter.h"
#include "Model/Entity.h"
#include "Model/Brush.h"
#include "Model/BrushFAce.h"
#include "Model/HitAdapter.h"

namespace TrenchBroom {
    namespace Model {
        bool HitFilterChain::matches(const Hit& hit) const {
            if (!m_filter->matches(hit))
                return false;
            return m_next->matches(hit);
        }
        
        TypedHitFilter::TypedHitFilter(const Hit::HitType typeMask) :
        m_typeMask(typeMask) {}
        
        bool TypedHitFilter::matches(const Hit& hit) const {
            return (hit.type() & m_typeMask) != 0;
        }
        
        DefaultHitFilter::DefaultHitFilter(const Filter& filter) :
        m_filter(filter) {}
        
        bool DefaultHitFilter::matches(const Hit& hit) const {
            if (hit.type() == Entity::EntityHit)
                return m_filter.pickable(hitAsEntity(hit));
            if (hit.type() == Brush::BrushHit)
                return m_filter.pickable(hitAsBrush(hit));
            return false;
        }
        
        PickResult::FirstHit firstHit(const PickResult& pickResult, const Hit::HitType type, const bool ignoreOccluders) {
            return pickResult.firstHit(TypedHitFilter(type), ignoreOccluders);
        }
        
        PickResult::FirstHit firstHit(const PickResult& pickResult, const Hit::HitType type, const Filter& modelFilter, const bool ignoreOccluders) {
            return pickResult.firstHit(chainHitFilters(TypedHitFilter(type), DefaultHitFilter(modelFilter)), ignoreOccluders);
        }
    }
}
