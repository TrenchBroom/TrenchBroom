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

using v3f = vm::vec<float, 3>;
using v3b = vm::vec<uint8_t, 3>;
using v4f = vm::vec<float, 4>;
using v4b = vm::vec<uint8_t, 4>;

TEST_CASE("isFloatColorRange")
{
  CHECK(isFloatColorRange(v3b{0, 0, 0}));
  CHECK(isFloatColorRange(v3b{1, 1, 1}));
  CHECK(isFloatColorRange(v3f{0, 0, 0}));
  CHECK(isFloatColorRange(v3f{1, 1, 1}));
  CHECK(isFloatColorRange(v3f{0, 0.5f, 1}));
  CHECK(isFloatColorRange(v4f{0, 0.5f, 0.75f, 1.0f}));
  CHECK(!isFloatColorRange(v3b{1, 1, 2}));
  CHECK(!isFloatColorRange(v3f{-1, 0.5f, 1}));
  CHECK(!isFloatColorRange(v3f{0, 0.5f, 1.1f}));
  CHECK(!isFloatColorRange(v4f{0, 0.5f, 0.75f, 1.1f}));
}

TEST_CASE("isByteColorRange")
{
  CHECK(isByteColorRange(v3b{0, 0, 0}));
  CHECK(isByteColorRange(v3b{0, 127, 255}));
  CHECK(isByteColorRange(v4b{0, 127, 255, 255}));
  CHECK(!isByteColorRange(v3f{0.0f, 0.0f, 256.0f}));
  CHECK(!isByteColorRange(v3f{-1.0f, 0.0f, 0.0f}));
}

TEST_CASE("RgbF")
{
  SECTION("RgbF()")
  {
    CHECK(RgbF{} == RgbF{0.0f, 0.0f, 0.0f});
  }

  SECTION("RgbF(float, float, float)")
  {
    const auto rgbF = RgbF{0.1f, 0.2f, 0.3f};
    CHECK(rgbF.vec() == v3f{0.1f, 0.2f, 0.3f});
  }

  SECTION("RgbF(const v<float, 3>&)")
  {
    CHECK(RgbF{v3f{0.1f, 0.2f, 0.3f}} == RgbF{0.1f, 0.2f, 0.3f});
  }

  SECTION("parse")
  {
    CHECK(RgbF::parse("0.0 0.5 1.0") == RgbF{0.0f, 0.5f, 1.0f});
    CHECK(RgbF::parse("  0.0\t0.5   1.0  ") == RgbF{0.0f, 0.5f, 1.0f});
    CHECK(RgbF::parse("0 0.5 1") == RgbF{0.0f, 0.5f, 1.0f});

    CHECK(RgbF::parse("0.1 0.2").is_error());
    CHECK(RgbF::parse("a b c").is_error());
    CHECK(RgbF::parse("").is_error());
  }

  SECTION("operator vm::vec conversion")
  {
    vm::vec<float, 3> v = RgbF{0.1f, 0.2f, 0.3f};
    CHECK(v == v3f{0.1f, 0.2f, 0.3f});
  }

  SECTION("vec")
  {
    CHECK(RgbF{0.1f, 0.2f, 0.3f}.vec() == v3f{0.1f, 0.2f, 0.3f});
  }

  SECTION("toFloat")
  {
    CHECK(RgbF{0.1f, 0.2f, 0.3f}.toFloat() == RgbF{0.1f, 0.2f, 0.3f});
  }

  SECTION("toRgbF")
  {
    CHECK(RgbF{0.1f, 0.2f, 0.3f}.toRgbF() == RgbF{0.1f, 0.2f, 0.3f});
  }

  SECTION("toRgbB")
  {
    CHECK(RgbF{0.1f, 0.2f, 0.3f}.toRgbB() == RgbB{v3b{v3f{0.1f, 0.2f, 0.3f} * 255.0f}});
  }

  SECTION("toRgbaF")
  {
    CHECK(RgbF{0.1f, 0.2f, 0.3f}.toRgbaF() == RgbaF{0.1f, 0.2f, 0.3f, 1.0f});
  }

  SECTION("toRgbaB")
  {
    CHECK(
      RgbF{0.1f, 0.2f, 0.3f}.toRgbaB()
      == RgbaB{v4b{v3f{0.1f, 0.2f, 0.3f} * 255.0f, 255}});
  }

  SECTION("toString")
  {
    CHECK(RgbF{0.0f, 0.5f, 1.0f}.toString() == "0 0.5 1");
  }
}

