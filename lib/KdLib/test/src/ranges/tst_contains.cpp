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

#include "kd/ranges/contains.h"

#include <array>
#include <cctype>
#include <forward_list>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{

struct Item
{
  int id;
  std::string name;
};

} // namespace

TEST_CASE("contains")
{
  SECTION("contains")
  {
    SECTION("range")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};

      CHECK(ranges::contains(v, 1));
      CHECK(ranges::contains(v, 4));
      CHECK(!ranges::contains(v, 5));
      CHECK(!ranges::contains(std::vector<int>{}, 1));
    }

    SECTION("iterator / sentinel")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};

      CHECK(ranges::contains(v.begin(), v.end(), 3));
      CHECK(!ranges::contains(v.begin(), v.begin() + 2, 3));
      CHECK(!ranges::contains(v.begin(), v.begin(), 1));

      // the sentinel has a different type than the iterator
      CHECK(
        ranges::contains(std::counted_iterator{v.begin(), 3}, std::default_sentinel, 3));
      CHECK(
        !ranges::contains(std::counted_iterator{v.begin(), 3}, std::default_sentinel, 4));
    }

    SECTION("value of a different type")
    {
      const auto v = std::vector<std::string>{"a", "b", "c"};

      CHECK(ranges::contains(v, "b"));
      CHECK(ranges::contains(v, std::string_view{"c"}));
      CHECK(!ranges::contains(v, "d"));
    }

    SECTION("projection")
    {
      const auto v = std::vector<Item>{{1, "one"}, {2, "two"}, {3, "three"}};

      CHECK(ranges::contains(v, 2, &Item::id));
      CHECK(!ranges::contains(v, 4, &Item::id));
      CHECK(ranges::contains(v, "three", &Item::name));
      CHECK(ranges::contains(v, 6, [](const auto& item) { return item.id * 2; }));
      CHECK(ranges::contains(v.begin(), v.end(), "one", &Item::name));
      CHECK(!ranges::contains(v.begin(), v.end(), "four", &Item::name));
    }

    SECTION("input range")
    {
      auto found = std::istringstream{"1 2 3 4"};
      CHECK(ranges::contains(std::views::istream<int>(found), 3));

      auto notFound = std::istringstream{"1 2 3 4"};
      CHECK(!ranges::contains(std::views::istream<int>(notFound), 5));
    }

    SECTION("view")
    {
      CHECK(ranges::contains(std::views::iota(1, 5), 4));
      CHECK(!ranges::contains(std::views::iota(1, 5), 5));
      CHECK(ranges::contains(
        std::views::iota(1) | std::views::transform([](const auto i) { return i * i; }),
        49));
    }

    SECTION("constexpr")
    {
      static constexpr auto a = std::array{1, 2, 3, 4};

      static_assert(ranges::contains(a, 3));
      static_assert(!ranges::contains(a, 5));
      static_assert(ranges::contains(a.begin(), a.end(), 3));
      static_assert(ranges::contains(a, 6, [](const auto i) { return i * 2; }));
    }
  }

  SECTION("contains_subrange")
  {
    SECTION("range")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};

      CHECK(ranges::contains_subrange(v, std::vector<int>{1}));
      CHECK(ranges::contains_subrange(v, std::vector<int>{1, 2}));
      CHECK(ranges::contains_subrange(v, std::vector<int>{2, 3}));
      CHECK(ranges::contains_subrange(v, std::vector<int>{3, 4}));
      CHECK(ranges::contains_subrange(v, v));
      CHECK(!ranges::contains_subrange(v, std::vector<int>{5}));
      CHECK(!ranges::contains_subrange(v, std::vector<int>{1, 3}));
      CHECK(!ranges::contains_subrange(v, std::vector<int>{2, 1}));
      CHECK(!ranges::contains_subrange(v, std::vector<int>{4, 5}));
      CHECK(!ranges::contains_subrange(v, std::vector<int>{1, 2, 3, 4, 5}));
    }

    SECTION("empty ranges")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      const auto e = std::vector<int>{};

      CHECK(ranges::contains_subrange(v, e));
      CHECK(ranges::contains_subrange(e, e));
      CHECK(!ranges::contains_subrange(e, v));
    }

    SECTION("iterator / sentinel")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      const auto s = std::vector<int>{3, 4};

      CHECK(ranges::contains_subrange(v.begin(), v.end(), s.begin(), s.end()));
      CHECK(!ranges::contains_subrange(v.begin(), v.begin() + 3, s.begin(), s.end()));
      CHECK(
        ranges::contains_subrange(v.begin(), v.begin() + 3, s.begin(), s.begin() + 1));
      CHECK(ranges::contains_subrange(v.begin(), v.begin(), s.begin(), s.begin()));

      // the sentinels have a different type than the iterators
      CHECK(ranges::contains_subrange(
        std::counted_iterator{v.begin(), 4},
        std::default_sentinel,
        std::counted_iterator{s.begin(), 2},
        std::default_sentinel));
      CHECK(!ranges::contains_subrange(
        std::counted_iterator{v.begin(), 3},
        std::default_sentinel,
        std::counted_iterator{s.begin(), 2},
        std::default_sentinel));
    }

    SECTION("ranges of different types")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};

      CHECK(ranges::contains_subrange(v, std::forward_list<long>{2, 3}));
      CHECK(ranges::contains_subrange(
        std::forward_list<long>{1, 2, 3}, v | std::views::take(2)));
      CHECK(ranges::contains_subrange(v, std::views::iota(2, 5)));
      CHECK(!ranges::contains_subrange(v, std::views::iota(2, 6)));
      CHECK(
        ranges::contains_subrange(std::string{"hello world"}, std::string_view{"o w"}));
    }

    SECTION("predicate")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};

      CHECK(
        ranges::contains_subrange(v, std::vector<int>{0, 1, 2}, std::ranges::greater{}));
      CHECK(
        !ranges::contains_subrange(v, std::vector<int>{0, 2, 4}, std::ranges::greater{}));
      CHECK(ranges::contains_subrange(
        std::string_view{"Hello World"},
        std::string_view{"LO WO"},
        [](const auto lhs, const auto rhs) {
          return std::tolower(static_cast<unsigned char>(lhs))
                 == std::tolower(static_cast<unsigned char>(rhs));
        }));
    }

    SECTION("projection")
    {
      const auto v = std::vector<Item>{{1, "one"}, {2, "two"}, {3, "three"}};

      CHECK(ranges::contains_subrange(v, std::vector<int>{2, 3}, {}, &Item::id));
      CHECK(!ranges::contains_subrange(v, std::vector<int>{3, 2}, {}, &Item::id));
      CHECK(ranges::contains_subrange(
        v, std::vector<std::string>{"one", "two"}, {}, &Item::name));
      CHECK(ranges::contains_subrange(
        std::vector<int>{0, 2, 4, 6, 8}, v, {}, {}, [](const auto& item) {
          return item.id * 2;
        }));
      CHECK(ranges::contains_subrange(v, v, {}, &Item::id, &Item::id));
      CHECK(ranges::contains_subrange(
        v.begin(), v.end(), v.begin() + 1, v.end(), {}, &Item::name, &Item::name));
    }

    SECTION("constexpr")
    {
      static constexpr auto a = std::array{1, 2, 3, 4};
      static constexpr auto s = std::array{2, 3};
      static constexpr auto n = std::array{3, 2};

      static_assert(ranges::contains_subrange(a, s));
      static_assert(!ranges::contains_subrange(a, n));
      static_assert(ranges::contains_subrange(a, std::array<int, 0>{}));
      static_assert(ranges::contains_subrange(a.begin(), a.end(), s.begin(), s.end()));
      static_assert(ranges::contains_subrange(a, n, std::ranges::not_equal_to{}));
    }
  }
}

} // namespace kdl
