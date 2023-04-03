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

#include <cstddef>
#include <tuple>

namespace kdl
{

namespace detail
{
template <typename T, size_t... I>
auto lift_and_impl(T&& tuple_of_functions, const std::index_sequence<I...>&)
{
  return [tuple_of_functions = std::forward<T>(tuple_of_functions)](auto&&... x) {
    return (... && std::get<I>(tuple_of_functions)(std::forward<decltype(x)>(x)...));
  };
}

template <typename T, size_t... I>
auto lift_or_impl(T&& tuple_of_functions, const std::index_sequence<I...>&)
{
  return [tuple_of_functions = std::forward<T>(tuple_of_functions)](auto&&... x) {
    return (... || std::get<I>(tuple_of_functions)(std::forward<decltype(x)>(x)...));
  };
}
} // namespace detail

template <typename... F>
auto lift_and(F&&... fun)
{
  return detail::lift_and_impl(
    std::make_tuple(std::forward<F>(fun)...), std::make_index_sequence<sizeof...(F)>{});
}

template <typename... F>
auto lift_or(F&&... fun)
{
  return detail::lift_or_impl(
    std::make_tuple(std::forward<F>(fun)...), std::make_index_sequence<sizeof...(F)>{});
}

} // namespace kdl
