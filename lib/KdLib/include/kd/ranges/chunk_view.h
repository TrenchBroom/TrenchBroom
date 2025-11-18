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

#include "detail/non_propagating_cache.h"
#include "detail/range_utils.h"

#include <algorithm>
#include <ranges>

namespace kdl
{
namespace ranges
{

template <std::ranges::view V>
  requires std::ranges::input_range<V>
class chunk_view : public std::ranges::view_interface<chunk_view<V>>
{
public:
  struct inner_iterator;

  struct outer_iterator
  {
    using iterator_concept = std::input_iterator_tag;
    using difference_type = std::ranges::range_difference_t<V>;

    struct value_type : public std::ranges::view_interface<value_type>
    {
      constexpr inner_iterator begin() const noexcept { return inner_iterator{*parent_}; }

      constexpr std::default_sentinel_t end() const noexcept
      {
        return std::default_sentinel;
      }

      constexpr auto size() const
        requires std::
          sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
      {
        const auto s = std::ranges::min(
          parent_->remainder_, std::ranges::end(parent_->base_) - *parent_->current_);
        return std::make_unsigned_t<decltype(s)>(s);
      }

      constexpr explicit value_type(chunk_view& parent)
        : parent_{std::addressof(parent)}
      {
      }

      chunk_view* parent_{nullptr};
    };

    outer_iterator(outer_iterator&&) = default;
    outer_iterator& operator=(outer_iterator&&) = default;

    constexpr value_type operator*() const { return value_type{*parent_}; }

    constexpr outer_iterator& operator++()
    {
      std::ranges::advance(
        *parent_->current_, parent_->remainder_, std::ranges::end(parent_->base_));
      parent_->remainder_ = parent_->n_;
      return *this;
    }

    constexpr void operator++(int) { ++*this; }

    friend constexpr bool operator==(
      const outer_iterator& i, const std::default_sentinel_t)
    {
      return *i.parent_->current_ == std::ranges::end(i.parent_->base_)
             && i.parent_->remainder_ == 0;
    }

    friend constexpr difference_type operator-(
      const std::default_sentinel_t, const outer_iterator& i)
      requires std::
        sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
    {
      const auto dist = std::ranges::end(i->parent_->base_) - *i.parent->current_;
      return dist < i.parent_->remainder
               ? dist == 0 ? 0 : 1
               : detail::div_ceil(dist - i.parent_->remainder_, i.parent->n_) + 1;
    }

    friend constexpr difference_type operator-(
      const outer_iterator& i, const std::default_sentinel_t s)
      requires std::
        sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
    {
      return -(s - i);
    }

    constexpr explicit outer_iterator(chunk_view& parent)
      : parent_{std::addressof(parent)}
    {
    }

    chunk_view* parent_{nullptr};
  };

  struct inner_iterator
  {
    using iterator_concept = std::input_iterator_tag;
    using difference_type = std::ranges::range_difference_t<V>;
    using value_type = std::ranges::range_value_t<V>;

    inner_iterator(inner_iterator&&) = default;
    inner_iterator& operator=(inner_iterator&&) = default;

    constexpr const std::ranges::iterator_t<V>& base() const&
    {
      return *parent_->current_;
    }

    constexpr std::ranges::range_reference_t<V> operator*() const
    {
      return **parent_->current_;
    }

    constexpr inner_iterator& operator++()
    {
      ++*parent_->current_;
      if (*parent_->current_ == std::ranges::end(parent_->base_))
      {
        parent_->remainder_ = 0;
      }
      else
      {
        --parent_->remainder_;
      }
      return *this;
    }

    constexpr void operator++(int) { ++*this; }

    friend constexpr bool operator==(
      const inner_iterator& i, const std::default_sentinel_t)
    {
      return i.parent_->remainder_ == 0;
    }

    friend constexpr difference_type operator-(
      const std::default_sentinel_t, const inner_iterator& i)
      requires std::
        sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
    {
      return std::ranges::min(
        i.parent_->remainder_, std::ranges::end(i.parent_->base_) - *i.parent_->current_);
    }

    friend constexpr difference_type operator-(
      const inner_iterator& i, const std::default_sentinel_t s)
      requires std::
        sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
    {
      return -(s - i);
    }

    friend constexpr auto iter_move(const inner_iterator& i) noexcept(
      noexcept(std::ranges::iter_move(i.base())))
    {
      return std::ranges::iter_move(i.base());
    }

    friend constexpr void iter_swap(
      const inner_iterator& x,
      const inner_iterator&
        y) noexcept(noexcept(std::ranges::
                               iter_swap(*x.parent_->current_, *y.parent_->current_)))
      requires std::indirectly_swappable<std::ranges::iterator_t<V>>
    {
      std::ranges::iter_swap(*x.parent_->current_, *y.parent_->current_);
    }

