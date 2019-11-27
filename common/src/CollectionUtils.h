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

    template <typename T, typename Cmp = std::equal_to<T> >
    struct PtrCmp {
    private:
        Cmp m_cmp;
    public:
        bool operator()(const T* lhs, const T* rhs) const {
            return m_cmp(*lhs, *rhs);
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
    template <typename I, typename C>
    I removeAll(const I vecBegin, const I vecEnd, C curItem, C endItem) {
        I last = vecEnd;
        while (curItem != endItem)
            last = std::remove(vecBegin, last, *curItem++);
        return last;
    }

    template <typename I>
    void deleteAll(I cur, const I end) {
        while (cur != end) {
            delete *cur;
            ++cur;
        }
    }

    template <typename C, typename P>
    void eraseIf(C& col, const P& pred) {
        auto cur = std::begin(col);
        while (cur != std::end(col)) {
            if (pred(*cur))
                cur = col.erase(cur);
            else
                ++cur;
        }
    }

    template <typename Col, typename Cmp>
    Col& retainMaximalElements(Col& col, const Cmp& cmp = Cmp()) {
        auto it = std::begin(col);
        while (it != std::end(col)) {
            const auto& cand = *it;
            bool erased = false;

            auto ne = std::next(it);
            while (!erased && ne != std::end(col)) {
                const auto& cur = *ne;

                if (cmp(cand, cur)) {
                    it = col.erase(it);
                    erased = true;
                } else if (cmp(cur, cand)) {
                    ne = col.erase(ne);
                } else {
                    ++ne;
                }
            }

            if (!erased)
                ++it;
        }

        return col;
    }

    template <typename Col, typename Cmp>
    Col findMaximalElements(const Col& col, const Cmp& cmp = Cmp()) {
        Col result(col);
        return retainMaximalElements(result, cmp);
    }

    template <typename I, typename Fac, typename Cmp>
    void equivalenceClasses(I rangeStart, I rangeEnd, const Fac& fac, const Cmp& cmp) {
        using E = typename I::value_type;

        std::list<I> workList;
        while (rangeStart != rangeEnd)
            workList.push_back(rangeStart++);

        while (!workList.empty()) {
            const E& root = *workList.front(); workList.pop_front();
            auto cls = fac();
            cls = root;

            auto cur = std::begin(workList);
            while (cur != std::end(workList)) {
                const E& cand = **cur;
                if (cmp(root, cand)) {
                    cls = cand;
                    cur = workList.erase(cur);
                } else {
                    ++cur;
                }
            }
        }
    }

    template <typename I, typename Cmp>
    std::list<std::list<typename I::value_type>> equivalenceClasses(I rangeStart, I rangeEnd, const Cmp& cmp) {
        using E = typename I::value_type;
        using Class = std::list<E>;
        using Result = std::list<Class>;

        Result result;
        equivalenceClasses(rangeStart, rangeEnd, [&result]() {
            result.emplace_back();
            return std::back_inserter(result.back());
        }, cmp);

        return result;
    }

    template <typename C, typename Cmp>
    std::list<std::list<typename C::value_type>> equivalenceClasses(C collection, const Cmp& cmp) {
        return equivalenceClasses(std::begin(collection), std::end(collection), cmp);
    }
}

namespace ListUtils {
    template <typename T>
    void append(std::list<T>& vec, const std::list<T>& items) {
        vec.insert(std::end(vec), std::begin(items), std::end(items));
    }

    template <typename T>
    void eraseAll(std::list<T>& vec, const std::list<T>& items) {
        vec.erase(removeAll(std::begin(vec), std::end(vec), std::begin(items), std::end(items)), std::end(vec));
    }

    template <typename T>
    void remove(std::vector<T>& list, const T item) {
        list.erase(std::remove(std::begin(list), std::end(list), item), std::end(list));
    }

    template <typename T>
    void removeAndDelete(std::vector<T*>& list, const T* item) {
        remove(list, item);
        delete item;
    }

    /**
     Removes the element at `pos` in `list`, and replaces it with the contents of list `other`.
     The list `other` is cleared as a side effect.

     Does nothing if `other` is empty.
     Returns an iterator to the start of the newly inserted elements.
     */
    template <typename T>
    typename std::list<T>::iterator replace(std::list<T>& list, typename std::list<T>::iterator pos, std::list<T>& other) {
        using DiffType = typename std::list<T>::iterator::difference_type;
        const DiffType count = DiffType(other.size());
        if (count == 0)
            return pos;
        pos = list.erase(pos);
        list.splice(pos, other);
        return std::prev(pos, count);
    }

    template <typename T>
    void clearAndDelete(std::list<T*>& list) {
        std::for_each(std::begin(list), std::end(list), Utils::Deleter<T>());
        list.clear();
    }
}

namespace VectorUtils {
    template <typename O, typename I>
    std::vector<O> create(const I& item) {
        return std::vector<O>(1, item);
    }

    template <typename O, typename I1, typename I2>
    std::vector<O> create(const I1& item1, const I2& item2) {
        std::vector<O> result;
        result.reserve(2);
        result.push_back(item1);
        result.push_back(item2);
        return result;
    }

    template <typename O, typename I1, typename I2, typename I3>
    std::vector<O> create(const I1& item1, const I2& item2, const I3& item3) {
        std::vector<O> result;
        result.reserve(3);
        result.push_back(item1);
        result.push_back(item2);
        result.push_back(item3);
        return result;
    }

    template <typename O, typename I1, typename I2, typename I3, typename I4>
    std::vector<O> create(const I1& item1, const I2& item2, const I3& item3, const I4& item4) {
        std::vector<O> result;
        result.reserve(4);
        result.push_back(item1);
        result.push_back(item2);
        result.push_back(item3);
        result.push_back(item4);
        return result;
    }

    template <typename O, typename I1, typename I2, typename I3, typename I4, typename I5>
    std::vector<O> create(const I1& item1, const I2& item2, const I3& item3, const I4& item4, const I5& item5) {
        std::vector<O> result;
        result.reserve(5);
        result.push_back(item1);
        result.push_back(item2);
        result.push_back(item3);
        result.push_back(item4);
        result.push_back(item5);
        return result;
    }

    template <typename T, typename C>
    int compare(const std::vector<T>& lhs, const std::vector<T>& rhs, const C& cmp) {
        using Vec = std::vector<T>;
        typename Vec::const_iterator lIt = std::begin(lhs);
        typename Vec::const_iterator lEnd = std::end(lhs);
        typename Vec::const_iterator rIt = std::begin(rhs);
        typename Vec::const_iterator rEnd = std::end(rhs);

        while (lIt < lEnd && rIt < rEnd) {
            const T& lElem = *lIt;
            const T& rElem = *rIt;

            if (cmp(lElem, rElem))
                return -1;
            if (cmp(rElem, lElem))
                return 1;
            ++lIt; ++rIt;
        }

        if (lIt < lEnd)
            return 1;
        if (rIt < rEnd)
            return -1;
        return 0;
    }

    template <typename T>
    int compare(const std::vector<T>& lhs, const std::vector<T>& rhs) {
        return compare(lhs, rhs, std::less<T>());
    }

    template <typename T, typename C>
    bool equals(const std::vector<T>& lhs, const std::vector<T>& rhs, const C& cmp) {
        if (lhs.size() != rhs.size())
            return false;
        return compare(lhs, rhs, cmp) == 0;
    }

    template <typename T>
    bool equals(const std::vector<T>& lhs, const std::vector<T>& rhs) {
        return equals(lhs, rhs, std::less<T>());
    }

    template <typename T>
    typename std::vector<T>::const_iterator find(const std::vector<T>& vec, const T& item) {
        return std::find(std::begin(vec), std::end(vec), item);
    }

    template <typename T>
    typename std::vector<T>::iterator find(std::vector<T>& vec, const T& item) {
        return std::find(std::begin(vec), std::end(vec), item);
    }

    template <typename T>
    typename std::vector<T*>::const_iterator find(const std::vector<T*>& vec, const T* item) {
        return std::find(std::begin(vec), std::end(vec), item);
    }

    template <typename T>
    typename std::vector<T*>::iterator find(std::vector<T*>& vec, const T* item) {
        return std::find(std::begin(vec), std::end(vec), item);
    }

    template <typename T>
    typename std::vector<T*>::const_iterator findOther(const std::vector<T*>& vec, const T* item) {
        using Iter = typename std::vector<T*>::const_iterator;
        Iter cur = std::begin(vec);
        Iter end = std::end(vec);
        while (cur != end) {
            if (*cur != item)
                return cur;
            ++cur;
        }
        return end;
    }

    template <typename T, class P>
    T* findIf(const std::vector<T*>& vec, const P& predicate) {
        typename std::vector<T*>::const_iterator it = std::find_if(std::begin(vec), std::end(vec), predicate);
        if (it == std::end(vec))
            return nullptr;
        return *it;
    }

    template <typename T, class P>
    const std::shared_ptr<T> findIf(const std::vector<std::shared_ptr<T> >& vec, const P& predicate) {
        typename std::vector<std::shared_ptr<T> >::const_iterator it = std::find_if(std::begin(vec), std::end(vec), predicate);
        if (it == std::end(vec))
            return std::shared_ptr<T>();
        return *it;
    }

    template <typename T, class P>
    const T* findIf(const std::vector<T>& vec, const P& predicate) {
        typename std::vector<T>::const_iterator it = std::find_if(std::begin(vec), std::end(vec), predicate);
        if (it == std::end(vec))
            return nullptr;
        return &(*it);
    }

    template <typename T, typename Cmp>
    bool contains(const std::vector<T>& vec, const T& item, const Cmp& cmp) {
        auto first = std::begin(vec);
        const auto last = std::end(vec);
        while (first != last)
            if (cmp(*(first++), item))
                return true;
        return false;
    }

    template <typename T1, typename T2>
    bool contains(const std::vector<T1>& vec, const T2& item) {
        return std::find(std::begin(vec), std::end(vec), item) != std::end(vec);
    }

    template <typename T>
    bool containsPtr(const std::vector<T*>& vec, const T* item) {
        // this const_cast is okay because we won't modify *item
        return contains(vec, const_cast<T*>(item), Utils::PtrCmp<T, std::equal_to<T> >());
    }

    template <typename T>
    size_t indexOf(const std::vector<T>& vec, const T& item) {
        for (size_t i = 0; i < vec.size(); ++i)
            if (vec[i] == item)
                return i;
        return vec.size();
    }

    template <typename T, typename O>
    void shiftLeft(std::vector<T>& vec, const O offset) {
        if (vec.empty() || offset == 0)
            return;

        // (offset > 0) is used to silence a compiler warning
        using DiffType = typename std::vector<T>::iterator::difference_type;
        const DiffType modOffset = static_cast<DiffType>(offset) % static_cast<DiffType>(vec.size());
        if (modOffset == 0)
            return;

        std::rotate(std::begin(vec), std::begin(vec) + modOffset, std::end(vec));
    }

    template <typename T>
    void shiftRight(std::vector<T>& vec, const size_t offset) {
        if (vec.empty() || offset == 0)
            return;

        using DiffType = typename std::vector<T>::iterator::difference_type;
        const DiffType modOffset = static_cast<DiffType>(offset) % static_cast<DiffType>(vec.size());
        const DiffType size = static_cast<DiffType>(vec.size());
		shiftLeft(vec, size - modOffset);
    }

    template <typename T>
    void swapPred([[maybe_unused]] std::vector<T>& vec, typename std::vector<T>::iterator i) {
        assert(i > std::begin(vec) && i < std::end(vec));
        std::iter_swap(i, std::prev(i));
    }

    template <typename T>
    void swapPred(std::vector<T>& vec, const size_t i) {
        typename std::vector<T>::iterator it = std::begin(vec);
        using DiffType = typename std::vector<T>::iterator::difference_type;
        std::advance(it, static_cast<DiffType>(i));
        swapPred(vec, it);
    }

    template <typename T>
    void swapSucc([[maybe_unused]] std::vector<T>& vec, typename std::vector<T>::iterator i) {
        assert(i >= std::begin(vec) && i < std::prev(std::end(vec)));
        std::iter_swap(i, std::next(i));
    }

    template <typename T>
    void swapSucc(std::vector<T>& vec, const size_t i) {
        typename std::vector<T>::iterator it = std::begin(vec);
        using DiffType = typename std::vector<T>::iterator::difference_type;
        std::advance(it, static_cast<DiffType>(i));
        swapSucc(vec, it);
    }

    template <typename T>
    void deleteAll(const std::vector<T*>& vec) {
        std::for_each(std::begin(vec), std::end(vec), Utils::Deleter<T>());
    }

    template <typename T>
    void erase(std::vector<T>& vec, const size_t index) {
        ensure(index < vec.size(), "index out of range");
        typename std::vector<T>::iterator it = std::begin(vec);
        using DiffType = typename std::vector<T>::iterator::difference_type;
        std::advance(it, static_cast<DiffType>(index));
        vec.erase(it);
    }


    template <typename T1, typename T2>
    bool erase(std::vector<T1>& vec, const T2& item) {
        typename std::vector<T1>::iterator it = std::remove(std::begin(vec), std::end(vec), item);
        if (it == std::end(vec))
            return false;
        vec.erase(it, std::end(vec));
        return true;
    }

    template <typename T1, typename T2>
    bool erase(std::vector<T1*>& vec, const T2* item) {
        typename std::vector<T1*>::iterator it = std::remove(std::begin(vec), std::end(vec), item);
        if (it == std::end(vec))
            return false;
        vec.erase(it, std::end(vec));
        return true;
    }

    template <typename T, class P>
    void eraseIf(std::vector<T>& vec, const P& pred) {
        vec.erase(std::remove_if(std::begin(vec), std::end(vec), pred), std::end(vec));
    }

    template <typename T>
    void eraseAll(std::vector<T>& vec, const std::vector<T>& items) {
        vec.erase(CollectionUtils::removeAll(std::begin(vec), std::end(vec), std::begin(items), std::end(items)), std::end(vec));
    }

    template <typename T, typename I>
    void eraseAll(std::vector<T>& vec, I cur, I end) {
        vec.erase(CollectionUtils::removeAll(std::begin(vec), std::end(vec), cur, end), std::end(vec));
    }

    template <typename T>
    std::vector<T> eraseAll(const std::vector<T>& vec, const std::vector<T>& items) {
        std::vector<T> result(vec);
        result.erase(CollectionUtils::removeAll(std::begin(result), std::end(result), std::begin(items), std::end(items)), std::end(result));
        return result;
    }

    template <typename T>
    bool eraseAndDelete(std::vector<T*>& vec, const T* item) {
        if (!erase(vec, item))
            return false;
        delete item;
        return true;
    }

    template <typename T>
    void eraseAndDelete(std::vector<T*>& vec, typename std::vector<T*>::iterator first, typename std::vector<T*>::iterator last) {
        std::for_each(first, last, Utils::Deleter<T>());
        vec.erase(first, last);
    }

    template <typename T>
    void eraseAndDelete(std::vector<T*>& vec, typename std::vector<T*>::iterator first) {
        eraseAndDelete(vec, first, std::end(vec));
    }

    template <typename T1, typename T2, typename R>
    void concatenate(const std::vector<T1>& vec1, const std::vector<T2>& vec2, std::vector<R>& result) {
        result.clear();
        result.reserve(vec1.size() + vec2.size());
        result.insert(std::end(result), std::begin(vec1), std::end(vec1));
        result.insert(std::end(result), std::begin(vec2), std::end(vec2));
    }

    template <typename T>
    std::vector<T> concatenate(const std::vector<T>& vec1, const std::vector<T>& vec2) {
        std::vector<T> result;
        concatenate(vec1, vec2, result);
        return result;
    }

    template <typename T1, typename T2>
    void append(std::vector<T1>& vec1, const T2* values, const size_t count) {
        vec1.reserve(vec1.size() + count);
        for (size_t i = 0; i < count; ++i) {
            vec1.push_back(*(values + i));
        }
    }

    template <typename T, typename K, typename C>
    void append(std::vector<T>& vec, const std::map<K,T,C>& map) {
        vec.reserve(vec.size() + map.size());
        for (const auto& entry : map) {
            vec.push_back(entry.second);
        }
    }

    template <typename T, class Cmp = std::less<T>>
    void sort(std::vector<T>& vec, const Cmp cmp = Cmp()) {
        std::sort(std::begin(vec), std::end(vec), cmp);
    }

    template <typename T, class Cmp = std::less<T>>
    void sortAndRemoveDuplicates(std::vector<T>& vec, const Cmp cmp = Cmp()) {
        std::sort(std::begin(vec), std::end(vec), cmp);
        auto it = std::unique(std::begin(vec), std::end(vec), Utils::EqualsUsingLess<T, Cmp>(cmp));
        vec.erase(it, std::end(vec));
    }

    template <typename T>
    std::vector<T> difference(const std::vector<T>& vec1, const std::vector<T>& vec2) {
        std::vector<T> result;
        for (const T& elem : vec1) {
            if (!VectorUtils::contains(vec2, elem))
                result.push_back(elem);
        }
        return result;
    }

    template <typename O, typename I>
    std::vector<O> cast(const std::vector<I>& input) {
        std::vector<O> output;
        output.reserve(input.size());
        for (const I& elem : input)
            output.push_back(O(elem));
        return output;
    }

    template <typename T, typename Cmp = std::less<T>>
    void orderedDifference(std::vector<T>& minuend, const std::vector<T>& subtrahend, const Cmp cmp = Cmp()) {
        auto mIt = std::begin(minuend);
        auto sIt = std::begin(subtrahend);
        auto sEnd = std::end(subtrahend);

        while (mIt != std::end(minuend) && sIt != sEnd) {
            const T& m = *mIt;
            const T& s = *sIt;
            if (cmp(m, s)) { // m < s
                ++mIt;
            } else if (cmp(s, m)) { // s < m
                ++sIt;
            } else { // s == m
                mIt = minuend.erase(mIt);
            }
        }
    }

    template <typename T, typename Cmp = std::less<T>>
    bool setIsSet(const std::vector<T>& set, const Cmp cmp = Cmp()) {
        if (set.size() < 2) {
            return true;
        }

        auto cur = std::begin(set);
        auto next = cur + 1;

        while (next != std::end(set)) {
            if (!cmp(*cur, *next)) {
                return false;
            }
            ++cur;
            ++next;
        }
        return true;
    }

    template <typename T, typename Cmp = std::less<T>>
    void setCreate(std::vector<T>& vec, const Cmp cmp = Cmp()) {
        std::sort(std::begin(vec), std::end(vec), cmp);
        auto end = std::unique(std::begin(vec), std::end(vec));
        vec.erase(end, std::end(vec));
    }

    template <typename T, typename Cmp = std::less<T>>
    std::vector<T> setCreate(const std::vector<T>& vec, const Cmp cmp = Cmp()) {
        auto result = vec;
        setCreate(result, cmp);
        return result;
    }

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

    template <typename T, typename I, typename Cmp = std::less<T>>
    void setInsert(std::vector<T>& vec, I cur, const I end, const Cmp& cmp = Cmp()) {
        while (cur != end) {
            auto it = std::lower_bound(std::begin(vec), std::end(vec), *cur, cmp);
            if (it == std::end(vec)) {
                vec.push_back(*cur);
            } else if (cmp(*it, *cur) || cmp(*cur, *it)) {
                vec.insert(it, *cur);
            } else
                *it = *cur;
            ++cur;
        }
    }

    template <typename T1, typename T2, typename Cmp = std::less<T1>>
    bool setRemove(std::vector<T1>& vec, const T2& object, const Cmp cmp = Cmp()) {
        typename std::vector<T1>::iterator it = std::lower_bound(std::begin(vec), std::end(vec), object, cmp);
        if (it != std::end(vec) && !cmp(*it, object) && !cmp(object, *it)) {
            vec.erase(it);
            return true;
        }
        return false;
    }

    template <typename T, typename I, typename Cmp = std::less<T>>
    void setRemove(std::vector<T>& vec, I cur, const I end, const Cmp cmp = Cmp()) {
        while (cur != end) {
            auto it = std::lower_bound(std::begin(vec), std::end(vec), *cur, cmp);
            if (it != std::end(vec) && !cmp(*it, *cur) && !cmp(*cur, *it)) {
                vec.erase(it);
            }
            ++cur;
        }
    }

    template <typename T, typename Cmp = std::less<T>>
    bool setContains(const std::vector<T>& vec, const T& object, const Cmp cmp = Cmp()) {
        return std::binary_search(std::begin(vec), std::end(vec), object, cmp);
    }

    template <typename T, typename Cmp = std::less<T>>
    std::vector<T> setUnion(const std::vector<T>& vec1, const std::vector<T>& vec2, const Cmp cmp = Cmp()) {
        std::vector<T> result;
        std::set_union(std::begin(vec1), std::end(vec1),
                       std::begin(vec2), std::end(vec2),
                       std::back_inserter(result), cmp);
        return result;
    }

    template <typename T, typename Cmp = std::less<T>>
    std::vector<T> setMinus(const std::vector<T>& minuend, const std::vector<T>& subtrahend, const Cmp cmp = Cmp()) {
        std::vector<T> result;
        std::set_difference(std::begin(minuend), std::end(minuend),
                            std::begin(subtrahend), std::end(subtrahend),
                            std::back_inserter(result), cmp);
        return result;
    }

    template <typename T, typename Cmp = std::less<T>>
    std::vector<T> setIntersection(const std::vector<T>& vec1, const std::vector<T>& vec2, const Cmp cmp = Cmp()) {
        std::vector<T> result;
        std::set_intersection(std::begin(vec1), std::end(vec1),
                              std::begin(vec2), std::end(vec2),
                              std::back_inserter(result), cmp);
        return result;
    }

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
    template <typename S>
    bool subset(const S& lhs, const S& rhs) {
        using C = typename S::key_compare;
        const C cmp = lhs.key_comp();
        return std::includes(std::begin(rhs), std::end(rhs),
                             std::begin(lhs), std::end(lhs), cmp);
    }

    struct SubsetCmp {
        template <typename S>
        bool operator()(const S& lhs, const S& rhs) const {
            return subset(lhs, rhs);
        }
    };

    template <typename T, size_t S>
    void makeSet(const std::array<T, S>& arr, std::set<T>& result) {
        result.insert(std::begin(arr), std::end(arr));
    }

    template <typename T, size_t S>
    std::set<T> makeSet(const std::array<T, S>& arr) {
        std::set<T> result;
        makeSet(arr, result);
        return result;
    }

    template <typename T>
    void makeSet(const std::vector<T>& vec, std::set<T>& result) {
        result.insert(std::begin(vec), std::end(vec));
    }

    template <typename T>
    std::set<T> makeSet(const std::vector<T>& vec) {
        std::set<T> result;
        makeSet(vec, result);
        return result;
    }

    template <typename T, typename C>
    void makeSet(const std::vector<T>& vec, std::set<T,C>& result) {
        result.insert(std::begin(vec), std::end(vec));
    }

	template <typename T, typename C>
    void minus(const std::set<T, C>& lhs, const std::set<T, C>& rhs, std::set<T, C>& result) {
        std::set_difference(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), std::insert_iterator<std::set<T, C> >(result, std::end(result)));
    }

    template <typename T, typename C>
    std::set<T, C> minus(const std::set<T, C>& lhs, const std::set<T, C>& rhs) {
        std::set<T, C> result(lhs.key_comp());
        minus(lhs, rhs, result);
        return result;
    }

    template <typename T, typename C>
    std::set<T, C> minus(const std::set<T, C>& lhs, const T& rhs) {
        std::set<T, C> result(lhs);
        result.erase(rhs);
        return result;
    }

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
    bool intersectionEmpty(const std::set<T, C>& lhs, const std::set<T, C>& rhs) {
        auto lhsIt = std::begin(lhs);
        auto lhsEnd = std::end(lhs);
        auto rhsIt = std::begin(rhs);
        auto rhsEnd = std::end(rhs);

        const C cmp = lhs.key_comp();

        while (lhsIt != lhsEnd && rhsIt != rhsEnd) {
            const T& l = *lhsIt;
            const T& r = *rhsIt;
            if (cmp(l,r)) {
                ++lhsIt;
            } else if (cmp(r,l)) {
                ++rhsIt;
            } else {
                return false;
            }
        }
        return true;
    }

    template <typename T, typename C>
    std::set<T, C> intersection(const std::set<T, C>& lhs, const std::set<T, C>& rhs) {
        std::set<T, C> result(lhs.key_comp());
        intersection(lhs, rhs, result);
        return result;
    }

    template <typename T, typename C>
    void clearAndDelete(std::set<T*, C>& set) {
        std::for_each(std::begin(set), std::end(set), Utils::Deleter<T>());
        set.clear();
    }

    template <typename T, typename C>
    void deleteAll(const std::set<T*, C>& set) {
        std::for_each(std::begin(set), std::end(set), Utils::Deleter<T>());
    }

    template <typename S>
    std::set<S> powerSet(const S& set) {
        using PowerSet = std::set<S>;

        PowerSet result;
        result.insert(S());

        for (const auto& elem : set) {
            PowerSet intermediate;

            for (auto subset : result) {
                subset.insert(elem);
                intermediate.insert(subset);
            }

            result.insert(std::begin(intermediate), std::end(intermediate));
        }

        return result;
    }

    template <typename S>
    typename S::value_type popFront(S& set) {
        assert(!set.empty());
        const auto it = std::begin(set);
        const typename S::value_type value = *it;
        set.erase(it);
        return value;
    }

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

    template <typename S>
    S& retainMaximalElements(S& set) {
        S temp = findMaximalElements(set);

        using std::swap;
        swap(set, temp);
        return set;
    }

    template <typename S>
    S& retainSupersets(S& set) {
        return CollectionUtils::retainMaximalElements(set, SubsetCmp());
    }

    template <typename S>
    S findSupersets(const S& set) {
        return CollectionUtils::findMaximalElements(set, SubsetCmp());
    }
}

