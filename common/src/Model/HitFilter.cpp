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
            HitFilter* doClone() const override {
                return new Always();
            }

            bool doMatches(const Hit& hit) const override {
                return true;
            }
        };

        class HitFilter::Never : public HitFilter {
        private:
            HitFilter* doClone() const override {
                return new Never();
            }

            bool doMatches(const Hit& hit) const override {
                return false;
            }
        };

        HitFilter* HitFilter::always() { return new Always(); }
        HitFilter* HitFilter::never() { return new Never(); }

        HitFilter::~HitFilter() {}

        HitFilter* HitFilter::clone() const {
            return doClone();
        }

        bool HitFilter::matches(const Hit& hit) const {
            return doMatches(hit);
        }

        HitFilterChain::HitFilterChain(const HitFilter* filter, const HitFilter* next) :
        m_filter(filter),
        m_next(next) {
            ensure(m_filter != nullptr, "filter is null");
            ensure(m_next != nullptr, "next is null");
        }

        HitFilterChain::~HitFilterChain() {
            delete m_filter;
            delete m_next;
        }

        HitFilter* HitFilterChain::doClone() const {
            return new HitFilterChain(m_filter->clone(), m_next->clone());
        }

        bool HitFilterChain::doMatches(const Hit& hit) const {
            if (!m_filter->matches(hit))
                return false;
            return m_next->matches(hit);
        }

        TypedHitFilter::TypedHitFilter(const Hit::HitType typeMask) :
        m_typeMask(typeMask) {}

        HitFilter* TypedHitFilter::doClone() const {
            return new TypedHitFilter(m_typeMask);
        }

        bool TypedHitFilter::doMatches(const Hit& hit) const {
            return (hit.type() & m_typeMask) != 0;
        }

        HitFilter* SelectionHitFilter::doClone() const {
            return new SelectionHitFilter();
        }

        bool SelectionHitFilter::doMatches(const Hit& hit) const {
            if (hit.type() == Entity::EntityHit)
                return hitToEntity(hit)->selected();
            if (hit.type() == Brush::BrushHit)
                return hitToBrush(hit)->selected() || hitToFace(hit)->selected();
            return false;
        }

        MinDistanceHitFilter::MinDistanceHitFilter(const FloatType minDistance) :
        m_minDistance(minDistance) {}

        HitFilter* MinDistanceHitFilter::doClone() const {
            return new MinDistanceHitFilter(m_minDistance);
        }

        bool MinDistanceHitFilter::doMatches(const Hit& hit) const {
            return hit.distance() >= m_minDistance;
        }

        ContextHitFilter::ContextHitFilter(const EditorContext& context) :
        m_context(context) {}

        HitFilter* ContextHitFilter::doClone() const {
            return new ContextHitFilter(m_context);
        }

        bool ContextHitFilter::doMatches(const Hit& hit) const {
            if (hit.type() == Entity::EntityHit) {
                return m_context.pickable(hitToEntity(hit));
            }
            if (hit.type() == Brush::BrushHit) {
                return m_context.pickable(hitToFace(hit));
            }
            return false;
        }
    }
}
