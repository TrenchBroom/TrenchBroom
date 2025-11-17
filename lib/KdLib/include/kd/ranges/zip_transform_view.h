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

#include "detail/movable_box.h"
#include "detail/range_utils.h"
#include "zip_view.h"

#include <ranges>
#include <tuple>
#include <type_traits>

namespace kdl
{
namespace ranges
{
namespace detail
{

template <bool Const, typename F, typename Base, typename... Views>
consteval auto get_zip_transform_iter_cat()
{
  if constexpr (
    std::ranges::forward_range<Base>
    && !std::is_reference_v<std::invoke_result_t<
         detail::maybe_const<Const, F>&,
         std::ranges::range_reference_t<detail::maybe_const<Const, Views>>...>>)
  {
    return std::input_iterator_tag{};
  }
  else
  {
    return get_iter_cat<detail::maybe_const<Const, Base>>();
  }
}

template <typename F, typename IterTuple, std::size_t... Is>
constexpr auto zip_transform_nothrow_invocable_impl(const std::index_sequence<Is...>)
{
  return noexcept(
    std::
      is_nothrow_invocable_v<F, decltype(*std::get<Is>(std::declval<IterTuple>()))...>);
}

template <typename F, typename IterTuple>
constexpr auto zip_transform_nothrow_invocable()
{
  return noexcept(zip_transform_nothrow_invocable_impl<F, IterTuple>(
    std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<IterTuple>>>()));
}

} // namespace detail

template <std::move_constructible F, std::ranges::input_range... Views>

  requires(std::ranges::view<Views> && ...) && (sizeof...(Views) > 0)
          && std::is_object_v<F>
          && std::regular_invocable<F&, std::ranges::range_reference_t<Views>...>
          && detail::can_reference<
            std::invoke_result_t<F&, std::ranges::range_reference_t<Views>...>>
class zip_transform_view
  : public std::ranges::view_interface<zip_transform_view<F, Views...>>
{
private:
  using InnerView = zip_view<Views...>;

  template <bool Const>
  using ziperator = std::ranges::iterator_t<detail::maybe_const<Const, InnerView>>;

  template <bool Const>
  using zentinel = std::ranges::sentinel_t<detail::maybe_const<Const, InnerView>>;

public:
  template <bool Const>
  class iterator
  {
  private:
    using Parent = detail::maybe_const<Const, zip_transform_view>;
    using Base = detail::maybe_const<Const, InnerView>;

  public:
    using iterator_category =
      decltype(detail::get_zip_transform_iter_cat<Const, F, Base, Views...>());
    using iterator_concept = ziperator<Const>::iterator_concept;
    using value_type = std::remove_cvref_t<std::invoke_result_t<
      detail::maybe_const<Const, F>&,
      std::ranges::range_reference_t<detail::maybe_const<Const, Views>>...>>;
    using difference_type = std::ranges::range_difference_t<Base>;

    iterator() = default;

    constexpr explicit iterator(iterator<!Const> i)
      requires Const && std::convertible_to<ziperator<false>, ziperator<Const>>
      : parent_{i.parent_}
      , inner_{std::move(i.inner_)}
    {
    }

    constexpr const auto& inner() const& noexcept { return inner_; }

    constexpr decltype(auto) operator*() const
      noexcept(detail::zip_transform_nothrow_invocable<F, decltype(inner_.base())>())
    {
      return std::apply(
        [&](const auto&... iters) -> decltype(auto) {
          return std::invoke(*parent_->fun_, *iters...);
        },
        inner_.base());
    }

    constexpr decltype(auto) operator[](const difference_type n) const
      requires std::ranges::random_access_range<Base>
    {
      return std::apply(
        [&]<class... Is>(const Is&... iters) -> decltype(auto) {
          return std::invoke(*parent_->fun_, iters[std::iter_difference_t<Is>(n)]...);
        },
        inner_.base());
    }

    constexpr iterator& operator++()
    {
      ++inner_;
      return *this;
    }

    constexpr void operator++(int) { ++*this; }

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
      --inner_;
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
      inner_ += n;
      return *this;
    }

    constexpr iterator& operator-=(const difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      inner_ -= n;
      return *this;
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
      requires std::equality_comparable<ziperator<Const>>
    {
      return x.inner_ == y.inner_;
    }

    friend constexpr bool operator<(const iterator& lhs, const iterator& rhs)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      return lhs.inner_ < rhs.inner_;
    }

    friend constexpr bool operator>(const iterator& lhs, const iterator& rhs)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      return lhs.inner_ > rhs.inner_;
    }

    friend constexpr bool operator<=(const iterator& lhs, const iterator& rhs)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      return lhs.inner_ <= rhs.inner_;
    }

