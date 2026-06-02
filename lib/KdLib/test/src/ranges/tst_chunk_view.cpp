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

#include "kd/ranges/chunk_view.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <sstream>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{

template <typename T>
auto make_chunked(std::vector<T> v, const int n)
{
  return ranges::chunk_view{std::move(v), n};
}

// Input-only view (move-only iterator) over a std::vector that exposes a sized
// sentinel. Used to force instantiation of chunk_view's input-range operator-
// overloads, whose bodies are only checked when the sized_sentinel_for
// constraint is satisfied.
template <typename T>
class sized_input_view : public std::ranges::view_interface<sized_input_view<T>>
{
  using base_iter = typename std::vector<T>::iterator;

public:
  class iterator
  {
  public:
    using iterator_concept = std::input_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;

    iterator() = default;
    explicit iterator(base_iter it)
      : it_{it}
    {
    }

    iterator(iterator&&) = default;
    iterator& operator=(iterator&&) = default;

    T& operator*() const { return *it_; }

    iterator& operator++()
    {
      ++it_;
      return *this;
    }
    void operator++(int) { ++it_; }

    base_iter base() const { return it_; }

  private:
    base_iter it_{};
  };

  class sentinel
  {
  public:
    sentinel() = default;
    explicit sentinel(base_iter end)
      : end_{end}
    {
    }

    friend bool operator==(const sentinel& s, const iterator& i)
    {
      return s.end_ == i.base();
    }

    friend std::ptrdiff_t operator-(const sentinel& s, const iterator& i)
    {
      return s.end_ - i.base();
    }

    friend std::ptrdiff_t operator-(const iterator& i, const sentinel& s)
    {
      return i.base() - s.end_;
    }

  private:
    base_iter end_{};
  };

  sized_input_view() = default;
  explicit sized_input_view(std::vector<T>& v)
    : v_{&v}
  {
  }

  iterator begin() { return iterator{v_->begin()}; }
  sentinel end() { return sentinel{v_->end()}; }

private:
  std::vector<T>* v_{};
};

} // namespace

