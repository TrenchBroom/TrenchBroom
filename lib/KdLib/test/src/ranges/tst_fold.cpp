/*
 Copyright (C) 2026 Kristian Duske

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

#include "kd/ranges/fold.h"

#include <optional>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{

TEST_CASE("fold")
{
  using namespace std::string_literals;

  const auto concat = [](const auto& lhs, const auto& rhs) { return lhs + rhs; };
  const auto empty = std::vector<std::string>{};
  const auto one = std::vector<std::string>{"a"};
  const auto three = std::vector<std::string>{"a", "b", "c"};

  SECTION("fold_left")
  {
    CHECK(fold_left(empty, "x"s, concat) == "x");
    CHECK(fold_left(three, "x"s, concat) == "xabc");
    CHECK(fold_left(three.begin(), three.end(), "x"s, concat) == "xabc");

    // the result has the type returned by the operation, not the type of init
    const auto sum =
      fold_left(std::vector<int>{1, 2}, 0, [](const double lhs, const int rhs) {
        return lhs + rhs + 0.5;
      });
    static_assert(std::is_same_v<decltype(sum), const double>);
    CHECK(sum == 4.0);
  }

  SECTION("fold_left_with_iter")
  {
    const auto [in, value] = fold_left_with_iter(three, "x"s, concat);
    CHECK(in == three.end());
    CHECK(value == "xabc");

    static_assert(
      std::is_same_v<
        decltype(fold_left_with_iter(std::vector<std::string>{}, "x"s, concat).in),
        std::ranges::dangling>);
  }

  SECTION("fold_left_first")
  {
    CHECK(fold_left_first(empty, concat) == std::nullopt);
    CHECK(fold_left_first(one, concat) == "a");
    CHECK(fold_left_first(three, concat) == "abc");
    CHECK(fold_left_first(three.begin(), three.end(), concat) == "abc");
  }

  SECTION("fold_left_first_with_iter")
  {
    const auto [emptyIn, emptyValue] = fold_left_first_with_iter(empty, concat);
    CHECK(emptyIn == empty.end());
    CHECK(emptyValue == std::nullopt);

    const auto [in, value] = fold_left_first_with_iter(three, concat);
    CHECK(in == three.end());
    CHECK(value == "abc");
  }

  SECTION("fold_right")
  {
    // the operation is called with the element first and the accumulator second
    CHECK(fold_right(empty, "x"s, concat) == "x");
    CHECK(fold_right(three, "x"s, concat) == "abcx");
    CHECK(fold_right(three.begin(), three.end(), "x"s, concat) == "abcx");
  }

  SECTION("fold_right_last")
  {
    CHECK(fold_right_last(empty, concat) == std::nullopt);
    CHECK(fold_right_last(one, concat) == "a");
    CHECK(fold_right_last(three, concat) == "abc");
    CHECK(
      fold_right_last(
        three, [](const auto& elem, const auto& accum) { return accum + elem; })
      == "cba");
  }
}

} // namespace kdl
