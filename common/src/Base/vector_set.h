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

#ifndef TRENCHBROOM_VECTOR_SET_H
#define TRENCHBROOM_VECTOR_SET_H

#include <algorithm> // for std::sort, std::unique, std::lower_bound, std::upper_bound
#include <cassert>
#include <functional> // for std::less
#include <iterator> // for std::distance
#include <memory> // for std::allocator
#include <vector>

namespace detail {
    template <typename T, typename Allocator, typename Compare>
    static void sort_unique(std::vector<T, Allocator>& vec, const Compare& cmp) {
        auto eq = [&cmp](const auto& lhs, const auto& rhs) { return !cmp(lhs,rhs) && !cmp(rhs, lhs); };
        std::sort(std::begin(vec), std::end(vec), cmp);
        vec.erase(std::unique(std::begin(vec), std::end(vec), eq), std::end(vec));
    }
}

/**
 * Adapts a collection to the read only interface of std::set. The underlying collection is expected to be sorted and it
 * must not contain any pair of two values which are equivalent according to the comparator used.
 *
 * If the given collection type is a reference, then this adapter will work directly on the underlying collection.
 * Otherwise, the collection will be copied by the constructor.
 *
 * In the former case, it must be ensured that the lifetime of this adapter does not exceed the lifetime of the
 * underlying collection.
 *
 * @tparam C the type of the underlying collection (this can be a reference)
 * @tparam Compare the type of the comparator to use for comparing the elements of the underlying collection
 */
template <typename C, typename Compare = std::less<typename C::value_type>>
class const_set_adapter {
protected:
    C m_data;
    Compare m_cmp;
public:
    using key_type = typename std::remove_reference<C>::type::value_type;
    using value_type = key_type;
    using size_type = typename std::remove_reference<C>::type::size_type;
    using difference_type = typename std::remove_reference<C>::type::difference_type;
    using key_compare = Compare;
    using value_compare = Compare;
    using allocator_type = typename std::remove_reference<C>::type::allocator_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
    using iterator = typename std::remove_reference<C>::type::iterator;
    using const_iterator = typename std::remove_reference<C>::type::const_iterator;
    using reverse_iterator = typename std::remove_reference<C>::type::reverse_iterator;
    using const_reverse_iterator = typename std::remove_reference<C>::type::const_reverse_iterator;
public:
    /**
     * Creates a set adapter using the given collection as its underlying collection. The given collection must
     * already be sorted and it must not contain any pair of two equivalent values according to the given comparator.
     *
     * If the given collection type is a reference, then this adapter will work directly on the underlying collection.
     * Otherwise, the collection will be copied or moved by the constructor.
     *
     * In the former case, it must be ensured that the lifetime of this adapter does not exceed the lifetime of the
     * underlying collection.
     *
     * @param data the collection
     * @param cmp the comparator
     */
    template <typename CC>
    const_set_adapter(CC&& data, const Compare& cmp) :
    m_data(std::forward<CC>(data)),
    m_cmp(cmp) {
        assert(check_invariant());
    }
public:
    /**
     * Returns a copy of the allocator being used by the underlying vector.
     */
    allocator_type get_allocator() const {
        return m_data.get_allocator();
    }
    /**
     * Returns a const iterator to the first element in the set. If the set is empty, the returned iterator will be
     * equal to cend().
     */
    const_iterator begin() const noexcept {
        return cbegin();
    }

    /**
     * Returns a const iterator to the first element in the set. If the set is empty, the returned iterator will be
     * equal to cend().
     */
    const_iterator cbegin() const noexcept {
        return std::cbegin(m_data);
    }

    /**
     * Returns a const iterator to the element following the last element in the set (past-the-end iterator).
     */
    const_iterator end() const noexcept {
        return cend();
    }

    /**
     * Returns a const iterator to the element following the last element in the set (past-the-end iterator).
     */
    const_iterator cend() const noexcept {
        return std::cend(m_data);
    }

