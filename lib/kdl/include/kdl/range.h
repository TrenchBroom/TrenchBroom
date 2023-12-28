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

#include <algorithm>

namespace kdl
{

/**
 * A range with begin and end members.
 *
 * @tparam I the types of the individual iterators
 */
template <typename I>
struct range
{
  using value_type = typename I::value_type;
  using iterator = I;

  I m_begin;
  I m_end;

  I begin() const { return m_begin; }

  I end() const { return m_end; }

  auto front() const { return *m_begin; }
  auto back() const { return *std::prev(m_end); }

  bool empty() const { return m_begin == m_end; }

  template <typename I2>
  bool operator==(const range<I2>& other) const
  {
    return std::equal(m_begin, m_end, other.m_begin, other.m_end);
  }

  template <typename I2>
  bool operator!=(const range<I2>& other) const
  {
    return !(*this == other);
  }
};

template <typename I>
range(I, I) -> range<I>;

} // namespace kdl
