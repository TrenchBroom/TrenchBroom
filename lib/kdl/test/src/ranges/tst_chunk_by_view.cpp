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

#include "range_test_utils.h"

#include "kdl/ranges/chunk_by_view.h"
#include "kdl/ranges/chunk_view.h"

#include <forward_list>
#include <ranges>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{

template <typename T, typename Pred>
auto make_chunked_by(std::vector<T> v, Pred&& pred)
{
  return ranges::chunk_by_view{std::move(v), std::forward<Pred>(pred)};
}

} // namespace

TEST_CASE("chunk_by")
{
  SECTION("iterator / sentinel")
  {
    SECTION("required types (forward range)")
    {
      auto l = std::forward_list{1, 2, 3, 4};
      auto c = l | views::chunk_by(std::ranges::equal_to{});

      using iter_type = decltype(c.begin());

      static_assert(
        std::is_same_v<iter_type::iterator_concept, std::forward_iterator_tag>);
      static_assert(
        std::is_same_v<iter_type::iterator_category, std::input_iterator_tag>);
    }

    SECTION("required types (random access range)")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto c = v | views::chunk_by(std::ranges::equal_to{});

      using iter_type = decltype(c.begin());

      static_assert(
        std::is_same_v<iter_type::iterator_concept, std::bidirectional_iterator_tag>);
      static_assert(
        std::is_same_v<iter_type::iterator_category, std::input_iterator_tag>);
    }

    SECTION("arithmetic (forward range)")
    {
      const auto l = std::forward_list{1, 1, 2, 3, 3, 3};
      auto c = l | views::chunk_by(std::ranges::equal_to{});

      auto it = c.begin();
      REQUIRE(std::ranges::equal(*it, std::vector<int>{1, 1}));

      it++;
      CHECK(std::ranges::equal(*it, std::vector<int>{2}));

      CHECK(std::ranges::equal(*++it, std::vector<int>{3, 3, 3}));
      CHECK(std::next(it) == c.end());
    }

    SECTION("arithmetic (bidirectional range)")
    {
      const auto v = std::vector{1, 1, 2, 3, 3, 3};
      auto c = v | views::chunk_by(std::ranges::equal_to{});

      auto it = c.begin();
      CHECK_FALSE(it == c.end());
      REQUIRE(std::ranges::equal(*it, std::vector<int>{1, 1}));

      CHECK(std::ranges::equal(*it++, std::vector<int>{1, 1}));
      CHECK(std::ranges::equal(*it, std::vector<int>{2}));
      CHECK_FALSE(it == c.end());

      CHECK(std::ranges::equal(*++it, std::vector<int>{3, 3, 3}));
      CHECK(std::ranges::equal(*it, std::vector<int>{3, 3, 3}));
      CHECK_FALSE(it == c.end());

      CHECK(std::next(it) == c.end());

      CHECK(std::ranges::equal(*it--, std::vector<int>{3, 3, 3}));
      CHECK(std::ranges::equal(*it, std::vector<int>{2}));

      CHECK(std::ranges::equal(*--it, std::vector<int>{1, 1}));
      CHECK(std::ranges::equal(*it, std::vector<int>{1, 1}));
    }
  }

  SECTION("examples")
  {
    CHECK(recursive_ranges_equal(
      make_chunked_by<int>({}, std::ranges::equal_to{}),
      std::vector<std::vector<int>>{}));
    CHECK(recursive_ranges_equal(
      make_chunked_by<int>({1}, std::ranges::equal_to{}),
      std::vector<std::vector<int>>{{1}}));
    CHECK(recursive_ranges_equal(
      make_chunked_by<int>({1, 2}, std::ranges::equal_to{}),
      std::vector<std::vector<int>>{{1}, {2}}));
    CHECK(recursive_ranges_equal(
      make_chunked_by<int>({1, 1, 2}, std::ranges::equal_to{}),
      std::vector<std::vector<int>>{{1, 1}, {2}}));
    CHECK(recursive_ranges_equal(
      make_chunked_by<int>({1, 2, 2}, std::ranges::equal_to{}),
      std::vector<std::vector<int>>{{1}, {2, 2}}));
    CHECK(recursive_ranges_equal(
      make_chunked_by<int>({1, 2, 2, 2, 3, 4, 1, 1, 2, 3}, std::ranges::less{}),
      std::vector<std::vector<int>>{{1, 2}, {2}, {2, 3, 4}, {1}, {1, 2, 3}}));
  }
}

} // namespace kdl
