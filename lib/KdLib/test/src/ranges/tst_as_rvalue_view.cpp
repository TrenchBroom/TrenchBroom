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

#include "kd/ranges/as_rvalue_view.h"

#include <sstream>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{

TEST_CASE("as_rvalue")
{
  SECTION("iterator / sentinel")
  {
    SECTION("required types")
    {
      SECTION("random access range")
      {
        auto v = std::vector<int>{1, 2, 3, 4};
        auto e = v | views::as_rvalue;

        using iterator_type = decltype(e.begin());
        static_assert(
          std::
            is_same_v<iterator_type::iterator_category, std::random_access_iterator_tag>);
        static_assert(std::is_same_v<iterator_type::value_type, int>);
        static_assert(std::is_same_v<iterator_type::reference, int&&>);
      }

      SECTION("input range")
      {
        auto i = std::istringstream{"1 2 3 4"};
        auto iv = std::ranges::istream_view<int>(i) | views::as_rvalue;
        auto e = iv | views::as_rvalue;

        using iterator_type = decltype(e.begin());
        static_assert(
          std::is_same_v<iterator_type::iterator_concept, std::input_iterator_tag>);
        static_assert(std::is_same_v<iterator_type::value_type, int>);
        static_assert(std::is_same_v<iterator_type::reference, int&&>);
      }
    }

    SECTION("arithmetic")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto e = v | views::as_rvalue;

      auto it = e.begin();
      REQUIRE(*it == 1);

      CHECK(*it++ == 1);
      CHECK(*it == 2);

      CHECK(*++it == 3);
      CHECK(*it == 3);

      CHECK(*it-- == 3);
      CHECK(*it == 2);

      CHECK(*--it == 1);
      CHECK(*it == 1);

      CHECK((it + 1) == std::next(e.begin()));
      CHECK((1 + it) == std::next(e.begin()));
      CHECK((it += 1) == std::next(e.begin()));
      CHECK(it == std::next(e.begin()));

      CHECK((it - 1) == e.begin());
      CHECK((it -= 1) == e.begin());
      CHECK(it == e.begin());
    }

    SECTION("subscript")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto e = v | views::as_rvalue;

      auto it = e.begin();
      CHECK(it[0] == 1);
      CHECK(it[1] == 2);
      CHECK(it[2] == 3);
      CHECK(it[3] == 4);

      // Check that the iterator is not invalidated
      CHECK(*it == 1);
    }

    SECTION("comparison")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto e = v | views::as_rvalue;

      auto i = e.begin();
      auto s = e.end();

      CHECK(i == i);
      CHECK(s == s);
      CHECK(i == e.begin());
      CHECK(std::next(i, 4) == s);

      CHECK(i != s);
      CHECK_FALSE(i != i);

      CHECK_FALSE(i < i);
      CHECK(i <= i);
      CHECK_FALSE(i > i);
      CHECK(i >= i);
    }
  }
}

} // namespace kdl
