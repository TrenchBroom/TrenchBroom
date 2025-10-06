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

#include "detail/integer_like.h"
#include "detail/iota_diff.h"
#include "detail/movable_box.h"

#include <ranges>
#include <tuple>

namespace kdl
{
namespace ranges
{
namespace detail
{

template <typename T>
concept integer_like_with_usable_difference_type =
  is_signed_integer_like_v<T> || (is_integer_like_v<T> && std::weakly_incrementable<T>);

} // namespace detail

template <std::move_constructible W, std::semiregular Bound = std::unreachable_sentinel_t>
  requires(
    std::is_object_v<W> && std::same_as<W, std::remove_cv_t<W>>
    && (detail::integer_like_with_usable_difference_type<Bound> || std::same_as<Bound, std::unreachable_sentinel_t>))
class repeat_view : public std::ranges::view_interface<repeat_view<W, Bound>>
{
public:
  struct iterator
  {
    using index_type = std::conditional_t<
      std::same_as<Bound, std::unreachable_sentinel_t>,
      std::ptrdiff_t,
      Bound>;

    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = W;
    using difference_type = std::conditional_t<
      detail::is_signed_integer_like_v<index_type>,
      index_type,
      iota_diff_t<index_type>>;

    iterator() = default;

    constexpr explicit iterator(const W* value, index_type b = index_type{})
      : value_{value}
      , current_{b}
    {
    }

    constexpr const W& operator*() const noexcept { return *value_; }

    constexpr const W& operator[](difference_type n) const noexcept
    {
      return *(*this + n);
    }

    constexpr iterator& operator++()
    {
      ++current_;
      return *this;
    }

    constexpr iterator operator++(int)
    {
      const auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator& operator--()
    {
      --current_;
      return *this;
    }

    constexpr iterator operator--(int)
    {
      const auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator& operator+=(const difference_type n)
    {
      current_ += n;
      return *this;
    }

    constexpr iterator& operator-=(const difference_type n)
    {
      current_ -= n;
      return *this;
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
    {
      return x.current_ == y.current_;
    }

    friend constexpr auto operator<=>(const iterator& x, const iterator& y)
    {
      return x.current_ <=> y.current_;
    }

    friend constexpr iterator operator+(iterator i, const difference_type n)
    {
      i += n;
      return i;
    }

    friend constexpr iterator operator+(const difference_type n, iterator i)
    {
      i += n;
      return i;
    }

    friend constexpr iterator operator-(iterator i, const difference_type n)
    {
      i -= n;
      return i;
    }

    friend constexpr difference_type operator-(const iterator& x, const iterator& y)
    {
      return static_cast<difference_type>(x.current_)
             - static_cast<difference_type>(y.current_);
    }

  private:
    const W* value_{};
    index_type current_{};
  };

  repeat_view()
    requires std::default_initializable<W>
  = default;

  constexpr explicit repeat_view(const W& value, Bound bound = Bound{})
    : value_{value}
    , bound_{bound}
  {
  }

  constexpr explicit repeat_view(W&& value, Bound bound = Bound())
    : value_{std::move(value)}
    , bound_{bound}
  {
  }

  template <class... WArgs, class... BoundArgs>
    requires std::constructible_from<W, WArgs...>
               && std::constructible_from<Bound, BoundArgs...>
  constexpr explicit repeat_view(
    std::piecewise_construct_t,
    std::tuple<WArgs...> value_args,
    std::tuple<BoundArgs...> bound_args = std::tuple<>{})
    : value_{std::make_from_tuple<W>(std::move(value_args))}
    , bound_{std::make_from_tuple<Bound>(std::move(bound_args))}
  {
  }

  constexpr iterator begin() const { return iterator{std::addressof(*value_)}; }

  constexpr iterator end() const
    requires(!std::same_as<Bound, std::unreachable_sentinel_t>)
  {
    return iterator{std::addressof(*value_), bound_};
  }

  constexpr std::unreachable_sentinel_t end() const { return std::unreachable_sentinel; }

  constexpr auto size() const
    requires(!std::same_as<Bound, std::unreachable_sentinel_t>)
  {
    return static_cast<std::make_unsigned_t<Bound>>(bound_);
  }

private:
  movable_box<W> value_;
  Bound bound_;
};

template <class W, class Bound = std::unreachable_sentinel_t>
repeat_view(W, Bound = Bound()) -> repeat_view<W, Bound>;

namespace views
{

template <typename W>
constexpr auto repeat(W&& value)
{
  return ranges::repeat_view{std::forward<W>(value)};
};

template <typename W, typename Bound>
constexpr auto repeat(W&& value, Bound&& bound)
{
  return ranges::repeat_view{std::forward<W>(value), std::forward<Bound>(bound)};
};

} // namespace views
} // namespace ranges

namespace views
{

template <typename W>
constexpr auto repeat(W&& value)
{
  return ranges::repeat_view{std::forward<W>(value)};
};

template <typename W, typename Bound>
constexpr auto repeat(W&& value, Bound&& bound)
{
  return ranges::repeat_view{std::forward<W>(value), std::forward<Bound>(bound)};
};

} // namespace views
} // namespace kdl
