/*
 Copyright (C) 2019 Kristian Duske

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

#ifndef TRENCHBROOM_INTRUSIVE_CIRCULAR_LIST_H
#define TRENCHBROOM_INTRUSIVE_CIRCULAR_LIST_H

#include <cstddef>

template <typename T>
class intrusive_circular_link {
    static_assert(!std::is_pointer<T>::value, "intrusive lists do not accept pointer arguments");
    template <typename, typename> friend class intrusive_circular_list;
private:
    T* m_next;
    T* m_previous;
public:
    explicit intrusive_circular_link(T* item) :
    m_next(item),
    m_previous(item) {}

    T* previous() const {
        return m_previous;
    }

    T* next() const {
        return m_next;
    }
private:
    void set_previous(T* previous) {
        m_previous = previous;
    }

    void set_next(T* next) {
        m_next = next;
    }

    void flip() {
        using std::swap;
        swap(m_next, m_previous);
    }
};

template <typename T, typename GetLink>
class intrusive_circular_list {
    static_assert(!std::is_pointer<T>::value, "intrusive lists do not accept pointer arguments");
public:
    using item = T;
    using get_link_info = GetLink;
    using link_info = intrusive_circular_link<T>;
private:
    T* m_head;
    std::size_t m_size;
public:
    /**
     * Creates a new empty list.
     */
    intrusive_circular_list() :
    m_head(nullptr),
    m_size(0u) {}

    /**
     * Destroys this list and its items.
     */
    ~intrusive_circular_list() {
        clear();
    }

    // since items can belong to at most one list, copy is not allowed
    intrusive_circular_list(intrusive_circular_list&) = delete;
    intrusive_circular_list& operator=(intrusive_circular_list&) = delete;

    // move constructor
    intrusive_circular_list(intrusive_circular_list&& other) noexcept {
        m_head = other.m_head;
        m_size = other.m_size;
        other.release();
    }

    // move assignment, cannot be noexcept because we might delete our items, and delete might throw
    intrusive_circular_list& operator=(intrusive_circular_list&& other) {
        clear();
        m_head = other.m_head;
        m_size = other.m_size;
        other.release();
    }

    /**
     * Returns true if this list is empty and false otherwise.
     */
    bool empty() const {
        return size() == 0u;
    }

    /**
     * Returns the number of items stored in this list.
     */
    std::size_t size() const {
        return m_size;
    }

    /**
     * Returns the first element in this list or null if this list is empty;
     */
    T* front() const {
        return m_head;
    }

    /**
     * Returns the last element in this list or null if this list is empty;
     */
    T* back() const {
        if (empty()) {
            return nullptr;
        } else {
            const auto get_link = GetLink();
            return get_link(m_head).previous();
        }
    }

    /**
     * Adds the given item to this list. The item's link must be a self loop.
     *
     * @param item the items to add, must not be null
     */
    void push_back(T* item) {
        assert(item != nullptr);
        assert(check_invariant());

        if (empty()) {
            m_head = item;
            m_size = 1u;
        } else {
            const auto get_link = GetLink();

            auto list_head = m_head;
            auto& list_head_link = get_link(list_head);

            auto list_tail = list_head_link.previous();
            auto& list_tail_link = get_link(list_tail);

            auto& item_link = get_link(item);

            list_head_link.set_previous(item);
            list_tail_link.set_next(item);

            item_link.set_previous(list_tail);
            item_link.set_next(list_head);

            ++m_size;
        }

        assert(check_invariant());
    }

    /**
     * Creates a new instance of T and adds it to this list.
     *
     * @tparam Args the types of the arguments to forward to T's constructor
     * @param args the arguments to forward to T's constructor
     * @return a pointer to the newly created instance of T
     */
    template <typename... Args>
    T* emplace_back(Args&&... args) {
        T* item = new T(std::forward<Args>(args)...);
        push_back(item);
        return item;
    }

    /**
     * Appends the items of the given list to the end of this list. Afterwards, the given list will be empty.
     *
     * @param list the list to append to this list
     */
    void append(intrusive_circular_list& list) {
        insert_after(back(), list);
    }

    /**
     * Inserts all items from the given list before the given item of this list. Afterwards, the given list will be
     * empty.
     *
     * @param position the item before which the items of the given list should be inserted
     * @param list the list to insert into this list
     */
    void insert_before(T* position, intrusive_circular_list& list) {
        splice_before(position, list, list.front(), list.back(), list.size());
    }

    /**
     * Inserts all items from the given list after the given item of this list. Afterwards, the given list will be
     * empty.
     *
     * @param position the item after which the items of the given list should be inserted
     * @param list the list to insert into this list
     */
    void insert_after(T* position, intrusive_circular_list& list) {
        splice_after(position, list, list.front(), list.back(), list.size());
    }

    /**
     * Moves items from the given list into this list before the given item of this list. The items will be removed
     * from the given list and inserted before the given item of this list. If the given position is null, then this list
     * must be empty and the given items will be added to it
     *
     * @param position the item before which the items of the given list should be inserted, or null if this list is empty
     * @param list the list which the given items should be moved from
     * @param first the first item to move into this list
     * @param last the last item to move into this list
     * @param count the number of items to move into this list
     */
    void splice_before(T* position, intrusive_circular_list& list, T* first, T* last, std::size_t count) {
        assert(position != nullptr || empty());
        if (empty()) {
            splice_after(position, list, first, last, count);
        } else {
            const auto get_link = GetLink();
            auto& position_link = get_link(position);
            auto previous = position_link.previous();
            splice_after(previous, list, first, last, count);
        }
    }


    /**
     * Moves items from the given list into this list after the given item of this list. The items will be removed
     * from the given list and inserted after the given item of this list. If the given position is null, then this list
     * must be empty and the given items will be added to it.
     *
     * @param position the item after which the items of the given list should be inserted, or null if this list is empty
     * @param list the list which the given items should be moved from
     * @param first the first item to moved into this list
     * @param last the last item to moved into this list
     * @param count the number of items to moved into this list
     */
    void splice_after(T* position, intrusive_circular_list& list, T* first, T* last, std::size_t count) {
        assert(position != nullptr || empty());
        assert(first != nullptr);
        assert(last != nullptr);
        assert(check_invariant());

        list.release(first, last, count);

        if (empty()) {
            m_head = first;
            m_size = count;
        } else {
            const auto get_link = GetLink();

            auto& first_link = get_link(first);
            auto& last_link = get_link(last);

            auto previous = position;
            auto& previous_link = get_link(previous);
            auto next = previous_link.next();
            auto& next_link = get_link(next);

            previous_link.set_next(first);
            next_link.set_previous(last);

            first_link.set_previous(previous);
            last_link.set_next(next);

            m_size += count;
        }

        assert(check_invariant());
    }

    /**
     * Moves items from the given list into this list, replacing the given items of this list.
     *
     * @param replace_first the first item of this list to be replaced, must not be null
     * @param replace_last the last item of this list to be replaced, must not be null
     * @param replace_count the number of items of this list to replace
     * @param list the list which the given items should be moved from
     * @param move_first the first item to move into this list, must not be null
     * @param move_last the last item to move into this list, must not be null
     * @param move_count the number of items to move into this list
     */
    void splice_replace(
        T* replace_first, T* replace_last, const std::size_t replace_count,
        intrusive_circular_list& list, T* move_first, T* move_last, const std::size_t move_count) {

        assert(replace_first != nullptr);
        assert(replace_last != nullptr);
        assert(replace_count > 0u);
        assert(replace_count <= size());
        assert(move_first != nullptr);
        assert(move_last != nullptr);
        assert(move_count > 0u);
        assert(move_count <= list.size());

        remove(replace_first, replace_last, replace_count);

        // m_head is now either null or it points to the predecessor of replace_first
        splice_after(m_head, list, move_first, move_last, move_count);
    }

    /**
     * Removes the given items from this list and deletes them.
     *
     * @param first the first item to remove
     * @param last the liast item to remove
     * @param count the number of items to remove
     */
    void remove(T* first, T* last, std::size_t count) {
        assert(first != nullptr);
        assert(last != nullptr);
        assert(count > 0u);
        assert(count <= size());
        assert(check_invariant());

        release(first, last, count);

        const auto get_link = GetLink();

        auto cur = first;
        const auto last_next = get_link(last).next();
        do {
            auto& cur_link = get_link(cur);
            auto next = cur_link.next();

            delete cur;
            cur = next;
        } while (cur != last_next);

        assert(check_invariant());
    }

    /**
     * Removes the given items from this list without deleting them. If the list is not empty after
     * removal of the given nodes, then the predecessor of the given first node becomes the head of this list.
     *
     * @param first the first item to remove
     * @param last the last item to remove
     * @param count the number of items to remove
     */
    void release(T* first, T* last, std::size_t count) {
        assert(first != nullptr);
        assert(last != nullptr);
        assert(count > 0u);
        assert(count <= size());
        assert(check_invariant());

        if (count == size()) {
            m_head = nullptr;
            m_size = 0u;
        } else {
            const auto get_link = GetLink();

            auto& first_link = get_link(first);
            auto& last_link = get_link(last);

            auto previous = first_link.previous();
            auto next = last_link.next();

            auto& previous_link = get_link(previous);
            auto& next_link = get_link(next);

            first_link.set_previous(last);
            last_link.set_next(first);

            previous_link.set_next(next);
            next_link.set_previous(previous);

            m_size -= count;
            m_head = previous;
        }

        assert(check_invariant());
    }

    /**
     * Clears this list and deletes all items.
     */
    void clear() {
        if (!empty()) {
            remove(front(), back(), size());
        }
    }

    /**
     * Clears this list without deleting its items.
     */
    void release() {
        m_head = nullptr;
        m_size = 0u;
    }
private:
    bool check_invariant() {
        if (m_head == nullptr) {
            return m_size == 0u;
        } else {
            const auto get_link = GetLink();

            std::size_t count = 0u;
            auto cur = m_head;
            do {
                auto next = get_link(cur).next();
                if (get_link(next).previous() != cur) {
                    return false;
                }
                ++count;
                cur = next;
            } while (cur != m_head);
            return m_size == count;
        }
    }
};



#endif //TRENCHBROOM_INTRUSIVE_CIRCULAR_LIST_H
