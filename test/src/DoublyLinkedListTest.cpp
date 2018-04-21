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

#include <gtest/gtest.h>

#include "DoublyLinkedList.h"

class Element;

class GetElementLink {
    typename DoublyLinkedList<Element, GetElementLink>::Link& operator()(Element* element) const;
    const typename DoublyLinkedList<Element, GetElementLink>::Link& operator()(const Element* element) const;
};

using ElementList = DoublyLinkedList<Element, GetElementLink>;
using ElementLink = typename ElementList::Link;

class Element {
private:
    bool& m_destructorRan;
    ElementLink m_link;
public:
    Element(bool& destructorRan) :
    m_destructorRan(destructorRan),
    m_link(this) {
        m_destructorRan = false;
    }

    ~Element() {
        m_destructorRan = true;
    }
};


TEST(DoublyLinkedListTest, clearEmptyList) {
    ElementList list;
    list.clear();
}

TEST(DoublyLinkedListTest, clearOneElementList) {
    bool d1 = false;

    Element* e1 = new Element(d1);

    ElementList list;
    list.append(e1, 1);

    list.clear();

    ASSERT_TRUE(d1);
}

TEST(DoublyLinkedListTest, clearTwoElementList) {
    bool d1 = false;
    bool d2 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);

    ElementList list;
    list.append(e1, 1);
    list.append(e2, 1);

    list.clear();

    ASSERT_TRUE(d1);
    ASSERT_TRUE(d2);
}

TEST(DoublyLinkedListTest, clearThreeElementList) {
    bool d1 = false;
    bool d2 = false;
    bool d3 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);
    Element* e3 = new Element(d3);

    ElementList list;
    list.append(e1, 1);
    list.append(e2, 1);
    list.append(e3, 1);

    list.clear();

    ASSERT_TRUE(d1);
    ASSERT_TRUE(d2);
    ASSERT_TRUE(d3);
}
