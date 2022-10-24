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

#include <tuple>

namespace kdl
{
inline auto tup_capture()
{
  return std::tuple<>{};
}

template <typename Type>
auto tup_capture(Type&& arg)
{
  if constexpr (std::is_rvalue_reference_v<Type>)
  {
    return std::tuple<std::decay_t<Type>>{std::forward<Type>(arg)};
  }
  return std::tuple<Type>{std::forward<Type>(arg)};
}

/**
 * Capture the given values as a tuple. The resulting tuple has the same number of
 * elements as the number of the given arguments. For each argument x of type X, the
 * corresponding tuple member has type
 *
 * - const X& if x is a const lvalue reference,
 * - X& if x is an lvalue reference,
 * - X if x is an rvalue.
 *
 * In the last case where x is an rvalue, the value of x is moved into the tuple.
 *
 * @tparam First the type of the first argument
 * @tparam Rest the types of the remaining arguments
 * @param first the first argument
 * @param rest the remaining arguments
 *
 * @return a tuple containing the values of the arguments, as per the rules defined above
 */
template <typename First, typename... Rest>
auto tup_capture(First&& first, Rest&&... rest)
{
  return std::tuple_cat(
    tup_capture(std::forward<First>(first)), tup_capture(std::forward<Rest>(rest)...));
}
} // namespace kdl
