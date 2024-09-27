/*
 Copyright 2024 Kristian Duske

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

#include <iterator>
#include <numeric>

namespace kdl
{

auto fold_left = [](auto&& range, auto&& init, auto&& op) {
  return std::accumulate(
    std::begin(range),
    std::end(range),
    std::forward<decltype(init)>(init),
    std::forward<decltype(op)>(op));
};

auto fold_right = [](auto&& range, auto&& init, auto&& op) {
  return std::accumulate(
    std::make_reverse_iterator(std::end(range)),
    std::make_reverse_iterator(std::begin(range)),
    std::forward<decltype(init)>(init),
    std::forward<decltype(op)>(op));
};

auto fold_left_first = [](auto&& range, auto&& op) {
  return fold_left(range, *std::begin(range), std::forward<decltype(op)>(op));
};

auto fold_right_first = [](auto&& range, auto&& op) {
  return fold_right(range, *std::prev(std::end(range)), std::forward<decltype(op)>(op));
};

} // namespace kdl