    /**
     * Returns a const reverse iterator to the first element of the reversed set. If the container is empty, the
     * returned iterator will be equal to crend().
     */
    const_reverse_iterator rbegin() const noexcept {
        return crbegin();
    }

    /**
     * Returns a const reverse iterator to the first element of the reversed set. If the container is empty, the
     * returned iterator will be equal to crend().
     */
    const_reverse_iterator crbegin() const noexcept {
        return std::cbegin(m_data);
    }

    /**
     * Returns a const reverse iterator to the element following the last element of the reversed container.
     */
    const_reverse_iterator rend() const noexcept {
        return crend();
    }

    /**
     * Returns a const reverse iterator to the element following the last element of the reversed container.
     */
    const_reverse_iterator crend() const noexcept {
        return std::cend(m_data);
    }

    /**
     * Indicates whether this set is empty.
     *
     * @return true if this set is empty and false otherwise
     */
    bool empty() const noexcept {
        return m_data.empty();
    }

    /**
     * Returns the number of values stored in this set.
     */
    size_type size() const noexcept {
        return m_data.size();
    }

    /**
     * Returns the maximum possible number of values that can be stored in this set.
     */
    size_type max_size() const noexcept {
        return m_data.max_size();
    }

    /**
     * Counts the number of values in this set which are equivalent to the given key.
     *
     * @tparam K the key type
     * @param x the key value
     * @return the number of values in this set which are equivalent to the given key
     */
    template <typename K>
    size_type count(const K& x) const {
        return find(x) != end() ? 1u : 0u;
    }

    /**
     * Finds the position of a value that is equivalent to the given key.
     *
     * @tparam K the key type
     * @param x the key
     * @return an iterator to a value that is equivalent to the given key, or end() if no such value can be found
     */
    template <typename K>
    const_iterator find(const K& k) const {
        auto it = lower_bound(k);
        if (it != end() && is_equivalent(k, *it)) {
            return it;
        } else {
            return end();
        }
    }

    /**
     * Returns a range of values which are equivalent to the given key.
     *
     * @tparam K the key type
     * @param x the key
     * @return a maximal range of equivalent values in this set
     */
    template <typename K>
    std::pair<const_iterator,const_iterator> equal_range(const K& x) const {
        return { lower_bound(x), upper_bound(x) };
    }

    /**
     * Returns the position of the greatest value that is less than or equal to the given key, or end() if no such
     * value can be found.
     *
     * @tparam K the key type
     * @param x the key
     * @return the position
     */
    template <typename K>
    const_iterator lower_bound(const K& x) const {
        // FIXME: implement without std::lower_bound
        return std::lower_bound(std::begin(m_data), std::end(m_data), x, m_cmp);
    }

    /**
     * Returns the position of the smallest value that is greater than the given key, or end() if no such value can be
     * found.
     *
     * @tparam K the key type
     * @param x the key
     * @return the position
     */
    template <typename K>
    const_iterator upper_bound(const K& x) const {
        // FIXME: implement without std::upper_bound
        return std::upper_bound(std::begin(m_data), std::end(m_data), x, m_cmp);
    }

    /**
     * Returns a copy of the comparator used to compare the keys. Returns the same comparator as value_comp().
     */
    key_compare key_comp() const {
        return m_cmp;
    }

    /**
     * Returns a copy of the comparator used to compare the values. Returns the same comparator as key_cmp().
     */
    value_compare value_comp() const {
        return m_cmp;
    }

    /**
     * Returns the capacity of the underlying collection.
     */
    size_type capacity() const {
        return m_data.capacity();
    }

    /**
     * Returns a const reference to the underlying collection.
     */
    const C& get_data() const {
        return m_data;
    }
protected:
    bool check_invariant() {
        if (empty() != (size() == 0u)) {
            return false;
        }

        if (size() > 1u) {
            for (auto cur = begin(), last = std::prev(end()); cur != last; ++cur) {
                auto next = std::next(cur);
                if (!m_cmp(*cur, *next)) {
                    return false;
                }
            }
        }

        return true;
    }

