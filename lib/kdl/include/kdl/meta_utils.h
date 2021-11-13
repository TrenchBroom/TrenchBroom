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

#include <type_traits>

namespace kdl {
/**
 * Type trait that checks if a given type is contained in a template parameter pack.
 *
 * @tparam T the type to check for
 * @tparam List the parameter pack to search
 */
template <typename T, typename... List> struct meta_contains {
  static constexpr bool value = (std::is_same_v<T, List> || ...);
};

template <typename T, typename... List>
inline constexpr bool meta_contains_v = meta_contains<T, List...>::value;

/**
 * Helper struct to hold a list as the result of a metaprogram.
 *
 * @tparam Types the types contained in the type list
 */
template <typename... Types> struct meta_type_list {};

/**
 * Append T to the given list.
 *
 * @tparam T the type or value to append
 * @tparam List the list to append to
 */
template <typename T, typename... List> struct meta_append {
  /**
   * The resulting type list.
   */
  using result = meta_type_list<List..., T>;
};

/**
 * Append T to the given list if B is true.
 *
 * @tparam B a boolean that controls whether or not T is appended
 * @tparam T the type or value to append
 * @tparam List the list to append to
 */
template <bool B, typename T, typename... List> struct meta_append_if {
  /**
   * The resulting type list.
   */
  using result = typename std::conditional<
    B, typename meta_append<T, List...>::result, meta_type_list<List...>>::type;
};

/**
 * Take the front of a list and return it and the remainder.
 */
template <typename Front, typename... Remainder> struct meta_front {
  /**
   * The first element.
   */
  using front = Front;
  /**
   * The remainder of the list.
   */
  using remainder = meta_type_list<Remainder...>;
};

template <typename Subset, typename Superset> struct meta_is_subset {};

template <typename... Subset, typename... Superset>
struct meta_is_subset<meta_type_list<Subset...>, meta_type_list<Superset...>> {
  static constexpr bool value = (meta_contains<Subset, Superset...>::value && ...);
};

namespace detail {
template <typename Result, typename Cur, typename Remainder> struct meta_remove_duplicates_impl {};

template <typename... Result, typename Cur>
struct meta_remove_duplicates_impl<meta_type_list<Result...>, Cur, meta_type_list<>> {
  using result = typename meta_append_if<!meta_contains_v<Cur, Result...>, Cur, Result...>::result;
};

template <typename... Result, typename Cur, typename... Remainder>
struct meta_remove_duplicates_impl<meta_type_list<Result...>, Cur, meta_type_list<Remainder...>> {
  using result = typename meta_remove_duplicates_impl<
    typename meta_append_if<!meta_contains_v<Cur, Result...>, Cur, Result...>::result,
    typename meta_front<Remainder...>::front, typename meta_front<Remainder...>::remainder>::result;
};
} // namespace detail

/**
 * Removes all duplicates from the given list.
 *
 * The resulting list contains every distinct element of the given list in the order in which the
 * elements first appeared in the list.
 */
template <typename... List> struct meta_remove_duplicates {
  using result = typename detail::meta_remove_duplicates_impl<
    meta_type_list<>, typename meta_front<List...>::front,
    typename meta_front<List...>::remainder>::result;
};
} // namespace kdl
