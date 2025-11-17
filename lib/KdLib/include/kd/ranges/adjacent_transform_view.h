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

#include "adjacent_view.h"
#include "detail/movable_box.h"
#include "detail/range_utils.h"

#include <ranges>
#include <tuple>
#include <type_traits>

namespace kdl
{
namespace ranges
{
namespace detail
{

template <typename F, typename V, std::size_t N>
struct adjacent_transform_invocable
{
private:
  using Tuple = tuple_repeat_t<V, N>;

  template <std::size_t... Is>
  constexpr static auto helper(const std::index_sequence<Is...>)
  {
    return std::regular_invocable<F, decltype(std::get<Is>(std::declval<Tuple>()))...>;
  }

public:
  constexpr static auto value = helper(std::make_index_sequence<N>());
};

template <typename F, typename V, std::size_t N>
constexpr static auto adjacent_transform_invocable_v =
  adjacent_transform_invocable<F, V, N>::value;

template <typename F, typename V, std::size_t N>
struct adjacent_transform_invoke_result
{
private:
  using Tuple = tuple_repeat_t<V, N>;

  template <std::size_t... Is>
  static auto helper(std::index_sequence<Is...>)
    -> std::invoke_result_t<F&, decltype(std::get<Is>(std::declval<Tuple>()))...>;

public:
  using type = decltype(helper(std::make_index_sequence<N>()));
};

template <typename F, typename V, std::size_t N>
using adjacent_transform_invoke_result_t =
  typename adjacent_transform_invoke_result<F, V, N>::type;

template <bool Const, typename F, typename Base, std::size_t N>
consteval auto get_adjacent_transform_iter_cat()
{
  if constexpr (
    !std::is_reference_v<
      adjacent_transform_invoke_result_t<F, std::ranges::range_reference_t<Base>, N>>)
  {
    return std::input_iterator_tag{};
  }
  else
  {
    return get_iter_cat<Base>;
  }
}

template <typename F, typename IterArray, std::size_t... Is>
constexpr auto adjacent_transform_nothrow_invocable_impl(const std::index_sequence<Is...>)
{
  return noexcept(
    std::
      is_nothrow_invocable_v<F, decltype(*std::get<Is>(std::declval<IterArray>()))...>);
}

template <typename F, typename IterArray>
constexpr auto adjacent_transform_nothrow_invocable()
{
  return noexcept(adjacent_transform_nothrow_invocable_impl<F, IterArray>(
    std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<IterArray>>>()));
}

} // namespace detail

template <std::ranges::forward_range V, std::move_constructible F, std::size_t N>
  requires std::ranges::view<V> && (N > 0) && std::is_object_v<F>
           && detail::
             adjacent_transform_invocable_v<F&, std::ranges::range_reference_t<V>, N>
           && detail::can_reference<detail::adjacent_transform_invoke_result_t<
             F&,
             std::ranges::range_reference_t<V>,
             N>>
class adjacent_transform_view
  : public std::ranges::view_interface<adjacent_transform_view<V, F, N>>
{
  using InnerView = adjacent_view<V, N>;
  template <bool Const>
  using inner_iterator = std::ranges::iterator_t<detail::maybe_const<Const, InnerView>>;
  template <bool Const>
  using inner_sentinel = std::ranges::sentinel_t<detail::maybe_const<Const, InnerView>>;

public:
  template <bool Const>
  class iterator
  {
    using Parent = detail::maybe_const<Const, adjacent_transform_view>;
    using Base = detail::maybe_const<Const, V>;

  public:
    using iterator_category =
      decltype(detail::get_adjacent_transform_iter_cat<Const, F, Base, N>());
    using iterator_concept = typename inner_iterator<Const>::iterator_concept;
    using value_type = std::remove_cvref_t<detail::adjacent_transform_invoke_result_t<
      detail::maybe_const<Const, F>,
      std::ranges::range_reference_t<Base>,
      N>>;
    using difference_type = std::ranges::range_difference_t<Base>;

    iterator() = default;
    constexpr explicit iterator(iterator<!Const> i)
      requires Const && std::convertible_to<inner_iterator<false>, inner_iterator<Const>>
      : parent_{i.parent_}
      , inner_{std::move(i.inner_)}
    {
    }

    constexpr decltype(auto) operator*() const
      noexcept(detail::adjacent_transform_nothrow_invocable<F, decltype(inner_.base())>())
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
        [&](const auto&... iters) -> decltype(auto) {
          return std::invoke(*parent_->fun_, iters[n]...);
        },
        inner_.base());
    }

