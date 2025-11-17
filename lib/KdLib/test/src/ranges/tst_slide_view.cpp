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

#include "kd/ranges/slide_view.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{

template <typename T>
auto make_slide(std::vector<T> v, const int n)
{
  return ranges::slide_view{std::move(v), n};
}

} // namespace

TEST_CASE("slide")
{
  SECTION("iterator / sentinel")
  {
    SECTION("required types")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto a = v | views::slide(3);

      static_assert(std::ranges::random_access_range<decltype(a)>);

      using iterator_type = decltype(a.begin());
      static_assert(
        std::is_same_v<iterator_type::iterator_concept, std::random_access_iterator_tag>);
      static_assert(
        std::is_same_v<iterator_type::iterator_category, std::input_iterator_tag>);

      using sentinel_type = decltype(a.end());
      static_assert(
        std::is_same_v<sentinel_type::iterator_concept, std::random_access_iterator_tag>);
    }

    SECTION("arithmetic")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto s = v | views::slide(2);

      auto it = s.begin();
      REQUIRE(std::ranges::equal(*it, std::vector{1, 2}));

      CHECK(std::ranges::equal(*it++, std::vector{1, 2}));
      CHECK(std::ranges::equal(*it, std::vector{2, 3}));

      CHECK(std::ranges::equal(*++it, std::vector{3, 4}));
      CHECK(std::ranges::equal(*it, std::vector{3, 4}));

      CHECK(std::next(it) == s.end());

      CHECK(std::ranges::equal(*it--, std::vector{3, 4}));
      CHECK(std::ranges::equal(*it, std::vector{2, 3}));

      CHECK(std::ranges::equal(*--it, std::vector{1, 2}));
      CHECK(std::ranges::equal(*it, std::vector{1, 2}));

      CHECK((it + 1) == std::next(s.begin()));
      CHECK((1 + it) == std::next(s.begin()));
      CHECK((it += 1) == std::next(s.begin()));
      CHECK(it == std::next(s.begin()));

      CHECK((it - 1) == s.begin());
      CHECK((it -= 1) == s.begin());
      CHECK(it == s.begin());

      CHECK(std::ranges::equal(*(s.begin() + 2), std::vector{3, 4}));
      CHECK(s.begin() + 3 == s.end());
    }

    SECTION("subscript")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto s = v | views::slide(2);

      auto it = s.begin();
      CHECK(std::ranges::equal(it[0], std::vector{1, 2}));
      CHECK(std::ranges::equal(it[1], std::vector{2, 3}));
      CHECK(std::ranges::equal(it[2], std::vector{3, 4}));

      // Check that the iterator is not invalidated
      CHECK(std::ranges::equal(*it, std::vector{1, 2}));
    }

    SECTION("comparison")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto x = v | views::slide(2);

      auto i = x.begin();
      auto s = x.end();

      CHECK(i == i);
      CHECK(s == s);
      CHECK(i == x.begin());
      CHECK(std::next(i, 3) == s);

      CHECK(i != s);
      CHECK_FALSE(i != i);

      CHECK_FALSE(i < i);
      CHECK(i <= i);
      CHECK_FALSE(i > i);
      CHECK(i >= i);
    }
  }


  SECTION("examples")
  {
    CHECK(
      recursive_ranges_equal(make_slide<int>({}, 2), std::vector<std::vector<int>>{}));
    CHECK(
      recursive_ranges_equal(make_slide<int>({1}, 2), std::vector<std::vector<int>>{}));
    CHECK(recursive_ranges_equal(
      make_slide<int>({1, 2}, 2), std::vector<std::vector<int>>{{1, 2}}));
    CHECK(recursive_ranges_equal(
      make_slide<int>({1, 2, 3}, 2), std::vector<std::vector<int>>{{1, 2}, {2, 3}}));
    CHECK(recursive_ranges_equal(
      make_slide<int>({1, 2, 3, 4}, 2),
      std::vector<std::vector<int>>{{1, 2}, {2, 3}, {3, 4}}));
    CHECK(recursive_ranges_equal(
      make_slide<int>({1, 2, 3, 4}, 3),
      std::vector<std::vector<int>>{{1, 2, 3}, {2, 3, 4}}));
    CHECK(recursive_ranges_equal(
      make_slide<int>({1, 2, 3}, 1), std::vector<std::vector<int>>{{1}, {2}, {3}}));
  }
}

} // namespace kdl
