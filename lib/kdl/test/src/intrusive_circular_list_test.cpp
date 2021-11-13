/*
 Copyright 2010-2019 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "kdl/intrusive_circular_list.h"

#include <vector>

#include <catch2/catch.hpp>

namespace kdl {
class element;
using element_link = intrusive_circular_link<element>;

class element {
private:
  friend class get_link;
  element_link m_link;

public:
  element()
    : m_link(this) {}

  virtual ~element() = default;

  const element& next() const { return *m_link.next(); }

  const element& previous() const { return *m_link.previous(); }
};

class delete_tracking_element : public element {
private:
  bool& m_deleted;

public:
  explicit delete_tracking_element(bool& deleted)
    : m_deleted(deleted) {
    m_deleted = false;
  }

  ~delete_tracking_element() override { m_deleted = true; }
};

class get_link {
public:
  element_link& operator()(element* element) const { return element->m_link; }
  const element_link& operator()(const element* element) const { return element->m_link; }
};

using list = intrusive_circular_list<element, get_link>;

template <typename Item> void assertLinks(Item* head, const std::vector<Item*>& items) {
  CHECK((head == nullptr) == (items.empty()));

  if (head != nullptr) {
    const auto get_link = kdl::get_link();

    // find the front of the list
    auto list_first = head;
    while (list_first != items.front()) {
      list_first = get_link(list_first).next();
      UNSCOPED_INFO("list head is not an item");
      CHECK(list_first != head);
    }
    auto list_cur = list_first;
    auto list_previous = get_link(list_cur).previous();

    for (std::size_t i = 0u; i < items.size(); ++i) {
      auto items_cur = items[i];
      CHECK(items_cur == list_cur);
      CHECK(get_link(list_previous).next() == list_cur);

      list_previous = list_cur;
      list_cur = get_link(list_cur).next();
    }

    CHECK(list_cur == list_first);
  }
}

template <typename List>
void assertList(const std::vector<typename List::value_type*>& expected, const List& actual) {
  CHECK(expected.empty() == actual.empty());
  CHECK(expected.size() == actual.size());

  if (!actual.empty()) {
    assertLinks(actual.front(), expected);
  }
}

TEST_CASE("intrusive_circular_list_test.constructor_default", "[intrusive_circular_list_test]") {
  assertList({}, list());
}

TEST_CASE(
  "intrusive_circular_list_test.constructor_initializer_list", "[intrusive_circular_list_test]") {
  assertList({}, list({}));

  auto* e1 = new element();
  assertList({e1}, list({e1}));

  auto* e2 = new element();
  auto* e3 = new element();
  assertList({e2, e3}, list({e2, e3}));
}

TEST_CASE("intrusive_circular_list_test.destructor_cleanup", "[intrusive_circular_list_test]") {
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

  CHECK(t1_deleted);
  CHECK(t2_deleted);
  CHECK(t3_deleted);
}

TEST_CASE("intrusive_circular_list_test.iterators", "[intrusive_circular_list_test]") {
  list l;

  // empty list
  CHECK(l.end() == l.begin());

  auto* e1 = l.emplace_back();

  auto it = l.begin();
  auto end = l.end();
  CHECK(end != it);

  CHECK(*it == e1);
  CHECK(*it++ == e1);
  CHECK(end == it);

  auto* e2 = l.emplace_back();

  it = l.begin();
  end = l.end();
  CHECK(end != it);

  CHECK(*it == e1);
  CHECK(*it++ == e1);
  CHECK(*it == e2);
  CHECK(*it++ == e2);
  CHECK(end == it);
}

TEST_CASE("intrusive_circular_list_test.reverse_iterators", "[intrusive_circular_list_test]") {
  list l;

  // empty list
  CHECK(l.rend() == l.rbegin());

  auto* e1 = l.emplace_back();

  auto it = l.rbegin();
  auto end = l.rend();
  CHECK(end != it);

  CHECK(*it == e1);
  CHECK(*it++ == e1);
  CHECK(end == it);

  auto* e2 = l.emplace_back();
  auto* e3 = l.emplace_back();

  it = l.rbegin();
  end = l.rend();
  CHECK(end != it);

  CHECK(*it == e3);
  CHECK(*it++ == e3);
  CHECK(*it == e2);
  CHECK(*it++ == e2);
  CHECK(*it == e1);
  CHECK(*it++ == e1);
  CHECK(end == it);
}

TEST_CASE("intrusive_circular_list_test.empty", "[intrusive_circular_list_test]") {
  list l;
  CHECK(l.empty());

  auto* e1 = new element();
  l.push_back(e1);
  CHECK_FALSE(l.empty());
}

TEST_CASE("intrusive_circular_list_test.size", "[intrusive_circular_list_test]") {
  list l;
  CHECK(l.size() == 0u);

  auto* e1 = new element();
  l.push_back(e1);
  CHECK(l.size() == 1u);
}

TEST_CASE("intrusive_circular_list_test.front", "[intrusive_circular_list_test]") {
  list l;

  auto* e1 = new element();
  auto* e2 = new element();
  auto* e3 = new element();

  CHECK(l.front() == nullptr);

  l.push_back(e1);
  CHECK(l.front() == e1);

  l.push_back(e2);
  CHECK(l.front() == e1);

  l.push_back(e3);
  CHECK(l.front() == e1);
}

TEST_CASE("intrusive_circular_list_test.back", "[intrusive_circular_list_test]") {
  list l;

  auto* e1 = new element();
  auto* e2 = new element();
  auto* e3 = new element();

  CHECK(l.back() == nullptr);

  l.push_back(e1);
  CHECK(l.back() == e1);

  l.push_back(e2);
  CHECK(l.back() == e2);

  l.push_back(e3);
  CHECK(l.back() == e3);
}

TEST_CASE("intrusive_circular_list_test.contains", "[intrusive_circular_list_test]") {
  list l;

  auto* e1 = new element();
  auto* e2 = new element();
  auto* e3 = new element();

  l.push_back(e1);
  l.push_back(e2);

  CHECK(l.contains(e1));
  CHECK(l.contains(e2));
  CHECK_FALSE(l.contains(e3));

  l.push_back(e3);
  CHECK(l.contains(e3));
}

TEST_CASE("intrusive_circular_list_test.push_back", "[intrusive_circular_list_test]") {
  list l;
  auto* e1 = new element();
  l.push_back(e1);
  assertList({e1}, l);

  auto* e2 = new element();
  l.push_back(e2);
  assertList({e1, e2}, l);

  auto* e3 = new element();
  l.push_back(e3);
  assertList({e1, e2, e3}, l);
}

TEST_CASE("intrusive_circular_list_test.remove_single_item", "[intrusive_circular_list_test]") {
  auto* e1 = new element();
  list l({e1});
  assertList({e1}, l.remove(e1));
  assertList({}, l);
}

TEST_CASE("intrusive_circular_list_test.remove_front_item", "[intrusive_circular_list_test]") {
  auto* e1 = new element();
  auto* e2 = new element();
  auto* e3 = new element();
  list l({e1, e2, e3});
  assertList({e1}, l.remove(e1));
  assertList({e2, e3}, l);
}

TEST_CASE("intrusive_circular_list_test.remove_mid_item", "[intrusive_circular_list_test]") {
  auto* e1 = new element();
  auto* e2 = new element();
  auto* e3 = new element();
  list l({e1, e2, e3});
  assertList({e2}, l.remove(e2));
  assertList({e3, e1}, l); // removal affects list head
}

TEST_CASE("intrusive_circular_list_test.remove_back_item", "[intrusive_circular_list_test]") {
  auto* e1 = new element();
  auto* e2 = new element();
  auto* e3 = new element();
  list l({e1, e2, e3});
  assertList({e3}, l.remove(e3));
  assertList({e1, e2}, l);
}

TEST_CASE("intrusive_circular_list_test.remove_single", "[intrusive_circular_list_test]") {
  auto e1_deleted = false;
  auto e2_deleted = false;
  auto e3_deleted = false;
  auto e4_deleted = false;

  list l;

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
  CHECK_FALSE(e1_deleted);
  CHECK(e2_deleted);
  CHECK_FALSE(e3_deleted);
  CHECK_FALSE(e4_deleted);
  assertList({e1, e3, e4}, l);

  // front element
  l.remove(list::iter(e3), std::next(list::iter(e3)), 1u);
  CHECK_FALSE(e1_deleted);
  CHECK(e2_deleted);
  CHECK(e3_deleted);
  CHECK_FALSE(e4_deleted);
  assertList({e1, e4}, l);

  // back element
  l.remove(list::iter(e1), std::next(list::iter(e1)), 1u);
  CHECK(e1_deleted);
  CHECK(e2_deleted);
  CHECK(e3_deleted);
  CHECK_FALSE(e4_deleted);
  assertList({e4}, l);

  // single element
  l.remove(list::iter(e4), std::next(list::iter(e4)), 1u);
  CHECK(e1_deleted);
  CHECK(e2_deleted);
  CHECK(e3_deleted);
  CHECK(e4_deleted);
  assertList({}, l);
}

TEST_CASE("intrusive_circular_list_test.remove_multiple", "[intrusive_circular_list_test]") {
  auto e1_deleted = false;
  auto e2_deleted = false;
  auto e3_deleted = false;
  auto e4_deleted = false;

  list l;

  auto* e1 = new delete_tracking_element(e1_deleted);
  auto* e2 = new delete_tracking_element(e2_deleted);
  auto* e3 = new delete_tracking_element(e3_deleted);
  auto* e4 = new delete_tracking_element(e4_deleted);

  l.push_back(e1);
  l.push_back(e2);
  l.push_back(e3);
  l.push_back(e4);

  l.remove(list::iter(e4), std::next(list::iter(e1)), 2u);
  CHECK(e1_deleted);
  CHECK_FALSE(e2_deleted);
  CHECK_FALSE(e3_deleted);
  CHECK(e4_deleted);
  assertList({e2, e3}, l);
}

TEST_CASE("intrusive_circular_list_test.remove_all", "[intrusive_circular_list_test]") {
  auto e1_deleted = false;
  auto e2_deleted = false;

  list l;

  auto* e1 = new delete_tracking_element(e1_deleted);
  auto* e2 = new delete_tracking_element(e2_deleted);

  l.push_back(e1);
  l.push_back(e2);

  l.remove(list::iter(e1), std::next(list::iter(e2)), 2u);
  CHECK(e1_deleted);
  CHECK(e2_deleted);
  assertList({}, l);
}

TEST_CASE("intrusive_circular_list_test.release_single", "[intrusive_circular_list_test]") {
  auto e1_deleted = false;
  auto e2_deleted = false;
  auto e3_deleted = false;
  auto e4_deleted = false;

  list l;

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
  CHECK_FALSE(e1_deleted);
  CHECK_FALSE(e2_deleted);
  CHECK_FALSE(e3_deleted);
  CHECK_FALSE(e4_deleted);
  assertList({e1, e3, e4}, l);
  assertLinks(e2, {e2});

  // front element
  l.release(list::iter(e3), std::next(list::iter(e3)), 1u);
  CHECK_FALSE(e1_deleted);
  CHECK_FALSE(e2_deleted);
  CHECK_FALSE(e3_deleted);
  CHECK_FALSE(e4_deleted);
  assertList({e1, e4}, l);
  assertLinks(e3, {e3});

  // back element
  l.release(list::iter(e1), std::next(list::iter(e1)), 1u);
  CHECK_FALSE(e1_deleted);
  CHECK_FALSE(e2_deleted);
  CHECK_FALSE(e3_deleted);
  CHECK_FALSE(e4_deleted);
  assertList({e4}, l);
  assertLinks(e1, {e1});

  // single element
  l.release(list::iter(e4), std::next(list::iter(e4)), 1u);
  CHECK_FALSE(e1_deleted);
  CHECK_FALSE(e2_deleted);
  CHECK_FALSE(e3_deleted);
  CHECK_FALSE(e4_deleted);
  assertList({}, l);
  assertLinks(e4, {e4});
}

TEST_CASE("intrusive_circular_list_test.release_multiple", "[intrusive_circular_list_test]") {
  auto e1_deleted = false;
  auto e2_deleted = false;
  auto e3_deleted = false;
  auto e4_deleted = false;

  list l;

  element* e1 = new delete_tracking_element(e1_deleted);
  element* e2 = new delete_tracking_element(e2_deleted);
  element* e3 = new delete_tracking_element(e3_deleted);
  element* e4 = new delete_tracking_element(e4_deleted);

  l.push_back(e1);
  l.push_back(e2);
  l.push_back(e3);
  l.push_back(e4);

  l.release(list::iter(e4), std::next(list::iter(e1)), 2u);
  CHECK_FALSE(e1_deleted);
  CHECK_FALSE(e2_deleted);
  CHECK_FALSE(e3_deleted);
  CHECK_FALSE(e4_deleted);
  assertList({e2, e3}, l);
  assertLinks(e4, {e1, e4});
}

TEST_CASE("intrusive_circular_list_test.release_all", "[intrusive_circular_list_test]") {
  auto e1_deleted = false;
  auto e2_deleted = false;

  list l;

  element* e1 = new delete_tracking_element(e1_deleted);
  element* e2 = new delete_tracking_element(e2_deleted);

  l.push_back(e1);
  l.push_back(e2);

  l.release(list::iter(e1), std::next(list::iter(e2)), 2u);
  CHECK_FALSE(e1_deleted);
  CHECK_FALSE(e2_deleted);
  assertList({}, l);
  assertLinks(e1, {e1, e2});
}

TEST_CASE("intrusive_circular_list_test.emplace_back", "[intrusive_circular_list_test]") {
  list l;

  auto* e1 = l.emplace_back();
  assertList({e1}, l);

  auto* e2 = l.emplace_back();
  assertList({e1, e2}, l);

  auto* e3 = l.emplace_back();
  assertList({e1, e2, e3}, l);
}

TEST_CASE("intrusive_circular_list_test.emplace_back_subtype", "[intrusive_circular_list_test]") {
  auto e1_deleted = false;
  {
    list l;
    auto* e1 = l.emplace_back<delete_tracking_element>(e1_deleted);
    assertList({e1}, l);
  }
  CHECK(e1_deleted);
}

TEST_CASE("intrusive_circular_list_test.reverse", "[intrusive_circular_list_test]") {
  auto* e1 = new element();
  auto* e2 = new element();
  auto* e3 = new element();
  list l({e1, e2, e3});

  l.reverse();
  assertList({e3, e2, e1}, l);
}

TEST_CASE("intrusive_circular_list_test.append_list", "[intrusive_circular_list_test]") {
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
  assertList({t1, t2, f1, f2, f3}, to);
  assertList({}, from);
}

TEST_CASE("intrusive_circular_list_test.insert_list_front", "[intrusive_circular_list_test]") {
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
  assertList({f1, f2, f3, t1, t2}, to);
  assertList({}, from);
}

TEST_CASE("intrusive_circular_list_test.insert_list_back", "[intrusive_circular_list_test]") {
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
  assertList({t1, t2, f1, f2, f3}, to);
  assertList({}, from);
}

TEST_CASE("intrusive_circular_list_test.splice_back_one_item", "[intrusive_circular_list_test]") {
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
  assertList({t1, t2, f1}, to);
  assertList({f2, f3}, from);
}

TEST_CASE("intrusive_circular_list_test.splice_back_two_items", "[intrusive_circular_list_test]") {
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
  assertList({t1, t2, f1, f2}, to);
  assertList({f3}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_one_item_into_empty_list",
  "[intrusive_circular_list_test]") {
  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  to.splice(std::begin(to), from, list::iter(f2), list::iter(f3), 1u);
  assertList({f2}, to);
  assertList({f1, f3}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_two_items_into_empty_list",
  "[intrusive_circular_list_test]") {
  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  to.splice(std::begin(to), from, list::iter(f2), list::iter(f1), 2u);
  assertList({f2, f3}, to);
  assertList({f1}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_all_items_into_empty_list",
  "[intrusive_circular_list_test]") {
  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  to.splice(std::end(to), from, std::begin(from), std::end(from), 3u);
  assertList({f1, f2, f3}, to);
  assertList({}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_one_item_into_front", "[intrusive_circular_list_test]") {
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
  assertList({f2, t1, t2, t3}, to);
  assertList({f1, f3}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_one_item_into_mid", "[intrusive_circular_list_test]") {
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
  assertList({t1, f2, t2, t3}, to);
  assertList({f1, f3}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_one_item_into_last", "[intrusive_circular_list_test]") {
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
  assertList({t1, t2, f2, t3}, to);
  assertList({f1, f3}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_last_two_items_into_front",
  "[intrusive_circular_list_test]") {
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
  assertList({f2, f3, t1, t2, t3}, to);
  assertList({f1}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_last_two_items_into_mid", "[intrusive_circular_list_test]") {
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
  assertList({t1, f2, f3, t2, t3}, to);
  assertList({f1}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_last_two_items_into_last",
  "[intrusive_circular_list_test]") {
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
  assertList({t1, t2, f2, f3, t3}, to);
  assertList({f1}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_last_and_first_items_items_front",
  "[intrusive_circular_list_test]") {
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
  assertList({f3, f1, t1, t2, t3}, to);
  assertList({f2}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_all_items_into_front", "[intrusive_circular_list_test]") {
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
  assertList({f3, f1, f2, t1, t2, t3}, to);
  assertList({}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_all_items_into_mid", "[intrusive_circular_list_test]") {
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
  assertList({t1, f3, f1, f2, t2, t3}, to);
  assertList({}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_all_items_into_last", "[intrusive_circular_list_test]") {
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
  assertList({t1, t2, f3, f1, f2, t3}, to);
  assertList({}, from);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_first_item_with_one_item",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(list::iter(t1), list::iter(t2), 1u, from, list::iter(f2), list::iter(f3), 1u);
  assertList({f2, t2, t3}, to);
  assertList({f1, f3}, from);

  CHECK(t1_deleted);
  CHECK_FALSE(t2_deleted);
  CHECK_FALSE(t3_deleted);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_mid_item_with_one_item",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(list::iter(t2), list::iter(t3), 1u, from, list::iter(f2), list::iter(f3), 1u);
  assertList({t1, f2, t3}, to);
  assertList({f1, f3}, from);

  CHECK_FALSE(t1_deleted);
  CHECK(t2_deleted);
  CHECK_FALSE(t3_deleted);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_last_item_with_one_item",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(list::iter(t3), std::end(to), 1u, from, list::iter(f2), list::iter(f3), 1u);
  assertList({t1, t2, f2}, to);
  assertList({f1, f3}, from);

  CHECK_FALSE(t1_deleted);
  CHECK_FALSE(t2_deleted);
  CHECK(t3_deleted);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_first_item_with_two_items",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(list::iter(t1), list::iter(t2), 1u, from, list::iter(f3), list::iter(f2), 2u);
  assertList({t2, t3, f3, f1}, to);
  assertList({f2}, from);

  CHECK(t1_deleted);
  CHECK_FALSE(t2_deleted);
  CHECK_FALSE(t3_deleted);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_mid_item_with_two_items",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(list::iter(t2), list::iter(t3), 1u, from, list::iter(f3), list::iter(f2), 2u);
  assertList({t1, f3, f1, t3}, to);
  assertList({f2}, from);

  CHECK_FALSE(t1_deleted);
  CHECK(t2_deleted);
  CHECK_FALSE(t3_deleted);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_last_item_with_two_items",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(list::iter(t3), list::iter(t1), 1u, from, list::iter(f3), list::iter(f2), 2u);
  assertList({t1, t2, f3, f1}, to);
  assertList({f2}, from);

  CHECK_FALSE(t1_deleted);
  CHECK_FALSE(t2_deleted);
  CHECK(t3_deleted);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_mid_item_with_all_items",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(list::iter(t2), list::iter(t3), 1u, from, list::iter(f3), list::iter(f3), 3u);
  assertList({t1, f3, f1, f2, t3}, to);
  assertList({}, from);

  CHECK_FALSE(t1_deleted);
  CHECK(t2_deleted);
  CHECK_FALSE(t3_deleted);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_first_two_items_with_two_items",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(list::iter(t1), list::iter(t3), 2u, from, list::iter(f1), list::iter(f3), 2u);
  assertList({f1, f2, t3}, to);
  assertList({f3}, from);

  CHECK(t1_deleted);
  CHECK(t2_deleted);
  CHECK_FALSE(t3_deleted);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_last_two_items_with_two_items",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(list::iter(t2), list::iter(t1), 2u, from, list::iter(f1), list::iter(f3), 2u);
  assertList({t1, f1, f2}, to);
  assertList({f3}, from);

  CHECK_FALSE(t1_deleted);
  CHECK(t2_deleted);
  CHECK(t3_deleted);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_last_and_first_items_with_two_items",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(list::iter(t3), list::iter(t2), 2u, from, list::iter(f1), list::iter(f3), 2u);
  assertList({t2, f1, f2}, to);
  assertList({f3}, from);

  CHECK(t1_deleted);
  CHECK_FALSE(t2_deleted);
  CHECK(t3_deleted);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_all_items_with_two_items",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(list::iter(t3), list::iter(t3), 3u, from, list::iter(f1), list::iter(f3), 2u);
  assertList({f1, f2}, to);
  assertList({f3}, from);

  CHECK(t1_deleted);
  CHECK(t2_deleted);
  CHECK(t3_deleted);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_all_items_with_one_item",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(list::iter(t2), list::iter(t2), 3u, from, list::iter(f1), list::iter(f2), 1u);
  assertList({f1}, to);
  assertList({f2, f3}, from);

  CHECK(t1_deleted);
  CHECK(t2_deleted);
  CHECK(t3_deleted);
}

TEST_CASE(
  "intrusive_circular_list_test.splice_replace_all_items_with_all_items",
  "[intrusive_circular_list_test]") {
  auto t1_deleted = false;
  auto t2_deleted = false;
  auto t3_deleted = false;

  list from;
  list to;

  auto* f1 = new element();
  auto* f2 = new element();
  auto* f3 = new element();

  from.push_back(f1);
  from.push_back(f2);
  from.push_back(f3);

  auto* t1 = new delete_tracking_element(t1_deleted);
  auto* t2 = new delete_tracking_element(t2_deleted);
  auto* t3 = new delete_tracking_element(t3_deleted);

  to.push_back(t1);
  to.push_back(t2);
  to.push_back(t3);

  to.splice_replace(
    std::begin(to), std::end(to), to.size(), from, std::begin(from), std::end(from), from.size());
  assertList({f1, f2, f3}, to);
  assertList({}, from);

  CHECK(t1_deleted);
  CHECK(t2_deleted);
  CHECK(t3_deleted);
}

TEST_CASE("intrusive_circular_list_test.release", "[intrusive_circular_list_test]") {
  auto e1_deleted = false;
  auto e2_deleted = false;

  list l;

  element* e1 = new delete_tracking_element(e1_deleted);
  element* e2 = new delete_tracking_element(e2_deleted);

  l.push_back(e1);
  l.push_back(e2);

  l.release();
  CHECK_FALSE(e1_deleted);
  CHECK_FALSE(e2_deleted);
  assertList({}, l);
  assertLinks(e1, {e1, e2});
}

TEST_CASE("intrusive_circular_list_test.clear_empty_list", "[intrusive_circular_list_test]") {
  list l;

  l.clear();
  assertList({}, l);
}

TEST_CASE("intrusive_circular_list_test.clear_with_items", "[intrusive_circular_list_test]") {
  auto e1_deleted = false;
  auto e2_deleted = false;

  list l;

  element* e1 = new delete_tracking_element(e1_deleted);
  element* e2 = new delete_tracking_element(e2_deleted);

  l.push_back(e1);
  l.push_back(e2);

  l.clear();
  CHECK(e1_deleted);
  CHECK(e2_deleted);
  assertList({}, l);
}
} // namespace kdl
