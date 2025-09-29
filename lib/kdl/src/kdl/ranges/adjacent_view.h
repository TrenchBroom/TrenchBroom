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

#include <algorithm>
#include <array>
#include <ranges>
#include <tuple>

namespace kdl
{
namespace ranges
{

template <std::ranges::forward_range V, std::size_t N>
  requires std::ranges::view<V> && (N > 0)
class adjacent_view : public std::ranges::view_interface<adjacent_view<V, N>>
{
public:
  struct as_sentinel
  {
  };

  template <bool Const>
  class iterator
  {
    using Base = detail::maybe_const<Const, V>;
    using iter_array = std::array<std::ranges::iterator_t<Base>, N>;

  public:
    using iterator_concept = decltype(detail::get_iter_cat<Base>());
    using iterator_category = std::input_iterator_tag;
    using value_type = detail::tuple_repeat_t<std::ranges::range_value_t<Base>, N>;
    using difference_type = std::ranges::range_difference_t<Base>;

    iterator() = default;

    constexpr explicit iterator(iterator<!Const> current)
      requires Const
               && std::
                 convertible_to<std::ranges::iterator_t<V>, std::ranges::iterator_t<Base>>
      : current_{std::move(current)}
    {
    }

    // not in spec, but useful for adjacent_transform_view
    constexpr const iter_array& base() const& noexcept { return current_; }

    // not in spec, but useful for adjacent_transform_view
    constexpr iter_array base() && { return std::move(current_); }

    constexpr auto operator*() const
    {
      return detail::tuple_transform([](auto&& i) { return *i; }, current_);
    }

    constexpr auto operator[](const difference_type n) const
      requires(std::ranges::random_access_range<Base>)
    {
      return detail::tuple_transform([&](auto&& i) { return i[n]; }, current_);
    }

    constexpr iterator& operator++()
    {
      std::apply([](auto&&... i) { return (++i, ...); }, current_);
      return *this;
    }

    constexpr auto operator++(int)
    {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator& operator--()
      requires(std::ranges::bidirectional_range<Base>)
    {
      std::apply([](auto&&... i) { (--i, ...); }, current_);
      return *this;
    }

    constexpr auto operator--(int)
      requires(std::ranges::bidirectional_range<Base>)
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator& operator+=(const difference_type n)
      requires(std::ranges::random_access_range<Base>)
    {
      std::apply([n](auto&&... i) { ((i += n), ...); }, current_);
      return *this;
    }

    constexpr iterator& operator-=(const difference_type n)
      requires(std::ranges::random_access_range<Base>)
    {
      std::apply([n](auto&&... i) { ((i -= n), ...); }, current_);
      return *this;
    }

    friend constexpr bool operator==(const iterator& lhs, const iterator& rhs)
    {
      return lhs.current_.back() == rhs.current_.back();
    }

    friend constexpr bool operator<(const iterator& lhs, const iterator& rhs)
      requires(std::ranges::random_access_range<Base>)
    {
      return lhs.current_.back() < rhs.current_.back();
    }

    friend constexpr bool operator>(const iterator& lhs, const iterator& rhs)
      requires(std::ranges::random_access_range<Base>)
    {
      return lhs.current_.back() > rhs.current_.back();
    }

    friend constexpr bool operator<=(const iterator& lhs, const iterator& rhs)
      requires(std::ranges::random_access_range<Base>)
    {
      return lhs.current_.back() <= rhs.current_.back();
    }

    friend constexpr bool operator>=(const iterator& lhs, const iterator& rhs)
      requires(std::ranges::random_access_range<Base>)
    {
      return lhs.current_.back() >= rhs.current_.back();
    }

    friend constexpr auto operator<=>(const iterator& lhs, const iterator& rhs)
      requires(
        (std::ranges::random_access_range<Base>
         && std::three_way_comparable<std::ranges::iterator_t<Base>>))
    {
      return lhs.current_.back() <=> rhs.current_.back();
    }

    friend constexpr iterator operator+(const iterator& i, const difference_type n)
      requires(std::ranges::random_access_range<Base>)
    {
      return std::apply(
        [n](auto&&... j) { return iterator{iter_array{j + n...}}; }, i.current_);
    }

    friend constexpr iterator operator+(const difference_type n, const iterator& i)
      requires(std::ranges::random_access_range<Base>)
    {
      return i + n;
    }

    friend constexpr auto operator-(const iterator& i, const difference_type n)
      requires(std::ranges::random_access_range<Base>)
    {
      return std::apply(
        [n](auto&&... j) { return iterator{iter_array{j - n...}}; }, i.current_);
    }

    friend constexpr difference_type operator-(const iterator& lhs, const iterator& rhs)
      requires(std::sized_sentinel_for<
               std::ranges::iterator_t<Base>,
               std::ranges::iterator_t<Base>>)
    {
      return lhs.current_.back() - rhs.current_.back();
    }

    friend constexpr auto iter_move(const iterator& i) noexcept(
      noexcept(
        std::ranges::iter_move(std::declval<const std::ranges::iterator_t<Base>&>()))
      && std::is_nothrow_move_constructible_v<
        std::ranges::range_rvalue_reference_t<Base>>)
    {
      return detail::tuple_transform(std::ranges::iter_move, i.current_);
    }

    friend constexpr void iter_swap(const iterator& lhs, const iterator& rhs) noexcept(
      std::ranges::iter_swap(
        std::declval<std::ranges::iterator_t<Base>>(),
        std::declval<std::ranges::iterator_t<Base>>()))
      requires std::indirectly_swappable<std::ranges::iterator_t<Base>>
    {
      for (std::size_t i = 0; i != N; ++i)
      {
        std::ranges::iter_swap(lhs.current_[i], rhs.current_[i]);
      }
    }

  private:
    friend class adjacent_view;

    constexpr iterator(
      const std::ranges::iterator_t<Base> begin, const std::ranges::sentinel_t<Base> end)
    {
      if (std::distance(begin, end) >= static_cast<difference_type>(N))
      {
        for (std::size_t i = 0; i < N; ++i)
        {
          current_[i] = std::next(begin, static_cast<difference_type>(i));
        }
      }
      else
      {
        std::ranges::fill(current_, end);
      }
    }

    constexpr iterator(
      const as_sentinel&,
      const std::ranges::iterator_t<Base> begin,
      const std::ranges::sentinel_t<Base> end)
    {
      const auto length = std::distance(begin, end);
      if (length >= static_cast<difference_type>(N))
      {
        for (std::size_t i = 0; i < N; ++i)
        {
          const auto offset = static_cast<difference_type>(N - i - 1);
          if constexpr (std::bidirectional_iterator<std::ranges::sentinel_t<Base>>)
          {
            current_[i] = std::ranges::prev(end, offset);
          }
          else
          {
            current_[i] = std::ranges::next(end, length - offset);
          }
        }
      }
      else
      {
        std::ranges::fill(current_, end);
      }
    }

    constexpr explicit iterator(iter_array current)
      : current_{std::move(current)}
    {
    }

    iter_array current_{};
  };

