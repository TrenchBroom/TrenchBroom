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

#include <gtest/gtest.h>

#include "intrusive_circular_list.h"

#include <vector>

class element;
using element_link = intrusive_circular_link<element>;

class element {
private:
    friend class get_link;
    element_link m_link;
public:
    element() :
        m_link(this) {
    }

    virtual ~element() = default;

    const element& next() const {
        return *m_link.next();
    }

    const element& previous() const {
        return *m_link.previous();
    }
};

class delete_tracking_element : public element {
private:
    bool& m_deleted;
public:
    explicit delete_tracking_element(bool& deleted) :
    m_deleted(deleted) {
        m_deleted = false;
    }

    ~delete_tracking_element() override {
        m_deleted = true;
    }
};

class get_link {
public:
    element_link& operator()(element* element) const {
        return element->m_link;
    }
    const element_link& operator()(const element* element) const {
        return element->m_link;
    }
};

using list = intrusive_circular_list<element, get_link>;

template <typename Item>
void assertLinks(Item* head, const std::vector<Item*>& items) {
    ASSERT_TRUE((head == nullptr) == (items.empty()));

    if (head != nullptr) {
        const auto get_link = ::get_link();

        // find the front of the list
        auto list_first = head;
        while (list_first != items.front()) {
            list_first = get_link(list_first).next();
            if (list_first == head) {
                FAIL() << "list head is not an item";
            }
        }
        auto list_cur = list_first;
        auto list_previous = get_link(list_cur).previous();

        for (std::size_t i = 0u; i < items.size(); ++i) {
            auto items_cur = items[i];
            ASSERT_EQ(list_cur, items_cur);
            ASSERT_EQ(list_cur, get_link(list_previous).next());

            list_previous = list_cur;
            list_cur = get_link(list_cur).next();
        }

        ASSERT_EQ(list_first, list_cur);
    }
}

template <typename List>
void assertList(const std::vector<typename List::item*>& expected, const List& actual) {
    ASSERT_EQ(actual.empty(), expected.empty());
    ASSERT_EQ(actual.size(), expected.size());

    if (!actual.empty()) {
        assertLinks(actual.front(), expected);
    }
}

TEST(intrusive_circular_list_test, constructor_default) {
    assertList({}, list());
}

TEST(intrusive_circular_list_test, constructor_initializer_list) {
    assertList({}, list({}));

    auto* e1 = new element();
    assertList({ e1 }, list({ e1 }));

    auto* e2 = new element();
    auto* e3 = new element();
    assertList({ e2, e3 }, list({ e2, e3 }));
}

TEST(intrusive_circular_list_test, destructor_cleanup) {
    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    {
        auto* t1 = new delete_tracking_element(t1_deleted);
        auto* t2 = new delete_tracking_element(t2_deleted);
        auto* t3 = new delete_tracking_element(t3_deleted);

        list l;
        l.push_back(t1);
        l.push_back(t2);
        l.push_back(t3);

        // l falls out of scope and destroys the elements
    }

    ASSERT_TRUE(t1_deleted);
    ASSERT_TRUE(t2_deleted);
    ASSERT_TRUE(t3_deleted);
}

TEST(intrusive_circular_list_test, iterators) {
    list l;

    // empty list
    ASSERT_EQ(l.begin(), l.end());

    auto* e1 = l.emplace_back();

    auto it = l.begin();
    auto end = l.end();
    ASSERT_NE(it, end);

    ASSERT_EQ(e1, *it);
    ASSERT_EQ(e1, *it++);
    ASSERT_EQ(it, end);

    auto* e2 = l.emplace_back();

    it = l.begin();
    end = l.end();
    ASSERT_NE(it, end);

    ASSERT_EQ(e1, *it);
    ASSERT_EQ(e1, *it++);
    ASSERT_EQ(e2, *it);
    ASSERT_EQ(e2, *it++);
    ASSERT_EQ(it, end);
}

