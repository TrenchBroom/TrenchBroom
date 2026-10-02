/*
 Copyright (C) 2025 Kristian Duske

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

#include "kd/ranges/to.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <ranges>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{

// An allocator that counts how often it is asked to allocate memory. The count is shared
// with all copies of the allocator, including rebound ones.
template <typename T>
struct counting_allocator
{
  using value_type = T;

  explicit counting_allocator(std::size_t& count)
    : allocation_count{&count}
  {
  }

  template <typename U>
  explicit counting_allocator(const counting_allocator<U>& other)
    : allocation_count{other.allocation_count}
  {
  }

  T* allocate(const std::size_t n)
  {
    ++*allocation_count;
    return std::allocator<T>{}.allocate(n);
  }

  void deallocate(T* p, const std::size_t n) { std::allocator<T>{}.deallocate(p, n); }

  template <typename U>
  bool operator==(const counting_allocator<U>& other) const
  {
    return allocation_count == other.allocation_count;
  }

  std::size_t* allocation_count;
};

// A forward iterator that counts how often it is incremented. The count is shared with
// all copies of the iterator. Its legacy iterator category is always
// std::forward_iterator_tag, regardless of the category of the wrapped iterator.
template <std::forward_iterator I>
struct counting_forward_iterator
{
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::iter_value_t<I>;
  using difference_type = std::iter_difference_t<I>;
  using reference = std::iter_reference_t<I>;

  counting_forward_iterator() = default;

  counting_forward_iterator(I i, std::size_t& count)
    : iter{std::move(i)}
    , increment_count{&count}
  {
  }

  reference operator*() const { return *iter; }

  counting_forward_iterator& operator++()
  {
    ++iter;
    ++*increment_count;
    return *this;
  }

  counting_forward_iterator operator++(int)
  {
    auto result = *this;
    ++*this;
    return result;
  }

  bool operator==(const counting_forward_iterator& other) const
  {
    return iter == other.iter;
  }

  I iter = {};
  std::size_t* increment_count = nullptr;
};

} // namespace

TEST_CASE("to")
{
  SECTION("with specified collection type")
  {
    CHECK(ranges::to<std::list<int>>(std::vector{1, 2, 3, 3}) == std::list{1, 2, 3, 3});
    CHECK(
      ranges::to<std::vector<int>>(std::vector{1, 2, 3, 3}) == std::vector{1, 2, 3, 3});
    CHECK(ranges::to<std::set<int>>(std::vector{1, 2, 3, 3}) == std::set{1, 2, 3});
    CHECK(
      ranges::to<std::unordered_set<int>>(std::vector{1, 2, 3, 3})
      == std::unordered_set{1, 2, 3});

    CHECK(
      ranges::to<std::map<int, std::string>>(
        std::vector<std::pair<int, std::string>>{{1, "1"}, {2, "2"}, {3, "3"}, {3, "4"}})
      == std::map<int, std::string>{{1, "1"}, {2, "2"}, {3, "3"}});
    CHECK(
      ranges::to<std::unordered_map<int, std::string>>(
        std::vector<std::pair<int, std::string>>{{1, "1"}, {2, "2"}, {3, "3"}, {3, "4"}})
      == std::unordered_map<int, std::string>{{1, "1"}, {2, "2"}, {3, "3"}});
  }

  SECTION("with deduced collection type")
  {
    CHECK(ranges::to<std::list>(std::vector{1, 2, 3, 3}) == std::list{1, 2, 3, 3});
    CHECK(ranges::to<std::vector>(std::vector{1, 2, 3, 3}) == std::vector{1, 2, 3, 3});
    CHECK(ranges::to<std::set>(std::vector{1, 2, 3, 3}) == std::set{1, 2, 3});
    CHECK(
      ranges::to<std::unordered_set>(std::vector{1, 2, 3, 3})
      == std::unordered_set{1, 2, 3});

    CHECK(
      ranges::to<std::map>(
        std::vector<std::pair<int, std::string>>{{1, "1"}, {2, "2"}, {3, "3"}, {3, "4"}})
      == std::map<int, std::string>{{1, "1"}, {2, "2"}, {3, "3"}});
    CHECK(
      ranges::to<std::unordered_map>(
        std::vector<std::pair<int, std::string>>{{1, "1"}, {2, "2"}, {3, "3"}, {3, "4"}})
      == std::unordered_map<int, std::string>{{1, "1"}, {2, "2"}, {3, "3"}});
  }

  SECTION("into a container that only supports push_back")
  {
    CHECK(
      (std::string{"h e l l o"}
       | std::views::filter([](const char c) { return c != ' '; })
       | ranges::to<std::string>())
      == std::string{"hello"});

    CHECK(ranges::to<std::string>(std::vector{'a', 'b', 'c'}) == std::string{"abc"});
  }

  SECTION("range of move-only elements")
  {
    // This isn't a common range, so its elements are appended to the vector one by one
    // instead of being passed to the vector's iterator constructor.
    const auto v =
      std::views::iota(0) | std::views::take_while([](const int i) { return i < 3; })
      | std::views::transform([](const int i) { return std::make_unique<int>(i); })
      | ranges::to<std::vector>();

    CHECK(
      std::ranges::equal(v, std::vector{0, 1, 2}, {}, [](const auto& p) { return *p; }));
  }

  SECTION("range of elements that cannot be moved")
  {
    // The size of this range is known and it doesn't have random access iterators, so we
    // would prefer to reserve the vector's memory and to append the elements one by one.
    // But that doesn't compile for elements that cannot be moved, so the range must be
    // passed to the vector's iterator constructor, which constructs the elements in
    // place.
    const auto source = std::list{0, 1, 2};
    const auto v = ranges::to<std::vector<std::atomic<int>>>(source);

    CHECK(std::ranges::equal(v, source, {}, [](const auto& a) { return a.load(); }));
  }

  SECTION("nested ranges")
  {
    CHECK(
      ranges::to<std::vector<std::vector<int>>>(
        std::vector<std::vector<int>>{{1, 2}, {3}, {4, 5}})
      == std::vector<std::vector<int>>{{1, 2}, {3}, {4, 5}});

    CHECK(
      ranges::to<std::vector>(std::vector<std::vector<int>>{{1, 2}, {3}, {4, 5}})
      == std::vector<std::vector<int>>{{1, 2}, {3}, {4, 5}});

    CHECK(
      ranges::to<std::vector>(
        std::vector<std::vector<std::vector<int>>>{{{1, 2}, {3}}, {{4, 5}}})
      == std::vector<std::vector<std::vector<int>>>{{{1, 2}, {3}}, {{4, 5}}});
  }

  SECTION("call wrapper with specified collection type")
  {
    auto t = ranges::to<std::vector<int>>();
    CHECK(t(std::vector{1, 2, 3}) == std::vector{1, 2, 3});
  }

  SECTION("call wrapper with deduced collection type")
  {
    auto t = ranges::to<std::vector>();
    CHECK(t(std::vector{1, 2, 3}) == std::vector{1, 2, 3});
  }

  SECTION("pipe operator with specified collection type")
  {
    auto v = std::vector{1, 2, 3};
    SECTION("when call wrapper is an rvalue")
    {
      CHECK((v | ranges::to<std::vector<int>>()) == std::vector{1, 2, 3});
    }

    SECTION("when call wrapper is an lvalue")
    {
      auto t = ranges::to<std::vector<int>>();
      CHECK((v | t) == std::vector{1, 2, 3});
    }

    SECTION("when call wrapper is a const lvalue")
    {
      const auto t = ranges::to<std::vector<int>>();
      CHECK((v | t) == std::vector{1, 2, 3});
    }
  }

  SECTION("pipe operator with deduced collection type")
  {
    auto v = std::vector{1, 2, 3};
    CHECK((v | ranges::to<std::vector>()) == std::vector{1, 2, 3});
  }

  SECTION("call wrapper forwards extra constructor arguments")
  {
    const auto alloc = std::allocator<int>{};

    auto t1 = ranges::to<std::vector<int>>(alloc);
    CHECK(t1(std::vector{1, 2, 3}) == std::vector{1, 2, 3});

    const auto t2 = ranges::to<std::vector<int>>(alloc);
    CHECK(t2(std::vector{1, 2, 3}) == std::vector{1, 2, 3});

    CHECK(
      (std::vector{1, 2, 3} | ranges::to<std::vector<int>>(alloc))
      == std::vector{1, 2, 3});
  }

  SECTION("allocates the container's memory up front if the size of the range is known")
  {
    using Vec = std::vector<int, counting_allocator<int>>;

    // Creating a vector of a known size is the baseline: it allocates the memory for its
    // elements exactly once. We compare against this baseline instead of a fixed count
    // because some standard library implementations allocate additional bookkeeping data.
    auto expected_allocation_count = std::size_t{0};
    {
      const auto expected = Vec(100, counting_allocator<int>{expected_allocation_count});
      REQUIRE(expected.size() == 100);
    }

    auto allocation_count = std::size_t{0};
    const auto alloc = counting_allocator<int>{allocation_count};

    SECTION("range with random access iterators")
    {
      const auto source = std::vector<int>(100, 1);

      const auto v = ranges::to<Vec>(source, alloc);
      CHECK(v.size() == 100);
      CHECK(allocation_count == expected_allocation_count);
    }

    SECTION("range with forward iterators, iterating over it only once")
    {
      // The vector's iterator constructor would iterate over this range twice: once to
      // determine its size and once to copy its elements.
      const auto source = std::list<int>(100, 1);

      auto increment_count = std::size_t{0};
      const auto r = std::ranges::subrange{
        counting_forward_iterator{source.begin(), increment_count},
        counting_forward_iterator{source.end(), increment_count},
        source.size()};

      const auto v = ranges::to<Vec>(r, alloc);
      CHECK(v.size() == 100);
      CHECK(allocation_count == expected_allocation_count);
      CHECK(increment_count == 100);
    }

    // The iterators of the following views are random access iterators in terms of the
    // C++20 iterator concepts, but they report std::input_iterator_tag as their legacy
    // iterator category because dereferencing them yields a prvalue.

    SECTION("iota_view")
    {
      const auto v = ranges::to<Vec>(std::views::iota(0, 100), alloc);
      CHECK(v.size() == 100);
      CHECK(allocation_count == expected_allocation_count);
    }

    SECTION("transform_view")
    {
      const auto source = std::vector<int>(100, 1);

      const auto v = ranges::to<Vec>(
        source | std::views::transform([](const int i) { return i * 2; }), alloc);
      CHECK(v.size() == 100);
      CHECK(allocation_count == expected_allocation_count);
    }
  }
}

} // namespace kdl
