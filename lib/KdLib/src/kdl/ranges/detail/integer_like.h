/*
 Copyright (C) 2025 Kristian Duske

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

#include <type_traits>

namespace kdl::ranges::detail
{

template <class T>
struct is_integer_like_impl
{
private:
  using U = std::remove_cv_t<std::remove_reference_t<T>>;
  static constexpr bool integral_but_not_bool =
    std::is_integral_v<U> && !std::is_same_v<U, bool>;

public:
  static constexpr bool value = integral_but_not_bool;
};

template <class T>
struct is_integer_like : std::integral_constant<bool, is_integer_like_impl<T>::value>
{
};

template <class T>
constexpr bool is_integer_like_v = is_integer_like<T>::value;

template <class T>
struct is_signed_integer_like_impl
{
private:
  using U = std::remove_cv_t<std::remove_reference_t<T>>;

  // For builtin integral types exclude bool and require signedness.
  static constexpr bool signed_integral_builtin =
    std::is_integral_v<U> && !std::is_same_v<U, bool> && std::is_signed_v<U>;

public:
  static constexpr bool value = signed_integral_builtin;
};

template <class T>
struct is_signed_integer_like
  : std::integral_constant<bool, is_signed_integer_like_impl<T>::value>
{
};

template <class T>
constexpr bool is_signed_integer_like_v = is_signed_integer_like<T>::value;

} // namespace kdl::ranges::detail