TEST_CASE("RgbB")
{
  SECTION("RgbB()")
  {
    CHECK(RgbB{} == RgbB{0, 0, 0});
  }

  SECTION("RgbB(uint8_t, uint8_t, uint8_t)")
  {
    const auto rgbB = RgbB{0, 64, 255};
    CHECK(rgbB.vec() == v3b{0, 64, 255});
  }

  SECTION("RgbB(const v3c&)")
  {
    CHECK(RgbB{v3b{0, 64, 255}} == RgbB{0, 64, 255});
  }

  SECTION("parse")
  {
    CHECK(RgbB::parse("0 127 255") == RgbB{0, 127, 255});
    CHECK(RgbB::parse("  0\t127   255  ") == RgbB{0, 127, 255});

    CHECK(RgbB::parse("0 127").is_error());
    CHECK(RgbB::parse("a b c").is_error());
    CHECK(RgbB::parse("").is_error());
    CHECK(RgbB::parse("256 0 0").is_error());
    CHECK(RgbB::parse("-1 0 0").is_error());
  }

  SECTION("vec")
  {
    CHECK(RgbB{0, 64, 255}.vec() == v3b{0, 64, 255});
  }

  SECTION("toFloat")
  {
    CHECK(RgbB{0, 64, 128}.toFloat() == RgbF{v3f{0, 64, 128} / 255.0f});
  }

  SECTION("toRgbF")
  {
    CHECK(RgbB{0, 64, 128}.toRgbF() == RgbF{v3f{0, 64, 128} / 255.0f});
  }

  SECTION("toRgbB")
  {
    CHECK(RgbB{0, 64, 128}.toRgbB() == RgbB{0, 64, 128});
  }

  SECTION("toRgbaF")
  {
    CHECK(RgbB{0, 64, 128}.toRgbaF() == RgbaF{v4f{0, 64, 128, 255} / 255.0f});
  }

  SECTION("toRgbaB")
  {
    CHECK(RgbB{0, 64, 128}.toRgbaB() == RgbaB{0, 64, 128, 255});
  }

  SECTION("toString")
  {
    CHECK(RgbB{0, 127, 255}.toString() == "0 127 255");
  }

  SECTION("operator vm::vec conversion")
  {
    vm::vec<uint8_t, 3> vc = RgbB{1, 2, 3};
    CHECK(vc == v3b{1, 2, 3});
  }
}

