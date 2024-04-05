/*
Copyright 2024 Ennis Massey

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
#include <type_traits>

namespace kdl
{

namespace ct_map
{

/**
 * Compile time pair between `I` and `T`, allowing mapping of constant values to types
 * @tparam IT type of `I`
 * @tparam I value of Key
 * @tparam T type `I` maps to
 */
template <typename IT, IT I, typename T>
struct ct_pair
{
  using type = T;

  static ct_pair get_pair(std::integral_constant<IT, I>) { return {}; }
};

template <typename... Pairs>
struct ct_map : public Pairs...
{
  using Pairs::get_pair...;

  template <typename IT, IT I>
  using find_type = typename decltype(get_pair(std::integral_constant<IT, I>{}))::type;
};
} // namespace ct_map
} // namespace kdl