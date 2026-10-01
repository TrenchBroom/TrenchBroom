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

#include <concepts>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>

// Backports of the C++23 fold algorithms from <algorithm>, see
// https://en.cppreference.com/w/cpp/algorithm/ranges/fold_left and the related pages.

namespace kdl
{

template <typename I, typename T>
struct in_value_result
{
  [[no_unique_address]] I in;
  [[no_unique_address]] T value;

  template <typename I2, typename T2>
    requires std::convertible_to<const I&, I2> && std::convertible_to<const T&, T2>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr operator in_value_result<I2, T2>() const&
  {
    return {in, value};
  }

  template <typename I2, typename T2>
    requires std::convertible_to<I, I2> && std::convertible_to<T, T2>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr operator in_value_result<I2, T2>() &&
  {
    return {std::move(in), std::move(value)};
  }
};

template <typename I, typename T>
using fold_left_with_iter_result = in_value_result<I, T>;

template <typename I, typename T>
using fold_left_first_with_iter_result = in_value_result<I, T>;

namespace detail
{

template <typename F>
class flipped
{
private:
  F m_f;

public:
  template <typename T, typename U>
    requires std::invocable<F&, U, T>
  std::invoke_result_t<F&, U, T> operator()(T&&, U&&);
};

template <typename F, typename T, typename I, typename U>
concept indirectly_binary_left_foldable_impl =
  std::movable<T> && std::movable<U> && std::convertible_to<T, U>
  && std::invocable<F&, U, std::iter_reference_t<I>>
  && std::assignable_from<U&, std::invoke_result_t<F&, U, std::iter_reference_t<I>>>;

template <typename F, typename T, typename I>
concept indirectly_binary_left_foldable =
  std::copy_constructible<F> && std::indirectly_readable<I>
  && std::invocable<F&, T, std::iter_reference_t<I>>
  && std::convertible_to<
    std::invoke_result_t<F&, T, std::iter_reference_t<I>>,
    std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<I>>>>
  && indirectly_binary_left_foldable_impl<
    F,
    T,
    I,
    std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<I>>>>;

template <typename F, typename T, typename I>
concept indirectly_binary_right_foldable =
  indirectly_binary_left_foldable<flipped<F>, T, I>;

// Replaces the iterator of a result with std::ranges::dangling if R is not a borrowed
// range, like the range overloads of the std algorithms do.
template <typename R, typename I, typename T>
constexpr auto toBorrowedResult(in_value_result<I, T>&& result)
{
  return in_value_result<std::ranges::borrowed_iterator_t<R>, T>{
    [&]() -> std::ranges::borrowed_iterator_t<R> {
      if constexpr (std::ranges::borrowed_range<R>)
      {
        return std::move(result.in);
      }
      else
      {
        return {};
      }
    }(),
    std::move(result.value)};
}

struct fold_left_with_iter_fn
{
  template <
    std::input_iterator I,
    std::sentinel_for<I> S,
    typename T,
    indirectly_binary_left_foldable<T, I> F>
  constexpr auto operator()(I first, S last, T init, F f) const
  {
    using U = std::decay_t<std::invoke_result_t<F&, T, std::iter_reference_t<I>>>;
    using Ret = fold_left_with_iter_result<I, U>;

    if (first == last)
    {
      return Ret{std::move(first), U(std::move(init))};
    }

    U accum = std::invoke(f, std::move(init), *first);
    for (++first; first != last; ++first)
    {
      accum = std::invoke(f, std::move(accum), *first);
    }
    return Ret{std::move(first), std::move(accum)};
  }

  template <
    std::ranges::input_range R,
    typename T,
    indirectly_binary_left_foldable<T, std::ranges::iterator_t<R>> F>
  constexpr auto operator()(R&& r, T init, F f) const
  {
    return toBorrowedResult<R>(
      (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(init), std::ref(f)));
  }
};

struct fold_left_fn
{
  template <
    std::input_iterator I,
    std::sentinel_for<I> S,
    typename T,
    indirectly_binary_left_foldable<T, I> F>
  constexpr auto operator()(I first, S last, T init, F f) const
  {
    return fold_left_with_iter_fn{}(std::move(first), last, std::move(init), std::ref(f))
      .value;
  }

