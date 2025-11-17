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

#include "kd/ranges/chunk_view.h"

#include <algorithm>
#include <ranges>
#include <sstream>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{

template <typename T>
auto make_chunked(std::vector<T> v, const int n)
{
  return ranges::chunk_view{std::move(v), n};
}

} // namespace

TEST_CASE("chunk")
{
  SECTION("iterator / sentinel")
  {
    SECTION("required types (input range)")
    {
      auto i = std::istringstream{"5 4 3 2 1"};
      auto iv = std::ranges::istream_view<int>(i);
      auto c = views::chunk(iv, 3);

      using outer_iter_type = decltype(c.begin());

      static_assert(
        std::is_same_v<outer_iter_type::iterator_concept, std::input_iterator_tag>);

      using inner_iter_type = decltype((*c.begin()).begin());
      static_assert(
        std::is_same_v<inner_iter_type::iterator_concept, std::input_iterator_tag>);
      static_assert(std::is_same_v<inner_iter_type::value_type, int>);
    }

    SECTION("required types (random access range)")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8};
      auto c = v | views::chunk(3);

      using outer_iter_type = decltype(c.begin());

      static_assert(
        std::
          is_same_v<outer_iter_type::iterator_concept, std::random_access_iterator_tag>);

      using inner_iter_type = decltype((*c.begin()).begin());
      static_assert(
        std::is_same_v<inner_iter_type::iterator_concept, std::contiguous_iterator_tag>);
      static_assert(std::is_same_v<inner_iter_type::value_type, int>);
    }

    SECTION("arithmetic (input range)")
    {
      auto i = std::istringstream{"1 2 3 4 5"};
      auto iv = std::ranges::istream_view<int>(i);
      auto c = views::chunk(iv, 2);

      auto it = c.begin();
      REQUIRE(std::ranges::equal(*it, std::vector<int>{1, 2}));

      it++;
      CHECK(std::ranges::equal(*it, std::vector<int>{3, 4}));

      CHECK(std::ranges::equal(*++it, std::vector<int>{5}));
      CHECK(it == c.end());
    }

    SECTION("arithmetic (random access range)")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5};
      auto c = v | views::chunk(2);

      auto it = c.begin();
      CHECK_FALSE(it == c.end());
      REQUIRE(std::ranges::equal(*it, std::vector<int>{1, 2}));

      CHECK(std::ranges::equal(*it++, std::vector<int>{1, 2}));
      CHECK(std::ranges::equal(*it, std::vector<int>{3, 4}));
      CHECK_FALSE(it == c.end());

      CHECK(std::ranges::equal(*++it, std::vector<int>{5}));
      CHECK(std::ranges::equal(*it, std::vector<int>{5}));
      CHECK_FALSE(it == c.end());

      CHECK(std::next(it) == c.end());

      CHECK(std::ranges::equal(*it--, std::vector<int>{5}));
      CHECK(std::ranges::equal(*it, std::vector<int>{3, 4}));

      CHECK(std::ranges::equal(*--it, std::vector<int>{1, 2}));
      CHECK(std::ranges::equal(*it, std::vector<int>{1, 2}));

      CHECK((it + 1) == std::next(c.begin()));
      CHECK((1 + it) == std::next(c.begin()));
      CHECK((it += 1) == std::next(c.begin()));
      CHECK(it == std::next(c.begin()));

      CHECK((it - 1) == c.begin());
      CHECK((it -= 1) == c.begin());
      CHECK(it == c.begin());

      CHECK(std::next(c.begin()) - c.begin() == 1);
      CHECK(std::next(c.begin(), 2) - c.begin() == 2);
      CHECK(c.begin() - std::next(c.begin()) == -1);

      CHECK(std::next(c.begin()) - c.begin() == 1);
      CHECK(std::next(c.begin(), 2) - c.begin() == 2);
      CHECK(c.end() - c.begin() == 3);
      CHECK(c.begin() - c.end() == -3);
    }

    SECTION("subscript (random access range)")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5};
      auto c = v | views::chunk(2);

      auto it = c.begin();
      CHECK(std::ranges::equal(it[0], std::vector<int>{1, 2}));
      CHECK(std::ranges::equal(it[1], std::vector<int>{3, 4}));
      CHECK(std::ranges::equal(it[2], std::vector<int>{5}));

      // Check that the iterator is not invalidated
      CHECK(std::ranges::equal(*it, std::vector<int>{1, 2}));
    }

    SECTION("comparison (random access range)")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5};
      auto c = v | views::chunk(2);

      auto i = c.begin();
      auto s = c.end();

      CHECK(i == i);
      CHECK(i == c.begin());
      CHECK(std::next(i, 3) == s);

      CHECK(i != s);
      CHECK_FALSE(i != i);

      CHECK_FALSE(i < i);
      CHECK(i <= i);
      CHECK_FALSE(i > i);
      CHECK(i >= i);
    }
  }

  SECTION("size (random access range)")
  {
    SECTION("with non-empty range")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5};
      const auto c = v | views::chunk(2);
      CHECK(c.size() == 3);
    }

    SECTION("with range having one incomplete element")
    {
      const auto v = std::vector<int>{1};
      const auto c = v | views::chunk(2);
      CHECK(c.size() == 1);
    }

    SECTION("with empty range")
    {
      const auto v = std::vector<int>{};
      const auto c = v | views::chunk(2);
      CHECK(c.size() == 0);
    }
  }

  SECTION("examples")
  {
    CHECK(
      recursive_ranges_equal(make_chunked<int>({}, 2), std::vector<std::vector<int>>{}));
    CHECK(recursive_ranges_equal(
      make_chunked<int>({1}, 2), std::vector<std::vector<int>>{{1}}));
    CHECK(recursive_ranges_equal(
      make_chunked<int>({1, 2}, 2), std::vector<std::vector<int>>{{1, 2}}));
    CHECK(recursive_ranges_equal(
      make_chunked<int>({1, 2, 3}, 2), std::vector<std::vector<int>>{{1, 2}, {3}}));
    CHECK(recursive_ranges_equal(
      make_chunked<int>({1, 2, 3, 4}, 2), std::vector<std::vector<int>>{{1, 2}, {3, 4}}));
    CHECK(recursive_ranges_equal(
      make_chunked<int>({1, 2, 3, 4}, 3), std::vector<std::vector<int>>{{1, 2, 3}, {4}}));
    CHECK(recursive_ranges_equal(
      make_chunked<int>({1, 2, 3}, 1), std::vector<std::vector<int>>{{1}, {2}, {3}}));
  }
}

} // namespace kdl
