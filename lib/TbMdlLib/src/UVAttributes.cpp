/*
 Copyright (C) 2026 Thomas Jones

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

#include "mdl/UVAttributes.h"

#include "kd/reflection_impl.h"

#include "vm/vec_io.h" // IWYU pragma: keep

namespace tb::mdl
{

kdl_reflect_impl(UVAttributes);

vm::vec2f modOffset(const vm::vec2f& offset, const vm::vec2f& textureSize)
{
  return offset - snapDown(offset, textureSize);
}

bool isValid(const UVAttributes& uvAttributes)
{
  return !vm::is_zero(uvAttributes.scale.x(), vm::Cf::almost_zero())
         && !vm::is_zero(uvAttributes.scale.y(), vm::Cf::almost_zero());
}

} // namespace tb::mdl