TEST_CASE("RgbaF")
{
  SECTION("RgbaF()")
  {
    CHECK(RgbaF{} == RgbaF{0.0f, 0.0f, 0.0f, 0.0f});
  }

  SECTION("RgbaF(float, float, float, float)")
  {
    const auto rgbF = RgbaF{0.1f, 0.2f, 0.3f, 0.4f};
    CHECK(rgbF.vec() == v4f{0.1f, 0.2f, 0.3f, 0.4f});
  }

  SECTION("RgbaF(const v<float, 4>&)")
  {
    CHECK(RgbaF{v4f{0.1f, 0.2f, 0.3f, 0.4f}} == RgbaF{0.1f, 0.2f, 0.3f, 0.4f});
  }

  SECTION("RgbaF(const RgbF&, float)")
  {
    CHECK(RgbaF{RgbF{0.1f, 0.2f, 0.3f}, 0.4f} == RgbaF{0.1f, 0.2f, 0.3f, 0.4f});
  }

  SECTION("parse")
  {
    CHECK(RgbaF::parse("0.0 0.5 1.0 0.4") == RgbaF{0.0f, 0.5f, 1.0f, 0.4f});
    CHECK(RgbaF::parse("  0.0\t0.5   1.0  0.4 ") == RgbaF{0.0f, 0.5f, 1.0f, 0.4f});
    CHECK(RgbaF::parse("0 0.5 1 0") == RgbaF{0.0f, 0.5f, 1.0f, 0.0f});

    CHECK(RgbaF::parse("0.1 0.2 0.3").is_error());
    CHECK(RgbaF::parse("a b c d").is_error());
    CHECK(RgbaF::parse("").is_error());
  }

  SECTION("operator vm::vec conversion")
  {
    vm::vec<float, 4> v = RgbaF{0.1f, 0.2f, 0.3f, 0.4f};
    CHECK(v == v4f{0.1f, 0.2f, 0.3f, 0.4f});
  }

  SECTION("vec")
  {
    CHECK(RgbaF{0.1f, 0.2f, 0.3f, 0.4f}.vec() == v4f{0.1f, 0.2f, 0.3f, 0.4f});
  }

  SECTION("toFloat")
  {
    CHECK(RgbaF{0.1f, 0.2f, 0.3f, 0.4f}.toFloat() == RgbaF{0.1f, 0.2f, 0.3f, 0.4f});
  }

  SECTION("toRgbF")
  {
    CHECK(RgbaF{0.1f, 0.2f, 0.3f, 0.4f}.toRgbF() == RgbF{0.1f, 0.2f, 0.3f});
  }

  SECTION("toRgbB")
  {
    CHECK(
      RgbaF{0.1f, 0.2f, 0.3f, 0.4f}.toRgbB()
      == RgbB{v3b{v3f{0.1f, 0.2f, 0.3f} * 255.0f}});
  }

  SECTION("toRgbaF")
  {
    CHECK(RgbaF{0.1f, 0.2f, 0.3f, 0.4f}.toRgbaF() == RgbaF{0.1f, 0.2f, 0.3f, 0.4f});
  }

  SECTION("toRgbaB")
  {
    CHECK(
      RgbaF{0.1f, 0.2f, 0.3f, 0.4f}.toRgbaB()
      == RgbaB{v4b{v4f{0.1f, 0.2f, 0.3f, 0.4f} * 255.0f}});
  }

  SECTION("toString")
  {
    CHECK(RgbaF{0.0f, 0.5f, 1.0f, 0.4f}.toString() == "0 0.5 1 0.4");
  }
}

TEST_CASE("RgbaB")
{
  SECTION("RgbaB()")
  {
    CHECK(RgbaB{} == RgbaB{0, 0, 0, 0});
  }

  SECTION("RgbaB(uint8_t, uint8_t, uint8_t, uint8_t)")
  {
    const auto rgbaB = RgbaB{0, 32, 64, 127};
    CHECK(rgbaB.vec() == v4b{0, 32, 64, 127});
  }

  SECTION("RgbaB(const v<uint8_t, 4>&)")
  {
    CHECK(RgbaB{v4b{0, 32, 64, 127}} == RgbaB{0, 32, 64, 127});
  }

  SECTION("RgbaB(const RgbB, uint8_t)")
  {
    CHECK(RgbaB{RgbB{1, 2, 3}, 4} == RgbaB{1, 2, 3, 4});
  }

  SECTION("parse")
  {
    CHECK(RgbaB::parse("0 127 255 127") == RgbaB{0, 127, 255, 127});
    CHECK(RgbaB::parse("  0\t127   255  127 ") == RgbaB{0, 127, 255, 127});

    CHECK(RgbaB::parse("0 127 255").is_error());
    CHECK(RgbaB::parse("a b c d").is_error());
    CHECK(RgbaB::parse("").is_error());
    CHECK(RgbaB::parse("0 0 0 256").is_error());
    CHECK(RgbaB::parse("0 -1 0 0").is_error());
  }

  SECTION("vec")
  {
    CHECK(RgbaB{0, 32, 64, 127}.vec() == v4b{0, 32, 64, 127});
  }

  SECTION("toFloat")
  {
    CHECK(RgbaB{0, 64, 128, 196}.toFloat() == RgbaF{v4f{0, 64, 128, 196} / 255.0f});
  }

  SECTION("toRgbF")
  {
    CHECK(RgbaB{0, 64, 128, 196}.toRgbF() == RgbF{v3f{0, 64, 128} / 255.0f});
  }

  SECTION("toRgbB")
  {
    CHECK(RgbaB{0, 64, 128, 196}.toRgbB() == RgbB{0, 64, 128});
  }

  SECTION("operator vm::vec conversion")
  {
    vm::vec<uint8_t, 4> vc = RgbaB{1, 2, 3, 4};
    CHECK(vc == v4b{1, 2, 3, 4});
  }

  SECTION("toRgbaF")
  {
    CHECK(RgbaB{0, 64, 128, 196}.toRgbaF() == RgbaF{v4f{0, 64, 128, 196} / 255.0f});
  }

  SECTION("toRgbaB")
  {
    CHECK(RgbaB{0, 64, 128, 196}.toRgbaB() == RgbaB{0, 64, 128, 196});
  }

  SECTION("toString")
  {
    CHECK(RgbaB{0, 127, 255, 127}.toString() == "0 127 255 127");
  }
}

