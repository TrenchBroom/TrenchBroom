/*
 Copyright 2026 Kristian Duske

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

#include "kd/unpack.h"

#include <array>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace kdl
{
namespace
{

struct move_only
{
  move_only() = default;
  move_only(const move_only&) = delete;
  move_only(move_only&&) = default;
};

} // namespace

TEST_CASE("unpack")
{
  using Catch::Matchers::RangeEquals;

  SECTION("calls the wrapped function with the tuple's elements")
  {
    const auto add = unpack([](int a, int b) { return a + b; });

    CHECK(add(std::tuple{1, 2}) == 3);
    CHECK(add(std::pair{1, 2}) == 3);
    CHECK(add(std::array{1, 2}) == 3);
  }

  SECTION("supports tuples with more than two elements")
  {
    const auto sum3 =
      unpack([](const auto& a, const auto& b, const auto& c) { return a + b + c; });

    CHECK(sum3(std::tuple{1, 2, 3}) == 6);
  }

  SECTION("is constexpr")
  {
    constexpr auto add = unpack([](int a, int b) { return a + b; });
    static_assert(add(std::tuple{1, 2}) == 3);
  }

  SECTION("can be called repeatedly")
  {
    const auto add = unpack([](int a, int b) { return a + b; });

    CHECK(add(std::tuple{1, 2}) == 3);
    CHECK(add(std::tuple{3, 4}) == 7);
  }

  SECTION("forwards the wrapped function's return value, including references")
  {
    auto value = 1;
    const auto identity = unpack([](int& x) -> int& { return x; });

    CHECK(&identity(std::tie(value)) == &value);
  }

  SECTION("forwards the value category and constness of each tuple element")
  {
    const auto classify = unpack([](auto&& x) {
      using X = decltype(x);
      if constexpr (std::is_lvalue_reference_v<X>)
      {
        return std::is_const_v<std::remove_reference_t<X>> ? std::string{"const lvalue"}
                                                           : std::string{"lvalue"};
      }
      else
      {
        return std::is_const_v<std::remove_reference_t<X>> ? std::string{"const rvalue"}
                                                           : std::string{"rvalue"};
      }
    });

    auto mutableTuple = std::tuple<int>{1};
    const auto constTuple = std::tuple<int>{2};

    CHECK(classify(mutableTuple) == "lvalue");
    CHECK(classify(constTuple) == "const lvalue");
    CHECK(classify(std::tuple<int>{3}) == "rvalue");

    // NOLINTNEXTLINE(bugprone-use-after-move)
    CHECK(classify(std::move(mutableTuple)) == "rvalue");

    // moving a const object still binds to a const rvalue reference, which is exactly
    // the category this asserts, so std::move here is intentional, not a mistake
    // NOLINTNEXTLINE(bugprone-use-after-move,performance-move-const-arg)
    CHECK(classify(std::move(constTuple)) == "const rvalue");
  }

  SECTION("moves elements out of an rvalue tuple")
  {
    const auto release = unpack([](move_only&& m) { return std::move(m); });

    auto t = std::tuple<move_only>{};
    auto result = release(std::move(t));

    static_assert(std::is_same_v<decltype(result), move_only>);
  }

  SECTION("does not move elements out of an lvalue tuple")
  {
    const auto observe = unpack([](const move_only&) {});

    auto t = std::tuple<move_only>{};
    observe(t);

    // t must still be valid since it was passed to the wrapped function by reference,
    // not moved from
    std::apply([](const move_only&) {}, t);
  }

  SECTION("works as a projection for std::views::transform")
  {
    const auto pairs = std::vector<std::pair<std::string, int>>{
      {"a", 1},
      {"b", 2},
      {"c", 3},
    };

    const auto values = pairs
                        | std::views::transform(
                          unpack([](const auto&, const auto& value) { return value; }));

    CHECK_THAT(values, RangeEquals(std::vector<int>{1, 2, 3}));
  }
}

} // namespace kdl