    constexpr explicit inner_iterator(chunk_view& parent)
      : parent_{std::addressof(parent)}
    {
    }

    chunk_view* parent_{nullptr};
  };

  constexpr explicit chunk_view(V base, std::ranges::range_difference_t<V> n)
    : base_{std::move(base)}
    , n_{n}
    , remainder_{0}
  {
  }

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return base_;
  }

  constexpr V base() && { return std::move(base_); }

  constexpr outer_iterator begin()
  {
    current_ = std::ranges::begin(base_);
    remainder_ = n_;
    return outer_iterator{*this};
  }

  constexpr std::default_sentinel_t end() const noexcept { return std::default_sentinel; }

  constexpr auto size()
    requires std::ranges::sized_range<V>
  {
    const auto s = detail::div_ceil(std::ranges::distance(base_), n_);
    return std::make_unsigned_t<decltype(s)>(s);
  }

  constexpr auto size() const
    requires std::ranges::sized_range<const V>
  {
    const auto s = detail::div_ceil(std::ranges::distance(base_), n_);
    return std::make_unsigned_t<decltype(s)>(s);
  }

  V base_{};
  std::ranges::range_difference_t<V> n_{};
  std::ranges::range_difference_t<V> remainder_{};
  non_propagating_cache<std::ranges::iterator_t<V>> current_{};
};

template <std::ranges::view V>
  requires std::ranges::forward_range<V>
class chunk_view<V> : public std::ranges::view_interface<chunk_view<V>>
{
public:
  template <bool Const>
  struct iterator
  {
    using Parent = detail::maybe_const<Const, chunk_view>;
    using Base = detail::maybe_const<Const, V>;

    using iterator_category = std::input_iterator_tag;
    using iterator_concept = decltype(detail::get_iter_cat<V>());
    using value_type = decltype(std::views::take(
      std::ranges::subrange(
        std::declval<std::ranges::iterator_t<Base>>(),
        std::declval<std::ranges::sentinel_t<Base>>()),
      std::declval<std::ranges::range_difference_t<Base>>()));
    using difference_type = std::ranges::range_difference_t<Base>;

    iterator() = default;

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
      , n_{i.n_}
      , missing_{i.missing_}
    {
    }

    constexpr std::ranges::iterator_t<Base> base() const { return current_; }

    constexpr value_type operator*() const
    {
      return std::ranges::views::take(std::ranges::subrange(current_, end_), n_);
    }

    constexpr value_type operator[](const difference_type pos) const
      requires std::ranges::random_access_range<Base>
    {
      return *(*this + pos);
    }

    constexpr iterator& operator++()
    {
      missing_ = std::ranges::advance(current_, n_, end_);
      return *this;
    }

