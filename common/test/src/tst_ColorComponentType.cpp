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

#include "ColorComponentType.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb
{

TEST_CASE("ColorComponentType")
{
  SECTION("float [0,1]")
  {
    using T = ColorComponentType<ColorChannel::r, float, 0.0f, 1.0f>;
    STATIC_CHECK(std::is_same_v<T::ValueType, float>);
    STATIC_CHECK(std::is_same_v<T::NormalizedValueType, double>);
    STATIC_CHECK(T::min == 0.0f);
    STATIC_CHECK(T::max == 1.0f);
    STATIC_CHECK(T::defaultValue() == 0.0f);
    STATIC_CHECK(T::normalizeValue(0.0f) == 0.0);
    STATIC_CHECK(T::normalizeValue(1.0f) == 1.0);
    STATIC_CHECK(T::fromNormalizedValue(0.0) == 0.0f);
    STATIC_CHECK(T::fromNormalizedValue(1.0) == 1.0f);

    STATIC_CHECK(T::inValueRange(0.0f));
    STATIC_CHECK(T::inValueRange(1.0f));
    STATIC_CHECK(!T::inValueRange(-1.0f));
    STATIC_CHECK(!T::inValueRange(2.0f));

    // not constexpr because from_chars is only constexpr in C++23
    CHECK(T::parse("0.0") == 0.0f);
    CHECK(T::parse("1") == 1.0f);
    CHECK(T::parse("") == std::nullopt);
    CHECK(T::parse("asdf") == std::nullopt);
  }

  SECTION("float [-1,1]")
  {
    using T = ColorComponentType<ColorChannel::r, float, -1.0f, 1.0f, 0.0f>;
    STATIC_CHECK(std::is_same_v<T::ValueType, float>);
    STATIC_CHECK(std::is_same_v<T::NormalizedValueType, double>);
    STATIC_CHECK(T::min == -1.0f);
    STATIC_CHECK(T::max == 1.0f);
    STATIC_CHECK(T::defaultValue() == 0.0f);
    STATIC_CHECK(T::normalizeValue(-1.0f) == 0.0);
    STATIC_CHECK(T::normalizeValue(0.0f) == 0.5);
    STATIC_CHECK(T::normalizeValue(1.0f) == 1.0);
    STATIC_CHECK(T::fromNormalizedValue(0.0) == -1.0f);
    STATIC_CHECK(T::fromNormalizedValue(0.5) == 0.0f);
    STATIC_CHECK(T::fromNormalizedValue(1.0) == 1.0f);

    STATIC_CHECK(T::inValueRange(-1.0f));
    STATIC_CHECK(T::inValueRange(0.0f));
    STATIC_CHECK(T::inValueRange(1.0f));
    STATIC_CHECK(!T::inValueRange(2.0f));

    // not constexpr because from_chars is only constexpr in C++23
    CHECK(T::parse("-1.0") == -1.0f);
    CHECK(T::parse("0.0") == 0.0f);
    CHECK(T::parse("1") == 1.0f);
    CHECK(T::parse("") == std::nullopt);
    CHECK(T::parse("asdf") == std::nullopt);
  }

  SECTION("byte [0,255] with default value 10")
  {
    using T = ColorComponentType<ColorChannel::r, uint8_t, 0, 255, 10>;
    STATIC_CHECK(std::is_same_v<T::ValueType, uint8_t>);
    STATIC_CHECK(std::is_same_v<T::NormalizedValueType, double>);
    STATIC_CHECK(T::min == 0);
    STATIC_CHECK(T::max == 255);
    STATIC_CHECK(T::defaultValue() == 10);
    STATIC_CHECK(T::normalizeValue(0) == 0.0);
    STATIC_CHECK(T::normalizeValue(255) == 1.0);
    STATIC_CHECK(T::fromNormalizedValue(0.0) == 0);
    STATIC_CHECK(T::fromNormalizedValue(1.0) == 255);

    STATIC_CHECK(!T::inValueRange(-1.0f));
    STATIC_CHECK(T::inValueRange(0.0f));
    STATIC_CHECK(T::inValueRange(1.0f));
    STATIC_CHECK(T::inValueRange(255.0f));
    STATIC_CHECK(!T::inValueRange(255.1f));

    // not constexpr because from_chars is only constexpr in C++23
    CHECK(T::parse("0") == 0);
    CHECK(T::parse("1") == 1);
    CHECK(T::parse("2") == 2);
    CHECK(T::parse("") == std::nullopt);
    CHECK(T::parse("asdf") == std::nullopt);
  }
}

} // namespace tb
