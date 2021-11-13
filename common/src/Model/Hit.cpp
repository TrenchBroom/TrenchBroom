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

#include "Hit.h"

#include "Ensure.h"

namespace TrenchBroom {
namespace Model {
const Hit Hit::NoHit = Hit(HitType::NoType, 0.0, vm::vec3::zero(), false);

bool Hit::isMatch() const {
  return m_type != HitType::NoType;
}

HitType::Type Hit::type() const {
  return m_type;
}

bool Hit::hasType(const HitType::Type typeMask) const {
  return (m_type & typeMask) != 0;
}

FloatType Hit::distance() const {
  return m_distance;
}

const vm::vec3& Hit::hitPoint() const {
  return m_hitPoint;
}

FloatType Hit::error() const {
  return m_error;
}

Hit selectClosest(const Hit& first, const Hit& second) {
  if (!first.isMatch()) {
    return second;
  }
  if (!second.isMatch()) {
    return first;
  }
  if (first.distance() <= second.distance()) {
    return first;
  } else {
    return second;
  }
}
} // namespace Model
} // namespace TrenchBroom