namespace MapUtils {
    template <typename K, typename V>
    struct Deleter {
    public:
        void operator()(std::pair<K, V*>& entry) {
            delete entry.second;
        }

        void operator()(std::pair<const K, V*>& entry) {
            delete entry.second;
        }
    };

    template <typename K, typename V>
    struct VectorDeleter {
    public:
        void operator()(std::pair<K, std::vector<V*> >& entry) {
            VectorUtils::clearAndDelete(entry.second);
        }

        void operator()(std::pair<const K, std::vector<V*> >& entry) {
            VectorUtils::clearAndDelete(entry.second);
        }
    };

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

    template <typename K, typename V, typename C>
    bool equals(const std::map<K, V, C>& map1, const std::map<K, V, C>& map2) {
        return equals(map1, map2, std::less<V>());
    }

    template <typename K, typename V, typename C>
    bool contains(const std::map<K, V, C>& map, const K& key) {
        return map.find(key) != std::end(map);
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
    std::map<K,V,C> minus(const std::map<K,V,C>& lhs, const K& rhs) {
        std::map<K,V,C> result(lhs);
        result.erase(rhs);
        return result;
    }

    template <typename K, typename V, typename C>
    bool removeAndDelete(std::map<K, V*, C>& map, const K& key) {
        using Map = std::map<K, V*, C>;
        typename Map::iterator it = map.find(key);
        if (it == std::end(map))
            return false;

        delete it->second;
        map.erase(it);
        return true;
    }

    template <typename K, typename V, typename C>
    void merge(std::map<K, std::vector<V>, C>& map1, const std::map<K, std::vector<V>, C>& map2) {
        using Vector = std::vector<V>;

        for (const auto& entry : map2) {
            const K& key = entry.first;
            const Vector& vector = entry.second;

            Vector& into = map1[key];
            VectorUtils::append(into, vector);
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
        Deleter<K,V> deleter; // need separate instance because for_each only allows modification of the items if the function is not const
        std::for_each(std::begin(map), std::end(map), deleter);
        map.clear();
    }

    template <typename K, typename V, typename C>
    void clearAndDelete(std::map<K, std::vector<V*>, C>& map) {
        VectorDeleter<K,V> deleter; // need separate instance because for_each only allows modification of the items if the function is not const
        std::for_each(std::begin(map), std::end(map), deleter);
        map.clear();
    }
}

#endif
