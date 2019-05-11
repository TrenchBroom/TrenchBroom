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

#ifndef TrenchBroom_DoublyLinkedList_h
#define TrenchBroom_DoublyLinkedList_h

#include "Ensure.h"

#include <iterator>

template <typename Item, typename GetLink>
class DoublyLinkedList {
public:
    class Link {
    private:
        friend class DoublyLinkedList<Item, GetLink>;
    private:
        Item* m_previous;
        Item* m_next;
    public:
        explicit Link(Item* item):
        m_previous(item),
        m_next(item) {
            ensure(item != nullptr, "item is null");
        }

        Item* previous() const {
            return m_previous;
        }

        Item* next() const {
            return m_next;
        }
    private:
        void setPrevious(Item* previous) {
            ensure(previous != nullptr, "previous is null");
            m_previous = previous;
        }

        void setNext(Item* next) {
            ensure(next != nullptr, "next is null");
            m_next = next;
        }

        void flip() {
            using std::swap;
            swap(m_previous, m_next);
        }
    };
private:
    template <typename ListType, typename ItemType, typename LinkType>
    class iterator_delegate_item;

    template <typename ListType, typename ItemType, typename LinkType>
    class iterator_delegate_end;

    template <typename ListType, typename ItemType, typename LinkType>
    class iterator_delegate_base {
    protected:
        ListType* m_list;
    private:
        size_t m_listVersion;
    protected:
        explicit iterator_delegate_base(ListType& list) :
        m_list(&list),
        m_listVersion(m_list->m_version) {}
    public:
        virtual ~iterator_delegate_base() = default;
    public:
        iterator_delegate_base* clone() const { return doClone(); }

        int compare(const iterator_delegate_base& other) const {
            if (index() < other.index())
                return -1;
            if (index() > other.index())
                return 1;
            return 0;
        }

        void increment()      { assert(checkListVersion()); doIncrement(); }
        size_t index() const  { assert(checkListVersion()); return doGetIndex(); }
        ItemType& reference() { assert(checkListVersion()); return doGetItem(); }
        ItemType item()       { assert(checkListVersion()); return doGetItem(); }
    private:
        bool checkListVersion() const { return m_listVersion == m_list->m_version; }
    private:
        virtual iterator_delegate_base* doClone() const = 0;
        virtual void doIncrement() = 0;
        virtual size_t doGetIndex() const = 0;
        virtual ItemType& doGetItem() = 0;
    };

    template <typename ListType, typename ItemType, typename LinkType>
    class iterator_delegate_item : public iterator_delegate_base<ListType, ItemType, LinkType> {
    private:
        using base = iterator_delegate_base<ListType, ItemType, LinkType>;
        ItemType m_item;
        size_t m_index;
    public:
        iterator_delegate_item(ListType& list, ItemType item, const size_t index) :
        base(list),
        m_item(item),
        m_index(index) {}
    private:
        base* doClone() const override {
            return new iterator_delegate_item(*base::m_list, m_item, m_index);
        }

        void doIncrement() override {
            ++m_index;

            LinkType& link = base::m_list->getLink(m_item);
            m_item = link.next();
        }

        size_t doGetIndex() const override {
            return m_index;
        }

        ItemType& doGetItem() override {
            assert(m_index < base::m_list->size());
            return m_item;
        }
    };

    template <typename ListType, typename ItemType, typename LinkType>
    class iterator_delegate_end : public iterator_delegate_base<ListType, ItemType, LinkType> {
    private:
        using base = iterator_delegate_base<ListType, ItemType, LinkType>;
    public:
        explicit iterator_delegate_end(ListType& list) :
        base(list) {}
    private:
        base* doClone() const override {
            return new iterator_delegate_end(*base::m_list);
        }

        void doIncrement() override {}

        size_t doGetIndex() const override {
            return base::m_list->size();
        }

        ItemType& doGetItem() override {
            static ItemType null = nullptr;
            return null;
        }
    };
public:
    template <typename ListType, typename ItemType, typename LinkType>
    class iterator_base {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = ItemType;
        using pointer = ItemType*;
        using reference = ItemType&;
    private:
        friend class DoublyLinkedList<Item, GetLink>;

        using delegate = iterator_delegate_base<ListType, ItemType, LinkType>;
        delegate* m_delegate;
    public:
        explicit iterator_base(delegate* delegate = nullptr) : m_delegate(delegate) {}
        iterator_base(const iterator_base& other) : m_delegate(other.m_delegate->clone()) {}
        ~iterator_base() { delete m_delegate; }