    bool is_equivalent(const value_type& lhs, const value_type& rhs) const {
        return !m_cmp(lhs, rhs) && !m_cmp(rhs, lhs);
    }
};

/**
 * Adapts a collection to the full interface of std::set. The underlying collection is expected to be sorted and it
 * must not contain any pair of two values which are equivalent according to the comparator used.
 *
 * If the given collection type is a reference, then this adapter will work directly on the underlying collection.
 * Otherwise, the collection will be copied by the constructor.
 *
 * In the former case, it must be ensured that the lifetime of this adapter does not exceed the lifetime of the
 * underlying collection.
 *
 * @tparam C the type of the underlying collection (this can be a reference)
 * @tparam Compare the type of the comparator to use for comparing the elements of the underlying collection
 */
template <typename C, typename Compare = std::less<typename C::value_type>>
class set_adapter : public const_set_adapter<C, Compare> {
private:
    using base = const_set_adapter<C, Compare>;
protected:
    using base::m_data;
    using base::m_cmp;
public:
public:
    using key_type = typename std::remove_reference<C>::type::value_type;
    using value_type = key_type;
    using size_type = typename std::remove_reference<C>::type::size_type;
    using difference_type = typename std::remove_reference<C>::type::difference_type;
    using key_compare = Compare;
    using value_compare = Compare;
    using allocator_type = typename std::remove_reference<C>::type::allocator_type;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
    using iterator = typename std::remove_reference<C>::type::iterator;
    using const_iterator = typename std::remove_reference<C>::type::const_iterator;
    using reverse_iterator = typename std::remove_reference<C>::type::reverse_iterator;
    using const_reverse_iterator = typename std::remove_reference<C>::type::const_reverse_iterator;
public:
    using base::const_set_adapter;

    using base::get_allocator;

    using base::begin;
    using base::cbegin;

    using base::end;
    using base::cend;

    using base::rbegin;
    using base::crbegin;

    using base::rend;
    using base::crend;

    using base::empty;
    using base::size;
    using base::max_size;

    using base::count;

    using base::find;
    using base::equal_range;
    using base::lower_bound;
    using base::upper_bound;

    using base::key_comp;
    using base::value_comp;

    using base::capacity;
    using base::get_data;
public:
    /**
     * Assigns the values in the given initializer list to this set. The set is cleared and the given values are
     * inserted.
     *
     * @param values the values to insert
     * @return a reference to this set
     */
    set_adapter& operator=(std::initializer_list<value_type> values) {
        m_data = values;
        detail::sort_unique(m_data, m_cmp);
        return *this;
    }

    /**
     * Returns an iterator to the first element in the set. If the set is empty, the returned iterator will be equal to
     * end().
     */
    iterator begin() noexcept {
        return std::begin(m_data);
    }

    /**
     * Returns an iterator to the element following the last element in the set (past-the-end iterator).
     */
    iterator end() noexcept {
        return std::end(m_data);
    }

    /**
     * Returns a reverse iterator to the first element of the reversed set. If the container is empty, the returned
     * iterator will be equal to rend().
     */
    reverse_iterator rbegin() noexcept {
        return std::rbegin(m_data);
    }

    /**
     * Returns a reverse iterator to the element following the last element of the reversed container.
     */
    reverse_iterator rend() noexcept {
        return std::rend(m_data);
    }

    /**
     * Clears all values from this set.
     *
     * Postcondition: empty()
     */
    void clear() {
        m_data.clear();
        assert(check_invariant());
    }

    /**
     * Inserts a copy of the given value into this set. If this set already contains a value that is equivalent to the
     * given value, nothing happens.
     *
     * If the given value could be inserted, then this function returns a pair of an iterator to the inserted value and
     * true, otherwise it returns a pair of an iterator to the value that prevented insertion and false.
     *
     * Postcondition: this set contains a value equivalent to the given value and its size has increased by one if the
     * given value could be inserted
     *
     * @param value the value to insert
     * @return a pair of an iterator and a boolean that indicates whether the value was inserted
     */
    std::pair<iterator, bool> insert(const value_type& value) {
        const auto result = do_insert(value);
        assert(check_invariant());
        return result;
    }

