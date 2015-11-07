/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
            m_bits.insert(m_bits.end(), index - m_bits.size() + 1, false);
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
        vec.insert(vec.end(), items.begin(), items.end());
    }

    template <typename T>
    void eraseAll(std::list<T*>& vec, const std::list<T*>& items) {
        vec.erase(removeAll(vec.begin(), vec.end(), items.begin(), items.end()), vec.end());
    }
    
    template <typename T>
    void remove(std::vector<T*>& list, const T* item) {
        list.erase(std::remove(list.begin(), list.end(), item), list.end());
    }

    template <typename T>
    void removeAndDelete(std::vector<T*>& list, const T* item) {
        remove(list, item);
        delete item;
    }
    
    
    template <typename T>
    void clearAndDelete(std::list<T*>& list) {
        std::for_each(list.begin(), list.end(), Utils::Deleter<T>());
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
    
    template <typename T>
    bool equals(const std::vector<T>& lhs, const std::vector<T>& rhs) {
        if (lhs.size() != rhs.size())
            return false;
        for (size_t i = 0; i < lhs.size(); ++i)
            if (lhs[i] != rhs[i])
                return false;
        return true;
    }
    
    template <typename T>
    typename std::vector<T>::const_iterator find(const std::vector<T>& vec, const T& item) {
        return std::find(vec.begin(), vec.end(), item);
    }
    
    template <typename T>
    typename std::vector<T>::iterator find(std::vector<T>& vec, const T& item) {
        return std::find(vec.begin(), vec.end(), item);
    }
    
    template <typename T>
    typename std::vector<T*>::const_iterator find(const std::vector<T*>& vec, const T* item) {
        return std::find(vec.begin(), vec.end(), item);
    }
    
    template <typename T>
    typename std::vector<T*>::iterator find(std::vector<T*>& vec, const T* item) {
        return std::find(vec.begin(), vec.end(), item);
    }
    
    template <typename T>
    typename std::vector<T*>::const_iterator findOther(const std::vector<T*>& vec, const T* item) {
        typedef typename std::vector<T*>::const_iterator Iter;
        Iter cur = vec.begin();
        Iter end = vec.end();
        while (cur != end) {
            if (*cur != item)
                return cur;
            ++cur;
        }
        return end;
    }
    
    template <typename T, class P>
    T* findIf(const std::vector<T*>& vec, const P& predicate) {
        typename std::vector<T*>::const_iterator it = std::find_if(vec.begin(), vec.end(), predicate);
        if (it == vec.end())
            return NULL;
        return *it;
    }
    
    template <typename T, class P>
    const std::tr1::shared_ptr<T> findIf(const std::vector<std::tr1::shared_ptr<T> >& vec, const P& predicate) {
        typename std::vector<std::tr1::shared_ptr<T> >::const_iterator it = std::find_if(vec.begin(), vec.end(), predicate);
        if (it == vec.end())
            return std::tr1::shared_ptr<T>();
        return *it;
    }
    
    template <typename T, class P>
    const T* findIf(const std::vector<T>& vec, const P& predicate) {
        typename std::vector<T>::const_iterator it = std::find_if(vec.begin(), vec.end(), predicate);
        if (it == vec.end())
            return NULL;
        return &(*it);
    }
    
    template <typename T, typename Cmp>
    bool contains(const std::vector<T>& vec, const T& item, const Cmp& cmp) {
        typedef std::vector<T> VecType;
        typedef typename VecType::const_iterator VecIter;
        
        VecIter first = vec.begin();
        const VecIter last = vec.end();
        while (first != last)
            if (cmp(*(first++), item))
                return true;
        return false;
    }
    
    template <typename T1, typename T2>
    bool contains(const std::vector<T1>& vec, const T2& item) {
        return std::find(vec.begin(), vec.end(), item) != vec.end();
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
            
        std::rotate(vec.begin(), vec.begin() + modOffset, vec.end());
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
    void clearAndDelete(std::vector<T*>& vec) {
        std::for_each(vec.begin(), vec.end(), Utils::Deleter<T>());
        vec.clear();
    }
    
    template <typename T>
    void deleteAll(const std::vector<T*>& vec) {
        std::for_each(vec.begin(), vec.end(), Utils::Deleter<T>());
    }
    
    template <typename T1, typename T2>
    bool erase(std::vector<T1>& vec, const T2& item) {
        typename std::vector<T1>::iterator it = std::remove(vec.begin(), vec.end(), item);
        if (it == vec.end())
            return false;
        vec.erase(it, vec.end());
        return true;
    }
    
    template <typename T1, typename T2>
    bool erase(std::vector<T1*>& vec, const T2* item) {
        typename std::vector<T1*>::iterator it = std::remove(vec.begin(), vec.end(), item);
        if (it == vec.end())
            return false;
        vec.erase(it, vec.end());
        return true;
    }
    
    template <typename T, class P>
    void eraseIf(std::vector<T>& vec, const P& pred) {
        vec.erase(std::remove_if(vec.begin(), vec.end(), pred), vec.end());
    }
    
    template <typename T>
    void eraseAll(std::vector<T*>& vec, const std::vector<T*>& items) {
        vec.erase(CollectionUtils::removeAll(vec.begin(), vec.end(), items.begin(), items.end()), vec.end());
    }
    
    template <typename T, typename I>
    void eraseAll(std::vector<T*>& vec, I cur, I end) {
        vec.erase(CollectionUtils::removeAll(vec.begin(), vec.end(), cur, end), vec.end());
    }
    
    template <typename T>
    std::vector<T*> eraseAll(const std::vector<T*>& vec, const std::vector<T*>& items) {
        std::vector<T*> result(vec);
        result.erase(CollectionUtils::removeAll(result.begin(), result.end(), items.begin(), items.end()), result.end());
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
        eraseAndDelete(vec, first, vec.end());
    }
    
    template <typename T1, typename T2, typename R>
    void concatenate(const std::vector<T1>& vec1, const std::vector<T2>& vec2, std::vector<R>& result) {
        result.clear();
        result.reserve(vec1.size() + vec2.size());
        result.insert(result.end(), vec1.begin(), vec1.end());
        result.insert(result.end(), vec2.begin(), vec2.end());
    }
    
    template <typename T>
    std::vector<T> concatenate(const std::vector<T>& vec1, const std::vector<T>& vec2) {
        std::vector<T> result;
        concatenate(vec1, vec2, result);
        return result;
    }
    
    template <typename T1, typename T2>
    void append(std::vector<T1>& vec1, const std::vector<T2>& vec2) {
        vec1.insert(vec1.end(), vec2.begin(), vec2.end());
    }
    
    template <typename T1, typename T2>
    void append(std::vector<T1>& vec1, const T2* values, const size_t count) {
        vec1.reserve(vec1.size() + count);
        for (size_t i = 0; i < count; ++i)
            vec1.push_back(*(values + i));
    }
    
    template <typename T>
    void sort(std::vector<T>& vec) {
        std::sort(vec.begin(), vec.end());
    }
    
    template <typename T, class Cmp>
    void sort(std::vector<T>& vec, const Cmp& cmp) {
        std::sort(vec.begin(), vec.end(), cmp);
    }
    
    template <typename T>
    void sortAndRemoveDuplicates(std::vector<T>& vec) {
        std::sort(vec.begin(), vec.end());
        typename std::vector<T>::iterator it = std::unique(vec.begin(), vec.end());
        vec.erase(it, vec.end());
    }
    
    template <typename T, class Cmp>
    void sortAndRemoveDuplicates(std::vector<T>& vec, const Cmp& cmp) {
        std::sort(vec.begin(), vec.end(), cmp);
        typename std::vector<T>::iterator it = std::unique(vec.begin(), vec.end(), Utils::EqualsUsingLess<T, Cmp>(cmp));
        vec.erase(it, vec.end());
    }
    
    template <typename T>
    std::vector<T> difference(const std::vector<T>& vec1, const std::vector<T>& vec2) {
        typedef std::vector<T> VecType;
        typedef typename VecType::const_iterator VecIter;
        
        VecType result;
        VecIter it, end;
        for (it = vec1.begin(), end = vec1.end(); it != end; ++it) {
            T elem = *it;
            if (!VectorUtils::contains(vec2, elem))
                result.push_back(elem);
        }
        return result;
    }
    
    template <typename O, typename I>
    std::vector<O> cast(const std::vector<I>& input) {
        std::vector<O> output;
        typename std::vector<I>::const_iterator it, end;
        for (it = input.begin(), end = input.end(); it != end; ++it)
            output.push_back(O(*it));
        return output;
    }

    template <typename T, typename Compare>
    void orderedDifference(std::vector<T>& minuend, const std::vector<T>& subtrahend, const Compare& cmp) {
        typedef std::vector<T> Vec;
        
        typename Vec::iterator mIt = minuend.begin();
        typename Vec::const_iterator sIt = subtrahend.begin();
        typename Vec::const_iterator sEnd = subtrahend.end();
        
        while (mIt != minuend.end() && sIt != sEnd) {
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
        I cur = set.begin();
        I next = cur + 1;
        
        Compare cmp;
        while (next != set.end()) {
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
        std::sort(vec.begin(), vec.end(), Compare());
        typename std::vector<T>::iterator end = std::unique(vec.begin(), vec.end());
        vec.erase(end, vec.end());
    }
    
    template <typename T>
    void setCreate(std::vector<T>& vec) {
        setCreate<T, std::less<T> >(vec);
    }

    template <typename T1, typename T2, typename Compare>
    bool setInsert(std::vector<T1>& vec, const T2& object, const Compare& cmp) {
        typename std::vector<T1>::iterator it = std::lower_bound(vec.begin(), vec.end(), object, cmp);
        if (it == vec.end()) {
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
            typename std::vector<T>::iterator it = std::lower_bound(vec.begin(), vec.end(), *cur, cmp);
            if (it == vec.end())
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
        typename std::vector<T1>::iterator it = std::lower_bound(vec.begin(), vec.end(), object, cmp);
        if (it != vec.end() && !cmp(*it, object) && !cmp(object, *it)) {
            vec.erase(it);
            return true;
        }
        return false;
    }
    
    template <typename T, typename I, typename Compare>
    void setRemove(std::vector<T>& vec, I cur, const I end) {
        Compare cmp;
        while (cur != end) {
            typename std::vector<T>::iterator it = std::lower_bound(vec.begin(), vec.end(), *cur, cmp);
            if (it != vec.end() && !cmp(*it, *cur) && !cmp(*cur, *it))
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
        typename std::vector<T>::const_iterator it = std::lower_bound(vec.begin(), vec.end(), object, cmp);
        return it != vec.end() && !cmp(*it, object) && !cmp(object, *it);
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
        
        I cur1 = vec1.begin();
        const I end1 = vec1.end();
        I cur2 = vec2.begin();
        const I end2 = vec2.end();
        
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
        typename Vec::iterator curMin = result.begin();
        typename Vec::const_iterator curSub = subtrahend.begin();
        const typename Vec::const_iterator endSub = subtrahend.end();
        
        Compare cmp;
        
        while (curMin != result.end() && curSub != endSub) {
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
        
        I cur1 = vec1.begin();
        const I end1 = vec1.end();
        I cur2 = vec2.begin();
        const I end2 = vec2.end();
        
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
        Iter lIt = lhs.begin();
        Iter lEnd = lhs.end();
        Iter rIt = rhs.begin();
        Iter rEnd = rhs.end();
        
        C cmp;
        
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
        result.insert(vec.begin(), vec.end());
    }

    template <typename T>
    std::set<T> makeSet(const std::vector<T>& vec) {
        std::set<T> result;
        makeSet(vec);
        return result;
    }

    template <typename T, typename C>
    void makeSet(const std::vector<T>& vec, std::set<T,C>& result) {
        result.insert(vec.begin(), vec.end());
    }
    
	template <typename T, typename C>
    void minus(const std::set<T, C>& lhs, const std::set<T, C>& rhs, std::set<T, C>& result) {
        std::set_difference(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::insert_iterator<std::set<T, C> >(result, result.end()));
    }
    
    template <typename T, typename C>
    std::set<T, C> minus(const std::set<T, C>& lhs, const std::set<T, C>& rhs) {
        std::set<T, C> result;
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
        result.insert(lhs.begin(), lhs.end());
        result.insert(rhs.begin(), rhs.end());
    }

    template <typename T, typename C>
    std::set<T, C> merge(const std::set<T, C>& lhs, const std::set<T, C>& rhs) {
        std::set<T, C> result;
        merge(lhs, rhs, result);
        return result;
    }
    
    template <typename T, typename C>
    void intersection(const std::set<T, C>& lhs, const std::set<T, C>& rhs, std::set<T, C>& result) {
        std::set_intersection(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::insert_iterator<std::set<T, C> >(result, result.end()));
    }
    
    template <typename T, typename C>
    void intersection(const std::set<T, C>& lhs, const std::set<T, C>& rhs, std::vector<T>& result) {
        std::set_intersection(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::back_inserter(result));
    }

    template <typename T, typename C>
    typename std::set<T,C>::iterator erase(std::set<T,C>& set, typename std::set<T,C>::iterator it) {
        typename std::set<T,C>::iterator tmp = it;
        ++it;
        set.erase(tmp);
        return it;
    }

    template <typename T, typename C>
    void clearAndDelete(std::set<T*, C>& set) {
        std::for_each(set.begin(), set.end(), Utils::Deleter<T>());
        set.clear();
    }
    
    template <typename T, typename C>
    void deleteAll(const std::set<T*, C>& set) {
        std::for_each(set.begin(), set.end(), Utils::Deleter<T>());
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

    template <typename K, typename V>
    bool contains(const std::map<K, V>& map, const K& key) {
        return map.find(key) != map.end();
    }
    
    template <typename K, typename V, typename L>
    const V& find(const std::map<K, V>& map, const L& key, const V& defaultValue) {
        typedef std::map<K, V> Map;
        typename Map::const_iterator it = map.find(key);
        if (it == map.end())
            return defaultValue;
        return it->second;
    }
    
    template <typename K, typename V>
    std::pair<bool, typename std::map<K, V>::iterator> findInsertPos(std::map<K, V>& map, const K& key) {
        typedef std::map<K, V> Map;
        typename Map::key_compare compare = map.key_comp();
        typename Map::iterator insertPos = map.lower_bound(key);
        if (insertPos == map.end() || compare(key, insertPos->first)) {
            if (insertPos != map.begin())
                --insertPos;
            return std::make_pair(false, insertPos);
        }
        return std::make_pair(true, insertPos);
    }
    
    template <typename K, typename V, typename W>
    typename std::map<K, V>::iterator findOrInsert(std::map<K, V>& map, const K& key, const W& value) {
        typedef std::map<K, V> Map;
        typedef std::pair<bool, typename Map::iterator> InsertPos;
        
        const InsertPos insertPos = findInsertPos(map, key);
        if (!insertPos.first)
            return map.insert(insertPos.second, std::make_pair(key, V(value)));
        return insertPos.second;
    }

    template <typename K, typename V>
    typename std::map<K, V>::iterator findOrInsert(std::map<K, V>& map, const K& key) {
        return findOrInsert(map, key, V());
    }
    
    template <typename K, typename V>
    bool insertOrFail(std::map<K, V>& map, const K& key, const V& value) {
        typedef std::map<K, V> Map;
        typename Map::key_compare compare = map.key_comp();
        typename Map::iterator insertPos = map.lower_bound(key);
        if (insertPos == map.end() || compare(key, insertPos->first)) {
            // the two keys are not equal (key is less than insertPos' key), so we must insert the value
            // in C++98, the insert position points to the element that precedes the inserted element!
            if (insertPos != map.begin())
                --insertPos;
            assert(map.count(key) == 0);
            map.insert(insertPos, std::make_pair(key, V(value)));
            assert(map.count(key) == 1);
            return true;
        }
        return false;
    }

    template <typename K, typename V>
    bool insertOrReplace(std::map<K, V>& map, const K& key, const V& value) {
        typedef std::map<K, V> Map;
        typename Map::key_compare compare = map.key_comp();
        typename Map::iterator insertPos = map.lower_bound(key);
        if (insertPos == map.end() || compare(key, insertPos->first)) {
            // the two keys are not equal (key is less than insertPos' key), so we must insert the value
            // in C++98, the insert position points to the element that precedes the inserted element!
            if (insertPos != map.begin())
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

    template <typename K, typename V>
    bool insertOrReplaceAndDelete(std::map<K, V*>& map, const K& key, V* value) {
        typedef std::map<K, V*> Map;
        typename Map::key_compare compare = map.key_comp();
        typename Map::iterator insertPos = map.lower_bound(key);
        if (insertPos == map.end() || compare(key, insertPos->first)) {
            // the two keys are not equal (key is less than insertPos' key), so we must insert the value
            // in C++98, the insert position points to the element that precedes the inserted element!
            if (insertPos != map.begin())
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
    
    template <typename K, typename V>
    bool removeAndDelete(std::map<K, V*>& map, const K& key) {
        typedef std::map<K, V*> Map;
        typename Map::iterator it = map.find(key);
        if (it == map.end())
            return false;
        
        delete it->second;
        map.erase(it);
        return true;
    }
    
    template <typename K, typename V>
    void merge(std::map<K, std::vector<V> >& map1, const std::map<K, std::vector<V> >& map2) {
        typedef std::vector<V> Vector;
        typedef std::map<K, Vector> Map;
        
        typename Map::const_iterator it, end;
        for (it = map2.begin(), end = map2.end(); it != end; ++it) {
            const K& key = it->first;
            const Vector& vector = it->second;
            
            Vector& into = map1[key];
            VectorUtils::append(into, vector);
        }
    }
    
    template <typename K, typename V>
    void clearAndDelete(std::map<K, V*>& map) {
        Deleter<K,V> deleter; // need separate instance because for_each only allows modification of the items if the function is not const
        std::for_each(map.begin(), map.end(), deleter);
        map.clear();
    }

    template <typename K, typename V>
    void clearAndDelete(std::map<K, std::vector<V*> >& map) {
        VectorDeleter<K,V> deleter; // need separate instance because for_each only allows modification of the items if the function is not const
        std::for_each(map.begin(), map.end(), deleter);
        map.clear();
    }
    
    template <typename K, typename V>
    std::vector<K> keyList(const std::map<K,V>& map) {
        std::vector<K> result;
        result.reserve(map.size());
        
        typename std::map<K,V>::const_iterator it, end;
        for (it = map.begin(), end = map.end(); it != end; ++it)
            result.push_back(it->first);
        return result;
    }
}

#endif
