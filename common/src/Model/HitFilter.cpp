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

#include "Ensure.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"

namespace TrenchBroom {
    namespace Model {
        class HitFilter::Always : public HitFilter {
        private:
            std::unique_ptr<HitFilter> doClone() const override {
                return std::make_unique<Always>();
            }

            bool doMatches(const Hit&) const override {
                return true;
            }
        };

        class HitFilter::Never : public HitFilter {
        private:
            std::unique_ptr<HitFilter> doClone() const override {
                return std::make_unique<Never>();
            }

            bool doMatches(const Hit&) const override {
                return false;
            }
        };

        std::unique_ptr<HitFilter> HitFilter::always() { return std::make_unique<Always>(); }
        std::unique_ptr<HitFilter> HitFilter::never() { return std::make_unique<Never>(); }

        HitFilter::~HitFilter() = default;

        std::unique_ptr<HitFilter> HitFilter::clone() const {
            return doClone();
        }

        bool HitFilter::matches(const Hit& hit) const {
            return doMatches(hit);
        }

        HitFilterChain::HitFilterChain(std::unique_ptr<const HitFilter> filter, std::unique_ptr<const HitFilter> next) :
        m_filter(std::move(filter)),
        m_next(std::move(next)) {
            ensure(m_filter != nullptr, "filter is null");
            ensure(m_next != nullptr, "next is null");
        }

        HitFilterChain::~HitFilterChain() = default;

        std::unique_ptr<HitFilter> HitFilterChain::doClone() const {
            return std::make_unique<HitFilterChain>(m_filter->clone(), m_next->clone());
        }

        bool HitFilterChain::doMatches(const Hit& hit) const {
            if (!m_filter->matches(hit)) {
                return false;
            } else {
                return m_next->matches(hit);
            }
        }

        TypedHitFilter::TypedHitFilter(const HitType::Type typeMask) :
        m_typeMask(typeMask) {}

        std::unique_ptr<HitFilter> TypedHitFilter::doClone() const {
            return std::make_unique<TypedHitFilter>(m_typeMask);
        }

        bool TypedHitFilter::doMatches(const Hit& hit) const {
            return (hit.type() & m_typeMask) != 0;
        }

        std::unique_ptr<HitFilter> SelectionHitFilter::doClone() const {
            return std::make_unique<SelectionHitFilter>();
        }

        bool SelectionHitFilter::doMatches(const Hit& hit) const {
            if (hit.type() == EntityNode::EntityHitType) {
                return hitToEntity(hit)->selected();
            } else if (hit.type() == BrushNode::BrushHitType) {
                return hitToBrush(hit)->selected() || hitToFace(hit)->selected();
            } else {
                return false;
            }
        }

        std::unique_ptr<HitFilter> TransitivelySelectedHitFilter::doClone() const {
            return std::make_unique<TransitivelySelectedHitFilter>();
        }

        bool TransitivelySelectedHitFilter::doMatches(const Hit& hit) const {
            if (hit.type() == EntityNode::EntityHitType) {
                return hitToEntity(hit)->transitivelySelected();
            } else if (hit.type() == BrushNode::BrushHitType) {
                return hitToBrush(hit)->transitivelySelected() || hitToFace(hit)->selected();
            } else {
                return false;
            }
        }

        MinDistanceHitFilter::MinDistanceHitFilter(const FloatType minDistance) :
        m_minDistance(minDistance) {}

        std::unique_ptr<HitFilter> MinDistanceHitFilter::doClone() const {
            return std::make_unique<MinDistanceHitFilter>(m_minDistance);
        }

        bool MinDistanceHitFilter::doMatches(const Hit& hit) const {
            return hit.distance() >= m_minDistance;
        }

        ContextHitFilter::ContextHitFilter(const EditorContext& context) :
        m_context(context) {}

        std::unique_ptr<HitFilter> ContextHitFilter::doClone() const {
            return std::make_unique<ContextHitFilter>(m_context);
        }

        bool ContextHitFilter::doMatches(const Hit& hit) const {
            if (hit.type() == EntityNode::EntityHitType) {
                return m_context.pickable(hitToEntity(hit));
            } else if (hit.type() == BrushNode::BrushHitType) {
                return m_context.pickable(hitToBrush(hit), hitToFace(hit));
            } else {
                return false;
            }
        }
    }
}
