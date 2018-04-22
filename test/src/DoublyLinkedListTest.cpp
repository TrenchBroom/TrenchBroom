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
public:
    auto& operator()(Element* element) const;
    const auto& operator()(const Element* element) const;
};

using ElementList = DoublyLinkedList<Element, GetElementLink>;
using ElementLink = typename ElementList::Link;

class Element {
private:
    friend class GetElementLink;

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

    Element* next() const {
        return m_link.next();
    }

    Element* previous() const {
        return m_link.previous();
    }
};

auto& GetElementLink::operator()(Element* element) const { return element->m_link; }
const auto& GetElementLink::operator()(const Element* element) const { return element->m_link; }

TEST(DoublyLinkedListTest, clearOnDestroy) {
    bool d1 = false;
    Element* e1 = new Element(d1);

    {
        ElementList list;
        list.append(e1, 1);
    }

    ASSERT_TRUE(d1);
}

TEST(DoublyLinkedListTest, empty) {
    bool d1 = false;
    Element* e1 = new Element(d1);

    ElementList list;
    ASSERT_TRUE(list.empty());
    list.append(e1, 1);
    ASSERT_FALSE(list.empty());
}

TEST(DoublyLinkedListTest, size) {
    bool d1 = false;
    bool d2 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);

    ElementList list;
    ASSERT_EQ(0u, list.size());
    list.append(e1, 1);
    ASSERT_EQ(1u, list.size());
    list.append(e2, 1);
    ASSERT_EQ(2u, list.size());
}

TEST(DoublyLinkedListTest, contains) {
    bool d1 = false;
    bool d2 = false;
    bool d3 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);
    Element* e3 = new Element(d3);

    ElementList list;
    ASSERT_FALSE(list.contains(e1));
    ASSERT_FALSE(list.contains(e2));
    ASSERT_FALSE(list.contains(e3));

    list.append(e1, 1);
    ASSERT_TRUE(list.contains(e1));
    ASSERT_FALSE(list.contains(e2));
    ASSERT_FALSE(list.contains(e3));

    list.append(e2, 1);
    ASSERT_TRUE(list.contains(e1));
    ASSERT_TRUE(list.contains(e2));
    ASSERT_FALSE(list.contains(e3));

    list.append(e3, 1);
    ASSERT_TRUE(list.contains(e1));
    ASSERT_TRUE(list.contains(e2));
    ASSERT_TRUE(list.contains(e3));
}

TEST(DoublyLinkedListTest, appendSingleElement) {
    bool d1 = false;
    bool d2 = false;
    bool d3 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);
    Element* e3 = new Element(d3);

    ElementList list;
    list.append(e1, 1);

    list.append(e2, 1);
    ASSERT_EQ(2u, list.size());
    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e2, e1->previous());
    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e2, e1->previous());

    list.append(e3, 1);
    ASSERT_EQ(3u, list.size());
    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e3, e1->previous());
    ASSERT_EQ(e3, e2->next());
    ASSERT_EQ(e1, e2->previous());
    ASSERT_EQ(e1, e3->next());
    ASSERT_EQ(e2, e3->previous());
}

TEST(DoublyLinkedListTest, appendTwoElements) {
    bool d1 = false;
    bool d2 = false;
    bool d3 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);
    Element* e3 = new Element(d3);

    ElementList source;
    source.append(e2, 1);
    source.append(e3, 1);
    source.release();

    ElementList list;
    list.append(e1, 1);
    list.append(e2, 2);

    ASSERT_EQ(3u, list.size());
    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e3, e1->previous());
    ASSERT_EQ(e3, e2->next());
    ASSERT_EQ(e1, e2->previous());
    ASSERT_EQ(e1, e3->next());
    ASSERT_EQ(e2, e3->previous());
}

