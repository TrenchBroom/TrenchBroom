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

#include "test_utils.h"

#include "kd/vector_utils.h"

#include <memory>
#include <ranges>
#include <set>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace kdl
{
namespace
{

struct AppendProbe
{
  int value = 0;
  bool movedFrom = false;

  explicit AppendProbe(const int i)
    : value{i}
  {
  }

  AppendProbe(const AppendProbe&) = default;
  AppendProbe& operator=(const AppendProbe&) = default;

  AppendProbe(AppendProbe&& other) noexcept
    : value{other.value}
  {
    other.movedFrom = true;
  }

  AppendProbe& operator=(AppendProbe&& other) noexcept
  {
    if (this != &other)
    {
      value = other.value;
      movedFrom = false;
      other.movedFrom = true;
    }
    return *this;
  }
};

} // namespace

using namespace Catch::Matchers;

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
  CHECK_THAT(v, Equals(std::vector<int>{1, 2}));
  CHECK(vec_pop_back(v) == 2);
  CHECK_THAT(v, Equals(std::vector<int>{1}));
  CHECK(vec_pop_back(v) == 1);
  CHECK_THAT(v, Equals(std::vector<int>{}));
}

struct base
{
  virtual ~base();
};

base::~base() = default;

struct derived : public base
{
  ~derived() override = default;
};

struct other : public base
{
  ~other() override = default;
};

TEST_CASE("vector_utils_test.vec_dynamic_cast")
{
  auto d = std::make_unique<derived>();
  auto o = std::make_unique<other>();
  const auto vd = std::vector<base*>{d.get(), o.get()};

  CHECK(vec_dynamic_cast<derived>(vd) == std::vector<derived*>{d.get()});
  CHECK(vec_dynamic_cast<other>(vd) == std::vector<other*>{o.get()});
}

TEST_CASE("vector_utils_test.vec_static_cast")
{
  auto d1 = std::make_unique<derived>();
  auto d2 = std::make_unique<derived>();
  const auto vd = std::vector<derived*>{d1.get(), d2.get()};

  CHECK(vec_static_cast<base*>(vd) == std::vector<base*>{d1.get(), d2.get()});
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
  result.push_back(std::forward<T>(t));
  (..., result.push_back(std::forward<R>(r)));
  return result;
}

TEST_CASE("vector_utils_test.vec_push_back")
{
  using ivec = std::vector<int>;
  using svec = std::vector<std::string>;

  CHECK_THAT(vec_push_back(ivec{}), Equals(ivec{}));
  CHECK_THAT(vec_push_back(ivec{}, 1), Equals(ivec{1}));
  CHECK_THAT(vec_push_back(ivec{1}, 2, 3), Equals(ivec{1, 2, 3}));

  CHECK_THAT(
    vec_push_back(svec{}, "hey", std::string{"there"}), Equals(svec{"hey", "there"}));
}

TEST_CASE("vector_utils_test.vec_append_lvalue_range_copies")
{
  auto v = std::vector<AppendProbe>{AppendProbe{1}};
  auto r = std::vector<AppendProbe>{AppendProbe{2}, AppendProbe{3}};

  vec_append(v, r);

  REQUIRE(v.size() == 3u);
  CHECK(v[0].value == 1);
  CHECK(v[1].value == 2);
  CHECK(v[2].value == 3);

  // NOLINTNEXTLINE(bugprone-use-after-move)
  CHECK_FALSE(r[0].movedFrom);

  // NOLINTNEXTLINE(bugprone-use-after-move)
  CHECK_FALSE(r[1].movedFrom);
}

TEST_CASE("vector_utils_test.vec_append_rvalue_range_moves")
{
  auto v = std::vector<AppendProbe>{AppendProbe{1}};
  auto r = std::vector<AppendProbe>{AppendProbe{2}, AppendProbe{3}};

  vec_append(v, std::move(r));

  REQUIRE(v.size() == 3u);
  CHECK(v[0].value == 1);
  CHECK(v[1].value == 2);
  CHECK(v[2].value == 3);

  // NOLINTNEXTLINE(bugprone-use-after-move)
  CHECK(r[0].movedFrom);

  // NOLINTNEXTLINE(bugprone-use-after-move)
  CHECK(r[1].movedFrom);
}

