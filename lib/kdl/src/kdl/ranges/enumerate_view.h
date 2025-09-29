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

#include <ranges>

namespace kdl
{
namespace ranges
{
namespace detail
{

template <class R>
concept range_with_movable_references =
  std::ranges::input_range<R>
  && std::move_constructible<std::ranges::range_reference_t<R>>
  && std::move_constructible<std::ranges::range_rvalue_reference_t<R>>;

}

template <std::ranges::view V>
  requires detail::range_with_movable_references<V>
class enumerate_view : public std::ranges::view_interface<enumerate_view<V>>
{
public:
  template <bool Const>
  class iterator
  {
    using Base = detail::maybe_const<Const, V>;

  public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = decltype(detail::get_iter_cat<Base>());
    using difference_type = std::ranges::range_difference_t<Base>;
    using value_type = std::tuple<difference_type, std::ranges::range_value_t<Base>>;

  private:
    // Note: this should use std::ranges::range_reference_t, but that doesn't compile with
    // -std=c++20 (it does with -std=c++23).
    using reference_type = std::tuple<difference_type, std::ranges::range_value_t<Base>>;

  public:
    iterator()
      requires std::default_initializable<std::ranges::iterator_t<Base>>
    = default;

    constexpr explicit iterator(iterator<!Const> i)
      requires Const
                 && std::convertible_to<
                   std::ranges::iterator_t<V>,
                   std::ranges::iterator_t<Base>>
      : current_{std::move(i.current_)}
      , pos_{i.pos}
    {
    }

    constexpr const std::ranges::iterator_t<Base>& base() const& noexcept
    {
      return current_;
    }

    constexpr std::ranges::iterator_t<Base> base() && { return std::move(current_); }

    constexpr difference_type index() const noexcept { return pos_; }

    constexpr auto operator*() const { return reference_type{pos_, *current_}; }

    constexpr auto operator[](const difference_type n) const
      requires std::ranges::random_access_range<Base>
    {
      return reference_type{pos_ + n, current_[n]};
    }

    constexpr iterator& operator++()
    {
      ++current_;
      ++pos_;
      return *this;
    }

    constexpr void operator++(int)
    {
      ++current_;
      ++pos_;
    }

