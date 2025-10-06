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
#include <tuple>

namespace kdl
{
namespace ranges
{
namespace detail
{

template <bool Const, class... Views>
concept all_forward =
  (std::ranges::forward_range<detail::maybe_const<Const, Views>> && ...);

template <bool B, typename C>
struct zip_iterator_category
{
  using iterator_category = C;
};

template <typename C>
struct zip_iterator_category<false, C>
{
};

template <typename... T, typename... U, size_t... I>
constexpr auto any_equal(
  const std::tuple<T...>& lhs,
  const std::tuple<U...>& rhs,
  const std::index_sequence<I...>)
{
  return ((std::get<I>(lhs) == std::get<I>(rhs)) || ...);
}

constexpr auto abs_min_of(const auto& x)
{
  return x;
}

constexpr auto abs_min_of(const auto& x, const auto&... rest)
{
  const auto& m = abs_min_of(rest...);
  return (std::abs(x) < std::abs(m)) ? x : m;
}

template <typename... T, typename... U, size_t... I>
constexpr auto abs_min_diff_impl(
  const std::tuple<T...>& lhs,
  const std::tuple<U...>& rhs,
  const std::index_sequence<I...>)
{
  return abs_min_of(std::get<I>(lhs) - std::get<I>(rhs)...);
}

template <typename... T, typename... U>
constexpr auto abs_min_diff(const std::tuple<T...>& lhs, const std::tuple<U...>& rhs)
  requires(sizeof...(T) == sizeof...(U))
{
  return abs_min_diff_impl(lhs, rhs, std::make_index_sequence<sizeof...(T)>());
}

template <std::size_t N, typename... I, typename... J>
constexpr auto iter_swap_impl(const std::tuple<I...>& lhs, const std::tuple<J...>& rhs)
  requires(sizeof...(I) == sizeof...(J))
{
  if (N < sizeof...(I))
  {
    std::ranges::iter_swap(std::get<N>(lhs), std::get<N>(rhs));
    iter_swap_impl<N + 1>(lhs, rhs);
  }
}

template <typename... I, typename... J>
constexpr auto iter_swap(const std::tuple<I...>& lhs, const std::tuple<J...>& rhs)
  requires(sizeof...(I) == sizeof...(J))
{
  iter_swap_impl<0>(lhs, rhs);
}

template <bool Const, typename... Views>
using zip_difference_type_t = std::common_type_t<
  std::ranges::range_difference_t<detail::maybe_const<Const, Views>>...>;
} // namespace detail

template <std::ranges::input_range... Views>
  requires(std::ranges::view<Views> && ...) && (sizeof...(Views) > 0)
class zip_view : public std::ranges::view_interface<zip_view<Views...>>
{
public:
  template <bool Const>
  class sentinel;

  template <bool Const>
  class iterator : public detail::zip_iterator_category<
                     ranges::detail::all_forward<Const, Views...>,
                     std::input_iterator_tag>
  {
    using result_type =
      std::tuple<std::ranges::range_value_t<detail::maybe_const<Const, Views>>...>;

    using iter_tuple =
      std::tuple<std::ranges::iterator_t<detail::maybe_const<Const, Views>>...>;

  public:
    using iterator_concept = decltype(detail::get_iter_cat<Views...>());
    // iterator category is conditional and must be inherited
    using value_type = result_type;
    using difference_type = detail::zip_difference_type_t<Const, Views...>;

    iterator() = default;

    constexpr explicit iterator(iterator<!Const> other)
      requires Const
               && (std::convertible_to<std::ranges::iterator_t<Views>, std::ranges::iterator_t<std::conditional_t<!Const, const Views, Views>>> && ...)
      : current_{std::move(other.current_)}
    {
    }

    // not in spec, but useful for zip_transform_view
    constexpr const iter_tuple& base() const& noexcept { return current_; }

    // not in spec, but useful for zip_transform_view
    constexpr iter_tuple base() && { return std::move(current_); }

    constexpr auto operator*() const
    {
      return detail::tuple_transform([](auto&& i) { return *i; }, current_);
    }

    constexpr auto operator[](const difference_type n) const
      requires(std::ranges::random_access_range<Views> && ...)
    {
      return detail::tuple_transform(
        [&](auto&& i) { return i[std::iter_difference_t<decltype(i)>(n)]; }, current_);
    }

    constexpr iterator& operator++()
    {
      std::apply([](auto&&... i) { (++i, ...); }, current_);
      return *this;
    }

    constexpr auto operator++(int)
    {
      if constexpr ((std::ranges::forward_range<Views> && ...))
      {
        auto tmp = *this;
        ++*this;
        return tmp;
      }
      else
      {
        std::apply([](auto&&... i) { (++i, ...); }, current_);
      }
    }

    constexpr iterator& operator--()
      requires(std::ranges::bidirectional_range<Views> && ...)
    {
      std::apply([](auto&&... i) { (--i, ...); }, current_);
      return *this;
    }

    constexpr auto operator--(int)
      requires(std::ranges::bidirectional_range<Views> && ...)
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator& operator+=(const difference_type n)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      std::apply([n](auto&&... i) { ((i += n), ...); }, current_);
      return *this;
    }

