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

#include "ColorT.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb
{

TEST_CASE("detail::ComponentValue")
{
  using T = ColorComponentType<ColorChannel::r, float, -1.0f, 1.0f, 0.0f>;
  using V = detail::ComponentValue<T>;

  STATIC_CHECK(std::is_same_v<V::Type, T>);

  STATIC_CHECK(V{}.value == 0.0f);
  STATIC_CHECK(V{0.5f}.value == 0.5f);
  STATIC_CHECK(V::fromNormalizedValue(0.5) == V{0.0f});
  CHECK(V::parse("0.5") == V{0.5f});
  CHECK(V::parse("") == std::nullopt);
  STATIC_CHECK(V{-1.0f}.normalize() == 0.0f);
  STATIC_CHECK(V{0.0f}.normalize() == 0.5f);
  STATIC_CHECK(V{1.0f}.normalize() == 1.0f);
}

TEST_CASE("detail::componentIndex")
{
  using R = ColorComponentType<ColorChannel::r, float, 0.0f, 1.0f, 0.0f>;
  using G = ColorComponentType<ColorChannel::g, float, 0.0f, 1.0f, 0.0f>;
  using B = ColorComponentType<ColorChannel::b, float, 0.0f, 1.0f, 0.0f>;

  STATIC_CHECK(detail::componentIndex<ColorChannel::r, R, G, B>() == 0);
  STATIC_CHECK(detail::componentIndex<ColorChannel::g, R, G, B>() == 1);
  STATIC_CHECK(detail::componentIndex<ColorChannel::b, R, G, B>() == 2);
}

TEST_CASE("detail::normalizedValues")
{
  using F = ColorComponentType<ColorChannel::r, float, -1.0f, 1.0f, 0.0f>;
  using B = ColorComponentType<ColorChannel::r, uint8_t, 0, 255>;
  using VF = detail::ComponentValue<F>;
  using VB = detail::ComponentValue<B>;

  STATIC_CHECK(detail::normalizedValues(std::tuple{}) == std::tuple{});
  STATIC_CHECK(
    detail::normalizedValues(std::tuple{VF{0.0f}, VB{0}}) == std::tuple{0.5, 0.0});
  STATIC_CHECK(
    detail::normalizedValues(std::tuple{VF{1.0f}, VB{0}}) == std::tuple{1.0, 0.0});
}

TEST_CASE("detail::componentVector")
{
  using R = ColorComponentType<ColorChannel::r, float, 0.0f, 1.0f, 0.0f>;
  using G = ColorComponentType<ColorChannel::g, float, 0.0f, 1.0f, 0.0f>;
  using B = ColorComponentType<ColorChannel::b, float, 0.0f, 1.0f, 0.0f>;

  STATIC_CHECK(
    componentVector(std::tuple{
      detail::ComponentValue<R>{0.1f},
      detail::ComponentValue<G>{0.2f},
      detail::ComponentValue<B>{0.3f}})
    == vm::vec3f{0.1f, 0.2f, 0.3f});
}

TEST_CASE("detail::fromValues")
{
  using F = ColorComponentType<ColorChannel::r, float, -1.0f, 1.0f, 0.0f>;
  using B = ColorComponentType<ColorChannel::r, uint8_t, 0, 255>;
  using VF = detail::ComponentValue<F>;
  using VB = detail::ComponentValue<B>;

  STATIC_CHECK(detail::fromValues<>(std::tuple{}) == std::tuple{});
  STATIC_CHECK(
    detail::fromValues<F, B>(std::tuple{0.0, 0.0}) == std::tuple{VF{0.0f}, VB{0}});
  STATIC_CHECK(
    detail::fromValues<F, B>(std::tuple{0.5, 0.0}) == std::tuple{VF{0.5f}, VB{0}});
  STATIC_CHECK(
    detail::fromValues<F, B>(std::tuple{1.0, 1.0}) == std::tuple{VF{1.0f}, VB{1}});
  STATIC_CHECK(detail::fromValues<F, B>(std::tuple{2.0, 1.0}) == std::nullopt);
  STATIC_CHECK(detail::fromValues<F, B>(std::tuple{1.0, -1.0}) == std::nullopt);
}

TEST_CASE("detail::fromNormalizedValues")
{
  using F = ColorComponentType<ColorChannel::r, float, -1.0f, 1.0f, 0.0f>;
  using B = ColorComponentType<ColorChannel::r, uint8_t, 0, 255>;
  using VF = detail::ComponentValue<F>;
  using VB = detail::ComponentValue<B>;

  STATIC_CHECK(detail::fromNormalizedValues<>(std::tuple{}) == std::tuple{});
  STATIC_CHECK(
    detail::fromNormalizedValues<F, B>(std::tuple{0.0, 0.0})
    == std::tuple{VF{-1.0f}, VB{0}});
  STATIC_CHECK(
    detail::fromNormalizedValues<F, B>(std::tuple{0.5, 0.0})
    == std::tuple{VF{0.0f}, VB{0}});
  STATIC_CHECK(
    detail::fromNormalizedValues<F, B>(std::tuple{1.0, 1.0})
    == std::tuple{VF{1.0f}, VB{255}});
}

TEST_CASE("detail::defaultValues")
{
  using F = ColorComponentType<ColorChannel::r, float, -1.0f, 1.0f, 0.0f>;
  using B = ColorComponentType<ColorChannel::r, uint8_t, 0, 255>;
  using VF = detail::ComponentValue<F>;
  using VB = detail::ComponentValue<B>;

  STATIC_CHECK(detail::defaultValues<>() == std::tuple{});
  STATIC_CHECK(detail::defaultValues<F, B>() == std::tuple{VF{}, VB{}});
}

TEST_CASE("detail::parseComponentValues")
{
  using F = ColorComponentType<ColorChannel::r, float, -1.0f, 1.0f, 0.0f>;
  using B = ColorComponentType<ColorChannel::r, uint8_t, 0, 255>;
  using VF = detail::ComponentValue<F>;
  using VB = detail::ComponentValue<B>;

  CHECK(
    detail::parseComponentValues<F, B>(std::vector{"0.5", "25"})
    == std::tuple{VF{0.5f}, VB{25}});

  CHECK(
    detail::parseComponentValues<F, B>(std::vector{"0.5", "25", "77"}) == std::nullopt);

  CHECK(detail::parseComponentValues<F, B>(std::vector{"0.5"}) == std::nullopt);
  CHECK(detail::parseComponentValues<F, B>(std::vector{"asdf", "25"}) == std::nullopt);
  CHECK(detail::parseComponentValues<F, B>(std::vector{"0.5", ""}) == std::nullopt);
}

TEST_CASE("ColorT")
{
  using Rf = ColorComponentType<ColorChannel::r, float, 0.0f, 1.0f, 0.5f>;
  using Gf = ColorComponentType<ColorChannel::g, float, 0.0f, 1.0f, 0.5f>;
  using Bf = ColorComponentType<ColorChannel::b, float, 0.0f, 1.0f, 0.5f>;
  using Af = ColorComponentType<ColorChannel::a, float, 0.0f, 1.0f, 1.0f>;
  using Rb = ColorComponentType<ColorChannel::r, uint8_t, 0, 255>;
  using Gb = ColorComponentType<ColorChannel::g, uint8_t, 0, 255>;
  using Bb = ColorComponentType<ColorChannel::b, uint8_t, 0, 255>;

  using VRf = detail::ComponentValue<Rf>;
  using VGf = detail::ComponentValue<Gf>;
  using VBf = detail::ComponentValue<Bf>;
  using VRb = detail::ComponentValue<Rb>;
  using VGb = detail::ComponentValue<Gb>;
  using VBb = detail::ComponentValue<Bb>;

  using Cf = ColorT<Rf, Gf, Bf>;
  using Cb = ColorT<Rb, Gb, Bb>;

  SECTION("ColorT()")
  {
    STATIC_CHECK(Cf{}.components() == std::tuple{VRf{0.5f}, VGf{0.5f}, VBf{0.5f}});
    STATIC_CHECK(Cb{}.components() == std::tuple{VRb{0}, VGb{0}, VBb{0}});
  }

  SECTION("ColorT(x, y, z)")
  {
    STATIC_CHECK(Cf{0.1f, 0.2f, 0.3f}.values() == std::tuple{0.1f, 0.2f, 0.3f});
    STATIC_CHECK(Cb{1, 2, 3}.values() == std::tuple{1, 2, 3});
  }

  SECTION("ColorT(tuple)")
  {
    STATIC_CHECK(
      Cf{std::tuple{VRf{0.1f}, VGf{0.2f}, VBf{0.3f}}}.values()
      == std::tuple{0.1f, 0.2f, 0.3f});
    STATIC_CHECK(Cb{std::tuple{VRb{1}, VGb{2}, VBb{3}}}.values() == std::tuple{1, 2, 3});
  }

  SECTION("ColorT(ColorT, value)")
  {
    using Rgf = ColorT<Rf, Gf>;
    using Caf = ColorT<Rf, Gf, Bf, Af>;

    STATIC_CHECK(Caf{Cf{0.1f, 0.2f, 0.3f}, 0.4f} == Caf{0.1f, 0.2f, 0.3f, 0.4f});
    STATIC_CHECK(Caf{Rgf{0.1f, 0.2f}, 0.3f, 0.4f} == Caf{0.1f, 0.2f, 0.3f, 0.4f});
  }

  SECTION("fromVec")
  {
    CHECK(Cf::fromVec(vm::vec3f{0.1f, 0.2f, 0.3f}) == Cf{0.1f, 0.2f, 0.3f});
    CHECK(Cb::fromVec(vm::vec3f{1.0f, 2.0f, 3.0f}) == Cb{1, 2, 3});
  }

  SECTION("fromValues")
  {
    CHECK(Cf::fromValues(std::tuple{0.1f, 0.2f, 0.3f}) == Cf{0.1f, 0.2f, 0.3f});
    CHECK(
      Cf::fromValues(std::tuple{1.1f, 0.2f, 0.3f})
      == Error{"Failed to create color from values 1.1, 0.2, 0.3"});
  }

  SECTION("fromNormalizedValues")
  {
    STATIC_CHECK(
      Cb::fromNormalizedValues(std::tuple{0.0f, 0.5f, 1.0f}) == Cb{0, 127, 255});
  }

  SECTION("parseComponents")
  {
    CHECK(Cf::parseComponents(std::vector{"0.1", "0.2", "0.3"}) == Cf{0.1f, 0.2f, 0.3f});
    CHECK(
      Cf::parseComponents(std::vector{"0.1", "0.2", "0.3", "0.4"})
      == Cf{0.1f, 0.2f, 0.3f});
    CHECK(
      Cf::parseComponents(std::vector{"2.1", "0.2", "0.3"})
      == Error{"Failed to parse '2.1 0.2 0.3' as color"});
    CHECK(
      Cf::parseComponents(std::vector{"0.1", "0.2"})
      == Error{"Failed to parse '0.1 0.2' as color"});
  }

  SECTION("parse")
  {
    CHECK(Cf::parse("0.1 0.2 0.3") == Cf{0.1f, 0.2f, 0.3f});
    CHECK(Cf::parse("0.1 0.2 0.3 0.4") == Cf{0.1f, 0.2f, 0.3f});
    CHECK(Cf::parse("2.1 0.2 0.3") == Error{"Failed to parse '2.1 0.2 0.3' as color"});
    CHECK(Cf::parse("0.1 0.2") == Error{"Failed to parse '0.1 0.2' as color"});
  }

  SECTION("numComponents")
  {
    using Caf = ColorT<Rf, Gf, Bf, Af>;

    STATIC_CHECK(Cf{0.1f, 0.2f, 0.3f}.numComponents() == 3);
    STATIC_CHECK(Caf{0.1f, 0.2f, 0.3f, 0.4f}.numComponents() == 4);
  }

  SECTION("values")
  {
    STATIC_CHECK(Cf{0.1f, 0.2f, 0.3f}.values() == std::tuple{0.1f, 0.2f, 0.3f});
  }

  SECTION("get")
  {
    STATIC_CHECK(Cf{0.1f, 0.2f, 0.3f}.get<ColorChannel::r>() == 0.1f);
    STATIC_CHECK(Cf{0.1f, 0.2f, 0.3f}.get<ColorChannel::g>() == 0.2f);
    STATIC_CHECK(Cf{0.1f, 0.2f, 0.3f}.get<ColorChannel::b>() == 0.3f);
  }

  SECTION("toVec")
  {
    STATIC_CHECK(Cf{0.1f, 0.2f, 0.3f}.toVec() == vm::vec3f{0.1f, 0.2f, 0.3f});
    STATIC_CHECK(Cb{1, 2, 3}.toVec() == vm::vec<uint8_t, 3>{1, 2, 3});
  }

  SECTION("to")
  {
    using Caf = ColorT<
      ColorComponents::Rf,
      ColorComponents::Gf,
      ColorComponents::Bf,
      ColorComponents::Af>;
    using Cab = ColorT<
      ColorComponents::Rb,
      ColorComponents::Gb,
      ColorComponents::Bb,
      ColorComponents::Ab>;

    STATIC_CHECK(Cf{0.0f, 0.5f, 1.0f}.to<Cf>() == Cf{0.0f, 0.5f, 1.0f});
    STATIC_CHECK(Cf{0.0f, 0.5f, 1.0f}.to<Cb>() == Cb{0, 127, 255});
    STATIC_CHECK(Cf{0.0f, 0.5f, 1.0f}.to<Caf>() == Caf{0.0f, 0.5f, 1.0f, 1.0f});
    STATIC_CHECK(Cf{0.0f, 0.5f, 1.0f}.to<Cab>() == Cab{0, 127, 255, 255});

    STATIC_CHECK(Cb{0, 0, 255}.to<Cf>() == Cf{0.0f, 0.0f, 1.0f});
    STATIC_CHECK(Cb{0, 0, 255}.to<Cb>() == Cb{0, 0, 255});
    STATIC_CHECK(Cb{0, 0, 255}.to<Caf>() == Caf{0.0f, 0.0f, 1.0f, 1.0f});
    STATIC_CHECK(Cb{0, 0, 255}.to<Cab>() == Cab{0, 0, 255, 255});

    STATIC_CHECK(Caf{0.0f, 0.5f, 1.0f, 1.0f}.to<Cf>() == Cf{0.0f, 0.5f, 1.0f});
    STATIC_CHECK(Caf{0.0f, 0.5f, 1.0f, 1.0f}.to<Cb>() == Cb{0, 127, 255});
    STATIC_CHECK(Caf{0.0f, 0.5f, 1.0f, 1.0f}.to<Caf>() == Caf{0.0f, 0.5f, 1.0f, 1.0f});
    STATIC_CHECK(Caf{0.0f, 0.5f, 1.0f, 1.0f}.to<Cab>() == Cab{0, 127, 255, 255});

    STATIC_CHECK(Cab{0, 0, 255, 255}.to<Cf>() == Cf{0.0f, 0.0f, 1.0f});
    STATIC_CHECK(Cab{0, 0, 255, 255}.to<Cb>() == Cb{0, 0, 255});
    STATIC_CHECK(Cab{0, 0, 255, 255}.to<Caf>() == Caf{0.0f, 0.0f, 1.0f, 1.0f});
    STATIC_CHECK(Cab{0, 0, 255, 255}.to<Cab>() == Cab{0, 0, 255, 255});
  }

  SECTION("toString")
  {
    using A = ColorComponentType<ColorChannel::a, unsigned int, 0, 1000>;

    using Cai = ColorT<ColorComponents::Rf, ColorComponents::Gf, ColorComponents::Bf, A>;

    CHECK(Cf{0.1f, 0.2f, 0.3f}.toString() == "0.1 0.2 0.3");
    CHECK(Cb{1, 2, 3}.toString() == "1 2 3");
    CHECK(Cai{0.1f, 0.2f, 0.3f, 500}.toString() == "0.1 0.2 0.3 500");
  }
}

} // namespace tb