TEST(DoublyLinkedListTest, insertOneElementBefore) {
    bool d1 = false;
    bool d2 = false;
    bool d3 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);
    Element* e3 = new Element(d3);

    ElementList list;
    list.append(e3, 1);

    list.insertBefore(e3, e2, 1);
    ASSERT_EQ(2u, list.size());
    ASSERT_EQ(e3, e2->next());
    ASSERT_EQ(e3, e2->previous());
    ASSERT_EQ(e2, e3->next());
    ASSERT_EQ(e2, e3->previous());

    list.insertBefore(e2, e1, 1);
    ASSERT_EQ(3u, list.size());
    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e3, e1->previous());
    ASSERT_EQ(e3, e2->next());
    ASSERT_EQ(e1, e2->previous());
    ASSERT_EQ(e1, e3->next());
    ASSERT_EQ(e2, e3->previous());
}

TEST(DoublyLinkedListTest, insertTwoElementsBefore) {
    bool d1 = false;
    bool d2 = false;
    bool d3 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);
    Element* e3 = new Element(d3);

    ElementList source;
    source.append(e1, 1);
    source.append(e2, 1);
    source.release();

    ElementList list;
    list.append(e3, 1);

    list.insertBefore(e3, e1, 2);
    ASSERT_EQ(3u, list.size());
    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e3, e1->previous());
    ASSERT_EQ(e3, e2->next());
    ASSERT_EQ(e1, e2->previous());
    ASSERT_EQ(e1, e3->next());
    ASSERT_EQ(e2, e3->previous());
}

TEST(DoublyLinkedListTest, insertOneElementAfter) {
    bool d1 = false;
    bool d2 = false;
    bool d3 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);
    Element* e3 = new Element(d3);

    ElementList list;
    list.append(e1, 1);

    list.insertAfter(e1, e2, 1);
    ASSERT_EQ(2u, list.size());
    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e2, e1->previous());
    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e2, e1->previous());

    list.insertAfter(e2, e3, 1);
    ASSERT_EQ(3u, list.size());
    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e3, e1->previous());
    ASSERT_EQ(e3, e2->next());
    ASSERT_EQ(e1, e2->previous());
    ASSERT_EQ(e1, e3->next());
    ASSERT_EQ(e2, e3->previous());
}

TEST(DoublyLinkedListTest, insertTwoElementsAfter) {
    bool d1 = false;
    bool d2 = false;
    bool d3 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);
    Element* e3 = new Element(d3);

    ElementList source;
    source.append(e2, 1);
    source.append(e3, 1);
    source.release();

    ElementList list;
    list.append(e1, 1);

    list.insertAfter(e1, e2, 2);
    ASSERT_EQ(3u, list.size());
    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e3, e1->previous());
    ASSERT_EQ(e3, e2->next());
    ASSERT_EQ(e1, e2->previous());
    ASSERT_EQ(e1, e3->next());
    ASSERT_EQ(e2, e3->previous());
}

TEST(DoublyLinkedListTest, replaceSingleElementInOneElementList) {
    bool de1 = false;
    bool dr1 = false;

    Element* e1 = new Element(de1);
    Element* r1 = new Element(dr1);

    ElementList list;
    list.append(e1, 1);

    list.replace(e1, e1, 1, r1, 1);

    ASSERT_EQ(1u, list.size());
    ASSERT_FALSE(list.contains(e1));
    ASSERT_TRUE(list.contains(r1));

    ASSERT_EQ(e1, e1->next());
    ASSERT_EQ(e1, e1->previous());

    ASSERT_EQ(r1, r1->next());
    ASSERT_EQ(r1, r1->previous());

    ASSERT_FALSE(de1);
    ASSERT_FALSE(dr1);

    delete e1;
}

TEST(DoublyLinkedListTest, replaceFirstElementInTwoElementList) {
    bool de1 = false;
    bool de2 = false;
    bool dr1 = false;

    Element* e1 = new Element(de1);
    Element* e2 = new Element(de2);
    Element* r1 = new Element(dr1);

    ElementList list;
    list.append(e1, 1);
    list.append(e2, 1);

    list.replace(e1, e1, 1, r1, 1);

    ASSERT_EQ(2u, list.size());
    ASSERT_FALSE(list.contains(e1));
    ASSERT_TRUE(list.contains(e2));
    ASSERT_TRUE(list.contains(r1));

    ASSERT_EQ(e1, e1->next());
    ASSERT_EQ(e1, e1->previous());

    ASSERT_EQ(r1, e2->next());
    ASSERT_EQ(r1, e2->previous());
    ASSERT_EQ(e2, r1->next());
    ASSERT_EQ(e2, r1->previous());

    ASSERT_FALSE(de1);
    ASSERT_FALSE(de2);
    ASSERT_FALSE(dr1);

    delete e1;
}

