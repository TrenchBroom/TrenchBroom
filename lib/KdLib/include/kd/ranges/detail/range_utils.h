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

#include <functional>
#include <ranges>
#include <type_traits>

namespace kdl::ranges
{
namespace detail
{

template <bool Const, typename T>
using maybe_const = std::conditional_t<Const, const T, T>;

template <class R>
concept simple_view = // exposition only
  std::ranges::view<R> && std::ranges::range<const R>
  && std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<const R>>
  && std::same_as<std::ranges::sentinel_t<R>, std::ranges::sentinel_t<const R>>;

template <typename T>
concept can_reference =
  std::is_object_v<T> || std::is_function_v<T> || std::is_reference_v<T>;

template <typename... V>
consteval auto get_iter_cat()
{
  if constexpr ((std::ranges::random_access_range<V> && ...))
  {
    return std::random_access_iterator_tag{};
  }
  else if constexpr ((std::ranges::bidirectional_range<V> && ...))
  {
    return std::bidirectional_iterator_tag{};
  }
  else if constexpr ((std::ranges::forward_range<V> && ...))
  {
    return std::forward_iterator_tag{};
  }
  else if constexpr ((std::ranges::input_range<V> && ...))
  {
    return std::input_iterator_tag{};
  }
  else
  {
    return std::output_iterator_tag{};
  }
}

template <class F, class Tuple>
constexpr auto tuple_transform(F&& f, Tuple&& tuple)
{
  return std::apply(
    [&]<typename... Ts>(Ts&&... args) {
      return std::tuple<std::invoke_result_t<F&, Ts>...>(
        std::invoke(f, std::forward<Ts>(args))...);
    },
    std::forward<Tuple>(tuple));
}

template <typename T, std::size_t N>
struct tuple_repeat
{
  using type = decltype(std::tuple_cat(
    std::declval<typename tuple_repeat<T, N - 1>::type>(),
    std::declval<std::tuple<T>>()));
};

template <typename T>
struct tuple_repeat<T, 1>
{
  using type = std::tuple<T>;
};

template <typename T, std::size_t N>
using tuple_repeat_t = typename tuple_repeat<T, N>::type;

//! Computes the smallest integer that is not less than num / denom.
template <typename I>
constexpr auto div_ceil(I num, I denom)
{
  return num / denom + (num % denom ? 1 : 0);
}

} // namespace detail

template <std::ranges::range R>
using const_iterator_t = decltype(std::ranges::cbegin(std::declval<R&>()));

template <std::ranges::range R>
using const_sentinel_t = decltype(std::ranges::cend(std::declval<R&>()));

} // namespace kdl::ranges
