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

#include "kd/reflection_decl.h"

#include "vm/bbox.h"

#include <iosfwd>
#include <optional>

namespace tb::mdl
{

enum class SoftMapBoundsType
{
  Game,
  Map
};

std::ostream& operator<<(std::ostream& lhs, SoftMapBoundsType rhs);

struct SoftMapBounds
{
  SoftMapBoundsType source;

  /**
   * std::nullopt indicates unlimited soft map bounds
   */
  std::optional<vm::bbox3d> bounds;

  kdl_reflect_decl(SoftMapBounds, source, bounds);
};

} // namespace tb::mdl
