/*
 Copyright 2010-2019 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "collection_utils.h"

#include <map>
#include <vector>

namespace kdl {
    /**
     * Returns a vector containing copies of the given map's keys. The keys are returned in the order in which they are
     * stored in the given map.
     *
     * @tparam K the key type
     * @tparam V the value type
     * @param m the map
     * @return a vector containing the keys
     */
    template<typename K, typename V>
    std::vector<K> map_keys(const std::map<K, V>& m) {
        std::vector<K> result;
        result.reserve(m.size());
        for (const auto& [key, value] : m) {
            result.push_back(key);
        }
        return result;
    }

    /**
     * Returns a vector containing copies of the given map's values. The values are returned in the order in which they
     * appear in the given map.
     *
     * @tparam K the key type
     * @tparam V the value type
     * @param m the map
     * @return a vector containing the values
     */
    template<typename K, typename V>
    std::vector<V> map_values(const std::map<K, V>& m) {
        std::vector<V> result;
        result.reserve(m.size());
        for (const auto& [key, value] : m) {
            result.push_back(value);
        }
        return result;
    }

    /**
     * Performs lexicographical comparison of the given maps. Entries of the maps are compared using the common key
     * comparator (of type C). If and only if the key comparison determines that two keys are equivalent, the given
     * value comparator is used to compare the corresponding values.
     *
     * Returns -1 if the first map is less than the second map, or +1 in the opposite case. If both maps are equivalent,
     * i.e. the corresponding keys and values are all equivalent, then 0 is returned.
     *
     * @tparam K the key type
     * @tparam V the value type
     * @tparam C the key comparator type
     * @tparam D the value comparator type
     * @param map1 the first map
     * @param map2 the second map
     * @param value_cmp the value comparator
     * @return an int indicating the result of the comparison
     */
    template<typename K, typename V, typename C, typename D = std::less<V>>
    int map_lexicographical_compare(const std::map<K, V, C>& map1, const std::map<K, V, C>& map2, const D& value_cmp = D()) {
        const auto key_cmp = C();
        return kdl::range_lexicographical_compare(std::begin(map1), std::end(map1), std::begin(map2),
            std::end(map2),
            [&key_cmp, &value_cmp](const auto& lhs, const auto& rhs) {
                const K& lhs_key = lhs.first;
                const K& rhs_key = rhs.first;
                if (key_cmp(lhs_key, rhs_key)) {
                    return true;
                } else if (key_cmp(rhs_key, lhs_key)) {
                    return false;
                } else {
                    const V& lhs_value = lhs.second;
                    const V& rhs_value = rhs.second;
                    return value_cmp(lhs_value, rhs_value);
                }
            });
    }

    /**
     * Checks if the given maps are equivalent according to their common key comparator and the given value comparator.
     * The maps are considered equivalent if and only if lexicographical_compare returns 0. This is the case if the
     * following conditions apply:
     *
     * - the maps have the same number of entries
     * - for each pair of corresponding entries, their keys are equivalent according to their common key comparator
     * - for each pair of corresponding entries, their values are equivalent according to the given value comparator
     *
     * @tparam K the key type
     * @tparam V the value type
     * @tparam C the key comparator type
     * @tparam D the value comparator type
     * @param map1 the first map
     * @param map2 the second map
     * @param valueCmp the value comparator
     * @return true if the given maps are equivalent and false otherwise
     */
    template<typename K, typename V, typename C, typename D = std::less<V>>
    bool map_is_equivalent(const std::map<K, V, C>& map1, const std::map<K, V, C>& map2, const D& valueCmp = D()) {
        if (map1.size() != map2.size()) {
            return false;
        }

        return map_lexicographical_compare(map1, map2, valueCmp) == 0;
    }

    /**
     * Returns the value of the given key or the given default value if the given key does not contain the given key.
     *
     * @tparam K the key type
     * @tparam V the value type
     * @param m the map
     * @param k the key to find
     * @param default_value the value to return if the given key is not found
     * @return the value of the given key in the given map, or the given default value
     */
    template<typename K, typename V>
    const V& map_find_or_default(const std::map<K, V>& m, const K& k, const V& default_value) {
        auto it = m.find(k);
        if (it == std::end(m)) {
            return default_value;
        } else {
            return it->second;
        }
    }

    /**
     * Returns a map containing the key / value pairs from both of the given maps. If a key is present in both of the
     * maps, then the value from the second given map is retained.
     *
     * @tparam K the key type
     * @tparam V the value type
     * @tparam C the key comparator type
     * @tparam A the allocator type
     * @param m1 the first map
     * @param m2 the second map
     * @return a map containing the union of the key / value pairs from both maps, with duplicate keys retaining the
     * values from the second map
     */
    template<typename K, typename V, typename C, typename A>
    std::map<K, V, C, A> map_union(const std::map<K, V, C, A>& m1, const std::map<K, V, C, A>& m2) {
        std::map<K, V, C, A> result;
        result.insert(std::begin(m2), std::end(m2)); // insert doesn't overwrite, so we need to insert m2 first
        result.insert(std::begin(m1), std::end(m1));
        return result;
    }

    /**
     * Merges two maps where the values are vectors of a common type. The resulting map will contain the keys from both
     * maps, and common keys contain the values from both maps appended. Specifically, if a key is present in both maps,
     * then the vector from the second map is appended to the vector from the first map, and the resulting vector is
     * the value for the key in the returned map.
     *
     * @tparam K the key type
     * @tparam V the type of the values stored in the vectors
     * @tparam C the key comparator type
     * @param m1 the first map
     * @param m2 the second map
     * @return a map that contains all keys from both maps, with the values appended as described above
     */
    template<typename K, typename V, typename C>
    std::map<K, std::vector<V>, C>
    map_merge(const std::map<K, std::vector<V>, C>& m1, const std::map<K, std::vector<V>, C>& m2) {
        if (m1.empty()) {
            return m2;
        } else if (m2.empty()) {
            return m1;
        }

        auto result = m1;
        for (const auto&[key, from] : m2) {
            std::vector<V>& into = result[key];
            into.insert(std::end(into), std::begin(from), std::end(from));
        }
        return result;
    }

    /**
     * For each vector stored as a value in the given map, applies the given deleter to each element of the vector, and
     * subsequently clears the given map.
     *
     * @tparam K the key type
     * @tparam V the type of the values stored in the vectors
     * @tparam D the deleter type, defaults to deleter
     * @param m the map
     * @param deleter the deleter to apply
     */
    template<typename K, typename V, typename D = deleter<V*>>
    void map_clear_and_delete(std::map<K, std::vector<V*>>& m, const D& deleter = D()) {
        for (auto& [key, value] : m) {
            kdl::col_delete_all(value, deleter);
        }
        m.clear();
    }
}

