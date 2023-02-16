/*
 Copyright 2023 Kristian Duske

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

#include "kdl/functional.h"

#include <functional>

#include <catch2/catch.hpp>

namespace kdl
{
TEST_CASE("lift_and")
{
  const auto f1 = [](int a, int b, int) { return a == b; };
  const auto f2 = [](int, int b, int c) { return b == c; };

  SECTION("Lambdas")
  {
    const auto f1_and_f2 = lift_and(f1, f2);

    CHECK(f1_and_f2(1, 1, 1));
    CHECK_FALSE(f1_and_f2(1, 1, 2));
    CHECK_FALSE(f1_and_f2(1, 2, 2));
    CHECK_FALSE(f1_and_f2(1, 2, 3));
  }

  SECTION("std::function")
  {
    const auto f1_and_f2 = lift_and(std::function{f1}, std::function{f2});

    CHECK(f1_and_f2(1, 1, 1));
    CHECK_FALSE(f1_and_f2(1, 1, 2));
    CHECK_FALSE(f1_and_f2(1, 2, 2));
    CHECK_FALSE(f1_and_f2(1, 2, 3));
  }
}

TEST_CASE("lift_or")
{
  const auto f1 = [](int a, int b, int) { return a == b; };
  const auto f2 = [](int, int b, int c) { return b == c; };

  SECTION("Lambdas")
  {
    const auto f1_or_f2 = lift_or(f1, f2);

    CHECK(f1_or_f2(1, 1, 1));
    CHECK(f1_or_f2(1, 1, 2));
    CHECK(f1_or_f2(1, 2, 2));
    CHECK_FALSE(f1_or_f2(1, 2, 3));
  }

  SECTION("std::function")
  {
    const auto f1_or_f2 = lift_or(std::function{f1}, std::function{f2});

    CHECK(f1_or_f2(1, 1, 1));
    CHECK(f1_or_f2(1, 1, 2));
    CHECK(f1_or_f2(1, 2, 2));
    CHECK_FALSE(f1_or_f2(1, 2, 3));
  }
}
} // namespace kdl