    /**
     * Inserts the given value into this set. If this set already contains a value that is equivalent to the given
     * value, nothing happens. If the value could be inserted, it will have been moved into this set.
     *
     * If the given value could be inserted, then this function returns a pair of an iterator to the inserted value and
     * true, otherwise it returns a pair of an iterator to the value that prevented insertion and false.
     *
     * Postcondition: this set contains a value equivalent to the given value and its size has increased by one if the
     * given value could be inserted
     *
     * @param value the value to insert
     * @return a pair of an iterator and a boolean that indicates whether the value was inserted
     */
    std::pair<iterator, bool> insert(value_type&& value) {
        const auto result = do_insert(std::move(value));
        assert(check_invariant());
        return result;
    }

    /**
     * Inserts a copy of the given value using the given hint to speed up insertion. If the given hint points to the
     * position of the first element that compares greater than the given value, then this function need not perform a
     * search for the insert position. If the given hint does not point to such a value, then the insert position is
     * determined by a binary search.
     *
     * If this set already contains a value that is equivalent to the given value, nothing happens.
     *
     * Postcondition: this set contains a value equivalent to the given value and its size has increased by one if the
     * given value could be inserted

     * @param hint an iterator pointing to the first element of this set that is greater than the given value, or the
     * end iterator if no such value exists
     * @param value the value to insert
     * @return an iterator pointing to the inserted value, or to the value that prevented insertion
     */
    iterator insert(const_iterator hint, const value_type& value) {
        const auto result = do_insert(hint, value).first;
        assert(check_invariant());
        return result;
    }

    /**
     * Inserts the given value using the given hint to speed up insertion. If the given hint points to the position of
     * the first element that compares greater than the given value, then this function need not perform a search for
     * the insert position. If the given hint does not point to such a value, then the insert position is determined by
     * a binary search.
     *
     * If this set already contains a value that is equivalent to the given value, nothing happens. If the value could
     * be inserted, it will have been moved into this set.
     *
     * Postcondition: this set contains a value equivalent to the given value and its size has increased by one if the
     * given value could be inserted

     * @param hint an iterator pointing to the first element of this set that is greater than the given value, or the
     * end iterator if no such value exists
     * @param value the value to insert
     * @return an iterator pointing to the inserted value, or to the value that prevented insertion
     */
    iterator insert(const_iterator hint, value_type&& value) {
        const auto result = do_insert(hint, std::move(value)).first;
        assert(check_invariant());
        return result;
    }

    /**
     * Inserts the values from the given range [first, last) into this set.
     *
     * Postcondition: for each value in the given range, this set contains an equivalent value and its size has
     * increased by one if the by the number of unique values in the given range which were not present in this set

     * @tparam I the iterator type
     * @param first the beginning of the range of values to insert
     * @param last the end of the range of values to insert (past-the-end iterator)
     */
    template <typename I>
    void insert(I first, I last) {
        while (first != last) {
            do_insert(*first);
            ++first;
        }
        assert(check_invariant());
    }


    /**
     * Inserts the values from the given range [first, last) into this set. The given count can be used to avoid costly
     * reallocations of the underlying vector. It should be at least the number of unique items in the given range which
     * are not yet present in this set.
     *
     * Postcondition: for each value in the given range, this set contains an equivalent value and its size has
     * increased by one if the by the number of unique values in the given range which were not present in this set

     * @tparam I the iterator type
     * @param the value by which to increase the underlying vector's capacity before insertion
     * @param first the beginning of the range of values to insert
     * @param last the end of the range of values to insert (past-the-end iterator)
     */
    template <typename I>
    void insert(const size_type count, I first, I last) {
        m_data.reserve(size() + count);
        insert(first, last);
    }