TEST(intrusive_circular_list_test, reverse_iterators) {
    list l;

    // empty list
    ASSERT_EQ(l.rbegin(), l.rend());

    auto* e1 = l.emplace_back();

    auto it = l.rbegin();
    auto end = l.rend();
    ASSERT_NE(it, end);

    ASSERT_EQ(e1, *it);
    ASSERT_EQ(e1, *it++);
    ASSERT_EQ(it, end);

    auto* e2 = l.emplace_back();
    auto* e3 = l.emplace_back();

    it = l.rbegin();
    end = l.rend();
    ASSERT_NE(it, end);

    ASSERT_EQ(e3, *it);
    ASSERT_EQ(e3, *it++);
    ASSERT_EQ(e2, *it);
    ASSERT_EQ(e2, *it++);
    ASSERT_EQ(e1, *it);
    ASSERT_EQ(e1, *it++);
    ASSERT_EQ(it, end);
}

TEST(intrusive_circular_list_test, empty) {
    list l;
    ASSERT_TRUE(l.empty());

    auto* e1 = new element();
    l.push_back(e1);
    ASSERT_FALSE(l.empty());
}

TEST(intrusive_circular_list_test, size) {
    list l;
    ASSERT_EQ(0u, l.size());

    auto* e1 = new element();
    l.push_back(e1);
    ASSERT_EQ(1u, l.size());
}

TEST(intrusive_circular_list_test, front) {
    list l;

    auto* e1 = new element();
    auto* e2 = new element();
    auto* e3 = new element();

    ASSERT_EQ(nullptr, l.front());

    l.push_back(e1);
    ASSERT_EQ(e1, l.front());

    l.push_back(e2);
    ASSERT_EQ(e1, l.front());

    l.push_back(e3);
    ASSERT_EQ(e1, l.front());
}

TEST(intrusive_circular_list_test, back) {
    list l;

    auto* e1 = new element();
    auto* e2 = new element();
    auto* e3 = new element();

    ASSERT_EQ(nullptr, l.back());

    l.push_back(e1);
    ASSERT_EQ(e1, l.back());

    l.push_back(e2);
    ASSERT_EQ(e2, l.back());

    l.push_back(e3);
    ASSERT_EQ(e3, l.back());
}

TEST(intrusive_circular_list_test, contains) {
    list l;

    auto* e1 = new element();
    auto* e2 = new element();
    auto* e3 = new element();

    l.push_back(e1);
    l.push_back(e2);

    ASSERT_TRUE(l.contains(e1));
    ASSERT_TRUE(l.contains(e2));
    ASSERT_FALSE(l.contains(e3));

    l.push_back(e3);
    ASSERT_TRUE(l.contains(e3));
}

TEST(intrusive_circular_list_test, push_back) {
    list l;
    auto* e1 = new element();
    l.push_back(e1);
    assertList({ e1 }, l);

    auto* e2 = new element();
    l.push_back(e2);
    assertList({ e1, e2 }, l);

    auto* e3 = new element();
    l.push_back(e3);
    assertList({ e1, e2, e3 }, l);
}

TEST(intrusive_circular_list_test, remove_single_item) {
    auto* e1 = new element();
    list l({ e1 });
    assertList({ e1 }, l.remove(e1));
    assertList({}, l);
}

TEST(intrusive_circular_list_test, remove_front_item) {
    auto* e1 = new element();
    auto* e2 = new element();
    auto* e3 = new element();
    list l({ e1, e2, e3 });
    assertList({ e1 }, l.remove(e1));
    assertList({ e2, e3 }, l);
}

TEST(intrusive_circular_list_test, remove_mid_item) {
    auto* e1 = new element();
    auto* e2 = new element();
    auto* e3 = new element();
    list l({ e1, e2, e3 });
    assertList({ e2 }, l.remove(e2));
    assertList({ e3, e1 }, l); // removal affects list head
}

TEST(intrusive_circular_list_test, remove_back_item) {
    auto* e1 = new element();
    auto* e2 = new element();
    auto* e3 = new element();
    list l({ e1, e2, e3 });
    assertList({ e3 }, l.remove(e3));
    assertList({ e1, e2 }, l);
}

