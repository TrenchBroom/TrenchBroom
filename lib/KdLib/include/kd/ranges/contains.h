/*
 Copyright (C) 2026 Kristian Duske

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

#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <utility>

// This file can only be used with C++20 or later.
static_assert(__cplusplus >= 202002L);

namespace kdl
{
namespace ranges
{
namespace detail
{

// see https://en.cppreference.com/w/cpp/algorithm/ranges/contains.html
struct contains_fn
{
  template <
    std::input_iterator I,
    std::sentinel_for<I> S,
    typename T,
    typename Proj = std::identity>
    requires std::
      indirect_binary_predicate<std::ranges::equal_to, std::projected<I, Proj>, const T*>
    constexpr bool operator()(I first, S last, const T& value, Proj proj = {}) const
  {
    return std::ranges::find(std::move(first), last, value, std::ref(proj)) != last;
  }

  template <std::ranges::input_range R, typename T, typename Proj = std::identity>
    requires std::indirect_binary_predicate<
      std::ranges::equal_to,
      std::projected<std::ranges::iterator_t<R>, Proj>,
      const T*>
  constexpr bool operator()(R&& r, const T& value, Proj proj = {}) const
  {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), value, std::ref(proj));
  }
};

// see https://en.cppreference.com/w/cpp/algorithm/ranges/contains.html
struct contains_subrange_fn
{
  template <
    std::forward_iterator I1,
    std::sentinel_for<I1> S1,
    std::forward_iterator I2,
    std::sentinel_for<I2> S2,
    typename Pred = std::ranges::equal_to,
    typename Proj1 = std::identity,
    typename Proj2 = std::identity>
    requires std::indirectly_comparable<I1, I2, Pred, Proj1, Proj2>
  constexpr bool operator()(
    I1 first1,
    S1 last1,
    I2 first2,
    S2 last2,
    Pred pred = {},
    Proj1 proj1 = {},
    Proj2 proj2 = {}) const
  {
    // An empty range is a subrange of every range.
    return first2 == last2
           || !std::ranges::search(
                 std::move(first1),
                 std::move(last1),
                 std::move(first2),
                 std::move(last2),
                 std::ref(pred),
                 std::ref(proj1),
                 std::ref(proj2))
                 .empty();
  }

  template <
    std::ranges::forward_range R1,
    std::ranges::forward_range R2,
    typename Pred = std::ranges::equal_to,
    typename Proj1 = std::identity,
    typename Proj2 = std::identity>
    requires std::indirectly_comparable<
      std::ranges::iterator_t<R1>,
      std::ranges::iterator_t<R2>,
      Pred,
      Proj1,
      Proj2>
  constexpr bool operator()(
    R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
  {
    return (*this)(
      std::ranges::begin(r1),
      std::ranges::end(r1),
      std::ranges::begin(r2),
      std::ranges::end(r2),
      std::ref(pred),
      std::ref(proj1),
      std::ref(proj2));
  }
};

} // namespace detail

/**
 * Checks whether the given range contains the given value. A backport of C++23's
 * std::ranges::contains.
 */
inline constexpr auto contains = detail::contains_fn{};

/**
 * Checks whether the second range is a subrange of the first range. A backport of C++23's
 * std::ranges::contains_subrange.
 */
inline constexpr auto contains_subrange = detail::contains_subrange_fn{};

} // namespace ranges
} // namespace kdl
