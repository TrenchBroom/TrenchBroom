/*
 Copyright 2010-2019 Kristian Duske

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

namespace kdl
{
/**
 * Type trait that checks if a given type is contained in a template parameter pack.
 *
 * @tparam T the type to check for
 * @tparam List the parameter pack to search
 */
template <typename T, typename... List>
struct meta_contains
{
  static constexpr auto value = (std::is_same_v<T, List> || ...);
};

template <typename T, typename... List>
inline constexpr auto meta_contains_v = meta_contains<T, List...>::value;

/**
 * Helper struct to hold a list as the result of a metaprogram.
 *
 * @tparam Types the types contained in the type list
 */
template <typename... Types>
struct meta_type_list
{
};

/**
 * Append T to the given list.
 *
 * @tparam T the type or value to append
 * @tparam List the list to append to
 */
template <typename T, typename... List>
struct meta_append
{
  /**
   * The resulting type list.
   */
  using type = meta_type_list<List..., T>;
};

template <typename T, typename... List>
using meta_append_t = typename meta_append<T, List...>::type;

/**
 * Append T to the given list if B is true.
 *
 * @tparam B a boolean that controls whether or not T is appended
 * @tparam T the type or value to append
 * @tparam List the list to append to
 */
template <bool B, typename T, typename... List>
struct meta_append_if
{
  /**
   * The resulting type list.
   */
  using type = typename std::
    conditional<B, meta_append_t<T, List...>, meta_type_list<List...>>::type;
};

template <bool B, typename T, typename... List>
using meta_append_if_t = typename meta_append_if<B, T, List...>::type;

/**
 * Take the front of a list and return it and the remainder.
 */
template <typename Front, typename... Remainder>
struct meta_front
{
  /**
   * The first element.
   */
  using front = Front;
  /**
   * The remainder of the list.
   */
  using remainder = meta_type_list<Remainder...>;
};

template <typename... T>
using meta_front_t = typename meta_front<T...>::front;

template <typename... T>
using meta_remainder_t = typename meta_front<T...>::remainder;

template <typename Subset, typename Superset>
struct meta_is_subset
{
};

template <typename... Subset, typename... Superset>
struct meta_is_subset<meta_type_list<Subset...>, meta_type_list<Superset...>>
{
  static constexpr auto value = (meta_contains<Subset, Superset...>::value && ...);
};

template <typename Subset, typename Superset>
inline constexpr auto meta_is_subset_v = meta_is_subset<Subset, Superset>::value;

namespace detail
{
template <typename Result, typename Cur, typename Remainder>
struct meta_remove_duplicates_impl
{
};

template <typename... Result, typename Cur>
struct meta_remove_duplicates_impl<meta_type_list<Result...>, Cur, meta_type_list<>>
{
  using type = meta_append_if_t<!meta_contains_v<Cur, Result...>, Cur, Result...>;
};

template <typename... Result, typename Cur, typename... Remainder>
struct meta_remove_duplicates_impl<
  meta_type_list<Result...>,
  Cur,
  meta_type_list<Remainder...>>
{
  using type = typename meta_remove_duplicates_impl<
    meta_append_if_t<!meta_contains_v<Cur, Result...>, Cur, Result...>,
    meta_front_t<Remainder...>,
    meta_remainder_t<Remainder...>>::type;
};
} // namespace detail

/**
 * Removes all duplicates from the given list.
 *
 * The resulting list contains every distinct element of the given list in the order in
 * which the elements first appeared in the list.
 */
template <typename... List>
struct meta_remove_duplicates
{
  using type = typename detail::meta_remove_duplicates_impl<
    meta_type_list<>,
    meta_front_t<List...>,
    meta_remainder_t<List...>>::type;
};

template <>
struct meta_remove_duplicates<>
{
  using type = meta_type_list<>;
};

template <typename... List>
using meta_remove_duplicates_t = typename meta_remove_duplicates<List...>::type;

} // namespace kdl
