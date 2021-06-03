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
            return firstHit(m_include, *m_hits);
        }

        std::vector<Hit> HitQuery::all() const {
            return allHits(m_include, *m_hits);
        }
    }
}
