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

#include "detail/range_utils.h"

#include <cstdint>
#include <ranges>
#include <tuple>

namespace kdl
{
namespace ranges
{
namespace detail
{

template <bool Const, class... Views>
concept all_random_access =
  (std::ranges::random_access_range<detail::maybe_const<Const, Views>> && ...);

template <bool Const, typename First, typename... Vs>
concept cartesian_product_is_random_access =
  (std::ranges::random_access_range<maybe_const<Const, First>> && ...
   && (std::ranges::random_access_range<maybe_const<Const, Vs>> && std::ranges::sized_range<maybe_const<Const, Vs>>));

template <typename R>
concept cartesian_product_common_arg =
  std::ranges::common_range<R>
  || (std::ranges::sized_range<R> && std::ranges::random_access_range<R>);

template <bool Const, typename First, typename... Vs>
concept cartesian_product_is_bidirectional =
  (std::ranges::bidirectional_range<maybe_const<Const, First>> && ...
   && (std::ranges::bidirectional_range<maybe_const<Const, Vs>> && cartesian_product_common_arg<maybe_const<Const, Vs>>));

template <typename First, typename... Vs>
concept cartesian_product_is_common = cartesian_product_common_arg<First>;

template <bool Const, typename First, typename... Vs>
consteval auto get_cartesian_product_iter_cat()
{
  if constexpr (cartesian_product_is_random_access<Const, First, Vs...>)
  {
    return std::random_access_iterator_tag{};
  }
  else if constexpr (cartesian_product_is_bidirectional<Const, First, Vs...>)
  {
    return std::bidirectional_iterator_tag{};
  }
  else if constexpr (std::ranges::forward_range<maybe_const<Const, First>>)
  {
    return std::forward_iterator_tag{};
  }
  else
  {
    return std::input_iterator_tag{};
  }
}

template <typename... Vs>
concept cartesian_product_is_sized = (std::ranges::sized_range<Vs> && ...);

template <
  bool Const,
  template <typename>
  typename FirstSent,
  typename First,
  typename... Vs>
concept cartesian_is_sized_sentinel =
  (std::sized_sentinel_for<
     FirstSent<maybe_const<Const, First>>,
     std::ranges::iterator_t<maybe_const<Const, First>>>
   && ...
   && (std::ranges::sized_range<maybe_const<Const, Vs>> && std::sized_sentinel_for<std::ranges::iterator_t<maybe_const<Const, Vs>>, std::ranges::iterator_t<maybe_const<Const, Vs>>>));

template <cartesian_product_common_arg R>
constexpr auto cartesian_common_arg_end(R& r)
{
  if constexpr (std::ranges::common_range<R>)
  {
    return std::ranges::end(r);
  }
  else
  {
    return std::ranges::begin(r) + std::ranges::distance(r);
  }
}

template <typename T, typename B, std::size_t... Is>
constexpr auto cartesian_any_end(const T& t, const B& b, const std::index_sequence<Is...>)
{
  return ((std::get<Is>(t) == std::ranges::end(std::get<Is>(b))) || ...);
}

template <typename Tuple>
constexpr auto cartesian_end_tuple(const Tuple& t)
{
  return std::apply(
    [](const auto& first, const auto&... rest) {
      return std::tuple{
        std::ranges::end(first),
        std::ranges::begin(rest)...,
      };
    },
    t);
}

template <typename Tuple, std::size_t... Is>
constexpr void cartesian_all_noexcept_iter_move(
  const Tuple& t,
  std::index_sequence<
    Is...>) noexcept((noexcept(std::ranges::iter_move(std::get<Is>(t))) && ...));

template <bool Const, typename... Vs>
constexpr auto cartesian_all_nothrow_move_constructible()
{
  return (
    std::is_nothrow_move_constructible_v<
      std::ranges::range_rvalue_reference_t<maybe_const<Const, Vs>>>
    && ...);
}

template <typename Tuple, std::size_t... Is>
constexpr auto cartesian_all_noexcept_iter_swap(
  const Tuple& t1, const Tuple& t2, const std::index_sequence<Is...>)
{
  return (noexcept(std::ranges::iter_swap(std::get<Is>(t1), std::get<Is>(t2))) && ...);
}

template <typename Tuple, std::size_t... Is>
constexpr void cartesian_iter_swap(Tuple& t1, Tuple& t2, const std::index_sequence<Is...>)
{
  (std::ranges::iter_swap(std::get<Is>(t1), std::get<Is>(t2)), ...);
}

} // namespace detail


template <std::ranges::input_range First, std::ranges::forward_range... Vs>
  requires(std::ranges::view<First> && ... && std::ranges::view<Vs>)
class cartesian_product_view
  : public std::ranges::view_interface<cartesian_product_view<First, Vs...>>
{
public:
  template <bool Const>
  class iterator
  {
    using Parent = detail::maybe_const<Const, cartesian_product_view>;
    using IterTuple = std::tuple<
      std::ranges::iterator_t<detail::maybe_const<Const, First>>,
      std::ranges::iterator_t<detail::maybe_const<Const, Vs>>...>;

  public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept =
      decltype(detail::get_cartesian_product_iter_cat<Const, First, Vs...>());
    using value_type = std::tuple<
      std::ranges::range_value_t<detail::maybe_const<Const, First>>,
      std::ranges::range_value_t<detail::maybe_const<Const, Vs>>...>;

    // Note: this should use std::ranges::range_reference_t, but that doesn't compile with
    // -std=c++20 (it does with -std=c++23).
    using reference = std::tuple<
      std::ranges::range_value_t<detail::maybe_const<Const, First>>,
      std::ranges::range_value_t<detail::maybe_const<Const, Vs>>...>;
    using difference_type = std::intmax_t;

    iterator() = default;

    explicit constexpr iterator(iterator<!Const> i)
      requires Const
                 && (std::convertible_to<std::ranges::iterator_t<First>, std::ranges::iterator_t<const First>> && ... && std::convertible_to<std::ranges::iterator_t<Vs>, std::ranges::iterator_t<const Vs>>)
      : parent_{i.parent_}
      , current_{std::move(i.current_)}
    {
    }

    constexpr value_type operator*() const
    {
      return detail::tuple_transform(
        [](auto& i) -> decltype(auto) { return *i; }, current_);
    }

    constexpr reference operator[](const difference_type n) const
      requires detail::cartesian_product_is_random_access<Const, First, Vs...>
    {
      return *((*this) + n);
    }

    constexpr iterator& operator++()
    {
      next();
      return *this;
    }

    constexpr void operator++(int) { return ++*this; }

    constexpr iterator operator++(int)
      requires std::ranges::forward_range<detail::maybe_const<Const, First>>
    {
      const auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator& operator--()
      requires detail::cartesian_product_is_bidirectional<Const, First, Vs...>
    {
      prev();
      return *this;
    }

    constexpr iterator operator--(int)
      requires detail::cartesian_product_is_bidirectional<Const, First, Vs...>
    {
      const auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator& operator+=(const difference_type n)
      requires detail::cartesian_product_is_random_access<Const, First, Vs...>
    {
      if (n > 0)
      {
        for (auto i = 0; i < n; ++i)
        {
          next();
        }
      }
      else if (n < 0)
      {
        for (auto i = 0; i < -n; ++i)
        {
          prev();
        }
      }
      return *this;
    }

    constexpr iterator& operator-=(difference_type n)
      requires detail::cartesian_product_is_random_access<Const, First, Vs...>
    {
      *this += -n;
      return *this;
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
      requires std::equality_comparable<
        std::ranges::iterator_t<detail::maybe_const<Const, First>>>
    {
      return x.current_ == y.current_;
    }

    friend constexpr bool operator==(const iterator& x, const std::default_sentinel_t)
    {
      return detail::cartesian_any_end(
        x.current_, x.parent_->bases_, std::index_sequence_for<First, Vs...>());
    }

    friend constexpr auto operator<=>(const iterator& x, const iterator& y)
      requires detail::all_random_access<Const, First, Vs...>
    {
      return x.current_ <=> y.current_;
    }

    friend constexpr iterator operator+(const iterator& i, const difference_type n)
      requires detail::cartesian_product_is_random_access<Const, First, Vs...>
    {
      return iterator{i} += n;
    }

    friend constexpr iterator operator+(const difference_type n, const iterator& i)
      requires detail::cartesian_product_is_random_access<Const, First, Vs...>
    {
      return i + n;
    }

    friend constexpr iterator operator-(const iterator& i, const difference_type n)
      requires detail::cartesian_product_is_random_access<Const, First, Vs...>
    {
      return iterator{i} -= n;
    }

    friend constexpr difference_type operator-(const iterator& i, const iterator& j)
      requires detail::
        cartesian_is_sized_sentinel<Const, std::ranges::iterator_t, First, Vs...>
    {
      return i.distance_from(j.current_);
    }

    friend constexpr difference_type operator-(
      const iterator& i, const std::default_sentinel_t)
      requires detail::
        cartesian_is_sized_sentinel<Const, std::ranges::sentinel_t, First, Vs...>
    {
      return i.distance_from(detail::cartesian_end_tuple(i.parent_->bases_));
    }

    friend constexpr difference_type operator-(
      const std::default_sentinel_t s, const iterator& i)
      requires detail::
        cartesian_is_sized_sentinel<Const, std::ranges::sentinel_t, First, Vs...>
    {
      return -(i - s);
    }

    friend constexpr auto iter_move(const iterator& i) noexcept(
      noexcept(detail::cartesian_all_noexcept_iter_move(
        i.current_, std::index_sequence_for<First, Vs...>()))
      && detail::cartesian_all_nothrow_move_constructible<Const, First, Vs...>())
    {
      return detail::tuple_transform(std::ranges::iter_move, i.current_);
    }

    friend constexpr void iter_swap(const iterator& x, const iterator& y) noexcept(
      noexcept(detail::cartesian_all_noexcept_iter_swap(
        x.current_, y.current_, std::index_sequence_for<First, Vs...>())))
      requires(
        std::indirectly_swappable<
          std::ranges::iterator_t<detail::maybe_const<Const, First>>>
        && ...
        && std::indirectly_swappable<
          std::ranges::iterator_t<detail::maybe_const<Const, Vs>>>)
    {
      cartesian_iter_swap(
        x.current_, y.current_, std::index_sequence_for<First, Vs...>());
    }

  private:
    friend class cartesian_product_view;

    constexpr iterator(Parent* parent, IterTuple current)
      : parent_{parent}
      , current_{std::move(current)}
    {
    }

    //! If called with default template parameter, recursively generates the next element
    //! (the tuple of iterators) in cartesian_product_view.
    template <std::size_t N = sizeof...(Vs)>
    constexpr void next()
    {
      auto& it = std::get<N>(current_);
      ++it;

      if constexpr (N > 0)
      {
        if (it == std::ranges::end(std::get<N>(parent_->bases_)))
        {
          it = std::ranges::begin(std::get<N>(parent_->bases_));
          next<N - 1>();
        }
      }
    }

    //! If called with default template parameter, recursively generates the previous
    //! element (the tuple of iterators) in cartesian_product_view.
    template <std::size_t N = sizeof...(Vs)>
    constexpr void prev()
    {
      auto& it = std::get<N>(current_);
      if constexpr (N > 0)
      {
        if (it == std::ranges::begin(std::get<N>(parent_->bases_)))
        {
          it = detail::cartesian_common_arg_end(std::get<N>(parent_->bases_));
          prev<N - 1>();
        }
      }
      --it;
    }

    //! Returns the "distance" (i.e., number of "hops") between two iterators.
    template <typename Tuple>
    constexpr difference_type distance_from(const Tuple& t) const
    {
      return scaled_sum(t);
    }

    template <std::size_t N = sizeof...(Vs), typename Tuple>
    constexpr auto scaled_sum(const Tuple& t) const
    {
      if constexpr (N > 0)
      {
        return scaled_distance<N>(t) + scaled_sum<N - 1>(t);
      }
      else
      {
        return scaled_distance<N>(t);
      }
    }

    template <std::size_t N, typename Tuple>
    constexpr auto scaled_distance(const Tuple& t) const
    {
      return static_cast<difference_type>(std::get<N>(current_) - std::get<N>(t))
             * scaled_size<N + 1>();
    }

    template <std::size_t N>
    constexpr auto scaled_size() const
    {
      if constexpr (N <= sizeof...(Vs))
      {
        return static_cast<difference_type>(
                 std::ranges::size(std::get<N>(parent_->bases_)))
               * scaled_size<N + 1>();
      }
      else
      {
        return static_cast<difference_type>(1);
      }
    }

    Parent* parent_{nullptr};
    IterTuple current_{};
  };

  constexpr cartesian_product_view() = default;

  constexpr explicit cartesian_product_view(First first_base, Vs... bases)
    : bases_{std::move(first_base), std::move(bases)...}
  {
  }

  constexpr iterator<false> begin()
    requires(!detail::simple_view<First> || ... || !detail::simple_view<Vs>)
  {
    return iterator<false>{this, detail::tuple_transform(std::ranges::begin, bases_)};
  }

  constexpr iterator<true> begin() const
    requires(std::ranges::range<const First> && ... && std::ranges::range<const Vs>)
  {
    return iterator<true>{this, detail::tuple_transform(std::ranges::begin, bases_)};
  }

  constexpr iterator<false> end()
    requires(
      (!detail::simple_view<First> || ... || !detail::simple_view<Vs>)
      && detail::cartesian_product_is_common<First, Vs...>)
  {
    const auto any_empty_except_first = std::apply(
      [](const auto&, const auto&... rest) { return (std::ranges::empty(rest) || ...); },
      bases_);

    auto t = std::apply(
      [&](auto& first, auto&... rest) {
        return std::tuple{
          any_empty_except_first ? std::ranges::begin(first)
                                 : detail::cartesian_common_arg_end(first),
          std::ranges::begin(rest)...,
        };
      },
      bases_);

    return iterator<false>{this, std::move(t)};
  }

  constexpr iterator<true> end() const
    requires detail::cartesian_product_is_common<const First, const Vs...>
  {
    const auto any_empty_except_first = std::apply(
      [](const auto&, const auto&... rest) { return (std::ranges::empty(rest) || ...); },
      bases_);

    auto t = std::apply(
      [&](auto& first, auto&... rest) {
        return std::tuple{
          any_empty_except_first ? std::ranges::begin(first)
                                 : detail::cartesian_common_arg_end(first),
          std::ranges::begin(rest)...,
        };
      },
      bases_);

    return iterator<true>{this, std::move(t)};
  }

  constexpr std::default_sentinel_t end() const noexcept { return std::default_sentinel; }

  constexpr auto size()
    requires detail::cartesian_product_is_sized<First, Vs...>
  {
    return const_cast<const cartesian_product_view*>(this)->size();
  }

  constexpr auto size() const
    requires detail::cartesian_product_is_sized<First, Vs...>
  {
    using SizeType = std::uintmax_t;

    return std::apply(
      [](const auto&... b) {
        return (static_cast<SizeType>(std::ranges::size(b)) * ...);
      },
      bases_);
  }

private:
  std::tuple<First, Vs...> bases_;
};

template <typename... Rs>
cartesian_product_view(Rs&&...) -> cartesian_product_view<std::views::all_t<Rs>...>;

namespace views
{

template <std::ranges::viewable_range... Rs>
constexpr auto cartesian_product(Rs&&... r)
{
  return ranges::cartesian_product_view{std::forward<Rs>(r)...};
};

} // namespace views
} // namespace ranges

namespace views
{

template <std::ranges::viewable_range... R>
constexpr auto cartesian_product(R&&... r)
{
  return ranges::views::cartesian_product(std::forward<R>(r)...);
};

} // namespace views
} // namespace kdl

template <typename... R>
constexpr bool
  std::ranges::enable_borrowed_range<kdl::ranges::cartesian_product_view<R...>> =
    (std::ranges::enable_borrowed_range<R> && ...);
