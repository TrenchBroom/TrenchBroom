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

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("UVAttributes")
{
  SECTION("modOffset")
  {
    CHECK(modOffset(vm::vec2f{0, 0}, vm::vec2f{64, 64}) == vm::vec2f{0, 0});
    CHECK(modOffset(vm::vec2f{32, 16}, vm::vec2f{64, 64}) == vm::vec2f{32, 16});
    CHECK(modOffset(vm::vec2f{64, 64}, vm::vec2f{64, 64}) == vm::vec2f{0, 0});
    CHECK(modOffset(vm::vec2f{96, 128}, vm::vec2f{64, 64}) == vm::vec2f{32, 0});
    CHECK(modOffset(vm::vec2f{-32, -64}, vm::vec2f{64, 64}) == vm::vec2f{-32, 0});
    CHECK(modOffset(vm::vec2f{80, 48}, vm::vec2f{64, 32}) == vm::vec2f{16, 16});
  }

  SECTION("isValid")
  {
    CHECK(isValid(UVAttributes{}));
    CHECK(isValid(UVAttributes{{16, 32}, {2, -2}, 45.0f}));
    CHECK_FALSE(isValid(UVAttributes{{0, 0}, {0, 1}, 0.0f}));
    CHECK_FALSE(isValid(UVAttributes{{0, 0}, {1, 0}, 0.0f}));
    CHECK_FALSE(isValid(UVAttributes{{0, 0}, {0, 0}, 0.0f}));
  }
}

} // namespace tb::mdl
