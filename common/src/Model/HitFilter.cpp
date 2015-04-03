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

#include "HitFilter.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/HitAdapter.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class HitFilter::Always : public HitFilter {
        private:
            bool doMatches(const Hit& hit) const {
                return true;
            }
        };
        
        class HitFilter::Never : public HitFilter {
        private:
            bool doMatches(const Hit& hit) const {
                return false;
            }
        };
        
        HitFilter* HitFilter::always() { return new Always(); }
        HitFilter* HitFilter::never() { return new Never(); }
        
        HitFilter::~HitFilter() {}
        
        bool HitFilter::matches(const Hit& hit) const {
            return doMatches(hit);
        }
        
        HitFilterChain::HitFilterChain(const HitFilter* filter, const HitFilter* next) :
        m_filter(filter),
        m_next(next) {
            assert(m_filter != NULL);
            assert(m_next != NULL);
        }
        
        HitFilterChain::~HitFilterChain() {
            delete m_filter;
            delete m_next;
        }
        
        bool HitFilterChain::doMatches(const Hit& hit) const {
            if (!m_filter->matches(hit))
                return false;
            return m_next->matches(hit);
        }
        
        TypedHitFilter::TypedHitFilter(const Hit::HitType typeMask) :
        m_typeMask(typeMask) {}
        
        bool TypedHitFilter::doMatches(const Hit& hit) const {
            return (hit.type() & m_typeMask) != 0;
        }

        bool SelectionHitFilter::doMatches(const Hit& hit) const {
            if (hit.type() == Group::GroupHit)
                return hitToGroup(hit)->selected();
            if (hit.type() == Entity::EntityHit)
                return hitToEntity(hit)->selected();
            if (hit.type() == Brush::BrushHit)
                return hitToBrush(hit)->selected() || hitToFace(hit)->selected();
            return false;
        }
        
        MinDistanceHitFilter::MinDistanceHitFilter(const FloatType minDistance) :
        m_minDistance(minDistance) {}

        bool MinDistanceHitFilter::doMatches(const Hit& hit) const {
            return hit.distance() >= m_minDistance;
        }

        ContextHitFilter::ContextHitFilter(const EditorContext& context) :
        m_context(context) {}
        
        bool ContextHitFilter::doMatches(const Hit& hit) const {
            if (hit.type() == Group::GroupHit)
                return m_context.pickable(hitToGroup(hit));
            if (hit.type() == Entity::EntityHit)
                return m_context.pickable(hitToEntity(hit));
            if (hit.type() == Brush::BrushHit)
                return m_context.pickable(hitToBrush(hit));
            return false;
        }
    }
}
