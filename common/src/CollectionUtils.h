/*
 Copyright (C) 2010-2017 Kristian Duske

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

#ifndef TrenchBroom_CollectionUtils_h
#define TrenchBroom_CollectionUtils_h

#include "Ensure.h"
#include "VectorUtilsMinimal.h"
#include "Base/VecUtils.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <ostream>
#include <vector>

// TODO: Clean up, split up, and reduce the number of system headers.

template <typename T, size_t S>
std::ostream& operator<<(std::ostream& str, const std::array<T,S>& a) {
    str << "[";
    for (size_t i = 0; i < a.size(); ++i) {
        str << a[i];
        if (i < a.size() - 1) {
            str << ", ";
        }
    }
    str << "]";
    return str;
}

namespace Utils {
    template <typename T>
    struct Deleter {
    public:
        void operator()(const T* ptr) const {
            delete ptr;
        }
    };

    template <typename T, typename Less>
    struct EqualsUsingLess {
    private:
        const Less& m_less;
    public:
        EqualsUsingLess(const Less& less) :
        m_less(less) {}

        bool operator()(const T& lhs, const T& rhs) const {
            return !m_less(lhs, rhs) && !m_less(rhs, lhs);
        }
    };
}

namespace CollectionUtils {
    // only used in CollectionUtils
    template <typename I, typename C>
    I removeAll(const I vecBegin, const I vecEnd, C curItem, C endItem) {
        I last = vecEnd;
        while (curItem != endItem)
            last = std::remove(vecBegin, last, *curItem++);
        return last;
    }
}

namespace VectorUtils {
    // remove in favor of vector_set
    template <typename T, typename Cmp = std::less<T>>
    void setCreate(std::vector<T>& vec, const Cmp cmp = Cmp()) {
        std::sort(std::begin(vec), std::end(vec), cmp);
        auto end = std::unique(std::begin(vec), std::end(vec));
        vec.erase(end, std::end(vec));
    }

    // remove in favor of vector_set
    template <typename T, typename Cmp = std::less<T>>
    std::vector<T> setCreate(const std::vector<T>& vec, const Cmp cmp = Cmp()) {
        auto result = vec;
        setCreate(result, cmp);
        return result;
    }

    // remove in favor of vector_set
    template <typename T1, typename T2, typename Cmp = std::less<T1>>
    bool setInsert(std::vector<T1>& vec, T2&& object, const Cmp& cmp = Cmp()) {
        auto it = std::lower_bound(std::begin(vec), std::end(vec), object, cmp);
        if (it == std::end(vec)) {
            vec.push_back(std::forward<T2>(object));
            return true;
        } else if (cmp(*it, object) || cmp(object, *it)) {
            vec.insert(it, std::forward<T2>(object));
            return true;
        } else {
            *it = std::forward<T2>(object);
            return false;
        }
    }

    // remove in favor of vector_set
    template <typename T1, typename T2, typename Cmp = std::less<T1>>
    bool setRemove(std::vector<T1>& vec, const T2& object, const Cmp cmp = Cmp()) {
        typename std::vector<T1>::iterator it = std::lower_bound(std::begin(vec), std::end(vec), object, cmp);
        if (it != std::end(vec) && !cmp(*it, object) && !cmp(object, *it)) {
            vec.erase(it);
            return true;
        }
        return false;
    }

    // remove in favor of vector_set
    template <typename T, typename Cmp = std::less<T>>
    bool setContains(const std::vector<T>& vec, const T& object, const Cmp cmp = Cmp()) {
        return std::binary_search(std::begin(vec), std::end(vec), object, cmp);
    }

    // remove in favor of vector_set
    template <typename T, typename Cmp = std::less<T>>
    std::vector<T> setUnion(const std::vector<T>& vec1, const std::vector<T>& vec2, const Cmp cmp = Cmp()) {
        std::vector<T> result;
        std::set_union(std::begin(vec1), std::end(vec1),
                       std::begin(vec2), std::end(vec2),
                       std::back_inserter(result), cmp);
        return result;
    }

    // generalize this for ranges, use output iterator to store result of applying lambda -- but then it's just
    // std::transform...
    /**
     * Returns a std::vector which contains `lambda` applied to each element in `vec`.
     *
     * @tparam T input element type
     * @tparam L type of lambda to apply
     * @param vec input vector
     * @param lambda lambda to apply
     * @return the transformed vector
     */
    template <typename T, typename L>
    auto map(const std::vector<T>& vec, L&& lambda) {
        using MappedElemType = decltype(lambda(std::declval<T>()));

        std::vector<MappedElemType> result;
        result.reserve(vec.size());
        for (const auto& elem : vec) {
            result.push_back(lambda(elem));
        }

        return result;
    }
}

