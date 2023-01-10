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

#include "kdl/vector_utils.h"

#include <memory>
#include <set>
#include <vector>

#include "test_utils.h"

#include <catch2/catch.hpp>

namespace kdl
{
TEST_CASE("vector_utils_test.vec_at")
{
  const auto cv = std::vector<int>{1, 2, 3};
  for (std::size_t i = 0u; i < cv.size(); ++i)
  {
    CHECK(vec_at(cv, static_cast<int>(i)) == cv[i]);
  }

  auto mv = std::vector<int>{1, 2, 3};
  vec_at(mv, 2) = 4;
  CHECK(mv[2] == 4);
}

TEST_CASE("vector_utils_test.vec_pop_back")
{
  auto v = std::vector<int>{1, 2, 3};
  CHECK(vec_pop_back(v) == 3);
  CHECK_THAT(v, Catch::Equals(std::vector<int>{1, 2}));
  CHECK(vec_pop_back(v) == 2);
  CHECK_THAT(v, Catch::Equals(std::vector<int>{1}));
  CHECK(vec_pop_back(v) == 1);
  CHECK_THAT(v, Catch::Equals(std::vector<int>{}));
}

struct base
{
  virtual ~base();
};

base::~base() = default;

struct derived : public base
{
  ~derived() override;
};

derived::~derived() = default;

TEST_CASE("vector_utils_test.vec_element_cast")
{
  auto vd = std::vector<derived*>{new derived(), new derived()};
  auto vb = vec_element_cast<base*>(vd);

  CHECK(vb.size() == vd.size());
  for (std::size_t i = 0u; i < vd.size(); ++i)
  {
    CHECK(vb[i] == vd[i]);
  }

  auto vbd = vec_element_cast<derived*>(vb);
  CHECK(vbd.size() == vb.size());
  for (std::size_t i = 0u; i < vb.size(); ++i)
  {
    CHECK(vbd[i] == vb[i]);
  }

  vec_clear_and_delete(vd);
}

TEST_CASE("vector_utils_test.vec_index_of")
{
  using vec = std::vector<int>;

  CHECK(vec_index_of(vec{}, 1) == std::nullopt);
  CHECK(vec_index_of(vec{2}, 1) == std::nullopt);
  CHECK(vec_index_of(vec{1}, 1) == 0u);
  CHECK(vec_index_of(vec{1, 2, 3}, 1) == 0u);
  CHECK(vec_index_of(vec{1, 2, 3}, 2) == 1u);
  CHECK(vec_index_of(vec{1, 2, 3}, 3) == 2u);
  CHECK(vec_index_of(vec{1, 2, 2}, 2) == 1u);
  CHECK(vec_index_of(vec{1, 2, 3}, 4) == std::nullopt);

  CHECK(vec_index_of(vec{}, [](const auto& i) { return i == 1; }) == std::nullopt);
  CHECK(vec_index_of(vec{2}, [](const auto& i) { return i == 1; }) == std::nullopt);
  CHECK(vec_index_of(vec{1}, [](const auto& i) { return i == 1; }) == 0u);
  CHECK(vec_index_of(vec{1, 2, 3}, [](const auto& i) { return i == 1; }) == 0u);
  CHECK(vec_index_of(vec{1, 2, 3}, [](const auto& i) { return i == 2; }) == 1u);
  CHECK(vec_index_of(vec{1, 2, 3}, [](const auto& i) { return i == 3; }) == 2u);
  CHECK(vec_index_of(vec{1, 2, 2}, [](const auto& i) { return i == 2; }) == 1u);
  CHECK(vec_index_of(vec{1, 2, 3}, [](const auto& i) { return i == 4; }) == std::nullopt);
}

TEST_CASE("vector_utils_test.vec_contains")
{
  using vec = std::vector<int>;

  CHECK_FALSE(vec_contains(vec{}, 1));
  CHECK_FALSE(vec_contains(vec{2}, 1));
  CHECK(vec_contains(vec{1}, 1));
  CHECK(vec_contains(vec{1, 2, 3}, 1));
  CHECK(vec_contains(vec{1, 2, 3}, 2));
  CHECK(vec_contains(vec{1, 2, 3}, 3));
  CHECK_FALSE(vec_contains(vec{1, 2, 3}, 4));

  CHECK_FALSE(vec_contains(vec{}, [](const auto& i) { return i == 1; }));
  CHECK_FALSE(vec_contains(vec{2}, [](const auto& i) { return i == 1; }));
  CHECK(vec_contains(vec{1}, [](const auto& i) { return i == 1; }));
  CHECK(vec_contains(vec{1, 2, 3}, [](const auto& i) { return i == 1; }));
  CHECK(vec_contains(vec{1, 2, 3}, [](const auto& i) { return i == 2; }));
  CHECK(vec_contains(vec{1, 2, 3}, [](const auto& i) { return i == 3; }));
  CHECK_FALSE(vec_contains(vec{1, 2, 3}, [](const auto& i) { return i == 4; }));
}

template <typename T, typename... R>
static auto makeVec(T&& t, R... r)
{
  std::vector<T> result;
  result.push_back(std::move(t));
  (..., result.push_back(std::forward<R>(r)));
  return result;
}

TEST_CASE("vector_utils_test.vec_concat")
{
  using vec = std::vector<int>;

  CHECK_THAT(vec_concat(vec{}), Catch::Equals(vec{}));
  CHECK_THAT(vec_concat(vec{}, vec{}), Catch::Equals(vec{}));
  CHECK_THAT(vec_concat(vec{1}), Catch::Equals(vec{1}));
  CHECK_THAT(vec_concat(vec{1}, vec{2}), Catch::Equals(vec{1, 2}));
}

TEST_CASE("vector_utils_test.vec_concat_move")
{
  auto v = makeVec(std::make_unique<int>(1));
  v = vec_concat(std::move(v), makeVec(std::make_unique<int>(2)));

  CHECK(*v[0] == 1);
  CHECK(*v[1] == 2);
}

TEST_CASE("vector_utils_test.vec_slice")
{
  using vec = std::vector<int>;

  CHECK_THAT(vec_slice(vec{}, 0, 0), Catch::Equals(vec{}));
  CHECK_THAT(vec_slice(vec{1, 2, 3}, 0, 0), Catch::Equals(vec{}));
  CHECK_THAT(vec_slice(vec{1, 2, 3}, 1, 0), Catch::Equals(vec{}));
  CHECK_THAT(vec_slice(vec{1, 2, 3}, 2, 0), Catch::Equals(vec{}));
  CHECK_THAT(vec_slice(vec{1, 2, 3}, 3, 0), Catch::Equals(vec{}));
  CHECK_THAT(vec_slice(vec{1, 2, 3}, 0, 1), Catch::Equals(vec{1}));
  CHECK_THAT(vec_slice(vec{1, 2, 3}, 1, 1), Catch::Equals(vec{2}));
  CHECK_THAT(vec_slice(vec{1, 2, 3}, 2, 1), Catch::Equals(vec{3}));
  CHECK_THAT(vec_slice(vec{1, 2, 3}, 0, 2), Catch::Equals(vec{1, 2}));
  CHECK_THAT(vec_slice(vec{1, 2, 3}, 1, 2), Catch::Equals(vec{2, 3}));
  CHECK_THAT(vec_slice(vec{1, 2, 3}, 0, 3), Catch::Equals(vec{1, 2, 3}));
}

TEST_CASE("vector_utils_test.vec_slice_prefix")
{
  using vec = std::vector<int>;

  CHECK_THAT(vec_slice_prefix(vec{}, 0), Catch::Equals(vec{}));
  CHECK_THAT(vec_slice_prefix(vec{1}, 1), Catch::Equals(vec{1}));
  CHECK_THAT(vec_slice_prefix(vec{1}, 0), Catch::Equals(vec{}));
  CHECK_THAT(vec_slice_prefix(vec{1, 2, 3}, 3), Catch::Equals(vec{1, 2, 3}));
  CHECK_THAT(vec_slice_prefix(vec{1, 2, 3}, 2), Catch::Equals(vec{1, 2}));
  CHECK_THAT(vec_slice_prefix(vec{1, 2, 3}, 1), Catch::Equals(vec{1}));
  CHECK_THAT(vec_slice_prefix(vec{1, 2, 3}, 0), Catch::Equals(vec{}));
}

TEST_CASE("vector_utils_test.vec_slice_suffix")
{
  using vec = std::vector<int>;

  CHECK_THAT(vec_slice_suffix(vec{}, 0), Catch::Equals(vec{}));
  CHECK_THAT(vec_slice_suffix(vec{1}, 0), Catch::Equals(vec{}));
  CHECK_THAT(vec_slice_suffix(vec{1}, 1), Catch::Equals(vec{1}));
  CHECK_THAT(vec_slice_suffix(vec{1, 2, 3}, 0), Catch::Equals(vec{}));
  CHECK_THAT(vec_slice_suffix(vec{1, 2, 3}, 1), Catch::Equals(vec{3}));
  CHECK_THAT(vec_slice_suffix(vec{1, 2, 3}, 2), Catch::Equals(vec{2, 3}));
  CHECK_THAT(vec_slice_suffix(vec{1, 2, 3}, 3), Catch::Equals(vec{1, 2, 3}));
}

template <typename T>
void test_erase(std::vector<T> from, const T& x, const std::vector<T>& exp)
{
  const auto originalFrom = from;
  CHECK_THAT(vec_erase(from, x), Catch::Equals(exp));
  CHECK_THAT(from, Catch::Equals(originalFrom));
  CHECK_THAT(vec_erase(std::move(from), x), Catch::Equals(exp));
}

TEST_CASE("vector_utils_test.vec_erase")
{
  test_erase<int>({}, 1, {});
  test_erase<int>({1}, 1, {});
  test_erase<int>({1}, 2, {1});
  test_erase<int>({1, 2, 1}, 2, {1, 1});
  test_erase<int>({1, 2, 1}, 1, {2});
}

template <typename T, typename P>
void test_erase_if(std::vector<T> from, const P& pred, const std::vector<T>& exp)
{
  const auto originalFrom = from;
  CHECK_THAT(vec_erase_if(from, pred), Catch::Equals(exp));
  CHECK_THAT(from, Catch::Equals(originalFrom));
  CHECK_THAT(vec_erase_if(std::move(from), pred), Catch::Equals(exp));
}

TEST_CASE("vector_utils_test.vec_erase_if")
{
  const auto pred = [](const int n) { return n % 2 == 0; };

  test_erase_if<int>({}, pred, {});
  test_erase_if<int>({1}, pred, {1});
  test_erase_if<int>({1, 2, 1}, pred, {1, 1});
  test_erase_if<int>({2, 1, 2}, pred, {1});
}

template <typename T>
void test_erase_at(std::vector<T> from, const std::size_t i, const std::vector<T>& exp)
{
  const auto originalFrom = from;
  CHECK_THAT(vec_erase_at(from, i), Catch::Equals(exp));
  CHECK_THAT(from, Catch::Equals(originalFrom));
  CHECK_THAT(vec_erase_at(std::move(from), i), Catch::Equals(exp));
}

TEST_CASE("vector_utils_test.vec_erase_at")
{
  test_erase_at<int>({1}, 0u, {});
  test_erase_at<int>({1, 2, 1}, 1u, {1, 1});
  test_erase_at<int>({2, 1, 2}, 0u, {1, 2});
}

template <typename T>
void test_erase_all(
  std::vector<T> from, const std::vector<T>& which, const std::vector<T>& exp)
{
  const auto originalFrom = from;
  CHECK_THAT(vec_erase_all(from, which), Catch::Equals(exp));
  CHECK_THAT(from, Catch::Equals(originalFrom));
  CHECK_THAT(vec_erase_all(std::move(from), which), Catch::Equals(exp));
}

TEST_CASE("vector_utils_test.vec_erase_all")
{
  test_erase_all<int>({}, {}, {});
  test_erase_all<int>({1, 2, 3}, {}, {1, 2, 3});
  test_erase_all<int>({1, 2, 3}, {1}, {2, 3});
  test_erase_all<int>({1, 2, 3}, {1, 2}, {3});
  test_erase_all<int>({1, 2, 3}, {1, 2, 3}, {});
  test_erase_all<int>({1, 2, 2, 3}, {2}, {1, 3});
}

TEST_CASE("vector_utils_test.vec_sort")
{
  // just a smoke test since we're just forwarding to std::sort
  CHECK_THAT(
    vec_sort(std::vector<int>{2, 3, 2, 1}), Catch::Equals(std::vector<int>{1, 2, 2, 3}));
}

TEST_CASE("vector_utils_test.vec_sort_and_remove_duplicates")
{
  // just a smoke test since we're just forwarding to std::sort and std::unique
  CHECK_THAT(
    vec_sort_and_remove_duplicates(std::vector<int>{2, 3, 2, 1}),
    Catch::Equals(std::vector<int>{1, 2, 3}));
}

TEST_CASE("vector_utils_test.vec_filter")
{
  CHECK_THAT(
    vec_filter(std::vector<int>{}, [](auto) { return false; }),
    Catch::Equals(std::vector<int>{}));
  CHECK_THAT(
    vec_filter(std::vector<int>{1, 2, 3}, [](auto) { return false; }),
    Catch::Equals(std::vector<int>{}));
  CHECK_THAT(
    vec_filter(std::vector<int>{1, 2, 3}, [](auto) { return true; }),
    Catch::Equals(std::vector<int>{1, 2, 3}));
  CHECK_THAT(
    vec_filter(std::vector<int>{1, 2, 3}, [](auto x) { return x % 2 == 0; }),
    Catch::Equals(std::vector<int>{2}));

  CHECK_THAT(
    vec_filter(std::vector<int>{1, 2, 3}, [](auto, auto i) { return i % 2 == 0; }),
    Catch::Equals(std::vector<int>{1, 3}));
}

struct MoveOnly
{
  MoveOnly() = default;

