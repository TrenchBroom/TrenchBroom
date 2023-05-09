/*
 Copyright 2010-2019 Kristian Duske
 Copyright 2015-2019 Eric Wasylishen

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

#include "test_utils.h"

#include <vecmath/forward.h>
#include <vecmath/vec_io.h>

#include <sstream>

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("vec_io.parse_valid_string")
{
  constexpr auto s = "1.0 3 3.5";

  const auto result = parse<float, 3>(s);
  CHECK(result.has_value());
  CHECK(*result == vec3f(1.0f, 3.0f, 3.5f));
}

TEST_CASE("vec_io.parse_short_string")
{
  constexpr auto s = "1.0 3";

  const auto result = parse<float, 3>(s);
  CHECK_FALSE(result.has_value());
}

TEST_CASE("vec_io.parse_long_string")
{
  constexpr auto s = "1.0 3 4 5";

  const auto result = parse<float, 3>(s);
  CHECK(result.has_value());
  CHECK(*result == vec3f(1.0f, 3.0f, 4.0f));
}

TEST_CASE("vec_io.parse_invalid_string")
{
  constexpr auto s = "asdf";

  const auto result = parse<float, 3>(s);
  CHECK_FALSE(result.has_value());
}

TEST_CASE("vec_io.parse_empty_string")
{
  constexpr auto s = "";

  const auto result = parse<float, 3>(s);
  CHECK_FALSE(result.has_value());
}

TEST_CASE("vec_io.parse_all")
{
  std::vector<vec3f> result;

  parse_all<float, 3>("", std::back_inserter(result));
  CHECK_THAT(result, Catch::Equals(std::vector<vec3f>{}));

  result.clear();
  parse_all<float, 3>("1.0 3 3.5 2.0 2.0 2.0", std::back_inserter(result));
  CHECK_THAT(
    result,
    Catch::Equals(std::vector<vec3f>{
      vec3f(1, 3, 3.5),
      vec3f(2, 2, 2),
    }));

  result.clear();
  parse_all<float, 3>("(1.0 3 3.5) (2.0 2.0 2.0)", std::back_inserter(result));
  CHECK_THAT(
    result,
    Catch::Equals(std::vector<vec3f>{
      vec3f(1, 3, 3.5),
      vec3f(2, 2, 2),
    }));

  result.clear();
  parse_all<float, 3>("(1.0 3 3.5), (2.0 2.0 2.0)", std::back_inserter(result));
  CHECK_THAT(
    result,
    Catch::Equals(std::vector<vec3f>{
      vec3f(1, 3, 3.5),
      vec3f(2, 2, 2),
    }));

  result.clear();
  parse_all<float, 3>("(1.0 3 3.5); (2.0 2.0 2.0)", std::back_inserter(result));
  CHECK_THAT(
    result,
    Catch::Equals(std::vector<vec3f>{
      vec3f(1, 3, 3.5),
      vec3f(2, 2, 2),
    }));

  result.clear();
  parse_all<float, 3>("1.0 3 3.5, 2.0 2.0 2.0", std::back_inserter(result));
  CHECK_THAT(
    result,
    Catch::Equals(std::vector<vec3f>{
      vec3f(1, 3, 3.5),
      vec3f(2, 2, 2),
    }));
}

TEST_CASE("vec_io.stream_insertion")
{
  std::stringstream str;
  str << vec3d(10, 10, 10);
  CHECK(str.str() == "10 10 10");
}
} // namespace vm
