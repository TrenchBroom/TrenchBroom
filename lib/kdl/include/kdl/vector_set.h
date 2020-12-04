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

#include "set_adapter.h"
#include "vector_set_forward.h"

#include <cassert>
#include <functional> // for std::less
#include <memory> // for std::allocator
#include <vector>

namespace kdl {
    // default template arguments are defined in vector_set_forward.h
    template <typename T, typename Compare, typename Allocator>
    class vector_set : public set_adapter<std::vector<T, Allocator>, Compare> {
    private:
        using vec_type = std::vector<T, Allocator>;
        using base = set_adapter<vec_type, Compare>;
        using base::m_data;
        using base::m_cmp;
    public:
        /**
         * Creates a new empty set with the given comparator and the given allocator.
         *
         * @param cmp the comparator to use, defaults to a newly created instance of Compare
         * @param alloc the allocator to use for the underlying vector, defaults to a newly created instance of Allocator
         */
        explicit vector_set(const Compare& cmp = Compare(), const Allocator& alloc = Allocator()) :
            base(std::vector<T, Allocator>(alloc), cmp) {}

        /**
         * Creates a new empty set with the given comparator and the given allocator. The underlying vector is initialized
         * with the given capacity.
         *
         * @param capacity the initial capacity of the underlying vector
         * @param cmp the comparator to use, defaults to a newly created instance of Compare
         * @param alloc the allocator to use for the underlying vector, defaults to a newly created instance of Allocator
         */
        explicit vector_set(const typename base::size_type capacity, const Compare& cmp = Compare(), const Allocator& alloc = Allocator()) :
            base(std::vector<T, Allocator>(alloc), cmp) {
            m_data.reserve(capacity);
        }

        /**
         * Creates a new empty set with the given allocator.
         *
         * @param alloc the allocator to use for the underlying vector, defaults to a newly created instance of Allocator
         */
        explicit vector_set(const Allocator& alloc) :
            base(std::vector<T, Allocator>(alloc), Compare()) {}

        /**
         * Creates a new empty set with the given allocator. The underlying vector is initialized with the given capacity.
         *
         * @param capacity the initial capacity of the underlying vector
         * @param alloc the allocator to use for the underlying vector, defaults to a newly created instance of Allocator
         */
        explicit vector_set(const typename base::size_type capacity, const Allocator& alloc) :
            base(std::vector<T, Allocator>(alloc), Compare()) {
            m_data.reserve(capacity);
        }

        /**
         * Creates a vector set containing the values in the given range [first, last).
         *
         * @tparam I the iterator type, must be an input iterator
         * @param first the start of the range of values to insert
         * @param last the end of the range of values to insert (exclusive)
         * @param cmp the comparator to use, defaults to a newly created instance of Compare
         * @param alloc the allocator to use for the underlying vector, defaults to a newly created instance of Allocator
         */
        template <typename I>
        vector_set(I first, I last, const Compare& cmp = Compare(), const Allocator& alloc = Allocator()) :
            base(std::vector<T, Allocator>(alloc), cmp) {
            m_data.insert(std::end(m_data), first, last);
            detail::sort_unique(m_data, m_cmp);
            assert(check_invariant());
        }

        /**
         * Creates a vector set containing the values in the given range [first, last).  The underlying vector is
         * initialized with the given capacity.
         *
         * @tparam I the iterator type, must be an input iterator
         * @param capacity the initial capacity of the underlying vector
         * @param first the start of the range of values to insert
         * @param last the end of the range of values to insert (exclusive)
         * @param cmp the comparator to use, defaults to a newly created instance of Compare
         * @param alloc the allocator to use for the underlying vector, defaults to a newly created instance of Allocator
         */
        template <typename I>
        vector_set(const typename base::size_type capacity, I first, I last, const Compare& cmp = Compare(), const Allocator& alloc = Allocator()) :
            base(std::vector<T, Allocator>(alloc), cmp) {
            m_data.reserve(capacity);
            m_data.insert(std::end(m_data), first, last);
            detail::sort_unique(m_data, m_cmp);
            assert(check_invariant());
        }

