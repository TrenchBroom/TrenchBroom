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

#include "kdl/ranges/repeat_view.h"

#include <algorithm>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{

TEST_CASE("repeat")
{
  SECTION("iterator / sentinel")
  {
    SECTION("required types (bounded range)")
    {
      auto r = views::repeat(2, 5);

      static_assert(std::ranges::random_access_range<decltype(r)>);

      using iterator_type = decltype(r.begin());
      static_assert(
        std::is_same_v<iterator_type::iterator_concept, std::random_access_iterator_tag>);
      static_assert(
        std::
          is_same_v<iterator_type::iterator_category, std::random_access_iterator_tag>);

      using sentinel_type = decltype(r.end());
      static_assert(
        std::is_same_v<sentinel_type::iterator_concept, std::random_access_iterator_tag>);
    }

    SECTION("required types (unbounded range)")
    {
      auto r = views::repeat(2);

      static_assert(std::ranges::random_access_range<decltype(r)>);

      using iterator_type = decltype(r.begin());
      static_assert(
        std::is_same_v<iterator_type::iterator_concept, std::random_access_iterator_tag>);
      static_assert(
        std::
          is_same_v<iterator_type::iterator_category, std::random_access_iterator_tag>);
    }

    SECTION("arithmetic (bounded range)")
    {
      auto r = views::repeat(2, 3);

      auto it = r.begin();
      CHECK(*it == 2);

      CHECK(*it++ == 2);
      CHECK(*it == 2);

      CHECK(*++it == 2);
      CHECK(*it == 2);

      CHECK(std::next(it) == r.end());

      CHECK(*it-- == 2);
      CHECK(*it == 2);

      CHECK(*--it == 2);
      CHECK(*it == 2);

      CHECK((it + 1) == std::next(r.begin()));
      CHECK((1 + it) == std::next(r.begin()));
      CHECK((it += 1) == std::next(r.begin()));
      CHECK(it == std::next(r.begin()));

      CHECK((it - 1) == r.begin());
      CHECK((it -= 1) == r.begin());
      CHECK(it == r.begin());

      CHECK(*(r.begin() + 2) == 2);
      CHECK(r.begin() + 3 == r.end());
    }

    SECTION("arithmetic (unbounded range)")
    {
      auto r = views::repeat(2);

      auto it = r.begin();
      CHECK(*it == 2);

      CHECK(*it++ == 2);
      CHECK(*it == 2);

      CHECK(*++it == 2);
      CHECK(*it == 2);

      CHECK(*it-- == 2);
      CHECK(*it == 2);

      CHECK(*--it == 2);
      CHECK(*it == 2);

      CHECK((it + 1) == std::next(r.begin()));
      CHECK((1 + it) == std::next(r.begin()));
      CHECK((it += 1) == std::next(r.begin()));
      CHECK(it == std::next(r.begin()));

      CHECK((it - 1) == r.begin());
      CHECK((it -= 1) == r.begin());
      CHECK(it == r.begin());

      CHECK(*(r.begin() + 2) == 2);
    }

    SECTION("subscript")
    {
      auto r = views::repeat(2);

      auto it = r.begin();
      CHECK(it[0] == 2);
      CHECK(it[1] == 2);
      CHECK(it[2] == 2);
    }

    SECTION("comparison (unbounded range)")
    {
      auto r = views::repeat(2);

      auto i = r.begin();
      auto s = r.end();

      CHECK(i == i);
      CHECK(i == r.begin());

      CHECK(i != s);
      CHECK_FALSE(i != i);

      CHECK_FALSE(i < i);
      CHECK(i <= i);
      CHECK_FALSE(i > i);
      CHECK(i >= i);
    }

    SECTION("comparison (bounded range)")
    {
      auto r = views::repeat(2, 3);

      auto i = r.begin();
      auto s = r.end();

      CHECK(i == i);
      CHECK(s == s);
      CHECK(i == r.begin());
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
    CHECK(std::ranges::equal(views::repeat(2, 0), std::vector<int>{}));
    CHECK(std::ranges::equal(views::repeat(2, 1), std::vector<int>{2}));
    CHECK(std::ranges::equal(views::repeat(2, 3), std::vector<int>{2, 2, 2}));
  }
}

} // namespace kdl