    constexpr iterator& operator-=(const difference_type n)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      std::apply([n](auto&&... i) { ((i -= n), ...); }, current_);
      return *this;
    }

    friend constexpr bool operator==(const iterator& lhs, const iterator& rhs)
      requires(
        std::equality_comparable<
          std::ranges::iterator_t<detail::maybe_const<Const, Views>>>
        && ...)
    {
      return lhs.current_ == rhs.current_;
    }

    friend constexpr bool operator<(const iterator& lhs, const iterator& rhs)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      return lhs.current_ < rhs.current_;
    }

    friend constexpr bool operator>(const iterator& lhs, const iterator& rhs)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      return lhs.current_ > rhs.current_;
    }

    friend constexpr bool operator<=(const iterator& lhs, const iterator& rhs)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      return lhs.current_ <= rhs.current_;
    }

    friend constexpr bool operator>=(const iterator& lhs, const iterator& rhs)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      return lhs.current_ >= rhs.current_;
    }

    friend constexpr auto operator<=>(const iterator& lhs, const iterator& rhs)
      requires(
        (std::ranges::random_access_range<Views>
         && std::three_way_comparable<std::ranges::iterator_t<Views>>)
        && ...)
    {
      return lhs.current_ <=> rhs.current_;
    }

    friend constexpr iterator operator+(const iterator& i, const difference_type n)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      return std::apply(
        [n](auto&&... j) { return iterator{iter_tuple{j + n...}}; }, i.current_);
    }

    friend constexpr iterator operator+(const difference_type n, const iterator& i)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      return i + n;
    }

    friend constexpr auto operator-(const iterator& i, const difference_type n)
      requires(std::ranges::random_access_range<Views> && ...)
    {
      return std::apply(
        [n](auto&&... j) { return iterator{iter_tuple{j - n...}}; }, i.current_);
    }

    friend constexpr difference_type operator-(const iterator& lhs, const iterator& rhs)
      requires(
        std::sized_sentinel_for<
          std::ranges::iterator_t<detail::maybe_const<Const, Views>>,
          std::ranges::iterator_t<detail::maybe_const<Const, Views>>>
        && ...)
    {
      return detail::abs_min_diff(lhs.current_, rhs.current_);
    }

    friend constexpr auto iter_move(const iterator& i) noexcept(
      (noexcept(std::ranges::iter_move(
         std::declval<
           const std::ranges::iterator_t<detail::maybe_const<Const, Views>>&>()))
       && ...)
      && (std::is_nothrow_move_constructible_v<std::ranges::range_rvalue_reference_t<detail::maybe_const<Const, Views>>> && ...))
    {
      return detail::tuple_transform(std::ranges::iter_move, i.current_);
    }

    friend constexpr void iter_swap(const iterator& lhs, const iterator& rhs) noexcept((
      noexcept(std::ranges::iter_swap(
        std::declval<const std::ranges::iterator_t<detail::maybe_const<Const, Views>>&>(),
        std::declval<
          const std::ranges::iterator_t<detail::maybe_const<Const, Views>>&>()))
      && ...))
      requires(
        std::indirectly_swappable<
          std::ranges::iterator_t<detail::maybe_const<Const, Views>>>
        && ...)
    {
      detail::iter_swap(lhs, rhs);
    }

  private:
    friend class zip_view;

    constexpr explicit iterator(iter_tuple current)
      : current_{std::move(current)}
    {
    }

    iter_tuple current_{};
  };

  template <bool Const>
  class sentinel
  {
    using sentinel_tuple =
      std::tuple<std::ranges::sentinel_t<detail::maybe_const<Const, Views>>...>;

  public:
    sentinel() = default;

    constexpr explicit sentinel(sentinel<!Const> s)
      requires Const
               && (std::convertible_to<std::ranges::sentinel_t<Views>, std::ranges::sentinel_t<detail::maybe_const<!Const, Views>>> && ...)
      : end_{std::move(s.end_)}
    {
    }

    template <bool OtherConst>
      requires(
        std::sentinel_for<
          std::ranges::sentinel_t<detail::maybe_const<Const, Views>>,
          std::ranges::iterator_t<detail::maybe_const<OtherConst, Views>>>
        && ...)
    friend constexpr bool operator==(const iterator<OtherConst>& i, const sentinel& s)
    {
      return detail::any_equal(
        i.base(), s.end_, std::make_index_sequence<sizeof...(Views)>());
    }

    template <bool OtherConst>
      requires(
        std::sized_sentinel_for<
          std::ranges::sentinel_t<detail::maybe_const<Const, Views>>,
          std::ranges::iterator_t<detail::maybe_const<OtherConst, Views>>>
        && ...)
    friend constexpr std::common_type_t<
      std::ranges::range_difference_t<detail::maybe_const<OtherConst, Views>>...>
    operator-(const iterator<OtherConst>& lhs, const sentinel& rhs)
    {
      return detail::abs_min_diff(lhs.base(), rhs.end_);
    }

    template <bool OtherConst>
      requires(
        std::sized_sentinel_for<
          std::ranges::sentinel_t<detail::maybe_const<Const, Views>>,
          std::ranges::iterator_t<detail::maybe_const<OtherConst, Views>>>
        && ...)
    friend constexpr std::common_type_t<
      std::ranges::range_difference_t<detail::maybe_const<OtherConst, Views>>...>
    operator-(const sentinel& lhs, const iterator<OtherConst>& rhs)
    {
      return detail::abs_min_diff(lhs.end_, rhs.base());
    }

  private:
    friend class zip_view;

    constexpr explicit sentinel(sentinel_tuple end)
      : end_{std::move(end)}
    {
    }

    sentinel_tuple end_;
  };

  constexpr zip_view() = default;

  constexpr explicit zip_view(Views... base_)
    : mBase{std::move(base_)...}
  {
  }

  constexpr auto begin()
    requires(!(ranges::detail::simple_view<Views> && ...))
  {
    return iterator<false>{detail::tuple_transform(std::ranges::begin, mBase)};
  }

  constexpr auto begin() const
    requires(ranges::detail::simple_view<Views> && ...)
  {
    return iterator<true>{detail::tuple_transform(std::ranges::begin, mBase)};
  }

  constexpr auto end()
  {
    return sentinel<false>{detail::tuple_transform(std::ranges::end, mBase)};
  }

  constexpr auto end() const
    requires(std::ranges::range<const Views> && ...)
  {
    return sentinel<true>{detail::tuple_transform(std::ranges::end, mBase)};
  }

  constexpr auto size() const
    requires(std::ranges::sized_range<Views> && ...)
  {
    return std::apply(
      [](auto&&... b) { return std::min({std::ranges::size(b)...}); }, mBase);
  }

private:
  std::tuple<Views...> mBase;
};

template <class... Rs>
zip_view(Rs&&...) -> zip_view<std::views::all_t<Rs>...>;

namespace views
{

template <typename... R>
  requires(std::ranges::input_range<R> && ...)
auto zip(R&&... r)
{
  return ranges::zip_view{std::forward<R>(r)...};
};

} // namespace views
} // namespace ranges

namespace views
{

template <typename... R>
  requires(std::ranges::input_range<R> && ...)
auto zip(R&&... r)
{
  return ranges::views::zip(std::forward<R>(r)...);
};

} // namespace views
} // namespace kdl

template <typename... Views>
constexpr bool std::ranges::enable_borrowed_range<kdl::ranges::zip_view<Views...>> =
  (std::ranges::enable_borrowed_range<Views> && ...);