        /**
         * Creates a vector set containing the values in the given vector. The capacity is initialized to the size of the
         * given vector.
         *
         * @tparam TT the value type of the given vector
         * @tparam AA the allocator type of the given vector
         * @param vec the vector
         * @param cmp the comparator to use, defaults to a newly created instance of Compare
         * @param alloc the allocator to use for the underlying vector, defaults to a newly created instance of Allocator
         */
        template <typename TT, typename AA>
        vector_set(const std::vector<TT,AA>& vec, const Compare& cmp = Compare(), const Allocator& alloc = Allocator()) :
            vector_set(vec.size(), std::begin(vec), std::end(vec), cmp, alloc) {}

        /**
         * Creates a vector set containing the values in the given initializer list.
         *
         * @param values the values to insert
         * @param cmp the comparator to use, defaults to a newly created instance of Compare
         * @param alloc the allocator to use for the underlying vector, defaults to a newly created instance of Allocator
         */
        vector_set(std::initializer_list<typename base::value_type> values, const Compare& cmp = Compare(), const Allocator& alloc = Allocator()) :
            vector_set(values.size(), std::move(values), cmp, alloc) {}

        /**
         * Creates a vector set containing the values in the given initializer list. The underlying vector is initialized
         * with the given capacity.
         *
         * @param capacity the initial capacity of the underlying vector
         * @param values the values to insert
         * @param cmp the comparator to use, defaults to a newly created instance of Compare
         * @param alloc the allocator to use for the underlying vector, defaults to a newly created instance of Allocator
         */
        vector_set(const typename base::size_type capacity, std::initializer_list<typename base::value_type> values, const Compare& cmp = Compare(), const Allocator& alloc = Allocator()) :
            vector_set(capacity, std::begin(values), std::end(values), cmp, alloc) {}

        /**
         * Assigns the values in the given initializer list to this set. The set is cleared and the given values are
         * inserted.
         *
         * @param values the values to insert
         * @return a reference to this set
         */
        vector_set& operator=(std::initializer_list<typename base::value_type> values) {
            m_data = values;
            detail::sort_unique(m_data, m_cmp);
            return *this;
        }

        /**
         * Assigns the values in the given vector to this set. The set is cleared and the values from the given vector
         * are inserted.
         *
         * @param values the vector to insert
         * @return a reference to this set
         */
        vector_set& operator=(std::vector<typename base::value_type> values) {
            m_data = values;
            detail::sort_unique(m_data, m_cmp);
            return *this;
        }
    private:
        using base::check_invariant;
    };

    /**
     * Deduction guide for vector constructor.
     */
    template <typename TT, typename AA, typename T = TT, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
    vector_set(std::vector<TT, AA>, const Compare& cmp = Compare(), const Allocator& alloc = Allocator()) -> vector_set<T, Compare, Allocator>;

    /**
     * Deduction guide for range constructor.
     */
    template <typename I, typename Compare = std::less<typename std::iterator_traits<I>::value_type>, typename Allocator = std::allocator<typename std::iterator_traits<I>::value_type>>
    vector_set(I first, I last, const Compare& cmp = Compare(), const Allocator& alloc = Allocator()) -> vector_set<typename std::iterator_traits<I>::value_type, Compare, Allocator>;

    /**
     * Deduction guide for range constructor with capacity.
     */
    template <typename I, typename Compare = std::less<typename std::iterator_traits<I>::value_type>, typename Allocator = std::allocator<typename std::iterator_traits<I>::value_type>>
    vector_set(const typename vector_set<typename std::iterator_traits<I>::value_type, Compare, Allocator>::size_type capacity, I first, I last, const Compare& cmp = Compare(), const Allocator& alloc = Allocator()) -> vector_set<typename std::iterator_traits<I>::value_type, Compare, Allocator>;
}

#endif //KDL_VECTOR_SET_H
