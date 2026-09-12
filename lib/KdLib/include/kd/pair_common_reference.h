/*
 Copyright (C) 2026 Kristian Duske

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

// This file is unnecessary when compiling with C++23 or later because the std
// library already contains a working implementation of std::basic_common_reference for
// pair-likes.
//
// This is the std::pair counterpart to kd/ranges/detail/tuple_common_reference.h, which
// does the same thing for std::tuple to support the ranges backports in kd/ranges/. This
// one exists to let kdl::flat_map's iterator, which synthesizes a std::pair<const Key&,
// T&> on dereference rather than referencing an actually stored pair (since keys and
// values live in separate parallel containers), satisfy std::indirectly_readable and
// therefore std::input_iterator. Without it, kdl::flat_map cannot be used with C++20
// ranges algorithms or views (e.g. std::ranges::all_of, std::views::keys).

#if __cplusplus == 202002L

#include <type_traits>
#include <utility>

namespace kdl::detail
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

} // namespace kdl::detail

namespace std
{

//! see https://cppreference.net/cpp/utility/tuple/basic_common_reference.html
//! Note: instead of requiring TPair and UPair to be pair-like, we require both to
//! specialize std::pair. This is enough to fix kdl::flat_map's iterator.
template <
  typename TPair,
  typename UPair,
  template <class>
  class TQual,
  template <class>
  class UQual>
  requires(
    kdl::detail::is_specialization_of_v<std::remove_cvref_t<TPair>, std::pair>
    && kdl::detail::is_specialization_of_v<std::remove_cvref_t<UPair>, std::pair>
    && std::is_same_v<TPair, std::decay_t<TPair>>
    && std::is_same_v<UPair, std::decay_t<UPair>>)
struct basic_common_reference<TPair, UPair, TQual, UQual>
{
  template <typename T, typename U>
  struct pair_common_reference_helper
  {
  };

  template <typename T1, typename T2, typename U1, typename U2>
  struct pair_common_reference_helper<std::pair<T1, T2>, std::pair<U1, U2>>
  {
    using type = std::pair<
      std::common_reference_t<TQual<T1>, UQual<U1>>,
      std::common_reference_t<TQual<T2>, UQual<U2>>>;
  };

  using type = pair_common_reference_helper<TPair, UPair>::type;
};

} // namespace std

#endif
