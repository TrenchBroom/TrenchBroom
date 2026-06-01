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

#include "kd/ranges/concat_view.h"

#include <array>
#include <compare>
#include <forward_list>
#include <iterator>
#include <list>
#include <memory>
#include <ranges>
#include <sstream>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace kdl::ranges
{

// operator<=> is only available when all underlying iterators are
// three_way_comparable, which is not the case for every standard library
// implementation (e.g. libc++'s __wrap_iter on some toolchains). This must be a
// template so the if constexpr discards the (non-)comparison branch instead of
// the compiler checking the always-non-dependent expression in a plain function.
template <typename Iterator>
void checkThreeWayLessIfAvailable(const Iterator& x, const Iterator& y)
{
  if constexpr (std::three_way_comparable<Iterator>)
  {
    CHECK((x <=> y) == std::strong_ordering::less);
  }
}

template <typename T>
concept has_iterator_category = requires { typename T::iterator_category; };

TEST_CASE("concat")
{
  using Catch::Matchers::RangeEquals;

  SECTION("iterator concept - random access (vector + vector)")
  {
    const auto a = std::vector<int>{1, 2, 3};
    const auto b = std::vector<int>{4, 5};

    auto c = ranges::concat_view{a, b};
    using iter_type = decltype(c.begin());

    static_assert(
      std::is_same_v<iter_type::iterator_concept, std::random_access_iterator_tag>);
    static_assert(std::is_same_v<iter_type::value_type, int>);
    static_assert(std::random_access_iterator<iter_type>);
  }

  SECTION("iterator concept - bidirectional (list + list)")
  {
    auto a = std::list<int>{1, 2, 3};
    auto b = std::list<int>{4, 5};

    auto c = ranges::concat_view{a, b};
    using iter_type = decltype(c.begin());

    static_assert(
      std::is_same_v<iter_type::iterator_concept, std::bidirectional_iterator_tag>);
    static_assert(std::bidirectional_iterator<iter_type>);
  }

  SECTION("iterator concept - forward (forward_list + forward_list)")
  {
    auto a = std::forward_list<int>{1, 2, 3};
    auto b = std::forward_list<int>{4, 5};

    auto c = ranges::concat_view{a, b};
    using iter_type = decltype(c.begin());

    static_assert(std::is_same_v<iter_type::iterator_concept, std::forward_iterator_tag>);
  }

  SECTION("iterator concept - input (istream + vector)")
  {
    auto i = std::istringstream{"1 2 3"};
    auto iv = std::ranges::istream_view<int>(i);
    const auto v = std::vector<int>{4, 5};

    auto c = ranges::concat_view{std::move(iv), v};
    using iter_type = decltype(c.begin());

    static_assert(std::is_same_v<iter_type::iterator_concept, std::input_iterator_tag>);
    // iterator_category is present only for forward-or-better concats.
    static_assert(!has_iterator_category<iter_type>);
  }

  SECTION("iterator_category is capped when a non-last range is not common")
  {
    // take_while_view of a vector is random-access but not a common_range.
    // As a non-last input it makes concat neither random-access nor
    // bidirectional, so iterator_category must not exceed the concept.
    auto a = std::vector<int>{1, 2, 3};
    const auto b = std::vector<int>{4, 5};
    auto tw = a | std::views::take_while([](int) { return true; });
    static_assert(std::ranges::random_access_range<decltype(tw)>);
    static_assert(!std::ranges::common_range<decltype(tw)>);

    auto c = ranges::concat_view{tw, b};
    using iter_type = decltype(c.begin());

    static_assert(std::is_same_v<iter_type::iterator_concept, std::forward_iterator_tag>);
    static_assert(
      std::is_same_v<iter_type::iterator_category, std::forward_iterator_tag>);
  }

  SECTION("basic concatenation of two vectors")
  {
    const auto a = std::vector<int>{1, 2, 3};
    const auto b = std::vector<int>{4, 5, 6};

    auto c = ranges::concat_view{a, b};
    CHECK_THAT(c, RangeEquals(std::vector<int>{1, 2, 3, 4, 5, 6}));
  }

  SECTION("concatenation of three ranges of different types")
  {
    const auto a = std::vector<int>{1, 2};
    const auto b = std::array<int, 3>{3, 4, 5};
    const auto l = std::list<int>{6, 7};

    auto c = ranges::concat_view{a, b, l};
    CHECK_THAT(c, RangeEquals(std::vector<int>{1, 2, 3, 4, 5, 6, 7}));
  }

  SECTION("concatenation with empty ranges interspersed")
  {
    const auto a = std::vector<int>{};
    const auto b = std::vector<int>{1, 2};
    const auto c = std::vector<int>{};
    const auto d = std::vector<int>{3};
    const auto e = std::vector<int>{};

    auto cv = ranges::concat_view{a, b, c, d, e};
    CHECK_THAT(cv, RangeEquals(std::vector<int>{1, 2, 3}));
  }

  SECTION("concatenation where all inputs are empty")
  {
    const auto a = std::vector<int>{};
    const auto b = std::vector<int>{};

    auto c = ranges::concat_view{a, b};
    CHECK(c.begin() == c.end());
  }

  SECTION("single-element ranges concatenate correctly")
  {
    const auto a = std::vector<int>{1};
    const auto b = std::vector<int>{2};
    const auto c = std::vector<int>{3};

    auto cv = ranges::concat_view{a, b, c};
    CHECK_THAT(cv, RangeEquals(std::vector<int>{1, 2, 3}));
  }

  SECTION("size() is the sum of input sizes")
  {
    const auto a = std::vector<int>{1, 2, 3};
    const auto b = std::vector<int>{4, 5};

    auto c = ranges::concat_view{a, b};
    CHECK(c.size() == 5);
  }

  SECTION("size() works when one input is empty")
  {
    const auto a = std::vector<int>{};
    const auto b = std::vector<int>{1, 2};

    auto c = ranges::concat_view{a, b};
    CHECK(c.size() == 2);
  }

  SECTION("end() returns default_sentinel when last range is not common_range")
  {
    // take_view of a non-random-access range yields a non-common range.
    auto a = std::vector<int>{1, 2, 3};
    auto b = std::list<int>{4, 5, 6, 7};
    auto c = ranges::concat_view{a, b | std::views::take(2)};

    CHECK_THAT(c, RangeEquals(std::vector<int>{1, 2, 3, 4, 5}));

    auto it = c.begin();
    auto end = c.end();
    static_assert(!std::is_same_v<decltype(it), decltype(end)>);
    while (it != end)
    {
      ++it;
    }
    CHECK(it == end);
  }

  SECTION("forward iteration with mixed range categories")
  {
    auto a = std::list<int>{1, 2};
    auto b = std::vector<int>{3, 4};

    auto c = ranges::concat_view{a, b};
    auto out = std::vector<int>{};
    for (auto x : c)
    {
      out.push_back(x);
    }
    CHECK_THAT(out, RangeEquals(std::vector<int>{1, 2, 3, 4}));
  }

  SECTION("bidirectional iteration walks back across the boundary")
  {
    auto a = std::list<int>{1, 2};
    auto b = std::list<int>{3, 4};

    auto c = ranges::concat_view{a, b};
    auto it = c.begin();
    ++it;
    ++it;
    ++it;
    CHECK(*it == 4);

    --it;
    CHECK(*it == 3);
    --it;
    CHECK(*it == 2);
    --it;
    CHECK(*it == 1);
  }

  SECTION("bidirectional iteration walks back from end across the boundary")
  {
    auto a = std::list<int>{1, 2};
    auto b = std::list<int>{3, 4};

    auto c = ranges::concat_view{a, b};
    auto it = c.end();
    --it;
    CHECK(*it == 4);
    --it;
    CHECK(*it == 3);
    --it;
    CHECK(*it == 2);
    --it;
    CHECK(*it == 1);
  }

  SECTION("random access: operator[] indexes across boundaries")
  {
    const auto a = std::vector<int>{10, 20, 30};
    const auto b = std::vector<int>{40, 50};
    const auto cc = std::vector<int>{60};

    auto c = ranges::concat_view{a, b, cc};
    auto it = c.begin();
    CHECK(it[0] == 10);
    CHECK(it[2] == 30);
    CHECK(it[3] == 40);
    CHECK(it[4] == 50);
    CHECK(it[5] == 60);
  }

  SECTION("random access: operator+= and operator-= move within and across ranges")
  {
    const auto a = std::vector<int>{1, 2, 3};
    const auto b = std::vector<int>{4, 5, 6};
    const auto cc = std::vector<int>{7, 8};

    auto c = ranges::concat_view{a, b, cc};
    auto it = c.begin();

    it += 2;
    CHECK(*it == 3);

    it += 1;
    CHECK(*it == 4);

    it += 4;
    CHECK(*it == 8);

    it -= 5;
    CHECK(*it == 3);

    it -= 2;
    CHECK(*it == 1);

    it += 7;
    CHECK(*it == 8);
  }

  SECTION("random access: difference between iterators is element distance")
  {
    const auto a = std::vector<int>{1, 2, 3};
    const auto b = std::vector<int>{4, 5};
    const auto cc = std::vector<int>{6, 7, 8};

    auto c = ranges::concat_view{a, b, cc};
    auto first = c.begin();
    auto last = c.end();

    CHECK(last - first == 8);
    CHECK(first - last == -8);

    auto i1 = first + 1;
    auto i4 = first + 4;
    CHECK(i4 - i1 == 3);
    CHECK(i1 - i4 == -3);

    CHECK(first - first == 0);
  }

  SECTION("difference between iterator and end sentinel")
  {
    // take_view of a non-random-access range yields a sentinel that
    // is sized w.r.t. the iterator, but not the same type as the iterator —
    // forcing concat_view::end() to return default_sentinel.
    const auto a = std::vector<int>{1, 2, 3};
    auto b = std::list<int>{4, 5, 6, 7};

    auto c = ranges::concat_view{a, b | std::views::take(3)};
    auto first = c.begin();
    auto last = c.end();

    static_assert(!std::is_same_v<decltype(first), decltype(last)>);

    CHECK(last - first == 6);
    CHECK(first - last == -6);
    auto mid = first;
    ++mid;
    ++mid;
    ++mid;
    CHECK(mid - last == -3);
  }

  SECTION("random access: comparison operators")
  {
    const auto a = std::vector<int>{1, 2};
    const auto b = std::vector<int>{3, 4};

    auto c = ranges::concat_view{a, b};
    auto first = c.begin();
    auto second = first + 1;

    CHECK(first == first);
    CHECK_FALSE(first == second);
    CHECK(first < second);
    CHECK(second > first);
    CHECK(first <= first);
    CHECK(second >= second);

    checkThreeWayLessIfAvailable(first, second);
  }

  SECTION("iterator equality across the boundary")
  {
    const auto a = std::vector<int>{1, 2};
    const auto b = std::vector<int>{3, 4};

    auto c = ranges::concat_view{a, b};
    auto it1 = c.begin();
    auto it2 = c.begin();

    CHECK(it1 == it2);
    ++it1;
    CHECK_FALSE(it1 == it2);
    ++it2;
    CHECK(it1 == it2);

    // advance into second range
    ++it1;
    CHECK_FALSE(it1 == it2);
    ++it2;
    CHECK(it1 == it2);
  }

  SECTION("views::concat with multiple arguments returns a concat_view")
  {
    const auto a = std::vector<int>{1, 2};
    const auto b = std::vector<int>{3, 4};

    auto c = views::concat(a, b);
    CHECK_THAT(c, RangeEquals(std::vector<int>{1, 2, 3, 4}));
  }

  SECTION("views::concat with a single argument returns views::all(r)")
  {
    auto a = std::vector<int>{1, 2, 3};
    auto c = views::concat(a);

    CHECK_THAT(c, RangeEquals(std::vector<int>{1, 2, 3}));
    static_assert(std::is_same_v<decltype(c), std::ranges::ref_view<std::vector<int>>>);
  }

  SECTION("concatenating ranges of compatible but different element types")
  {
    const auto a = std::vector<int>{1, 2, 3};
    const auto b = std::vector<long>{4, 5};

    auto c = ranges::concat_view{a, b};
    using iter_type = decltype(c.begin());
    static_assert(std::is_same_v<iter_type::value_type, long>);

    auto out = std::vector<long>{};
    for (auto x : c)
    {
      out.push_back(x);
    }
    CHECK_THAT(out, RangeEquals(std::vector<long>{1, 2, 3, 4, 5}));
  }

  SECTION("const view is iterable")
  {
    const auto a = std::vector<int>{1, 2};
    const auto b = std::vector<int>{3, 4};

    const auto c = ranges::concat_view{a, b};

    static_assert(std::ranges::range<const decltype(c)>);
    CHECK_THAT(c, RangeEquals(std::vector<int>{1, 2, 3, 4}));
  }

  SECTION("iter_move transfers ownership of move-only elements")
  {
    auto a = std::vector<std::unique_ptr<int>>{};
    a.push_back(std::make_unique<int>(1));
    a.push_back(std::make_unique<int>(2));

    auto b = std::vector<std::unique_ptr<int>>{};
    b.push_back(std::make_unique<int>(3));

    auto c = ranges::concat_view{a, b};

    auto out = std::vector<std::unique_ptr<int>>{};
    for (auto it = c.begin(); it != c.end(); ++it)
    {
      out.push_back(std::ranges::iter_move(it));
    }

    REQUIRE(out.size() == 3);
    CHECK(*out[0] == 1);
    CHECK(*out[1] == 2);
    CHECK(*out[2] == 3);

    for (const auto& p : a)
    {
      CHECK(p == nullptr);
    }
    for (const auto& p : b)
    {
      CHECK(p == nullptr);
    }
  }

  SECTION("iter_swap on same underlying range")
  {
    auto a = std::vector<int>{1, 2, 3};
    auto b = std::vector<int>{4, 5, 6};

    auto c = ranges::concat_view{a, b};
    auto it1 = c.begin();
    auto it2 = it1 + 1;

    std::ranges::iter_swap(it1, it2);

    CHECK(a[0] == 2);
    CHECK(a[1] == 1);
    CHECK(a[2] == 3);
  }

  SECTION("iter_swap across underlying ranges")
  {
    auto a = std::vector<int>{1, 2};
    auto b = std::vector<int>{3, 4};

    auto c = ranges::concat_view{a, b};
    auto it1 = c.begin();
    auto it2 = it1 + 2;

    std::ranges::iter_swap(it1, it2);

    CHECK(a[0] == 3);
    CHECK(b[0] == 1);
  }

  SECTION("single input range")
  {
    auto a = std::vector<int>{1, 2, 3};
    auto c = ranges::concat_view{a};
    CHECK_THAT(c, RangeEquals(std::vector<int>{1, 2, 3}));
  }

  SECTION("single empty input range")
  {
    auto a = std::vector<int>{};
    auto c = ranges::concat_view{a};
    CHECK(c.begin() == c.end());
    CHECK(c.size() == 0);
  }

  SECTION("first range empty triggers satisfy<0> at begin()")
  {
    const auto a = std::vector<int>{};
    const auto b = std::vector<int>{1, 2, 3};

    auto c = ranges::concat_view{a, b};
    CHECK_THAT(c, RangeEquals(std::vector<int>{1, 2, 3}));
    CHECK(*c.begin() == 1);
  }

  SECTION("multiple leading empty ranges trigger recursive satisfy")
  {
    const auto a = std::vector<int>{};
    const auto b = std::vector<int>{};
    const auto cc = std::vector<int>{};
    const auto d = std::vector<int>{1, 2};

    auto c = ranges::concat_view{a, b, cc, d};
    CHECK_THAT(c, RangeEquals(std::vector<int>{1, 2}));
    CHECK(*c.begin() == 1);
  }

  SECTION("last range empty: forward iteration terminates correctly")
  {
    const auto a = std::vector<int>{1, 2};
    const auto b = std::vector<int>{};

    auto c = ranges::concat_view{a, b};
    CHECK_THAT(c, RangeEquals(std::vector<int>{1, 2}));
    CHECK(c.size() == 2);
  }

  SECTION("last range empty: --it from end walks back through empty range")
  {
    auto a = std::list<int>{1, 2};
    auto b = std::list<int>{};

    auto c = ranges::concat_view{a, b};
    auto it = c.end();
    --it;
    CHECK(*it == 2);
    --it;
    CHECK(*it == 1);
    CHECK(it == c.begin());
  }

  SECTION("empty range in the middle: random access operator+= steps over it")
  {
    const auto a = std::vector<int>{1, 2};
    const auto b = std::vector<int>{};
    const auto cc = std::vector<int>{3, 4};

    auto c = ranges::concat_view{a, b, cc};
    CHECK_THAT(c, RangeEquals(std::vector<int>{1, 2, 3, 4}));

    auto it = c.begin();
    it += 2;
    CHECK(*it == 3);
    it += 1;
    CHECK(*it == 4);
    it -= 2;
    CHECK(*it == 2);
    it -= 1;
    CHECK(*it == 1);
  }

  SECTION("empty range in the middle: bidirectional --it skips over it")
  {
    auto a = std::list<int>{1, 2};
    auto b = std::list<int>{};
    auto cc = std::list<int>{3, 4};

    auto c = ranges::concat_view{a, b, cc};
    auto it = c.end();
    --it;
    CHECK(*it == 4);
    --it;
    CHECK(*it == 3);
    --it;
    CHECK(*it == 2);
    --it;
    CHECK(*it == 1);
  }

  SECTION("only the first range non-empty, rest empty")
  {
    const auto a = std::vector<int>{1, 2, 3};
    const auto b = std::vector<int>{};
    const auto cc = std::vector<int>{};

    auto c = ranges::concat_view{a, b, cc};
    CHECK_THAT(c, RangeEquals(std::vector<int>{1, 2, 3}));
    CHECK(c.size() == 3);

    auto it = c.begin();
    it += 2;
    CHECK(*it == 3);
    CHECK(c.end() - it == 1);
  }

  SECTION("operator-(default_sentinel, iterator) returns positive distance")
  {
    const auto a = std::vector<int>{1, 2, 3};
    auto b = std::list<int>{4, 5, 6, 7};

    auto c = ranges::concat_view{a, b | std::views::take(3)};
    auto first = c.begin();
    auto last = c.end();

    CHECK(last - first == 6);
    auto mid = first;
    ++mid;
    ++mid;
    CHECK(last - mid == 4);
  }

  SECTION("conversion from iterator<false> to iterator<true>")
  {
    // owning_view<vector<int>> is not a simple_view, so the const and non-const
    // begin()/end() overloads yield distinct iterator specializations.
    auto c = ranges::concat_view{std::vector<int>{1, 2}, std::vector<int>{3, 4}};
    using NonConstIter = decltype(c.begin());
    using ConstIter = decltype(std::as_const(c).begin());
    static_assert(!std::is_same_v<NonConstIter, ConstIter>);
    static_assert(std::is_constructible_v<ConstIter, NonConstIter>);

    auto nci = NonConstIter{c.begin()};
    ++nci;
    auto ci = ConstIter{nci};
    CHECK(*ci == 2);
    ++ci;
    CHECK(*ci == 3);
  }

  SECTION("default-constructed iterator")
  {
    using View = ranges::concat_view<
      std::ranges::ref_view<std::vector<int>>,
      std::ranges::ref_view<std::vector<int>>>;
    using Iter = decltype(std::declval<View&>().begin());

    static_assert(std::default_initializable<Iter>);
    const auto it = Iter{};
    (void)it;
  }
}

} // namespace kdl::ranges
