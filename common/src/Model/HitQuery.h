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

#pragma once

#include "FloatType.h"
#include "Model/HitType.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Hit;

        class HitQuery {
        private:
            using HitFilter = std::function<bool(const Hit&)>;

            const std::vector<Hit>* m_hits;
            HitFilter m_include;
            HitFilter m_exclude;
        public:
            explicit HitQuery(const std::vector<Hit>& hits);

            HitQuery type(HitType::Type typeMask) &&;
            HitQuery occluded(HitType::Type typeMask = HitType::AnyType) &&;
            HitQuery selected() &&;
            HitQuery transitivelySelected() &&;
            HitQuery minDistance(FloatType minDistance) &&;

            bool empty() const;
            const Hit& first() const;
            std::vector<Hit> all() const;
        };
    }
}

