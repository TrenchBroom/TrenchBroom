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

#ifndef TrenchBroom_DoublyLinkedList_h
#define TrenchBroom_DoublyLinkedList_h

#include <iterator>

template <typename Item>
class DoublyLinkedList {
public:
    class Link {
    private:
        friend class DoublyLinkedList<Item>;
    private:
        Item* m_previous;
        Item* m_next;
    public:
        Link():
        m_previous(NULL),
        m_next(NULL) {}
        
        Item* previous() const {
            return m_previous;
        }
        
        Item* next() const {
            return m_next;
        }
    private:
        void setPrevious(Item* previous) {
            m_previous = previous;
        }
        
        void setNext(Item* next) {
            m_next = next;
        }
        
        void unlink() {
            m_previous = m_next = NULL;
        }
    };
public:
    template <typename ListType, typename ItemType, typename LinkType>
    class IteratorBase : public std::iterator<std::bidirectional_iterator_tag,ItemType> {
    private:
        ListType* m_list;
        ItemType m_current;
    public:
        IteratorBase() :
        m_list(NULL),
        m_current(NULL) {}
        
        IteratorBase(ListType* list, ItemType current) :
        m_list(list),
        m_current(current) {
            assert(m_list != NULL);
        }
        
        IteratorBase(const IteratorBase& other) :
        m_list(other.m_list),
        m_current(other.m_current) {}
        
        IteratorBase& operator++() {
            forward();
            return *this;
        }
        
        IteratorBase operator++(int) {
            IteratorBase temp(*this);
            forward();
            return temp;
        }
        
        IteratorBase& operator--() {
            backward();
            return *this;
        }
        
        IteratorBase operator--(int) {
            IteratorBase temp(*this);
            backward();
            return temp;
        }

        bool operator==(const IteratorBase& rhs) {
            return m_current == rhs.m_current;
        }

        bool operator!=(const IteratorBase& rhs) {
            return m_current != rhs.m_current;
        }
        
        ItemType operator*() {
            return m_current;
        }
    private:
        void forward() {
            if (m_list != NULL) {
                if (m_current == NULL) {
                    m_current = m_list->m_head;
                } else {
                    LinkType& link = m_list->getLink(m_current);
                    m_current = link.next();
                }
            }
        }
        
        void backward() {
            if (m_list != NULL) {
                if (m_current == NULL) {
                    m_current = m_list->m_tail;
                } else {
                    LinkType& link = m_list->getLink(m_current);
                    m_current = link.previous();
                }
            }
        }
    };
    
    typedef IteratorBase<DoublyLinkedList<Item>, Item*, Link> iterator;
    typedef IteratorBase<const DoublyLinkedList<Item>, const Item*, const Link> const_iterator;
private:
    friend class ListIterator;
    
    Item* m_head;
    Item* m_tail;
    size_t m_size;
public:
    DoublyLinkedList() :
    m_head(NULL),
    m_tail(NULL),
    m_size(0) {}
    
    virtual ~DoublyLinkedList() {}
    
    bool empty() const {
        return m_size == 0;
    }
    
    size_t size() const {
        return m_size;
    }
    
    iterator begin() {
        return iterator(this, m_head);
    }
    
    iterator end() {
        return iterator(this, NULL);
    }
    
    const_iterator begin() const {
        return const_iterator(this, m_head);
    }
    
    const_iterator end() const {
        return const_iterator(this, NULL);
    }

    bool contains(Item* item) const {
        assert(item != NULL);
        
        Item* curItem = m_head;
        while (curItem != NULL) {
            if (curItem == item)
                return true;
            curItem = next(curItem);
        }
        return false;
    }
    
    void append(Item* item) {
        assert(item != NULL);
        assert(!contains(item));
        
        Link& itemLink = getLink(item);
        if (m_head == NULL) {
            m_head = m_tail = item;
        } else {
            Link& tailLink = getLink(m_tail);
            itemLink.setPrevious(m_tail);
            tailLink.setNext(item);
            m_tail = item;
        }
        
        ++m_size;
    }
    
    void remove(Item* item) {
        assert(!empty());
        assert(contains(item));
        
        Link& itemLink = getLink(item);
        
        if (m_head == item) {
            m_head = itemLink.next();
        } else {
            Item* pred = itemLink.previous();
            Link& predLink = getLink(pred);
            predLink.next = itemLink.next;
        }
        
        if (m_tail == item) {
            m_tail = itemLink.previous;
        } else {
            Item* succ = itemLink.next();
            Link& succLink = getLink(succ);
            succLink.previous = itemLink.previous;
        }
        
        itemLink.unlink(item);
        --m_size;
    }
private:
    Item* next(Item* item) const {
        assert(item != NULL);
        Link& link = getLink(item);
        return link.next();
    }
    
    Link& getLink(Item* item) const {
        assert(item != NULL);
        return doGetLink(item);
    }
    
    const Link& getLink(const Item* item) const {
        assert(item != NULL);
        return doGetLink(item);
    }
    
    virtual Link& doGetLink(Item* item) const = 0;
    virtual const Link& doGetLink(const Item* item) const = 0;
};

#endif
