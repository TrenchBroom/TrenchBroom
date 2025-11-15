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

#include "ColorVariantT.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb
{

TEST_CASE("ColorVariantT")
{
  using Rf = ColorComponentType<ColorChannel::r, float, 0.0f, 1.0f, 0.5f>;
  using Gf = ColorComponentType<ColorChannel::g, float, 0.0f, 1.0f, 0.5f>;
  using Bf = ColorComponentType<ColorChannel::b, float, 0.0f, 1.0f, 0.5f>;
  using Af = ColorComponentType<ColorChannel::a, float, 0.0f, 1.0f, 1.0f>;
  using Rb = ColorComponentType<ColorChannel::r, uint8_t, 0, 255>;
  using Gb = ColorComponentType<ColorChannel::g, uint8_t, 0, 255>;
  using Bb = ColorComponentType<ColorChannel::b, uint8_t, 0, 255>;

  using Caf = ColorT<Rf, Gf, Bf, Af>;
  using Cf = ColorT<Rf, Gf, Bf>;
  using Cb = ColorT<Rb, Gb, Bb>;

  using CV = ColorVariantT<Caf, Cf, Cb>;

  SECTION("ColorVariantT(Color)") {}

  SECTION("operator=(Color)")
  {
    auto color = CV{Cf{0.1f, 0.2f, 0.3f}};
    color = Cb{1, 2, 3};

    CHECK(color == CV{Cb{1, 2, 3}});
  }

  SECTION("fromVec")
  {
    CHECK(CV::fromVec(vm::vec3f{0.1f, 0.2f, 0.3f}) == Cf{0.1f, 0.2f, 0.3f});
    CHECK(CV::fromVec(vm::vec4f{0.1f, 0.2f, 0.3f, 0.4f}) == Caf{0.1f, 0.2f, 0.3f, 0.4f});
    CHECK(CV::fromVec(vm::vec3f{1.0f, 2.0f, 3.0f}) == Cb{1, 2, 3});
  }

  SECTION("fromValues")
  {
    CHECK(CV::fromValues(0.0f, 0.0f, 0.0f) == Cf{0.0f, 0.0f, 0.0f});
    CHECK(CV::fromValues(0, 0, 0) == Cf{0.0f, 0.0f, 0.0f});
    CHECK(CV::fromValues(2.0f, 0.0f, 0.0f) == Cb{2, 0, 0});
    CHECK(
      CV::fromValues(-1.0f, 0.0f, 0.0f)
      == Error{"Failed to create color from values -1, 0, 0"});
  }

  SECTION("parseComponents")
  {
    CHECK(CV::parseComponents(std::vector{"1", "2", "3"}) == CV{Cb{1, 2, 3}});
    CHECK(CV::parseComponents(std::vector{"0", "0", "0"}) == CV{Cf{0.0f, 0.0f, 0.0f}});
    CHECK(
      CV::parseComponents(std::vector{"0", "0", "0", "0"})
      == CV{Caf{0.0f, 0.0f, 0.0f, 0.0f}});
    CHECK(
      CV::parseComponents(std::vector{"0", "0", "0", "0", "0"})
      == CV{Caf{0.0f, 0.0f, 0.0f, 0.0f}});
    CHECK(
      CV::parseComponents(std::vector{"0", "0"})
      == Error{"Failed to parse '0 0' as color"});
  }

  SECTION("parse")
  {
    CHECK(CV::parse("1 2 3") == CV{Cb{1, 2, 3}});
    CHECK(CV::parse("0 0 0") == CV{Cf{0.0f, 0.0f, 0.0f}});
    CHECK(CV::parse("0 0 0 0") == CV{Caf{0.0f, 0.0f, 0.0f, 0.0f}});
    CHECK(CV::parse("1 2 3 4") == CV{Cb{1, 2, 3}});
    CHECK(CV::parse("0 0") == Error{"Failed to parse '0 0' as color"});
  }

  SECTION("numComponents")
  {
    CHECK(CV{Cf{0.1f, 0.2f, 0.3f}}.numComponents() == 3);
    CHECK(CV{Caf{0.1f, 0.2f, 0.3f, 0.4f}}.numComponents() == 4);
  }

  SECTION("is")
  {
    STATIC_CHECK(CV{Cf{0.1f, 0.2f, 0.3f}}.is<Cf>());
    STATIC_CHECK(CV{Cb{0, 127, 255}}.is<Cb>());
  }

  SECTION("to")
  {
    STATIC_CHECK(CV{Cf{0.1f, 0.2f, 0.3f}}.to<Cf>() == Cf{0.1f, 0.2f, 0.3f});
    STATIC_CHECK(CV{Cf{0.0f, 0.5f, 1.0f}}.to<Cb>() == Cb{0, 127, 255});

    STATIC_CHECK(CV{Cb{0, 127, 255}}.to<Cb>() == Cb{0, 127, 255});
    STATIC_CHECK(CV{Cb{0, 0, 255}}.to<Cf>() == Cf{0.0f, 0.0f, 1.0f});
  }

  SECTION("toString")
  {
    CHECK(CV{Cf{0.1f, 0.2f, 0.3f}}.toString() == "0.1 0.2 0.3");
    CHECK(CV{Cb{0, 127, 255}}.toString() == "0 127 255");
  }
}

} // namespace tb
