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

#include <algorithm>
#include <cstdarg>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <vector>
#include "SharedPointer.h"
#include "Macros.h"

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

class Bitset {
private:
    std::vector<bool> m_bits;
public:
    Bitset(const size_t initialSize = 64) :
    m_bits(initialSize, false) {}

    bool operator[](const size_t index) const {
        if (index >= m_bits.size())
            return false;
        return m_bits[index];
    }

    std::vector<bool>::reference operator[](const size_t index) {
        if (index >= m_bits.size())
            m_bits.insert(std::end(m_bits), index - m_bits.size() + 1, false);
        return m_bits[index];
    }

    void reset() {
        m_bits = std::vector<bool>(64, false);
    }
};

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
}

namespace ListUtils {
    template <typename T>
    void append(std::list<T*>& vec, const std::list<T*>& items) {
        vec.insert(std::end(vec), std::begin(items), std::end(items));
    }

    template <typename T>
    void eraseAll(std::list<T*>& vec, const std::list<T*>& items) {
        vec.erase(removeAll(std::begin(vec), std::end(vec), std::begin(items), std::end(items)), std::end(vec));
    }

    template <typename T>
    void remove(std::vector<T*>& list, const T* item) {
        list.erase(std::remove(std::begin(list), std::end(list), item), std::end(list));
    }

    template <typename T>
    void removeAndDelete(std::vector<T*>& list, const T* item) {
        remove(list, item);
        delete item;
    }

