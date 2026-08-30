/*
 Copyright (C) 2026 Kristian Duske

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

#include "mdl/CatchConfig.h"
#include "mdl/UvAttributes.h"

#include "vm/vec_io.h" // IWYU pragma: keep

#include <limits>
#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{

TEST_CASE("UvAttributes")
{
  static constexpr auto nan = std::numeric_limits<float>::quiet_NaN();
  static constexpr auto inf = std::numeric_limits<float>::infinity();

  SECTION("validateUvAttributes")
  {
    // clang-format off
    const auto
    [uvAttributes,             expected] = GENERATE(table<UvAttributes, bool>({
    // finite offset, scale and rotation
    {{{1, 2}, {3, 4}, 45.0f},  true},
    // a scale of 0 is still finite
    {{{0, 0}, {0, 0}, 0.0f},   true},
    // non-finite offset
    {{{nan, 0}, {1, 1}, 0.0f}, false},
    {{{0, inf}, {1, 1}, 0.0f}, false},
    // non-finite scale
    {{{0, 0}, {nan, 1}, 0.0f}, false},
    {{{0, 0}, {1, inf}, 0.0f}, false},
    // non-finite rotation
    {{{0, 0}, {1, 1}, nan},    false},
    {{{0, 0}, {1, 1}, inf},    false},
    }));
    // clang-format on

    CAPTURE(uvAttributes);
    CHECK(
      validateUvAttributes(uvAttributes.offset, uvAttributes.scale, uvAttributes.rotation)
      == expected);
  }

  SECTION("valid")
  {
    // clang-format off
    const auto 
    [uvAttributes,             expected] = GENERATE(table<UvAttributes, bool>({
    // finite, non-zero scale
    {{{1, 2}, {3, 4}, 45.0f},  true},
    // a scale of 0 in either component is invalid, even though it's finite
    {{{0, 0}, {0, 1}, 0.0f},   false},
    {{{0, 0}, {1, 0}, 0.0f},   false},
    // non-finite values are invalid too
    {{{nan, 0}, {1, 1}, 0.0f}, false},
    }));
    // clang-format on

    CAPTURE(uvAttributes);
    CHECK(uvAttributes.valid() == expected);
  }
}

} // namespace tb::mdl