        static iterator_base begin(ListType& list) {
            return item(list, list.m_head, 0);
        }

        static iterator_base item(ListType& list, Item* item, const size_t index) {
            return iterator_base(new iterator_delegate_item<ListType, ItemType, LinkType>(list, item, index));
        }

        static iterator_base end(ListType& list) {
            return iterator_base(new iterator_delegate_end<ListType, ItemType, LinkType>(list));
        }

        iterator_base& operator=(const iterator_base& other) {
            if (this != &other) {
                delete m_delegate;
                m_delegate = other.m_delegate->clone();
            }
            return *this;
        }

        bool operator<(const iterator_base& other) const  { return compare(other) <  0; }
        bool operator>(const iterator_base& other) const  { return compare(other) >  0; }
        bool operator==(const iterator_base& other) const { return compare(other) == 0; }
        bool operator!=(const iterator_base& other) const { return compare(other) != 0; }

        // prefix increment
        iterator_base& operator++() {
            m_delegate->increment();
            return *this;
        }

        // postfix increment
        const iterator_base operator++(int) {
            iterator_base result(*this);
            m_delegate->increment();
            return result;
        }

        ItemType& operator*() const { return m_delegate->reference(); }
        ItemType operator->() const { return m_delegate->item(); }
    private:
        int compare(const iterator_base& other) const {
            ensure(m_delegate != nullptr, "delegate is null");
            ensure(other.m_delegate != nullptr, "other delegate is null");
            return m_delegate->compare(*other.m_delegate);
        }

        size_t index() const {
            ensure(m_delegate != nullptr, "delegate is null");
            return m_delegate->index();
        }
    };

    using iterator       = iterator_base<      DoublyLinkedList<Item, GetLink>, Item*,       Link>;
    using const_iterator = iterator_base<const DoublyLinkedList<Item, GetLink>, Item*, const Link>;
private:
    friend class ListIterator;

    GetLink m_getLink;
    Item* m_head;
    size_t m_size;
    size_t m_version;
public:
    DoublyLinkedList() :
    m_getLink(),
    m_head(nullptr),
    m_size(0),
    m_version(0) {}

    DoublyLinkedList(DoublyLinkedList&& other) noexcept :
    m_getLink(std::move(other.m_getLink)),
    m_head(other.m_head),
    m_size(other.m_size),
    m_version(other.m_version) {
        other.m_head = nullptr;
        other.m_size = 0;
        other.m_version += 1;
    }

    virtual ~DoublyLinkedList() {
        clear();
    }

    // FIXME iterators may report version change after swapping since they store a reference to their list!
    friend void swap(DoublyLinkedList& first, DoublyLinkedList& second) {
        using std::swap;
        swap(first.m_head, second.m_head);
        swap(first.m_size, second.m_size);
        swap(first.m_version, second.m_version);
    }

    DoublyLinkedList& operator=(DoublyLinkedList&& other) {
        clear();
        m_head = other.m_head;
        m_size = other.m_size;
        m_version = other.m_version;
        other.m_head = nullptr;
        other.m_size = 0;
        other.m_version += 1;
        return *this;
    }
public:
    // Copying is not allowed since this is an intrusive list.
    DoublyLinkedList(const DoublyLinkedList& other) = delete;
    DoublyLinkedList& operator=(const DoublyLinkedList& other) = delete;
public:
    bool empty() const {
        return m_size == 0;
    }

    size_t size() const {
        return m_size;
    }

    iterator begin() {
        return iterator::begin(*this);
    }

