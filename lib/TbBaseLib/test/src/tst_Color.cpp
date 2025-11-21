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

#include "Color.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb
{

TEST_CASE("mixColors")
{
  CHECK(
    mixColors(RgbF{0.0f, 0.0f, 0.0f}, RgbF{1.0f, 1.0f, 1.0f}, 0.5f)
    == RgbF{0.5f, 0.5f, 0.5f});

  CHECK(
    mixColors(RgbaF{0.0f, 0.0f, 0.0f, 0.0f}, RgbaF{1.0f, 0.0f, 0.0f, 1.0f}, 0.25f)
    == RgbaF{0.25f, 0.0f, 0.0f, 0.25f});

  CHECK(
    mixColors(RgbF{0.2f, 0.3f, 0.4f}, RgbF{0.8f, 0.9f, 1.0f}, -0.5f)
    == RgbF{0.2f, 0.3f, 0.4f});
  CHECK(
    mixColors(RgbF{0.2f, 0.3f, 0.4f}, RgbF{0.8f, 0.9f, 1.0f}, 1.5f)
    == RgbF{0.8f, 0.9f, 1.0f});

  CHECK(
    mixColors(RgbaF{0.2f, 0.3f, 0.4f, 0.1f}, RgbaF{0.8f, 0.9f, 1.0f, 0.9f}, -0.5f)
    == RgbaF{0.2f, 0.3f, 0.4f, 0.1f});
  CHECK(
    mixColors(RgbaF{0.2f, 0.3f, 0.4f, 0.1f}, RgbaF{0.8f, 0.9f, 1.0f, 0.9f}, 2.0f)
    == RgbaF{0.8f, 0.9f, 1.0f, 0.9f});
}

TEST_CASE("blendColor")
{
  CHECK(blendColor(RgbaF{0.1f, 0.2f, 0.3f, 0.5f}, 0.6f) == RgbaF{0.1f, 0.2f, 0.3f, 0.3f});
}

} // namespace tb
