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

#include "kd/ranges/stride_view.h"

#include <algorithm>
#include <sstream>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{

template <typename T>
auto make_stride(std::vector<T> v, const int n)
{
  return ranges::stride_view{std::move(v), n};
}

} // namespace

TEST_CASE("stride")
{
  SECTION("iterator / sentinel")
  {
    SECTION("required types (random access range)")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5, 6, 7};
      auto s = v | views::stride(2);

      static_assert(std::ranges::random_access_range<decltype(s)>);

      using iterator_type = decltype(s.begin());
      static_assert(
        std::is_same_v<iterator_type::iterator_concept, std::random_access_iterator_tag>);
      static_assert(
        std::
          is_same_v<iterator_type::iterator_category, std::random_access_iterator_tag>);

      using sentinel_type = decltype(s.end());
      static_assert(
        std::is_same_v<sentinel_type::iterator_concept, std::random_access_iterator_tag>);
    }

    SECTION("required types (input range)")
    {
      auto i = std::istringstream{"5 4 3 2 1"};
      auto iv = std::ranges::istream_view<int>(i);
      auto s = iv | views::stride(3);

      using iterator_type = decltype(s.begin());
      static_assert(
        std::is_same_v<iterator_type::iterator_concept, std::input_iterator_tag>);
    }

    SECTION("arithmetic")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8};
      auto s = v | views::stride(3);

      auto it = s.begin();
      CHECK(*it == 1);

      CHECK(*it++ == 1);
      CHECK(*it == 4);

      CHECK(*++it == 7);
      CHECK(*it == 7);

      CHECK(std::next(it) == s.end());

      CHECK(*it-- == 7);
      CHECK(*it == 4);

      CHECK(*--it == 1);
      CHECK(*it == 1);

      CHECK((it + 1) == std::next(s.begin()));
      CHECK((1 + it) == std::next(s.begin()));
      CHECK((it += 1) == std::next(s.begin()));
      CHECK(it == std::next(s.begin()));

      CHECK((it - 1) == s.begin());
      CHECK((it -= 1) == s.begin());
      CHECK(it == s.begin());

      CHECK(*(s.begin() + 2) == 7);
      CHECK(s.begin() + 3 == s.end());
    }

    SECTION("subscript")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8};
      auto s = v | views::stride(3);

      auto it = s.begin();
      CHECK(it[0] == 1);
      CHECK(it[1] == 4);
      CHECK(it[2] == 7);

      // Check that the iterator is not invalidated
      CHECK(*it == 1);
    }

    SECTION("comparison")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8};
      auto x = v | views::stride(3);

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
    CHECK(std::ranges::equal(make_stride<int>({}, 2), std::vector<int>{}));
    CHECK(std::ranges::equal(make_stride<int>({1}, 2), std::vector<int>{1}));
    CHECK(std::ranges::equal(make_stride<int>({1, 2}, 2), std::vector<int>{1}));
    CHECK(std::ranges::equal(make_stride<int>({1, 2, 3}, 2), std::vector<int>{1, 3}));
    CHECK(std::ranges::equal(make_stride<int>({1, 2, 3, 4}, 2), std::vector<int>{1, 3}));
    CHECK(std::ranges::equal(make_stride<int>({1, 2, 3, 4}, 3), std::vector<int>{1, 4}));
    CHECK(std::ranges::equal(make_stride<int>({1, 2, 3}, 1), std::vector<int>{1, 2, 3}));
  }
}

} // namespace kdl
