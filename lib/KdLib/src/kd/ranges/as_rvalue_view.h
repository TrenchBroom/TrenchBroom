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

template <std::ranges::view V>
  requires std::ranges::input_range<V>
class as_rvalue_view : public std::ranges::view_interface<as_rvalue_view<V>>
{
public:
  as_rvalue_view()
    requires std::default_initializable<V>
  = default;

  constexpr explicit as_rvalue_view(V base)
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
    return std::move_iterator{std::ranges::begin(base_)};
  }

  constexpr auto begin() const
    requires std::ranges::range<const V>
  {
    return std::move_iterator{std::ranges::begin(base_)};
  }

  constexpr auto end()
    requires(!detail::simple_view<V>)
  {
    if constexpr (std::ranges::common_range<V>)
    {
      return std::move_iterator{std::ranges::end(base_)};
    }
    else
    {
      return std::move_sentinel{std::ranges::end(base_)};
    }
  }

  constexpr auto end() const
    requires std::ranges::range<const V>
  {
    if constexpr (std::ranges::common_range<V>)
    {
      return std::move_iterator{std::ranges::end(base_)};
    }
    else
    {
      return std::move_sentinel{std::ranges::end(base_)};
    }
  }

  constexpr auto size()
    requires std::ranges::sized_range<V>
  {
    return const_cast<const as_rvalue_view*>(this)->size();
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
as_rvalue_view(R&& r) -> as_rvalue_view<std::ranges::views::all_t<R>>;

namespace views
{
namespace detail
{

struct as_rvalue_view_fn
{
  template <typename R>
  constexpr auto operator()(R&& r) const
  {
    return as_rvalue_view{std::forward<R>(r)};
  }

  template <std::ranges::input_range R>
  constexpr friend auto operator|(R&& r, const as_rvalue_view_fn&)
  {
    return as_rvalue_view{std::forward<R>(r)};
  }
};

} // namespace detail

constexpr detail::as_rvalue_view_fn as_rvalue;

} // namespace views
} // namespace ranges

namespace views
{

constexpr ranges::views::detail::as_rvalue_view_fn as_rvalue;

} // namespace views
} // namespace kdl

template <typename View>
constexpr bool std::ranges::enable_borrowed_range<kdl::ranges::as_rvalue_view<View>> =
  std::ranges::enable_borrowed_range<View>;