TEST(DoublyLinkedListTest, replaceLastElementInTwoElementList) {
    bool de1 = false;
    bool de2 = false;
    bool dr1 = false;

    Element* e1 = new Element(de1);
    Element* e2 = new Element(de2);
    Element* r1 = new Element(dr1);

    ElementList list;
    list.append(e1, 1);
    list.append(e2, 1);

    list.replace(e2, e2, 1, r1, 1);

    ASSERT_EQ(2u, list.size());
    ASSERT_TRUE(list.contains(e1));
    ASSERT_FALSE(list.contains(e2));
    ASSERT_TRUE(list.contains(r1));

    ASSERT_EQ(e2, e2->next());
    ASSERT_EQ(e2, e2->previous());

    ASSERT_EQ(r1, e1->next());
    ASSERT_EQ(r1, e1->previous());
    ASSERT_EQ(e1, r1->next());
    ASSERT_EQ(e1, r1->previous());

    ASSERT_FALSE(de1);
    ASSERT_FALSE(de2);
    ASSERT_FALSE(dr1);

    delete e2;
}

TEST(DoublyLinkedListTest, replaceTwoElementsByOneElementInFourElementList) {
    bool de1 = false;
    bool de2 = false;
    bool de3 = false;
    bool de4 = false;
    bool dr1 = false;

    Element* e1 = new Element(de1);
    Element* e2 = new Element(de2);
    Element* e3 = new Element(de3);
    Element* e4 = new Element(de4);
    Element* r1 = new Element(dr1);

    ElementList list;
    list.append(e1, 1);
    list.append(e2, 1);
    list.append(e3, 1);
    list.append(e4, 1);

    list.replace(e2, e3, 2, r1, 1);

    ASSERT_EQ(3u, list.size());
    ASSERT_TRUE(list.contains(e1));
    ASSERT_FALSE(list.contains(e2));
    ASSERT_FALSE(list.contains(e3));
    ASSERT_TRUE(list.contains(e4));
    ASSERT_TRUE(list.contains(r1));

    ASSERT_EQ(e3, e2->next());
    ASSERT_EQ(e3, e2->previous());
    ASSERT_EQ(e2, e3->next());
    ASSERT_EQ(e2, e3->previous());

    ASSERT_EQ(r1, e1->next());
    ASSERT_EQ(e4, e1->previous());
    ASSERT_EQ(e4, r1->next());
    ASSERT_EQ(e1, r1->previous());
    ASSERT_EQ(e1, e4->next());
    ASSERT_EQ(r1, e4->previous());

    ASSERT_FALSE(de1);
    ASSERT_FALSE(de2);
    ASSERT_FALSE(de3);
    ASSERT_FALSE(de4);
    ASSERT_FALSE(dr1);

    delete e2;
    delete e3;
}