namespace SetUtils {
    // how is this useful? remove and construct the set in place
    template <typename T, size_t S>
    void makeSet(const std::array<T, S>& arr, std::set<T>& result) {
        result.insert(std::begin(arr), std::end(arr));
    }

    // how is this useful? remove and construct the set in place
    template <typename T, size_t S>
    std::set<T> makeSet(const std::array<T, S>& arr) {
        std::set<T> result;
        makeSet(arr, result);
        return result;
    }

    // how is this useful? remove and construct the set in place
    template <typename T>
    void makeSet(const std::vector<T>& vec, std::set<T>& result) {
        result.insert(std::begin(vec), std::end(vec));
    }

    // how is this useful? remove and construct the set in place
    template <typename T>
    std::set<T> makeSet(const std::vector<T>& vec) {
        std::set<T> result;
        makeSet(vec, result);
        return result;
    }

    // how is this useful? remove and construct the set in place
    template <typename T, typename C>
    void makeSet(const std::vector<T>& vec, std::set<T,C>& result) {
        result.insert(std::begin(vec), std::end(vec));
    }

    // rename to difference
	template <typename T, typename C>
    void minus(const std::set<T, C>& lhs, const std::set<T, C>& rhs, std::set<T, C>& result) {
        std::set_difference(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), std::insert_iterator<std::set<T, C> >(result, std::end(result)));
    }

    // rename to difference
    template <typename T, typename C>
    std::set<T, C> minus(const std::set<T, C>& lhs, const std::set<T, C>& rhs) {
        std::set<T, C> result(lhs.key_comp());
        minus(lhs, rhs, result);
        return result;
    }

    // rename to difference
    template <typename T, typename C>
    std::set<T, C> minus(const std::set<T, C>& lhs, const T& rhs) {
        std::set<T, C> result(lhs);
        result.erase(rhs);
        return result;
    }

    // rename to union
    template <typename T, typename C>
    void merge(std::set<T, C>& lhs, const std::set<T, C>& rhs) {
        lhs.insert(std::begin(rhs), std::end(rhs));
    }

    template <typename T, typename C>
    void merge(const std::set<T, C>& lhs, const std::set<T, C>& rhs, std::set<T, C>& result) {
        result.insert(std::begin(lhs), std::end(lhs));
        result.insert(std::begin(rhs), std::end(rhs));
    }

    template <typename T, typename C>
    std::set<T, C> merge(const std::set<T, C>& lhs, const std::set<T, C>& rhs) {
        std::set<T, C> result(lhs.key_comp());
        merge(lhs, rhs, result);
        return result;
    }

    template <typename T, typename C>
    void intersection(const std::set<T, C>& lhs, const std::set<T, C>& rhs, std::set<T, C>& result) {
        const C cmp = result.key_comp();
        std::set_intersection(std::begin(lhs), std::end(lhs),
                              std::begin(rhs), std::end(rhs),
                              std::inserter(result, std::end(result)),
                              cmp);
    }

    template <typename T, typename C>
    void intersection(const std::set<T, C>& lhs, const std::set<T, C>& rhs, std::vector<T>& result) {
        std::set_intersection(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), std::back_inserter(result));
    }

    template <typename T, typename C>
    std::set<T, C> intersection(const std::set<T, C>& lhs, const std::set<T, C>& rhs) {
        std::set<T, C> result(lhs.key_comp());
        intersection(lhs, rhs, result);
        return result;
    }

    // unused
    template <typename S>
    typename S::value_type popFront(S& set) {
        assert(!set.empty());
        const auto it = std::begin(set);
        const typename S::value_type value = *it;
        set.erase(it);
        return value;
    }

    // unused
    template <typename S>
    S findMaximalElements(const S& set) {
        using V = typename S::value_type;
        using C = typename S::value_compare;
        const C& cmp = set.value_comp();

        S result;
        for (auto it = std::begin(set), end = std::end(set); it != end; ++it) {
            const V& cand = *it;
            if (!std::any_of(std::next(it), std::end(set), [&cand, &cmp](const V& cur) { return cmp(cand, cur); }))
                result.insert(cand);
        }
        return result;
    }
}

