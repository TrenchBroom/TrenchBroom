/*
 Copyright 2021 Kristian Duske

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

namespace kdl
{
template <typename I>
class deref_iterator
{
public:
  using iterator_category = typename I::iterator_category;
  using difference_type = typename I::difference_type;
  using value_type =
    std::remove_reference_t<decltype(*(std::declval<typename I::value_type>()))>;
  using reference = std::add_lvalue_reference_t<value_type>;
  using pointer = std::add_pointer_t<value_type>;

private:
  I m_it;

public:
  explicit deref_iterator(I it)
    : m_it{it}
  {
  }

  friend bool operator<(const deref_iterator& lhs, const deref_iterator& rhs)
  {
    return lhs.m_it < rhs.m_it;
  }
  friend bool operator<(const deref_iterator& lhs, const I& rhs)
  {
    return lhs.m_it < rhs;
  }
  friend bool operator<(const I& lhs, const deref_iterator& rhs)
  {
    return lhs < rhs.m_it;
  }

  friend bool operator>(const deref_iterator& lhs, const deref_iterator& rhs)
  {
    return lhs.m_it > rhs.m_it;
  }
  friend bool operator>(const deref_iterator& lhs, const I& rhs)
  {
    return lhs.m_it > rhs;
  }
  friend bool operator>(const I& lhs, const deref_iterator& rhs)
  {
    return lhs > rhs.m_it;
  }

  friend bool operator==(const deref_iterator& lhs, const deref_iterator& rhs)
  {
    return lhs.m_it == rhs.m_it;
  }
  friend bool operator==(const deref_iterator& lhs, const I& rhs)
  {
    return lhs.m_it == rhs;
  }
  friend bool operator==(const I& lhs, const deref_iterator& rhs)
  {
    return lhs == rhs.m_it;
  }

  friend bool operator!=(const deref_iterator& lhs, const deref_iterator& rhs)
  {
    return lhs.m_it != rhs.m_it;
  }
  friend bool operator!=(const deref_iterator& lhs, const I& rhs)
  {
    return lhs.m_it != rhs;
  }
  friend bool operator!=(const I& lhs, const deref_iterator& rhs)
  {
    return lhs != rhs.m_it;
  }

  friend deref_iterator operator+(const deref_iterator& lhs, const difference_type rhs)
  {
    return deref_iterator{lhs.m_it + rhs};
  }

  friend deref_iterator operator+(const difference_type& lhs, const deref_iterator rhs)
  {
    return rhs + lhs;
  }

  friend deref_iterator& operator+=(deref_iterator& lhs, const difference_type rhs)
  {
    lhs.m_it += rhs;
    return lhs;
  }

  friend difference_type operator-(const deref_iterator& lhs, const deref_iterator& rhs)
  {
    return lhs.m_it - rhs.m_it;
  }

  friend deref_iterator operator-(const deref_iterator& lhs, const difference_type rhs)
  {
    return deref_iterator{lhs.m_it - rhs};
  }

  friend deref_iterator& operator-=(deref_iterator& lhs, const difference_type rhs)
  {
    lhs.m_it -= rhs;
    return lhs;
  }

  reference operator[](const difference_type n) const { return *m_it[n]; }

  deref_iterator& operator++()
  {
    m_it++;
    return *this;
  }

  deref_iterator operator++(int)
  {
    auto result = deref_iterator(*this);
    ++m_it;
    return result;
  }

  reference operator*() const { return **m_it; }
  pointer operator->() const { return *m_it; }
};

template <typename I>
deref_iterator(I) -> deref_iterator<I>;

/**
 * Creates a deref range of the given range.
 *
 * @tparam C the types of the range
 * @param c the ranges to iterate
 */
template <typename C>
auto make_deref_range(C&& c)
{
  return range{deref_iterator(std::begin(c)), deref_iterator(std::end(c))};
}
} // namespace kdl
