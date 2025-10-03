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

template <bool B, bool Const, typename V>
struct stride_iterator_category
{
private:
  using C = std::iterator_traits<
    std::ranges::iterator_t<detail::maybe_const<Const, V>>>::iterator_category;

public:
  using iterator_category = std::conditional_t<
    std::derived_from<C, std::random_access_iterator_tag>,
    std::random_access_iterator_tag,
    C>;
};

template <bool Const, typename V>
struct stride_iterator_category<false, Const, V>
{
};

} // namespace detail

template <std::ranges::input_range V>
  requires std::ranges::view<V>
class stride_view : public std::ranges::view_interface<stride_view<V>>
{
public:
  template <bool Const>
  class iterator : public detail::stride_iterator_category<
                     std::ranges::forward_range<detail::maybe_const<Const, V>>,
                     Const,
                     V>
  {
    using Parent = detail::maybe_const<Const, stride_view>;
    using Base = detail::maybe_const<Const, V>;

  public:
    using iterator_concept = decltype(detail::get_iter_cat<V>());
    using value_type = std::ranges::range_value_t<Base>;
    using difference_type = std::ranges::range_difference_t<Base>;

    iterator()
      requires std::default_initializable<std::ranges::iterator_t<Base>>
    = default;

    constexpr explicit iterator(iterator<!Const> i)
      requires Const
                 && std::convertible_to<
                   std::ranges::iterator_t<V>,
                   std::ranges::iterator_t<Base>>
                 && std::convertible_to<
                   std::ranges::sentinel_t<V>,
                   std::ranges::sentinel_t<Base>>
      : current_{std::move(i.current_)}
      , end_{std::move(i.end_)}
      , stride_{i.stride_}
      , missing_{i.missing_}
    {
    }

    constexpr std::ranges::iterator_t<Base> base() && { return std::move(current_); }

    constexpr const std::ranges::iterator_t<Base>& base() const& noexcept
    {
      return current_;
    }

    constexpr decltype(auto) operator*() const { return *current_; }

    constexpr decltype(auto) operator[](difference_type n) const
      requires std::ranges::random_access_range<Base>
    {
      return *(*this + n);
    }

    constexpr iterator& operator++()
    {
      missing_ = std::ranges::advance(current_, stride_, end_);
      return *this;
    }

    constexpr void operator++(int) { return ++*this; }

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
      std::ranges::advance(current_, missing_ - stride_);
      missing_ = 0;
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
      if (n > 0)
      {
        std::ranges::advance(current_, stride_ * (n - 1));
        missing_ = std::ranges::advance(current_, stride_, end_);
      }
      else if (n < 0)
      {
        std::ranges::advance(current_, stride_ * n + missing_);
        missing_ = 0;
      }

      return *this;
    }

    constexpr iterator& operator-=(const difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      return *this += -n;
    }

