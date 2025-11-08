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

#include "catch/CatchConfig.h"

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

TEST_CASE("rgbToHSB")
{
  float h = 0.0f;
  float s = 0.0f;
  float br = 0.0f;

  // red -> hue 0, saturation 1, brightness 1
  rgbToHSB(1.0f, 0.0f, 0.0f, h, s, br);
  CHECK(h == 0.0f);
  CHECK(s == 1.0f);
  CHECK(br == 1.0f);

  // yellow -> hue 1/6, saturation 1, brightness 1
  rgbToHSB(1.0f, 1.0f, 0.0f, h, s, br);
  CHECK(h == Catch::Approx(1.0f / 6.0f));
  CHECK(s == Catch::Approx(1.0f));
  CHECK(br == Catch::Approx(1.0f));

  // gray -> hue 0, saturation 0, brightness 0.5
  rgbToHSB(0.5f, 0.5f, 0.5f, h, s, br);
  CHECK(h == 0.0f);
  CHECK(s == 0.0f);
  CHECK(br == Catch::Approx(0.5f));
}

} // namespace tb