    /**
     * Inserts the values from the given initializer list into this set.
     *
     * Postcondition: for each value in the given list, this set contains an equivalent value and its size has
     * increased by one if the by the number of unique values in the given list which were not present in this set
     *
     * @param values the values to insert
     */
    void insert(std::initializer_list<value_type> values) {
        insert(values.size(), std::begin(values), std::end(values));
    }

    /**
     * Inserts a new value constructed from the given arguments into this set. If this set already contains a value that
     * is equivalent to the newly constructed value, the constructed value is destroyed and otherwise nothing happens.
     *
     * If the constructed value could be inserted, then this function returns a pair of an iterator to the inserted value and
     * true, otherwise it returns a pair of an iterator to the value that prevented insertion and false.
     *
     * The newly constructed value is not emplaced into the underlying vector; rather, it is moved into it.
     *
     * Postcondition: this set contains a value equivalent to the given value and its size has increased by one if the
     * given value could be inserted
     *
     * @param value the value to insert
     * @return a pair of an iterator and a boolean that indicates whether the value was inserted
     */
    template <typename... Args>
    std::pair<iterator,bool> emplace(Args&&... args) {
        return insert(T(std::forward<Args>(args)...));
    }

    /**
     * Inserts a new value constructed from the given arguments into this set, using the given hint to speed up
     * insertion. If the given hint points to the first element that compares greater than the newly constructed value,
     * then this function need not perform a search for the insert position. If the given hint does not point to such a
     * value, then the insert position is determined by a binary search.
     *
     * If this set already contains a value that is equivalent to the newly constructed value, the constructed value is
     * destroyed and otherwise nothing happens.
     *
     * If the constructed value could be inserted, then this function returns a pair of an iterator to the inserted value and
     * true, otherwise it returns a pair of an iterator to the value that prevented insertion and false.
     *
     * The newly constructed value is not emplaced into the underlying vector; rather, it is moved into it.
     *
     * Postcondition: this set contains a value equivalent to the constructed value and its size has increased by one if
     * the value could be inserted
     *
     * @param value the value to insert
     * @return a pair of an iterator and a boolean that indicates whether the value was inserted
     */
    template <typename... Args>
    iterator emplace_hint(const_iterator hint, Args&&... args) {
        return insert(hint, T(std::forward<Args>(args)...));
    }

    /**
     * Erases the value at the given position from this set. If the given position is not valid in this set, then the
     * behavior is undefined.
     *
     * Postcondition: the set does not contain the value that was previously at the given position the size has
     * decreased by one
     *
     * @param pos the position of the value to erase from this set
     */
    void erase(iterator pos) {
        m_data.erase(pos);
        assert(check_invariant());
    }

    /**
     * Erases the value at the given position from this set. If the given position is not valid in this set, then the
     * behavior is undefined.
     *
     * Postcondition: the set does not contain the value that was previously at the given position the size has
     * decreased by one

     * @param pos the position of the value to erase from this set
     */
    void erase(const_iterator pos) {
        m_data.erase(pos);
        assert(check_invariant());
    }

    /**
     * Erases all values in the given range [first, last) from this set. If the given range is not valid for this set,
     * then the behavior is undefined.
     *
     * Postcondition: the set does not contain any of the values in the given range, and the size has decreased by the
     * length of the given range

     * @param first the start of the range to erase
     * @param last the end of the range to erase (past-the-end iterator)
     * @return an iterator to the value following the last erased value
     */
    iterator erase(const_iterator first, const_iterator last) {
        const auto result = m_data.erase(first, last);
        assert(check_invariant());
        return result;
    }

    /**
     * Erases the values from this set which are equivalent to the given key.
     *
     * Postcondition: the set does not contain any value equivalent to the given key, and the size has decreased
     * accordingly
     *
     * @param key the key for the value to erase
     * @return the number of erased values
     */
    size_type erase(const key_type& key) {
        const auto size_before = size();
        auto [begin, end] = equal_range(key);
        erase(begin, end);
        return size_before - size();
    }