TEST(DoublyLinkedListTest, replaceTwoElementsByTwoElementsInFourElementList) {
    bool de1 = false;
    bool de2 = false;
    bool de3 = false;
    bool de4 = false;
    bool dr1 = false;
    bool dr2 = false;

    Element* e1 = new Element(de1);
    Element* e2 = new Element(de2);
    Element* e3 = new Element(de3);
    Element* e4 = new Element(de4);
    Element* r1 = new Element(dr1);
    Element* r2 = new Element(dr2);

    ElementList repl;
    repl.append(r1, 1);
    repl.append(r2, 1);
    repl.release();

    ElementList list;
    list.append(e1, 1);
    list.append(e2, 1);
    list.append(e3, 1);
    list.append(e4, 1);

    list.replace(e2, e3, 2, r1, 2);

    ASSERT_EQ(4u, list.size());
    ASSERT_TRUE(list.contains(e1));
    ASSERT_FALSE(list.contains(e2));
    ASSERT_FALSE(list.contains(e3));
    ASSERT_TRUE(list.contains(e4));
    ASSERT_TRUE(list.contains(r1));
    ASSERT_TRUE(list.contains(r2));

    ASSERT_EQ(e3, e2->next());
    ASSERT_EQ(e3, e2->previous());
    ASSERT_EQ(e2, e3->next());
    ASSERT_EQ(e2, e3->previous());

    ASSERT_EQ(r1, e1->next());
    ASSERT_EQ(e4, e1->previous());
    ASSERT_EQ(r2, r1->next());
    ASSERT_EQ(e1, r1->previous());
    ASSERT_EQ(e4, r2->next());
    ASSERT_EQ(r1, r2->previous());
    ASSERT_EQ(e1, e4->next());
    ASSERT_EQ(r2, e4->previous());

    ASSERT_FALSE(de1);
    ASSERT_FALSE(de2);
    ASSERT_FALSE(de3);
    ASSERT_FALSE(de4);
    ASSERT_FALSE(dr1);
    ASSERT_FALSE(dr2);

    delete e2;
    delete e3;
}

TEST(DoublyLinkedListTest, removeFromOneElementList) {
    bool d1 = false;

    Element* e1 = new Element(d1);

    ElementList list;
    list.append(e1, 1);

    list.remove(e1, e1, 1);

    ASSERT_TRUE(list.empty());
    ASSERT_EQ(0u, list.size());
    ASSERT_FALSE(list.contains(e1));
    ASSERT_FALSE(d1);

    ASSERT_EQ(e1, e1->next());
    ASSERT_EQ(e1, e1->previous());

    delete e1;
}

TEST(DoublyLinkedListTest, removeFirstFromTwoElementList) {
    bool d1 = false;
    bool d2 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);

    ElementList list;
    list.append(e1, 1);
    list.append(e2, 1);

    list.remove(e1, e1, 1);

    ASSERT_FALSE(list.empty());
    ASSERT_EQ(1u, list.size());
    ASSERT_FALSE(list.contains(e1));
    ASSERT_TRUE(list.contains(e2));

    ASSERT_EQ(e2, e2->next());
    ASSERT_EQ(e2, e2->previous());
    ASSERT_EQ(e1, e1->next());
    ASSERT_EQ(e1, e1->previous());

    ASSERT_FALSE(d1);

    delete e1;
}

TEST(DoublyLinkedListTest, removeSecondFromTwoElementList) {
    bool d1 = false;
    bool d2 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);

    ElementList list;
    list.append(e1, 1);
    list.append(e2, 1);

    list.remove(e2, e2, 1);

    ASSERT_FALSE(list.empty());
    ASSERT_EQ(1u, list.size());
    ASSERT_TRUE(list.contains(e1));
    ASSERT_FALSE(list.contains(e2));

    ASSERT_EQ(e2, e2->next());
    ASSERT_EQ(e2, e2->previous());
    ASSERT_EQ(e1, e1->next());
    ASSERT_EQ(e1, e1->previous());

    ASSERT_FALSE(d2);

    delete e2;
}

TEST(DoublyLinkedListTest, removeAllFromTwoElementList) {
    bool d1 = false;
    bool d2 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);

    ElementList list;
    list.append(e1, 1);
    list.append(e2, 1);

    list.remove(e1, e2, 2);

    ASSERT_TRUE(list.empty());
    ASSERT_FALSE(list.contains(e1));
    ASSERT_FALSE(list.contains(e2));

    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e2, e1->previous());
    ASSERT_EQ(e1, e2->next());
    ASSERT_EQ(e1, e2->previous());

    ASSERT_FALSE(d1);
    ASSERT_FALSE(d2);

    delete e1;
    delete e2;
}

