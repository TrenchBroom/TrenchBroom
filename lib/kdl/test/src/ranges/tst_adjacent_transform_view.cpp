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

#include "kdl/ranges/adjacent_transform_view.h"
#include "kdl/ranges/to.h"

#include <forward_list>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{

TEST_CASE("adjacent_transform")
{
  const auto f = [](auto x, auto y, auto z) { return x * y * z; };

  SECTION("properties")
  {
    const auto v = std::vector{1, 2, 3, 4};
    auto a = v | views::adjacent_transform<3>(f);
    static_assert(std::ranges::view<decltype(a)>);
  }

  SECTION("iterator / sentinel")
  {
    SECTION("required types")
    {
      SECTION("random access range")
      {
        const auto v = std::vector{1, 2, 3, 4};
        auto a = v | views::adjacent_transform<3>(f);

        static_assert(std::ranges::random_access_range<decltype(a)>);

        using iterator_type = decltype(a.begin());
        static_assert(
          std::
            is_same_v<iterator_type::iterator_concept, std::random_access_iterator_tag>);
        static_assert(
          std::is_same_v<iterator_type::iterator_category, std::input_iterator_tag>);
        static_assert(std::is_same_v<iterator_type::value_type, int>);

        using sentinel_type = decltype(a.end());
        static_assert(
          std::
            is_same_v<sentinel_type::iterator_concept, std::random_access_iterator_tag>);
      }

      SECTION("forward range")
      {
        const auto l = std::forward_list<int>{1, 2, 3, 4};
        auto a = l | views::adjacent_transform<3>(f);

        using iterator_type = decltype(a.begin());
        static_assert(
          std::is_same_v<iterator_type::iterator_concept, std::forward_iterator_tag>);
        static_assert(
          std::is_same_v<iterator_type::iterator_category, std::input_iterator_tag>);
        static_assert(std::is_same_v<iterator_type::value_type, int>);
      }
    }

    SECTION("arithmetic")
    {
      const auto v = std::vector{1, 2, 3, 4, 5};
      auto a = v | views::adjacent_transform<3>(f);

      auto it = a.begin();
      REQUIRE(*it == 6);

      CHECK(*it++ == 6);
      CHECK(*it == 24);

      CHECK(*++it == 60);
      CHECK(*it == 60);

      CHECK(*it-- == 60);
      CHECK(*it == 24);

      CHECK(*--it == 6);
      CHECK(*it == 6);

      CHECK((it + 1) == std::next(a.begin()));
      CHECK((1 + it) == std::next(a.begin()));
      CHECK((it += 1) == std::next(a.begin()));
      CHECK(it == std::next(a.begin()));

      CHECK((it - 1) == a.begin());
      CHECK((it -= 1) == a.begin());
      CHECK(it == a.begin());

      CHECK(*(a.begin() + 2) == 60);
      CHECK(a.begin() + 3 == a.end());
    }

    SECTION("subscript")
    {
      const auto v = std::vector{1, 2, 3, 4, 5};
      auto a = v | views::adjacent_transform<3>(f);

      auto it = a.begin();
      CHECK(it[0] == 6);
      CHECK(it[1] == 24);
      CHECK(it[2] == 60);

      // Check that the iterator is not invalidated
      CHECK(*it == 6);
    }

    SECTION("comparison")
    {
      const auto v = std::vector{1, 2, 3, 4, 5};
      auto a = v | views::adjacent_transform<3>(f);

      auto i = a.begin();
      auto s = a.end();

      CHECK(i == i);
      CHECK(s == s);
      CHECK(i == a.begin());
      CHECK(std::next(i, 3) == s);

      CHECK(i != s);
      CHECK_FALSE(i != i);

      CHECK_FALSE(i < i);
      CHECK(i <= i);
      CHECK_FALSE(i > i);
      CHECK(i >= i);
    }
  }

  SECTION("pairwise_transform")
  {
    const auto g = [](auto x, auto y) { return x * y; };

    const auto v = std::vector{1, 2, 3, 4};
    auto a = v | views::pairwise_transform(g);

    CHECK(a[0] == 2);
    CHECK(a[1] == 6);
    CHECK(a[2] == 12);
  }

  SECTION("examples")
  {
    const auto v = std::vector{1, 2, 3, 4};
    const auto var_sum = [](auto&&... x) { return (x + ...); };

    CHECK(
      (v | views::adjacent_transform<1>(var_sum) | ranges::to<std::vector>())
      == std::vector{1, 2, 3, 4});
    CHECK(
      (v | views::adjacent_transform<2>(var_sum) | ranges::to<std::vector>())
      == std::vector{3, 5, 7});
    CHECK(
      (v | views::adjacent_transform<3>(var_sum) | ranges::to<std::vector>())
      == std::vector{6, 9});
    CHECK(
      (v | views::adjacent_transform<4>(var_sum) | ranges::to<std::vector>())
      == std::vector{10});
    CHECK(
      (v | views::adjacent_transform<5>(var_sum) | ranges::to<std::vector>())
      == std::vector<int>{});
  }
}

} // namespace kdl
