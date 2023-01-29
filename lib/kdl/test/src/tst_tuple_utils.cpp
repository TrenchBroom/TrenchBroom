/*
 Copyright 2021 Kristian Duske

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

#include <kdl/tuple_utils.h>

#include <catch2/catch_all.hpp>

namespace kdl
{
struct move_only
{
  move_only() = default;
  move_only(const move_only&) = delete;
  move_only(move_only&&) = default;
};

TEST_CASE("tup_capture")
{
  SECTION("Capturing single objects")
  {
    auto str = std::string{};
    const auto cstr = std::string{};

    static_assert(
      std::is_same_v<decltype(tup_capture(std::string{})), std::tuple<std::string>>,
      "rvalues must be captured by value");
    static_assert(
      std::is_same_v<decltype(tup_capture(str)), std::tuple<std::string&>>,
      "lvalue references must be captured by by lvalue reference");
    static_assert(
      std::is_same_v<decltype(tup_capture(cstr)), std::tuple<const std::string&>>,
      "const lvalue references must be captured by const lvalue reference");

    tup_capture(move_only{}); // rvalues are moved
  }

  SECTION("Capturing primitive types")
  {
    int i = 1;
    const size_t y = 2;

    static_assert(
      std::is_same_v<decltype(tup_capture(i)), std::tuple<int&>>,
      "rvalues must be captured by value");
    static_assert(
      std::is_same_v<decltype(tup_capture(y)), std::tuple<const size_t&>>,
      "rvalues must be captured by value");
  }

  SECTION("Capturing multiple values")
  {
    int i = 1;
    const auto cstr = std::string{};

    static_assert(
      std::is_same_v<
        decltype(tup_capture(std::string{}, i, cstr)),
        std::tuple<std::string, int&, const std::string&>>,
      "can capture multiple arguments");
  }
}
} // namespace kdl
