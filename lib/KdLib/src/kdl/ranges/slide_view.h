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

#include <ranges>

namespace kdl
{
namespace ranges
{
namespace detail
{

template <bool B, typename V>
class maybe_slide_caches_first
{
protected:
  non_propagating_cache<std::ranges::iterator_t<V>> cached_begin_{};
};

template <typename V>
struct maybe_slide_caches_first<false, V>
{
};

template <bool B, typename V>
class maybe_slide_caches_last
{
protected:
  non_propagating_cache<std::ranges::iterator_t<V>> cached_end_{};
};

template <typename V>
struct maybe_slide_caches_last<false, V>
{
};

template <bool B, typename V>
class maybe_iterator_caches_last
{
protected:
  std::ranges::iterator_t<V> last_ele_{};
};

template <typename V>
struct maybe_iterator_caches_last<false, V>
{
};

template <typename V>
concept slide_caches_nothing =
  std::ranges::random_access_range<V> && std::ranges::sized_range<V>;

template <typename V>
concept slide_caches_last =
  !slide_caches_nothing<V> && std::ranges::bidirectional_range<V>
  && std::ranges::common_range<V>;

template <typename V>
concept slide_caches_first = !slide_caches_nothing<V> && !slide_caches_last<V>;

} // namespace detail

template <std::ranges::forward_range V>
  requires std::ranges::view<V>
class slide_view
  : public std::ranges::view_interface<slide_view<V>>,
    protected detail::maybe_slide_caches_first<detail::slide_caches_first<V>, V>,
    protected detail::maybe_slide_caches_last<detail::slide_caches_last<V>, V>
{
public:
  template <bool Const>
  class iterator : protected detail::maybe_iterator_caches_last<
                     detail::slide_caches_first<detail::maybe_const<Const, V>>,
                     detail::maybe_const<Const, V>>
  {
    using Base = detail::maybe_const<Const, V>;
    static constexpr auto caches_last = detail::slide_caches_first<Base>;

  public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = decltype(detail::get_iter_cat<V>());
    using value_type = decltype(std::views::counted(
      std::declval<std::ranges::iterator_t<Base>>(),
      std::declval<std::ranges::range_difference_t<Base>>()));
    using difference_type = std::ranges::range_difference_t<Base>;

    iterator() = default;

    constexpr explicit iterator(iterator<!Const> i)
      requires Const
                 && std::convertible_to<
                   std::ranges::iterator_t<V>,
                   std::ranges::iterator_t<Base>>
      : current_{std::move(i.current_)}
      , n_{i.n_}
    {
    }

    constexpr auto operator*() const { return std::views::counted(current_, n_); }

    constexpr auto operator[](const difference_type pos) const
      requires std::ranges::random_access_range<Base>
    {
      return std::views::counted(current_ + pos, n_);
    }

    constexpr iterator& operator++()
    {
      current_ = std::ranges::next(current_);
      if constexpr (caches_last)
      {
        this->last_ele_ = std::ranges::next(this->last_ele_);
      }
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
      current_ = std::ranges::prev(current_);
      if constexpr (caches_last)
      {
        this->last_ele_ = std::ranges::prev(this->last_ele_);
      }
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
      current_ = current_ + n;
      if constexpr (caches_last)
      {
        this->last_ele_ = this->last_ele_ + n;
      }
      return *this;
    }

    constexpr iterator& operator-=(const difference_type n)
      requires std::ranges::random_access_range<Base>
    {
      current_ = current_ - n;
      if constexpr (caches_last)
      {
        this->last_ele_ = this->last_ele_ - n;
      }
      return *this;
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
    {
      if constexpr (caches_last)
      {
        return x.last_ele == y.last_ele;
      }
      else
      {
        return x.current_ == y.current_;
      }
    }

    friend constexpr bool operator<(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
    {
      return x.current_ < y.current_;
    }

    friend constexpr bool operator>(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
    {
      return y.current_ < x.current_;
    }

    friend constexpr bool operator<=(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
    {
      return !(y.current_ < x.current_);
    }

    friend constexpr bool operator>=(const iterator& x, const iterator& y)
      requires std::ranges::random_access_range<Base>
    {
      return !(x.current_ < y.current_);
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
      if constexpr (caches_last)
      {
        return x.last_ele_ - y.last_ele_;
      }
      else
      {
        return x.current_ - y.current_;
      }
    }

  private:
    friend class slide_view;

    constexpr iterator(
      std::ranges::iterator_t<Base> current, std::ranges::range_difference_t<Base> n)
      requires(!caches_last)
      : current_{std::move(current)}
      , n_{n}
    {
    }

    constexpr iterator(
      std::ranges::iterator_t<Base> current,
      std::ranges::iterator_t<Base> last_ele,
      std::ranges::range_difference_t<Base> n)
      requires caches_last
      : detail::maybe_iterator_caches_last<true, Base>{std::move(last_ele)}
      , current_{std::move(current)}
      , n_{n}
    {
    }

    std::ranges::iterator_t<Base> current_{};
    std::ranges::range_difference_t<Base> n_{};
  };

  // Only used when detail::slide_caches_first<V> holds.
  class sentinel
  {
  public:
    sentinel() = default;

    template <bool Const>
    friend constexpr bool operator==(const iterator<Const>& x, const sentinel& y)
    {
      return x.last_ele_ == y.end_;
    }

    friend constexpr std::ranges::range_difference_t<V> operator-(
      const iterator<false>& x, const sentinel& y)
      requires std::
        sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
    {
      return x.last_ele_ - y.end_;
    }

    friend constexpr std::ranges::range_difference_t<V> operator-(
      const sentinel& y, const iterator<false>& x)
      requires std::
        sized_sentinel_for<std::ranges::sentinel_t<V>, std::ranges::iterator_t<V>>
    {
      return y.end_ - x.last_ele_;
    }

  private:
    constexpr explicit sentinel(std::ranges::sentinel_t<V> end)
      : end_{std::move(end)}
    {
    }

    std::ranges::sentinel_t<V> end_{};
  };

  constexpr explicit slide_view(V base, std::ranges::range_difference_t<V> n)
    : base_{std::move(base)}
    , n_{n}
  {
  }

  constexpr auto begin()
    requires(!(detail::simple_view<V> && detail::slide_caches_nothing<const V>))
  {
    const auto b = std::ranges::begin(base_);

    if constexpr (detail::slide_caches_first<V>)
    {
      const auto e = std::ranges::end(base_);
      return iterator<false>{b, std::ranges::next(b, n_ - 1, e), n_};
    }
    else
    {
      return iterator<false>{b, n_};
    }
  }

  constexpr auto begin() const
    requires detail::slide_caches_nothing<const V>
  {
    return iterator<true>{std::ranges::begin(base_), n_};
  }

  constexpr auto end()
    requires(!(detail::simple_view<V> && detail::slide_caches_nothing<const V>))
  {
    if constexpr (detail::slide_caches_nothing<V>)
    {
      return iterator<false>{
        std::ranges::begin(base_) + std::ranges::range_difference_t<V>(size()), n_};
    }
    else if constexpr (detail::slide_caches_last<V>)
    {
      return iterator<false>{
        std::ranges::prev(std::ranges::end(base_), n_ - 1, std::ranges::begin(base_)),
        n_};
    }
    else if constexpr (std::ranges::common_range<V>)
    {
      return iterator<false>{std::ranges::end(base_), std::ranges::end(base_), n_};
    }
    else
    {
      return sentinel{std::ranges::end(base_)};
    }
  }

  constexpr auto end() const
    requires detail::slide_caches_nothing<const V>
  {
    return begin() + std::ranges::range_difference_t<const V>(size());
  }

  constexpr auto size()
    requires std::ranges::sized_range<V>
  {
    return const_cast<const slide_view*>(this)->size();
  }

  constexpr auto size() const
    requires std::ranges::sized_range<const V>
  {
    auto s = std::ranges::distance(base_) - n_ + 1;
    if (s < 0)
    {
      s = 0;
    }
    return std::make_unsigned_t<decltype(s)>(s);
  }

private:
  V base_{};
  std::ranges::range_difference_t<V> n_;
};

template <class R>
slide_view(R&&, std::ranges::range_difference_t<R>) -> slide_view<std::views::all_t<R>>;

namespace views
{

template <std::ranges::viewable_range R>
constexpr auto slide(R&& r, std::ranges::range_difference_t<R> n)
{
  return ranges::slide_view{std::forward<R>(r), n};
};

namespace detail
{

template <typename DifferenceType>
struct slide_view_helper
{
  DifferenceType n;
};

template <typename DifferenceType>
slide_view_helper(DifferenceType) -> slide_view_helper<DifferenceType>;

template <std::ranges::viewable_range R, typename DifferenceType>
auto operator|(R&& r, const slide_view_helper<DifferenceType>& h)
{
  return slide(r, static_cast<std::ranges::range_difference_t<R>>(h.n));
}

} // namespace detail

template <typename DifferenceType>
constexpr auto slide(const DifferenceType n)
{
  return detail::slide_view_helper{n};
};

} // namespace views
} // namespace ranges

namespace views
{

template <std::ranges::viewable_range R>
constexpr auto slide(R&& r, std::ranges::range_difference_t<R> n)
{
  return ranges::views::slide(std::forward<R>(r), n);
};

template <typename DifferenceType>
constexpr auto slide(const DifferenceType n)
{
  return ranges::views::slide(n);
};

} // namespace views
} // namespace kdl

template <typename V>
constexpr bool std::ranges::enable_borrowed_range<kdl::ranges::slide_view<V>> =
  std::ranges::enable_borrowed_range<V>;