  template <bool Const>
  class sentinel
  {
    using Base = std::conditional_t<Const, const V, V>;

  public:
    sentinel() = default;

    constexpr explicit sentinel(sentinel<!Const> s)
      requires Const
               && std::
                 convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<Base>>
      : end_{std::move(s.end_)}
    {
    }

    template <bool OtherConst>
      requires std::sentinel_for<
        std::ranges::sentinel_t<Base>,
        std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
    friend constexpr bool operator==(const iterator<OtherConst>& lhs, const sentinel& rhs)
    {
      return lhs.current_.back() == rhs.end_;
    }

    template <bool OtherConst>
      requires std::sized_sentinel_for<
        std::ranges::sentinel_t<Base>,
        std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
    friend constexpr std::ranges::range_difference_t<detail::maybe_const<OtherConst, V>>
    operator-(const iterator<OtherConst>& lhs, const sentinel& rhs)
    {
      return lhs.current_.back() - rhs.end_;
    }

    template <bool OtherConst>

      requires std::sized_sentinel_for<
        std::ranges::sentinel_t<Base>,
        std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
    friend constexpr std::ranges::range_difference_t<detail::maybe_const<OtherConst, V>>
    operator-(const sentinel& lhs, const iterator<OtherConst>& rhs)
    {
      return lhs.end_ - rhs.current_.back();
    }

  private:
    friend class adjacent_view;

    std::ranges::sentinel_t<Base> end_{};
  };

  adjacent_view()
    requires std::default_initializable<V>
  = default;

  constexpr explicit adjacent_view(V base)
    : base_{base}
  {
  }

  constexpr auto begin()
    requires(!detail::simple_view<V>)
  {
    return iterator<false>{std::ranges::begin(base_), std::ranges::end(base_)};
  }

  constexpr auto begin() const
    requires std::ranges::range<const V>
  {
    return iterator<true>{std::ranges::begin(base_), std::ranges::end(base_)};
  }

  constexpr auto end()
    requires(!detail::simple_view<V>)
  {
    if constexpr (std::ranges::common_range<V>)
    {
      return iterator<false>{
        as_sentinel{}, std::ranges::begin(base_), std::ranges::end(base_)};
    }
    else
    {
      return sentinel<false>{std::ranges::end(base_)};
    }
  }

  constexpr auto end() const
    requires std::ranges::range<const V>
  {
    if constexpr (std::ranges::common_range<const V>)
    {
      return iterator<true>{
        as_sentinel{}, std::ranges::begin(base_), std::ranges::end(base_)};
    }
    else
    {
      return sentinel<true>{std::ranges::end(base_)};
    }
  }

  constexpr auto size()
    requires std::ranges::sized_range<V>
  {
    return std::ranges::size(base_);
  }

  constexpr auto size() const
    requires std::ranges::sized_range<const V>
  {
    return std::ranges::size(base_);
  }

private:
  V base_{};
};

namespace views
{
namespace detail
{

template <std::size_t N>
struct adjacent_view_fn
{
  template <typename R>
  constexpr auto operator()(R&& r) const
  {
    return adjacent_view<std::ranges::views::all_t<R>, N>{std::forward<R>(r)};
  }

  template <std::ranges::input_range R>
  constexpr friend auto operator|(R&& r, const adjacent_view_fn&)
  {
    return adjacent_view<std::ranges::views::all_t<R>, N>{std::forward<R>(r)};
  }
};

} // namespace detail

template <std::size_t N>
inline detail::adjacent_view_fn<N> adjacent;

} // namespace views
} // namespace ranges

namespace views
{

template <std::size_t N>
constexpr ranges::views::detail::adjacent_view_fn<N> adjacent;

constexpr ranges::views::detail::adjacent_view_fn<2> pairwise;

} // namespace views
} // namespace kdl

template <typename V, std::size_t N>
constexpr bool std::ranges::enable_borrowed_range<kdl::ranges::adjacent_view<V, N>> =
  std::ranges::enable_borrowed_range<V>;
