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

#include "vm/approx.h"
#include "vm/forward.h"
#include "vm/vec_ext.h"
#include "vm/vec_io.h"

#include <array>
#include <vector>

#include <catch2/catch.hpp>

namespace vm
{
// ========== operations on ranges of vectors ==========

TEST_CASE("vec_ext.operator_plus_vector")
{
  using Catch::Matchers::Equals;

  const auto in = std::vector<vec3f>{vec3f(1, 2, 3), vec3f(2, 3, 4)};
  const auto exp = std::vector<vec3f>{vec3f(0, 3, 1), vec3f(1, 4, 2)};
  CHECK_THAT(in + vec3f(-1, +1, -2), Equals(exp));
  CHECK_THAT(vec3f(-1, +1, -2) + in, Equals(exp));
}

TEST_CASE("vec_ext.operator_plus_array")
{
  constexpr auto in = std::array<vec3f, 2>{vec3f(1, 2, 3), vec3f(2, 3, 4)};
  constexpr auto exp = std::array<vec3f, 2>{vec3f(0, 3, 1), vec3f(1, 4, 2)};
  CHECK(in + vec3f(-1, +1, -2) == exp);
  CHECK(vec3f(-1, +1, -2) + in == exp);
}

TEST_CASE("vec_ext.operator_multiply_vector")
{
  using Catch::Matchers::Equals;

  const auto in = std::vector<vec3f>{vec3f(1, 2, 3), vec3f(2, 3, 4)};
  const auto exp = std::vector<vec3f>{vec3f(3, 6, 9), vec3f(6, 9, 12)};
  CHECK_THAT(in * 3.0f, Equals(exp));
  CHECK_THAT(3.0f * in, Equals(exp));
}

TEST_CASE("vec_ext.operator_multiply_array")
{
  constexpr auto in = std::array<vec3f, 2>{vec3f(1, 2, 3), vec3f(2, 3, 4)};
  constexpr auto exp = std::array<vec3f, 2>{vec3f(3, 6, 9), vec3f(6, 9, 12)};
  CHECK(in * 3.0f == exp);
  CHECK(3.0f * in == exp);
}
} // namespace vm
