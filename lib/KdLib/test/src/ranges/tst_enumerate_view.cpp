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

#include "kd/ranges/enumerate_view.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{

TEST_CASE("enumerate")
{
  SECTION("iterator / sentinel")
  {
    SECTION("required types")
    {
      SECTION("const range")
      {
        const auto v = std::vector<int>{1, 2, 3, 4};
        auto e = v | views::enumerate;

        using iterator_type = decltype(e.begin());
        static_assert(
          std::
            is_same_v<iterator_type::iterator_concept, std::random_access_iterator_tag>);
        static_assert(
          std::is_same_v<iterator_type::iterator_category, std::input_iterator_tag>);
        static_assert(std::is_same_v<
                      iterator_type::value_type,
                      std::tuple<iterator_type::difference_type, int>>);

        using sentinel_type = decltype(e.end());
        static_assert(
          std::
            is_same_v<sentinel_type::iterator_concept, std::random_access_iterator_tag>);
        static_assert(
          std::is_same_v<sentinel_type::iterator_category, std::input_iterator_tag>);
        static_assert(std::is_same_v<
                      sentinel_type::value_type,
                      std::tuple<sentinel_type::difference_type, int>>);
      }

      SECTION("non const range")
      {
        auto v = std::vector<int>{1, 2, 3, 4};
        auto e = v | views::enumerate;

        using iterator_type = decltype(e.begin());
        static_assert(
          std::
            is_same_v<iterator_type::iterator_concept, std::random_access_iterator_tag>);
        static_assert(
          std::is_same_v<iterator_type::iterator_category, std::input_iterator_tag>);
        static_assert(std::is_same_v<
                      iterator_type::value_type,
                      std::tuple<iterator_type::difference_type, int>>);

        using sentinel_type = decltype(e.end());
        static_assert(
          std::
            is_same_v<sentinel_type::iterator_concept, std::random_access_iterator_tag>);
        static_assert(
          std::is_same_v<sentinel_type::iterator_category, std::input_iterator_tag>);
        static_assert(std::is_same_v<
                      sentinel_type::value_type,
                      std::tuple<sentinel_type::difference_type, int>>);
      }
    }

    SECTION("base")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto e = v | views::enumerate;

      auto i = e.begin();
      auto s = e.end();

      CHECK(i.base() == v.begin());
      CHECK(s.base() == v.end());
    }

    SECTION("arithmetic")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto e = v | views::enumerate;

      auto it = e.begin();
      REQUIRE(*it == std::tuple{0, 1});

      CHECK(*it++ == std::tuple{0, 1});
      CHECK(*it == std::tuple{1, 2});

      CHECK(*++it == std::tuple{2, 3});
      CHECK(*it == std::tuple{2, 3});

      CHECK(*it-- == std::tuple{2, 3});
      CHECK(*it == std::tuple{1, 2});

      CHECK(*--it == std::tuple{0, 1});
      CHECK(*it == std::tuple{0, 1});

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
      auto e = v | views::enumerate;

      auto it = e.begin();
      CHECK(it[0] == std::tuple{0, 1});
      CHECK(it[1] == std::tuple{1, 2});
      CHECK(it[2] == std::tuple{2, 3});
      CHECK(it[3] == std::tuple{3, 4});

      // Check that the iterator is not invalidated
      CHECK(*it == std::tuple{0, 1});
    }

    SECTION("comparison")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      auto e = v | views::enumerate;

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

  SECTION("input ranges")
  {
    auto ints = std::istringstream{"1 2 3 4"};

    SECTION("as lvalue")
    {
      auto e = std::ranges::istream_view<int>(ints) | views::enumerate;
      static_assert(std::ranges::input_range<decltype(e)>);

      using tuple_type = typename std::ranges::iterator_t<decltype(e)>::value_type;

      CHECK(
        std::ranges::equal(e, std::vector<tuple_type>{{0, 1}, {1, 2}, {2, 3}, {3, 4}}));
    }
  }

  SECTION("bidirectional ranges")
  {
    SECTION("as rvalue")
    {
      using tuple_type = typename std::ranges::iterator_t<
        decltype(std::vector<int>{1, 2, 3, 4} | views::enumerate)>::value_type;

      CHECK(std::ranges::equal(
        std::vector<int>{1, 2, 3, 4} | views::enumerate,
        std::vector<tuple_type>{{0, 1}, {1, 2}, {2, 3}, {3, 4}}));

      CHECK(std::ranges::equal(
        std::vector<int>{} | views::enumerate, std::vector<tuple_type>{}));
    }

    SECTION("as lvalue")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      const auto e = v | views::enumerate;
      static_assert(std::ranges::bidirectional_range<decltype(e)>);

      using tuple_type =
        typename std::ranges::iterator_t<decltype(v | views::enumerate)>::value_type;

      CHECK(
        std::ranges::equal(e, std::vector<tuple_type>{{0, 1}, {1, 2}, {2, 3}, {3, 4}}));
    }
  }

  SECTION("various underlying ranges")
  {
    SECTION("array")
    {
      const int a[] = {1, 2, 3, 4};
      auto e = views::enumerate(a);

      using tuple_type = typename std::ranges::iterator_t<decltype(e)>::value_type;

      CHECK(
        std::ranges::equal(e, std::vector<tuple_type>{{0, 1}, {1, 2}, {2, 3}, {3, 4}}));
    }

    SECTION("nested types")
    {
      const auto v = std::vector<std::map<int, std::string>>{
        {{1, "a"}, {2, "b"}},
        {{3, "c"}},
        {{4, "d"}, {5, "e"}, {6, "f"}},
      };

      auto e = views::enumerate(v);

      using tuple_type = typename std::ranges::iterator_t<decltype(e)>::value_type;

      CHECK(std::ranges::equal(
        e,
        std::vector<tuple_type>{
          {0, {{1, "a"}, {2, "b"}}},
          {1, {{3, "c"}}},
          {2, {{4, "d"}, {5, "e"}, {6, "f"}}},
        }));
    }

    SECTION("initializer list")
    {
      const auto l = std::initializer_list<int>{1, 2, 3, 4};
      auto e = views::enumerate(l);

      using tuple_type = typename std::ranges::iterator_t<decltype(e)>::value_type;

      CHECK(
        std::ranges::equal(e, std::vector<tuple_type>{{0, 1}, {1, 2}, {2, 3}, {3, 4}}));
    }
  }
}

} // namespace kdl
