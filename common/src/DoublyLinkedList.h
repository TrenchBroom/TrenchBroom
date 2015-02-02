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
        Link(Item* item):
        m_previous(item),
        m_next(item) {
            assert(item != NULL);
        }
        
        Item* previous() const {
            return m_previous;
        }
        
        Item* next() const {
            return m_next;
        }
    private:
        void setPrevious(Item* previous) {
            assert(previous != NULL);
            m_previous = previous;
        }
        
        void setNext(Item* next) {
            assert(next != NULL);
            m_next = next;
        }
        
        bool selfLoop(Item* item) const {
            return m_previous == item && m_next == item;
        }
        
        void unlink(Item* item) {
            m_previous = m_next = item;
        }
    };
public:
    template <typename ListType, typename ItemType, typename LinkType>
    class IteratorBase {
    private:
        ListType& m_list;
        size_t m_listVersion;
        ItemType m_next;
        size_t m_index;
        bool m_removed;
    public:
        IteratorBase(ListType& list) :
        m_list(list),
        m_listVersion(m_list.m_version),
        m_next(m_list.m_head),
        m_index(0),
        m_removed(false) {}

        bool hasNext() const {
            assert(m_listVersion == m_list.m_version);
            return m_index < m_list.size();
        }
        
        ItemType next() {
            assert(m_listVersion == m_list.m_version);
            ItemType item = m_next;
            advance();
            m_removed = false;
            return item;
        }
        
        void remove() {
            assert(m_listVersion == m_list.m_version);
            assert(m_index > 0);
            LinkType& link = m_list.getLink(m_next);
            ItemType previous = link.previous();
            m_list.remove(previous);
            m_listVersion = m_list.m_version;
            m_removed = true;
            --m_index;
        }
    private:
        void advance() {
            assert(hasNext());
            LinkType& link = m_list.getLink(m_next);
            m_next = link.next();
            ++m_index;
        }
    };
    
    typedef IteratorBase<DoublyLinkedList<Item>, Item*, Link> Iter;
    typedef IteratorBase<const DoublyLinkedList<Item>, const Item*, const Link> ConstIter;
private:
    friend class ListIterator;
    
    Item* m_head;
    size_t m_size;
    size_t m_version;
public:
    DoublyLinkedList() :
    m_head(NULL),
    m_size(0),
    m_version(0) {}
    
    virtual ~DoublyLinkedList() {}
    
    bool empty() const {
        return m_size == 0;
    }
    
    size_t size() const {
        return m_size;
    }
    
    Iter iterator() {
        return Iter(*this);
    }
    
    ConstIter iterator() const {
        return ConstIter(*this);
    }

    bool contains(Item* item) const {
        assert(item != NULL);
        
        if (m_head == NULL)
            return false;
        
        Item* curItem = m_head;
        do {
            if (curItem == item)
                return true;
            curItem = next(curItem);
        } while (curItem != m_head);
        return false;
    }
    
    void append(Item* item) {
        assert(item != NULL);
        
        if (m_head == NULL) {
            m_head = item;
            ++m_size;
            ++m_version;
        } else {
            insertAfter(getTail(), item);
        }
    }
    
    void insertAfter(Item* pred, Item* item) {
        assert(pred != NULL);
        assert(item != NULL);
        assert(m_head != NULL);
        assert(contains(pred));
        assert(!contains(item));
        
        Link& itemLink = getLink(item);
        assert(itemLink.selfLoop(item));

        Link& predLink = getLink(pred);
        Item* succ = predLink.next();
        Link& succLink = getLink(succ);
        
        predLink.setNext(item);
        itemLink.setPrevious(pred);
        itemLink.setNext(succ);
        succLink.setPrevious(item);
        
        ++m_size;
        ++m_version;
    }
    
    void replace(Item* item, Item* replacement) {
        insertAfter(item, replacement);
        remove(item);
    }
    
    void remove(Item* item) {
        assert(!empty());
        assert(contains(item));
        
        Link& itemLink = getLink(item);
        Item* pred = itemLink.previous();
        Link& predLink = getLink(pred);
        Item* succ = itemLink.next();
        Link& succLink = getLink(succ);
        
        predLink.setNext(succ);
        succLink.setPrevious(pred);
        itemLink.unlink(item);

        if (m_head == item)
            m_head = succ;
        
        --m_size;
        ++m_version;
    }
    
    void deleteAll() {
        if (m_head != NULL) {
            Item* item = m_head;
            while (item != m_head) {
                Item* nextItem = next(item);
                delete item;
                item = nextItem;
            }
            m_head = NULL;
        }
    }
private:
    Item* next(Item* item) const {
        assert(item != NULL);
        Link& link = getLink(item);
        return link.next();
    }
    
    Item* getTail() const {
        if (m_head == NULL)
            return NULL;
        Link& headLink = getLink(m_head);
        return headLink.previous();
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