TEST(intrusive_circular_list_test, remove_single) {
    list l;

    auto e1_deleted = false;
    auto e2_deleted = false;
    auto e3_deleted = false;
    auto e4_deleted = false;

    auto* e1 = new delete_tracking_element(e1_deleted);
    auto* e2 = new delete_tracking_element(e2_deleted);
    auto* e3 = new delete_tracking_element(e3_deleted);
    auto* e4 = new delete_tracking_element(e4_deleted);

    l.push_back(e1);
    l.push_back(e2);
    l.push_back(e3);
    l.push_back(e4);

    // mid element
    l.remove(list::iter(e2), std::next(list::iter(e2)), 1u);
    ASSERT_FALSE(e1_deleted);
    ASSERT_TRUE(e2_deleted);
    ASSERT_FALSE(e3_deleted);
    ASSERT_FALSE(e4_deleted);
    assertList({ e1, e3, e4 }, l);

    // front element
    l.remove(list::iter(e3), std::next(list::iter(e3)), 1u);
    ASSERT_FALSE(e1_deleted);
    ASSERT_TRUE(e2_deleted);
    ASSERT_TRUE(e3_deleted);
    ASSERT_FALSE(e4_deleted);
    assertList({ e1, e4 }, l);

    // back element
    l.remove(list::iter(e1), std::next(list::iter(e1)), 1u);
    ASSERT_TRUE(e1_deleted);
    ASSERT_TRUE(e2_deleted);
    ASSERT_TRUE(e3_deleted);
    ASSERT_FALSE(e4_deleted);
    assertList({ e4 }, l);

    // single element
    l.remove(list::iter(e4), std::next(list::iter(e4)), 1u);
    ASSERT_TRUE(e1_deleted);
    ASSERT_TRUE(e2_deleted);
    ASSERT_TRUE(e3_deleted);
    ASSERT_TRUE(e4_deleted);
    assertList({}, l);
}

TEST(intrusive_circular_list_test, remove_multiple) {
    list l;

    auto e1_deleted = false;
    auto e2_deleted = false;
    auto e3_deleted = false;
    auto e4_deleted = false;

    auto* e1 = new delete_tracking_element(e1_deleted);
    auto* e2 = new delete_tracking_element(e2_deleted);
    auto* e3 = new delete_tracking_element(e3_deleted);
    auto* e4 = new delete_tracking_element(e4_deleted);

    l.push_back(e1);
    l.push_back(e2);
    l.push_back(e3);
    l.push_back(e4);

    l.remove(list::iter(e4), std::next(list::iter(e1)), 2u);
    ASSERT_TRUE(e1_deleted);
    ASSERT_FALSE(e2_deleted);
    ASSERT_FALSE(e3_deleted);
    ASSERT_TRUE(e4_deleted);
    assertList({ e2, e3 }, l);
}


TEST(intrusive_circular_list_test, remove_all) {
    list l;

    auto e1_deleted = false;
    auto e2_deleted = false;

    auto* e1 = new delete_tracking_element(e1_deleted);
    auto* e2 = new delete_tracking_element(e2_deleted);

    l.push_back(e1);
    l.push_back(e2);

    l.remove(list::iter(e1), std::next(list::iter(e2)), 2u);
    ASSERT_TRUE(e1_deleted);
    ASSERT_TRUE(e2_deleted);
    assertList({}, l);
}


