/*
 Copyright (C) 2010 Kristian Duske

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

#include "kd/collection_utils.h"

#include <algorithm>
#include <functional>
#include <iterator>

namespace kdl
{

/**
 * Performs lexicographical comparison of the given maps. Entries of the maps are compared
 * using the maps' common key comparator (as returned by `key_comp()`). If and only if the
 * key comparison determines that two keys are equivalent, the given value comparator is
 * used to compare the corresponding values.
 *
 * Since this walks both maps in iteration order and expects that order to be sorted by
 * key, `M` must be an ordered map type such as `std::map` or `kdl::flat_map`. It cannot
 * be used with `std::unordered_map`, whose iteration order is unspecified.
 *
 * Returns -1 if the first map is less than the second map, or +1 in the opposite case. If
 * both maps are equivalent, i.e. the corresponding keys and values are all equivalent,
 * then 0 is returned.
 *
 * @tparam M the map type, must be ordered (e.g. std::map or kdl::flat_map)
 * @tparam D the value comparator type
 * @param map1 the first map
 * @param map2 the second map
 * @param value_cmp the value comparator
 * @return an int indicating the result of the comparison
 */
template <typename M, typename D = std::less<typename M::mapped_type>>
int map_lexicographical_compare(const M& map1, const M& map2, const D& value_cmp = D())
{
  const auto key_cmp = map1.key_comp();
  return kdl::range_lexicographical_compare(
    std::begin(map1),
    std::end(map1),
    std::begin(map2),
    std::end(map2),
    [&key_cmp, &value_cmp](const auto& lhs, const auto& rhs) {
      return key_cmp(lhs.first, rhs.first)   ? true
             : key_cmp(rhs.first, lhs.first) ? false
                                             : value_cmp(lhs.second, rhs.second);
    });
}

/**
 * Checks if the given maps are equivalent according to the given value comparator. The
 * maps are considered equivalent if and only if they have the same number of entries, and
 * for each entry in the first map, the second map contains an entry with an equivalent
 * key (per `M::key_type`'s equality) whose value is equivalent according to `value_cmp`.
 *
 * Unlike `map_lexicographical_compare`, this does not depend on iteration order, so `M`
 * may be any map type that supports `size()`, iteration, and `find()` - including
 * `std::map`, `std::unordered_map`, and `kdl::flat_map`.
 *
 * @tparam M the map type
 * @tparam D the value comparator type
 * @param map1 the first map
 * @param map2 the second map
 * @param value_cmp the value comparator
 * @return true if the given maps are equivalent and false otherwise
 */
template <typename M, typename D = std::less<typename M::mapped_type>>
bool map_is_equivalent(const M& map1, const M& map2, const D& value_cmp = D())
{
  if (map1.size() != map2.size())
  {
    return false;
  }

  return std::ranges::all_of(map1, [&](const auto& entry) {
    const auto it = map2.find(entry.first);
    return it != std::end(map2) && !value_cmp(entry.second, it->second)
           && !value_cmp(it->second, entry.second);
  });
}

/**
 * Returns the value of the given key or the given default value if the given map does not
 * contain the given key.
 *
 * @tparam M the map type
 * @param m the map
 * @param k the key to find
 * @param default_value the value to return if the given key is not found
 * @return the value of the given key in the given map, or the given default value
 */
template <typename M>
const typename M::mapped_type& map_find_or_default(
  const M& m, const typename M::key_type& k, const typename M::mapped_type& default_value)
{
  const auto it = m.find(k);
  return it != std::end(m) ? it->second : default_value;
}

/**
 * Returns a map containing the key / value pairs from both of the given maps. If a key is
 * present in both of the maps, then the value from the second given map is retained.
 *
 * @tparam M the map type
 * @param m1 the first map
 * @param m2 the second map
 * @return a map containing the union of the key / value pairs from both maps, with
 * duplicate keys retaining the values from the second map
 */
template <typename M>
M map_union(const M& m1, const M& m2)
{
  auto result = M{};
  result.insert(
    std::begin(m2),
    std::end(m2)); // insert doesn't overwrite, so we need to insert m2 first
  result.insert(std::begin(m1), std::end(m1));
  return result;
}

/**
 * Merges two maps where the values are vectors of a common type. The resulting map will
 * contain the keys from both maps, and common keys contain the values from both maps
 * appended. Specifically, if a key is present in both maps, then the vector from the
 * second map is appended to the vector from the first map, and the resulting vector is
 * the value for the key in the returned map.
 *
 * @tparam M the map type; M::mapped_type must be a sequence container such as std::vector
 * @param m1 the first map
 * @param m2 the second map
 * @return a map that contains all keys from both maps, with the values appended as
 * described above
 */
template <typename M>
M map_merge(const M& m1, const M& m2)
{
  if (m1.empty())
  {
    return m2;
  }
  if (m2.empty())
  {
    return m1;
  }

  auto result = m1;
  for (const auto& [key, from] : m2)
  {
    auto& into = result[key];
    into.insert(std::end(into), std::begin(from), std::end(from));
  }
  return result;
}

/**
 * For each vector stored as a value in the given map, applies the given deleter to each
 * element of the vector, and subsequently clears the given map.
 *
 * @tparam M the map type; M::mapped_type must be a std::vector of pointers
 * @tparam D the deleter type, defaults to deleter
 * @param m the map
 * @param deleter the deleter to apply
 */
template <typename M, typename D = deleter<typename M::mapped_type::value_type>>
void map_clear_and_delete(M& m, const D& deleter = D())
{
  // uses auto&& (rather than auto&) because kdl::flat_map's iterator returns pairs of
  // references by value, which a plain lvalue reference cannot bind to
  for (auto&& [key, value] : m)
  {
    kdl::col_delete_all(value, deleter);
  }
  m.clear();
}

} // namespace kdl
