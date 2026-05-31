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

#include "kd/ranges/join_with_view.h"

#include <forward_list>
#include <memory>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace kdl::ranges
{
namespace
{

template <typename T>
concept has_iterator_category = requires { typename T::iterator_category; };

} // namespace

TEST_CASE("join_with")
{
  using Catch::Matchers::RangeEquals;

  SECTION("required iterator types (forward range of forward ranges)")
  {
    auto v = std::vector<std::vector<int>>{{1, 2}, {3, 4}};
    auto pattern = std::vector<int>{0};
    auto j = ranges::join_with_view{v, pattern};

    using iter_type = decltype(j.begin());
    static_assert(
      std::is_same_v<iter_type::iterator_concept, std::bidirectional_iterator_tag>);
    static_assert(
      std::is_same_v<iter_type::iterator_category, std::bidirectional_iterator_tag>);
    static_assert(std::is_same_v<iter_type::value_type, int>);
  }

  SECTION("required iterator types (forward range of forward-only ranges)")
  {
    auto v = std::vector<std::forward_list<int>>{{1, 2}, {3, 4}};
    auto pattern = std::forward_list<int>{0};
    auto j = ranges::join_with_view{v, pattern};

    using iter_type = decltype(j.begin());
    static_assert(std::is_same_v<iter_type::iterator_concept, std::forward_iterator_tag>);
    static_assert(
      std::is_same_v<iter_type::iterator_category, std::forward_iterator_tag>);
  }

  SECTION("iterator_category absent when inner reference is a prvalue")
  {
    // transform's reference type is a prvalue, so iterator_category should not
    // be defined per the C++23 spec.
    auto in = std::istringstream{"1 2"};
    auto numbers = std::ranges::istream_view<int>(in);
    auto v = numbers | std::views::transform([](int n) { return std::vector<int>{n}; });
    auto j = ranges::join_with_view{std::move(v), 0};

    using iter_type = decltype(j.begin());
    static_assert(!has_iterator_category<iter_type>);
  }

  SECTION("basic joining with separator (vector of vectors)")
  {
    const auto v = std::vector<std::vector<int>>{{1, 2, 3}, {4, 5}, {6}};
    const auto pattern = std::vector<int>{0};

    auto j = ranges::join_with_view{v, pattern};
    CHECK_THAT(j, RangeEquals(std::vector<int>{1, 2, 3, 0, 4, 5, 0, 6}));
  }

  SECTION("joining empty inner ranges emits the separator between them")
  {
    const auto v = std::vector<std::vector<int>>{{}, {1, 2}, {}, {3}};
    const auto pattern = std::vector<int>{0};

    auto j = ranges::join_with_view{v, pattern};
    CHECK_THAT(j, RangeEquals(std::vector<int>{0, 1, 2, 0, 0, 3}));
  }

  SECTION("joining with a single-element separator (value form)")
  {
    const auto v = std::vector<std::vector<int>>{{1, 2}, {3}, {4, 5}};

    auto j = ranges::join_with_view{v, 0};
    CHECK_THAT(j, RangeEquals(std::vector<int>{1, 2, 0, 3, 0, 4, 5}));
  }

  SECTION("single inner range yields no separators")
  {
    const auto v = std::vector<std::vector<int>>{{1, 2, 3}};
    const auto pattern = std::vector<int>{0};

    auto j = ranges::join_with_view{v, pattern};
    CHECK_THAT(j, RangeEquals(std::vector<int>{1, 2, 3}));
  }

  SECTION("outer empty range")
  {
    const auto v = std::vector<std::vector<int>>{};
    const auto pattern = std::vector<int>{0};

    auto j = ranges::join_with_view{v, pattern};
    CHECK(j.begin() == j.end());
  }

  SECTION("single empty inner range")
  {
    const auto v = std::vector<std::vector<int>>{{}};
    const auto pattern = std::vector<int>{0};

    auto j = ranges::join_with_view{v, pattern};
    CHECK(j.begin() == j.end());
  }

  SECTION("all inner ranges empty yields only separators")
  {
    const auto v = std::vector<std::vector<int>>{{}, {}, {}};
    const auto pattern = std::vector<int>{7, 8};

    auto j = ranges::join_with_view{v, pattern};
    CHECK_THAT(j, RangeEquals(std::vector<int>{7, 8, 7, 8}));
  }

  SECTION("empty separator behaves like plain join")
  {
    const auto v = std::vector<std::vector<int>>{{1, 2}, {3, 4}, {5}};
    const auto pattern = std::vector<int>{};

    auto j = ranges::join_with_view{v, pattern};
    CHECK_THAT(j, RangeEquals(std::vector<int>{1, 2, 3, 4, 5}));
  }

  SECTION("pipe form")
  {
    const auto v = std::vector<std::vector<int>>{{1, 2}, {3, 4}};
    auto j = v | views::join_with(std::vector<int>{0});
    CHECK_THAT(j, RangeEquals(std::vector<int>{1, 2, 0, 3, 4}));
  }

  SECTION("two-argument call form")
  {
    const auto v = std::vector<std::vector<int>>{{1, 2}, {3, 4}};
    auto j = views::join_with(v, 0);
    CHECK_THAT(j, RangeEquals(std::vector<int>{1, 2, 0, 3, 4}));
  }

  SECTION("multi-element separator joining strings")
  {
    const auto v = std::vector<std::string>{"hello", "world", "foo"};
    auto j = ranges::join_with_view{v, std::string_view{", "}};

    CHECK_THAT(j, RangeEquals(std::string_view{"hello, world, foo"}));
  }

  SECTION("const view")
  {
    const auto v = std::vector<std::vector<int>>{{1, 2}, {3, 4}};
    const auto j = ranges::join_with_view{v, std::vector<int>{0}};

    static_assert(std::ranges::range<const decltype(j)>);
    CHECK_THAT(j, RangeEquals(std::vector<int>{1, 2, 0, 3, 4}));
  }

  SECTION("bidirectional iteration")
  {
    auto v = std::vector<std::vector<int>>{{1, 2}, {3, 4}, {5}};
    auto pattern = std::vector<int>{0};
    auto j = ranges::join_with_view{v, pattern};

    auto it = j.begin();
    const auto end = j.end();

    CHECK(*it == 1);
    ++it;
    CHECK(*it == 2);
    ++it;
    CHECK(*it == 0);
    ++it;
    CHECK(*it == 3);

    --it;
    CHECK(*it == 0);
    --it;
    CHECK(*it == 2);
    --it;
    CHECK(*it == 1);

    auto last = j.begin();
    while (std::next(last) != end)
    {
      ++last;
    }
    CHECK(*last == 5);

    --last;
    CHECK(*last == 0);
    --last;
    CHECK(*last == 4);
  }

  SECTION("iterator equality")
  {
    auto v = std::vector<std::vector<int>>{{1, 2}, {3}};
    auto pattern = std::vector<int>{0};
    auto j = ranges::join_with_view{v, pattern};

    auto it1 = j.begin();
    auto it2 = j.begin();
    CHECK(it1 == it2);

    ++it2;
    CHECK_FALSE(it1 == it2);

    ++it1;
    CHECK(it1 == it2);
  }

  SECTION("base() accessor")
  {
    auto v = std::vector<std::vector<int>>{{1, 2}, {3, 4}};
    auto pattern = std::vector<int>{0};

    auto j = ranges::join_with_view{v, pattern};
    CHECK_THAT(j.base(), RangeEquals(v));

    auto moved = std::move(j).base();
    CHECK_THAT(moved, RangeEquals(v));
  }

  SECTION("move-only element types: iter_move transfers ownership")
  {
    auto inner1 = std::vector<std::unique_ptr<int>>{};
    inner1.push_back(std::make_unique<int>(1));
    inner1.push_back(std::make_unique<int>(2));

    auto inner2 = std::vector<std::unique_ptr<int>>{};
    inner2.push_back(std::make_unique<int>(3));

    auto v = std::vector<std::vector<std::unique_ptr<int>>>{};
    v.push_back(std::move(inner1));
    v.push_back(std::move(inner2));

    auto pattern = std::vector<std::unique_ptr<int>>{};
    auto j = ranges::join_with_view{v, std::move(pattern)};

    auto out = std::vector<std::unique_ptr<int>>{};
    for (auto it = j.begin(); it != j.end(); ++it)
    {
      out.push_back(std::ranges::iter_move(it));
    }

    REQUIRE(out.size() == 3);
    CHECK(*out[0] == 1);
    CHECK(*out[1] == 2);
    CHECK(*out[2] == 3);

    for (const auto& inner : v)
    {
      for (const auto& p : inner)
      {
        CHECK(p == nullptr);
      }
    }
  }

  SECTION("input-only outer range with prvalue inner ranges")
  {
    // istream_view is an input-only range. Wrapping it in transform yields an
    // input-only range whose reference type is a prvalue std::vector<int> —
    // this exercises both the !forward_range<V> path (parent's outer_it_
    // cache) and the !ref_is_glvalue path (parent's inner_ cache).
    auto in = std::istringstream{"1 2 3 4 5"};
    auto numbers = std::ranges::istream_view<int>(in);

    auto v =
      numbers | std::views::transform([](int n) { return std::vector<int>{n, n * 10}; });

    auto j = ranges::join_with_view{std::move(v), 0};

    CHECK_THAT(
      j, RangeEquals(std::vector<int>{1, 10, 0, 2, 20, 0, 3, 30, 0, 4, 40, 0, 5, 50}));
  }

  SECTION("sentinel comparison with non-common outer range")
  {
    // take_view of a vector is not a common_range, so end() yields a sentinel
    // rather than an iterator.
    auto v = std::vector<std::vector<int>>{{1, 2}, {3, 4}, {5, 6}};
    auto j = ranges::join_with_view{v | std::views::take(2), 0};

    CHECK_THAT(j, RangeEquals(std::vector<int>{1, 2, 0, 3, 4}));

    auto it = j.begin();
    CHECK_FALSE(it == j.end());
    while (it != j.end())
    {
      ++it;
    }
    CHECK(it == j.end());
  }
}

} // namespace kdl::ranges
