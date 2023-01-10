/*
 Copyright 2010-2019 Kristian Duske

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

#include "kdl/set_adapter.h"

#include <vector>

#include <catch2/catch.hpp>

namespace kdl
{
TEST_CASE("const_set_adapter_test.wrap_set")
{
  const auto v = std::vector<int>{1, 2, 3, 4};
  CHECK_THAT(wrap_set(v).get_data(), Catch::Equals(v));
}

TEST_CASE("const_set_adapter_test.iterators")
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

TEST_CASE("const_set_adapter_test.reverse_iterators")
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

TEST_CASE("const_set_adapter_test.empty")
{
  const auto v1 = std::vector<int>();
  CHECK(wrap_set(v1).empty());

  const auto v2 = std::vector<int>{1};
  CHECK_FALSE(wrap_set(v2).empty());
}

TEST_CASE("const_set_adapter_test.col_total_size")
{
  const auto v1 = std::vector<int>();
  CHECK(wrap_set(v1).size() == 0u);

  const auto v2 = std::vector<int>{1, 2};
  CHECK(wrap_set(v2).size() == 2u);
}

TEST_CASE("const_set_adapter_test.max_size")
{
  const auto v = std::vector<int>();
  CHECK(wrap_set(v).max_size() == v.max_size());
}

TEST_CASE("const_set_adapter_test.count")
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

TEST_CASE("const_set_adapter_test.find")
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

TEST_CASE("const_set_adapter_test.equal_range")
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

TEST_CASE("const_set_adapter_test.lower_bound")
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

TEST_CASE("const_set_adapter_test.upper_bound")
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

TEST_CASE("const_set_adapter_test.capacity")
{
  const auto v1 = std::vector<int>();
  const auto s1 = wrap_set(v1);
  CHECK(s1.capacity() == v1.capacity());
}

TEST_CASE("const_set_adapter_test.get_data")
{
  const auto v1 = std::vector<int>();
  const auto s1 = wrap_set(v1);
  const auto& d = s1.get_data();
  CHECK(&d == &v1);
}

TEST_CASE("const_set_adapter_test.operator_equal")
{
  CHECK(wrap_set(std::vector<int>({})) == wrap_set(std::vector<int>({})));
  CHECK(wrap_set(std::vector<int>({1, 2, 3})) == wrap_set(std::vector<int>({1, 2, 3})));
  CHECK_FALSE(
    wrap_set(std::vector<int>({2, 3})) == wrap_set(std::vector<int>({1, 2, 3})));
  CHECK_FALSE(wrap_set(std::vector<int>({3})) == wrap_set(std::vector<int>({1, 2, 3})));
  CHECK_FALSE(
    wrap_set(std::vector<int>({1, 2, 3})) == wrap_set(std::vector<int>({2, 3})));
  CHECK_FALSE(wrap_set(std::vector<int>({1, 2, 3})) == wrap_set(std::vector<int>({3})));
}

TEST_CASE("const_set_adapter_test.operator_not_equal")
{
  CHECK_FALSE(wrap_set(std::vector<int>({})) != wrap_set(std::vector<int>({})));
  CHECK_FALSE(
    wrap_set(std::vector<int>({1, 2, 3})) != wrap_set(std::vector<int>({1, 2, 3})));
  CHECK(wrap_set(std::vector<int>({2, 3})) != wrap_set(std::vector<int>({1, 2, 3})));
  CHECK(wrap_set(std::vector<int>({3})) != wrap_set(std::vector<int>({1, 2, 3})));
  CHECK(wrap_set(std::vector<int>({1, 2, 3})) != wrap_set(std::vector<int>({2, 3})));
  CHECK(wrap_set(std::vector<int>({1, 2, 3})) != wrap_set(std::vector<int>({3})));
}

TEST_CASE("const_set_adapter_test.operator_less_than")
{
  CHECK_FALSE(wrap_set(std::vector<int>({})) < wrap_set(std::vector<int>({})));
  CHECK(wrap_set(std::vector<int>({})) < wrap_set(std::vector<int>({1})));
  CHECK(wrap_set(std::vector<int>({})) < wrap_set(std::vector<int>({1, 2})));
  CHECK(wrap_set(std::vector<int>({})) < wrap_set(std::vector<int>({1, 2, 3})));
  CHECK(wrap_set(std::vector<int>({1})) < wrap_set(std::vector<int>({1, 2, 3})));
  CHECK(wrap_set(std::vector<int>({1, 2})) < wrap_set(std::vector<int>({1, 2, 3})));
  CHECK_FALSE(
    wrap_set(std::vector<int>({1, 2, 3})) < wrap_set(std::vector<int>({1, 2, 3})));
  CHECK_FALSE(wrap_set(std::vector<int>({1, 2, 3})) < wrap_set(std::vector<int>({1, 2})));
  CHECK(wrap_set(std::vector<int>({1, 2, 3})) < wrap_set(std::vector<int>({2, 3})));
}

TEST_CASE(
  "const_set_adapter_test.operator_less_than_or_equal", "[const_set_adapter_test]")
{
  CHECK(wrap_set(std::vector<int>({})) <= wrap_set(std::vector<int>({})));
  CHECK(wrap_set(std::vector<int>({})) <= wrap_set(std::vector<int>({1})));
  CHECK(wrap_set(std::vector<int>({})) <= wrap_set(std::vector<int>({1, 2})));
  CHECK(wrap_set(std::vector<int>({})) <= wrap_set(std::vector<int>({1, 2, 3})));
  CHECK(wrap_set(std::vector<int>({1})) <= wrap_set(std::vector<int>({1, 2, 3})));
  CHECK(wrap_set(std::vector<int>({1, 2})) <= wrap_set(std::vector<int>({1, 2, 3})));
  CHECK(wrap_set(std::vector<int>({1, 2, 3})) <= wrap_set(std::vector<int>({1, 2, 3})));
  CHECK_FALSE(
    wrap_set(std::vector<int>({1, 2, 3})) <= wrap_set(std::vector<int>({1, 2})));
  CHECK(wrap_set(std::vector<int>({1, 2, 3})) <= wrap_set(std::vector<int>({2, 3})));
}

TEST_CASE("const_set_adapter_test.operator_greater_than")
{
  CHECK_FALSE(wrap_set(std::vector<int>({})) > wrap_set(std::vector<int>({})));
  CHECK_FALSE(wrap_set(std::vector<int>({})) > wrap_set(std::vector<int>({1})));
  CHECK_FALSE(wrap_set(std::vector<int>({})) > wrap_set(std::vector<int>({1, 2})));
  CHECK_FALSE(wrap_set(std::vector<int>({})) > wrap_set(std::vector<int>({1, 2, 3})));
  CHECK_FALSE(wrap_set(std::vector<int>({1})) > wrap_set(std::vector<int>({1, 2, 3})));
  CHECK_FALSE(wrap_set(std::vector<int>({1, 2})) > wrap_set(std::vector<int>({1, 2, 3})));
  CHECK_FALSE(
    wrap_set(std::vector<int>({1, 2, 3})) > wrap_set(std::vector<int>({1, 2, 3})));
  CHECK(wrap_set(std::vector<int>({1, 2, 3})) > wrap_set(std::vector<int>({1, 2})));
  CHECK_FALSE(wrap_set(std::vector<int>({1, 2, 3})) > wrap_set(std::vector<int>({2, 3})));
}

TEST_CASE(
  "const_set_adapter_test.operator_greater_than_or_equal", "[const_set_adapter_test]")
{
  CHECK(wrap_set(std::vector<int>({})) >= wrap_set(std::vector<int>({})));
  CHECK_FALSE(wrap_set(std::vector<int>({})) >= wrap_set(std::vector<int>({1})));
  CHECK_FALSE(wrap_set(std::vector<int>({})) >= wrap_set(std::vector<int>({1, 2})));
  CHECK_FALSE(wrap_set(std::vector<int>({})) >= wrap_set(std::vector<int>({1, 2, 3})));
  CHECK_FALSE(wrap_set(std::vector<int>({1})) >= wrap_set(std::vector<int>({1, 2, 3})));
  CHECK_FALSE(
    wrap_set(std::vector<int>({1, 2})) >= wrap_set(std::vector<int>({1, 2, 3})));
  CHECK(wrap_set(std::vector<int>({1, 2, 3})) >= wrap_set(std::vector<int>({1, 2, 3})));
  CHECK(wrap_set(std::vector<int>({1, 2, 3})) >= wrap_set(std::vector<int>({1, 2})));
  CHECK_FALSE(
    wrap_set(std::vector<int>({1, 2, 3})) >= wrap_set(std::vector<int>({2, 3})));
}

TEST_CASE("set_adapter_test.wrap_set")
{
  auto v = std::vector<int>{1, 2, 3};
  CHECK_THAT(wrap_set(v).get_data(), Catch::Equals(v));
}

TEST_CASE("set_adapter_test.create_set")
{
  auto v = std::vector<int>{1, 2, 3, 2, 5};
  CHECK_THAT(create_set(v).get_data(), Catch::Equals(std::vector<int>{1, 2, 3, 5}));
}

TEST_CASE("set_adapter_test.operator_assign_with_initializer_list")
{
  auto v = std::vector<int>{1, 2, 3, 2, 5};
  auto s = create_set(v);

  s = {5, 6, 7, 6, 3};
  CHECK_THAT(s.get_data(), Catch::Equals(std::vector<int>{3, 5, 6, 7}));
}

TEST_CASE("set_adapter_test.clear")
{
  auto v = std::vector<int>{1, 2, 3};
  auto s = wrap_set(v);

  s.clear();
  CHECK(s.empty());
  CHECK(v.empty());
}

TEST_CASE("set_adapter_test.insert_with_value")
{
  auto v = std::vector<int>();
  auto s = wrap_set(v);

  auto [it, success] = s.insert(2);
  CHECK(success);
  CHECK(it == std::begin(s));

  std::tie(it, success) = s.insert(2);
  CHECK_FALSE(success);
  CHECK(it == std::begin(s));

  std::tie(it, success) = s.insert(1);
  CHECK(success);
  CHECK(it == std::begin(s));

  std::tie(it, success) = s.insert(2);
  CHECK_FALSE(success);
  CHECK(it == std::next(std::begin(s), 1));

  std::tie(it, success) = s.insert(3);
  CHECK(success);
  CHECK(it == std::next(std::begin(s), 2));

  CHECK_THAT(v, Catch::Equals(std::vector<int>{1, 2, 3}));
}

TEST_CASE("set_adapter_test.insert_with_value_and_hint")
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

  CHECK_THAT(v, Catch::Equals(std::vector<int>{1, 2, 3}));
}

TEST_CASE("set_adapter_test.insert_with_range")
{
  auto v = std::vector<int>();
  auto s = wrap_set(v);

  const auto r = std::vector<int>{4, 2, 2, 3, 4, 1};
  s.insert(std::begin(r), std::end(r));

  CHECK_THAT(v, Catch::Equals(std::vector<int>{1, 2, 3, 4}));
}

TEST_CASE("set_adapter_test.insert_with_range_and_count")
{
  auto v = std::vector<int>();
  auto s = wrap_set(v);

  const auto r = std::vector<int>{4, 2, 2, 3, 4, 1};
  s.insert(r.size(), std::begin(r), std::end(r));

  CHECK_THAT(v, Catch::Equals(std::vector<int>{1, 2, 3, 4}));
}

TEST_CASE("set_adapter_test.emplace")
{
  auto v = std::vector<int>();
  auto s = wrap_set(v);

  // emplace must create the value in any case for comparison, so there is no point in
  // checking whether or not a value was created

  auto [it, success] = s.emplace(2);
  CHECK(success);
  CHECK(it == std::begin(s));

  std::tie(it, success) = s.emplace(2);
  CHECK_FALSE(success);
  CHECK(it == std::begin(s));

  std::tie(it, success) = s.emplace(1);
  CHECK(success);
  CHECK(it == std::begin(s));

  std::tie(it, success) = s.emplace(2);
  CHECK_FALSE(success);
  CHECK(it == std::next(std::begin(s), 1));

  std::tie(it, success) = s.emplace(3);
  CHECK(success);
  CHECK(it == std::next(std::begin(s), 2));

  CHECK_THAT(v, Catch::Equals(std::vector<int>{1, 2, 3}));
}

TEST_CASE("set_adapter_test.emplace_hint")
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

  CHECK_THAT(v, Catch::Equals(std::vector<int>{1, 2, 3}));
}

TEST_CASE("set_adapter_test.erase_with_iterator")
{
  auto v = std::vector<int>{1, 2, 3};
  auto s = wrap_set(v);

  s.erase(std::next(std::begin(s), 1));
  CHECK_THAT(v, Catch::Equals(std::vector<int>{1, 3}));

  s.erase(std::next(std::begin(s), 1));
  CHECK_THAT(v, Catch::Equals(std::vector<int>{1}));

  s.erase(std::next(std::begin(s), 0));
  CHECK_THAT(v, Catch::Equals(std::vector<int>{}));
}

TEST_CASE("set_adapter_test.erase_with_range")
{
  auto v = std::vector<int>{1, 2, 3};
  auto s = wrap_set(v);

  auto it = s.erase(std::next(std::begin(s), 0), std::next(std::begin(s), 2));
  CHECK(it == std::next(std::begin(s), 0));
  CHECK_THAT(v, Catch::Equals(std::vector<int>{3}));

  it = s.erase(std::next(std::begin(s), 0), std::next(std::begin(s), 1));
  CHECK(it == std::end(s));
  CHECK_THAT(v, Catch::Equals(std::vector<int>{}));
}

TEST_CASE("set_adapter_test.erase_with_value")
{
  auto v = std::vector<int>{1, 2, 3};
  auto s = wrap_set(v);

  CHECK(s.erase(4) == 0u);
  CHECK_THAT(v, Catch::Equals(std::vector<int>{1, 2, 3}));

  CHECK(s.erase(2) == 1u);
  CHECK_THAT(v, Catch::Equals(std::vector<int>{1, 3}));

  CHECK(s.erase(3) == 1u);
  CHECK_THAT(v, Catch::Equals(std::vector<int>{1}));

  CHECK(s.erase(1) == 1u);
  CHECK_THAT(v, Catch::Equals(std::vector<int>{}));

  CHECK(s.erase(1) == 0u);
  CHECK_THAT(v, Catch::Equals(std::vector<int>{}));
}

TEST_CASE("set_adapter_test.swap")
{
  // swap only works if the underlying collection is stored by value
  auto s = set_adapter<std::vector<int>, std::less<int>>(std::vector<int>({1, 2, 3}));
  auto t = set_adapter<std::vector<int>, std::less<int>>(std::vector<int>({4, 5}));

  REQUIRE_THAT(s.get_data(), Catch::Equals(std::vector<int>{1, 2, 3}));
  REQUIRE_THAT(t.get_data(), Catch::Equals(std::vector<int>{4, 5}));

  using std::swap;
  swap(s, t);

  CHECK_THAT(s.get_data(), Catch::Equals(std::vector<int>{4, 5}));
  CHECK_THAT(t.get_data(), Catch::Equals(std::vector<int>{1, 2, 3}));
}

TEST_CASE("set_adapter_test.release_data")
{
  auto v = std::vector<int>{1, 2, 3};
  auto s = wrap_set(v);

  auto w = s.release_data();
  CHECK_THAT(w, Catch::Equals(std::vector<int>{1, 2, 3}));
  CHECK(s.empty());
  CHECK(v.empty());
}
} // namespace kdl
