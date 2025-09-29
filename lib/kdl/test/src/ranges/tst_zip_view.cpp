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

#include "kdl/ranges/zip_view.h"

#include <algorithm>
#include <array>
#include <map>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>


namespace kdl
{

TEST_CASE("zip")
{
  SECTION("iterator / sentinel")
  {
    SECTION("required types")
    {
      auto i = std::istringstream{"4 3 2 1"};
      auto iv = std::ranges::istream_view<int>(i);

      const auto v = std::vector<int>{1, 2, 3, 4};
      const auto w = std::array<float, 3>{5.0f, 6.0f, 7.0f};

      auto zf = views::zip(iv, v);
      using iter_type_zf = decltype(zf.begin());

      static_assert(
        std::is_same_v<iter_type_zf::iterator_concept, std::input_iterator_tag>);

      const auto zr = views::zip(v, w);
      using iter_type_zr = decltype(zr.begin());

      static_assert(
        std::is_same_v<iter_type_zr::iterator_concept, std::random_access_iterator_tag>);
      static_assert(
        std::is_same_v<iter_type_zr::iterator_category, std::input_iterator_tag>);
      static_assert(std::is_same_v<iter_type_zr::value_type, std::tuple<int, float>>);

      static_assert(std::random_access_iterator<std::ranges::iterator_t<decltype(zr)>>);
    }

    SECTION("arithmetic")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      const auto w = std::array<float, 3>{5.0f, 6.0f, 7.0f};

      const auto z = views::zip(v, w);

      auto it = z.begin();
      REQUIRE(*it == std::tuple{1, 5.0f});

      CHECK(*it++ == std::tuple{1, 5.0f});
      CHECK(*it == std::tuple{2, 6.0f});

      CHECK(*++it == std::tuple{3, 7.0f});
      CHECK(*it == std::tuple{3, 7.0f});

      CHECK(std::next(it) == z.end());

      CHECK(*it-- == std::tuple{3, 7.0f});
      CHECK(*it == std::tuple{2, 6.0f});

      CHECK(*--it == std::tuple{1, 5.0f});
      CHECK(*it == std::tuple{1, 5.0f});

      CHECK((it + 1) == std::next(z.begin()));
      CHECK((1 + it) == std::next(z.begin()));
      CHECK((it += 1) == std::next(z.begin()));
      CHECK(it == std::next(z.begin()));

      CHECK((it - 1) == z.begin());
      CHECK((it -= 1) == z.begin());
      CHECK(it == z.begin());

      CHECK(std::next(z.begin()) - z.begin() == 1);
      CHECK(std::next(z.begin(), 2) - z.begin() == 2);
      CHECK(z.begin() - std::next(z.begin()) == -1);

      CHECK(std::next(z.begin()) - z.begin() == 1);
      CHECK(std::next(z.begin(), 2) - z.begin() == 2);
      CHECK(z.end() - z.begin() == 3);
      CHECK(z.begin() - z.end() == -3);
    }

    SECTION("subscript")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      const auto w = std::array<float, 3>{5.0f, 6.0f, 7.0f};

      const auto z = views::zip(v, w);

      auto it = z.begin();
      CHECK(it[0] == std::tuple{1, 5.0f});
      CHECK(it[1] == std::tuple{2, 6.0f});
      CHECK(it[2] == std::tuple{3, 7.0f});

      // Check that the iterator is not invalidated
      CHECK(*it == std::tuple{1, 5.0f});
    }

    SECTION("comparison")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      const auto w = std::array<float, 3>{5.0f, 6.0f, 7.0f};

      const auto z = views::zip(v, w);

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

  SECTION("size")
  {
    SECTION("with non-empty sized ranges")
    {
      const auto v = std::vector<int>{1, 2, 3, 4};
      const auto w = std::array<float, 3>{5.0f, 6.0f, 7.0f};

      const auto z = views::zip(v, w);
      CHECK(z.size() == 3);
    }

    SECTION("with empty sized ranges")
    {
      const auto v = std::vector<int>{};
      const auto w = std::array<float, 3>{5.0f, 6.0f, 7.0f};

      const auto z = views::zip(v, w);
      CHECK(z.size() == 0);
    }
  }

  SECTION("input ranges")
  {
    auto s = std::istringstream{"1 2 3 4"};
    auto v = std::ranges::istream_view<int>(s);
    auto w = std::vector{5, 6, 7, 8};

    SECTION("as lvalue")
    {
      auto z = views::zip(v, w);
      static_assert(std::ranges::input_range<decltype(z)>);

      CHECK(std::ranges::equal(
        z, std::vector<std::tuple<int, int>>{{1, 5}, {2, 6}, {3, 7}, {4, 8}}));
    }
  }

  SECTION("bidirectional ranges")
  {
    SECTION("as rvalue")
    {
      CHECK(std::ranges::equal(
        views::zip(std::vector<int>{1, 2}, std::vector<int>{3, 4}),
        std::vector<std::tuple<int, int>>{{1, 3}, {2, 4}}));

      CHECK(std::ranges::equal(
        views::zip(std::vector<int>{}, std::vector<int>{}),
        std::vector<std::tuple<int, int>>{}));
    }

    SECTION("as lvalue")
    {
      const auto v = std::vector<int>{1, 2};
      const auto w = std::vector<int>{3, 4};
      const auto z = views::zip(v, w);

      static_assert(std::ranges::bidirectional_range<decltype(z)>);

      CHECK(std::ranges::equal(z, std::vector<std::tuple<int, int>>{{1, 3}, {2, 4}}));
    }
  }

  SECTION("various underlying ranges")
  {
    SECTION("array")
    {
      const int a[] = {1, 2};
      const auto v = std::vector<int>{3, 4};
      auto z = views::zip(a, v);
      CHECK(std::ranges::equal(z, std::vector<std::tuple<int, int>>{{1, 3}, {2, 4}}));
    }

    SECTION("map types")
    {
      const auto v = std::vector<int>{1, 2};
      const auto m = std::map<int, std::string>{{3, "three"}, {4, "four"}};
      const auto z = views::zip(v, m);

      CHECK(std::ranges::equal(
        z,
        std::vector<std::tuple<int, std::pair<const int, std::string>>>{
          {1, {3, "three"}},
          {2, {4, "four"}},
        }));
    }

    SECTION("initializer list")
    {
      const auto v = std::vector<int>{1, 2};
      const auto l = std::initializer_list<int>{3, 4};
      const auto z = views::zip(v, l);
      CHECK(std::ranges::equal(z, std::vector<std::tuple<int, int>>{{1, 3}, {2, 4}}));
    }
  }
}

} // namespace kdl
