/*
 Copyright (C) 2025 Kristian Duske

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

#include "kdl/ranges/to.h"

#include <list>
#include <map>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{

TEST_CASE("to")
{
  SECTION("with specified collection type")
  {
    CHECK(ranges::to<std::list<int>>(std::vector{1, 2, 3, 3}) == std::list{1, 2, 3, 3});
    CHECK(
      ranges::to<std::vector<int>>(std::vector{1, 2, 3, 3}) == std::vector{1, 2, 3, 3});
    CHECK(ranges::to<std::set<int>>(std::vector{1, 2, 3, 3}) == std::set{1, 2, 3});
    CHECK(
      ranges::to<std::unordered_set<int>>(std::vector{1, 2, 3, 3})
      == std::unordered_set{1, 2, 3});

    CHECK(
      ranges::to<std::map<int, std::string>>(
        std::vector<std::pair<int, std::string>>{{1, "1"}, {2, "2"}, {3, "3"}, {3, "4"}})
      == std::map<int, std::string>{{1, "1"}, {2, "2"}, {3, "3"}});
    CHECK(
      ranges::to<std::unordered_map<int, std::string>>(
        std::vector<std::pair<int, std::string>>{{1, "1"}, {2, "2"}, {3, "3"}, {3, "4"}})
      == std::unordered_map<int, std::string>{{1, "1"}, {2, "2"}, {3, "3"}});
  }

  SECTION("with deduced collecion type")
  {
    CHECK(ranges::to<std::list>(std::vector{1, 2, 3, 3}) == std::list{1, 2, 3, 3});
    CHECK(ranges::to<std::vector>(std::vector{1, 2, 3, 3}) == std::vector{1, 2, 3, 3});
    CHECK(ranges::to<std::set>(std::vector{1, 2, 3, 3}) == std::set{1, 2, 3});
    CHECK(
      ranges::to<std::unordered_set>(std::vector{1, 2, 3, 3})
      == std::unordered_set{1, 2, 3});

    CHECK(
      ranges::to<std::map>(
        std::vector<std::pair<int, std::string>>{{1, "1"}, {2, "2"}, {3, "3"}, {3, "4"}})
      == std::map<int, std::string>{{1, "1"}, {2, "2"}, {3, "3"}});
    CHECK(
      ranges::to<std::unordered_map>(
        std::vector<std::pair<int, std::string>>{{1, "1"}, {2, "2"}, {3, "3"}, {3, "4"}})
      == std::unordered_map<int, std::string>{{1, "1"}, {2, "2"}, {3, "3"}});
  }

  SECTION("call wrapper with specified collection type")
  {
    auto t = ranges::to<std::vector<int>>();
    CHECK(t(std::vector{1, 2, 3}) == std::vector{1, 2, 3});
  }

  SECTION("call wrapper with deduced collection type")
  {
    auto t = ranges::to<std::vector>();
    CHECK(t(std::vector{1, 2, 3}) == std::vector{1, 2, 3});
  }

  SECTION("pipe operator with specified collection type")
  {
    auto v = std::vector{1, 2, 3};
    SECTION("when call wrapper is an rvalue")
    {
      CHECK((v | ranges::to<std::vector<int>>()) == std::vector{1, 2, 3});
    }

    SECTION("when call wrapper is an lvalue")
    {
      auto t = ranges::to<std::vector<int>>();
      CHECK((v | t) == std::vector{1, 2, 3});
    }

    SECTION("when call wrapper is a const lvalue")
    {
      const auto t = ranges::to<std::vector<int>>();
      CHECK((v | t) == std::vector{1, 2, 3});
    }
  }

  SECTION("pipe operator with deduced collection type")
  {
    auto v = std::vector{1, 2, 3};
    CHECK((v | ranges::to<std::vector>()) == std::vector{1, 2, 3});
  }
}

} // namespace kdl
