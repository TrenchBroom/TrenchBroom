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

#include "kd/ranges/zip_transform_view.h"

#include <forward_list>
#include <memory>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace kdl
{

TEST_CASE("zip_transform")
{
  using namespace Catch::Matchers;

  const auto prod = [](auto&&... x) { return (x * ...); };

  SECTION("properties")
  {
    const auto v = std::vector{1, 2, 3};
    const auto w = std::vector{4.0, 5.0};
    auto z = views::zip_transform(prod, v, w);
    static_assert(std::ranges::view<decltype(z)>);
  }

  SECTION("iterator / sentinel")
  {
    SECTION("required types")
    {
      SECTION("random access range")
      {
        const auto v = std::vector{1, 2, 3};
        const auto w = std::vector{4.0, 5.0};
        auto z = views::zip_transform(prod, v, w);

        static_assert(std::ranges::random_access_range<decltype(z)>);

        using iterator_type = decltype(z.begin());
        static_assert(
          std::
            is_same_v<iterator_type::iterator_concept, std::random_access_iterator_tag>);
        static_assert(
          std::is_same_v<iterator_type::iterator_category, std::input_iterator_tag>);
        static_assert(std::is_same_v<iterator_type::value_type, double>);
      }

      SECTION("forward range")
      {
        const auto l = std::forward_list<int>{1, 2, 3};
        const auto w = std::vector{4.0, 5.0};
        auto z = views::zip_transform(prod, l, w);

        using iterator_type = decltype(z.begin());
        static_assert(
          std::is_same_v<iterator_type::iterator_concept, std::forward_iterator_tag>);
        static_assert(
          std::is_same_v<iterator_type::iterator_category, std::input_iterator_tag>);
        static_assert(std::is_same_v<iterator_type::value_type, double>);
      }
    }

    SECTION("arithmetic")
    {
      const auto v = std::vector{1, 2, 3};
      const auto w = std::vector{4.0, 5.0, 6.0, 7.0};
      auto z = views::zip_transform(prod, v, w);

      auto it = z.begin();
      REQUIRE(*it == 4.0);

      CHECK(*it++ == 4.0);
      CHECK(*it == 10.0);

      CHECK(*++it == 18.0);
      CHECK(*it == 18.0);

      CHECK(*it-- == 18.0);
      CHECK(*it == 10.0);

      CHECK(*--it == 4.0);
      CHECK(*it == 4.0);

      CHECK((it + 1) == std::next(z.begin()));
      CHECK((1 + it) == std::next(z.begin()));
      CHECK((it += 1) == std::next(z.begin()));
      CHECK(it == std::next(z.begin()));

      CHECK((it - 1) == z.begin());
      CHECK((it -= 1) == z.begin());
      CHECK(it == z.begin());

      CHECK(*(z.begin() + 2) == 18.0);
      CHECK(z.begin() + 3 == z.end());
    }

    SECTION("subscript")
    {
      const auto v = std::vector{1, 2, 3};
      const auto w = std::vector{4.0, 5.0, 6.0, 7.0};
      auto z = views::zip_transform(prod, v, w);

      auto it = z.begin();
      CHECK(it[0] == 4.0);
      CHECK(it[1] == 10.0);
      CHECK(it[2] == 18.0);

      // Check that the iterator is not invalidated
      CHECK(*it == 4.0);
    }

    SECTION("comparison")
    {
      const auto v = std::vector{1, 2, 3};
      const auto w = std::vector{4.0, 5.0, 6.0, 7.0};
      auto z = views::zip_transform(prod, v, w);

      auto i = z.begin();
      auto s = z.end();

      CHECK(i == i);
      CHECK(i == z.begin());
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
    const auto v = std::vector{1, 2, 3};
    const auto w = std::vector{4.0, 5.0, 6.0, 7.0};
    auto z = views::zip_transform(prod, v, w);

    CHECK_THAT(z, RangeEquals(std::vector<double>{4.0, 10.0, 18.0}));
  }


  SECTION("move-only value types")
  {
    const auto prod_deref = [](auto&&... x) { return (*x * ...); };

    auto lhs = std::vector<std::unique_ptr<int>>{};
    lhs.push_back(std::make_unique<int>(2));
    lhs.push_back(std::make_unique<int>(3));

    auto rhs = std::vector<std::unique_ptr<int>>{};
    rhs.push_back(std::make_unique<int>(4));
    rhs.push_back(std::make_unique<int>(5));

    auto z = views::zip_transform(prod_deref, lhs, rhs);

    CHECK_THAT(z, RangeEquals(std::vector<int>{8, 15}));
  }
}

} // namespace kdl
