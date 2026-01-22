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

#include <tuple>
#include <type_traits>

// This file must not be included when compiling with C++23 or later because the std
// library already contains a working implementation of std::basic_common_reference for
// tuple-likes.
static_assert(__cplusplus == 202002L);

namespace kdl::ranges::detail
{

// taken from https://www.open-std.org/JTC1/SC22/WG21/docs/papers/2020/p2098r0.pdf
template <class T, template <class...> class Primary>
struct is_specialization_of : std::false_type
{
};

template <template <class...> class Primary, class... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type
{
};

template <class T, template <class...> class Primary>
inline constexpr bool is_specialization_of_v = is_specialization_of<T, Primary>::value;

} // namespace kdl::ranges::detail

namespace std
{

//! see https://cppreference.net/cpp/utility/tuple/basic_common_reference.html
//! Note: instead of requiring TTuple and UTuple to be tuple-like, we require both to
//! specialize std::tuple. This is enough to fix enumerate_view.
template <
  typename TTuple,
  typename UTuple,
  template <class>
  class TQual,
  template <class>
  class UQual>
  requires(
    kdl::ranges::detail::is_specialization_of_v<std::remove_cvref_t<TTuple>, std::tuple>
    && kdl::ranges::detail::
      is_specialization_of_v<std::remove_cvref_t<UTuple>, std::tuple>
    && std::is_same_v<TTuple, std::decay_t<TTuple>>
    && std::is_same_v<UTuple, std::decay_t<UTuple>>
    && std::tuple_size_v<TTuple> == std::tuple_size_v<UTuple>)
struct basic_common_reference<TTuple, UTuple, TQual, UQual>
{
  template <typename T, typename U>
  struct tuple_common_reference_helper
  {
  };

  template <typename... TTypes, typename... UTypes>
  struct tuple_common_reference_helper<std::tuple<TTypes...>, std::tuple<UTypes...>>
  {
    using type = std::tuple<std::common_reference_t<TQual<TTypes>, UQual<UTypes>>...>;
  };

  using type = tuple_common_reference_helper<TTuple, UTuple>::type;
};

} // namespace std
