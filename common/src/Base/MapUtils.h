/*
 Copyright (C) 2010-2019 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRENCHBROOM_MAPUTILS_H
#define TRENCHBROOM_MAPUTILS_H

#include "ColUtils.h"

#include <map>
#include <vector>

namespace MapUtils {
    template <typename K, typename V>
    std::vector<K> keys(const std::map<K, V>& m) {
        std::vector<K> result;
        result.reserve(m.size());
        for (const auto& e : m) {
            result.push_back(e.first);
        }
        return result;
    }

    template <typename K, typename V>
    std::vector<V> values(const std::map<K, V>& m) {
        std::vector<V> result;
        result.reserve(m.size());
        for (const auto& e : m) {
            result.push_back(e.second);
        }
        return result;
    }

    template <typename K, typename V, typename C, typename D = std::less<V>>
    int lexicographicalCompare(const std::map<K, V, C>& map1, const std::map<K, V, C>& map2, const D& valueCmp = D()) {
        const auto keyCmp = C();
        return ColUtils::lexicographicalCompare(std::begin(map1), std::end(map1), std::begin(map2), std::end(map2),
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

    template <typename K, typename V, typename C, typename D = std::less<V>>
    bool equivalent(const std::map<K, V, C>& map1, const std::map<K, V, C>& map2, const D& valueCmp) {
        if (map1.size() != map2.size()) {
            return false;
        }

        return lexicographicalCompare(map1, map2, valueCmp) == 0;
    }

    template <typename K, typename V, typename L>
    const V& findOrDefault(const std::map<K, V>& m, const L& k, const V& defaultValue) {
        auto it = m.find(k);
        if (it == std::end(m)) {
            return defaultValue;
        } else {
            return it->second;
        }
    }

    template <typename K, typename V, typename C, typename A>
    std::map<K, V, C, A> mapUnion(const std::map<K, V, C, A>& m1, const std::map<K, V, C, A>& m2) {
        std::map<K, V, C, A> result;
        result.insert(std::begin(m1), std::end(m1));
        result.insert(std::begin(m2), std::end(m2));
        return result;
    }

    template <typename K, typename V, typename C>
    std::map<K, std::vector<V>, C> mergeVectorMaps(const std::map<K, std::vector<V>, C>& m1, const std::map<K, std::vector<V>, C>& m2) {
        if (m1.empty()) {
            return m2;
        } else if (m2.empty()) {
            return m1;
        }

        auto result = m1;
        for (const auto& [key, from] : m2) {
            std::vector<V>& into = result[key];
            into.insert(std::end(into), std::begin(from), std::end(from));
        }
        return result;
    }

    template <typename K, typename V>
    void clearAndDelete(std::map<K, std::vector<V*>>& m) {
        for (auto& e : m) {
            ColUtils::deleteAll(e.second);
        }
        m.clear();
    }
}

#endif //TRENCHBROOM_MAPUTILS_H
