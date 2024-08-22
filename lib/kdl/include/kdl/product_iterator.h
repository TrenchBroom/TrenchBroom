/*
 Copyright 2023 Kristian Duske

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

#include "kdl/range.h"

#include <cassert>
#include <iterator>
#include <tuple>

namespace kdl
{
/**
 * Wraps several iterators and offers every combination of their values as a tuple of
 * references.
 *
 * @tparam I the types of the iterators
 */
template <typename... I>
class product_iterator
{
public:
  static_assert(sizeof...(I) > 0, "At least one iterator is required.");

  using iterator_category = std::forward_iterator_tag;
  using difference_type = long;
  using value_type = std::tuple<typename I::reference...>;
  using pointer = value_type*;
  using reference = value_type&;

private:
  using tuple_type = std::tuple<I...>;
  tuple_type m_begins;
  tuple_type m_ends;
  tuple_type m_curs;

public:
  explicit product_iterator(I... begins, I... ends)
    : m_begins{std::forward<I>(begins)...}
    , m_ends{std::forward<I>(ends)...}
    , m_curs{m_begins}
  {
    if (((std::get<I>(m_begins) == std::get<I>(m_ends)) || ...))
    {
      m_curs = m_ends;
    }
  }

  friend bool operator<(const product_iterator& lhs, const product_iterator& rhs)
  {
    return lhs.m_curs < rhs.m_curs;
  }
  friend bool operator>(const product_iterator& lhs, const product_iterator& rhs)
  {
    return lhs.m_curs > rhs.m_curs;
  }

  friend bool operator==(const product_iterator& lhs, const product_iterator& rhs)
  {
    return lhs.m_curs == rhs.m_curs;
  }

  friend bool operator!=(const product_iterator& lhs, const product_iterator& rhs)
  {
    return lhs.m_curs != rhs.m_curs;
  }

  product_iterator& operator++()
  {
    advance();
    return *this;
  }

  product_iterator operator++(int)
  {
    auto result = product_iterator(*this);
    advance();
    return result;
  }

  value_type operator*() const { return dereference(); }
  value_type operator->() const { return dereference(); }

private:
  template <size_t C>
  bool advance()
  {
    if constexpr (C < std::tuple_size_v<tuple_type>)
    {
      auto& cur = std::get<C>(m_curs);
      if (cur == std::get<C>(m_ends))
      {
        return false;
      }

      ++cur;
      if (cur != std::get<C>(m_ends))
      {
        return true;
      }

      if (advance<C + 1>())
      {
        cur = std::get<C>(m_begins);
        return true;
      }
    }

    return false;
  }

  void advance()
  {
    assert(((std::get<I>(m_curs) != std::get<I>(m_ends)) || ...));
    advance<0>();
  }

  template <size_t... Idx>
  value_type dereference(std::index_sequence<Idx...>) const
  {
    return std::forward_as_tuple(*std::get<Idx>(m_curs)...);
  }

  value_type dereference() const
  {
    return dereference(std::make_index_sequence<std::tuple_size_v<tuple_type>>{});
  }
};

/**
 * Deduction guide.
 */
template <typename... R>
product_iterator(R... ranges) -> product_iterator<typename R::iterator...>;

template <typename... R>
auto make_product_begin(R&&... r)
{
  return product_iterator<decltype(std::forward<R>(r).begin())...>{
    std::forward<R>(r).begin()..., std::forward<R>(r).end()...};
}

template <typename... R>
auto make_product_end(R&&... r)
{
  return product_iterator<decltype(std::forward<R>(r).begin())...>{
    std::forward<R>(r).end()..., std::forward<R>(r).end()...};
}

template <typename... R>
auto make_product_range(R&&... r)
{
  using I = decltype(make_product_begin(std::forward<R>(r)...));
  return range<I>{
    make_product_begin(std::forward<R>(r)...), make_product_end(std::forward<R>(r)...)};
}
} // namespace kdl
