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

#include <iterator>

namespace kdl {
namespace detail {
// To allow ADL with custom begin/end
using std::begin;
using std::end;

template <typename T>
auto is_iterable(int) -> decltype(
  begin(std::declval<T&>()) != end(std::declval<T&>()),   // begin/end and operator !=
  void(),                                                 // Handle evil operator ,
  ++std::declval<decltype(begin(std::declval<T&>()))&>(), // operator ++
  void(*begin(std::declval<T&>())),                       // operator*
  std::true_type{});

template <typename T> std::false_type is_iterable(...);

} // namespace detail

template <typename T> using is_iterable = decltype(detail::is_iterable<T>(0));
template <typename T> inline constexpr bool is_iterable_v = is_iterable<T>::value;

namespace detail {
template <typename T, typename S, typename = void> struct can_print : std::false_type {};

template <typename T, typename S>
struct can_print<T, S, std::void_t<decltype(std::declval<S&>() << std::declval<const T&>())>>
  : std::true_type {};

} // namespace detail

template <typename T, typename S = std::ostream> using can_print = detail::can_print<T, S>;

template <typename T, typename S = std::ostream>
inline constexpr bool can_print_v = can_print<T, S>::value;

} // namespace kdl
