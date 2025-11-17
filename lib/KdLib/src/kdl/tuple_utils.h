/*
 Copyright 2021 Kristian Duske

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

#include <cstddef>
#include <tuple>

namespace kdl
{
namespace detail
{

template <std::size_t Offset, std::size_t... I>
constexpr auto offset_index_sequence(std::index_sequence<I...>)
  -> std::index_sequence<(Offset + I)...>;

template <std::size_t Start, std::size_t Count>
using make_index_range =
  decltype(offset_index_sequence<Start>(std::make_index_sequence<Count>{}));

template <typename Tuple, std::size_t... I>
constexpr auto tup_slice_impl(Tuple&& t, std::index_sequence<I...>)
{
  return std::tuple{std::get<I>(std::forward<Tuple>(t))...};
}

} // namespace detail

inline auto tup_capture()
{
  return std::tuple<>{};
}

template <typename Type>
auto tup_capture(Type&& arg)
{
  if constexpr (std::is_rvalue_reference_v<Type>)
  {
    return std::tuple<std::decay_t<Type>>{std::forward<Type>(arg)};
  }
  return std::tuple<Type>{std::forward<Type>(arg)};
}

/**
 * Capture the given values as a tuple. The resulting tuple has the same number of
 * elements as the number of the given arguments. For each argument x of type X, the
 * corresponding tuple member has type
 *
 * - const X& if x is a const lvalue reference,
 * - X& if x is an lvalue reference,
 * - X if x is an rvalue.
 *
 * In the last case where x is an rvalue, the value of x is moved into the tuple.
 *
 * @tparam First the type of the first argument
 * @tparam Rest the types of the remaining arguments
 * @param first the first argument
 * @param rest the remaining arguments
 *
 * @return a tuple containing the values of the arguments, as per the rules defined above
 */
template <typename First, typename... Rest>
auto tup_capture(First&& first, Rest&&... rest)
{
  return std::tuple_cat(
    tup_capture(std::forward<First>(first)), tup_capture(std::forward<Rest>(rest)...));
}

/**
 * Return a slice of length Count out of the given tuple starting at Start.
 *
 * @tparam Start the start index of the slice to return
 * @tparam Count the length of the slice to return
 * @tparam Tuple the type of the tuple
 * @param t the tuple to slice
 *
 * @return a tuple containing Count elements of t starting at Start
 */
template <std::size_t Start, std::size_t Count, typename Tuple>
constexpr auto tup_slice(Tuple&& t)
{
  static_assert(
    Start + Count <= std::tuple_size_v<std::remove_reference_t<Tuple>>,
    "slice is out of bounds");

  return detail::tup_slice_impl(
    std::forward<Tuple>(t), detail::make_index_range<Start, Count>{});
}

template <std::size_t M, typename Tuple>
constexpr auto tup_prefix(Tuple&& t)
{
  return tup_slice<0, M>(std::forward<Tuple>(t));
}

template <std::size_t M, typename Tuple>
constexpr auto tup_suffix(Tuple&& t)
{
  const auto TupleSize = std::tuple_size_v<std::remove_reference_t<Tuple>>;
  return tup_slice<TupleSize - M, M>(std::forward<Tuple>(t));
}


} // namespace kdl
