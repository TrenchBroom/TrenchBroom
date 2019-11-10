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

template <typename List>
void assertList(const List& list, const std::vector<typename List::item*>& items) {
    ASSERT_EQ(list.empty(), items.empty());
    ASSERT_EQ(list.size(), items.size());

    if (!list.empty()) {
        ASSERT_EQ(list.front(), items.front());
        ASSERT_EQ(list.back(), items.back());

        const auto get_link = typename List::get_link_info();
        auto list_cur = list.front();
        auto list_previous = list.back();

        for (std::size_t i = 0u; i < items.size(); ++i) {
            auto items_cur = items[i];
            ASSERT_EQ(list_cur, items_cur);

            ASSERT_EQ(list_previous, get_link(list_cur).previous());
            ASSERT_EQ(list_cur, get_link(list_previous).next());

            list_previous = list_cur;
            list_cur = get_link(list_cur).next();
        }
    }
}

TEST(intrusive_circular_list_test, constructor_default) {
    list l;
    assertList(l, {});
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

TEST(intrusive_circular_list_test, push_back) {
    list l;
    auto* e1 = new element();
    l.push_back(e1);
    assertList(l, { e1 });

    auto* e2 = new element();
    l.push_back(e2);
    assertList(l, { e1, e2 });

    auto* e3 = new element();
    l.push_back(e3);
    assertList(l, { e1, e2, e3 });
}

TEST(intrusive_circular_list_test, emplace_back) {
    list l;

    auto* e1 = l.emplace_back();
    assertList(l, { e1 });

    auto* e2 = l.emplace_back();
    assertList(l, { e1, e2 });

    auto* e3 = l.emplace_back();
    assertList(l, { e1, e2, e3 });
}

TEST(intrusive_circular_list_test, append) {
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
    assertList(to, { t1, t2, f1, f2, f3 });
    assertList(from, {});
}

TEST(intrusive_circular_list_test, insert_before_front) {
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

    to.insert_before(t1, from);
    assertList(to, { t1, t2, f1, f2, f3 });
    assertList(from, {});
}

TEST(intrusive_circular_list_test, insert_before_back) {
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

    to.insert_before(t2, from);
    assertList(to, { t1, f1, f2, f3, t2 });
    assertList(from, {});
}

TEST(intrusive_circular_list_test, insert_after_front) {
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

    to.insert_after(t1, from);
    assertList(to, { t1, f1, f2, f3, t2 });
    assertList(from, {});
}

TEST(intrusive_circular_list_test, insert_after_back) {
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

    to.insert_after(t2, from);
    assertList(to, { t1, t2, f1, f2, f3 });
    assertList(from, {});
}

TEST(intrusive_circular_list_test, splice_one_item_before_empty_list) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    to.splice_before(nullptr, from, f2, f2, 1u);
    assertList(to, { f2 });
    assertList(from, { f1, f3 });
}

TEST(intrusive_circular_list_test, splice_two_items_before_empty_list) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    to.splice_before(nullptr, from, f2, f3, 2u);
    assertList(to, { f2, f3 });
    assertList(from, { f1 });
}

TEST(intrusive_circular_list_test, splice_all_items_before_empty_list) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    to.splice_before(nullptr, from, f1, f3, 3u);
    assertList(to, { f1, f2, f3 });
    assertList(from, {});
}

TEST(intrusive_circular_list_test, splice_one_item_before_front) {
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

    to.splice_before(t1, from, f2, f2, 1u);
    assertList(to, { t1, t2, t3, f2 });
    assertList(from, { f1, f3 });
}

TEST(intrusive_circular_list_test, splice_one_item_before_mid) {
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

    to.splice_before(t2, from, f2, f2, 1u);
    assertList(to, { t1, f2, t2, t3 });
    assertList(from, { f1, f3 });
}

TEST(intrusive_circular_list_test, splice_one_item_before_last) {
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

    to.splice_before(t3, from, f2, f2, 1u);
    assertList(to, { t1, t2, f2, t3 });
    assertList(from, { f1, f3 });
}

TEST(intrusive_circular_list_test, splice_last_two_items_before_front) {
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

    to.splice_before(t1, from, f2, f3, 2u);
    assertList(to, { t1, t2, t3, f2, f3 });
    assertList(from, { f1 });
}

TEST(intrusive_circular_list_test, splice_last_two_items_before_mid) {
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

    to.splice_before(t2, from, f2, f3, 2u);
    assertList(to, { t1, f2, f3, t2, t3 });
    assertList(from, { f1 });
}

TEST(intrusive_circular_list_test, splice_last_two_items_before_last) {
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

    to.splice_before(t3, from, f2, f3, 2u);
    assertList(to, { t1, t2, f2, f3, t3 });
    assertList(from, { f1 });
}

TEST(intrusive_circular_list_test, splice_last_and_first_items_before_front) {
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

    to.splice_before(t1, from, f3, f1, 2u);
    assertList(to, { t1, t2, t3, f3, f1 });
    assertList(from, { f2 });
}

TEST(intrusive_circular_list_test, splice_all_items_before_front) {
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

    to.splice_before(t1, from, f3, f2, 3u);
    assertList(to, { t1, t2, t3, f3, f1, f2 });
    assertList(from, {});
}

TEST(intrusive_circular_list_test, splice_all_items_before_mid) {
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

    to.splice_before(t2, from, f3, f2, 3u);
    assertList(to, { t1, f3, f1, f2, t2, t3 });
    assertList(from, {});
}

TEST(intrusive_circular_list_test, splice_all_items_before_last) {
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

    to.splice_before(t3, from, f3, f2, 3u);
    assertList(to, { t1, t2, f3, f1, f2, t3 });
    assertList(from, {});
}