    /**
     * Swaps this set with the given set.
     *
     * @param other the set to swap with
     */
    void swap(set_adapter& other) noexcept {
        using std::swap;
        swap(m_data, other.m_data);
        assert(check_invariant());
    }

    /**
     * Finds the position of a value that is equivalent to the given key.
     *
     * @tparam K the key type
     * @param x the key
     * @return an iterator to a value that is equivalent to the given key, or end() if no such value can be found
     */
    template <typename K>
    iterator find(const K& x) {
        auto it = lower_bound(x);
        if (it != end() && is_equivalent(x, *it)) {
            return it;
        } else {
            return end();
        }
    }

    /**
     * Returns a range of values which are equivalent to the given key.
     *
     * @tparam K the key type
     * @param x the key
     * @return a maximal range of equivalent values in this set
     */
    template <typename K>
    std::pair<iterator,iterator> equal_range(const K& x) {
        return { lower_bound(x), upper_bound(x) };
    }

    /**
     * Returns the position of the greatest value that is less than or equal to the given key, or end() if no such
     * value can be found.
     *
     * @tparam K the key type
     * @param x the key
     * @return the position
     */
    template <typename K>
    iterator lower_bound(const K& x) {
        // FIXME: implement without std::lower_bound
        return std::lower_bound(std::begin(m_data), std::end(m_data), x, m_cmp);
    }

    /**
     * Returns the position of the smallest value that is greater than the given key, or end() if no such value can be
     * found.
     *
     * @tparam K the key type
     * @param x the key
     * @return the position
     */
    template <typename K>
    iterator upper_bound(const K& x) {
        // FIXME: implement without std::upper_bound
        return std::upper_bound(std::begin(m_data), std::end(m_data), x, m_cmp);
    }

    /**
     * Returns the underlying vector. Afterwards, this set will be empty.
     */
    C release_data() {
        return C(std::move(m_data));
    }
protected:
    using base::check_invariant;
    using base::is_equivalent;
private:
    template <typename TT>
    std::pair<iterator, bool> do_insert(TT&& value) {
        const auto hint = insert_hint(value);
        return do_insert(hint, std::forward<TT>(value));
    }

    template <typename TT>
    std::pair<iterator, bool> do_insert(const_iterator hint, TT&& value) {
        const auto pos = hint != begin() ? std::prev(hint) : hint;
        if (!empty() && is_equivalent(value, *pos)) {
            const auto offset = std::distance(cbegin(), pos);
            return { std::next(begin(), offset), false };
        } else {
            return { m_data.insert(hint, std::forward<TT>(value)), true };
        }
    }

    const_iterator insert_hint(const_iterator hint, const value_type& value) const {
        // correct the hint if necessary
        if (hint != end() && !m_cmp(value, *hint)) {
            return insert_hint(value);
        }

        if (hint != begin()) {
            const auto pos = std::prev(hint);
            if (!cmp(*pos, value)) {
                return insert_hint(value);
            }
        }

        return hint;
    }

    const_iterator insert_hint(const value_type& value) const {
        return upper_bound(value);
    }
};

namespace detail {
    template <typename C1, typename C2, typename Compare>
    int compare(const const_set_adapter<C1, Compare>& lhs, const const_set_adapter<C2, Compare>& rhs) {
        constexpr auto cmp = Compare();
        auto lhsIt = std::begin(lhs);
        auto lhsEnd = std::end(lhs);
        auto rhsIt = std::begin(rhs);
        auto rhsEnd = std::end(rhs);

        while (lhsIt != lhsEnd && rhsIt != rhsEnd) {
            if (cmp(*lhsIt, *rhsIt)) {
                return -1;
            } else if (cmp(*rhsIt, *lhsIt)) {
                return 1;
            } else {
                ++lhsIt;
                ++rhsIt;
            }
        }

        if (lhsIt == lhsEnd && rhsIt != rhsEnd) {
            return -1;
        } else if (lhsIt != lhsEnd && rhsIt == rhsEnd) {
            return 1;
        } else {
            return 0;
        }
    }
}



