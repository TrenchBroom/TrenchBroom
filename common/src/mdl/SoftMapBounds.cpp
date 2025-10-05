/*
 Copyright (C) 2025 Kristian Duske

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

#include "SoftMapBounds.h"

#include "kdl/reflection_impl.h"

#include "vm/bbox_io.h" // IWYU pragma: keep

#include <ostream>

namespace tb::mdl
{

std::ostream& operator<<(std::ostream& lhs, const SoftMapBoundsType rhs)
{
  switch (rhs)
  {
  case SoftMapBoundsType::Game:
    lhs << "Game";
    break;
  case SoftMapBoundsType::Map:
    lhs << "Map";
    break;
  }
  return lhs;
}

kdl_reflect_impl(SoftMapBounds);

} // namespace tb::mdl