namespace MapUtils {
    template <typename K, typename V, typename C>
    std::set<K, C> keySet(const std::map<K, V, C>& map) {
        std::set<K, C> result;
        for (const auto& entry : map)
            result.insert(entry.first);
        return result;
    }

    template <typename K, typename V, typename C_K, typename C_V>
    std::set<V, C_V> valueSet(const std::map<K, V, C_K>& map) {
        std::set<V, C_V> result;
        for (const auto& entry : map)
            result.insert(entry.second);
        return result;
    }

    template <typename K, typename V, typename C>
    std::set<V, std::less<V> > valueSet(const std::map<K, V, C>& map) {
        return valueSet<K, V, C, std::less<V> >(map);
    }

    template <typename K, typename V, typename C>
    std::vector<K> keyList(const std::map<K,V,C>& map) {
        std::vector<K> result;
        result.reserve(map.size());

        for (const auto& entry : map)
            result.push_back(entry.first);
        return result;
    }

    template <typename K, typename V, typename C>
    std::vector<V> valueList(const std::map<K,V,C>& map) {
        std::vector<V> result;
        result.reserve(map.size());
        for (const auto& entry : map)
            result.push_back(entry.second);
        return result;
    }

    template <typename K, typename V, typename C, typename D>
    int compare(const std::map<K, V, C>& map1, const std::map<K, V, C>& map2, const D& valueCmp) {
        auto it1 = std::begin(map1);
        auto end1 = std::end(map1);
        auto it2 = std::begin(map2);
        auto end2 = std::end(map2);

        while (it1 != end1 && it2 != end2) {
            const K& key1 = it1->first;
            const K& key2 = it2->first;
            if (map1.key_comp()(key1, key2))
                return -1;
            if (map1.key_comp()(key2, key1))
                return 1;

            const V& value1 = it1->second;
            const V& value2 = it2->second;
            if (valueCmp(value1, value2))
                return -1;
            if (valueCmp(value2, value1))
                return 1;

            ++it1; ++it2;
        }

        if (it1 != end1)
            return 1;
        if (it2 != end2)
            return -1;
        return 0;
    }

    template <typename K, typename V, typename C>
    int compare(const std::map<K, V, C>& map1, const std::map<K, V, C>& map2) {
        return compare(map1, map2, std::less<V>());
    }

    template <typename K, typename V, typename C, typename D>
    bool equals(const std::map<K, V, C>& map1, const std::map<K, V, C>& map2, const D& valueCmp) {
        if (map1.size() != map2.size())
            return false;

        for (const auto& entry : map1) {
            const K& key = entry.first;
            const V& value1 = entry.second;

            const auto it2 = map2.find(key);
            if (it2 == std::end(map2))
                return false;

            const V& value2 = it2->second;
            if (valueCmp(value1, value2) || valueCmp(value2, value1))
                return false;
        }

        return true;
    }

    template <typename K, typename V, typename C, typename L>
    const V& find(const std::map<K, V, C>& map, const L& key, const V& defaultValue) {
        using Map = std::map<K, V, C>;
        typename Map::const_iterator it = map.find(key);
        if (it == std::end(map))
            return defaultValue;
        return it->second;
    }

