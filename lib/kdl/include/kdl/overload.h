/*
 Copyright 2010-2019 Kristian Duske

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

#include <utility>

namespace kdl {
/**
 * Creates a type that inherits from all of its type parameters and `operator()` from each
 * supertype. Can be used with std::visit to create on-the-fly visitors from lambdas. So usually,
 * the supertypes are lambdas.
 *
 * @see https://dev.to/tmr232/that-overloaded-trick-overloading-lambdas-in-c17
 *
 * @tparam Ts the lambdas to inherit from
 */
template <typename... Ts> struct overload_impl : Ts... {
  using Ts::operator()...;
  overload_impl(Ts&&... ts)
    : Ts(std::forward<Ts>(ts))... {}
};

/**
 * Factory function to create an overload.
 */
template <typename... Ts> auto overload(Ts&&... ts) {
  return overload_impl<Ts...>{std::forward<Ts>(ts)...};
}
} // namespace kdl