  template <
    std::ranges::input_range R,
    typename T,
    indirectly_binary_left_foldable<T, std::ranges::iterator_t<R>> F>
  constexpr auto operator()(R&& r, T init, F f) const
  {
    return (*this)(
      std::ranges::begin(r), std::ranges::end(r), std::move(init), std::ref(f));
  }
};

struct fold_left_first_with_iter_fn
{
  template <
    std::input_iterator I,
    std::sentinel_for<I> S,
    indirectly_binary_left_foldable<std::iter_value_t<I>, I> F>
    requires std::constructible_from<std::iter_value_t<I>, std::iter_reference_t<I>>
  constexpr auto operator()(I first, S last, F f) const
  {
    using U =
      decltype(fold_left_fn{}(std::move(first), last, std::iter_value_t<I>(*first), f));
    using Ret = fold_left_first_with_iter_result<I, std::optional<U>>;

    if (first == last)
    {
      return Ret{std::move(first), std::optional<U>{}};
    }

    auto init = std::optional<U>{std::in_place, *first};
    for (++first; first != last; ++first)
    {
      *init = std::invoke(f, std::move(*init), *first);
    }
    return Ret{std::move(first), std::move(init)};
  }

  template <
    std::ranges::input_range R,
    indirectly_binary_left_foldable<
      std::ranges::range_value_t<R>,
      std::ranges::iterator_t<R>> F>
    requires std::
      constructible_from<std::ranges::range_value_t<R>, std::ranges::range_reference_t<R>>
    constexpr auto operator()(R&& r, F f) const
  {
    return toBorrowedResult<R>(
      (*this)(std::ranges::begin(r), std::ranges::end(r), std::ref(f)));
  }
};

struct fold_left_first_fn
{
  template <
    std::input_iterator I,
    std::sentinel_for<I> S,
    indirectly_binary_left_foldable<std::iter_value_t<I>, I> F>
    requires std::constructible_from<std::iter_value_t<I>, std::iter_reference_t<I>>
  constexpr auto operator()(I first, S last, F f) const
  {
    return fold_left_first_with_iter_fn{}(std::move(first), last, std::ref(f)).value;
  }

  template <
    std::ranges::input_range R,
    indirectly_binary_left_foldable<
      std::ranges::range_value_t<R>,
      std::ranges::iterator_t<R>> F>
    requires std::
      constructible_from<std::ranges::range_value_t<R>, std::ranges::range_reference_t<R>>
    constexpr auto operator()(R&& r, F f) const
  {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::ref(f));
  }
};

struct fold_right_fn
{
  template <
    std::bidirectional_iterator I,
    std::sentinel_for<I> S,
    typename T,
    indirectly_binary_right_foldable<T, I> F>
  constexpr auto operator()(I first, S last, T init, F f) const
  {
    using U = std::decay_t<std::invoke_result_t<F&, std::iter_reference_t<I>, T>>;

    if (first == last)
    {
      return U(std::move(init));
    }

    I tail = std::ranges::next(first, last);
    U accum = std::invoke(f, *--tail, std::move(init));
    while (first != tail)
    {
      accum = std::invoke(f, *--tail, std::move(accum));
    }
    return accum;
  }

  template <
    std::ranges::bidirectional_range R,
    typename T,
    indirectly_binary_right_foldable<T, std::ranges::iterator_t<R>> F>
  constexpr auto operator()(R&& r, T init, F f) const
  {
    return (*this)(
      std::ranges::begin(r), std::ranges::end(r), std::move(init), std::ref(f));
  }
};

struct fold_right_last_fn
{
  template <
    std::bidirectional_iterator I,
    std::sentinel_for<I> S,
    indirectly_binary_right_foldable<std::iter_value_t<I>, I> F>
    requires std::constructible_from<std::iter_value_t<I>, std::iter_reference_t<I>>
  constexpr auto operator()(I first, S last, F f) const
  {
    using U = decltype(fold_right_fn{}(first, last, std::iter_value_t<I>(*first), f));

    if (first == last)
    {
      return std::optional<U>{};
    }

    I tail = std::ranges::prev(std::ranges::next(first, std::move(last)));
    return std::optional<U>{
      std::in_place,
      fold_right_fn{}(std::move(first), tail, std::iter_value_t<I>(*tail), std::move(f))};
  }

  template <
    std::ranges::bidirectional_range R,
    indirectly_binary_right_foldable<
      std::ranges::range_value_t<R>,
      std::ranges::iterator_t<R>> F>
    requires std::
      constructible_from<std::ranges::range_value_t<R>, std::ranges::range_reference_t<R>>
    constexpr auto operator()(R&& r, F f) const
  {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::ref(f));
  }
};

} // namespace detail

inline constexpr auto fold_left = detail::fold_left_fn{};
inline constexpr auto fold_left_first = detail::fold_left_first_fn{};
inline constexpr auto fold_right = detail::fold_right_fn{};
inline constexpr auto fold_right_last = detail::fold_right_last_fn{};
inline constexpr auto fold_left_with_iter = detail::fold_left_with_iter_fn{};
inline constexpr auto fold_left_first_with_iter = detail::fold_left_first_with_iter_fn{};

} // namespace kdl