TEST(intrusive_circular_list_test, release_single) {
    list l;

    auto e1_deleted = false;
    auto e2_deleted = false;
    auto e3_deleted = false;
    auto e4_deleted = false;

    element* e1 = new delete_tracking_element(e1_deleted);
    element* e2 = new delete_tracking_element(e2_deleted);
    element* e3 = new delete_tracking_element(e3_deleted);
    element* e4 = new delete_tracking_element(e4_deleted);

    l.push_back(e1);
    l.push_back(e2);
    l.push_back(e3);
    l.push_back(e4);

    // mid element
    l.release(list::iter(e2), std::next(list::iter(e2)), 1u);
    ASSERT_FALSE(e1_deleted);
    ASSERT_FALSE(e2_deleted);
    ASSERT_FALSE(e3_deleted);
    ASSERT_FALSE(e4_deleted);
    assertList({ e1, e3, e4 }, l);
    assertLinks(e2, { e2 });

    // front element
    l.release(list::iter(e3), std::next(list::iter(e3)), 1u);
    ASSERT_FALSE(e1_deleted);
    ASSERT_FALSE(e2_deleted);
    ASSERT_FALSE(e3_deleted);
    ASSERT_FALSE(e4_deleted);
    assertList({ e1, e4 }, l);
    assertLinks(e3, { e3 });

    // back element
    l.release(list::iter(e1), std::next(list::iter(e1)), 1u);
    ASSERT_FALSE(e1_deleted);
    ASSERT_FALSE(e2_deleted);
    ASSERT_FALSE(e3_deleted);
    ASSERT_FALSE(e4_deleted);
    assertList({ e4 }, l);
    assertLinks(e1, { e1 });

    // single element
    l.release(list::iter(e4), std::next(list::iter(e4)), 1u);
    ASSERT_FALSE(e1_deleted);
    ASSERT_FALSE(e2_deleted);
    ASSERT_FALSE(e3_deleted);
    ASSERT_FALSE(e4_deleted);
    assertList({}, l);
    assertLinks(e4, { e4 });
}

TEST(intrusive_circular_list_test, release_multiple) {
    list l;

    auto e1_deleted = false;
    auto e2_deleted = false;
    auto e3_deleted = false;
    auto e4_deleted = false;

    element* e1 = new delete_tracking_element(e1_deleted);
    element* e2 = new delete_tracking_element(e2_deleted);
    element* e3 = new delete_tracking_element(e3_deleted);
    element* e4 = new delete_tracking_element(e4_deleted);

    l.push_back(e1);
    l.push_back(e2);
    l.push_back(e3);
    l.push_back(e4);

    l.release(list::iter(e4), std::next(list::iter(e1)), 2u);
    ASSERT_FALSE(e1_deleted);
    ASSERT_FALSE(e2_deleted);
    ASSERT_FALSE(e3_deleted);
    ASSERT_FALSE(e4_deleted);
    assertList({ e2, e3 }, l);
    assertLinks(e4, { e1, e4 });
}

TEST(intrusive_circular_list_test, release_all) {
    list l;

    auto e1_deleted = false;
    auto e2_deleted = false;

    element* e1 = new delete_tracking_element(e1_deleted);
    element* e2 = new delete_tracking_element(e2_deleted);

    l.push_back(e1);
    l.push_back(e2);

    l.release(list::iter(e1), std::next(list::iter(e2)), 2u);
    ASSERT_FALSE(e1_deleted);
    ASSERT_FALSE(e2_deleted);
    assertList({}, l);
    assertLinks(e1, { e1, e2 });
}

TEST(intrusive_circular_list_test, emplace_back) {
    list l;

    auto* e1 = l.emplace_back();
    assertList({ e1 }, l);

    auto* e2 = l.emplace_back();
    assertList({ e1, e2 }, l);

    auto* e3 = l.emplace_back();
    assertList({ e1, e2, e3 }, l);
}

TEST(intrusive_circular_list_test, emplace_back_subtype) {
    auto e1_deleted = false;
    {
        list l;
        auto* e1 = l.emplace_back<delete_tracking_element>(e1_deleted);
        assertList({ e1 }, l);
    }
    ASSERT_TRUE(e1_deleted);
}

TEST(intrusive_circular_list_test, reverse) {
    auto* e1 = new element();
    auto* e2 = new element();
    auto* e3 = new element();
    list l({ e1, e2, e3 });

    l.reverse();
    assertList({e3, e2, e1}, l);
}

TEST(intrusive_circular_list_test, append_list) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();

    to.push_back(t1);
    to.push_back(t2);

    to.append(from);
    assertList({ t1, t2, f1, f2, f3 }, to);
    assertList({}, from);
}

