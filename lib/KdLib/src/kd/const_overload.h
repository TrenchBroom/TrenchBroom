/*
 Copyright 2025 Kristian Duske

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
#include <utility>
#include <vector>

namespace kdl::detail
{

template <typename T>
constexpr decltype(auto) asConst(T& t)
{
  static_assert(!std::is_const_v<T>, "must not be const");
  return std::as_const(t);
}

template <typename T>
constexpr decltype(auto) asNonConstReturnValue(const T* t)
{
  return const_cast<T*>(t);
}

template <typename T>
constexpr decltype(auto) asNonConstReturnValue(const T& t)
{
  return const_cast<T&>(t);
}

template <typename T>
decltype(auto) asNonConstReturnValue(const std::vector<const T*>& t)
{
  const auto pNonConstData = const_cast<T**>(t.data());
  return std::vector<T*>{pNonConstData, pNonConstData + t.size()};
}

} // namespace kdl::detail

#define KDL_CONST_OVERLOAD(f)                                                            \
  kdl::detail::asNonConstReturnValue(kdl::detail::asConst(*this).f)