TEST(intrusive_circular_list_test, splice_one_item_after_empty_list) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    to.splice_after(nullptr, from, f2, f2, 1u);
    assertList(to, { f2 });
    assertList(from, { f1, f3 });
}

TEST(intrusive_circular_list_test, splice_two_items_after_empty_list) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    to.splice_after(nullptr, from, f2, f3, 2u);
    assertList(to, { f2, f3 });
    assertList(from, { f1 });
}

TEST(intrusive_circular_list_test, splice_all_items_after_empty_list) {
    list from;
    list to;

    auto* f1 = new element();
    auto* f2 = new element();
    auto* f3 = new element();

    from.push_back(f1);
    from.push_back(f2);
    from.push_back(f3);

    to.splice_after(nullptr, from, f1, f3, 3u);
    assertList(to, { f1, f2, f3 });
    assertList(from, {});
}

TEST(intrusive_circular_list_test, splice_one_item_after_front) {
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

    to.splice_after(t1, from, f2, f2, 1u);
    assertList(to, { t1, f2, t2, t3 });
    assertList(from, { f1, f3 });
}

TEST(intrusive_circular_list_test, splice_one_item_after_mid) {
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

    to.splice_after(t2, from, f2, f2, 1u);
    assertList(to, { t1, t2, f2, t3 });
    assertList(from, { f1, f3 });
}

TEST(intrusive_circular_list_test, splice_one_item_after_last) {
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

    to.splice_after(t3, from, f2, f2, 1u);
    assertList(to, { t1, t2, t3, f2 });
    assertList(from, { f1, f3 });
}

TEST(intrusive_circular_list_test, splice_last_two_items_after_front) {
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

    to.splice_after(t1, from, f2, f3, 2u);
    assertList(to, { t1, f2, f3, t2, t3 });
    assertList(from, { f1 });
}

TEST(intrusive_circular_list_test, splice_last_two_items_after_mid) {
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

    to.splice_after(t2, from, f2, f3, 2u);
    assertList(to, { t1, t2, f2, f3, t3 });
    assertList(from, { f1 });
}

TEST(intrusive_circular_list_test, splice_last_two_items_after_last) {
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

    to.splice_after(t3, from, f2, f3, 2u);
    assertList(to, { t1, t2, t3, f2, f3 });
    assertList(from, { f1 });
}

TEST(intrusive_circular_list_test, splice_last_and_first_items_after_front) {
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

    to.splice_after(t1, from, f3, f1, 2u);
    assertList(to, { t1, f3, f1, t2, t3 });
    assertList(from, { f2 });
}

TEST(intrusive_circular_list_test, splice_all_items_after_front) {
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

    to.splice_after(t1, from, f3, f2, 3u);
    assertList(to, { t1, f3, f1, f2, t2, t3 });
    assertList(from, {});
}

TEST(intrusive_circular_list_test, splice_all_items_after_mid) {
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

    to.splice_after(t2, from, f3, f2, 3u);
    assertList(to, { t1, t2, f3, f1, f2, t3 });
    assertList(from, {});
}

TEST(intrusive_circular_list_test, splice_all_items_after_last) {
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

    to.splice_after(t3, from, f3, f2, 3u);
    assertList(to, { t1, t2, t3, f3, f1, f2 });
    assertList(from, {});
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

    to.splice_replace(t1, t1, 1u, from, f2, f2, 1u);
    assertList(to, { t3, f2, t2 }); // removal affects list head
    assertList(from, { f1, f3 });

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

    to.splice_replace(t2, t2, 1u, from, f2, f2, 1u);
    assertList(to, { t1, f2, t3 });
    assertList(from, { f1, f3 });

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

    to.splice_replace(t3, t3, 1u, from, f2, f2, 1u);
    assertList(to, { t2, f2, t1 }); // removal affects list head
    assertList(from, { f1, f3 });

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

    to.splice_replace(t1, t1, 1u, from, f3, f1, 2u);
    assertList(to, { t3, f3, f1, t2 }); // removal affects list head
    assertList(from, { f2 });

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

    to.splice_replace(t2, t2, 1u, from, f3, f1, 2u);
    assertList(to, { t1, f3, f1, t3 });
    assertList(from, { f2 });

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

    to.splice_replace(t3, t3, 1u, from, f3, f1, 2u);
    assertList(to, { t2, f3, f1, t1 }); // removal affects list head
    assertList(from, { f2 });

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

    to.splice_replace(t2, t2, 1u, from, f3, f2, 3u);
    assertList(to, { t1, f3, f1, f2, t3 });
    assertList(from, {});

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

    to.splice_replace(t1, t2, 2u, from, f1, f2, 2u);
    assertList(to, { t3, f1, f2 });
    assertList(from, { f3 });

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

    to.splice_replace(t2, t3, 2u, from, f1, f2, 2u);
    assertList(to, { t1, f1, f2 });
    assertList(from, { f3 });

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

    to.splice_replace(t3, t1, 2u, from, f1, f2, 2u);
    assertList(to, { t2, f1, f2 });
    assertList(from, { f3 });

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

    to.splice_replace(t3, t2, 3u, from, f1, f2, 2u);
    assertList(to, { f1, f2 });
    assertList(from, { f3 });

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

    to.splice_replace(t2, t1, 3u, from, f1, f1, 1u);
    assertList(to, { f1 });
    assertList(from, { f3, f2 }); // removal affects list head

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

    to.splice_replace(t1, t3, 3u, from, f1, f3, 3u);
    assertList(to, { f1, f2, f3 });
    assertList(from, {});

    ASSERT_TRUE(t1_deleted);
    ASSERT_TRUE(t2_deleted);
    ASSERT_TRUE(t3_deleted);
}
