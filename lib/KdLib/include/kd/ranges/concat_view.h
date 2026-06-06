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

#include "detail/concatable.h"
#include "detail/integer_like.h"
#include "detail/range_utils.h"

#include <concepts>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace kdl
{
namespace ranges
{
namespace detail
{

template <typename Head, typename... Tail>
struct extract_last : extract_last<Tail...>
{
};

template <typename T>
struct extract_last<T>
{
  using type = T;
};

template <typename... Ts>
using extract_last_t = typename extract_last<Ts...>::type;

template <bool Const, typename... Views>
concept concat_all_random_access =
  (std::ranges::random_access_range<maybe_const<Const, Views>> && ...);

template <bool Const, typename... Views>
concept concat_all_bidirectional =
  (std::ranges::bidirectional_range<maybe_const<Const, Views>> && ...);

template <bool Const, typename... Views>
concept concat_all_forward =
  (std::ranges::forward_range<maybe_const<Const, Views>> && ...);

template <bool Const, typename First, typename... Tail>
struct concat_all_common_except_last
{
  static constexpr bool value = std::ranges::common_range<maybe_const<Const, First>>
                                && concat_all_common_except_last<Const, Tail...>::value;
};

template <bool Const, typename Last>
struct concat_all_common_except_last<Const, Last>
{
  static constexpr bool value = true;
};

template <bool Const, typename... Views>
concept concat_is_random_access =
  concat_all_random_access<Const, Views...>
  && concat_all_common_except_last<Const, Views...>::value;

template <bool Const, typename... Views>
concept concat_is_bidirectional =
  concat_all_bidirectional<Const, Views...>
  && concat_all_common_except_last<Const, Views...>::value;

template <bool Const, typename First, typename... Tail>
struct concat_all_but_first_sized
{
  static constexpr bool value =
    (std::ranges::sized_range<maybe_const<Const, Tail>> && ...);
};

// Calls f.template operator()<I>() where I matches idx. Asserts that idx < N.
template <std::size_t I, std::size_t N, typename F>
constexpr decltype(auto) invoke_at_index_impl(std::size_t idx, F&& f)
{
  if constexpr (I + 1 < N)
  {
    if (idx == I)
    {
      return std::forward<F>(f).template operator()<I>();
    }
    return invoke_at_index_impl<I + 1, N>(idx, std::forward<F>(f));
  }
  else
  {
    return std::forward<F>(f).template operator()<I>();
  }
}

template <std::size_t N, typename F>
constexpr decltype(auto) invoke_at_index(std::size_t idx, F&& f)
{
  static_assert(N > 0);
  return invoke_at_index_impl<0, N>(idx, std::forward<F>(f));
}

// iterator_category is present only when all underlying ranges model
// forward_range; otherwise this base contributes no such member type (see the
// specialization below).
template <bool Const, typename... Views>
struct concat_iterator_category
{
};

template <bool Const, typename... Views>
  requires concat_all_forward<Const, Views...>
struct concat_iterator_category<Const, Views...>
{
private:
  template <typename Tag>
  static constexpr bool derive_tag =
    (std::derived_from<
       typename std::iterator_traits<
         std::ranges::iterator_t<maybe_const<Const, Views>>>::iterator_category,
       Tag>
     && ...);
  static constexpr bool derive_random_access =
    derive_tag<std::random_access_iterator_tag>
    && concat_is_random_access<Const, Views...>;
  static constexpr bool derive_bidirectional =
    derive_tag<std::bidirectional_iterator_tag>
    && concat_is_bidirectional<Const, Views...>;
  static constexpr bool derive_forward = derive_tag<std::forward_iterator_tag>;

public:
  using iterator_category = std::conditional_t<
    !std::is_reference_v<concat_reference_t<maybe_const<Const, Views>...>>,
    std::input_iterator_tag,
    std::conditional_t<
      derive_random_access,
      std::random_access_iterator_tag,
      std::conditional_t<
        derive_bidirectional,
        std::bidirectional_iterator_tag,
        std::conditional_t<
          derive_forward,
          std::forward_iterator_tag,
          std::input_iterator_tag>>>>;
};

} // namespace detail


template <std::ranges::input_range... Views>
  requires(std::ranges::view<Views> && ...) && (sizeof...(Views) > 0)
          && detail::concatable<Views...>
class concat_view : public std::ranges::view_interface<concat_view<Views...>>
{
public:
  template <bool Const>
  class iterator : public detail::concat_iterator_category<Const, Views...>
  {
  public:
    using iterator_concept = std::conditional_t<
      detail::concat_is_random_access<Const, Views...>,
      std::random_access_iterator_tag,
      std::conditional_t<
        detail::concat_is_bidirectional<Const, Views...>,
        std::bidirectional_iterator_tag,
        std::conditional_t<
          detail::concat_all_forward<Const, Views...>,
          std::forward_iterator_tag,
          std::input_iterator_tag>>>;

    using value_type = detail::concat_value_t<detail::maybe_const<Const, Views>...>;

    using difference_type = std::common_type_t<
      std::ranges::range_difference_t<detail::maybe_const<Const, Views>>...>;

    iterator() = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr iterator(iterator<!Const> i)
      requires Const
                 && (std::convertible_to<std::ranges::iterator_t<Views>, std::ranges::iterator_t<const Views>> && ...)
      : parent_{i.parent_}
      , it_{detail::invoke_at_index<sizeof...(Views)>(
          i.it_.index(), [&i]<std::size_t Idx>() -> base_iter {
            return base_iter{std::in_place_index<Idx>, std::get<Idx>(std::move(i.it_))};
          })}
    {
    }

    constexpr decltype(auto) operator*() const
    {
      using reference = detail::concat_reference_t<detail::maybe_const<Const, Views>...>;
      return std::visit([](auto&& it) -> reference { return *it; }, it_);
    }

    constexpr iterator& operator++()
    {
      detail::invoke_at_index<sizeof...(Views)>(it_.index(), [this]<std::size_t I>() {
        ++std::get<I>(it_);
        this->template satisfy<I>();
      });
      return *this;
    }

    constexpr void operator++(int) { ++*this; }

    constexpr iterator operator++(int)
      requires detail::concat_all_forward<Const, Views...>
    {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator& operator--()
      requires detail::concat_is_bidirectional<Const, Views...>
    {
      detail::invoke_at_index<sizeof...(Views)>(
        it_.index(), [this]<std::size_t I>() { this->template prev<I>(); });
      return *this;
    }

    constexpr iterator operator--(int)
      requires detail::concat_is_bidirectional<Const, Views...>
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator& operator+=(difference_type n)
      requires detail::concat_is_random_access<Const, Views...>
    {
      if (n > 0)
      {
        detail::invoke_at_index<sizeof...(Views)>(
          it_.index(), [this, n]<std::size_t I>() {
            auto& v = std::get<I>(parent_->base_);
            auto idx =
              static_cast<difference_type>(std::get<I>(it_) - std::ranges::begin(v));
            this->template advance_fwd<I>(idx, n);
          });
      }
      else if (n < 0)
      {
        detail::invoke_at_index<sizeof...(Views)>(
          it_.index(), [this, n]<std::size_t I>() {
            auto& v = std::get<I>(parent_->base_);
            auto idx =
              static_cast<difference_type>(std::get<I>(it_) - std::ranges::begin(v));
            this->template advance_bwd<I>(idx, -n);
          });
      }
      return *this;
    }

    constexpr iterator& operator-=(difference_type n)
      requires detail::concat_is_random_access<Const, Views...>
    {
      *this += -n;
      return *this;
    }

    constexpr decltype(auto) operator[](difference_type n) const
      requires detail::concat_is_random_access<Const, Views...>
    {
      return *((*this) + n);
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
      requires(
        std::equality_comparable<
          std::ranges::iterator_t<detail::maybe_const<Const, Views>>>
        && ...)
    {
      return x.it_ == y.it_;
    }

    friend constexpr bool operator==(const iterator& x, std::default_sentinel_t)
    {
      constexpr auto last = sizeof...(Views) - 1;
      return x.it_.index() == last
             && std::get<last>(x.it_)
                  == std::ranges::end(x.template get_base_view_<last>());
    }

    friend constexpr bool operator<(const iterator& x, const iterator& y)
      requires detail::concat_all_random_access<Const, Views...>
    {
      return x.it_ < y.it_;
    }

    friend constexpr bool operator>(const iterator& x, const iterator& y)
      requires detail::concat_all_random_access<Const, Views...>
    {
      return x.it_ > y.it_;
    }

    friend constexpr bool operator<=(const iterator& x, const iterator& y)
      requires detail::concat_all_random_access<Const, Views...>
    {
      return x.it_ <= y.it_;
    }

    friend constexpr bool operator>=(const iterator& x, const iterator& y)
      requires detail::concat_all_random_access<Const, Views...>
    {
      return x.it_ >= y.it_;
    }

    friend constexpr auto operator<=>(const iterator& x, const iterator& y)
      requires detail::concat_all_random_access<Const, Views...>
               && (std::three_way_comparable<std::ranges::iterator_t<detail::maybe_const<Const, Views>>> && ...)
    {
      return x.it_ <=> y.it_;
    }

    friend constexpr iterator operator+(const iterator& it, difference_type n)
      requires detail::concat_is_random_access<Const, Views...>
    {
      auto tmp = it;
      tmp += n;
      return tmp;
    }

    friend constexpr iterator operator+(difference_type n, const iterator& it)
      requires detail::concat_is_random_access<Const, Views...>
    {
      return it + n;
    }

    friend constexpr iterator operator-(const iterator& it, difference_type n)
      requires detail::concat_is_random_access<Const, Views...>
    {
      auto tmp = it;
      tmp -= n;
      return tmp;
    }

    friend constexpr difference_type operator-(const iterator& x, const iterator& y)
      requires detail::concat_is_random_access<Const, Views...>
    {
      return detail::invoke_at_index<sizeof...(Views)>(
        x.it_.index(), [&]<std::size_t IdxX>() -> difference_type {
          return detail::invoke_at_index<sizeof...(Views)>(
            y.it_.index(), [&]<std::size_t IdxY>() -> difference_type {
              if constexpr (IdxX > IdxY)
              {
                auto dx = static_cast<difference_type>(
                  std::get<IdxX>(x.it_)
                  - std::ranges::begin(x.template get_base_view_<IdxX>()));
                auto dy = static_cast<difference_type>(
                  std::ranges::end(y.template get_base_view_<IdxY>())
                  - std::get<IdxY>(y.it_));
                auto s = x.template sum_sizes_in_range<IdxY + 1, IdxX>();
                return dy + s + dx;
              }
              else if constexpr (IdxX < IdxY)
              {
                return -(y - x);
              }
              else
              {
                return static_cast<difference_type>(
                  std::get<IdxX>(x.it_) - std::get<IdxY>(y.it_));
              }
            });
        });
    }

    friend constexpr difference_type operator-(const iterator& x, std::default_sentinel_t)
      requires(std::sized_sentinel_for<
                 std::ranges::sentinel_t<detail::maybe_const<Const, Views>>,
                 std::ranges::iterator_t<detail::maybe_const<Const, Views>>>
               && ...)
              && detail::concat_all_but_first_sized<Const, Views...>::value
    {
      return detail::invoke_at_index<sizeof...(Views)>(
        x.it_.index(), [&]<std::size_t IdxX>() -> difference_type {
          auto dx = static_cast<difference_type>(
            std::ranges::end(x.template get_base_view_<IdxX>()) - std::get<IdxX>(x.it_));
          auto s = x.template sum_sizes_in_range<IdxX + 1, sizeof...(Views)>();
          return -(dx + s);
        });
    }

    friend constexpr difference_type operator-(std::default_sentinel_t, const iterator& x)
      requires(std::sized_sentinel_for<
                 std::ranges::sentinel_t<detail::maybe_const<Const, Views>>,
                 std::ranges::iterator_t<detail::maybe_const<Const, Views>>>
               && ...)
              && detail::concat_all_but_first_sized<Const, Views...>::value
    {
      return -(x - std::default_sentinel);
    }

    friend constexpr decltype(auto) iter_move(const iterator& it) noexcept(
      ((std::is_nothrow_invocable_v<
          decltype(std::ranges::iter_move),
          const std::ranges::iterator_t<detail::maybe_const<Const, Views>>&>
        && std::is_nothrow_convertible_v<
          std::ranges::range_rvalue_reference_t<detail::maybe_const<Const, Views>>,
          detail::concat_rvalue_reference_t<detail::maybe_const<Const, Views>...>>)
       && ...))
    {
      using rvalue_reference =
        detail::concat_rvalue_reference_t<detail::maybe_const<Const, Views>...>;
      return std::visit(
        [](const auto& i) -> rvalue_reference { return std::ranges::iter_move(i); },
        it.it_);
    }

    // The spec writes the first noexcept term as noexcept(ranges::swap(*x, *y)), but
    // naming the parameters here trips up MSVC, so we use declval of the reference type
    // instead. This is harmless: it only drops operator*'s (never-throwing) potential
    // exception from the computation, so iter_swap is at most reported more noexcept.
    friend constexpr void iter_swap(const iterator& x, const iterator& y) noexcept(
      noexcept(std::ranges::swap(
        std::declval<detail::concat_reference_t<detail::maybe_const<Const, Views>...>>(),
        std::declval<detail::concat_reference_t<detail::maybe_const<Const, Views>...>>()))
      && (noexcept(std::ranges::iter_swap(std::declval<const std::ranges::iterator_t<detail::maybe_const<Const, Views>>&>(), std::declval<const std::ranges::iterator_t<detail::maybe_const<Const, Views>>&>())) && ...))
      requires std::swappable_with<
                 detail::concat_reference_t<detail::maybe_const<Const, Views>...>,
                 detail::concat_reference_t<detail::maybe_const<Const, Views>...>>
               && (std::indirectly_swappable<std::ranges::iterator_t<detail::maybe_const<Const, Views>>> && ...)
    {
      std::visit(
        [&](const auto& it1, const auto& it2) {
          if constexpr (std::is_same_v<decltype(it1), decltype(it2)>)
          {
            std::ranges::iter_swap(it1, it2);
          }
          else
          {
            std::ranges::swap(*x, *y);
          }
        },
        x.it_,
        y.it_);
    }

  private:
    friend class concat_view;
    friend class iterator<!Const>;

    // The non-member friend functions defined inside iterator's class body are not
    // members of concat_view, so they cannot access concat_view's private base_ directly.
    // Routing the access through this member function works because member functions of
    // iterator (a nested class of concat_view) may access concat_view's private members,
    // and friend functions of iterator may call iterator's private member functions.
    template <std::size_t I>
    constexpr auto& get_base_view_() const noexcept
    {
      return std::get<I>(parent_->base_);
    }

    using base_iter =
      std::variant<std::ranges::iterator_t<detail::maybe_const<Const, Views>>...>;

    using Parent = detail::maybe_const<Const, concat_view>;

    template <typename... Args>
    constexpr explicit iterator(Parent* parent, Args&&... args)
      requires std::constructible_from<base_iter, Args&&...>
      : parent_{parent}
      , it_{std::forward<Args>(args)...}
    {
    }

    template <std::size_t I>
    constexpr void satisfy()
    {
      if constexpr (I + 1 < sizeof...(Views))
      {
        if (std::get<I>(it_) == std::ranges::end(std::get<I>(parent_->base_)))
        {
          it_.template emplace<I + 1>(
            std::ranges::begin(std::get<I + 1>(parent_->base_)));
          satisfy<I + 1>();
        }
      }
    }

    template <std::size_t I>
    constexpr void prev()
    {
      if constexpr (I == 0)
      {
        --std::get<0>(it_);
      }
      else
      {
        if (std::get<I>(it_) == std::ranges::begin(std::get<I>(parent_->base_)))
        {
          it_.template emplace<I - 1>(std::ranges::end(std::get<I - 1>(parent_->base_)));
          prev<I - 1>();
        }
        else
        {
          --std::get<I>(it_);
        }
      }
    }

    template <std::size_t I>
    constexpr void advance_fwd(difference_type offset, difference_type steps)
    {
      using under_diff = std::iter_difference_t<std::variant_alternative_t<I, base_iter>>;
      if constexpr (I + 1 == sizeof...(Views))
      {
        std::get<I>(it_) += static_cast<under_diff>(steps);
      }
      else
      {
        auto n = static_cast<difference_type>(
          std::ranges::distance(std::get<I>(parent_->base_)));
        if (offset + steps < n)
        {
          std::get<I>(it_) += static_cast<under_diff>(steps);
        }
        else
        {
          it_.template emplace<I + 1>(
            std::ranges::begin(std::get<I + 1>(parent_->base_)));
          advance_fwd<I + 1>(0, offset + steps - n);
        }
      }
    }

    template <std::size_t I>
    constexpr void advance_bwd(difference_type offset, difference_type steps)
    {
      using under_diff = std::iter_difference_t<std::variant_alternative_t<I, base_iter>>;
      if constexpr (I == 0)
      {
        std::get<I>(it_) -= static_cast<under_diff>(steps);
      }
      else
      {
        if (offset >= steps)
        {
          std::get<I>(it_) -= static_cast<under_diff>(steps);
        }
        else
        {
          auto prev_size = static_cast<difference_type>(
            std::ranges::distance(std::get<I - 1>(parent_->base_)));
          it_.template emplace<I - 1>(std::ranges::end(std::get<I - 1>(parent_->base_)));
          advance_bwd<I - 1>(prev_size, steps - offset);
        }
      }
    }

    template <std::size_t Start, std::size_t End>
    constexpr difference_type sum_sizes_in_range() const
    {
      if constexpr (Start < End)
      {
        return static_cast<difference_type>(
                 std::ranges::size(std::get<Start>(parent_->base_)))
               + sum_sizes_in_range<Start + 1, End>();
      }
      else
      {
        return 0;
      }
    }

    Parent* parent_{nullptr};
    base_iter it_;
  };

  constexpr concat_view() = default;

  constexpr explicit concat_view(Views... views)
    : base_{std::move(views)...}
  {
  }

  constexpr iterator<false> begin()
    requires(!(detail::simple_view<Views> && ...))
  {
    auto it = iterator<false>{
      this, std::in_place_index<0>, std::ranges::begin(std::get<0>(base_))};
    it.template satisfy<0>();
    return it;
  }

  constexpr iterator<true> begin() const
    requires(std::ranges::range<const Views> && ...) && detail::concatable<const Views...>
  {
    auto it = iterator<true>{
      this, std::in_place_index<0>, std::ranges::begin(std::get<0>(base_))};
    it.template satisfy<0>();
    return it;
  }

  constexpr auto end()
    requires(!(detail::simple_view<Views> && ...))
  {
    using LastView = detail::extract_last_t<Views...>;
    if constexpr (
      detail::concat_all_forward<false, Views...> && std::ranges::common_range<LastView>)
    {
      constexpr auto n = sizeof...(Views);
      return iterator<false>{
        this, std::in_place_index<n - 1>, std::ranges::end(std::get<n - 1>(base_))};
    }
    else
    {
      return std::default_sentinel;
    }
  }

  constexpr auto end() const
    requires(std::ranges::range<const Views> && ...) && detail::concatable<const Views...>
  {
    using LastView = detail::extract_last_t<const Views...>;
    if constexpr (
      detail::concat_all_forward<true, Views...> && std::ranges::common_range<LastView>)
    {
      constexpr auto n = sizeof...(Views);
      return iterator<true>{
        this, std::in_place_index<n - 1>, std::ranges::end(std::get<n - 1>(base_))};
    }
    else
    {
      return std::default_sentinel;
    }
  }

  constexpr auto size()
    requires(std::ranges::sized_range<Views> && ...)
  {
    return std::apply(
      [](auto... sizes) {
        using CT = std::common_type_t<decltype(sizes)...>;
        return (make_unsigned_like_t<CT>(sizes) + ...);
      },
      detail::tuple_transform(std::ranges::size, base_));
  }

  constexpr auto size() const
    requires(std::ranges::sized_range<const Views> && ...)
  {
    return std::apply(
      [](auto... sizes) {
        using CT = std::common_type_t<decltype(sizes)...>;
        return (make_unsigned_like_t<CT>(sizes) + ...);
      },
      detail::tuple_transform(std::ranges::size, base_));
  }

private:
  std::tuple<Views...> base_;
};

template <typename... R>
concat_view(R&&...) -> concat_view<std::views::all_t<R>...>;


namespace views
{

struct concat_fn
{
  template <std::ranges::input_range R>
  constexpr auto operator()(R&& r) const
  {
    return std::views::all(std::forward<R>(r));
  }

  template <typename First, typename... Tail>
    requires(sizeof...(Tail) > 0)
  constexpr auto operator()(First&& first, Tail&&... tail) const
  {
    return ranges::concat_view{std::forward<First>(first), std::forward<Tail>(tail)...};
  }
};

inline constexpr concat_fn concat{};

} // namespace views
} // namespace ranges

namespace views
{

template <typename... R>
constexpr auto concat(R&&... r)
{
  return ranges::views::concat(std::forward<R>(r)...);
}

} // namespace views
} // namespace kdl