TEST(DoublyLinkedListTest, removeMiddleFromThreeElementList) {
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

    list.remove(e2, e2, 1);

    ASSERT_FALSE(list.empty());
    ASSERT_TRUE(list.contains(e1));
    ASSERT_FALSE(list.contains(e2));
    ASSERT_TRUE(list.contains(e3));

    ASSERT_EQ(e3, e1->next());
    ASSERT_EQ(e3, e1->previous());
    ASSERT_EQ(e2, e2->next());
    ASSERT_EQ(e2, e2->previous());
    ASSERT_EQ(e1, e3->next());
    ASSERT_EQ(e1, e3->previous());

    ASSERT_FALSE(d2);

    delete e2;
}

TEST(DoublyLinkedListTest, removeTwoElements) {
    bool d1 = false;
    bool d2 = false;
    bool d3 = false;
    bool d4 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);
    Element* e3 = new Element(d3);
    Element* e4 = new Element(d4);

    ElementList list;
    list.append(e1, 1);
    list.append(e2, 1);
    list.append(e3, 1);
    list.append(e4, 1);

    list.remove(e2, e3, 2);

    ASSERT_FALSE(list.empty());
    ASSERT_TRUE(list.contains(e1));
    ASSERT_FALSE(list.contains(e2));
    ASSERT_FALSE(list.contains(e3));
    ASSERT_TRUE(list.contains(e4));

    ASSERT_EQ(e4, e1->next());
    ASSERT_EQ(e4, e1->previous());
    ASSERT_EQ(e3, e2->next());
    ASSERT_EQ(e3, e2->previous());
    ASSERT_EQ(e2, e3->next());
    ASSERT_EQ(e2, e3->previous());
    ASSERT_EQ(e1, e4->next());
    ASSERT_EQ(e1, e4->previous());

    ASSERT_FALSE(d2);
    ASSERT_FALSE(d3);

    delete e2;
    delete e3;
}

TEST(DoublyLinkedListTest, reverseEmptyList) {
    ElementList list;
    list.reverse();
    ASSERT_TRUE(list.empty());
}

TEST(DoublyLinkedListTest, reverseOneElementList) {
    bool d1 = false;

    Element* e1 = new Element(d1);

    ElementList list;
    list.append(e1, 1);

    list.reverse();

    ASSERT_EQ(1u, list.size());
    ASSERT_EQ(e1, e1->next());
    ASSERT_EQ(e1, e1->previous());

    ASSERT_FALSE(d1);
}

TEST(DoublyLinkedListTest, reverseTwoElementList) {
    bool d1 = false;
    bool d2 = false;

    Element* e1 = new Element(d1);
    Element* e2 = new Element(d2);

    ElementList list;
    list.append(e1, 1);
    list.append(e2, 1);

    list.reverse();

    ASSERT_EQ(2u, list.size());
    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e2, e1->previous());
    ASSERT_EQ(e1, e2->next());
    ASSERT_EQ(e1, e2->previous());

    ASSERT_FALSE(d1);
    ASSERT_FALSE(d2);
}

TEST(DoublyLinkedListTest, reverseThreeElementList) {
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

    ASSERT_EQ(e2, e1->next());
    ASSERT_EQ(e3, e1->previous());
    ASSERT_EQ(e3, e2->next());
    ASSERT_EQ(e1, e2->previous());
    ASSERT_EQ(e1, e3->next());
    ASSERT_EQ(e2, e3->previous());

    list.reverse();

    ASSERT_EQ(3u, list.size());
    ASSERT_EQ(e3, e1->next());
    ASSERT_EQ(e2, e1->previous());
    ASSERT_EQ(e1, e2->next());
    ASSERT_EQ(e3, e2->previous());
    ASSERT_EQ(e2, e3->next());
    ASSERT_EQ(e1, e3->previous());

    ASSERT_FALSE(d1);
    ASSERT_FALSE(d2);
    ASSERT_FALSE(d3);
}

TEST(DoublyLinkedListTest, release) {
    bool d1 = false;

    Element* e1 = new Element(d1);

    {
        ElementList list;
        list.append(e1, 1);
        list.release();
    }

    ASSERT_FALSE(d1);
    delete e1;
}

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