TEST(intrusive_circular_list_test, insert_list_front) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();

    to.push_back(t1);
    to.push_back(t2);

    to.insert(list::iter(t1), from);
    assertList({ f1, f2, f3, t1, t2 }, to);
    assertList({}, from);
}

TEST(intrusive_circular_list_test, insert_list_back) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();

    to.push_back(t1);
    to.push_back(t2);

    to.insert(std::end(to), from);
    assertList({ t1, t2, f1, f2, f3 }, to);
    assertList({}, from);
}

TEST(intrusive_circular_list_test, splice_back_one_item) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();

    to.push_back(t1);
    to.push_back(t2);

    to.splice_back(from, list::iter(f1), list::iter(f2), 1u);
    assertList({ t1, t2, f1 }, to);
    assertList({ f2, f3 }, from);
}

TEST(intrusive_circular_list_test, splice_back_two_items) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();

    to.push_back(t1);
    to.push_back(t2);

    to.splice_back(from, list::iter(f1), list::iter(f3), 2u);
    assertList({ t1, t2, f1, f2 }, to);
    assertList({ f3 }, from);
}

TEST(intrusive_circular_list_test, splice_one_item_into_empty_list) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    to.splice(std::begin(to), from, list::iter(f2), list::iter(f3), 1u);
    assertList({ f2 }, to);
    assertList({ f1, f3 }, from);
}

TEST(intrusive_circular_list_test, splice_two_items_into_empty_list) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    to.splice(std::begin(to), from, list::iter(f2), list::iter(f1), 2u);
    assertList({ f2, f3 }, to);
    assertList({ f1 }, from);
}

TEST(intrusive_circular_list_test, splice_all_items_into_empty_list) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    to.splice(std::end(to), from, std::begin(from), std::end(from), 3u);
    assertList({ f1, f2, f3 }, to);
    assertList({}, from);
}

TEST(intrusive_circular_list_test, splice_one_item_into_front) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();
    auto* t3 = new element();

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice(list::iter(t1), from, list::iter(f2), list::iter(f3), 1u);
    assertList({ f2, t1, t2, t3 }, to);
    assertList({ f1, f3 }, from);
}

TEST(intrusive_circular_list_test, splice_one_item_into_mid) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();
    auto* t3 = new element();

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice(list::iter(t2), from, list::iter(f2), list::iter(f3), 1u);
    assertList({ t1, f2, t2, t3 }, to);
    assertList({ f1, f3 }, from);
}

TEST(intrusive_circular_list_test, splice_one_item_into_last) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();
    auto* t3 = new element();

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice(list::iter(t3), from, list::iter(f2), list::iter(f3), 1u);
    assertList({ t1, t2, f2, t3 }, to);
    assertList({ f1, f3 }, from);
}

TEST(intrusive_circular_list_test, splice_last_two_items_into_front) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();
    auto* t3 = new element();

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice(list::iter(t1), from, list::iter(f2), list::iter(f1), 2u);
    assertList({ f2, f3, t1, t2, t3 }, to);
    assertList({ f1 }, from);
}

TEST(intrusive_circular_list_test, splice_last_two_items_into_mid) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();
    auto* t3 = new element();

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice(list::iter(t2), from, list::iter(f2), std::end(from), 2u);
    assertList({ t1, f2, f3, t2, t3 }, to);
    assertList({ f1 }, from);
}

TEST(intrusive_circular_list_test, splice_last_two_items_into_last) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();
    auto* t3 = new element();

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice(list::iter(t3), from, list::iter(f2), std::next(list::iter(f3)), 2u);
    assertList({ t1, t2, f2, f3, t3 }, to);
    assertList({ f1 }, from);
}

TEST(intrusive_circular_list_test, splice_last_and_first_items_items_front) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();
    auto* t3 = new element();

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice(list::iter(t1), from, list::iter(f3), list::iter(f2), 2u);
    assertList({ f3, f1, t1, t2, t3 }, to);
    assertList({ f2 }, from);
}

