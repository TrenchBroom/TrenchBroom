/*
 Copyright 2022 Kristian Duske

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

#include <ostream>

namespace kdl {
template <typename R> struct streamable_range_wrapper { const R& range; };

template <typename R> auto make_streamable(R&& range) {
  return streamable_range_wrapper<R>{std::forward<R>(range)};
}

template <typename R>
std::ostream& operator<<(std::ostream& lhs, const streamable_range_wrapper<R>& adapter) {
  using std::begin;
  using std::end;

  auto cur = begin(adapter.range);
  if (cur != end(adapter.range)) {
    lhs << "[";
    lhs << *cur++;
    while (cur != end(adapter.range)) {
      lhs << "," << *cur++;
    }
    lhs << "]";
  }
  return lhs;
}
} // namespace kdl