    constexpr iterator& operator++()
    {
      ++inner_;
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
    {
      return x.inner_ == y.inner_;
    }

    friend constexpr bool operator<(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
    {
      return x.inner_ < y.inner_;
    }

    friend constexpr bool operator>(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
    {
      return x.inner_ > y.inner_;
    }

    friend constexpr bool operator<=(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
    {
      return x.inner_ <= y.inner_;
    }

    friend constexpr bool operator>=(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
    {
      return x.inner_ >= y.inner_;
    }

    friend constexpr auto operator<=>(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
               and std::three_way_comparable<std::ranges::iterator_t<Base>>
    {
      return x.inner_ <=> y.inner_;
    }

    friend constexpr iterator operator+(const iterator& i, const difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      return iterator{*i.parent_, i.inner_ + n};
    }

    friend constexpr iterator operator+(const difference_type n, const iterator& i)
      requires std::ranges::random_access_range<Base>
    {
      return i + n;
    }

    friend constexpr iterator operator-(const iterator& i, const difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      return iterator{*i.parent_, i.inner_ - n};
    }

    friend constexpr difference_type operator-(const iterator& i, const iterator& j)
      requires std::sized_sentinel_for<inner_iterator<Const>, inner_iterator<Const>>
    {
      return i.inner_ - j.inner_;
    }

  private:
    friend class adjacent_transform_view;

    constexpr iterator(Parent& parent, inner_iterator<Const> inner)
      : parent_{std::addressof(parent)}
      , inner_{std::move(inner)}
    {
    }

    Parent* parent_{nullptr};
    inner_iterator<Const> inner_{};
  };

  template <bool Const>
  class sentinel
  {
  public:
    sentinel() = default;

    constexpr explicit sentinel(sentinel<!Const> i)
      requires Const && std::convertible_to<inner_sentinel<false>, inner_sentinel<Const>>
      : inner_{std::move(i.inner_)}
    {
    }

    template <bool OtherConst>
      requires std::sentinel_for<inner_sentinel<Const>, inner_iterator<OtherConst>>
    friend constexpr bool operator==(const iterator<OtherConst>& x, const sentinel& y)
    {
      return x.inner_ == y.inner_;
    }

    template <bool OtherConst>
      requires std::sized_sentinel_for<inner_sentinel<Const>, inner_iterator<OtherConst>>
    friend constexpr std::ranges::range_difference_t<
      detail::maybe_const<OtherConst, InnerView>>
    operator-(const iterator<OtherConst>& x, const sentinel& y)
    {
      return x.inner_ - y.inner_;
    }

    template <bool OtherConst>
      requires std::sized_sentinel_for<inner_sentinel<Const>, inner_iterator<OtherConst>>
    friend constexpr std::ranges::range_difference_t<
      detail::maybe_const<OtherConst, InnerView>>
    operator-(const sentinel& y, const iterator<OtherConst>& x)
    {
      return y.inner_ - x.inner_;
    }

  private:
    friend class adjacent_transform_view;

    constexpr explicit sentinel(inner_sentinel<Const> inner)
      : inner_{std::move(inner)}
    {
    }

    inner_sentinel<Const> inner_{};
  };

  adjacent_transform_view() = default;

  constexpr explicit adjacent_transform_view(V base, F fun)
    : fun_{std::move(fun)}
    , inner_{std::move(base)}
  {
  }

  constexpr auto begin() { return iterator<false>{*this, inner_.begin()}; }

  constexpr auto begin() const
    requires std::ranges::range<const InnerView>
             && detail::adjacent_transform_invocable_v<
               const F&,
               std::ranges::range_reference_t<const V>,
               N>
  {
    return iterator<true>{*this, inner_.begin()};
  }

  constexpr auto end()
  {
    if constexpr (std::ranges::common_range<InnerView>)
    {
      return iterator<false>(*this, inner_.end());
    }
    else
    {
      return sentinel<false>(inner_.end());
    }
  }

  constexpr auto end() const
    requires std::ranges::range<const InnerView>
             && detail::adjacent_transform_invocable_v<
               const F&,
               std::ranges::range_reference_t<const V>,
               N>
  {
    if constexpr (std::ranges::common_range<InnerView>)
    {
      return iterator<true>(*this, inner_.end());
    }
    else
    {
      return sentinel<true>(inner_.end());
    }
  }

  constexpr auto size()
    requires std::ranges::sized_range<InnerView>
  {
    return inner_.size();
  }

  constexpr auto size() const
    requires std::ranges::sized_range<const InnerView>
  {
    return inner_.size();
  }

private:
  movable_box<F> fun_;
  adjacent_view<V, N> inner_;
};

namespace views
{
namespace detail
{

template <std::size_t N, typename F>
  requires std::is_object_v<F>
struct adjacent_transform_view_fn
{
  explicit adjacent_transform_view_fn(F fun)
    : fun_{std::move(fun)}
  {
  }

  template <typename R>
  constexpr auto operator()(R&& r) const&
  {
    return adjacent_transform_view<std::ranges::views::all_t<R>, F, N>{
      std::forward<R>(r), fun_};
  }

  template <typename R>
  constexpr auto operator()(R&& r) &&
  {
    return adjacent_transform_view<std::ranges::views::all_t<R>, F, N>{
      std::forward<R>(r), std::move(fun_)};
  }

  template <std::ranges::input_range R>
  constexpr friend auto operator|(R&& r, const adjacent_transform_view_fn& fn)
  {
    return fn(std::forward<R>(r));
  }

  template <std::ranges::input_range R>
  constexpr friend auto operator|(R&& r, adjacent_transform_view_fn&& fn)
  {
    return std::move(fn)(std::forward<R>(r));
  }

private:
  F fun_;
};

} // namespace detail

template <std::size_t N, typename F>
constexpr auto adjacent_transform(F&& f)
{
  return detail::adjacent_transform_view_fn<N, std::remove_cvref_t<F>>(
    std::forward<F>(f));
};

} // namespace views
} // namespace ranges

namespace views
{

template <std::size_t N, typename F>
constexpr auto adjacent_transform(F&& f)
{
  return ranges::views::adjacent_transform<N>(std::forward<F>(f));
};

template <typename F>
constexpr auto pairwise_transform(F&& f)
{
  return adjacent_transform<2>(std::forward<F>(f));
};

} // namespace views
} // namespace kdl

template <typename V, typename F, std::size_t N>
constexpr bool
  std::ranges::enable_borrowed_range<kdl::ranges::adjacent_transform_view<V, F, N>> =
    std::ranges::enable_borrowed_range<V>;
