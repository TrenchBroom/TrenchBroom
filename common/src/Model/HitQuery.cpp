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
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"

#include <kdl/vector_utils.h>

namespace TrenchBroom {
    namespace Model {
        HitQuery::HitQuery(const std::vector<Hit>& hits) :
        m_hits{&hits},
        m_include{[](const Hit&) { return true; }},
        m_exclude{[](const Hit&) { return false; }} {}

        HitQuery HitQuery::type(const HitType::Type typeMask) && {
            m_include = std::move(m_include) && HitFilters::type(typeMask);
            return std::move(*this);
        }

        HitQuery HitQuery::occluded(const HitType::Type typeMask) && {
            m_exclude = HitFilters::type(typeMask);
            return std::move(*this);
        }

        HitQuery HitQuery::selected() && {
            m_include = std::move(m_include) && HitFilters::selected();
            return std::move(*this);
        }

        HitQuery HitQuery::transitivelySelected() && {
            m_include = std::move(m_include) && HitFilters::transitivelySelected();
            return std::move(*this);
        }

        HitQuery HitQuery::minDistance(const FloatType minDistance_) && {
            m_include = std::move(m_include) && HitFilters::minDistance(minDistance_);
            return std::move(*this);
        }

        bool HitQuery::empty() const {
            return m_hits->empty();
        }

        const Hit& HitQuery::first() const {
            if (!empty()) {
                auto it = std::begin(*m_hits);
                auto end = std::end(*m_hits);
                auto bestMatch = end;

                auto bestMatchError = std::numeric_limits<FloatType>::max();
                auto bestOccluderError = std::numeric_limits<FloatType>::max();

                bool containsOccluder = false;
                while (it != end && !containsOccluder) {
                    const FloatType distance = it->distance();
                    do {
                        const Hit& hit = *it;
                        if (m_include(hit)) {
                            if (hit.error() < bestMatchError) {
                                bestMatch = it;
                                bestMatchError = hit.error();
                            }
                        } else if (!m_exclude(hit)) {
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
            return kdl::vec_filter(*m_hits, m_include);
        }
    }
}