    friend constexpr bool operator>=(const iterator& lhs, const iterator& rhs)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      return lhs.inner_ >= rhs.inner_;
    }

    friend constexpr auto operator<=>(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
               && std::three_way_comparable<ziperator<Const>>
    {
      return x.inner_ <=> y.inner_;
    }

    friend constexpr iterator operator+(const iterator& i, difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      return iterator{i.parent_, i.inner_ + n};
    }

    friend constexpr iterator operator+(difference_type n, const iterator& i)
      requires std::ranges::random_access_range<Base>
    {
      return i + n;
    }

    friend constexpr iterator operator-(const iterator& i, difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      return iterator{i.parent_, i.inner_ - n};
    }

    friend constexpr difference_type operator-(const iterator& i, const iterator& j)
      requires std::sized_sentinel_for<ziperator<Const>, ziperator<Const>>
    {
      return i.inner_ - j.inner_;
    }

  private:
    friend class zip_transform_view;

    template <bool>
    friend class sentinel;

    constexpr iterator(Parent* parent, ziperator<Const> inner)
      : parent_{parent}
      , inner_{std::move(inner)}
    {
    }

    Parent* parent_{nullptr};
    ziperator<Const> inner_{};
  };

  template <bool Const>
  class sentinel
  {
  public:
    sentinel() = default;

    constexpr explicit sentinel(sentinel<!Const> i)
      requires Const && std::convertible_to<zentinel<false>, zentinel<Const>>
      : inner_{std::move(i.inner_)}
    {
    }

    template <bool OtherConst>
      requires std::sentinel_for<zentinel<Const>, ziperator<OtherConst>>
    friend constexpr bool operator==(const iterator<OtherConst>& x, const sentinel& y)
    {
      return x.inner() == y.inner_;
    }

    template <bool OtherConst>
      requires std::sized_sentinel_for<zentinel<Const>, ziperator<OtherConst>>
    friend constexpr std::ranges::range_difference_t<
      detail::maybe_const<OtherConst, InnerView>>
    operator-(const iterator<OtherConst>& x, const sentinel& y)
    {
      return x.inner() - y.inner_;
    }

    template <bool OtherConst>
      requires std::sized_sentinel_for<zentinel<Const>, ziperator<OtherConst>>
    friend constexpr std::ranges::range_difference_t<
      detail::maybe_const<OtherConst, InnerView>>
    operator-(const sentinel& y, const iterator<OtherConst>& x)
    {
      return y.inner_ - x.inner();
    }

  private:
    friend class zip_transform_view;

    constexpr explicit sentinel(zentinel<Const> inner)
      : inner_{std::move(inner)}
    {
    }

    zentinel<Const> inner_{};
  };

  zip_transform_view() = default;

  constexpr explicit zip_transform_view(F fun, Views... views)
    : zip_{std::move(views)...}
    , fun_{std::move(fun)}
  {
  }

  constexpr auto begin() { return iterator<false>{this, zip_.begin()}; }

  constexpr auto begin() const
    requires std::ranges::range<const zip_view<Views...>>
  {
    return iterator<true>{this, zip_.begin()};
  }

  constexpr auto end()
  {
    if constexpr (std::ranges::common_range<InnerView>)
    {
      return iterator<false>{this, zip_.end()};
    }
    else
    {
      return sentinel<false>{zip_.end()};
    }
  }

  constexpr auto end() const
    requires std::ranges::range<const InnerView>
             && std::
               regular_invocable<const F&, std::ranges::range_reference_t<const Views>...>
  {
    if constexpr (std::ranges::common_range<const InnerView>)
    {
      return iterator<true>{this, zip_.end()};
    }
    else
    {
      return sentinel<true>{zip_.end()};
    }
  }

  constexpr auto size()
    requires std::ranges::sized_range<InnerView>
  {
    return zip_.size();
  }

  constexpr auto size() const
    requires std::ranges::sized_range<const InnerView>
  {
    return zip_.size();
  }

private:
  InnerView zip_{};
  movable_box<F> fun_{};
};

template <typename F, typename... Rs>
zip_transform_view(F, Rs&&...) -> zip_transform_view<F, std::views::all_t<Rs>...>;

namespace views
{

template <typename F, typename... R>
  requires(std::ranges::input_range<R> && ...)
auto zip_transform(F&& f, R&&... r)
{
  return ranges::zip_transform_view{std::forward<F>(f), std::forward<R>(r)...};
};

} // namespace views
} // namespace ranges

namespace views
{

template <typename F, typename... R>
  requires(std::ranges::input_range<R> && ...)
auto zip_transform(F&& f, R&&... r)
{
  return ranges::views::zip_transform(std::forward<F>(f), std::forward<R>(r)...);
};

} // namespace views
} // namespace kdl

template <typename F, typename... Views>
constexpr bool
  std::ranges::enable_borrowed_range<kdl::ranges::zip_transform_view<F, Views...>> =
    (std::ranges::enable_borrowed_range<Views> && ...);
