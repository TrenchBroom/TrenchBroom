/*
 Copyright 2020 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#include <vecmath/forward.h>
#include <vecmath/mat_io.h>

#include <sstream>

#include "test_utils.h"

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("mat_io.parse_valid_string_square")
{
  SECTION("Parse 2x2 matrix")
  {
    constexpr auto s = "1.0 2 3 4.5";

    const auto result = parse<float, 2, 2>(s);
    CHECK(result.has_value());
    CHECK(*result == mat2x2f{1.0f, 2.0f, 3.0f, 4.5f});
  }

  SECTION("Parse 4x4 matrix")
  {
    constexpr auto s = "1 0 0 2 0 1 0 0 0 0 1 0 0 0 0 1";

    const auto result = parse<float, 4, 4>(s);
    CHECK(result.has_value());
    CHECK(
      *result
      == mat4x4f{
        1.0f,
        0.0f,
        0.0f,
        2.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f});
  }
}

TEST_CASE("mat_io.parse_valid_string_non_square")
{
  constexpr auto s = "1.0 2 3 4.5 5 6";

  SECTION("Parse 2x3 matrix")
  {
    const auto result = parse<float, 2, 3>(s);
    CHECK(result.has_value());
    CHECK(*result == mat<float, 2, 3>{1.0f, 2.0f, 3.0f, 4.5f, 5.0f, 6.0f});
  }

  SECTION("Parse 3x2 matrix")
  {
    const auto result = parse<float, 3, 2>(s);
    CHECK(result.has_value());
    CHECK(*result == mat<float, 3, 2>{1.0f, 2.0f, 3.0f, 4.5f, 5.0f, 6.0f});
  }
}

TEST_CASE("mat_io.parse_short_string")
{
  constexpr auto s = "1.0 2 3";

  const auto result = parse<float, 2, 2>(s);
  CHECK_FALSE(result.has_value());
}

TEST_CASE("mat_io.parse_long_string")
{
  constexpr auto s = "1.0 2 3 4.5 5";

  const auto result = parse<float, 2, 2>(s);
  CHECK(result.has_value());
  CHECK(*result == mat2x2f{1.0f, 2.0f, 3.0f, 4.5f});
}

TEST_CASE("mat_io.parse_invalid_string")
{
  constexpr auto s = "asdf";

  const auto result = parse<float, 2, 2>(s);
  CHECK_FALSE(result.has_value());
}

TEST_CASE("mat_io.parse_empty_string")
{
  constexpr auto s = "";

  const auto result = parse<float, 2, 2>(s);
  CHECK_FALSE(result.has_value());
}

TEST_CASE("mat_io.stream_insertion")
{
  std::stringstream str;
  str << mat3x3d{1, 2, 3, 4, 5, 6, 7, 8, 9};
  CHECK(str.str() == "1 2 3 4 5 6 7 8 9");
}
} // namespace vm
