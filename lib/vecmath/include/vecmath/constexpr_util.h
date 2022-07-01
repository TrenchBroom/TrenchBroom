/*
 Copyright 2010-2019 Kristian Duske
 Copyright 2015-2019 Eric Wasylishen

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <cstddef>
#include <iterator>

namespace vm {
namespace detail {
/**
 * Swaps the given values.
 *
 * @tparam T the value type
 * @param lhs the left hand side
 * @param rhs the right hand side
 */
template <class T> constexpr void swap(T& lhs, T& rhs) {
  auto tmp = std::move(lhs);
  lhs = std::move(rhs);
  rhs = std::move(tmp);
}

/**
 * Swaps the contents of the given iterators.
 *
 * @tparam I the iterator type
 * @param lhs the left hand side
 * @param rhs the right hand side
 */
template <class I> constexpr void iter_swap(I lhs, I rhs) {
  swap(*lhs, *rhs);
}

/**
 * Sorts the given range using the given comparator.
 *
 * @tparam I the iterator type, which must be a random access iterator
 * @tparam C the comparator type
 * @param left the beginning of the range
 * @param right the end of the range
 * @param cmp the comparator, which must induce a strict weak ordering on the elements in the given
 * range
 */
template <typename I, typename C> constexpr void sort(I left, I right, const C& cmp) {
  if (left < right) {
    auto mid = left;

    for (auto i = std::next(left); i < right; ++i) {
      if (cmp(*i, *left)) {
        iter_swap(++mid, i);
      }
    }

    iter_swap(left, mid);
    sort(left, mid, cmp);
    sort(std::next(mid), right, cmp);
  }
}
} // namespace detail
} // namespace vm
