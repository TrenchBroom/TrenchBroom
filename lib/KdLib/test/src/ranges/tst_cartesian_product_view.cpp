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

#include "kd/ranges/cartesian_product_view.h"

#include <algorithm>
#include <forward_list>
#include <memory>
#include <sstream>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace kdl
{
namespace
{

template <typename T, typename U>
auto make(std::vector<T> v, std::vector<U> w)
{
  return views::cartesian_product(std::move(v), std::move(w));
}

} // namespace

TEST_CASE("cartesian_product")
{
  using namespace Catch::Matchers;

  SECTION("iterator / sentinel")
  {
    SECTION("required types (random access range)")
    {
      const auto v = std::vector<int>{1, 2, 3};
      const auto w = std::vector<float>{4.0f, 5.0f};
      auto c = views::cartesian_product(v, w);

      static_assert(std::ranges::random_access_range<decltype(c)>);

      using iterator_type = decltype(c.begin());
      static_assert(
        std::is_same_v<iterator_type::iterator_concept, std::random_access_iterator_tag>);
      static_assert(
        std::is_same_v<iterator_type::iterator_category, std::input_iterator_tag>);

      using sentinel_type = decltype(c.end());
      static_assert(
        std::is_same_v<sentinel_type::iterator_concept, std::random_access_iterator_tag>);
    }

    SECTION("required types (first range is input range)")
    {
      auto i = std::istringstream{"5 4 3 2 1"};
      auto iv = std::ranges::istream_view<int>(i);
      auto w = std::vector<float>{4.0f, 5.0f};
      auto c = views::cartesian_product(iv, w);

      using iterator_type = decltype(c.begin());
      static_assert(
        std::is_same_v<iterator_type::iterator_concept, std::input_iterator_tag>);
    }

    SECTION("required types (first range is forward range)")
    {
      auto l = std::forward_list<int>{5, 4, 3, 2, 1};
      auto w = std::vector<float>{4.0f, 5.0f};
      auto c = views::cartesian_product(l, w);

      using iterator_type = decltype(c.begin());
      static_assert(
        std::is_same_v<iterator_type::iterator_concept, std::forward_iterator_tag>);
    }

    SECTION("required types (first range is not a sized range)")
    {
      auto i = std::views::iota(0);
      auto w = std::vector<float>{4.0f, 5.0f};
      auto c = views::cartesian_product(i, w);

      using iterator_type = decltype(c.begin());
      static_assert(
        std::is_same_v<iterator_type::iterator_concept, std::random_access_iterator_tag>);
    }

    SECTION("arithmetic")
    {
      const auto v = std::vector<int>{1, 2};
      const auto w = std::vector<float>{4.0f, 5.0f};
      auto c = views::cartesian_product(v, w);

      auto it = c.begin();
      CHECK(*it == std::tuple{1, 4.0f});

      CHECK(*it++ == std::tuple{1, 4.0f});
      CHECK(*it == std::tuple{1, 5.0f});

      CHECK(*++it == std::tuple{2, 4.0f});
      CHECK(*it == std::tuple{2, 4.0f});
      CHECK(*++it == std::tuple{2, 5.0f});

      CHECK(std::next(it) == c.end());

      CHECK(*it-- == std::tuple{2, 5.0f});
      CHECK(*it == std::tuple{2, 4.0f});

      CHECK(*--it == std::tuple{1, 5.0f});
      CHECK(*it == std::tuple{1, 5.0f});
      CHECK(*--it == std::tuple{1, 4.0f});

      CHECK((it + 1) == std::next(c.begin()));
      CHECK((1 + it) == std::next(c.begin()));
      CHECK((it += 1) == std::next(c.begin()));
      CHECK(it == std::next(c.begin()));

      CHECK((it - 1) == c.begin());
      CHECK((it -= 1) == c.begin());
      CHECK(it == c.begin());

      CHECK(*(c.begin() + 2) == std::tuple{2, 4.0f});
      CHECK(c.begin() + 4 == c.end());
    }

    SECTION("subscript")
    {
      const auto v = std::vector<int>{1, 2};
      const auto w = std::vector<float>{4.0f, 5.0f};
      auto c = views::cartesian_product(v, w);

      auto it = c.begin();
      CHECK(it[0] == std::tuple{1, 4.0f});
      CHECK(it[1] == std::tuple{1, 5.0f});
      CHECK(it[2] == std::tuple{2, 4.0f});
      CHECK(it[3] == std::tuple{2, 5.0f});

      // Check that the iterator is not invalidated
      CHECK(*it == std::tuple{1, 4.0f});
    }

    SECTION("comparison")
    {
      const auto v = std::vector<int>{1, 2};
      const auto w = std::vector<float>{4.0f, 5.0f};
      auto c = views::cartesian_product(v, w);

      auto i = c.begin();
      auto s = c.end();

      CHECK(i == i);
      CHECK(s == s);
      CHECK(i == c.begin());
      CHECK(std::next(i, 4) == s);

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
    CHECK_THAT(
      (make<int, float>({}, {})), RangeEquals(std::vector<std::tuple<int, float>>{}));
    CHECK_THAT(
      (make<int, float>({1}, {})), RangeEquals(std::vector<std::tuple<int, float>>{}));
    CHECK_THAT(
      (make<int, float>({}, {4.0f})), RangeEquals(std::vector<std::tuple<int, float>>{}));
    CHECK_THAT(
      (make<int, float>({1}, {4.0f})),
      RangeEquals(std::vector<std::tuple<int, float>>{{1, 4.0f}}));
    CHECK_THAT(
      (make<int, float>({1, 2}, {4.0f})),
      RangeEquals(std::vector<std::tuple<int, float>>{{1, 4.0f}, {2, 4.0f}}));
    CHECK_THAT(
      (make<int, float>({1}, {4.0f, 5.0f})),
      RangeEquals(std::vector<std::tuple<int, float>>{{1, 4.0f}, {1, 5.0f}}));
    CHECK_THAT(
      (make<int, float>({1, 2}, {4.0f, 5.0f})),
      RangeEquals(
        std::vector<std::tuple<int, float>>{{1, 4.0f}, {1, 5.0f}, {2, 4.0f}, {2, 5.0f}}));
    CHECK_THAT(
      (make<int, float>({1, 2, 3}, {4.0f, 5.0f})),
      RangeEquals(std::vector<std::tuple<int, float>>{
        {1, 4.0f}, {1, 5.0f}, {2, 4.0f}, {2, 5.0f}, {3, 4.0f}, {3, 5.0f}}));
    CHECK_THAT(
      (make<int, float>({1, 2}, {4.0f, 5.0f, 6.0f})),
      RangeEquals(std::vector<std::tuple<int, float>>{
        {1, 4.0f}, {1, 5.0f}, {1, 6.0f}, {2, 4.0f}, {2, 5.0f}, {2, 6.0f}}));
  }


  SECTION("move-only values")
  {
    auto v = std::vector<std::unique_ptr<int>>{};
    v.push_back(std::make_unique<int>(1));
    v.push_back(std::make_unique<int>(2));

    auto w = std::vector<float>{1.0f};

    CHECK_THAT(
      views::cartesian_product(v, w),
      RangeEquals(std::vector<std::tuple<std::unique_ptr<int>&, float&>>{
        {v[0], w[0]},
        {v[1], w[0]},
      }));
  }
}

} // namespace kdl