    template <typename K, typename V, typename C>
    std::pair<bool, typename std::map<K, V, C>::iterator> findInsertPos(std::map<K, V, C>& map, const K& key) {
        if (map.empty())
            return std::make_pair(false, std::end(map));

        const auto compare = map.key_comp();

         // Note that C++11 expects upper bound instead of lower bound as the insertion hint!
        const auto ub = map.upper_bound(key);
        if (ub == std::begin(map) || compare(std::prev(ub)->first, key)) {
            // key was not found in the map, otherwise it would precede the insert pos
            assert(map.count(key) == 0);
            return std::make_pair(false, ub);
        }

        assert(map.count(key) == 1);
        return std::make_pair(true, ub);
    }

    template <typename K, typename V, typename C, typename W = V>
    typename std::map<K, V, C>::iterator findOrInsert(std::map<K, V, C>& map, const K& key, const W& value = W()) {
        const auto insertPos = findInsertPos(map, key);
        if (!insertPos.first)
            return map.insert(insertPos.second, std::make_pair(key, V(value)));

        // As of C++11, the insert pos points to the upper bound of the key, so we need to rewind
        assert(insertPos.second != std::begin(map));
        return std::prev(insertPos.second);
    }

    template <typename K, typename V, typename C>
    bool insertOrFail(std::map<K, V, C>& map, const K& key, const V& value) {
        const auto insertPos = findInsertPos(map, key);
        if (!insertPos.first) {
            assert(map.count(key) == 0);
            map.insert(insertPos.second, std::make_pair(key, V(value)));
            assert(map.count(key) == 1);
            return true;
        }

        return false;
    }

    template <typename K, typename V, typename C, typename Val>
    bool insertOrReplace(std::map<K, V, C>& map, const K& key, Val&& value) {
        const auto insertPos = findInsertPos(map, key);
        if (!insertPos.first) {
            assert(map.count(key) == 0);
            map.insert(insertPos.second, std::make_pair(key, std::forward<Val>(value)));
            assert(map.count(key) == 1);
            return true;
        } else {
            assert(insertPos.second != std::begin(map));
            // As of C++11, the insert pos points to the upper bound of the key, so we need to rewind
            std::prev(insertPos.second)->second = std::forward<Val>(value);
            return false;
        }
    }

    template <typename K, typename V, typename C>
    bool insertOrReplaceAndDelete(std::map<K, V*, C>& map, const K& key, V* value) {
        const auto insertPos = findInsertPos(map, key);
        if (!insertPos.first) {
            assert(map.count(key) == 0);
            map.insert(insertPos.second, std::make_pair(key, value));
            assert(map.count(key) == 1);
            return true;
        } else {
            assert(insertPos.second != std::begin(map));
            // As of C++11, the insert pos points to the upper bound of the key, so we need to rewind
            const auto prev = std::prev(insertPos.second);
            delete prev->second;
            prev->second = value;
            return false;
        }
    }

    template <typename K, typename V, typename C>
    void merge(std::map<K, std::vector<V>, C>& map1, const std::map<K, std::vector<V>, C>& map2) {
        using Vector = std::vector<V>;

        for (const auto& entry : map2) {
            const K& key = entry.first;
            const Vector& vector = entry.second;

            Vector& into = map1[key];
            VecUtils::append(into, vector);
        }
    }

    template <typename K, typename V, typename C>
    void concatenate(const std::map<K,V,C>& map1, const std::map<K,V,C>& map2, std::map<K,V,C>& result) {
        result.clear();
        result.insert(std::begin(map2), std::end(map2));
        result.insert(std::begin(map1), std::end(map1));
    }

    template <typename K, typename V, typename C>
    std::map<K,V,C> concatenate(const std::map<K,V,C>& map1, const std::map<K,V,C>& map2) {
        std::map<K,V,C> result;
        concatenate(map1, map2, result);
        return result;
    }

    template <typename K, typename V, typename C>
    void clearAndDelete(std::map<K, V*, C>& map) {
        Utils::Deleter<V> deleter;
        for (auto [key, value] : map) {
            deleter(value);
        }
        map.clear();
    }

    template <typename K, typename V, typename C>
    void clearAndDelete(std::map<K, std::vector<V*>, C>& map) {
        for (auto [key, value] : map) {
            VectorUtils::clearAndDelete(value);
        }
        map.clear();
    }
}

#endif
