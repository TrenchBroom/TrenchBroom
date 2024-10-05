/*
 Copyright (C) 2010 Kristian Duske

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

#include "Model/HitType.h"

#include "vm/forward.h"
#include "vm/vec.h" // IWYU pragma: keep

#include <any>

namespace TrenchBroom::Model
{
class Hit
{
public:
  static const Hit NoHit;

private:
  HitType::Type m_type;
  double m_distance;
  vm::vec3d m_hitPoint;
  std::any m_target;
  double m_error;

public:
  template <typename T>
  Hit(
    const HitType::Type type,
    const double distance,
    const vm::vec3d& hitPoint,
    T target,
    const double error = 0.0)
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
  double distance() const;
  const vm::vec3d& hitPoint() const;
  double error() const;

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
} // namespace TrenchBroom::Model