TEST_CASE("Color")
{
  SECTION("fromVec")
  {
    CHECK(Color::fromVec(v3f{0.0f, 0.5f, 1.0f}) == Color{RgbF{0.0f, 0.5f, 1.0f}});
    CHECK(Color::fromVec(v3b{0, 127, 255}) == Color{RgbB{0, 127, 255}});
    CHECK(
      Color::fromVec(v4f{0.0f, 0.5f, 1.0f, 0.4f})
      == Color{RgbaF{0.0f, 0.5f, 1.0f, 0.4f}});
    CHECK(Color::fromVec(v4b{0, 127, 255, 127}) == Color{RgbaB{0, 127, 255, 127}});
    CHECK(Color::fromVec(v3f{0.0f, 0.5f, 2.0f}).is_error());
  }

  SECTION("parse")
  {
    CHECK(Color::parse("0.0 0.5 1.0") == Color{RgbF{0.0f, 0.5f, 1.0f}});
    CHECK(Color::parse("0 127 255") == Color{RgbB{0, 127, 255}});
    CHECK(Color::parse("0.0 0.5 1.0 0.4") == Color{RgbaF{0.0f, 0.5f, 1.0f, 0.4f}});
    CHECK(Color::parse("0 127 255 127") == Color{RgbaB{0, 127, 255, 127}});

    CHECK(Color::parse("  0.0\t0.5   1.0  ") == Color{RgbF{0.0f, 0.5f, 1.0f}});
    CHECK(Color::parse("  0\t127   255  ") == Color{RgbB{0, 127, 255}});

    CHECK(Color::parse("").is_error());
    CHECK(Color::parse("a b c").is_error());
    CHECK(Color::parse("0.0 0.5").is_error());
  }

  SECTION("isFloat / isByte")
  {
    using T = std::tuple<Color, bool, bool>;

    const auto [color, expectedIsFloat, expectedIsByte] = GENERATE(values<T>({
      {RgbF{0.1f, 0.2f, 0.3f}, true, false},
      {RgbaF{0.1f, 0.2f, 0.3f, 0.4f}, true, false},
      {RgbB{1, 2, 3}, false, true},
      {RgbaB{1, 2, 3, 4}, false, true},
    }));

    CAPTURE(color);

    CHECK(color.isFloat() == expectedIsFloat);
    CHECK(color.isByte() == expectedIsByte);
  }

  SECTION("toFloat")
  {
    using T = std::tuple<Color, Color>;

    const auto [color, expectedFloat] = GENERATE(values<T>({
      {RgbF{0.1f, 0.2f, 0.3f}, RgbF{0.1f, 0.2f, 0.3f}},
      {RgbaF{0.1f, 0.2f, 0.3f, 0.4f}, RgbaF{0.1f, 0.2f, 0.3f, 0.4f}},
      {RgbB{1, 2, 3}, RgbF{v3f{1.0f, 2.0f, 3.0f} / 255.0f}},
      {RgbaB{1, 2, 3, 4}, RgbaF{v4f{1.0f, 2.0f, 3.0f, 4.0f} / 255.0f}},
    }));

    CAPTURE(color);

    CHECK(color.toFloat() == expectedFloat);
  }

  SECTION("toByte")
  {
    using T = std::tuple<Color, Color>;

    const auto [color, expectedByte] = GENERATE(values<T>({
      {RgbF{0.1f, 0.2f, 0.3f}, RgbB{v3b{v3f{0.1f, 0.2f, 0.3f} * 255.0f}}},
      {RgbaF{0.1f, 0.2f, 0.3f, 0.4f}, RgbaB{v4b{v4f{0.1f, 0.2f, 0.3f, 0.4f} * 255.0f}}},
      {RgbB{1, 2, 3}, RgbB{1, 2, 3}},
      {RgbaB{1, 2, 3, 4}, RgbaB{1, 2, 3, 4}},
    }));

    CAPTURE(color);

    CHECK(color.toByte() == expectedByte);
  }

  SECTION("toRgbF")
  {
    using T = std::tuple<Color, RgbF>;

    const auto [color, expectedRgbF] = GENERATE(values<T>({
      {RgbF{0.1f, 0.2f, 0.3f}, RgbF{0.1f, 0.2f, 0.3f}},
      {RgbaF{0.1f, 0.2f, 0.3f, 0.4f}, RgbF{0.1f, 0.2f, 0.3f}},
      {RgbB{1, 2, 3}, RgbF{v3f{1.0f, 2.0f, 3.0f} / 255.0f}},
      {RgbaB{1, 2, 3, 4}, RgbF{v3f{1.0f, 2.0f, 3.0f} / 255.0f}},
    }));

    CAPTURE(color);

    CHECK(color.toRgbF() == expectedRgbF);
  }

  SECTION("toRgbaF")
  {
    using T = std::tuple<Color, RgbaF>;

    const auto [color, expectedRgbaF] = GENERATE(values<T>({
      {RgbF{0.1f, 0.2f, 0.3f}, RgbaF{0.1f, 0.2f, 0.3f, 1.0f}},
      {RgbaF{0.1f, 0.2f, 0.3f, 0.4f}, RgbaF{0.1f, 0.2f, 0.3f, 0.4f}},
      {RgbB{1, 2, 3}, RgbaF{v4f{1.0f, 2.0f, 3.0f, 255.0f} / 255.0f}},
      {RgbaB{1, 2, 3, 4}, RgbaF{v4f{1.0f, 2.0f, 3.0f, 4.0f} / 255.0f}},
    }));

    CAPTURE(color);

    CHECK(color.toRgbaF() == expectedRgbaF);
  }

  SECTION("toRgbB")
  {
    using T = std::tuple<Color, RgbB>;

    const auto [color, expectedRgbB] = GENERATE(values<T>({
      {RgbF{0.1f, 0.2f, 0.3f}, RgbB{v3b{v3f{0.1f, 0.2f, 0.3f} * 255.0f}}},
      {RgbaF{0.1f, 0.2f, 0.3f, 0.4f}, RgbB{v3b{v3f{0.1f, 0.2f, 0.3f} * 255.0f}}},
      {RgbB{1, 2, 3}, RgbB{1, 2, 3}},
      {RgbaB{1, 2, 3, 4}, RgbB{1, 2, 3}},
    }));

    CAPTURE(color);

    CHECK(color.toRgbB() == expectedRgbB);
  }

  SECTION("toRgbaB")
  {
    using T = std::tuple<Color, RgbaB>;

    const auto [color, expectedRgbaB] = GENERATE(values<T>({
      {RgbF{0.1f, 0.2f, 0.3f}, RgbaB{v4b{v4f{0.1f, 0.2f, 0.3f, 1.0f} * 255.0f}}},
      {RgbaF{0.1f, 0.2f, 0.3f, 0.4f}, RgbaB{v4b{v4f{0.1f, 0.2f, 0.3f, 0.4f} * 255.0f}}},
      {RgbB{1, 2, 3}, RgbaB{1, 2, 3, 255}},
      {RgbaB{1, 2, 3, 4}, RgbaB{1, 2, 3, 4}},
    }));

    CAPTURE(color);

    CHECK(color.toRgbaB() == expectedRgbaB);
  }

  SECTION("toString")
  {
    using T = std::tuple<Color, std::string>;

    const auto [color, expectedString] = GENERATE(values<T>({
      {RgbF{0.1f, 0.2f, 0.3f}, "0.1 0.2 0.3"},
      {RgbaF{0.1f, 0.2f, 0.3f, 0.4f}, "0.1 0.2 0.3 0.4"},
      {RgbB{1, 2, 3}, "1 2 3"},
      {RgbaB{1, 2, 3, 4}, "1 2 3 4"},
    }));

    CAPTURE(color);

    CHECK(color.toString() == expectedString);
  }
}

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
  // RgbaF clamping
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
