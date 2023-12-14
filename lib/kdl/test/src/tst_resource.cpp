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

#include "kdl/resource.h"

#include <catch2/catch.hpp>

namespace kdl
{
TEST_CASE("resource")
{
  auto deleter_calls = std::vector<int>{};
  auto deleter = [&](const auto i) {
    deleter_calls.push_back(i);
    ;
  };

  SECTION("destructor calls deleter")
  {
    {
      auto r = resource{1, deleter};
      REQUIRE(deleter_calls.empty());
    }

    CHECK(deleter_calls == std::vector<int>{1});
  }

  SECTION("move constructor")
  {
    {
      auto r = resource{1, deleter};
      REQUIRE(deleter_calls.empty());

      {
        auto s = resource{std::move(r)};
        CHECK(*s == 1);
        REQUIRE(deleter_calls.empty());
      }

      CHECK(deleter_calls == std::vector<int>{1});
    }

    CHECK(deleter_calls == std::vector<int>{1});
  }

  SECTION("move assignment")
  {
    {
      auto r = resource{1, deleter};
      REQUIRE(deleter_calls.empty());

      {
        auto s = std::move(r);
        CHECK(*s == 1);
        REQUIRE(deleter_calls.empty());
      }

      CHECK(deleter_calls == std::vector<int>{1});
    }

    CHECK(deleter_calls == std::vector<int>{1});
  }

  SECTION("value assignment")
  {
    {
      auto r = resource{1, deleter};
      REQUIRE(deleter_calls.empty());

      r = 2;
      CHECK(*r == 2);
      CHECK(deleter_calls == std::vector<int>{1});
    }

    CHECK(deleter_calls == std::vector<int>{1, 2});
  }

  SECTION("operator bool")
  {
    CHECK(bool(resource{1, [](auto) {}}));
    CHECK_FALSE(bool(resource{0, [](auto) {}}));

    const auto i = 0;
    CHECK(bool(resource{&i, [](auto) {}}));
    CHECK_FALSE(bool(resource{static_cast<int*>(nullptr), [](auto) {}}));
  }

  SECTION("release")
  {
    {
      auto r = resource{1, deleter};
      REQUIRE(deleter_calls.empty());

      CHECK(r.release() == 1);
      CHECK(deleter_calls.empty());
    }

    CHECK(deleter_calls.empty());
  }
}
} // namespace kdl