TEST(intrusive_circular_list_test, splice_all_items_into_front) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();
    auto* t3 = new element();

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice(std::begin(to), from, list::iter(f3), list::iter(f3), 3u);
    assertList({ f3, f1, f2, t1, t2, t3 }, to);
    assertList({}, from);
}

TEST(intrusive_circular_list_test, splice_all_items_into_mid) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();
    auto* t3 = new element();

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice(list::iter(t2), from, list::iter(f3), list::iter(f3), 3u);
    assertList({ t1, f3, f1, f2, t2, t3 }, to);
    assertList({}, from);
}

TEST(intrusive_circular_list_test, splice_all_items_into_last) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto* t1 = new element();
    auto* t2 = new element();
    auto* t3 = new element();

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice(list::iter(t3), from, list::iter(f3), list::iter(f3), 3u);
    assertList({ t1, t2, f3, f1, f2, t3 }, to);
    assertList({}, from);
}

TEST(intrusive_circular_list_test, splice_replace_first_item_with_one_item) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(list::iter(t1), list::iter(t2), 1u, from, list::iter(f2), list::iter(f3), 1u);
    assertList({ f2, t2, t3 }, to);
    assertList({ f1, f3 }, from);

    ASSERT_TRUE(t1_deleted);
    ASSERT_FALSE(t2_deleted);
    ASSERT_FALSE(t3_deleted);
}

TEST(intrusive_circular_list_test, splice_replace_mid_item_with_one_item) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(list::iter(t2), list::iter(t3), 1u, from, list::iter(f2), list::iter(f3), 1u);
    assertList({ t1, f2, t3 }, to);
    assertList({ f1, f3 }, from);

    ASSERT_FALSE(t1_deleted);
    ASSERT_TRUE(t2_deleted);
    ASSERT_FALSE(t3_deleted);
}

TEST(intrusive_circular_list_test, splice_replace_last_item_with_one_item) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(list::iter(t3), std::end(to), 1u, from, list::iter(f2), list::iter(f3), 1u);
    assertList({ t1, t2, f2 }, to);
    assertList({ f1, f3 }, from);

    ASSERT_FALSE(t1_deleted);
    ASSERT_FALSE(t2_deleted);
    ASSERT_TRUE(t3_deleted);
}

TEST(intrusive_circular_list_test, splice_replace_first_item_with_two_items) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(list::iter(t1), list::iter(t2), 1u, from, list::iter(f3), list::iter(f2), 2u);
    assertList({ t2, t3, f3, f1 }, to);
    assertList({ f2 }, from);

    ASSERT_TRUE(t1_deleted);
    ASSERT_FALSE(t2_deleted);
    ASSERT_FALSE(t3_deleted);
}

TEST(intrusive_circular_list_test, splice_replace_mid_item_with_two_items) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(list::iter(t2), list::iter(t3), 1u, from, list::iter(f3), list::iter(f2), 2u);
    assertList({ t1, f3, f1, t3 }, to);
    assertList({ f2 }, from);

    ASSERT_FALSE(t1_deleted);
    ASSERT_TRUE(t2_deleted);
    ASSERT_FALSE(t3_deleted);
}

TEST(intrusive_circular_list_test, splice_replace_last_item_with_two_items) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(list::iter(t3), list::iter(t1), 1u, from, list::iter(f3), list::iter(f2), 2u);
    assertList({ t1, t2, f3, f1 }, to);
    assertList({ f2 }, from);

    ASSERT_FALSE(t1_deleted);
    ASSERT_FALSE(t2_deleted);
    ASSERT_TRUE(t3_deleted);
}

TEST(intrusive_circular_list_test, splice_replace_mid_item_with_all_items) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(list::iter(t2), list::iter(t3), 1u, from, list::iter(f3), list::iter(f3), 3u);
    assertList({ t1, f3, f1, f2, t3 }, to);
    assertList({}, from);

    ASSERT_FALSE(t1_deleted);
    ASSERT_TRUE(t2_deleted);
    ASSERT_FALSE(t3_deleted);
}