TEST_CASE("chunk")
{
  SECTION("iterator / sentinel")
  {
    SECTION("required types (input range)")
    {
      auto i = std::istringstream{"5 4 3 2 1"};
      auto iv = std::ranges::istream_view<int>(i);
      auto c = views::chunk(iv, 3);

      using outer_iter_type = decltype(c.begin());

      static_assert(
        std::is_same_v<outer_iter_type::iterator_concept, std::input_iterator_tag>);

      using inner_iter_type = decltype((*c.begin()).begin());
      static_assert(
        std::is_same_v<inner_iter_type::iterator_concept, std::input_iterator_tag>);
      static_assert(std::is_same_v<inner_iter_type::value_type, int>);
    }

    SECTION("required types (random access range)")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8};
      auto c = v | views::chunk(3);

      using outer_iter_type = decltype(c.begin());

      static_assert(
        std::
          is_same_v<outer_iter_type::iterator_concept, std::random_access_iterator_tag>);

      using inner_iter_type = decltype((*c.begin()).begin());
      static_assert(
        std::is_same_v<inner_iter_type::iterator_concept, std::contiguous_iterator_tag>);
      static_assert(std::is_same_v<inner_iter_type::value_type, int>);
    }

    SECTION("arithmetic (input range)")
    {
      auto i = std::istringstream{"1 2 3 4 5"};
      auto iv = std::ranges::istream_view<int>(i);
      auto c = views::chunk(iv, 2);

      auto it = c.begin();
      REQUIRE(std::ranges::equal(*it, std::vector<int>{1, 2}));

      it++;
      CHECK(std::ranges::equal(*it, std::vector<int>{3, 4}));

      CHECK(std::ranges::equal(*++it, std::vector<int>{5}));
      CHECK(it == c.end());
    }

    SECTION("difference (input range with sized sentinel)")
    {
      auto v = std::vector<int>{1, 2, 3, 4, 5};
      auto c = ranges::chunk_view{sized_input_view<int>{v}, 2};

      using view_type = decltype(c);
      static_assert(std::sized_sentinel_for<
                    std::ranges::sentinel_t<sized_input_view<int>>,
                    std::ranges::iterator_t<sized_input_view<int>>>);
      static_assert(
        std::is_same_v<decltype(c.begin())::iterator_concept, std::input_iterator_tag>);
      static_assert(!std::ranges::forward_range<view_type>);

      CHECK((std::default_sentinel - c.begin()) == 3);
      CHECK((c.begin() - std::default_sentinel) == -3);
    }

    SECTION("arithmetic (random access range)")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5};
      auto c = v | views::chunk(2);

      auto it = c.begin();
      CHECK_FALSE(it == c.end());
      REQUIRE(std::ranges::equal(*it, std::vector<int>{1, 2}));

      CHECK(std::ranges::equal(*it++, std::vector<int>{1, 2}));
      CHECK(std::ranges::equal(*it, std::vector<int>{3, 4}));
      CHECK_FALSE(it == c.end());

      CHECK(std::ranges::equal(*++it, std::vector<int>{5}));
      CHECK(std::ranges::equal(*it, std::vector<int>{5}));
      CHECK_FALSE(it == c.end());

      CHECK(std::next(it) == c.end());

      CHECK(std::ranges::equal(*it--, std::vector<int>{5}));
      CHECK(std::ranges::equal(*it, std::vector<int>{3, 4}));

      CHECK(std::ranges::equal(*--it, std::vector<int>{1, 2}));
      CHECK(std::ranges::equal(*it, std::vector<int>{1, 2}));

      CHECK((it + 1) == std::next(c.begin()));
      CHECK((1 + it) == std::next(c.begin()));
      CHECK((it += 1) == std::next(c.begin()));
      CHECK(it == std::next(c.begin()));

      CHECK((it - 1) == c.begin());
      CHECK((it -= 1) == c.begin());
      CHECK(it == c.begin());

      CHECK(std::next(c.begin()) - c.begin() == 1);
      CHECK(std::next(c.begin(), 2) - c.begin() == 2);
      CHECK(c.begin() - std::next(c.begin()) == -1);

      CHECK(std::next(c.begin()) - c.begin() == 1);
      CHECK(std::next(c.begin(), 2) - c.begin() == 2);
      CHECK(c.end() - c.begin() == 3);
      CHECK(c.begin() - c.end() == -3);
    }

    SECTION("subscript (random access range)")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5};
      auto c = v | views::chunk(2);

      auto it = c.begin();
      CHECK(std::ranges::equal(it[0], std::vector<int>{1, 2}));
      CHECK(std::ranges::equal(it[1], std::vector<int>{3, 4}));
      CHECK(std::ranges::equal(it[2], std::vector<int>{5}));

      // Check that the iterator is not invalidated
      CHECK(std::ranges::equal(*it, std::vector<int>{1, 2}));
    }

    SECTION("comparison (random access range)")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5};
      auto c = v | views::chunk(2);

      auto i = c.begin();
      auto s = c.end();

      CHECK(i == i);
      CHECK(i == c.begin());
      CHECK(std::next(i, 3) == s);

      CHECK(i != s);
      CHECK_FALSE(i != i);

      CHECK_FALSE(i < i);
      CHECK(i <= i);
      CHECK_FALSE(i > i);
      CHECK(i >= i);
    }
  }

  SECTION("size (random access range)")
  {
    SECTION("with non-empty range")
    {
      const auto v = std::vector<int>{1, 2, 3, 4, 5};
      const auto c = v | views::chunk(2);
      CHECK(c.size() == 3);
    }

    SECTION("with range having one incomplete element")
    {
      const auto v = std::vector<int>{1};
      const auto c = v | views::chunk(2);
      CHECK(c.size() == 1);
    }

    SECTION("with empty range")
    {
      const auto v = std::vector<int>{};
      const auto c = v | views::chunk(2);
      CHECK(c.size() == 0);
    }
  }

  SECTION("move-only value types")
  {
    using move_only = std::unique_ptr<int>;

    auto v = std::vector<move_only>{};
    v.push_back(std::make_unique<int>(1));
    v.push_back(std::make_unique<int>(2));

    auto c = v | views::chunk(2);

    CHECK(recursive_ranges_equal(
      c,
      std::vector{
        std::ranges::subrange{v.begin(), v.end()},
      }));
  }

  SECTION("examples")
  {
    CHECK(
      recursive_ranges_equal(make_chunked<int>({}, 2), std::vector<std::vector<int>>{}));
    CHECK(recursive_ranges_equal(
      make_chunked<int>({1}, 2), std::vector<std::vector<int>>{{1}}));
    CHECK(recursive_ranges_equal(
      make_chunked<int>({1, 2}, 2), std::vector<std::vector<int>>{{1, 2}}));
    CHECK(recursive_ranges_equal(
      make_chunked<int>({1, 2, 3}, 2), std::vector<std::vector<int>>{{1, 2}, {3}}));
    CHECK(recursive_ranges_equal(
      make_chunked<int>({1, 2, 3, 4}, 2), std::vector<std::vector<int>>{{1, 2}, {3, 4}}));
    CHECK(recursive_ranges_equal(
      make_chunked<int>({1, 2, 3, 4}, 3), std::vector<std::vector<int>>{{1, 2, 3}, {4}}));
    CHECK(recursive_ranges_equal(
      make_chunked<int>({1, 2, 3}, 1), std::vector<std::vector<int>>{{1}, {2}, {3}}));
  }
}

} // namespace kdl