    iterator end() {
        return iterator::end(*this);
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_iterator end() const {
        return cend();
    }

    const_iterator cbegin() const {
        return const_iterator::begin(*this);
    }

    const_iterator cend() const {
        return const_iterator::end(*this);
    }

    iterator erase(const iterator& it) {
        Item* item = *it;
        Item* newItem = next(item);
        const size_t index = it.index();

        remove(item);
        return iterator::item(*this, newItem, index);
    }

    bool contains(const Item* item) const {
        ensure(item != nullptr, "item is null");

        if (m_head == nullptr)
            return false;

        Item* curItem = m_head;
        do {
            if (curItem == item)
                return true;
            curItem = next(curItem);
        } while (curItem != m_head);
        return false;
    }

    Item* front() const {
        assert(!empty());
        return m_head;
    }

    Item* back() const {
        assert(!empty());
        return getTail();
    }

    void append(Item* item, const size_t count) {
        ensure(item != nullptr, "item is null");

        if (m_head == nullptr) {
            m_head = item;
            m_size += count;
            ++m_version;
        } else {
            insertAfter(getTail(), item, count);
        }

        assert(check());
    }

    void insertBefore(Item* succ, Item* items, const size_t count) {
        ensure(succ != nullptr, "successor is null");
        ensure(items != nullptr, "items is null");
        ensure(m_head != nullptr, "head is null");
        assert(contains(succ));

        insertAfter(succ->previous(), items, count);
    }

    void insertAfter(Item* pred, Item* items, const size_t count) {
        ensure(pred != nullptr, "predecessor is null");
        ensure(items != nullptr, "items is null");
        ensure(m_head != nullptr, "head is null");
        assert(contains(pred));

        Item* first = items;
        Link& firstLink = getLink(first);
        Item* last = firstLink.previous();
        Link& lastLink = getLink(last);

        Link& predLink = getLink(pred);
        Item* succ = predLink.next();
        Link& succLink = getLink(succ);

        predLink.setNext(first);
        firstLink.setPrevious(pred);
        lastLink.setNext(succ);
        succLink.setPrevious(last);

        m_size += count;
        ++m_version;

        assert(check());
    }

    void replace(Item* from, Item* to, const size_t removeCount, Item* with, const size_t insertCount) {
        insertAfter(to, with, insertCount);
        remove(from, to, removeCount);
    }

    void remove(Item* item) {
        assert(!empty());
        assert(contains(item));
        remove(item, item, 1);
    }

    void remove(Item* from, Item* to, const size_t count) {
        assert(!empty());

        Link& fromLink = getLink(from);
        Link& toLink = getLink(to);

        Item* pred = fromLink.previous();
        Link& predLink = getLink(pred);

        Item* succ = toLink.next();
        Link& succLink = getLink(succ);

        predLink.setNext(succ);
        succLink.setPrevious(pred);

        fromLink.setPrevious(to);
        toLink.setNext(from);

        if (succ == from)
            m_head = nullptr;
        else
            m_head = succ;

        m_size -= count;
        ++m_version;

        assert(check());
    }

    void reverse() {
        if (!empty()) {
            Item* cur = m_head;
            do {
                Link& link = getLink(cur);
                Item* next = link.next();

                link.flip();
                cur = next;
            } while (cur != m_head);
            ++m_version;
        }
        assert(check());
    }

    void release() {
        m_head = nullptr;
        m_size = 0;
    }

    void clear() {
        if (m_head != nullptr) {
            Item* item = m_head;
            do {
                Item* nextItem = next(item);
                delete item;
                item = nextItem;
            } while (item != m_head);
            m_head = nullptr;
            m_size = 0;
            ++m_version;
        }
        assert(check());
    }
private:
    Item* next(Item* item) const {
        ensure(item != nullptr, "item is null");
        Link& link = getLink(item);
        return link.next();
    }

    Item* previous(Item* item) const {
        ensure(item != nullptr, "item is null");
        Link& link = getLink(item);
        return link.previous();
    }

    Item* getTail() const {
        if (m_head == nullptr)
            return nullptr;
        return previous(m_head);
    }

    Link& getLink(Item* item) const {
        ensure(item != nullptr, "item is null");
        return m_getLink(item);
    }

    const Link& getLink(const Item* item) const {
        ensure(item != nullptr, "item is null");
        return m_getLink(item);
    }

    bool check() const {
        return checkLinks() && checkSize();
    }

    bool checkLinks() const {
        if (m_head == nullptr)
            return true;

        const Item* item = m_head;
        do {
            const Link& link = getLink(item);
            const Item* next = link.next();
            if (next == nullptr)
                return false;
            const Link& nextLink = getLink(next);
            if (nextLink.previous() != item)
                return false;
            item = next;
        } while (item != m_head);
        return true;
    }

    bool checkSize() const {
        if (m_head == nullptr)
            return m_size == 0;

        size_t size = 0;
        const Item* item = m_head;
        do {
            const Link& link = getLink(item);
            item = link.next();
            ++size;
        } while (item != m_head);
        return m_size == size;
    }
};

#endif