TEST_CASE("vector_utils_test.vec_append_non_common_range_copies")
{
  // views::iota(2) is an unbounded range (sentinel = unreachable_sentinel_t); take_view
  // over a non-sized range uses counted_iterator/default_sentinel, so the result is NOT
  // a common_range — the old insert(begin, end) call would fail to compile.
  const auto r = std::views::iota(2) | std::views::take(3); // yields {2, 3, 4}
  static_assert(!std::ranges::common_range<decltype(r)>);

  auto v = std::vector<int>{1};
  vec_append(v, r);

  CHECK_THAT(v, Equals(std::vector<int>{1, 2, 3, 4}));
}

TEST_CASE("vector_utils_test.vec_append_non_common_range_moves")
{
  // subrange<counted_iterator, default_sentinel_t> is a non-common range.
  // iter_move on counted_iterator delegates to iter_move on its base, so elements
  // are moved — the old make_move_iterator(end) would fail to compile here.
  auto src = std::vector<AppendProbe>{AppendProbe{2}, AppendProbe{3}};
  const auto n = static_cast<std::ptrdiff_t>(src.size());
  auto r =
    std::ranges::subrange(std::counted_iterator(src.begin(), n), std::default_sentinel);
  static_assert(!std::ranges::common_range<decltype(r)>);

  auto v = std::vector<AppendProbe>{AppendProbe{1}};
  vec_append(v, std::move(r));

  REQUIRE(v.size() == 3u);
  CHECK(v[1].value == 2);
  CHECK(v[2].value == 3);

  // NOLINTNEXTLINE(bugprone-use-after-move)
  CHECK(src[0].movedFrom);

  // NOLINTNEXTLINE(bugprone-use-after-move)
  CHECK(src[1].movedFrom);
}

template <typename T>
void test_erase(std::vector<T> from, const T& x, const std::vector<T>& exp)
{
  const auto originalFrom = from;
  CHECK_THAT(vec_erase(from, x), Equals(exp));
  CHECK_THAT(from, Equals(originalFrom));
  CHECK_THAT(vec_erase(std::move(from), x), Equals(exp));
}

TEST_CASE("vector_utils_test.vec_erase")
{
  test_erase<int>({}, 1, {});
  test_erase<int>({1}, 1, {});
  test_erase<int>({1}, 2, {1});
  test_erase<int>({1, 2, 1}, 2, {1, 1});
  test_erase<int>({1, 2, 1}, 1, {2});
}

template <typename T>
void test_erase_at(std::vector<T> from, const std::size_t i, const std::vector<T>& exp)
{
  const auto originalFrom = from;
  CHECK_THAT(vec_erase_at(from, i), Equals(exp));
  CHECK_THAT(from, Equals(originalFrom));
  CHECK_THAT(vec_erase_at(std::move(from), i), Equals(exp));
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
  CHECK_THAT(vec_erase_all(from, which), Equals(exp));
  CHECK_THAT(from, Equals(originalFrom));
  CHECK_THAT(vec_erase_all(std::move(from), which), Equals(exp));
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
    vec_sort(std::vector<int>{2, 3, 2, 1}), Equals(std::vector<int>{1, 2, 2, 3}));
}

TEST_CASE("vector_utils_test.vec_sort_and_remove_duplicates")
{
  // just a smoke test since we're just forwarding to std::sort and std::unique
  CHECK_THAT(
    vec_sort_and_remove_duplicates(std::vector<int>{2, 3, 2, 1}),
    Equals(std::vector<int>{1, 2, 3}));
}

TEST_CASE("vector_utils_test.vec_sort_and_remove_duplicates_in_place")
{
  auto v = std::vector<int>{2, 3, 2, 1};
  vec_sort_and_remove_duplicates(v);
  CHECK_THAT(v, Equals(std::vector<int>{1, 2, 3}));
}

TEST_CASE("vector_utils_test.vec_sort_and_remove_duplicates_const_lvalue")
{
  const auto v = std::vector<int>{2, 3, 2, 1};
  CHECK_THAT(vec_sort_and_remove_duplicates(v), Equals(std::vector<int>{1, 2, 3}));

  // the original vector must be left unchanged
  CHECK_THAT(v, Equals(std::vector<int>{2, 3, 2, 1}));
}

