/*
 Copyright 2024 Kristian Duske

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

#include "kd/optional_utils.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{

struct MoveOnly
{
  bool moved_from = false;

  MoveOnly() = default;

  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;

  MoveOnly(MoveOnly&& other) noexcept { other.moved_from = true; }
  MoveOnly& operator=(MoveOnly&& other) noexcept
  {
    if (this != &other)
    {
      other.moved_from = true;
    }
    return *this;
  }
};

std::optional<int> maybeTwice(const int x)
{
  return x * 2;
}

int twice(const int x)
{
  return x * 2;
}

} // namespace

TEST_CASE("optional_utils")
{
  SECTION("optional_and_then")
  {
    const auto f = [](int x) { return std::optional{x * 2}; };
    CHECK((std::optional<int>{42} | optional_and_then(f)) == 84);
    CHECK((std::optional<int>{} | optional_and_then(f)) == std::nullopt);

    SECTION("with a function")
    {
      CHECK((std::optional<int>{42} | optional_and_then(maybeTwice)) == 84);
    }

    SECTION("stores a copy of the function")
    {
      auto g = std::function<std::optional<int>(int)>{[](int x) { return x * 2; }};
      const auto helper = optional_and_then(g);
      g = [](int x) { return x * 3; };
      CHECK((std::optional<int>{42} | helper) == 84);
    }

    SECTION("with a mutable function")
    {
      const auto g = [n = 0](int x) mutable { return std::optional{x + ++n}; };
      const auto o = std::optional<int>{42};
      CHECK((o | optional_and_then(g)) == 43);
      CHECK((std::optional<int>{42} | optional_and_then(g)) == 43);
    }
  }

  SECTION("optional_or_else")
  {
    const auto f = [] { return std::optional{43}; };
    CHECK((std::optional<int>{42} | optional_or_else(f)) == 42);
    CHECK((std::optional<int>{} | optional_or_else(f)) == 43);

    SECTION("stores a copy of the function")
    {
      auto g = std::function<std::optional<int>()>{[] { return 43; }};
      const auto helper = optional_or_else(g);
      g = [] { return 44; };
      CHECK((std::optional<int>{} | helper) == 43);
    }

    SECTION("with a mutable function")
    {
      const auto g = [n = 43]() mutable { return std::optional{n++}; };
      const auto o = std::optional<int>{};
      CHECK((o | optional_or_else(g)) == 43);
      CHECK((std::optional<int>{} | optional_or_else(g)) == 43);
    }
  }

  SECTION("optional_transform")
  {
    const auto f = [](int x) { return x * 2; };
    CHECK((std::optional<int>{42} | optional_transform(f)) == 84);
    CHECK((std::optional<int>{} | optional_transform(f)) == std::nullopt);

    SECTION("with a function")
    {
      CHECK((std::optional<int>{42} | optional_transform(twice)) == 84);
    }

    SECTION("stores a copy of the function")
    {
      auto g = std::function<int(int)>{[](int x) { return x * 2; }};
      const auto helper = optional_transform(g);
      g = [](int x) { return x * 3; };
      CHECK((std::optional<int>{42} | helper) == 84);
    }

    SECTION("with a mutable function")
    {
      const auto g = [n = 0](int x) mutable { return x + ++n; };
      const auto o = std::optional<int>{42};
      CHECK((o | optional_transform(g)) == 43);
      CHECK((std::optional<int>{42} | optional_transform(g)) == 43);
    }
  }

  SECTION("optional_value_or")
  {
    SECTION("rvalue fallback (copyable)")
    {
      CHECK((std::optional<int>{42} | optional_value_or(43)) == 42);
      CHECK((std::optional<int>{} | optional_value_or(43)) == 43);
    }

    SECTION("rvalue fallback (move-only)")
    {
      CHECK(
        *(std::optional<std::unique_ptr<int>>{}
          | optional_value_or(std::make_unique<int>(43)))
        == 43);
    }

    SECTION("lvalue fallback (copyable)")
    {
      // long enough to defeat the small string optimization
      const auto expected = std::string{"a fallback value that does not fit into SSO"};
      auto fallback = expected;
      CHECK(
        (std::optional<std::string>{"value"} | optional_value_or(fallback)) == "value");
      CHECK((std::optional<std::string>{} | optional_value_or(fallback)) == expected);
      CHECK(fallback == expected);
    }
  }
}

} // namespace kdl
