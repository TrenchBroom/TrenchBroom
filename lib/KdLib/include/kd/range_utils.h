/*
 Copyright 2024 Kristian Duske

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
#include <optional>
#include <ranges>

namespace kdl
{
namespace detail
{

template <typename R>
auto get_begin(const R& range)
{
  return range.begin();
}

template <typename R>
auto get_end(const R& range)
{
  return range.end();
}

template <typename R>
auto get_begin(R&& range)
{
  return std::make_move_iterator(range.begin());
}

template <typename R>
auto get_end(R&& range)
{
  return std::make_move_iterator(range.end());
}

} // namespace detail

/**
 * Finds the smallest index at which the given predicate is satisified in the given
 * range. If the given range does not such a value, an empty optional is
 * returned.
 *
 * @tparam R the type of the range
 * @tparam P the predicate type
 * @param r the range to check
 * @param p the predicate
 * @return the smallest index at which the given predicate is satisfied in the given
 * range or an empty optional if the given range does not contain such a value
 */
template <
  std::ranges::range R,
  typename P,
  typename std::enable_if_t<
    std::is_invocable_r_v<bool, P, const std::ranges::range_value_t<R>&>>* = nullptr>
auto index_of(const R& r, P&& p)
{
  const auto it = std::ranges::find_if(r, std::forward<P>(p));
  const auto dist = std::distance(std::begin(r), it);
  const auto index = static_cast<std::ranges::range_size_t<R>>(dist);
  return it != std::end(r) ? std::optional{index} : std::nullopt;
}

/**
 * Finds the smallest index at which the given value is found in the given range. If
 * the given range does not contain the given value, an empty optional is returned.
 *
 * @tparam R the type of the range
 * @tparam X the value type
 * @param r the range to check
 * @param x the value to find
 * @return the smallest index at which the given value is found in the given range or
 * an empty optional if the given range does not contain such a value
 */
template <std::ranges::range R, typename X>
auto index_of(const R& r, const X& x)
{
  return index_of(r, [&](const auto& e) { return e == x; });
}

template <std::ranges::range R>
auto pred(R&& range, std::ranges::iterator_t<R> iter)
{
  return iter == std::ranges::begin(range) ? std::ranges::prev(std::ranges::end(range))
                                           : std::ranges::prev(iter);
}

template <std::ranges::range R>
auto succ(R&& range, std::ranges::iterator_t<R> iter)
{
  auto next = std::ranges::next(iter);
  return next == std::ranges::end(range) ? std::ranges::begin(range) : next;
}

} // namespace kdl
