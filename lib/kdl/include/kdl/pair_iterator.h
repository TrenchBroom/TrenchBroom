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

#include "range.h"

#include <cassert>
#include <iterator>
#include <tuple>

namespace kdl
{
template <typename I>
class pair_iterator
{
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = long;
  using value_type = std::tuple<typename I::reference, typename I::reference>;
  using pointer = value_type*;
  using reference = value_type&;

private:
  I m_cur1;
  I m_cur2;
  I m_end;

public:
  explicit pair_iterator(I begin, I end)
    : m_cur1{begin}
    , m_cur2{begin != end ? std::next(begin) : end}
    , m_end{end}
  {
    if (m_cur2 == m_end)
    {
      m_cur1 = end;
    }
  }

  friend bool operator<(const pair_iterator& lhs, const pair_iterator& rhs)
  {
    return std::forward_as_tuple(lhs.m_cur1, lhs.m_cur2)
           < std::forward_as_tuple(rhs.m_cur1, rhs.m_cur2);
  }

  friend bool operator>(const pair_iterator& lhs, const pair_iterator& rhs)
  {
    return std::forward_as_tuple(lhs.m_cur1, lhs.m_cur2)
           > std::forward_as_tuple(rhs.m_cur1, rhs.m_cur2);
  }

  friend bool operator==(const pair_iterator& lhs, const pair_iterator& rhs)
  {
    return std::forward_as_tuple(lhs.m_cur1, lhs.m_cur2)
           == std::forward_as_tuple(rhs.m_cur1, rhs.m_cur2);
  }

  friend bool operator!=(const pair_iterator& lhs, const pair_iterator& rhs)
  {
    return std::forward_as_tuple(lhs.m_cur1, lhs.m_cur2)
           != std::forward_as_tuple(rhs.m_cur1, rhs.m_cur2);
  }

  pair_iterator& operator++()
  {
    advance();
    return *this;
  }

  pair_iterator operator++(int)
  {
    auto result = product_iterator(*this);
    advance();
    return result;
  }

  value_type operator*() const { return dereference(); }
  value_type operator->() const { return dereference(); }

private:
  void advance()
  {
    assert(m_cur1 != m_end || m_cur2 != m_end);

    ++m_cur2;
    if (m_cur2 == m_end && m_cur1 != m_end)
    {
      ++m_cur1;
      m_cur2 = std::next(m_cur1);
      if (m_cur2 == m_end)
      {
        m_cur1 = m_end;
      }
    }
  }

  value_type dereference() const { return std::forward_as_tuple(*m_cur1, *m_cur2); }
};

/**
 * Deduction guide.
 */
template <typename I>
pair_iterator(I, I) -> pair_iterator<I>;

template <typename R>
auto make_pair_begin(R&& r)
{
  return pair_iterator{std::forward<R>(r).begin(), std::forward<R>(r).end()};
}

template <typename R>
auto make_pair_end(R&& r)
{
  return pair_iterator{std::forward<R>(r).end(), std::forward<R>(r).end()};
}

template <typename R>
auto make_pair_range(R&& r)
{
  using I = decltype(make_pair_begin(std::forward<R>(r)));
  return range<I>{make_pair_begin(r), make_pair_end(r)};
}
} // namespace kdl
