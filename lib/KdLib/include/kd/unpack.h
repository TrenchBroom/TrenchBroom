/*
 Copyright 2026 Kristian Duske

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

#include <tuple>
#include <utility>

namespace kdl
{

/**
 * Wraps the given function so that it can be called with a single tuple-like argument
 * instead of a list of individual arguments. The elements of the tuple are unpacked and
 * forwarded to the wrapped function as individual arguments, preserving the value
 * category of the tuple argument. The wrapped function's return value, including
 * references, is forwarded unchanged.
 *
 * @tparam F the type of the function to wrap
 * @param f_ the function to wrap
 *
 * @return a function that accepts a single tuple-like argument and calls f with its
 * unpacked elements
 */
template <typename F>
constexpr auto unpack(F&& f_)
{
  return [f = std::forward<F>(f_)]<typename T>(T&& x) -> decltype(auto) {
    return std::apply(f, std::forward<T>(x));
  };
}

} // namespace kdl
