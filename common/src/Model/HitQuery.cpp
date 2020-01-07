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

#include "HitQuery.h"

#include "FloatType.h"
#include "Model/EditorContext.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"

#include <vecmath/scalar.h>

namespace TrenchBroom {
    namespace Model {
        HitQuery::HitQuery(const std::vector<Hit>& hits, const EditorContext& editorContext) :
        m_hits(&hits),
        m_editorContext(&editorContext),
        m_include(HitFilter::always()),
        m_exclude(HitFilter::never()) {}

        HitQuery::HitQuery(const std::vector<Hit>& hits) :
        m_hits(&hits),
        m_editorContext(nullptr),
        m_include(HitFilter::always()),
        m_exclude(HitFilter::never()) {}

        HitQuery::HitQuery(const HitQuery& other) :
        m_hits(other.m_hits),
        m_editorContext(other.m_editorContext),
        m_include(other.m_include->clone()),
        m_exclude(other.m_exclude->clone()) {}

        HitQuery::~HitQuery() = default;

        HitQuery& HitQuery::operator=(HitQuery other) {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        void swap(HitQuery& lhs, HitQuery& rhs) {
            using std::swap;
            swap(lhs.m_hits, rhs.m_hits);
            swap(lhs.m_editorContext, rhs.m_editorContext);
            swap(lhs.m_include, rhs.m_include);
            swap(lhs.m_exclude, rhs.m_exclude);
        }

        HitQuery& HitQuery::pickable() {
            if (m_editorContext != nullptr) {
                m_include = std::make_unique<HitFilterChain>(std::make_unique<ContextHitFilter>(*m_editorContext), std::move(m_include));
            }
            return *this;
        }

        HitQuery& HitQuery::type(const HitType::Type type) {
            m_include = std::make_unique<HitFilterChain>(std::make_unique<TypedHitFilter>(type), std::move(m_include));
            return *this;
        }

        HitQuery& HitQuery::occluded(const HitType::Type type) {
            m_exclude = std::make_unique<TypedHitFilter>(type);
            return *this;
        }

        HitQuery& HitQuery::selected() {
            m_include = std::make_unique<HitFilterChain>(std::make_unique<SelectionHitFilter>(), std::move(m_include));
            return *this;
        }

        HitQuery& HitQuery::transitivelySelected() {
            m_include = std::make_unique<HitFilterChain>(std::make_unique<TransitivelySelectedHitFilter>(), std::move(m_include));
            return *this;
        }

        HitQuery& HitQuery::minDistance(const FloatType minDistance) {
            m_include = std::make_unique<HitFilterChain>(std::make_unique<MinDistanceHitFilter>(minDistance), std::move(m_include));
            return *this;
        }

        bool HitQuery::empty() const {
            return m_hits->empty();
        }

        const Hit& HitQuery::first() const {
            if (!m_hits->empty()) {
                auto it = m_hits->begin();
                auto end = m_hits->end();
                auto bestMatch = end;

                FloatType bestMatchError = std::numeric_limits<FloatType>::max();
                FloatType bestOccluderError = std::numeric_limits<FloatType>::max();

                bool containsOccluder = false;
                while (it != end && !containsOccluder) {
                    if (!visible(*it)) { // Don't consider hidden objects during picking at all.
                        ++it;
                        continue;
                    }

                    const FloatType distance = it->distance();
                    do {
                        const Hit& hit = *it;
                        if (m_include->matches(hit)) {
                            if (hit.error() < bestMatchError) {
                                bestMatch = it;
                                bestMatchError = hit.error();
                            }
                        } else if (!m_exclude->matches(hit)) {
                            bestOccluderError = vm::min(bestOccluderError, hit.error());
                            containsOccluder = true;
                        }
                        ++it;
                    } while (it != end && vm::is_equal(it->distance(), distance, vm::C::almost_zero()));
                }

                if (bestMatch != end && bestMatchError <= bestOccluderError) {
                    return *bestMatch;
                }
            }
            return Hit::NoHit;
        }

        std::vector<Hit> HitQuery::all() const {
            std::vector<Hit> result;
            for (const Hit& hit : *m_hits) {
                if (m_include->matches(hit)) {
                    result.push_back(hit);
                }
            }
            return result;
        }

        bool HitQuery::visible(const Hit& hit) const {
            if (m_editorContext == nullptr) {
                return true;
            }

            Node* node = hitToNode(hit);
            if (node == nullptr) {
                return true;
            }

            return m_editorContext->visible(hitToNode(hit));
        }
    }
}