    friend constexpr bool operator==(const iterator& x, const std::default_sentinel_t)
    {
      return x.current_ == x.end_;
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
      requires std::equality_comparable<std::ranges::iterator_t<Base>>
    {
      return x.current_ == y.current_;
    }

    friend constexpr bool operator<(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
    {
      return x.current_ < y.current_;
    }

    friend constexpr bool operator>(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
    {
      return y < x;
    }

    friend constexpr bool operator<=(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
    {
      return !(y < x);
    }

    friend constexpr bool operator>=(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
    {
      return !(x < y);
    }

    friend constexpr auto operator<=>(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
               && std::three_way_comparable<std::ranges::iterator_t<Base>>
    {
      return x.current_ <=> y.current_;
    }

    friend constexpr iterator operator+(const iterator& i, const difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      auto r = i;
      r += n;
      return r;
    }

    friend constexpr iterator operator+(const difference_type n, const iterator& i)
      requires std::ranges::random_access_range<Base>
    {
      return i + n;
    }

    friend constexpr iterator operator-(const iterator& i, const difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      auto r = i;
      r -= n;
      return r;
    }

    friend constexpr difference_type operator-(const iterator& x, const iterator& y)
      requires std::
        sized_sentinel_for<std::ranges::iterator_t<Base>, std::ranges::iterator_t<Base>>
    {
      const auto n = x.current_ - y.current_;
      if constexpr (std::ranges::forward_range<Base>)
      {
        return (n + x.missing_ - y.missing_) / x.stride_;
      }
      else
      {
        return n < 0 ? -detail::div_ceil(-n, x.stride_) : detail::div_ceil(n, x.stride_);
      }
    }

    friend constexpr difference_type operator-(
      const std::default_sentinel_t, const iterator& x)
      requires std::
        sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base>>
    {
      return detail::div_ceil(x.end_ - x.current_, x.stride_);
    }

    friend constexpr difference_type operator-(
      const iterator& x, const std::default_sentinel_t s)
      requires std::
        sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base>>
    {
      return -(s - x);
    }

    friend constexpr std::ranges::range_rvalue_reference_t<Base> iter_move(
      const iterator& i) noexcept(noexcept(std::ranges::iter_move(i.current_)))
    {
      return std::ranges::iter_move(i.current_);
    }

    friend constexpr void iter_swap(const iterator& x, const iterator& y)

      noexcept(noexcept(std::ranges::iter_swap(x.current_, y.current_)))
      requires std::indirectly_swappable<std::ranges::iterator_t<Base>>
    {
      std::ranges::iter_swap(x.current_, y.current_);
    }

  private:
    friend class stride_view;

    constexpr iterator(
      Parent& parent,
      std::ranges::iterator_t<Base> current,
      std::ranges::range_difference_t<Base> missing = 0)
      : current_{std::move(current)}
      , end_{std::ranges::end(parent.base_)}
      , stride_{parent.stride_}
      , missing_{missing}
    {
    }

    std::ranges::iterator_t<Base> current_{};
    std::ranges::sentinel_t<Base> end_{};
    std::ranges::range_difference_t<Base> stride_{0};
    std::ranges::range_difference_t<Base> missing_{0};
  };

  constexpr explicit stride_view(V base, const std::ranges::range_difference_t<V> stride)
    : base_{std::move(base)}
    , stride_{stride}
  {
  }

  constexpr std::ranges::range_difference_t<V> stride() const noexcept { return stride_; }

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return base_;
  }

  constexpr V base() && { return std::move(base_); }

  constexpr auto begin()
    requires(!detail::simple_view<V>)
  {
    return iterator<false>{*this, std::ranges::begin(base_)};
  }

  constexpr auto begin() const
    requires std::ranges::range<const V>
  {
    return iterator<true>{*this, std::ranges::begin(base_)};
  }

  constexpr auto end()
    requires(!detail::simple_view<V>)
  {
    return end_impl<false, V>(*this);
  }

  constexpr auto end() const
    requires std::ranges::range<const V>
  {
    return end_impl<true, const V>(*this);
  }

  constexpr auto size()
    requires std::ranges::sized_range<V>
  {
    return const_cast<const stride_view*>(this)->size();
  }

  constexpr auto size() const
    requires std::ranges::sized_range<const V>
  {
    const auto s = detail::div_ceil(std::ranges::distance(base_), stride_);
    return std::make_unsigned_t<decltype(s)>(s);
  }

private:
  template <bool Const>
  friend class iterator;

  template <bool Const, typename Base>
  constexpr auto end_impl(detail::maybe_const<Const, stride_view>& parent) const
  {
    if constexpr (
      std::ranges::common_range<Base> && std::ranges::sized_range<Base>
      && std::ranges::forward_range<Base>)
    {
      const auto missing =
        (parent.stride_ - std::ranges::distance(parent.base_) % parent.stride_)
        % parent.stride_;
      return iterator<Const>{parent, std::ranges::end(parent.base_), missing};
    }
    else if constexpr (
      std::ranges::common_range<Base> && !std::ranges::bidirectional_range<Base>)
    {
      return iterator<Const>{parent, std::ranges::end(parent.base_)};
    }
    else
    {
      return std::default_sentinel;
    }
  }

  V base_{};
  std::ranges::range_difference_t<V> stride_;
};

template <class R>
stride_view(R&&, std::ranges::range_difference_t<R>) -> stride_view<std::views::all_t<R>>;

namespace views
{

template <std::ranges::viewable_range R>
constexpr auto stride(R&& r, std::ranges::range_difference_t<R> n)
{
  return ranges::stride_view{std::forward<R>(r), n};
};

namespace detail
{

template <typename DifferenceType>
struct stride_view_helper
{
  DifferenceType n;
};

template <typename DifferenceType>
stride_view_helper(DifferenceType) -> stride_view_helper<DifferenceType>;

template <std::ranges::viewable_range R, typename DifferenceType>
auto operator|(R&& r, const stride_view_helper<DifferenceType>& h)
{
  return stride(r, static_cast<std::ranges::range_difference_t<R>>(h.n));
}

} // namespace detail

template <typename DifferenceType>
constexpr auto stride(const DifferenceType n)
{
  return detail::stride_view_helper{n};
};

} // namespace views
} // namespace ranges

namespace views
{

template <std::ranges::viewable_range R>
constexpr auto stride(R&& r, std::ranges::range_difference_t<R> n)
{
  return ranges::views::stride(std::forward<R>(r), n);
};

template <typename DifferenceType>
constexpr auto stride(const DifferenceType n)
{
  return ranges::views::stride(n);
};

} // namespace views
} // namespace kdl

template <typename V>
constexpr bool std::ranges::enable_borrowed_range<kdl::ranges::stride_view<V>> =
  std::ranges::enable_borrowed_range<V>;
