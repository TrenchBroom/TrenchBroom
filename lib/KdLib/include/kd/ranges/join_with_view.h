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
#include "detail/non_propagating_cache.h"
#include "detail/range_utils.h"

#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>
#include <variant>

namespace kdl
{
namespace ranges
{
namespace detail
{

// Per LWG 4074, this is defined in terms of concatable rather than only
// requiring the common value/reference/rvalue-reference types to exist: the
// latter is underconstrained and admits a join_with_view whose iterator does
// not model input_range.
template <typename R, typename P>
concept compatible_joinable_ranges = concatable<R, P>;

template <bool Const, typename V, typename Pattern>
consteval auto get_join_with_iter_concept()
{
  using Base = maybe_const<Const, V>;
  using InnerBase = std::ranges::range_reference_t<Base>;
  using PatternBase = maybe_const<Const, Pattern>;
  using InnerVal = std::remove_reference_t<InnerBase>;

  if constexpr (
    std::is_reference_v<InnerBase> && std::ranges::bidirectional_range<Base>
    && std::ranges::bidirectional_range<InnerVal>
    && std::ranges::bidirectional_range<PatternBase>
    && std::ranges::common_range<InnerVal> && std::ranges::common_range<PatternBase>)
  {
    return std::bidirectional_iterator_tag{};
  }
  else if constexpr (
    std::is_reference_v<InnerBase> && std::ranges::forward_range<Base>
    && std::ranges::forward_range<InnerVal>)
  {
    return std::forward_iterator_tag{};
  }
  else
  {
    return std::input_iterator_tag{};
  }
}

struct join_with_empty_inner
{
};

struct join_with_empty_outer
{
};

template <typename V>
struct join_with_inner_member
{
  non_propagating_cache<std::remove_cv_t<std::ranges::range_reference_t<V>>> inner_{};
};

template <typename V>
using join_with_inner_base = std::conditional_t<
  std::is_reference_v<std::ranges::range_reference_t<V>>,
  join_with_empty_inner,
  join_with_inner_member<V>>;

template <typename V>
struct join_with_outer_member
{
  non_propagating_cache<std::ranges::iterator_t<V>> outer_it_{};
};

template <typename V>
using join_with_outer_base = std::conditional_t<
  std::ranges::forward_range<V>,
  join_with_empty_outer,
  join_with_outer_member<V>>;

// Per [range.join.with.iterator], iterator_category is present only when
// ref_is_glvalue && forward_range<Base> && forward_range<InnerBase>; otherwise
// the iterator only exposes iterator_concept. We model the conditional
// presence with an empty/non-empty base class.
template <bool Present, bool Const, typename V, typename Pattern>
struct join_with_iter_category
{
};

template <bool Const, typename V, typename Pattern>
struct join_with_iter_category<true, Const, V, Pattern>
{
private:
  using Base = maybe_const<Const, V>;
  using InnerBase = std::ranges::range_reference_t<Base>;
  using InnerVal = std::remove_reference_t<InnerBase>;
  using PatternBase = maybe_const<Const, Pattern>;

  using OuterIter = std::ranges::iterator_t<Base>;
  using InnerIter = std::ranges::iterator_t<InnerVal>;
  using PatternIter = std::ranges::iterator_t<PatternBase>;

  using OuterC = typename std::iterator_traits<OuterIter>::iterator_category;
  using InnerC = typename std::iterator_traits<InnerIter>::iterator_category;
  using PatternC = typename std::iterator_traits<PatternIter>::iterator_category;

  using CommonRef = std::common_reference_t<
    std::iter_reference_t<InnerIter>,
    std::iter_reference_t<PatternIter>>;

