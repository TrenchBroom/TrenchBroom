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
#include "Macros.h"
#include "Model/HitType.h"

#include "vecmath/vec.h"

#include <any>

namespace TrenchBroom
{
namespace Model
{
class Hit
{
public:
  static const Hit NoHit;

private:
  HitType::Type m_type;
  FloatType m_distance;
  vm::vec3 m_hitPoint;
  std::any m_target;
  FloatType m_error;

public:
  template <typename T>
  Hit(
    const HitType::Type type,
    const FloatType distance,
    const vm::vec3& hitPoint,
    T target,
    const FloatType error = 0.0)
    : m_type(type)
    , m_distance(distance)
    , m_hitPoint(hitPoint)
    , m_target(std::move(target))
    , m_error(error)
  {
  }

  bool isMatch() const;
  HitType::Type type() const;
  bool hasType(HitType::Type typeMask) const;
  FloatType distance() const;
  const vm::vec3& hitPoint() const;
  FloatType error() const;

  template <typename T>
  T target() const
  {
    return std::any_cast<T>(m_target);
  }
};

Hit selectClosest(const Hit& first, const Hit& second);

template <typename... Hits>
Hit selectClosest(const Hit& first, const Hits&... rest)
{
  return selectClosest(first, selectClosest(rest...));
}
} // namespace Model
} // namespace TrenchBroom
