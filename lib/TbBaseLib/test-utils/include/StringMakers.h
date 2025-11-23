/*
 Copyright (C) 2023 Kristian Duske

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

#include "kd/result_io.h" // IWYU pragma: keep

#include "vm/bbox_io.h"  // IWYU pragma: keep
#include "vm/line_io.h"  // IWYU pragma: keep
#include "vm/mat_io.h"   // IWYU pragma: keep
#include "vm/plane_io.h" // IWYU pragma: keep
#include "vm/ray_io.h"   // IWYU pragma: keep
#include "vm/vec_io.h"   // IWYU pragma: keep

#include <catch2/catch_test_macros.hpp>

namespace Catch
{

template <typename Value, typename... Errors>
struct StringMaker<kdl::result<Value, Errors...>>
{
  static std::string convert(const kdl::result<Value, Errors...>& result)
  {
    auto str = std::stringstream{};
    str << result;
    return str.str();
  }
};

} // namespace Catch