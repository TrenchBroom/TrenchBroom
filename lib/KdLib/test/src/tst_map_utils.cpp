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

#include "kd/flat_map.h"
#include "kd/map_utils.h"

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{

// thin, non-asserting wrappers that let the std::map test cases below use bare braced
// initializers without pinning K/V at every call site
template <typename K, typename V>
int lexCompare(const std::map<K, V>& lhs, const std::map<K, V>& rhs)
{
  return map_lexicographical_compare(lhs, rhs);
}

template <typename K, typename V>
bool isEquivalent(const std::map<K, V>& lhs, const std::map<K, V>& rhs)
{
  return map_is_equivalent(lhs, rhs);
}

template <typename K, typename V>
V findOrDefault(const std::map<K, V>& m, const K& key, const V& defaultValue)
{
  return map_find_or_default(m, key, defaultValue);
}

template <typename K, typename V>
std::map<K, V> unionOf(const std::map<K, V>& m1, const std::map<K, V>& m2)
{
  return map_union(m1, m2);
}

template <typename K, typename V>
std::map<K, std::vector<V>> mergeOf(
  const std::map<K, std::vector<V>>& m1, const std::map<K, std::vector<V>>& m2)
{
  return map_merge(m1, m2);
}

TEST_CASE("map_utils")
{
  SECTION("map_lexicographical_compare")
  {
    CHECK(lexCompare<int, int>({}, {}) == 0);
    CHECK(lexCompare<int, int>({{1, 2}, {2, 3}}, {{1, 2}, {2, 3}}) == 0);
    CHECK(lexCompare<int, int>({{1, 2}, {2, 3}}, {{1, 2}, {3, 3}}) == -1);
    CHECK(lexCompare<int, int>({{1, 2}, {3, 3}}, {{1, 2}, {2, 3}}) == +1);
    CHECK(lexCompare<int, int>({{1, 2}, {3, 3}}, {{2, 2}, {3, 3}}) == -1);
    CHECK(lexCompare<int, int>({{1, 2}, {2, 3}, {3, 4}}, {{1, 2}, {2, 3}}) == +1);
    CHECK(lexCompare<int, int>({{1, 2}, {2, 3}}, {{1, 2}, {2, 3}, {3, 4}}) == -1);
    CHECK(lexCompare<int, int>({{1, 2}, {2, 3}}, {{1, 2}, {2, 4}}) == -1);

    // also works with kdl::flat_map, since both are ordered map types
    CHECK(
      map_lexicographical_compare(
        kdl::flat_map<int, int>{{1, 2}, {2, 3}}, kdl::flat_map<int, int>{{1, 2}, {3, 3}})
      == -1);
  }

  SECTION("map_is_equivalent")
  {
    CHECK(isEquivalent<int, int>({}, {}));
    CHECK(isEquivalent<int, int>({{1, 2}, {2, 3}}, {{1, 2}, {2, 3}}));
    CHECK(!isEquivalent<int, int>({{1, 2}, {2, 3}}, {{1, 2}, {3, 3}}));
    CHECK(!isEquivalent<int, int>({{1, 2}, {3, 3}}, {{1, 2}, {2, 3}}));
    CHECK(!isEquivalent<int, int>({{1, 2}, {3, 3}}, {{2, 2}, {3, 3}}));
    CHECK(!isEquivalent<int, int>({{1, 2}, {2, 3}, {3, 4}}, {{1, 2}, {2, 3}}));
    CHECK(!isEquivalent<int, int>({{1, 2}, {2, 3}}, {{1, 2}, {2, 3}, {3, 4}}));

    // also works with unordered maps and flat maps, which have no defined iteration order
    CHECK(map_is_equivalent(
      std::unordered_map<int, int>{{1, 2}, {2, 3}},
      std::unordered_map<int, int>{{2, 3}, {1, 2}}));
    CHECK(!map_is_equivalent(
      std::unordered_map<int, int>{{1, 2}, {2, 3}},
      std::unordered_map<int, int>{{1, 2}}));
    CHECK(map_is_equivalent(
      kdl::flat_map<int, int>{{1, 2}, {2, 3}}, kdl::flat_map<int, int>{{2, 3}, {1, 2}}));
  }

  SECTION("map_find_or_default")
  {
    CHECK(findOrDefault<int, std::string>({}, 1, "default") == "default");
    CHECK(findOrDefault<int, std::string>({{1, "value"}}, 1, "default") == "value");

    CHECK(
      map_find_or_default(
        std::unordered_map<int, std::string>{{1, "value"}}, 1, "default")
      == "value");
    CHECK(
      map_find_or_default(kdl::flat_map<int, std::string>{{1, "value"}}, 2, "default")
      == "default");
  }

  SECTION("map_union")
  {
    CHECK(unionOf<int, int>({}, {}) == std::map<int, int>{});
    CHECK(unionOf<int, int>({{1, 2}}, {}) == std::map<int, int>{{1, 2}});
    CHECK(unionOf<int, int>({}, {{1, 2}}) == std::map<int, int>{{1, 2}});
    CHECK(unionOf<int, int>({{1, 2}}, {{1, 2}}) == std::map<int, int>{{1, 2}});
    CHECK(unionOf<int, int>({}, {{1, 2}, {2, 3}}) == std::map<int, int>{{1, 2}, {2, 3}});
    CHECK(unionOf<int, int>({{1, 2}}, {{2, 3}}) == std::map<int, int>{{1, 2}, {2, 3}});
    CHECK(unionOf<int, int>({{1, 2}}, {{1, 3}}) == std::map<int, int>{{1, 3}});

    CHECK(
      map_union(
        std::unordered_map<int, int>{{1, 2}},
        std::unordered_map<int, int>{{1, 3}, {2, 4}})
      == std::unordered_map<int, int>{{1, 3}, {2, 4}});
    CHECK(
      map_union(kdl::flat_map<int, int>{{1, 2}}, kdl::flat_map<int, int>{{1, 3}, {2, 4}})
      == kdl::flat_map<int, int>{{1, 3}, {2, 4}});
  }

  SECTION("map_merge")
  {
    CHECK(mergeOf<int, int>({}, {}) == (std::map<int, std::vector<int>>{}));
    CHECK(
      mergeOf<int, int>({{1, {1, 2}}}, {})
      == (std::map<int, std::vector<int>>{{1, {1, 2}}}));
    CHECK(
      mergeOf<int, int>({}, {{1, {1, 2}}})
      == (std::map<int, std::vector<int>>{{1, {1, 2}}}));
    CHECK(
      mergeOf<int, int>({{1, {1, 2}}}, {{2, {3, 4}}})
      == (std::map<int, std::vector<int>>{{1, {1, 2}}, {2, {3, 4}}}));
    CHECK(
      mergeOf<int, int>({{1, {1, 2}}}, {{1, {3, 4}}})
      == (std::map<int, std::vector<int>>{{1, {1, 2, 3, 4}}}));

    CHECK(
      map_merge(
        std::unordered_map<int, std::vector<int>>{{1, {1, 2}}},
        std::unordered_map<int, std::vector<int>>{{1, {3, 4}}})
      == (std::unordered_map<int, std::vector<int>>{{1, {1, 2, 3, 4}}}));
    CHECK(
      map_merge(
        kdl::flat_map<int, std::vector<int>>{{1, {1, 2}}},
        kdl::flat_map<int, std::vector<int>>{{2, {3, 4}}})
      == (kdl::flat_map<int, std::vector<int>>{{1, {1, 2}}, {2, {3, 4}}}));
  }

  SECTION("map_clear_and_delete")
  {
    bool d1 = false;
    bool d2 = false;
    bool d3 = false;
    bool d4 = false;

    auto m = std::map<int, std::vector<deletable*>>(
      {{1, {}},
       {2, {new deletable{d1}, new deletable{d2}}},
       {3, {}},
       {4, {new deletable{d3}}},
       {5, {new deletable{d4}}}});

    map_clear_and_delete(m);
    CHECK(m.empty());
    CHECK(d1);
    CHECK(d2);
    CHECK(d3);
    CHECK(d4);
  }

  SECTION("map_clear_and_delete with unordered_map")
  {
    bool d1 = false;
    bool d2 = false;

    auto m = std::unordered_map<int, std::vector<deletable*>>(
      {{1, {new deletable{d1}}}, {2, {new deletable{d2}}}});

    map_clear_and_delete(m);
    CHECK(m.empty());
    CHECK(d1);
    CHECK(d2);
  }

  SECTION("map_clear_and_delete with flat_map")
  {
    bool d1 = false;
    bool d2 = false;

    auto m = kdl::flat_map<int, std::vector<deletable*>>{
      {1, {new deletable{d1}}}, {2, {new deletable{d2}}}};

    map_clear_and_delete(m);
    CHECK(m.empty());
    CHECK(d1);
    CHECK(d2);
  }
}

} // namespace kdl