    template <typename T>
    typename std::list<T>::iterator replace(std::list<T>& list, typename std::list<T>::iterator pos, std::list<T>& other) {
        typedef typename std::list<T>::iterator::difference_type DiffType;
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

    template <typename T>
    void clearToZero(std::vector<T>& vec) {
        using std::swap;
        std::vector<T> empty(0);
        swap(vec, empty);
    }

    template <typename T, typename C>
    int compare(const std::vector<T>& lhs, const std::vector<T>& rhs, const C& cmp) {
        typedef std::vector<T> Vec;
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
        typedef typename std::vector<T*>::const_iterator Iter;
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
            return NULL;
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
            return NULL;
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
        typedef typename std::vector<T>::iterator::difference_type DiffType;
        const DiffType modOffset = static_cast<DiffType>(offset) % static_cast<DiffType>(vec.size());
        if (modOffset == 0)
            return;

        std::rotate(std::begin(vec), std::begin(vec) + modOffset, std::end(vec));
    }

    template <typename T>
    void shiftRight(std::vector<T>& vec, const size_t offset) {
        if (vec.empty() || offset == 0)
            return;

        typedef typename std::vector<T>::iterator::difference_type DiffType;
        const DiffType modOffset = static_cast<DiffType>(offset) % static_cast<DiffType>(vec.size());
        const DiffType size = static_cast<DiffType>(vec.size());
		shiftLeft(vec, size - modOffset);
    }

    template <typename T>
    void swapPred(std::vector<T>& vec, typename std::vector<T>::iterator i) {
        assert(i > std::begin(vec) && i < std::end(vec));
        std::iter_swap(i, i-1);
    }

    template <typename T>
    void swapPred(std::vector<T>& vec, const size_t i) {
        typename std::vector<T>::iterator it = std::begin(vec);
        typedef typename std::vector<T>::iterator::difference_type DiffType;
        std::advance(it, static_cast<DiffType>(i));
        swapPred(vec, it);
    }

    template <typename T>
    void swapSucc(std::vector<T>& vec, typename std::vector<T>::iterator i) {
        assert(i >= std::begin(vec) && i < std::end(vec) - 1);
        std::iter_swap(i, i+1);
    }

    template <typename T>
    void swapSucc(std::vector<T>& vec, const size_t i) {
        typename std::vector<T>::iterator it = std::begin(vec);
        typedef typename std::vector<T>::iterator::difference_type DiffType;
        std::advance(it, static_cast<DiffType>(i));
        swapSucc(vec, it);
    }

    template <typename T>
    void clearAndDelete(std::vector<T*>& vec) {
        std::for_each(std::begin(vec), std::end(vec), Utils::Deleter<T>());
        vec.clear();
    }

    template <typename T>
    void deleteAll(const std::vector<T*>& vec) {
        std::for_each(std::begin(vec), std::end(vec), Utils::Deleter<T>());
    }

    template <typename T>
    void erase(std::vector<T>& vec, const size_t index) {
        ensure(index < vec.size(), "index out of range");
        typename std::vector<T>::iterator it = std::begin(vec);
        typedef typename std::vector<T>::iterator::difference_type DiffType;
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
    void append(std::vector<T1>& vec1, const std::vector<T2>& vec2) {
        vec1.insert(std::end(vec1), std::begin(vec2), std::end(vec2));
    }

    template <typename T1, typename T2>
    void append(std::vector<T1>& vec1, const T2* values, const size_t count) {
        vec1.reserve(vec1.size() + count);
        for (size_t i = 0; i < count; ++i)
            vec1.push_back(*(values + i));
    }

    template <typename T, typename K, typename C>
    void append(std::vector<T>& vec, const std::map<K,T,C>& map) {
        vec.reserve(vec.size() + map.size());
        for (const auto& entry : map)
            vec.push_back(entry.second);
    }

    template <typename T>
    void sort(std::vector<T>& vec) {
        std::sort(std::begin(vec), std::end(vec));
    }

    template <typename T, class Cmp>
    void sort(std::vector<T>& vec, const Cmp& cmp) {
        std::sort(std::begin(vec), std::end(vec), cmp);
    }

    template <typename T>
    void sortAndRemoveDuplicates(std::vector<T>& vec) {
        std::sort(std::begin(vec), std::end(vec));
        typename std::vector<T>::iterator it = std::unique(std::begin(vec), std::end(vec));
        vec.erase(it, std::end(vec));
    }

    template <typename T, class Cmp>
    void sortAndRemoveDuplicates(std::vector<T>& vec, const Cmp& cmp) {
        std::sort(std::begin(vec), std::end(vec), cmp);
        typename std::vector<T>::iterator it = std::unique(std::begin(vec), std::end(vec), Utils::EqualsUsingLess<T, Cmp>(cmp));
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

    template <typename T, typename Compare>
    void orderedDifference(std::vector<T>& minuend, const std::vector<T>& subtrahend, const Compare& cmp) {
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

    template <typename T>
    void orderedDifference(std::vector<T>& minuend, const std::vector<T>& subtrahend) {
        orderedDifference(minuend, subtrahend, std::less<T>());
    }

    template <typename T, typename Compare>
    bool setIsSet(const std::vector<T>& set) {
        if (set.size() < 2)
            return true;

        typedef typename std::vector<T>::const_iterator I;
        I cur = std::begin(set);
        I next = cur + 1;

        Compare cmp;
        while (next != std::end(set)) {
            if (!cmp(*cur, *next))
                return false;
            ++cur;
            ++next;
        }
        return true;
    }

    template <typename T>
    bool setIsSet(const std::vector<T>& set) {
        return setIsSet<T, std::less<T> >(set);
    }


    template <typename T, typename Compare>
    void setCreate(std::vector<T>& vec) {
        std::sort(std::begin(vec), std::end(vec), Compare());
        typename std::vector<T>::iterator end = std::unique(std::begin(vec), std::end(vec));
        vec.erase(end, std::end(vec));
    }

    template <typename T>
    void setCreate(std::vector<T>& vec) {
        setCreate<T, std::less<T> >(vec);
    }

    template <typename T1, typename T2, typename Compare>
    bool setInsert(std::vector<T1>& vec, const T2& object, const Compare& cmp) {
        typename std::vector<T1>::iterator it = std::lower_bound(std::begin(vec), std::end(vec), object, cmp);
        if (it == std::end(vec)) {
            vec.push_back(object);
            return true;
        }
        if (cmp(*it, object) || cmp(object, *it)) {
            vec.insert(it, object);
            return true;
        }
        *it = object;
        return false;
    }

    template <typename T1, typename T2, typename Compare>
    bool setInsert(std::vector<T1>& vec, const T2& object) {
        return setInsert(vec, object, Compare());
    }

    template <typename T, typename I, typename Compare>
    void setInsert(std::vector<T>& vec, I cur, const I end, const Compare& cmp) {
        while (cur != end) {
            typename std::vector<T>::iterator it = std::lower_bound(std::begin(vec), std::end(vec), *cur, cmp);
            if (it == std::end(vec))
                vec.push_back(*cur);
            else if (cmp(*it, *cur) || cmp(*cur, *it))
                vec.insert(it, *cur);
            else
                *it = *cur;
            ++cur;
        }
    }

    template <typename T, typename I, typename Compare>
    void setInsert(std::vector<T>& vec, I cur, const I end) {
        return setInsert(vec, cur, end, Compare());
    }

    template <typename T1, typename T2, typename Compare>
    bool setRemove(std::vector<T1>& vec, const T2& object) {
        Compare cmp;
        typename std::vector<T1>::iterator it = std::lower_bound(std::begin(vec), std::end(vec), object, cmp);
        if (it != std::end(vec) && !cmp(*it, object) && !cmp(object, *it)) {
            vec.erase(it);
            return true;
        }
        return false;
    }

    template <typename T, typename I, typename Compare>
    void setRemove(std::vector<T>& vec, I cur, const I end) {
        Compare cmp;
        while (cur != end) {
            typename std::vector<T>::iterator it = std::lower_bound(std::begin(vec), std::end(vec), *cur, cmp);
            if (it != std::end(vec) && !cmp(*it, *cur) && !cmp(*cur, *it))
                vec.erase(it);
            ++cur;
        }
    }

    template <typename T1, typename T2>
    bool setInsert(std::vector<T1>& vec, const T2& object) {
        return setInsert<T1, T2, std::less<T1> >(vec, object);
    }

    template <typename T, typename I>
    void setInsert(std::vector<T>& vec, I cur, const I end) {
        setInsert<T, I, std::less<T> >(vec, cur, end);
    }

    template <typename T1, typename T2>
    bool setRemove(std::vector<T1>& vec, const T2& object) {
        return setRemove<T1, T2, std::less<T1> >(vec, object);
    }

    template <typename T, typename I>
    void setRemove(std::vector<T>& vec, I cur, const I end) {
        setRemove<T, I, std::less<T> >(vec, cur, end);
    }

    template <typename T, typename Compare>
    bool setContains(const std::vector<T>& vec, const T& object) {
        Compare cmp;
        typename std::vector<T>::const_iterator it = std::lower_bound(std::begin(vec), std::end(vec), object, cmp);
        return it != std::end(vec) && !cmp(*it, object) && !cmp(object, *it);
    }

    template <typename T>
    bool setContains(const std::vector<T>& vec, const T& object) {
        return setContains<T, std::less<T> >(vec, object);
    }

    template <typename T, typename Compare>
    std::vector<T> setUnion(const std::vector<T>& vec1, const std::vector<T>& vec2) {
        if (vec1.empty())
            return vec2;
        if (vec2.empty())
            return vec1;

        typedef typename std::vector<T> Vec;
        typedef typename Vec::const_iterator I;

        I cur1 = std::begin(vec1);
        const I end1 = std::end(vec1);
        I cur2 = std::begin(vec2);
        const I end2 = std::end(vec2);

        Vec result;
        Compare cmp;

        while (cur1 != end1 && cur2 != end2) {
            if (cmp(*cur1, *cur2)) {
                result.push_back(*cur1);
                ++cur1;
            } else if (cmp(*cur2, *cur1)) {
                result.push_back(*cur2);
                ++cur2;
            } else {
                // both values are equal
                result.push_back(*cur1);
                ++cur1;
                ++cur2;
            }
        }

        while (cur1 != end1)
            result.push_back(*cur1++);
        while (cur2 != end2)
            result.push_back(*cur2++);

        return result;
    }

    template <typename T>
    std::vector<T> setUnion(const std::vector<T>& vec1, const std::vector<T>& vec2) {
        return setUnion<T, std::less<T> >(vec1, vec2);
    }

    template <typename T, typename Compare>
    std::vector<T> setMinus(const std::vector<T>& minuend, const std::vector<T>& subtrahend) {
        if (minuend.empty() || subtrahend.empty())
            return minuend;

        typedef typename std::vector<T> Vec;

        Vec result = minuend;
        typename Vec::iterator curMin = std::begin(result);
        typename Vec::const_iterator curSub = std::begin(subtrahend);
        const typename Vec::const_iterator endSub = std::end(subtrahend);

        Compare cmp;

        while (curMin != std::end(result) && curSub != endSub) {
            if (cmp(*curMin, *curSub)) {
                ++curMin;
            } else if (cmp(*curSub, *curMin)) {
                ++curSub;
            } else {
                // both values are equal
                curMin = result.erase(curMin);
            }
        }

        return result;
    }

    template <typename T>
    std::vector<T> setMinus(const std::vector<T>& minuend, const std::vector<T>& subtrahend) {
        return setMinus<T, std::less<T> >(minuend, subtrahend);
    }

    template <typename T, typename Compare>
    std::vector<T> setIntersection(const std::vector<T>& vec1, const std::vector<T>& vec2) {
        if (vec1.empty())
            return vec1;
        if (vec2.empty())
            return vec2;

        typedef typename std::vector<T> Vec;
        typedef typename Vec::const_iterator I;

        I cur1 = std::begin(vec1);
        const I end1 = std::end(vec1);
        I cur2 = std::begin(vec2);
        const I end2 = std::end(vec2);

        Vec result;
        Compare cmp;

        while (cur1 != end1 && cur2 != end2) {
            if (cmp(*cur1, *cur2)) {
                ++cur1;
            } else if (cmp(*cur2, *cur1)) {
                ++cur2;
            } else {
                // both values are equal
                result.push_back(*cur1);
                ++cur1;
                ++cur2;
            }
        }

        return result;
    }

    template <typename T>
    std::vector<T> setIntersection(const std::vector<T>& vec1, const std::vector<T>& vec2) {
        return setIntersection<T, std::less<T> >(vec1, vec2);
    }
}

namespace SetUtils {
    template<typename T, typename C, typename A>
    class set_insert_iterator : public std::output_iterator_tag {
    public:
        typedef set_insert_iterator<T,C,A> _my_type;
        typedef std::set<T,C,A> container_type;
        typedef typename container_type::const_reference const_reference;
        typedef typename container_type::value_type value_type;
    private:
        container_type& m_set;
    public:
        set_insert_iterator(std::set<T,C,A>& set)
        : m_set(set) {}

        _my_type& operator=(const value_type& value) {
            m_set.insert(value);
            return *this;
        }

        /*
        container_type& operator=(value_type&& value)
        {
            m_set.insert(std::forward<value_type>(value));
            return (*this);
        }
        */

        _my_type& operator*() {
            return *this;
        }

        _my_type& operator++() {
            return *this;
        }

        _my_type& operator++(int) {
            return *this;
        }
    };

    template<typename T, typename C, typename A>
    inline set_insert_iterator<T,C,A> set_inserter(std::set<T,C,A>& set) {
        return set_insert_iterator<T,C,A>(set);
    }

    template <typename T, typename C>
    bool subset(const std::set<T,C>& lhs, const std::set<T,C>& rhs) {
        if (lhs.size() > rhs.size())
            return false;

        typedef typename std::set<T,C>::const_iterator Iter;
        Iter lIt = std::begin(lhs);
        Iter lEnd = std::end(lhs);
        Iter rIt = std::begin(rhs);
        Iter rEnd = std::end(rhs);

        const C& cmp = lhs.key_comp();

        while (lIt != lEnd) {
            // forward rhs until we find the element
            while (rIt != rEnd && cmp(*rIt, *lIt)) ++rIt;
            if (rIt == rEnd || cmp(*lIt, *rIt)) // we didn't find it
                return false;

            // we found it, continue with next element
            ++lIt;
        }
        return true;
    }

    template <typename T>
    void makeSet(const std::vector<T>& vec, std::set<T>& result) {
        result.insert(std::begin(vec), std::end(vec));
    }

    template <typename T>
    std::set<T> makeSet(const std::vector<T>& vec) {
        std::set<T> result;
        makeSet(vec);
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
        std::set_intersection(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), std::insert_iterator<std::set<T, C> >(result, std::end(result)));
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
        
        const C& cmp = lhs.key_comp();
        
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
    const std::set<T, C> intersection(const std::set<T, C>& lhs, const std::set<T, C>& rhs) {
        std::set<T, C> result(lhs.key_comp());
        intersection(lhs, rhs, result);
        return result;
    }
    
    // TODO: In C++11, std::set::erase already returns an iterator to the next element
    template <typename T, typename C>
    typename std::set<T,C>::iterator erase(std::set<T,C>& set, typename std::set<T,C>::iterator it) {
        typename std::set<T,C>::iterator tmp = it;
        ++it;
        set.erase(tmp);
        return it;
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
        typedef std::map<K, V, C> Map;
        typename Map::const_iterator it = map.find(key);
        if (it == std::end(map))
            return defaultValue;
        return it->second;
    }

    template <typename K, typename V, typename C>
    std::pair<bool, typename std::map<K, V, C>::iterator> findInsertPos(std::map<K, V, C>& map, const K& key) {
        typedef std::map<K, V, C> Map;
        typename Map::key_compare compare = map.key_comp();
        typename Map::iterator insertPos = map.lower_bound(key);
        if (insertPos == std::end(map) || compare(key, insertPos->first)) {
            if (insertPos != std::begin(map))
                --insertPos;
            return std::make_pair(false, insertPos);
        }
        return std::make_pair(true, insertPos);
    }

    template <typename K, typename V, typename C, typename W>
    typename std::map<K, V, C>::iterator findOrInsert(std::map<K, V, C>& map, const K& key, const W& value) {
        typedef std::map<K, V, C> Map;
        typedef std::pair<bool, typename Map::iterator> InsertPos;

        const InsertPos insertPos = findInsertPos(map, key);
        if (!insertPos.first)
            return map.insert(insertPos.second, std::make_pair(key, V(value)));
        return insertPos.second;
    }

    template <typename K, typename V, typename C>
    typename std::map<K, V, C>::iterator findOrInsert(std::map<K, V, C>& map, const K& key) {
        typedef std::map<K, V, C> Map;
        typedef std::pair<bool, typename Map::iterator> InsertPos;

        const InsertPos insertPos = findInsertPos(map, key);
        if (!insertPos.first)
            return map.insert(insertPos.second, std::make_pair(key, V()));
        return insertPos.second;
    }

    template <typename K, typename V, typename C>
    bool insertOrFail(std::map<K, V, C>& map, const K& key, const V& value) {
        typedef std::map<K, V, C> Map;
        typename Map::key_compare compare = map.key_comp();
        typename Map::iterator insertPos = map.lower_bound(key);
        if (insertPos == std::end(map) || compare(key, insertPos->first)) {
            // the two keys are not equal (key is less than insertPos' key), so we must insert the value
            // in C++98, the insert position points to the element that precedes the inserted element!
            if (insertPos != std::begin(map))
                --insertPos;
            assert(map.count(key) == 0);
            map.insert(insertPos, std::make_pair(key, V(value)));
            assert(map.count(key) == 1);
            return true;
        }
        return false;
    }

    template <typename K, typename V, typename C>
    bool insertOrReplace(std::map<K, V, C>& map, const K& key, const V& value) {
        typedef std::map<K, V, C> Map;
        typename Map::key_compare compare = map.key_comp();
        typename Map::iterator insertPos = map.lower_bound(key);
        if (insertPos == std::end(map) || compare(key, insertPos->first)) {
            // the two keys are not equal (key is less than insertPos' key), so we must insert the value
            // in C++98, the insert position points to the element that precedes the inserted element!
            if (insertPos != std::begin(map))
                --insertPos;
            assert(map.count(key) == 0);
            map.insert(insertPos, std::make_pair(key, value));
            assert(map.count(key) == 1);
            return true;
        } else {
            // the two keys are equal because insertPos either points to the pair with the same key or the one
            // right after the position where the given pair would be inserted
            insertPos->second = value;
            return false;
        }
    }

    template <typename K, typename V, typename C>
    bool insertOrReplaceAndDelete(std::map<K, V*, C>& map, const K& key, V* value) {
        typedef std::map<K, V*, C> Map;
        typename Map::key_compare compare = map.key_comp();
        typename Map::iterator insertPos = map.lower_bound(key);
        if (insertPos == std::end(map) || compare(key, insertPos->first)) {
            // the two keys are not equal (key is less than insertPos' key), so we must insert the value
            // in C++98, the insert position points to the element that precedes the inserted element!
            if (insertPos != std::begin(map))
                --insertPos;
            assert(map.count(key) == 0);
            map.insert(insertPos, std::make_pair(key, value));
            assert(map.count(key) == 1);
            return true;
        } else {
            // the two keys are equal because insertPos either points to the pair with the same key or the one
            // right after the position where the given pair would be inserted
            delete insertPos->second;
            insertPos->second = value;
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
        typedef std::map<K, V*, C> Map;
        typename Map::iterator it = map.find(key);
        if (it == std::end(map))
            return false;

        delete it->second;
        map.erase(it);
        return true;
    }

    template <typename K, typename V, typename C>
    void merge(std::map<K, std::vector<V>, C>& map1, const std::map<K, std::vector<V>, C>& map2) {
        typedef std::vector<V> Vector;

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