    constexpr iterator operator++(int)
    {
      const auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator& operator--()
      requires std::ranges::bidirectional_range<Base>
    {
      std::ranges::advance(current_, missing_ - n_);
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

    constexpr iterator& operator+=(const difference_type x)
      requires std::ranges::random_access_range<Base>
    {
      if (x > 0)
      {
        std::ranges::advance(current_, n_ * (x - 1));
        missing_ = std::ranges::advance(current_, n_, end_);
      }
      else if (x < 0)
      {
        std::ranges::advance(current_, n_ * x + missing_);
        missing_ = 0;
      }
      return *this;
    }

    constexpr iterator& operator-=(const difference_type x)
      requires std::ranges::random_access_range<Base>
    {
      return *this += -x;
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
    {
      return x.current_ == y.current_;
    }

    friend constexpr bool operator==(const iterator& x, const std::default_sentinel_t)
    {
      return x.current_ == x.end_;
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

    friend constexpr iterator operator+(const iterator& i, const difference_type pos)
      requires std::ranges::random_access_range<Base>
    {
      auto r = i;
      r += pos;
      return r;
    }

    friend constexpr iterator operator+(const difference_type pos, const iterator& i)
      requires std::ranges::random_access_range<Base>
    {
      return i + pos;
    }

    friend constexpr iterator operator-(const iterator& i, const difference_type pos)
      requires std::ranges::random_access_range<Base>
    {
      auto r = i;
      r -= pos;
      return r;
    }

    friend constexpr difference_type operator-(const iterator& i, const iterator& j)
      requires std::
        sized_sentinel_for<std::ranges::iterator_t<Base>, std::ranges::iterator_t<Base>>
    {
      return (i.current_ - j.current_ + i.missing_ - j.missing_) / i.n_;
    }

    friend constexpr difference_type operator-(
      const std::default_sentinel_t, const iterator& i)
      requires std::
        sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base>>
    {
      return detail::div_ceil(i.end_ - i.current_, i.n_);
    }

    friend constexpr difference_type operator-(
      const iterator& i, const std::default_sentinel_t s)
      requires std::
        sized_sentinel_for<std::ranges::sentinel_t<Base>, std::ranges::iterator_t<Base>>
    {
      return -(s - i);
    }

    constexpr iterator(
      Parent* parent,
      std::ranges::iterator_t<Base> current,
      std::ranges::range_difference_t<Base> missing = 0)
      : current_{std::move(current)}
      , end_{std::ranges::end(parent->base_)}
      , n_{parent->n_}
      , missing_{missing}
    {
    }

    std::ranges::iterator_t<Base> current_{};
    std::ranges::sentinel_t<Base> end_{};
    std::ranges::range_difference_t<Base> n_{};
    std::ranges::range_difference_t<Base> missing_{};
  };

  constexpr explicit chunk_view(V base, const std::ranges::range_difference_t<V> n)
    : base_{std::move(base)}
    , n_{n}
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
    return iterator<false>{this, std::ranges::begin(base_)};
  }

  constexpr auto begin() const
    requires std::ranges::forward_range<const V>
  {
    return iterator<true>{this, std::ranges::begin(base_)};
  }

  constexpr auto end()
    requires(!detail::simple_view<V>)
  {
    if constexpr (std::ranges::common_range<V> && std::ranges::sized_range<V>)
    {
      const auto missing = (n_ - std::ranges::distance(base_) % n_) % n_;
      return iterator<false>{this, std::ranges::end(base_), missing};
    }
    else if constexpr (
      std::ranges::common_range<V> && !std::ranges::bidirectional_range<V>)
    {
      return iterator<false>{this, std::ranges::end(base_)};
    }
    else
    {
      return std::default_sentinel;
    }
  }

  constexpr auto end() const
    requires std::ranges::forward_range<const V>
  {
    if constexpr (std::ranges::common_range<const V> && std::ranges::sized_range<const V>)
    {
      auto missing = (n_ - std::ranges::distance(base_) % n_) % n_;
      return iterator<true>{this, std::ranges::end(base_), missing};
    }
    else if constexpr (
      std::ranges::common_range<const V> && !std::ranges::bidirectional_range<const V>)
    {
      return iterator<true>{this, std::ranges::end(base_)};
    }
    else
    {
      return std::default_sentinel;
    }
  }

  constexpr auto size()
    requires std::ranges::sized_range<V>
  {
    const auto s = detail::div_ceil(std::ranges::distance(base_), n_);
    return std::make_unsigned_t<decltype(s)>(s);
  }

  constexpr auto size() const
    requires std::ranges::sized_range<const V>
  {
    const auto s = detail::div_ceil(std::ranges::distance(base_), n_);
    return std::make_unsigned_t<decltype(s)>(s);
  }

private:
  V base_{};
  std::ranges::range_difference_t<V> n_{};
};

template <class R>
chunk_view(R&&, std::ranges::range_difference_t<R>) -> chunk_view<std::views::all_t<R>>;

namespace views
{

template <std::ranges::viewable_range R>
constexpr auto chunk(R&& r, std::ranges::range_difference_t<R> n)
{
  return ranges::chunk_view{std::forward<R>(r), n};
};

namespace detail
{

template <typename DifferenceType>
struct chunk_view_helper
{
  DifferenceType n;
};

template <typename DifferenceType>
chunk_view_helper(DifferenceType) -> chunk_view_helper<DifferenceType>;

template <std::ranges::viewable_range R, typename DifferenceType>
auto operator|(R&& r, const chunk_view_helper<DifferenceType>& h)
{
  return chunk(std::forward<R>(r), static_cast<std::ranges::range_difference_t<R>>(h.n));
}

} // namespace detail

template <typename DifferenceType>
constexpr auto chunk(const DifferenceType n)
{
  return detail::chunk_view_helper{n};
};

} // namespace views
} // namespace ranges

namespace views
{

template <std::ranges::viewable_range R>
constexpr auto chunk(R&& r, std::ranges::range_difference_t<R> n)
{
  return ranges::views::chunk(std::forward<R>(r), n);
};

template <typename DifferenceType>
constexpr auto chunk(const DifferenceType n)
{
  return ranges::views::chunk(n);
};

} // namespace views
} // namespace kdl

template <typename V>
constexpr bool std::ranges::enable_borrowed_range<kdl::ranges::chunk_view<V>> =
  std::ranges::enable_borrowed_range<V>;
