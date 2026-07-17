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

#pragma once

#include "kd/reflection_decl.h"

#include "vm/vec.h"

namespace tb::mdl
{

struct UVAttributes
{
  vm::vec2f offset = vm::vec2f{0, 0};
  vm::vec2f scale = vm::vec2f{1, 1};
  float rotation = 0.0f;

  kdl_reflect_decl(UVAttributes, offset, scale, rotation);
};

vm::vec2f modOffset(const vm::vec2f& offset, const vm::vec2f& textureSize);

bool isValid(const UVAttributes& uvAttributes);

} // namespace tb::mdl
