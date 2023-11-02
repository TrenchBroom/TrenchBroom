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
#include <utility>

namespace kdl
{

template <typename I, typename Predicate>
class grouped_iterator
{
public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = range<I>;
  using difference_type = typename I::difference_type;
  using pointer = value_type*;
  using reference = value_type&;

private:
  range<I> m_range;
  Predicate m_predicate;
  range<I> m_group;

public:
  grouped_iterator(range<I> range, Predicate predicate)
    : m_range{std::move(range)}
    , m_predicate{std::move(predicate)}
    , m_group{
        m_range.begin() != m_range.end()
          ? find_next(m_range.begin(), m_range.end(), m_predicate)
          : m_range}
  {
  }

  bool operator<(const grouped_iterator& other) const
  {
    return m_group.begin() < other.m_group.begin();
  }
  bool operator>(const grouped_iterator& other) const
  {
    return m_group.begin() > other.m_group.begin();
  }
  bool operator==(const grouped_iterator& other) const
  {
    return m_group.begin() == other.m_group.begin();
  }
  bool operator!=(const grouped_iterator& other) const { return !(*this == other); }

  // prefix
  grouped_iterator& operator++()
  {
    if (m_group.begin() != m_range.end())
    {
      m_group = find_next(m_group.end(), m_range.end(), m_predicate);
    }
    return *this;
  }
  grouped_iterator& operator--()
  {
    if (m_group.begin() != m_range.begin())
    {
      m_group = find_prev(m_range.begin(), m_group.begin(), m_predicate);
    }
    return *this;
  }

  // postfix
  grouped_iterator operator++(int)
  {
    auto result = grouped_iterator{*this};
    ++*this;
    return result;
  }
  grouped_iterator operator--(int)
  {
    auto result = grouped_iterator{*this};
    --*this;
    return result;
  }

  value_type operator*() { return m_group; }

private:
  static range<I> find_prev(I range_begin, I group_begin, const Predicate& predicate)
  {
    assert(group_begin != range_begin);

    auto group_end = group_begin;
    group_begin = std::prev(group_end);
    while (group_begin != range_begin && predicate(std::prev(group_begin), group_begin))
    {
      --group_begin;
    }

    return {group_begin, group_end};
  }

  static range<I> find_next(I group_end, I range_end, const Predicate& predicate)
  {
    auto group_begin = group_end;
    while (group_end != range_end && predicate(*group_begin, *group_end))
    {
      ++group_end;
    }

    return {group_begin, group_end};
  }
};

template <typename I, typename Predicate>
grouped_iterator(I, Predicate) -> grouped_iterator<I, Predicate>;

template <typename C, typename Predicate>
auto make_grouped_range(const C& c, Predicate predicate)
{
  return range{
    grouped_iterator{range{c.begin(), c.end()}, predicate},
    grouped_iterator{range{c.end(), c.end()}, predicate},
  };
}

} // namespace kdl