  MoveOnly(const MoveOnly& other) = delete;
  MoveOnly& operator=(const MoveOnly& other) = delete;

  MoveOnly(MoveOnly&& other) noexcept = default;
  MoveOnly& operator=(MoveOnly&& other) = default;
};

TEST_CASE("vector_utils_test.vec_filter_rvalue")
{
  const auto makeVec = []() {
    auto vec = std::vector<MoveOnly>{};
    vec.emplace_back();
    vec.emplace_back();
    return vec;
  };

  CHECK(vec_filter(makeVec(), [](const auto&) { return true; }).size() == 2u);
  CHECK(
    vec_filter(makeVec(), [](const auto&, auto i) { return i % 2u == 1u; }).size() == 1u);
}

TEST_CASE("vector_utils_test.vec_transform")
{
  CHECK_THAT(
    vec_transform(std::vector<int>{}, [](auto x) { return x + 10; }),
    Catch::Equals(std::vector<int>{}));
  CHECK_THAT(
    vec_transform(std::vector<int>{1, 2, 3}, [](auto x) { return x + 10; }),
    Catch::Equals(std::vector<int>{11, 12, 13}));
  CHECK_THAT(
    vec_transform(std::vector<int>{1, 2, 3}, [](auto x) { return x + 10.0; }),
    Catch::Equals(std::vector<double>{11.0, 12.0, 13.0}));
  CHECK_THAT(
    vec_transform(
      std::vector<int>{1, 2, 3},
      [](auto x, auto i) { return x + static_cast<double>(i); }),
    Catch::Equals(std::vector<double>{1.0, 3.0, 5.0}));
}

struct X
{
};

TEST_CASE("vector_utils_test.vec_transform_lvalue")
{
  std::vector<X> v{X{}, X{}, X{}};

  CHECK(vec_transform(v, [](X& x) { return x; }).size() == 3u);
  CHECK(vec_transform(v, [](X& x, std::size_t) { return x; }).size() == 3u);
}

TEST_CASE("vector_utils_test.vec_transform_rvalue")
{
  CHECK(
    vec_transform(std::vector<X>{X()}, [](X&& x) { return std::move(x); }).size() == 1u);
  CHECK(
    vec_transform(std::vector<X>{X()}, [](X&& x, std::size_t) { return std::move(x); })
      .size()
    == 1u);
}

TEST_CASE("vector_utils_test.vec_flatten")
{
  CHECK_THAT(
    vec_flatten(std::vector<std::vector<int>>{}), Catch::Equals(std::vector<int>{}));
  CHECK_THAT(
    vec_flatten(std::vector<std::vector<int>>{{1}}), Catch::Equals(std::vector<int>{1}));
  CHECK_THAT(
    vec_flatten(std::vector<std::vector<int>>{{}, {}}),
    Catch::Equals(std::vector<int>{}));
  CHECK_THAT(
    vec_flatten(std::vector<std::vector<int>>{{1}, {}}),
    Catch::Equals(std::vector<int>{1}));
  CHECK_THAT(
    vec_flatten(std::vector<std::vector<int>>{{}, {1}}),
    Catch::Equals(std::vector<int>{1}));
  CHECK_THAT(
    vec_flatten(std::vector<std::vector<int>>{{1}, {2}}),
    Catch::Equals(std::vector<int>{1, 2}));
  CHECK_THAT(
    vec_flatten(std::vector<std::vector<int>>{{1, 2}, {3}}),
    Catch::Equals(std::vector<int>{1, 2, 3}));
  CHECK_THAT(
    vec_flatten(std::vector<std::vector<int>>{{1, 2}, {3}}),
    Catch::Equals(std::vector<int>{1, 2, 3}));
  CHECK_THAT(
    vec_flatten(std::vector<std::vector<int>>{{1, 2}, {2, 3}}),
    Catch::Equals(std::vector<int>{1, 2, 2, 3}));
}

TEST_CASE("vector_utils_test.set_difference")
{
  using vec = std::vector<int>;
  using set = std::set<int>;
  CHECK_THAT(set_difference(set({}), set({})), Catch::Equals(vec{}));
  CHECK_THAT(set_difference(set({}), set({1, 2})), Catch::Equals(vec{}));
  CHECK_THAT(set_difference(set({1}), set({1, 2})), Catch::Equals(vec{}));
  CHECK_THAT(set_difference(set({1, 2}), set({1, 2})), Catch::Equals(vec{}));
  CHECK_THAT(set_difference(set({1, 2}), set({1, 2, 3, 4})), Catch::Equals(vec{}));
  CHECK_THAT(set_difference(set({1, 2, 3}), set({1, 2})), Catch::Equals(vec{3}));
  CHECK_THAT(set_difference(set({1, 2, 3}), set({1, 2})), Catch::Equals(vec{3}));
  CHECK_THAT(set_difference(set({1, 2, 3}), set({2})), Catch::Equals(vec{1, 3}));
}

TEST_CASE("vector_utils_test.set_union")
{
  using vec = std::vector<int>;
  using set = std::set<int>;
  CHECK_THAT(set_union(set({}), set({})), Catch::Equals(vec{}));
  CHECK_THAT(set_union(set({}), set({1, 2})), Catch::Equals(vec{1, 2}));
  CHECK_THAT(set_union(set({1}), set({1, 2})), Catch::Equals(vec{1, 2}));
  CHECK_THAT(set_union(set({1, 2}), set({1, 2})), Catch::Equals(vec{1, 2}));
  CHECK_THAT(set_union(set({1, 2}), set({1, 2, 3, 4})), Catch::Equals(vec{1, 2, 3, 4}));
  CHECK_THAT(set_union(set({1, 2, 3}), set({2, 4})), Catch::Equals(vec{1, 2, 3, 4}));
}

TEST_CASE("vector_utils_test.set_intersection")
{
  using vec = std::vector<int>;
  using set = std::set<int>;
  CHECK_THAT(set_intersection(set({}), set({})), Catch::Equals(vec{}));
  CHECK_THAT(set_intersection(set({}), set({1, 2})), Catch::Equals(vec{}));
  CHECK_THAT(set_intersection(set({1}), set({1, 2})), Catch::Equals(vec{1}));
  CHECK_THAT(set_intersection(set({1, 2}), set({1, 2})), Catch::Equals(vec{1, 2}));
  CHECK_THAT(set_intersection(set({1, 2}), set({1, 2, 3, 4})), Catch::Equals(vec{1, 2}));
  CHECK_THAT(set_intersection(set({1, 2, 3}), set({1, 2})), Catch::Equals(vec{1, 2}));
  CHECK_THAT(
    set_intersection(set({1, 2, 3, 4}), set({1, 3, 5})), Catch::Equals(vec{1, 3}));
}

TEST_CASE("vector_utils_test.vec_clear_to_zero")
{
  auto v = std::vector<int>{1, 2, 3};
  CHECK(v.capacity() > 0u);

  vec_clear_to_zero(v);
  CHECK(v.empty());
  CHECK(v.capacity() == 0u);
}

TEST_CASE("vector_utils_test.vec_clear_and_delete")
{
  bool d1 = false;
  bool d2 = false;
  bool d3 = false;
  auto v = std::vector<deletable*>({
    new deletable(d1),
    new deletable(d2),
    new deletable(d3),
  });

  vec_clear_and_delete(v);
  CHECK(v.empty());
  CHECK(d1);
  CHECK(d2);
  CHECK(d3);
}
} // namespace kdl