TEST(intrusive_circular_list_test, splice_replace_first_two_items_with_two_items) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(list::iter(t1), list::iter(t3), 2u, from, list::iter(f1), list::iter(f3), 2u);
    assertList({ f1, f2, t3 }, to);
    assertList({ f3 }, from);

    ASSERT_TRUE(t1_deleted);
    ASSERT_TRUE(t2_deleted);
    ASSERT_FALSE(t3_deleted);
}

TEST(intrusive_circular_list_test, splice_replace_last_two_items_with_two_items) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(list::iter(t2), list::iter(t1), 2u, from, list::iter(f1), list::iter(f3), 2u);
    assertList({ t1, f1, f2 }, to);
    assertList({ f3 }, from);

    ASSERT_FALSE(t1_deleted);
    ASSERT_TRUE(t2_deleted);
    ASSERT_TRUE(t3_deleted);
}

TEST(intrusive_circular_list_test, splice_replace_last_and_first_items_with_two_items) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(list::iter(t3), list::iter(t2), 2u, from, list::iter(f1), list::iter(f3), 2u);
    assertList({ t2, f1, f2 }, to);
    assertList({ f3 }, from);

    ASSERT_TRUE(t1_deleted);
    ASSERT_FALSE(t2_deleted);
    ASSERT_TRUE(t3_deleted);
}

TEST(intrusive_circular_list_test, splice_replace_all_items_with_two_items) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(list::iter(t3), list::iter(t3), 3u, from, list::iter(f1), list::iter(f3), 2u);
    assertList({ f1, f2 }, to);
    assertList({ f3 }, from);

    ASSERT_TRUE(t1_deleted);
    ASSERT_TRUE(t2_deleted);
    ASSERT_TRUE(t3_deleted);
}

TEST(intrusive_circular_list_test, splice_replace_all_items_with_one_item) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(list::iter(t2), list::iter(t2), 3u, from, list::iter(f1), list::iter(f2), 1u);
    assertList({ f1 }, to);
    assertList({ f2, f3 }, from);

    ASSERT_TRUE(t1_deleted);
    ASSERT_TRUE(t2_deleted);
    ASSERT_TRUE(t3_deleted);
}

TEST(intrusive_circular_list_test, splice_replace_all_items_with_all_items) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto* t1 = new delete_tracking_element(t1_deleted);
    auto* t2 = new delete_tracking_element(t2_deleted);
    auto* t3 = new delete_tracking_element(t3_deleted);

    to.push_back(t1);
    to.push_back(t2);
    to.push_back(t3);

    to.splice_replace(std::begin(to), std::end(to), to.size(), from, std::begin(from), std::end(from), from.size());
    assertList({ f1, f2, f3 }, to);
    assertList({}, from);

    ASSERT_TRUE(t1_deleted);
    ASSERT_TRUE(t2_deleted);
    ASSERT_TRUE(t3_deleted);
}

TEST(intrusive_circular_list_test, release) {
    list l;

    auto e1_deleted = false;
    auto e2_deleted = false;

    element* e1 = new delete_tracking_element(e1_deleted);
    element* e2 = new delete_tracking_element(e2_deleted);

    l.push_back(e1);
    l.push_back(e2);

    l.release();
    ASSERT_FALSE(e1_deleted);
    ASSERT_FALSE(e2_deleted);
    assertList({}, l);
    assertLinks(e1, { e1, e2 });
}

TEST(intrusive_circular_list_test, clear_empty_list) {
    list l;

    l.clear();
    assertList({}, l);
}

TEST(intrusive_circular_list_test, clear_with_items) {
    list l;

    auto e1_deleted = false;
    auto e2_deleted = false;

    element* e1 = new delete_tracking_element(e1_deleted);
    element* e2 = new delete_tracking_element(e2_deleted);

    l.push_back(e1);
    l.push_back(e2);

    l.clear();
    ASSERT_TRUE(e1_deleted);
    ASSERT_TRUE(e2_deleted);
    assertList({}, l);
}
