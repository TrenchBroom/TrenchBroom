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
#include "detail/non_propagating_cache.h"

#include <algorithm>
#include <functional>
#include <ranges>

namespace kdl
{
namespace ranges
{
namespace detail
{

template <typename V>
constexpr auto get_chunk_by_iter_tag()
{
  if constexpr (std::ranges::bidirectional_range<V>)
  {
    return std::bidirectional_iterator_tag{};
  }
  else
  {
    return std::forward_iterator_tag{};
  }
}

} // namespace detail

template <
  std::ranges::forward_range V,
  std::indirect_binary_predicate<std::ranges::iterator_t<V>, std::ranges::iterator_t<V>>
    Pred>
  requires std::ranges::view<V> && std::is_object_v<Pred>
class chunk_by_view : public std::ranges::view_interface<chunk_by_view<V, Pred>>
{
public:
  class iterator
  {
  public:
    using value_type = std::ranges::subrange<std::ranges::iterator_t<V>>;
    using difference_type = std::ranges::range_difference_t<V>;
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = decltype(detail::get_chunk_by_iter_tag<V>());

    iterator() = default;

    constexpr value_type operator*() const
    {
      return std::ranges::subrange{current_, next_};
    }

    constexpr iterator& operator++()
    {
      current_ = next_;
      next_ = parent_->find_next(current_);
      return *this;
    }

    constexpr iterator operator++(int)
    {
      const auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator& operator--()
      requires std::ranges::bidirectional_range<V>
    {
      next_ = current_;
      current_ = parent_->find_prev(next_);
      return *this;
    }

    constexpr iterator operator--(int)
      requires std::ranges::bidirectional_range<V>
    {
      const auto tmp = *this;
      --*this;
      return tmp;
    }

    friend constexpr bool operator==(const iterator& x, const iterator& y)
    {
      return x.current_ == y.current_;
    }

    friend constexpr bool operator==(const iterator& x, const std::default_sentinel_t)
    {
      return x.current_ == x.next_;
    }

  private:
    friend class chunk_by_view;

    constexpr iterator(
      chunk_by_view& parent,
      std::ranges::iterator_t<V> current,
      std::ranges::iterator_t<V> next)
      : parent_{std::addressof(parent)}
      , current_{std::move(current)}
      , next_{std::move(next)}
    {
    }

    chunk_by_view* parent_{nullptr};
    std::ranges::iterator_t<V> current_{};
    std::ranges::iterator_t<V> next_{};
  };

  chunk_by_view()
    requires std::default_initializable<V> && std::default_initializable<Pred>
  = default;

  constexpr explicit chunk_by_view(V base, Pred pred)
    : base_{std::move(base)}
    , pred_{std::move(pred)}
  {
  }

  constexpr V base() const&
    requires std::copy_constructible<V>
  {
    return base_;
  }

  constexpr V base() && { return std::move(base_); }

  constexpr const Pred& pred() const { return *pred_; }

  constexpr iterator begin()
  {
    if (!begin_)
    {
      begin_ = find_next(std::ranges::begin(base_));
    }

    return iterator{*this, std::ranges::begin(base_), begin_.value()};
  }

  constexpr auto end()
  {
    if constexpr (std::ranges::common_range<V>)
    {
      return iterator{*this, std::ranges::end(base_), std::ranges::end(base_)};
    }
    else
    {
      return std::default_sentinel;
    }
  }

private:
  constexpr std::ranges::iterator_t<V> find_next(std::ranges::iterator_t<V> current)
  {
    return std::ranges::next(
      std::ranges::adjacent_find(
        current, std::ranges::end(base_), std::not_fn(std::ref(*pred_))),
      1,
      std::ranges::end(base_));
  }

  constexpr std::ranges::iterator_t<V> find_prev(std::ranges::iterator_t<V> current)
    requires std::ranges::bidirectional_range<V>
  {
    auto i = std::ranges::prev(current);
    while (i != std::ranges::begin(base_)
           && std::invoke(*pred_, *std::ranges::prev(i), *i))
    {
      --i;
    }
    return i;
  }

  V base_{};
  movable_box<Pred> pred_{};
  non_propagating_cache<std::ranges::iterator_t<V>> begin_{};
};

template <class R, class Pred>
chunk_by_view(R&&, Pred&&) -> chunk_by_view<std::views::all_t<R>, Pred>;

namespace views
{

template <std::ranges::viewable_range R, typename Pred>
constexpr auto chunk_by(R&& r, Pred&& pred)
{
  return ranges::chunk_by_view{std::forward<R>(r), std::forward<Pred>(pred)};
};

namespace detail
{

template <typename Pred>
struct chunk_by_view_helper
{
  Pred pred;
};

template <typename Pred>
chunk_by_view_helper(Pred) -> chunk_by_view_helper<Pred>;

template <std::ranges::viewable_range R, typename Pred>
auto operator|(R&& r, const chunk_by_view_helper<Pred>& h)
{
  return chunk_by(std::forward<R>(r), h.pred);
}

template <std::ranges::viewable_range R, typename Pred>
auto operator|(R&& r, chunk_by_view_helper<Pred>&& h)
{
  return chunk_by(std::forward<R>(r), std::move(h.pred));
}

} // namespace detail

template <typename Pred>
constexpr auto chunk_by(Pred&& pred)
{
  return detail::chunk_by_view_helper{std::forward<Pred>(pred)};
};

} // namespace views
} // namespace ranges

namespace views
{

template <std::ranges::viewable_range R>
constexpr auto chunk_by(R&& r, std::ranges::range_difference_t<R> n)
{
  return ranges::views::chunk_by(std::forward<R>(r), n);
};

template <typename DifferenceType>
constexpr auto chunk_by(const DifferenceType n)
{
  return ranges::views::chunk_by(n);
};

} // namespace views
} // namespace kdl