template <typename C1, typename C2, typename Compare>
bool operator==(const const_set_adapter<C1, Compare>& lhs, const const_set_adapter<C2, Compare>& rhs) {
    return lhs.size() == rhs.size() && detail::compare(lhs, rhs) == 0;
}

template <typename C1, typename C2, typename Compare>
bool operator!=(const const_set_adapter<C1, Compare>& lhs, const const_set_adapter<C2, Compare>& rhs) {
    return lhs.size() != rhs.size() || detail::compare(lhs, rhs) != 0;
}

template <typename C1, typename C2, typename Compare>
bool operator<(const const_set_adapter<C1, Compare>& lhs, const const_set_adapter<C2, Compare>& rhs) {
    return detail::compare(lhs, rhs) < 0;
}

template <typename C1, typename C2, typename Compare>
bool operator<=(const const_set_adapter<C1, Compare>& lhs, const const_set_adapter<C2, Compare>& rhs) {
    return detail::compare(lhs, rhs) <= 0;
}

template <typename C1, typename C2, typename Compare>
bool operator>(const const_set_adapter<C1, Compare>& lhs, const const_set_adapter<C2, Compare>& rhs) {
    return detail::compare(lhs, rhs) > 0;
}

template <typename C1, typename C2, typename Compare>
bool operator>=(const const_set_adapter<C1, Compare>& lhs, const const_set_adapter<C2, Compare>& rhs) {
    return detail::compare(lhs, rhs) >= 0;
}

template <typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
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
     * @tparam TT the element type of the given vector (should be convertible to T)
     * @tparam AA the allocator type of the given vector
     * @param vec the vector
     * @param cmp the comparator to use, defaults to a newly created instance of Compare
     * @param alloc the allocator to use for the underlying vector, defaults to a newly created instance of Allocator
     */
    template <typename AA>
    vector_set(const std::vector<T,AA>& vec, const Compare& cmp = Compare(), const Allocator& alloc = Allocator()) :
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
private:
    using base::check_invariant;
};

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

/**
 * Adapts the given collection to the read only interface of std::set. The given collection is expected to be sorted and
 * it must not contain any pair of two values which are equivalent according to the comparator used.
 *
 * It must be ensured that the lifetime of returned adapter does not exceed the lifetime of the given collection.
 *
 * @param data the collection
 * @param cmp the comparator
 * @return a set adapter using the given collection as its underlying collection
 */
template <typename C, typename Compare = std::less<typename C::value_type>>
const_set_adapter<const C&, Compare> adapt_vector_set(const C& data, const Compare& cmp = Compare()) {
    return const_set_adapter<const C&, Compare>(data, cmp);
}

/**
 * Adapts the given collection to the full interface of std::set. The given collection is expected to be sorted and it
 * must not contain any pair of two values which are equivalent according to the comparator used.
 *
 * It must be ensured that the lifetime of returned adapter does not exceed the lifetime of the given collection.
 *
 * @param data the collection
 * @param cmp the comparator
 * @return a set adapter using the given collection as its underlying collection
 */
template <typename C, typename Compare = std::less<typename C::value_type>>
set_adapter<C&, Compare> adapt_vector_set(C& data, const Compare& cmp = Compare()) {
    return set_adapter<C&, Compare>(data, cmp);
}

/**
 * Adapts the given collection to the full interface of std::set. The given collection will be sorted and its elements
 * made unique, that is, consecutive elements which are equivalent according to the given comparator will be discarded.
 *
 * It must be ensured that the lifetime of returned adapter does not exceed the lifetime of the given collection.
 *
 * @param data the collection
 * @param cmp the comparator
 * @return a set adapter using the given collection as its underlying collection
 */
template <typename C, typename Compare = std::less<typename C::value_type>>
set_adapter<C&, Compare> create_vector_set(C& data, const Compare& cmp = Compare()) {
    detail::sort_unique(data, cmp);
    return set_adapter<C&, Compare>(data, cmp);
}

#endif //TRENCHBROOM_VECTOR_SET_H
