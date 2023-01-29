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

#include "kdl/binary_relation.h"

#include <algorithm>

#include <catch2/catch_all.hpp>

namespace kdl
{
template <typename L, typename R>
bool checkRelation(
  const binary_relation<L, R>& act, const std::vector<std::pair<L, R>>& exp)
{
  return std::equal(std::begin(act), std::end(act), std::begin(exp), std::end(exp));
}

TEST_CASE("binary_relation_test.constructor_default")
{
  using relation = binary_relation<int, std::string>;

  relation r;
  CHECK(r.empty());
}

TEST_CASE("binary_relation_test.constructor_intializer_list")
{
  using relation = binary_relation<int, std::string>;

  relation r({
    {1, "a"},
    {1, "b"},
    {2, "b"},
    {3, "b"},
    {4, "c"},
    {4, "c"},
  });

  CHECK(checkRelation(
    r,
    {
      {1, "a"},
      {1, "b"},
      {2, "b"},
      {3, "b"},
      {4, "c"},
    }));
}

TEST_CASE("binary_relation_test.empty")
{
  using relation = binary_relation<int, std::string>;

  CHECK(relation().empty());
  CHECK_FALSE(relation({{1, "a"}}).empty());
}

TEST_CASE("binary_relation_test.size")
{
  using relation = binary_relation<int, std::string>;

  CHECK(relation().size() == 0u);
  CHECK(relation({{1, "a"}}).size() == 1u);
  CHECK(relation({{1, "a"}, {1, "b"}}).size() == 2u);
  CHECK(relation({{1, "a"}, {1, "b"}, {2, "c"}}).size() == 3u);
}

TEST_CASE("binary_relation_test.contains")
{
  using relation = binary_relation<int, std::string>;

  CHECK_FALSE(relation().contains(1, "a"));
  CHECK_FALSE(relation({{1, "b"}}).contains(1, "a"));
  CHECK_FALSE(relation({{2, "a"}}).contains(1, "a"));
  CHECK(relation({{1, "a"}}).contains(1, "a"));
}

TEST_CASE("binary_relation_test.count_left")
{
  using relation = binary_relation<int, std::string>;

  CHECK(relation().count_left("a") == 0u);
  CHECK(relation({{1, "b"}}).count_left("a") == 0u);
  CHECK(relation({{1, "a"}}).count_left("a") == 1u);
  CHECK(relation({{1, "a"}, {1, "b"}}).count_left("a") == 1u);
  CHECK(relation({{1, "a"}, {1, "b"}, {2, "a"}}).count_left("a") == 2u);
}

TEST_CASE("binary_relation_test.count_right")
{
  using relation = binary_relation<int, std::string>;

  CHECK(relation().count_right(1) == 0u);
  CHECK(relation({{2, "a"}}).count_right(1) == 0u);
  CHECK(relation({{1, "a"}}).count_right(1) == 1u);
  CHECK(relation({{1, "a"}, {2, "a"}}).count_right(1) == 1u);
  CHECK(relation({{1, "a"}, {1, "b"}, {2, "a"}}).count_right(1) == 2u);
}

TEST_CASE("binary_relation_test.iterator")
{
  using relation = binary_relation<int, std::string>;

  relation r;
  CHECK(std::begin(r) == std::end(r));
  CHECK_FALSE(std::begin(r) != std::end(r));

  r.insert(1, "a");
  r.insert(1, "b");
  r.insert(2, "b");
  r.insert(3, "c");

  auto it = std::begin(r);
  auto end = std::end(r);

  CHECK_FALSE(it == end);
  CHECK(it != end);
  CHECK(*it == std::make_pair(1, std::string("a")));

  ++it;
  CHECK_FALSE(it == end);
  CHECK(it != end);
  CHECK(*it == std::make_pair(1, std::string("b")));

  ++it;
  CHECK_FALSE(it == end);
  CHECK(it != end);
  CHECK(*it == std::make_pair(2, std::string("b")));

  ++it;
  CHECK_FALSE(it == end);
  CHECK(it != end);
  CHECK(*it == std::make_pair(3, std::string("c")));

  ++it;
  CHECK(it == end);
  CHECK_FALSE(it != end);
}

template <typename T, typename I>
void assertRange(const std::vector<T>& exp, const std::pair<I, I>& act)
{
  CHECK(std::equal(std::begin(exp), std::end(exp), act.first, act.second));
}

TEST_CASE("binary_relation_test.left_range")
{
  using relation = binary_relation<int, std::string>;

  assertRange<int>({}, relation().left_range("a"));
  assertRange<int>({}, relation({{1, "b"}}).left_range("a"));
  assertRange<int>({1}, relation({{1, "a"}}).left_range("a"));
  assertRange<int>({1, 2}, relation({{1, "a"}, {2, "a"}, {3, "b"}}).left_range("a"));
}

TEST_CASE("binary_relation_test.right_range")
{
  using relation = binary_relation<int, std::string>;

  assertRange<std::string>({}, relation().right_range(1));
  assertRange<std::string>({}, relation({{2, "b"}}).right_range(1));
  assertRange<std::string>({"a"}, relation({{1, "a"}}).right_range(1));
  assertRange<std::string>(
    {"a", "b"}, relation({{1, "a"}, {1, "b"}, {2, "c"}}).right_range(1));
}

TEST_CASE("binary_relation_test.insert_relation")
{
  using relation = binary_relation<int, std::string>;

  relation r;
  r.insert(relation({
    {1, "a"},
    {1, "b"},
    {2, "b"},
    {3, "b"},
    {4, "c"},
    {4, "c"},
  }));

  CHECK(checkRelation(
    r,
    {
      {1, "a"},
      {1, "b"},
      {2, "b"},
      {3, "b"},
      {4, "c"},
    }));
}

TEST_CASE("binary_relation_test.insert_right_range")
{
  using relation = binary_relation<int, std::string>;

  relation r;

  const size_t left_1 = 1;
  const std::vector<std::string> right_1({"a", "b"});

  r.insert(left_1, std::begin(right_1), std::end(right_1));

  CHECK(r.size() == 2u);
  CHECK(r.contains(left_1, right_1[0]));
  CHECK(r.contains(left_1, right_1[1]));
  CHECK(r.count_left(right_1[0]) == 1u);
  CHECK(r.count_left(right_1[1]) == 1u);
  CHECK(r.count_right(left_1) == 2u);
  CHECK(std::equal(std::begin(right_1), std::end(right_1), r.right_begin(left_1)));

  const size_t left_2 = 2;
  const std::vector<std::string> right_2({"b", "c"});

  r.insert(left_2, std::begin(right_2), std::end(right_2));

  CHECK(r.size() == 4u);
  CHECK(r.contains(left_2, right_2[0]));
  CHECK(r.contains(left_2, right_2[1]));
  CHECK(r.count_left(right_2[0]) == 2u);
  CHECK(r.count_left(right_2[1]) == 1u);
  CHECK(r.count_right(left_2) == 2u);
  CHECK(std::equal(std::begin(right_2), std::end(right_2), r.right_begin(left_2)));

  const size_t left_3 = left_1;
  const std::vector<std::string> right_3({"a", "b", "c"});
  r.insert(left_1, std::begin(right_3), std::end(right_3));

  CHECK(r.size() == 5u);
  CHECK(r.contains(left_3, right_3[0]));
  CHECK(r.contains(left_3, right_3[1]));
  CHECK(r.contains(left_3, right_3[2]));
  CHECK(r.count_left(right_3[0]) == 1u);
  CHECK(r.count_left(right_3[1]) == 2u);
  CHECK(r.count_left(right_3[2]) == 2u);
  CHECK(r.count_right(left_3) == 3u);
  CHECK(std::equal(std::begin(right_3), std::end(right_3), r.right_begin(left_3)));
}

TEST_CASE("binary_relation_test.insert_left_range")
{
  using relation = binary_relation<std::string, size_t>;

  relation r;

  const std::vector<std::string> left_1({"a", "b"});
  const size_t right_1 = 1;

  r.insert(std::begin(left_1), std::end(left_1), right_1);

  CHECK(r.size() == 2u);
  CHECK(r.contains(left_1[0], right_1));
  CHECK(r.contains(left_1[1], right_1));
  CHECK(r.count_right(left_1[0]) == 1u);
  CHECK(r.count_right(left_1[1]) == 1u);
  CHECK(r.count_left(right_1) == 2u);
  CHECK(std::equal(std::begin(left_1), std::end(left_1), r.left_begin(right_1)));

  const std::vector<std::string> left_2({"b", "c"});
  const size_t right_2 = 2;

  r.insert(std::begin(left_2), std::end(left_2), right_2);

  CHECK(r.size() == 4u);
  CHECK(r.contains(left_2[0], right_2));
  CHECK(r.contains(left_2[1], right_2));
  CHECK(r.count_right(left_2[0]) == 2u);
  CHECK(r.count_right(left_2[1]) == 1u);
  CHECK(r.count_left(right_2) == 2u);
  CHECK(std::equal(std::begin(left_2), std::end(left_2), r.left_begin(right_2)));

  const std::vector<std::string> left_3({"a", "b", "c"});
  const size_t right_3 = right_1;
  r.insert(std::begin(left_3), std::end(left_3), right_3);

  CHECK(r.size() == 5u);
  CHECK(r.contains(left_3[0], right_3));
  CHECK(r.contains(left_3[1], right_3));
  CHECK(r.contains(left_3[2], right_3));
  CHECK(r.count_right(left_3[0]) == 1u);
  CHECK(r.count_right(left_3[1]) == 2u);
  CHECK(r.count_right(left_3[2]) == 2u);
  CHECK(r.count_left(right_3) == 3u);
  CHECK(std::equal(std::begin(left_3), std::end(left_3), r.left_begin(right_3)));
}

TEST_CASE("binary_relation_test.insert_values")
{
  using relation = binary_relation<int, std::string>;

  relation r;
  CHECK(r.insert(1, "a"));

  CHECK(r.size() == 1u);
  CHECK_FALSE(r.empty());
  CHECK(r.contains(1, "a"));
  CHECK(r.count_left("a") == 1u);
  CHECK(r.count_right(1) == 1u);

  CHECK_FALSE(r.insert(1, "a"));
  CHECK(r.size() == 1u);

  CHECK(r.insert(1, "b"));
  CHECK(r.size() == 2u);
  CHECK(r.contains(1, "b"));
  CHECK(r.count_left("a") == 1u);
  CHECK(r.count_left("b") == 1u);
  CHECK(r.count_right(1) == 2u);

  CHECK(r.insert(2, "b"));
  CHECK(r.size() == 3u);
  CHECK(r.count_left("a") == 1u);
  CHECK(r.count_left("b") == 2u);
  CHECK(r.count_right(1) == 2u);
  CHECK(r.count_right(2) == 1u);
}

TEST_CASE("binary_relation_test.erase")
{
  using relation = binary_relation<int, std::string>;

  relation r;
  r.insert(1, "a");
  r.insert(1, "b");
  r.insert(2, "b");
  r.insert(3, "c");

  // just to make sure
  CHECK(r.size() == 4u);
  CHECK(r.contains(1, "a"));
  CHECK(r.contains(1, "b"));
  CHECK(r.contains(2, "b"));
  CHECK(r.contains(3, "c"));

  CHECK_FALSE(r.erase(3, "a"));
  CHECK_FALSE(r.erase(4, ""));
  CHECK_FALSE(r.erase(3, "a"));

  CHECK(r.erase(1, "a"));
  CHECK(r.size() == 3u);
  CHECK_FALSE(r.contains(1, "a"));
  CHECK(r.contains(1, "b"));
  CHECK(r.contains(2, "b"));
  CHECK(r.contains(3, "c"));
  CHECK_FALSE(r.erase(1, "a"));

  CHECK(r.erase(3, "c"));
  CHECK(r.size() == 2u);
  CHECK_FALSE(r.contains(1, "a"));
  CHECK(r.contains(1, "b"));
  CHECK(r.contains(2, "b"));
  CHECK_FALSE(r.contains(3, "c"));
  CHECK_FALSE(r.erase(3, "c"));

  CHECK(r.erase(1, "b"));
  CHECK(r.size() == 1u);
  CHECK_FALSE(r.contains(1, "a"));
  CHECK_FALSE(r.contains(1, "b"));
  CHECK(r.contains(2, "b"));
  CHECK_FALSE(r.contains(3, "c"));
  CHECK_FALSE(r.erase(1, "b"));

  CHECK(r.erase(2, "b"));
  CHECK(r.size() == 0u);
  CHECK_FALSE(r.contains(1, "a"));
  CHECK_FALSE(r.contains(1, "b"));
  CHECK_FALSE(r.contains(2, "b"));
  CHECK_FALSE(r.contains(3, "c"));
  CHECK_FALSE(r.erase(2, "b"));
}
} // namespace kdl