struct MoveOnly
{
  MoveOnly() = default;

  MoveOnly(const MoveOnly& other) = delete;
  MoveOnly& operator=(const MoveOnly& other) = delete;

  MoveOnly(MoveOnly&& other) noexcept = default;
  MoveOnly& operator=(MoveOnly&& other) = default;
};

TEST_CASE("vector_utils_test.set_difference")
{
  using vec = std::vector<int>;
  using set = std::set<int>;
  CHECK_THAT(set_difference(set({}), set({})), Equals(vec{}));
  CHECK_THAT(set_difference(set({}), set({1, 2})), Equals(vec{}));
  CHECK_THAT(set_difference(set({1}), set({1, 2})), Equals(vec{}));
  CHECK_THAT(set_difference(set({1, 2}), set({1, 2})), Equals(vec{}));
  CHECK_THAT(set_difference(set({1, 2}), set({1, 2, 3, 4})), Equals(vec{}));
  CHECK_THAT(set_difference(set({1, 2, 3}), set({1, 2})), Equals(vec{3}));
  CHECK_THAT(set_difference(set({1, 2, 3}), set({1, 2})), Equals(vec{3}));
  CHECK_THAT(set_difference(set({1, 2, 3}), set({2})), Equals(vec{1, 3}));
}

TEST_CASE("vector_utils_test.set_union")
{
  using vec = std::vector<int>;
  using set = std::set<int>;
  CHECK_THAT(set_union(set({}), set({})), Equals(vec{}));
  CHECK_THAT(set_union(set({}), set({1, 2})), Equals(vec{1, 2}));
  CHECK_THAT(set_union(set({1}), set({1, 2})), Equals(vec{1, 2}));
  CHECK_THAT(set_union(set({1, 2}), set({1, 2})), Equals(vec{1, 2}));
  CHECK_THAT(set_union(set({1, 2}), set({1, 2, 3, 4})), Equals(vec{1, 2, 3, 4}));
  CHECK_THAT(set_union(set({1, 2, 3}), set({2, 4})), Equals(vec{1, 2, 3, 4}));
}

TEST_CASE("vector_utils_test.set_intersection")
{
  using vec = std::vector<int>;
  using set = std::set<int>;
  CHECK_THAT(set_intersection(set({}), set({})), Equals(vec{}));
  CHECK_THAT(set_intersection(set({}), set({1, 2})), Equals(vec{}));
  CHECK_THAT(set_intersection(set({1}), set({1, 2})), Equals(vec{1}));
  CHECK_THAT(set_intersection(set({1, 2}), set({1, 2})), Equals(vec{1, 2}));
  CHECK_THAT(set_intersection(set({1, 2}), set({1, 2, 3, 4})), Equals(vec{1, 2}));
  CHECK_THAT(set_intersection(set({1, 2, 3}), set({1, 2})), Equals(vec{1, 2}));
  CHECK_THAT(set_intersection(set({1, 2, 3, 4}), set({1, 3, 5})), Equals(vec{1, 3}));
}

TEST_CASE("vector_utils_test.set_has_shared_element")
{
  using vec = std::vector<int>;
  CHECK_FALSE(set_has_shared_element(vec{}, vec{}));
  CHECK_FALSE(set_has_shared_element(vec{}, vec{4}));
  CHECK_FALSE(set_has_shared_element(vec{1}, vec{4}));
  CHECK_FALSE(set_has_shared_element(vec{1, 2, 3}, vec{4}));
  CHECK(set_has_shared_element(vec{1}, vec{1}));
  CHECK(set_has_shared_element(vec{1, 2, 3}, vec{1}));
  CHECK(set_has_shared_element(vec{1}, vec{1, 2, 3}));
  CHECK(set_has_shared_element(vec{1, 2, 3}, vec{2}));
  CHECK(set_has_shared_element(vec{2}, vec{1, 2, 3}));
  CHECK(set_has_shared_element(vec{1, 2, 3}, vec{3}));
  CHECK(set_has_shared_element(vec{3}, vec{1, 2, 3}));
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
