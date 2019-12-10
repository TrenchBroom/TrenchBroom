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

#include "Constants.h"
#include "Model/EditorContext.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"

#include <vecmath/scalar.h>

namespace TrenchBroom {
    namespace Model {
        HitQuery::HitQuery(const std::list<Hit>& hits, const EditorContext& editorContext) :
        m_hits(&hits),
        m_editorContext(&editorContext),
        m_include(HitFilter::always()),
        m_exclude(HitFilter::never()) {}

        HitQuery::HitQuery(const std::list<Hit>& hits) :
        m_hits(&hits),
        m_editorContext(nullptr),
        m_include(HitFilter::always()),
        m_exclude(HitFilter::never()) {}

        HitQuery::HitQuery(const HitQuery& other) :
        m_hits(other.m_hits),
        m_editorContext(other.m_editorContext),
        m_include(other.m_include->clone()),
        m_exclude(other.m_exclude->clone()) {}

        HitQuery::~HitQuery() {
            delete m_include;
            delete m_exclude;
        }

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
                m_include = new HitFilterChain(new ContextHitFilter(*m_editorContext), m_include);
            }
            return *this;
        }

        HitQuery& HitQuery::type(const HitType::Type type) {
            m_include = new HitFilterChain(new TypedHitFilter(type), m_include);
            return *this;
        }

        HitQuery& HitQuery::occluded(const HitType::Type type) {
            delete m_exclude;
            m_exclude = new TypedHitFilter(type);
            return *this;
        }

        HitQuery& HitQuery::selected() {
            m_include = new HitFilterChain(new Model::SelectionHitFilter(), m_include);
            return *this;
        }

        HitQuery& HitQuery::transitivelySelected() {
            m_include = new HitFilterChain(new Model::TransitivelySelectedHitFilter(), m_include);
            return *this;
        }

        HitQuery& HitQuery::minDistance(const FloatType minDistance) {
            m_include = new HitFilterChain(new Model::MinDistanceHitFilter(minDistance), m_include);
            return *this;
        }

        bool HitQuery::empty() const {
            return m_hits->empty();
        }

        const Hit& HitQuery::first() const {
            if (!m_hits->empty()) {
                std::list<Hit>::const_iterator it = m_hits->begin();
                const std::list<Hit>::const_iterator end = m_hits->end();
                std::list<Hit>::const_iterator bestMatch = end;
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

                if (bestMatch != end && bestMatchError <= bestOccluderError)
                    return *bestMatch;
            }
            return Hit::NoHit;
        }

        std::list<Hit> HitQuery::all() const {
            std::list<Hit> result;
            for (const Hit& hit : *m_hits) {
                if (m_include->matches(hit))
                    result.push_back(hit);
            }
            return result;
        }

        bool HitQuery::visible(const Hit& hit) const {
            if (m_editorContext == nullptr)
                return true;
            Node* node = hitToNode(hit);
            if (node == nullptr)
                return true;
            return m_editorContext->visible(hitToNode(hit));
        }
    }
}
