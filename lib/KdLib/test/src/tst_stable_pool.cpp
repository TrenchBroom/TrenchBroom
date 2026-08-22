/*
 Copyright (C) 2026 Kristian Duske

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

#include "kd/stable_pool.h"

#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{

class element
{
private:
  int m_value;
  bool* m_deleted;

public:
  explicit element(const int value, bool* deleted = nullptr)
    : m_value{value}
    , m_deleted{deleted}
  {
  }

  ~element()
  {
    if (m_deleted != nullptr)
    {
      *m_deleted = true;
    }
  }

  element(const element&) = delete;
  element& operator=(const element&) = delete;

  int value() const { return m_value; }
};

// small page size so that tests can cheaply exercise multi-page behavior
using small_pool = stable_pool<element, 4>;

} // namespace

TEST_CASE("stable_pool")
{
  SECTION("emplace returns a usable reference")
  {
    auto pool = small_pool{};
    auto& e = pool.emplace(42);

    CHECK(e.value() == 42);
    CHECK(pool.size() == 1u);
    CHECK(!pool.empty());
  }

  SECTION("empty pool")
  {
    auto pool = small_pool{};
    CHECK(pool.empty());
    CHECK(pool.size() == 0u);
  }

  SECTION(
    "addresses remain stable across further insertions, including across a page boundary")
  {
    auto pool = small_pool{};

    auto refs = std::vector<element*>{};
    for (int i = 0; i < 10; ++i)
    {
      refs.push_back(&pool.emplace(i));
    }

    // inserting more elements, including enough to force a new page to be allocated,
    // must not invalidate any previously returned reference
    for (int i = 10; i < 20; ++i)
    {
      pool.emplace(i);
    }

    for (int i = 0; i < 10; ++i)
    {
      CHECK(refs[static_cast<std::size_t>(i)]->value() == i);
    }
  }

  SECTION("erase destroys the object and frees the slot for reuse")
  {
    auto pool = small_pool{};

    auto deleted = false;
    auto& e = pool.emplace(1, &deleted);
    CHECK(!deleted);
    CHECK(pool.size() == 1u);

    pool.erase(e);
    CHECK(deleted);
    CHECK(pool.size() == 0u);

    auto& reused = pool.emplace(2);
    CHECK(&reused == &e);
    CHECK(pool.size() == 1u);
  }

  SECTION("erasing does not affect other live elements")
  {
    auto pool = small_pool{};

    auto& e1 = pool.emplace(1);
    auto& e2 = pool.emplace(2);
    auto& e3 = pool.emplace(3);

    pool.erase(e2);

    CHECK(e1.value() == 1);
    CHECK(e3.value() == 3);
    CHECK(pool.size() == 2u);
  }

  SECTION("destroying the pool destroys any objects still alive in it")
  {
    auto deleted1 = false;
    auto deleted2 = false;

    {
      auto pool = small_pool{};
      pool.emplace(1, &deleted1);
      auto& e2 = pool.emplace(2, &deleted2);

      pool.erase(e2);
      CHECK(deleted2);
      CHECK(!deleted1);
    }

    CHECK(deleted1);
  }

  SECTION("many elements across many pages")
  {
    auto pool = small_pool{};

    constexpr auto count = 4 * 25 + 3; // several full pages plus a partial one
    auto refs = std::vector<element*>{};
    for (int i = 0; i < count; ++i)
    {
      refs.push_back(&pool.emplace(i));
    }

    CHECK(pool.size() == static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i)
    {
      CHECK(refs[static_cast<std::size_t>(i)]->value() == i);
    }
  }

  SECTION("move construction preserves element addresses")
  {
    auto pool = small_pool{};
    auto& e1 = pool.emplace(1);
    auto& e2 = pool.emplace(2);

    auto moved = small_pool{std::move(pool)};

    CHECK(e1.value() == 1);
    CHECK(e2.value() == 2);
    CHECK(moved.size() == 2u);

    // the moved-from pool must be left empty, not merely stripped of its pages
    // NOLINTBEGIN(bugprone-use-after-move)
    CHECK(pool.empty());
    CHECK(pool.size() == 0u);

    auto& e3 = moved.emplace(3);
    CHECK(e3.value() == 3);
    CHECK(moved.size() == 3u);

    // the moved-from pool must be safe to keep using without corrupting the pool it was
    // moved into
    auto& e4 = pool.emplace(4);
    CHECK(e4.value() == 4);
    CHECK(pool.size() == 1u);
    CHECK(e1.value() == 1);
    CHECK(e2.value() == 2);
    CHECK(e3.value() == 3);
    // NOLINTEND(bugprone-use-after-move)
  }

  SECTION("emplace does not leak the slot if construction throws")
  {
    struct throwing
    {
      explicit throwing(const bool shouldThrow)
      {
        if (shouldThrow)
        {
          throw std::runtime_error{"boom"};
        }
      }
    };

    auto pool = stable_pool<throwing, 4>{};

    auto& e1 = pool.emplace(false);
    pool.erase(e1);

    CHECK_THROWS_AS(pool.emplace(true), std::runtime_error);
    CHECK(pool.size() == 0u);

    // the slot that the throwing construction failed to occupy must be reusable, not lost
    auto& e2 = pool.emplace(false);
    CHECK(&e2 == &e1);
    CHECK(pool.size() == 1u);
  }

  SECTION("move assignment preserves element addresses")
  {
    auto pool = small_pool{};
    auto& e1 = pool.emplace(1);
    auto& e2 = pool.emplace(2);

    auto previousDeleted = false;
    auto moved = small_pool{};
    moved.emplace(-1, &previousDeleted);

    moved = std::move(pool);
    CHECK(previousDeleted);

    CHECK(e1.value() == 1);
    CHECK(e2.value() == 2);
    CHECK(moved.size() == 2u);

    // the moved-from pool must be left empty and safe to keep using
    // NOLINTBEGIN(bugprone-use-after-move)
    CHECK(pool.empty());
    CHECK(pool.size() == 0u);

    auto& e3 = pool.emplace(3);
    CHECK(e3.value() == 3);
    CHECK(pool.size() == 1u);
    CHECK(e1.value() == 1);
    CHECK(e2.value() == 2);
    // NOLINTEND(bugprone-use-after-move)
  }

  SECTION("self-move-assignment leaves the pool unchanged")
  {
    auto pool = small_pool{};
    auto& e1 = pool.emplace(1);
    auto& e2 = pool.emplace(2);

    auto& poolRef = pool;
    pool = std::move(poolRef);

    CHECK(pool.size() == 2u);
    CHECK(e1.value() == 1);
    CHECK(e2.value() == 2);
  }
}

} // namespace kdl