    constexpr iterator operator++(int)
      requires std::ranges::forward_range<Base>
    {
      const auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator& operator--()
      requires std::ranges::bidirectional_range<Base>
    {
      --current_;
      --pos_;
      return *this;
    }

    constexpr iterator operator--(int)
      requires std::ranges::bidirectional_range<Base>
    {
      const auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator& operator+=(const difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      current_ += n;
      pos_ += n;
      return *this;
    }

    constexpr iterator& operator-=(const difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      current_ -= n;
      pos_ -= n;
      return *this;
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y) noexcept
    {
      return x.pos_ == y.pos_;
    }

    friend constexpr std::strong_ordering operator<=>(
      const iterator& x, const iterator& y) noexcept
    {
      return x.pos_ <=> y.pos_;
    }

    friend constexpr iterator operator+(const iterator& i, const difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      auto temp = i;
      temp += n;
      return temp;
    }

    friend constexpr iterator operator+(const difference_type n, const iterator& i)
      requires std::ranges::random_access_range<Base>
    {
      return i + n;
    }

    friend constexpr iterator operator-(const iterator& i, const difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      auto temp = i;
      temp -= n;
      return temp;
    }

    friend constexpr difference_type operator-(
      const iterator& i, const iterator& j) noexcept
    {
      return i.pos_ - j.pos_;
    }

    friend constexpr auto iter_move(const iterator& i) noexcept(
      noexcept(std::ranges::iter_move(i.current_))
      and std::is_nothrow_move_constructible_v<
        std::ranges::range_rvalue_reference_t<Base>>)
    {
      return std::tuple<difference_type, std::ranges::range_rvalue_reference_t<Base>>(
        i.pos_, std::ranges::iter_move(i.current_));
    }

  private:
    friend class enumerate_view;

    constexpr explicit iterator(
      std::ranges::iterator_t<Base> current, const difference_type pos)
      : current_{std::move(current)}
      , pos_{pos}
    {
    }

    std::ranges::iterator_t<Base> current_{};
    difference_type pos_{};
  };

  template <bool Const>
  class sentinel
  {
    using Base = detail::maybe_const<Const, V>;

  public:
    sentinel() = default;

    constexpr explicit sentinel(sentinel<!Const> i)
      requires Const
               && std::
                 convertible_to<std::ranges::sentinel_t<V>, std::ranges::sentinel_t<Base>>
      : end_{std::move(i.end_)}
    {
    }

    constexpr std::ranges::sentinel_t<Base> base() const { return end_; }

    friend constexpr bool operator==(const iterator<Const>& x, const sentinel& y)
    {
      return x.base() == y.end_;
    }

    template <bool OtherConst>
      requires std::sized_sentinel_for<
        std::ranges::sentinel_t<Base>,
        std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
    friend constexpr std::ranges::range_difference_t<detail::maybe_const<OtherConst, V>>
    operator-(const iterator<OtherConst>& x, const sentinel& y)
    {
      x.base() - y.base();
    }

    template <bool OtherConst>
      requires std::sized_sentinel_for<
        std::ranges::sentinel_t<Base>,
        std::ranges::iterator_t<detail::maybe_const<OtherConst, V>>>
    friend constexpr std::ranges::range_difference_t<detail::maybe_const<OtherConst, V>>
    operator-(const sentinel& y, const iterator<OtherConst>& x)
    {
      return y.base() - x.base();
    }

  private:
    friend class enumerate_view;

    constexpr explicit sentinel(std::ranges::sentinel_t<Base> end)
      : end_{std::move(end)}
    {
    }

    std::ranges::sentinel_t<Base> end_;
  };

  enumerate_view()
    requires std::default_initializable<V>
  = default;

  constexpr explicit enumerate_view(V base)
    : base_{std::move(base)}
  {
  }

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return base_;
  }

  constexpr V base() && { return std::move(base_); }

  constexpr auto begin()
    requires(!detail::simple_view<V>)
  {
    return iterator<false>(std::ranges::begin(base_), 0);
  }

  constexpr auto begin() const
    requires detail::range_with_movable_references<const V>
  {
    return iterator<true>(std::ranges::begin(base_), 0);
  }

  constexpr auto end()
    requires(!detail::simple_view<V>)
  {
    if constexpr (
      std::ranges::forward_range<V> && std::ranges::common_range<V>
      && std::ranges::sized_range<V>)
    {
      return iterator<false>(std::ranges::end(base_), std::ranges::distance(base_));
    }
    else
    {
      return sentinel<false>(std::ranges::end(base_));
    }
  }

  constexpr auto end() const
    requires detail::range_with_movable_references<const V>
  {
    if constexpr (
      std::ranges::forward_range<const V> and std::ranges::common_range<const V>
      and std::ranges::sized_range<const V>)
    {
      return iterator<true>(std::ranges::end(base_), std::ranges::distance(base_));
    }
    else
    {
      return sentinel<true>(std::ranges::end(base_));
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

template <typename R>
  requires std::ranges::input_range<R>
enumerate_view(R&& r) -> enumerate_view<std::ranges::views::all_t<R>>;

namespace views
{
namespace detail
{

struct enumerate_view_fn
{
  template <typename R>
  constexpr auto operator()(R&& r) const
  {
    return enumerate_view{std::forward<R>(r)};
  }

  template <std::ranges::input_range R>
  constexpr friend auto operator|(R&& r, const enumerate_view_fn&)
  {
    return enumerate_view{std::forward<R>(r)};
  }
};

} // namespace detail

constexpr detail::enumerate_view_fn enumerate;

} // namespace views
} // namespace ranges

namespace views
{

constexpr ranges::views::detail::enumerate_view_fn enumerate;

} // namespace views
} // namespace kdl

template <typename View>
constexpr bool std::ranges::enable_borrowed_range<kdl::ranges::enumerate_view<View>> =
  std::ranges::enable_borrowed_range<View>;
