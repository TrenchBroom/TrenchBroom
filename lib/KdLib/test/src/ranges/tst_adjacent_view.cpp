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

#include "kd/ranges/adjacent_view.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{

TEST_CASE("adjacent")
{
  SECTION("properties")
  {
    const auto v = std::vector<int>{1, 2, 3, 4};
    auto a = v | views::adjacent<3>;
    static_assert(std::ranges::view<decltype(a)>);
  }

  SECTION("iterator / sentinel")
  {
    SECTION("required types")
    {
      SECTION("const range")
      {
        const auto v = std::vector<int>{1, 2, 3, 4};
        auto a = v | views::adjacent<3>;

        static_assert(std::ranges::random_access_range<decltype(a)>);

        using iterator_type = decltype(a.begin());
        static_assert(
          std::
            is_same_v<iterator_type::iterator_concept, std::random_access_iterator_tag>);
        static_assert(
          std::is_same_v<iterator_type::iterator_category, std::input_iterator_tag>);
        static_assert(
          std::is_same_v<iterator_type::value_type, std::tuple<int, int, int>>);

        using sentinel_type = decltype(a.end());
        static_assert(
          std::
            is_same_v<sentinel_type::iterator_concept, std::random_access_iterator_tag>);
      }

      SECTION("non const range")
      {
        auto v = std::vector<int>{1, 2, 3, 4};
        auto a = v | views::adjacent<3>;

        using iterator_type = decltype(a.begin());
        static_assert(
          std::
            is_same_v<iterator_type::iterator_concept, std::random_access_iterator_tag>);
        static_assert(
          std::is_same_v<iterator_type::iterator_category, std::input_iterator_tag>);
        static_assert(
          std::is_same_v<iterator_type::value_type, std::tuple<int, int, int>>);
      }
    }

    SECTION("arithmetic")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto a = v | views::adjacent<2>;

      auto it = a.begin();
      REQUIRE(*it == std::tuple{1, 2});

      CHECK(*it++ == std::tuple{1, 2});
      CHECK(*it == std::tuple{2, 3});

      CHECK(*++it == std::tuple{3, 4});
      CHECK(*it == std::tuple{3, 4});

      CHECK(*it-- == std::tuple{3, 4});
      CHECK(*it == std::tuple{2, 3});

      CHECK(*--it == std::tuple{1, 2});
      CHECK(*it == std::tuple{1, 2});

      CHECK((it + 1) == std::next(a.begin()));
      CHECK((1 + it) == std::next(a.begin()));
      CHECK((it += 1) == std::next(a.begin()));
      CHECK(it == std::next(a.begin()));

      CHECK((it - 1) == a.begin());
      CHECK((it -= 1) == a.begin());
      CHECK(it == a.begin());

      CHECK(*(a.begin() + 2) == std::tuple{3, 4});
      CHECK(a.begin() + 3 == a.end());
    }

    SECTION("subscript")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto a = v | views::adjacent<2>;

      auto it = a.begin();
      CHECK(it[0] == std::tuple{1, 2});
      CHECK(it[1] == std::tuple{2, 3});
      CHECK(it[2] == std::tuple{3, 4});

      // Check that the iterator is not invalidated
      CHECK(*it == std::tuple{1, 2});
    }

    SECTION("comparison")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto a = v | views::adjacent<2>;

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

  SECTION("pairwise")
  {
    const auto v = std::vector<int>{1, 2, 3, 4};
    auto a = v | views::pairwise;

    CHECK(a[0] == std::tuple{1, 2});
    CHECK(a[1] == std::tuple{2, 3});
    CHECK(a[2] == std::tuple{3, 4});
  }
}

} // namespace kdl
