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

#include "mdl/SoftMapBounds.h"

#include "kd/reflection_impl.h"

#include "vm/bbox_io.h" // IWYU pragma: keep

#include <ostream>
#include <sstream>

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

std::optional<vm::bbox3d> parseSoftMapBounds(const std::string_view str)
{
  if (const auto v = vm::parse<double, 6u>(str))
  {
    return vm::bbox3d{{(*v)[0], (*v)[1], (*v)[2]}, {(*v)[3], (*v)[4], (*v)[5]}};
  }
  return std::nullopt;
}

std::string serializeSoftMapBounds(const vm::bbox3d& bounds)
{
  auto result = std::stringstream{};
  result << bounds.min << " " << bounds.max;
  return result.str();
}

} // namespace tb::mdl
