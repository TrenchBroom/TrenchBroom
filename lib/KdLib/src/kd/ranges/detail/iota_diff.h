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

#pragma once

#include <iterator>
#include <type_traits>

namespace kdl
{
namespace detail
{

// recursive helper: pick the first candidate type whose sizeof > sizeof(I)
template <typename I, typename... Candidates>
struct pick_first_wider;

// no candidates left: fallback to signed version of I (width >= I)
template <typename I>
struct pick_first_wider<I>
{
  using type = std::make_signed_t<I>;
};

template <typename I, typename First, typename... Rest>
struct pick_first_wider<I, First, Rest...>
{
  // only consider First if it's an integral signed type
  static constexpr bool first_is_signed_integral =
    std::is_integral_v<First> && std::is_signed_v<First>;

  using type = std::conditional_t<
    first_is_signed_integral && (sizeof(First) > sizeof(I)),
    First,
    typename pick_first_wider<I, Rest...>::type>;
};

} // namespace detail

template <typename I>
struct iota_diff
{
private:
  using iter_diff_t = std::iter_difference_t<I>;

  // Candidate signed integer types in increasing "typical" width order.
  // Adjust or extend the list if your platform has other standard wider types.
  using first_wider_signed_t =
    typename detail::pick_first_wider<I, signed char, short, int, long, long long>::type;

public:
  using type = std::conditional_t<
    // Rule 1: if I is not integral OR iter_difference_t<I> is wider than I
    (!std::is_integral_v<I> || (sizeof(iter_diff_t) > sizeof(I))),
    iter_diff_t,
    // else: choose a signed integer type of width greater than I if possible,
    // otherwise fallback to make_signed_t<I> (signed type of width >= I).
    first_wider_signed_t>;
};

template <typename I>
using iota_diff_t = typename iota_diff<I>::type;

} // namespace kdl