  static consteval auto compute()
  {
    if constexpr (!std::is_reference_v<CommonRef>)
    {
      return std::input_iterator_tag{};
    }
    else if constexpr (
      std::derived_from<OuterC, std::bidirectional_iterator_tag>
      && std::derived_from<InnerC, std::bidirectional_iterator_tag>
      && std::derived_from<PatternC, std::bidirectional_iterator_tag>
      && std::ranges::common_range<InnerVal> && std::ranges::common_range<PatternBase>)
    {
      return std::bidirectional_iterator_tag{};
    }
    else if constexpr (
      std::derived_from<OuterC, std::forward_iterator_tag>
      && std::derived_from<InnerC, std::forward_iterator_tag>
      && std::derived_from<PatternC, std::forward_iterator_tag>)
    {
      return std::forward_iterator_tag{};
    }
    else
    {
      return std::input_iterator_tag{};
    }
  }

public:
  using iterator_category = decltype(compute());
};

} // namespace detail


template <std::ranges::input_range V, std::ranges::forward_range Pattern>
  requires std::ranges::view<V>
             && std::ranges::input_range<std::ranges::range_reference_t<V>>
             && std::ranges::view<Pattern>
             && detail::
               compatible_joinable_ranges<std::ranges::range_reference_t<V>, Pattern>
class join_with_view : public std::ranges::view_interface<join_with_view<V, Pattern>>,
                       private detail::join_with_inner_base<V>,
                       private detail::join_with_outer_base<V>
{
  using InnerRng = std::ranges::range_reference_t<V>;

public:
  template <bool Const>
  class iterator
    : public detail::join_with_iter_category<
        std::is_reference_v<std::ranges::range_reference_t<detail::maybe_const<Const, V>>>
          && std::ranges::forward_range<detail::maybe_const<Const, V>>
          && std::ranges::forward_range<std::remove_reference_t<
            std::ranges::range_reference_t<detail::maybe_const<Const, V>>>>,
        Const,
        V,
        Pattern>
  {
    using Parent = detail::maybe_const<Const, join_with_view>;
    using Base = detail::maybe_const<Const, V>;
    using InnerBase = std::ranges::range_reference_t<Base>;
    using PatternBase = detail::maybe_const<Const, Pattern>;

    using OuterIter = std::ranges::iterator_t<Base>;
    using InnerIter = std::ranges::iterator_t<std::remove_reference_t<InnerBase>>;
    using PatternIter = std::ranges::iterator_t<PatternBase>;

    // The C++23 spec stores OuterIter as a member only when forward_range<Base>.
    // When Base is non-forward, the outer iterator lives in the parent's cache
    // and we keep a default-constructible placeholder here so that OuterIter
    // need not be default-constructible (e.g. libc++'s istream_view::iterator).
    using OuterMember =
      std::conditional_t<std::ranges::forward_range<Base>, OuterIter, std::monostate>;

    static constexpr bool ref_is_glvalue = std::is_reference_v<InnerBase>;

  public:
    using iterator_concept =
      decltype(detail::get_join_with_iter_concept<Const, V, Pattern>());

    using value_type = std::common_type_t<
      std::ranges::range_value_t<InnerBase>,
      std::ranges::range_value_t<PatternBase>>;

    using difference_type = std::common_type_t<
      std::ranges::range_difference_t<Base>,
      std::ranges::range_difference_t<InnerBase>,
      std::ranges::range_difference_t<PatternBase>>;

    iterator() = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr iterator(iterator<!Const> i)
      requires Const && std::convertible_to<std::ranges::iterator_t<V>, OuterIter>
                 && std::convertible_to<std::ranges::iterator_t<InnerRng>, InnerIter>
                 && std::convertible_to<std::ranges::iterator_t<Pattern>, PatternIter>
      : parent_{i.parent_}
      , outer_it_{std::move(i.outer_it_)}
    {
      if (i.inner_it_.index() == 0)
      {
        inner_it_.template emplace<0>(std::get<0>(std::move(i.inner_it_)));
      }
      else
      {
        inner_it_.template emplace<1>(std::get<1>(std::move(i.inner_it_)));
      }
    }

    constexpr decltype(auto) operator*() const
    {
      using reference = std::common_reference_t<
        std::iter_reference_t<InnerIter>,
        std::iter_reference_t<PatternIter>>;
      return std::visit([](auto& it) -> reference { return *it; }, inner_it_);
    }

    constexpr iterator& operator++()
    {
      std::visit([](auto& it) { ++it; }, inner_it_);
      satisfy();
      return *this;
    }

    constexpr void operator++(int) { ++*this; }

    constexpr iterator operator++(int)
      requires ref_is_glvalue
               && std::forward_iterator<OuterIter> && std::forward_iterator<InnerIter>
    {
      auto tmp = iterator{*this};
      ++*this;
      return tmp;
    }

    constexpr iterator& operator--()
      requires ref_is_glvalue && std::ranges::bidirectional_range<Base>
               && std::ranges::bidirectional_range<std::remove_reference_t<InnerBase>>
               && std::ranges::bidirectional_range<PatternBase>
               && std::ranges::common_range<std::remove_reference_t<InnerBase>>
               && std::ranges::common_range<PatternBase>
    {
      if (outer_it_ == std::ranges::end(parent_->base_))
      {
        auto&& inner = *--outer_it_;
        inner_it_.template emplace<1>(std::ranges::end(inner));
      }
      while (true)
      {
        if (inner_it_.index() == 0)
        {
          auto& it = std::get<0>(inner_it_);
          if (it == std::ranges::begin(parent_->pattern_))
          {
            auto&& inner = *--outer_it_;
            inner_it_.template emplace<1>(std::ranges::end(inner));
          }
          else
          {
            break;
          }
        }
        else
        {
          auto& it = std::get<1>(inner_it_);
          auto&& inner = *outer_it_;
          if (it == std::ranges::begin(inner))
          {
            inner_it_.template emplace<0>(std::ranges::end(parent_->pattern_));
          }
          else
          {
            break;
          }
        }
      }
      std::visit([](auto& it) { --it; }, inner_it_);
      return *this;
    }

    constexpr iterator operator--(int)
      requires ref_is_glvalue && std::ranges::bidirectional_range<Base>
               && std::ranges::bidirectional_range<std::remove_reference_t<InnerBase>>
               && std::ranges::bidirectional_range<PatternBase>
               && std::ranges::common_range<std::remove_reference_t<InnerBase>>
               && std::ranges::common_range<PatternBase>
    {
      auto tmp = iterator{*this};
      --*this;
      return tmp;
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
      requires ref_is_glvalue
               && std::ranges::forward_range<Base> && std::equality_comparable<InnerIter>
    {
      return x.outer_it_ == y.outer_it_ && x.inner_it_ == y.inner_it_;
    }

    friend constexpr decltype(auto) iter_move(const iterator& x)
    {
      using rvalue_reference = std::common_reference_t<
        std::iter_rvalue_reference_t<InnerIter>,
        std::iter_rvalue_reference_t<PatternIter>>;
      return std::visit<rvalue_reference>(
        [](const auto& it) -> rvalue_reference { return std::ranges::iter_move(it); },
        x.inner_it_);
    }

    friend constexpr void iter_swap(const iterator& x, const iterator& y)
      requires std::indirectly_swappable<InnerIter, PatternIter>
    {
      std::visit(
        [](const auto& it1, const auto& it2) { std::ranges::iter_swap(it1, it2); },
        x.inner_it_,
        y.inner_it_);
    }

  private:
    friend class join_with_view;
    template <bool>
    friend class sentinel;
    friend class iterator<!Const>;

    constexpr iterator(Parent& parent, OuterIter outer)
      requires std::ranges::forward_range<Base>
      : parent_{std::addressof(parent)}
      , outer_it_{std::move(outer)}
    {
      if (outer_it_ != std::ranges::end(parent_->base_))
      {
        update_inner();
        satisfy();
      }
    }

    constexpr explicit iterator(Parent& parent)
      requires(!std::ranges::forward_range<Base>)
      : parent_{std::addressof(parent)}
    {
      if (get_outer() != std::ranges::end(parent_->base_))
      {
        update_inner();
        satisfy();
      }
    }

  public:
    // Public so sentinel's non-member comparison operator can call it on MSVC, which
    // rejects private access in that context.
    constexpr OuterIter& get_outer()
    {
      if constexpr (std::ranges::forward_range<Base>)
      {
        return outer_it_;
      }
      else
      {
        return *parent_->outer_it_;
      }
    }

    constexpr const OuterIter& get_outer() const
    {
      if constexpr (std::ranges::forward_range<Base>)
      {
        return outer_it_;
      }
      else
      {
        return *parent_->outer_it_;
      }
    }

  private:
    constexpr void update_inner()
    {
      if constexpr (ref_is_glvalue)
      {
        inner_it_.template emplace<1>(std::ranges::begin(*get_outer()));
      }
      else
      {
        inner_it_.template emplace<1>(
          std::ranges::begin(parent_->inner_.emplace_deref(get_outer())));
      }
    }

    constexpr auto& get_inner()
    {
      if constexpr (ref_is_glvalue)
      {
        return *get_outer();
      }
      else
      {
        return *parent_->inner_;
      }
    }

    constexpr void satisfy()
    {
      while (true)
      {
        if (inner_it_.index() == 0)
        {
          if (std::get<0>(inner_it_) != std::ranges::end(parent_->pattern_))
          {
            break;
          }
          update_inner();
        }
        else
        {
          auto& inner = get_inner();
          if (std::get<1>(inner_it_) != std::ranges::end(inner))
          {
            break;
          }
          ++get_outer();
          if (get_outer() == std::ranges::end(parent_->base_))
          {
            if constexpr (ref_is_glvalue)
            {
              inner_it_.template emplace<0>();
            }
            break;
          }
          inner_it_.template emplace<0>(std::ranges::begin(parent_->pattern_));
        }
      }
    }

    Parent* parent_{nullptr};
    [[no_unique_address]] OuterMember outer_it_{};
    std::variant<PatternIter, InnerIter> inner_it_{};
  };

  template <bool Const>
  class sentinel
  {
    using Parent = detail::maybe_const<Const, join_with_view>;
    using Base = detail::maybe_const<Const, V>;

  public:
    sentinel() = default;

    constexpr explicit sentinel(Parent& parent)
      : end_{std::ranges::end(parent.base_)}
    {
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr sentinel(sentinel<!Const> s)
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
    friend constexpr bool operator==(const iterator<OtherConst>& x, const sentinel& y)
    {
      return x.get_outer() == y.end_;
    }

  private:
    friend class sentinel<!Const>;

    std::ranges::sentinel_t<Base> end_{};
  };

  join_with_view()
    requires std::default_initializable<V> && std::default_initializable<Pattern>
  = default;

  constexpr explicit join_with_view(V base, Pattern pattern)
    : base_{std::move(base)}
    , pattern_{std::move(pattern)}
  {
  }

  template <std::ranges::input_range R>
    requires std::constructible_from<V, std::views::all_t<R>>
               && std::constructible_from<
                 Pattern,
                 std::ranges::single_view<std::ranges::range_value_t<InnerRng>>>
  constexpr explicit join_with_view(R&& r, std::ranges::range_value_t<InnerRng> e)
    : base_{std::views::all(std::forward<R>(r))}
    , pattern_{std::views::single(std::move(e))}
  {
  }

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return base_;
  }

  constexpr V base() && { return std::move(base_); }

  constexpr auto begin()
  {
    if constexpr (std::ranges::forward_range<V>)
    {
      constexpr bool use_const = detail::simple_view<V> && std::is_reference_v<InnerRng>
                                 && detail::simple_view<Pattern>;
      return iterator<use_const>{*this, std::ranges::begin(base_)};
    }
    else
    {
      this->outer_it_.emplace(std::ranges::begin(base_));
      return iterator<false>{*this};
    }
  }

  constexpr auto begin() const
    requires std::ranges::forward_range<const V>
             && std::ranges::forward_range<const Pattern>
             && std::is_reference_v<std::ranges::range_reference_t<const V>>
             && std::ranges::input_range<std::ranges::range_reference_t<const V>>
             && detail::compatible_joinable_ranges<
               std::ranges::range_reference_t<const V>,
               const Pattern>
  {
    return iterator<true>{*this, std::ranges::begin(base_)};
  }

  constexpr auto end()
  {
    if constexpr (
      std::ranges::forward_range<V> && std::is_reference_v<InnerRng>
      && std::ranges::forward_range<InnerRng> && std::ranges::common_range<V>
      && std::ranges::common_range<InnerRng>)
    {
      constexpr bool use_const = detail::simple_view<V> && detail::simple_view<Pattern>;
      return iterator<use_const>{*this, std::ranges::end(base_)};
    }
    else
    {
      constexpr bool use_const = detail::simple_view<V> && detail::simple_view<Pattern>;
      return sentinel<use_const>{*this};
    }
  }

  constexpr auto end() const
    requires std::ranges::forward_range<const V>
             && std::ranges::forward_range<const Pattern>
             && std::is_reference_v<std::ranges::range_reference_t<const V>>
             && std::ranges::input_range<std::ranges::range_reference_t<const V>>
             && detail::compatible_joinable_ranges<
               std::ranges::range_reference_t<const V>,
               const Pattern>
  {
    using InnerConstRng = std::ranges::range_reference_t<const V>;
    if constexpr (
      std::ranges::forward_range<InnerConstRng> && std::ranges::common_range<const V>
      && std::ranges::common_range<InnerConstRng>)
    {
      return iterator<true>{*this, std::ranges::end(base_)};
    }
    else
    {
      return sentinel<true>{*this};
    }
  }

private:
  V base_{};
  Pattern pattern_{};
};

template <typename R, typename P>
join_with_view(R&&, P&&) -> join_with_view<std::views::all_t<R>, std::views::all_t<P>>;

template <std::ranges::input_range R>
join_with_view(R&&, std::ranges::range_value_t<std::ranges::range_reference_t<R>>)
  -> join_with_view<
    std::views::all_t<R>,
    std::ranges::single_view<
      std::ranges::range_value_t<std::ranges::range_reference_t<R>>>>;

namespace views
{

template <std::ranges::viewable_range R, typename Pattern>
constexpr auto join_with(R&& r, Pattern&& pattern)
{
  return ranges::join_with_view{std::forward<R>(r), std::forward<Pattern>(pattern)};
}

namespace detail
{

template <typename Pattern>
struct join_with_view_helper
{
  Pattern pattern;
};

template <typename Pattern>
join_with_view_helper(Pattern) -> join_with_view_helper<Pattern>;

template <std::ranges::viewable_range R, typename Pattern>
constexpr auto operator|(R&& r, const join_with_view_helper<Pattern>& h)
{
  return join_with(std::forward<R>(r), h.pattern);
}

template <std::ranges::viewable_range R, typename Pattern>
constexpr auto operator|(R&& r, join_with_view_helper<Pattern>&& h)
{
  return join_with(std::forward<R>(r), std::move(h.pattern));
}

} // namespace detail

template <typename Pattern>
constexpr auto join_with(Pattern&& pattern)
{
  return detail::join_with_view_helper{std::forward<Pattern>(pattern)};
}

} // namespace views
} // namespace ranges

namespace views
{

template <std::ranges::viewable_range R, typename Pattern>
constexpr auto join_with(R&& r, Pattern&& pattern)
{
  return ranges::views::join_with(std::forward<R>(r), std::forward<Pattern>(pattern));
}

template <typename Pattern>
constexpr auto join_with(Pattern&& pattern)
{
  return ranges::views::join_with(std::forward<Pattern>(pattern));
}

} // namespace views
} // namespace kdl
