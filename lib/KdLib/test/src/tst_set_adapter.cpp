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

#include "kd/set_adapter.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace kdl
{
using namespace Catch::Matchers;

TEST_CASE("const_set_adapter")
{
  SECTION("wrap_set")
  {
    const auto v = std::vector<int>{1, 2, 3, 4};
    CHECK_THAT(wrap_set(v).get_data(), Equals(v));
  }

  SECTION("iterators")
  {
    const auto v = std::vector<int>{1, 2, 3, 4};
    const auto s = wrap_set(v);

    auto it = std::begin(s);
    for (std::size_t i = 0u; i < v.size(); ++i)
    {
      CHECK(*it == v[i]);
      ++it;
    }
    CHECK(it == std::end(s));
  }

  SECTION("reverse_iterators")
  {
    const auto v = std::vector<int>{1, 2, 3, 4};
    const auto s = wrap_set(v);

    auto it = std::rbegin(s);
    for (std::size_t i = 0u; i < v.size(); ++i)
    {
      CHECK(*it == v[v.size() - i - 1u]);
      ++it;
    }
    CHECK(it == std::rend(s));
  }

  SECTION("empty")
  {
    const auto v1 = std::vector<int>();
    CHECK(wrap_set(v1).empty());

    const auto v2 = std::vector<int>{1};
    CHECK(!wrap_set(v2).empty());
  }

  SECTION("col_total_size")
  {
    const auto v1 = std::vector<int>();
    CHECK(wrap_set(v1).size() == 0u);

    const auto v2 = std::vector<int>{1, 2};
    CHECK(wrap_set(v2).size() == 2u);
  }

  SECTION("max_size")
  {
    const auto v = std::vector<int>();
    CHECK(wrap_set(v).max_size() == v.max_size());
  }

  SECTION("count")
  {
    const auto v1 = std::vector<int>();
    CHECK(wrap_set(v1).count(1) == 0u);

    const auto v2 = std::vector<int>{1, 2, 3};
    CHECK(wrap_set(v2).count(0) == 0u);
    CHECK(wrap_set(v2).count(1) == 1u);
    CHECK(wrap_set(v2).count(2) == 1u);
    CHECK(wrap_set(v2).count(3) == 1u);
    CHECK(wrap_set(v2).count(4) == 0u);
  }

  SECTION("find")
  {
    const auto v1 = std::vector<int>();
    const auto s1 = wrap_set(v1);
    CHECK(s1.find(1) == std::end(s1));

    const auto v2 = std::vector<int>{1, 2, 3};
    const auto s2 = wrap_set(v2);
    CHECK(s2.find(0) == std::end(s2));
    CHECK(s2.find(1) == std::next(std::begin(s2), 0));
    CHECK(s2.find(2) == std::next(std::begin(s2), 1));
    CHECK(s2.find(3) == std::next(std::begin(s2), 2));
    CHECK(s2.find(4) == std::end(s2));
  }

  SECTION("equal_range")
  {
    const auto v1 = std::vector<int>();
    const auto s1 = wrap_set(v1);
    CHECK(s1.equal_range(1) == std::make_pair(std::end(s1), std::end(s1)));

    const auto v2 = std::vector<int>{1, 2, 3};
    const auto s2 = wrap_set(v2);
    CHECK(
      s2.equal_range(0)
      == std::make_pair(std::next(std::begin(s2), 0), std::next(std::begin(s2), 0)));
    CHECK(
      s2.equal_range(1)
      == std::make_pair(std::next(std::begin(s2), 0), std::next(std::begin(s2), 1)));
    CHECK(
      s2.equal_range(2)
      == std::make_pair(std::next(std::begin(s2), 1), std::next(std::begin(s2), 2)));
    CHECK(
      s2.equal_range(3)
      == std::make_pair(std::next(std::begin(s2), 2), std::next(std::begin(s2), 3)));
    CHECK(
      s2.equal_range(4)
      == std::make_pair(std::next(std::begin(s2), 3), std::next(std::begin(s2), 3)));
  }

  SECTION("lower_bound")
  {
    const auto v1 = std::vector<int>();
    const auto s1 = wrap_set(v1);
    CHECK(s1.lower_bound(1) == std::end(s1));

    const auto v2 = std::vector<int>{1, 2, 3};
    const auto s2 = wrap_set(v2);
    CHECK(s2.lower_bound(0) == std::next(std::begin(s2), 0));
    CHECK(s2.lower_bound(1) == std::next(std::begin(s2), 0));
    CHECK(s2.lower_bound(2) == std::next(std::begin(s2), 1));
    CHECK(s2.lower_bound(3) == std::next(std::begin(s2), 2));
    CHECK(s2.lower_bound(4) == std::next(std::begin(s2), 3));
  }

  SECTION("upper_bound")
  {
    const auto v1 = std::vector<int>();
    const auto s1 = wrap_set(v1);
    CHECK(s1.upper_bound(1) == std::end(s1));

    const auto v2 = std::vector<int>{1, 2, 3};
    const auto s2 = wrap_set(v2);
    CHECK(s2.upper_bound(0) == std::next(std::begin(s2), 0));
    CHECK(s2.upper_bound(1) == std::next(std::begin(s2), 1));
    CHECK(s2.upper_bound(2) == std::next(std::begin(s2), 2));
    CHECK(s2.upper_bound(3) == std::next(std::begin(s2), 3));
    CHECK(s2.upper_bound(4) == std::next(std::begin(s2), 3));
  }

  SECTION("capacity")
  {
    const auto v1 = std::vector<int>();
    const auto s1 = wrap_set(v1);
    CHECK(s1.capacity() == v1.capacity());
  }

  SECTION("get_data")
  {
    const auto v1 = std::vector<int>();
    const auto s1 = wrap_set(v1);
    const auto& d = s1.get_data();
    CHECK(&d == &v1);
  }

  SECTION("operator==")
  {
    CHECK(wrap_set(std::vector<int>({})) == wrap_set(std::vector<int>({})));
    CHECK(wrap_set(std::vector<int>({1, 2, 3})) == wrap_set(std::vector<int>({1, 2, 3})));
    CHECK(!(wrap_set(std::vector<int>({2, 3})) == wrap_set(std::vector<int>({1, 2, 3}))));
    CHECK(!(wrap_set(std::vector<int>({3})) == wrap_set(std::vector<int>({1, 2, 3}))));
    CHECK(!(wrap_set(std::vector<int>({1, 2, 3})) == wrap_set(std::vector<int>({2, 3}))));
    CHECK(!(wrap_set(std::vector<int>({1, 2, 3})) == wrap_set(std::vector<int>({3}))));
  }

  SECTION("operator!=")
  {
    CHECK(!(wrap_set(std::vector<int>({})) != wrap_set(std::vector<int>({}))));
    CHECK(
      !(wrap_set(std::vector<int>({1, 2, 3})) != wrap_set(std::vector<int>({1, 2, 3}))));
    CHECK(wrap_set(std::vector<int>({2, 3})) != wrap_set(std::vector<int>({1, 2, 3})));
    CHECK(wrap_set(std::vector<int>({3})) != wrap_set(std::vector<int>({1, 2, 3})));
    CHECK(wrap_set(std::vector<int>({1, 2, 3})) != wrap_set(std::vector<int>({2, 3})));
    CHECK(wrap_set(std::vector<int>({1, 2, 3})) != wrap_set(std::vector<int>({3})));
  }

  SECTION("operator<")
  {
    CHECK(!(wrap_set(std::vector<int>({})) < wrap_set(std::vector<int>({}))));
    CHECK(wrap_set(std::vector<int>({})) < wrap_set(std::vector<int>({1})));
    CHECK(wrap_set(std::vector<int>({})) < wrap_set(std::vector<int>({1, 2})));
    CHECK(wrap_set(std::vector<int>({})) < wrap_set(std::vector<int>({1, 2, 3})));
    CHECK(wrap_set(std::vector<int>({1})) < wrap_set(std::vector<int>({1, 2, 3})));
    CHECK(wrap_set(std::vector<int>({1, 2})) < wrap_set(std::vector<int>({1, 2, 3})));
    CHECK(
      !(wrap_set(std::vector<int>({1, 2, 3})) < wrap_set(std::vector<int>({1, 2, 3}))));
    CHECK(!(wrap_set(std::vector<int>({1, 2, 3})) < wrap_set(std::vector<int>({1, 2}))));
    CHECK(wrap_set(std::vector<int>({1, 2, 3})) < wrap_set(std::vector<int>({2, 3})));
  }

  SECTION("operator<=")
  {
    CHECK(wrap_set(std::vector<int>({})) <= wrap_set(std::vector<int>({})));
    CHECK(wrap_set(std::vector<int>({})) <= wrap_set(std::vector<int>({1})));
    CHECK(wrap_set(std::vector<int>({})) <= wrap_set(std::vector<int>({1, 2})));
    CHECK(wrap_set(std::vector<int>({})) <= wrap_set(std::vector<int>({1, 2, 3})));
    CHECK(wrap_set(std::vector<int>({1})) <= wrap_set(std::vector<int>({1, 2, 3})));
    CHECK(wrap_set(std::vector<int>({1, 2})) <= wrap_set(std::vector<int>({1, 2, 3})));
    CHECK(wrap_set(std::vector<int>({1, 2, 3})) <= wrap_set(std::vector<int>({1, 2, 3})));
    CHECK(!(wrap_set(std::vector<int>({1, 2, 3})) <= wrap_set(std::vector<int>({1, 2}))));
    CHECK(wrap_set(std::vector<int>({1, 2, 3})) <= wrap_set(std::vector<int>({2, 3})));
  }

  SECTION("operator>")
  {
    CHECK(!(wrap_set(std::vector<int>({})) > wrap_set(std::vector<int>({}))));
    CHECK(!(wrap_set(std::vector<int>({})) > wrap_set(std::vector<int>({1}))));
    CHECK(!(wrap_set(std::vector<int>({})) > wrap_set(std::vector<int>({1, 2}))));
    CHECK(!(wrap_set(std::vector<int>({})) > wrap_set(std::vector<int>({1, 2, 3}))));
    CHECK(!(wrap_set(std::vector<int>({1})) > wrap_set(std::vector<int>({1, 2, 3}))));
    CHECK(!(wrap_set(std::vector<int>({1, 2})) > wrap_set(std::vector<int>({1, 2, 3}))));
    CHECK(
      !(wrap_set(std::vector<int>({1, 2, 3})) > wrap_set(std::vector<int>({1, 2, 3}))));
    CHECK(wrap_set(std::vector<int>({1, 2, 3})) > wrap_set(std::vector<int>({1, 2})));
    CHECK(!(wrap_set(std::vector<int>({1, 2, 3})) > wrap_set(std::vector<int>({2, 3}))));
  }

  SECTION("operator>=")
  {
    CHECK(wrap_set(std::vector<int>({})) >= wrap_set(std::vector<int>({})));
    CHECK(!(wrap_set(std::vector<int>({})) >= wrap_set(std::vector<int>({1}))));
    CHECK(!(wrap_set(std::vector<int>({})) >= wrap_set(std::vector<int>({1, 2}))));
    CHECK(!(wrap_set(std::vector<int>({})) >= wrap_set(std::vector<int>({1, 2, 3}))));
    CHECK(!(wrap_set(std::vector<int>({1})) >= wrap_set(std::vector<int>({1, 2, 3}))));
    CHECK(!(wrap_set(std::vector<int>({1, 2})) >= wrap_set(std::vector<int>({1, 2, 3}))));
    CHECK(wrap_set(std::vector<int>({1, 2, 3})) >= wrap_set(std::vector<int>({1, 2, 3})));
    CHECK(wrap_set(std::vector<int>({1, 2, 3})) >= wrap_set(std::vector<int>({1, 2})));
    CHECK(!(wrap_set(std::vector<int>({1, 2, 3})) >= wrap_set(std::vector<int>({2, 3}))));
  }
}

TEST_CASE("set_adapter")
{
  SECTION("wrap_set")
  {
    auto v = std::vector<int>{1, 2, 3};
    CHECK_THAT(wrap_set(v).get_data(), Equals(v));
  }

  SECTION("create_set")
  {
    auto v = std::vector<int>{1, 2, 3, 2, 5};
    CHECK_THAT(create_set(v).get_data(), Equals(std::vector<int>{1, 2, 3, 5}));
  }

  SECTION("operator=")
  {
    auto v = std::vector<int>{1, 2, 3, 2, 5};
    auto s = create_set(v);

    s = {5, 6, 7, 6, 3};
    CHECK_THAT(s.get_data(), Equals(std::vector<int>{3, 5, 6, 7}));
  }

  SECTION("clear")
  {
    auto v = std::vector<int>{1, 2, 3};
    auto s = wrap_set(v);

    s.clear();
    CHECK(s.empty());
    CHECK(v.empty());
  }

  SECTION("insert")
  {
    SECTION("with value")
    {
      auto v = std::vector<int>();
      auto s = wrap_set(v);

      auto [it, success] = s.insert(2);
      CHECK(success);
      CHECK(it == std::begin(s));

      std::tie(it, success) = s.insert(2);
      CHECK(!success);
      CHECK(it == std::begin(s));

      std::tie(it, success) = s.insert(1);
      CHECK(success);
      CHECK(it == std::begin(s));

      std::tie(it, success) = s.insert(2);
      CHECK(!success);
      CHECK(it == std::next(std::begin(s), 1));

      std::tie(it, success) = s.insert(3);
      CHECK(success);
      CHECK(it == std::next(std::begin(s), 2));

      CHECK_THAT(v, Equals(std::vector<int>{1, 2, 3}));
    }

    SECTION("with value and hint")
    {
      auto v = std::vector<int>();
      auto s = wrap_set(v);

      auto it = s.insert(std::end(s), 2);
      CHECK(it == std::begin(s));

      it = s.insert(s.upper_bound(1), 1);
      CHECK(it == std::begin(s));

      it = s.insert(s.upper_bound(2), 2);
      CHECK(it == std::next(std::begin(s), 1));

      it = s.insert(s.begin(), 3); // wrong hint, must still work
      CHECK(it == std::next(std::begin(s), 2));

      CHECK_THAT(v, Equals(std::vector<int>{1, 2, 3}));
    }

    SECTION("with range")
    {
      auto v = std::vector<int>();
      auto s = wrap_set(v);

      const auto r = std::vector<int>{4, 2, 2, 3, 4, 1};
      s.insert(std::begin(r), std::end(r));

      CHECK_THAT(v, Equals(std::vector<int>{1, 2, 3, 4}));
    }

    SECTION("with range and count")
    {
      auto v = std::vector<int>();
      auto s = wrap_set(v);

      const auto r = std::vector<int>{4, 2, 2, 3, 4, 1};
      s.insert(r.size(), std::begin(r), std::end(r));

      CHECK_THAT(v, Equals(std::vector<int>{1, 2, 3, 4}));
    }
  }

  SECTION("emplace")
  {
    auto v = std::vector<int>();
    auto s = wrap_set(v);

    // emplace must create the value in any case for comparison, so there is no point in
    // checking whether or not a value was created

    auto [it, success] = s.emplace(2);
    CHECK(success);
    CHECK(it == std::begin(s));

    std::tie(it, success) = s.emplace(2);
    CHECK(!success);
    CHECK(it == std::begin(s));

    std::tie(it, success) = s.emplace(1);
    CHECK(success);
    CHECK(it == std::begin(s));

    std::tie(it, success) = s.emplace(2);
    CHECK(!success);
    CHECK(it == std::next(std::begin(s), 1));

    std::tie(it, success) = s.emplace(3);
    CHECK(success);
    CHECK(it == std::next(std::begin(s), 2));

    CHECK_THAT(v, Equals(std::vector<int>{1, 2, 3}));
  }

  SECTION("emplace_hint")
  {
    auto v = std::vector<int>();
    auto s = wrap_set(v);

    // emplace must create the value in any case for comparison, so there is no point in
    // checking whether or not a value was created

    auto it = s.emplace_hint(std::end(s), 2);
    CHECK(it == std::begin(s));

    it = s.emplace_hint(s.upper_bound(1), 1);
    CHECK(it == std::begin(s));

    it = s.emplace_hint(s.upper_bound(2), 2);
    CHECK(it == std::next(std::begin(s), 1));

    it = s.emplace_hint(s.begin(), 3); // wrong hint, must still work
    CHECK(it == std::next(std::begin(s), 2));

    CHECK_THAT(v, Equals(std::vector<int>{1, 2, 3}));
  }

  SECTION("erase")
  {
    SECTION("with iterator")
    {
      auto v = std::vector<int>{1, 2, 3};
      auto s = wrap_set(v);

      s.erase(std::next(std::begin(s), 1));
      CHECK_THAT(v, Equals(std::vector<int>{1, 3}));

      s.erase(std::next(std::begin(s), 1));
      CHECK_THAT(v, Equals(std::vector<int>{1}));

      s.erase(std::next(std::begin(s), 0));
      CHECK_THAT(v, Equals(std::vector<int>{}));
    }

    SECTION("with range")
    {
      auto v = std::vector<int>{1, 2, 3};
      auto s = wrap_set(v);

      auto it = s.erase(std::next(std::begin(s), 0), std::next(std::begin(s), 2));
      CHECK(it == std::next(std::begin(s), 0));
      CHECK_THAT(v, Equals(std::vector<int>{3}));

      it = s.erase(std::next(std::begin(s), 0), std::next(std::begin(s), 1));
      CHECK(it == std::end(s));
      CHECK_THAT(v, Equals(std::vector<int>{}));
    }

    SECTION("with value")
    {
      auto v = std::vector<int>{1, 2, 3};
      auto s = wrap_set(v);

      CHECK(s.erase(4) == 0u);
      CHECK_THAT(v, Equals(std::vector<int>{1, 2, 3}));

      CHECK(s.erase(2) == 1u);
      CHECK_THAT(v, Equals(std::vector<int>{1, 3}));

      CHECK(s.erase(3) == 1u);
      CHECK_THAT(v, Equals(std::vector<int>{1}));

      CHECK(s.erase(1) == 1u);
      CHECK_THAT(v, Equals(std::vector<int>{}));

      CHECK(s.erase(1) == 0u);
      CHECK_THAT(v, Equals(std::vector<int>{}));
    }
  }

  SECTION("swap")
  {
    // swap only works if the underlying collection is stored by value
    auto s = set_adapter<std::vector<int>, std::less<int>>(std::vector<int>({1, 2, 3}));
    auto t = set_adapter<std::vector<int>, std::less<int>>(std::vector<int>({4, 5}));

    REQUIRE_THAT(s.get_data(), Equals(std::vector<int>{1, 2, 3}));
    REQUIRE_THAT(t.get_data(), Equals(std::vector<int>{4, 5}));

    using std::swap;
    swap(s, t);

    CHECK_THAT(s.get_data(), Equals(std::vector<int>{4, 5}));
    CHECK_THAT(t.get_data(), Equals(std::vector<int>{1, 2, 3}));
  }

  SECTION("release_data")
  {
    auto v = std::vector<int>{1, 2, 3};
    auto s = wrap_set(v);

    auto w = s.release_data();
    CHECK_THAT(w, Equals(std::vector<int>{1, 2, 3}));
    CHECK(s.empty());
    CHECK(v.empty());
  }
}

} // namespace kdl
