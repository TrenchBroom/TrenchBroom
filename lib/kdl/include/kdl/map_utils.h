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

#ifndef KDL_MAP_UTILS_H
#define KDL_MAP_UTILS_H

#include "collection_utils.h"

#include <map>
#include <vector>

namespace kdl {
    template<typename K, typename V>
    std::vector<K> keys(const std::map<K, V>& m) {
        std::vector<K> result;
        result.reserve(m.size());
        for (const auto& e : m) {
            result.push_back(e.first);
        }
        return result;
    }

    template<typename K, typename V>
    std::vector<V> values(const std::map<K, V>& m) {
        std::vector<V> result;
        result.reserve(m.size());
        for (const auto& e : m) {
            result.push_back(e.second);
        }
        return result;
    }

    template<typename K, typename V, typename C, typename D = std::less<V>>
    int
    lexicographicalCompare(const std::map<K, V, C>& map1, const std::map<K, V, C>& map2, const D& valueCmp = D()) {
        const auto keyCmp = C();
        return kdl::lexicographical_compare(std::begin(map1), std::end(map1), std::begin(map2),
            std::end(map2),
            [&keyCmp, &valueCmp](const auto& lhs, const auto& rhs) {
                const K& lhsKey = lhs.first;
                const K& rhsKey = rhs.first;
                if (keyCmp(lhsKey, rhsKey)) {
                    return -1;
                } else if (keyCmp(rhsKey, lhsKey)) {
                    return 1;
                } else {
                    const V& lhsValue = lhs.second;
                    const V& rhsValue = lhs.second;
                    if (valueCmp(lhsValue, rhsValue)) {
                        return -1;
                    } else if (valueCmp(rhsValue, lhsValue)) {
                        return 1;
                    } else {
                        return 0;
                    }
                }
            });
    }

    template<typename K, typename V, typename C, typename D = std::less<V>>
    bool equivalent(const std::map<K, V, C>& map1, const std::map<K, V, C>& map2, const D& valueCmp) {
        if (map1.size() != map2.size()) {
            return false;
        }

        return lexicographical_compare(map1, map2, valueCmp) == 0;
    }

    template<typename K, typename V, typename L>
    const V& findOrDefault(const std::map<K, V>& m, const L& k, const V& defaultValue) {
        auto it = m.find(k);
        if (it == std::end(m)) {
            return defaultValue;
        } else {
            return it->second;
        }
    }

    template<typename K, typename V, typename C, typename A>
    std::map<K, V, C, A> mapUnion(const std::map<K, V, C, A>& m1, const std::map<K, V, C, A>& m2) {
        std::map<K, V, C, A> result;
        result.insert(std::begin(m1), std::end(m1));
        result.insert(std::begin(m2), std::end(m2));
        return result;
    }

    template<typename K, typename V, typename C>
    std::map<K, std::vector<V>, C>
    mergeVectorMaps(const std::map<K, std::vector<V>, C>& m1, const std::map<K, std::vector<V>, C>& m2) {
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

    template<typename K, typename V>
    void clearAndDelete(std::map<K, std::vector<V*>>& m) {
        for (auto& e : m) {
            kdl::delete_all(e.second);
        }
        m.clear();
    }
}

#endif //KDL_MAP_UTILS_H
