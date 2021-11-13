/*
 Copyright (C) 2021 Kristian Duske

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

#include <functional>

namespace TrenchBroom {
namespace Model {
class Hit;

using HitFilter = std::function<bool(const Hit& hit)>;

namespace HitFilters {
HitFilter any();
HitFilter none();

HitFilter type(HitType::Type typeMask = HitType::AnyType);
HitFilter selected();
HitFilter transitivelySelected();
HitFilter minDistance(FloatType minDistance);
} // namespace HitFilters

HitFilter operator&&(HitFilter lhs, HitFilter rhs);
HitFilter operator||(HitFilter lhs, HitFilter rhs);
HitFilter operator!(HitFilter filter);
} // namespace Model
} // namespace TrenchBroom
