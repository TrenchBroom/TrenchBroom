/*
 Copyright (C) 2010 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#include "kd/intrusive_circular_list.h"

#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{
class element;
using element_link = intrusive_circular_link<element>;

class element
{
private:
  friend class get_link;
  element_link m_link;

public:
  element()
    : m_link{this}
  {
  }

  virtual ~element() = default;

  [[maybe_unused]] const element& next() const { return *m_link.next(); }

  [[maybe_unused]] const element& previous() const { return *m_link.previous(); }
};

// Tracks whether it has been destroyed, so that tests can prove that the list never
// destroys its items on its own.
class delete_tracking_element : public element
{
private:
  bool& m_deleted;

public:
  explicit delete_tracking_element(bool& deleted)
    : m_deleted(deleted)
  {
    m_deleted = false;
  }

  ~delete_tracking_element() override { m_deleted = true; }
};

class get_link
{
public:
  [[maybe_unused]] element_link& operator()(element* element) const
  {
    return element->m_link;
  }

  [[maybe_unused]] const element_link& operator()(const element* element) const
  {
    return element->m_link;
  }
};

using list = intrusive_circular_list<element, get_link>;

template <typename Item>
void assertLinks(Item* head, const std::vector<Item*>& items)
{
  CHECK((head == nullptr) == (items.empty()));

  if (head != nullptr)
  {
    const auto get_link = kdl::get_link();

    // find the front of the list
    element* list_first = head;
    while (list_first != items.front())
    {
      list_first = get_link(list_first).next();
      UNSCOPED_INFO("list head is not an item");
      CHECK(list_first != head);
    }
    element* list_cur = list_first;
    element* list_previous = get_link(list_cur).previous();

    for (std::size_t i = 0u; i < items.size(); ++i)
    {
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
void assertList(
  const std::vector<typename List::value_type>& expected, const List& actual)
{
  CHECK(expected.empty() == actual.empty());
  CHECK(expected.size() == actual.size());

  if (!actual.empty())
  {
    assertLinks(actual.front(), expected);
  }
}

} // namespace

TEST_CASE("intrusive_circular_list")
{
  SECTION("constructor")
  {
    SECTION("default")
    {
      assertList({}, list());
    }

    SECTION("initializer_list")
    {
      assertList({}, list({}));

      auto e1 = element{};
      assertList({&e1}, list({&e1}));

      auto e2 = element{};
      auto e3 = element{};
      assertList({&e2, &e3}, list({&e2, &e3}));
    }
  }

  SECTION("destructor")
  {
    auto t1_deleted = false;
    auto t2_deleted = false;
    auto t3_deleted = false;

    auto t1 = delete_tracking_element{t1_deleted};
    auto t2 = delete_tracking_element{t2_deleted};
    auto t3 = delete_tracking_element{t3_deleted};

    {
      list l;
      l.push_back(&t1);
      l.push_back(&t2);
      l.push_back(&t3);

      // l falls out of scope here; this must not affect t1, t2 or t3
    }

    CHECK(!t1_deleted);
    CHECK(!t2_deleted);
    CHECK(!t3_deleted);
  }

  SECTION("move assignment")
  {
    auto e1 = element{};
    auto e2 = element{};

    auto l1 = list{&e1, &e2};
    auto l2 = list{};

    l2 = std::move(l1);
    assertList({&e1, &e2}, l2);
    // NOLINTBEGIN(bugprone-use-after-move)
    assertList({}, l1);
    // NOLINTEND(bugprone-use-after-move)
  }

  SECTION("self-move-assignment leaves the list unchanged")
  {
    auto e1 = element{};
    auto e2 = element{};

    auto l = list{&e1, &e2};

    auto& lRef = l;
    l = std::move(lRef);

    assertList({&e1, &e2}, l);
  }

  SECTION("iterators")
  {
    list l;

    // empty list
    CHECK(l.end() == l.begin());

    auto e1 = element{};
    l.push_back(&e1);

    auto it = l.begin();
    auto end = l.end();
    CHECK(end != it);

    CHECK(*it == &e1);
    CHECK(*it++ == &e1);
    CHECK(end == it);

    auto e2 = element{};
    l.push_back(&e2);

    it = l.begin();
    end = l.end();
    CHECK(end != it);

    CHECK(*it == &e1);
    CHECK(*it++ == &e1);
    CHECK(*it == &e2);
    CHECK(*it++ == &e2);
    CHECK(end == it);
  }

  SECTION("reverse_iterators")
  {
    list l;

    // empty list
    CHECK(l.rend() == l.rbegin());

    auto e1 = element{};
    l.push_back(&e1);

    auto it = l.rbegin();
    auto end = l.rend();
    CHECK(end != it);

    CHECK(*it == &e1);
    CHECK(*it++ == &e1);
    CHECK(end == it);

    auto e2 = element{};
    auto e3 = element{};
    l.push_back(&e2);
    l.push_back(&e3);

    it = l.rbegin();
    end = l.rend();
    CHECK(end != it);

    CHECK(*it == &e3);
    CHECK(*it++ == &e3);
    CHECK(*it == &e2);
    CHECK(*it++ == &e2);
    CHECK(*it == &e1);
    CHECK(*it++ == &e1);
    CHECK(end == it);
  }

  SECTION("empty")
  {
    list l;
    CHECK(l.empty());

    auto e1 = element{};
    l.push_back(&e1);
    CHECK(!l.empty());
  }

  SECTION("size")
  {
    list l;
    CHECK(l.size() == 0u);

    auto e1 = element{};
    l.push_back(&e1);
    CHECK(l.size() == 1u);
  }

  SECTION("front")
  {
    list l;

    auto e1 = element{};
    auto e2 = element{};
    auto e3 = element{};

    CHECK(l.front() == nullptr);

    l.push_back(&e1);
    CHECK(l.front() == &e1);

    l.push_back(&e2);
    CHECK(l.front() == &e1);

    l.push_back(&e3);
    CHECK(l.front() == &e1);
  }

  SECTION("back")
  {
    list l;

    auto e1 = element{};
    auto e2 = element{};
    auto e3 = element{};

    CHECK(l.back() == nullptr);

    l.push_back(&e1);
    CHECK(l.back() == &e1);

    l.push_back(&e2);
    CHECK(l.back() == &e2);

    l.push_back(&e3);
    CHECK(l.back() == &e3);
  }

  SECTION("contains")
  {
    list l;

    auto e1 = element{};
    auto e2 = element{};
    auto e3 = element{};

    l.push_back(&e1);
    l.push_back(&e2);

    CHECK(l.contains(&e1));
    CHECK(l.contains(&e2));
    CHECK(!l.contains(&e3));

    l.push_back(&e3);
    CHECK(l.contains(&e3));
  }

  SECTION("push_back")
  {
    list l;
    auto e1 = element{};
    l.push_back(&e1);
    assertList({&e1}, l);

    auto e2 = element{};
    l.push_back(&e2);
    assertList({&e1, &e2}, l);

    auto e3 = element{};
    l.push_back(&e3);
    assertList({&e1, &e2, &e3}, l);
  }

  SECTION("remove")
  {
    SECTION("single item")
    {
      SECTION("only item")
      {
        auto e1 = element{};
        list l({&e1});
        assertList({&e1}, l.remove(&e1));
        assertList({}, l);
      }

      SECTION("front item")
      {
        auto e1 = element{};
        auto e2 = element{};
        auto e3 = element{};
        list l({&e1, &e2, &e3});
        assertList({&e1}, l.remove(&e1));
        assertList({&e2, &e3}, l);
      }

      SECTION("mid item")
      {
        auto e1 = element{};
        auto e2 = element{};
        auto e3 = element{};
        list l({&e1, &e2, &e3});
        assertList({&e2}, l.remove(&e2));
        assertList({&e3, &e1}, l); // removal affects list head
      }

      SECTION("back item")
      {
        auto e1 = element{};
        auto e2 = element{};
        auto e3 = element{};
        list l({&e1, &e2, &e3});
        assertList({&e3}, l.remove(&e3));
        assertList({&e1, &e2}, l);
      }
    }

    SECTION("range")
    {
      SECTION("single")
      {
        auto e1_deleted = false;
        auto e2_deleted = false;
        auto e3_deleted = false;
        auto e4_deleted = false;

        auto e1 = delete_tracking_element{e1_deleted};
        auto e2 = delete_tracking_element{e2_deleted};
        auto e3 = delete_tracking_element{e3_deleted};
        auto e4 = delete_tracking_element{e4_deleted};

        list l;
        l.push_back(&e1);
        l.push_back(&e2);
        l.push_back(&e3);
        l.push_back(&e4);

        // mid element -- the returned (and discarded) list must not delete it
        l.remove(list::iter(&e2), std::next(list::iter(&e2)), 1u);
        CHECK(!e1_deleted);
        CHECK(!e2_deleted);
        CHECK(!e3_deleted);
        CHECK(!e4_deleted);
        assertList({&e1, &e3, &e4}, l);

        // front element
        l.remove(list::iter(&e3), std::next(list::iter(&e3)), 1u);
        CHECK(!e1_deleted);
        CHECK(!e2_deleted);
        CHECK(!e3_deleted);
        CHECK(!e4_deleted);
        assertList({&e1, &e4}, l);

        // back element
        l.remove(list::iter(&e1), std::next(list::iter(&e1)), 1u);
        CHECK(!e1_deleted);
        CHECK(!e2_deleted);
        CHECK(!e3_deleted);
        CHECK(!e4_deleted);
        assertList({&e4}, l);

        // single element
        l.remove(list::iter(&e4), std::next(list::iter(&e4)), 1u);
        CHECK(!e1_deleted);
        CHECK(!e2_deleted);
        CHECK(!e3_deleted);
        CHECK(!e4_deleted);
        assertList({}, l);
      }

      SECTION("multiple")
      {
        auto e1_deleted = false;
        auto e2_deleted = false;
        auto e3_deleted = false;
        auto e4_deleted = false;

        auto e1 = delete_tracking_element{e1_deleted};
        auto e2 = delete_tracking_element{e2_deleted};
        auto e3 = delete_tracking_element{e3_deleted};
        auto e4 = delete_tracking_element{e4_deleted};

        list l;
        l.push_back(&e1);
        l.push_back(&e2);
        l.push_back(&e3);
        l.push_back(&e4);

        l.remove(list::iter(&e4), std::next(list::iter(&e1)), 2u);
        CHECK(!e1_deleted);
        CHECK(!e2_deleted);
        CHECK(!e3_deleted);
        CHECK(!e4_deleted);
        assertList({&e2, &e3}, l);
      }

      SECTION("all")
      {
        auto e1_deleted = false;
        auto e2_deleted = false;

        auto e1 = delete_tracking_element{e1_deleted};
        auto e2 = delete_tracking_element{e2_deleted};

        list l;
        l.push_back(&e1);
        l.push_back(&e2);

        l.remove(list::iter(&e1), std::next(list::iter(&e2)), 2u);
        CHECK(!e1_deleted);
        CHECK(!e2_deleted);
        assertList({}, l);
      }
    }
  }

  SECTION("release")
  {
    SECTION("range")
    {
      SECTION("single")
      {
        auto e1_deleted = false;
        auto e2_deleted = false;
        auto e3_deleted = false;
        auto e4_deleted = false;

        auto e1 = delete_tracking_element{e1_deleted};
        auto e2 = delete_tracking_element{e2_deleted};
        auto e3 = delete_tracking_element{e3_deleted};
        auto e4 = delete_tracking_element{e4_deleted};

        list l;
        l.push_back(&e1);
        l.push_back(&e2);
        l.push_back(&e3);
        l.push_back(&e4);

        // mid element
        l.release(list::iter(&e2), std::next(list::iter(&e2)), 1u);
        CHECK(!e1_deleted);
        CHECK(!e2_deleted);
        CHECK(!e3_deleted);
        CHECK(!e4_deleted);
        assertList({&e1, &e3, &e4}, l);
        assertLinks(&e2, {&e2});

        // front element
        l.release(list::iter(&e3), std::next(list::iter(&e3)), 1u);
        CHECK(!e1_deleted);
        CHECK(!e2_deleted);
        CHECK(!e3_deleted);
        CHECK(!e4_deleted);
        assertList({&e1, &e4}, l);
        assertLinks(&e3, {&e3});

        // back element
        l.release(list::iter(&e1), std::next(list::iter(&e1)), 1u);
        CHECK(!e1_deleted);
        CHECK(!e2_deleted);
        CHECK(!e3_deleted);
        CHECK(!e4_deleted);
        assertList({&e4}, l);
        assertLinks(&e1, {&e1});

        // single element
        l.release(list::iter(&e4), std::next(list::iter(&e4)), 1u);
        CHECK(!e1_deleted);
        CHECK(!e2_deleted);
        CHECK(!e3_deleted);
        CHECK(!e4_deleted);
        assertList({}, l);
        assertLinks(&e4, {&e4});
      }

      SECTION("multiple")
      {
        auto e1_deleted = false;
        auto e2_deleted = false;
        auto e3_deleted = false;
        auto e4_deleted = false;

        auto e1 = delete_tracking_element{e1_deleted};
        auto e2 = delete_tracking_element{e2_deleted};
        auto e3 = delete_tracking_element{e3_deleted};
        auto e4 = delete_tracking_element{e4_deleted};

        list l;
        l.push_back(&e1);
        l.push_back(&e2);
        l.push_back(&e3);
        l.push_back(&e4);

        l.release(list::iter(&e4), std::next(list::iter(&e1)), 2u);
        CHECK(!e1_deleted);
        CHECK(!e2_deleted);
        CHECK(!e3_deleted);
        CHECK(!e4_deleted);
        assertList({&e2, &e3}, l);
        assertLinks(&e4, {&e1, &e4});
      }

      SECTION("all")
      {
        auto e1_deleted = false;
        auto e2_deleted = false;

        auto e1 = delete_tracking_element{e1_deleted};
        auto e2 = delete_tracking_element{e2_deleted};

        list l;
        l.push_back(&e1);
        l.push_back(&e2);

        l.release(list::iter(&e1), std::next(list::iter(&e2)), 2u);
        CHECK(!e1_deleted);
        CHECK(!e2_deleted);
        assertList({}, l);
        assertLinks(&e1, {&e1, &e2});
      }
    }

    SECTION("whole list")
    {
      auto e1_deleted = false;
      auto e2_deleted = false;

      auto e1 = delete_tracking_element{e1_deleted};
      auto e2 = delete_tracking_element{e2_deleted};

      list l;
      l.push_back(&e1);
      l.push_back(&e2);

      l.release();
      CHECK(!e1_deleted);
      CHECK(!e2_deleted);
      assertList({}, l);
      assertLinks(&e1, {&e1, &e2});
    }
  }

  SECTION("reverse")
  {
    auto e1 = element{};
    auto e2 = element{};
    auto e3 = element{};
    list l({&e1, &e2, &e3});

    l.reverse();
    assertList({&e3, &e2, &e1}, l);
  }

  SECTION("append")
  {
    list from;
    list to;

    auto f1 = element{};
    auto f2 = element{};
    auto f3 = element{};

    from.push_back(&f1);
    from.push_back(&f2);
    from.push_back(&f3);

    auto t1 = element{};
    auto t2 = element{};

    to.push_back(&t1);
    to.push_back(&t2);

    to.append(from);
    assertList({&t1, &t2, &f1, &f2, &f3}, to);
    assertList({}, from);
  }

  SECTION("insert")
  {
    SECTION("into front")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};

      to.push_back(&t1);
      to.push_back(&t2);

      to.insert(list::iter(&t1), from);
      assertList({&f1, &f2, &f3, &t1, &t2}, to);
      assertList({}, from);
    }

    SECTION("into back")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};

      to.push_back(&t1);
      to.push_back(&t2);

      to.insert(std::end(to), from);
      assertList({&t1, &t2, &f1, &f2, &f3}, to);
      assertList({}, from);
    }
  }

  SECTION("splice_back")
  {
    SECTION("one item")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};

      to.push_back(&t1);
      to.push_back(&t2);

      to.splice_back(from, list::iter(&f1), list::iter(&f2), 1u);
      assertList({&t1, &t2, &f1}, to);
      assertList({&f2, &f3}, from);
    }

    SECTION("two items")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};

      to.push_back(&t1);
      to.push_back(&t2);

      to.splice_back(from, list::iter(&f1), list::iter(&f3), 2u);
      assertList({&t1, &t2, &f1, &f2}, to);
      assertList({&f3}, from);
    }
  }

  SECTION("splice")
  {
    SECTION("one item into empty list")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      to.splice(std::begin(to), from, list::iter(&f2), list::iter(&f3), 1u);
      assertList({&f2}, to);
      assertList({&f1, &f3}, from);
    }

    SECTION("two items into empty list")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      to.splice(std::begin(to), from, list::iter(&f2), list::iter(&f1), 2u);
      assertList({&f2, &f3}, to);
      assertList({&f1}, from);
    }

    SECTION("all items into empty list")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      to.splice(std::end(to), from, std::begin(from), std::end(from), 3u);
      assertList({&f1, &f2, &f3}, to);
      assertList({}, from);
    }

    SECTION("one item into front")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};
      auto t3 = element{};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice(list::iter(&t1), from, list::iter(&f2), list::iter(&f3), 1u);
      assertList({&f2, &t1, &t2, &t3}, to);
      assertList({&f1, &f3}, from);
    }

    SECTION("one item into mid")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};
      auto t3 = element{};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice(list::iter(&t2), from, list::iter(&f2), list::iter(&f3), 1u);
      assertList({&t1, &f2, &t2, &t3}, to);
      assertList({&f1, &f3}, from);
    }

    SECTION("one item into last")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};
      auto t3 = element{};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice(list::iter(&t3), from, list::iter(&f2), list::iter(&f3), 1u);
      assertList({&t1, &t2, &f2, &t3}, to);
      assertList({&f1, &f3}, from);
    }

    SECTION("last two items into front")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};
      auto t3 = element{};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice(list::iter(&t1), from, list::iter(&f2), list::iter(&f1), 2u);
      assertList({&f2, &f3, &t1, &t2, &t3}, to);
      assertList({&f1}, from);
    }

    SECTION("last two items into mid")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};
      auto t3 = element{};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice(list::iter(&t2), from, list::iter(&f2), std::end(from), 2u);
      assertList({&t1, &f2, &f3, &t2, &t3}, to);
      assertList({&f1}, from);
    }

    SECTION("last two items into last")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};
      auto t3 = element{};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice(list::iter(&t3), from, list::iter(&f2), std::next(list::iter(&f3)), 2u);
      assertList({&t1, &t2, &f2, &f3, &t3}, to);
      assertList({&f1}, from);
    }

    SECTION("last and first items into front")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};
      auto t3 = element{};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice(list::iter(&t1), from, list::iter(&f3), list::iter(&f2), 2u);
      assertList({&f3, &f1, &t1, &t2, &t3}, to);
      assertList({&f2}, from);
    }

    SECTION("all items into front")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};
      auto t3 = element{};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice(std::begin(to), from, list::iter(&f3), list::iter(&f3), 3u);
      assertList({&f3, &f1, &f2, &t1, &t2, &t3}, to);
      assertList({}, from);
    }

    SECTION("all items into mid")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};
      auto t3 = element{};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice(list::iter(&t2), from, list::iter(&f3), list::iter(&f3), 3u);
      assertList({&t1, &f3, &f1, &f2, &t2, &t3}, to);
      assertList({}, from);
    }

    SECTION("all items into last")
    {
      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = element{};
      auto t2 = element{};
      auto t3 = element{};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice(list::iter(&t3), from, list::iter(&f3), list::iter(&f3), 3u);
      assertList({&t1, &t2, &f3, &f1, &f2, &t3}, to);
      assertList({}, from);
    }
  }

  SECTION("splice_replace")
  {
    SECTION("first item with one item")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        list::iter(&t1), list::iter(&t2), 1u, from, list::iter(&f2), list::iter(&f3), 1u);
      assertList({&f2, &t2, &t3}, to);
      assertList({&f1, &f3}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }

    SECTION("mid item with one item")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        list::iter(&t2), list::iter(&t3), 1u, from, list::iter(&f2), list::iter(&f3), 1u);
      assertList({&t1, &f2, &t3}, to);
      assertList({&f1, &f3}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }

    SECTION("last item with one item")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        list::iter(&t3), std::end(to), 1u, from, list::iter(&f2), list::iter(&f3), 1u);
      assertList({&t1, &t2, &f2}, to);
      assertList({&f1, &f3}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }

    SECTION("first item with two items")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        list::iter(&t1), list::iter(&t2), 1u, from, list::iter(&f3), list::iter(&f2), 2u);
      assertList({&t2, &t3, &f3, &f1}, to);
      assertList({&f2}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }

    SECTION("mid item with two items")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        list::iter(&t2), list::iter(&t3), 1u, from, list::iter(&f3), list::iter(&f2), 2u);
      assertList({&t1, &f3, &f1, &t3}, to);
      assertList({&f2}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }

    SECTION("last item with two items")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        list::iter(&t3), list::iter(&t1), 1u, from, list::iter(&f3), list::iter(&f2), 2u);
      assertList({&t1, &t2, &f3, &f1}, to);
      assertList({&f2}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }

    SECTION("mid item with all items")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        list::iter(&t2), list::iter(&t3), 1u, from, list::iter(&f3), list::iter(&f3), 3u);
      assertList({&t1, &f3, &f1, &f2, &t3}, to);
      assertList({}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }

    SECTION("first two items with two items")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        list::iter(&t1), list::iter(&t3), 2u, from, list::iter(&f1), list::iter(&f3), 2u);
      assertList({&f1, &f2, &t3}, to);
      assertList({&f3}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }

    SECTION("last two items with two items")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        list::iter(&t2), list::iter(&t1), 2u, from, list::iter(&f1), list::iter(&f3), 2u);
      assertList({&t1, &f1, &f2}, to);
      assertList({&f3}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }

    SECTION("last and first items with two items")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        list::iter(&t3), list::iter(&t2), 2u, from, list::iter(&f1), list::iter(&f3), 2u);
      assertList({&t2, &f1, &f2}, to);
      assertList({&f3}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }

    SECTION("all items with two items")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        list::iter(&t3), list::iter(&t3), 3u, from, list::iter(&f1), list::iter(&f3), 2u);
      assertList({&f1, &f2}, to);
      assertList({&f3}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }

    SECTION("all items with one item")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        list::iter(&t2), list::iter(&t2), 3u, from, list::iter(&f1), list::iter(&f2), 1u);
      assertList({&f1}, to);
      assertList({&f2, &f3}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }

    SECTION("all items with all items")
    {
      auto t1_deleted = false;
      auto t2_deleted = false;
      auto t3_deleted = false;

      list from;
      list to;

      auto f1 = element{};
      auto f2 = element{};
      auto f3 = element{};

      from.push_back(&f1);
      from.push_back(&f2);
      from.push_back(&f3);

      auto t1 = delete_tracking_element{t1_deleted};
      auto t2 = delete_tracking_element{t2_deleted};
      auto t3 = delete_tracking_element{t3_deleted};

      to.push_back(&t1);
      to.push_back(&t2);
      to.push_back(&t3);

      to.splice_replace(
        std::begin(to),
        std::end(to),
        to.size(),
        from,
        std::begin(from),
        std::end(from),
        from.size());
      assertList({&f1, &f2, &f3}, to);
      assertList({}, from);

      CHECK(!t1_deleted);
      CHECK(!t2_deleted);
      CHECK(!t3_deleted);
    }
  }
}
} // namespace kdl
