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

#include "kd/flat_set.h"

#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{
using fset = flat_set<int>;

struct tagged
{
  int id;
  int tag;
};

struct tagged_less
{
  bool operator()(const tagged& lhs, const tagged& rhs) const { return lhs.id < rhs.id; }
};

using tagged_set = flat_set<tagged, tagged_less>;

std::vector<int> to_vector(const fset& s)
{
  return {s.begin(), s.end()};
}

} // namespace

TEST_CASE("flat_set")
{
  SECTION("constructor")
  {
    SECTION("default")
    {
      const auto s = fset{};
      CHECK(s.empty());
      CHECK(s.size() == 0u);
    }

    SECTION("with_comparator")
    {
      const auto s = fset{std::less<int>{}};
      CHECK(s.empty());
    }

    SECTION("with_container")
    {
      CHECK(to_vector(fset{std::vector<int>{}}) == std::vector<int>{});
      CHECK(
        to_vector(fset{std::vector<int>{2, 1, 3, 1, 2}}) == std::vector<int>{1, 2, 3});
    }

    SECTION("with_sorted_unique_container")
    {
      const auto s = fset{sorted_unique, std::vector<int>{1, 2, 3}};
      CHECK(to_vector(s) == std::vector<int>{1, 2, 3});
    }

    SECTION("with_range")
    {
      const auto v = std::vector<int>{2, 1, 3, 1, 2};
      CHECK(to_vector(fset{v.begin(), v.end()}) == std::vector<int>{1, 2, 3});
    }

    SECTION("with_sorted_unique_range")
    {
      const auto v = std::vector<int>{1, 2, 3};
      const auto s = fset{sorted_unique, v.begin(), v.end()};
      CHECK(to_vector(s) == std::vector<int>{1, 2, 3});
    }

    SECTION("with_initializer_list")
    {
      CHECK(to_vector(fset{}) == std::vector<int>{});
      CHECK(to_vector(fset{2, 1, 3, 1, 2}) == std::vector<int>{1, 2, 3});
    }

    SECTION("with_sorted_unique_initializer_list")
    {
      const auto s = fset{sorted_unique, {1, 2, 3}};
      CHECK(to_vector(s) == std::vector<int>{1, 2, 3});
    }

    SECTION("deduction_guide")
    {
      // parentheses are required here to select the container and range constructors
      // instead of the initializer_list constructor, which takes priority for braced
      // argument lists
      auto v = std::vector<int>{2, 1, 3};
      const auto s1 = flat_set(v);
      CHECK(std::vector<int>(s1.begin(), s1.end()) == std::vector<int>{1, 2, 3});

      const auto s2 = flat_set(v.begin(), v.end());
      CHECK(std::vector<int>(s2.begin(), s2.end()) == std::vector<int>{1, 2, 3});

      const auto s3 = flat_set{1, 2, 3};
      CHECK(to_vector(s3) == std::vector<int>{1, 2, 3});
    }
  }

  SECTION("assignment")
  {
    auto s = fset{7, 8, 9};
    s = {2, 1, 3, 1, 2};
    CHECK(to_vector(s) == std::vector<int>{1, 2, 3});
  }

  SECTION("iterators")
  {
    const auto s = fset{3, 1, 2};
    CHECK(to_vector(s) == std::vector<int>{1, 2, 3});
    CHECK(std::vector<int>(s.rbegin(), s.rend()) == std::vector<int>{3, 2, 1});
    CHECK(std::vector<int>(s.cbegin(), s.cend()) == std::vector<int>{1, 2, 3});
    CHECK(std::vector<int>(s.crbegin(), s.crend()) == std::vector<int>{3, 2, 1});
  }

  SECTION("capacity")
  {
    CHECK(fset{}.empty());
    CHECK(fset{}.size() == 0u);
    CHECK(!fset{1}.empty());
    CHECK(fset{1, 2, 3}.size() == 3u);
  }

  SECTION("clear")
  {
    auto s = fset{1, 2, 3};
    s.clear();
    CHECK(s.empty());
  }

  SECTION("insert")
  {
    SECTION("single_value")
    {
      auto s = fset{1, 3};

      auto [it1, inserted1] = s.insert(2);
      CHECK(inserted1);
      CHECK(*it1 == 2);
      CHECK(to_vector(s) == std::vector<int>{1, 2, 3});

      auto [it2, inserted2] = s.insert(2);
      CHECK(!inserted2);
      CHECK(*it2 == 2);
      CHECK(to_vector(s) == std::vector<int>{1, 2, 3});
    }

    SECTION("does_not_overwrite_existing_value")
    {
      auto s = tagged_set{tagged{1, 100}};

      const auto [it, inserted] = s.insert(tagged{1, 200});
      CHECK(!inserted);
      CHECK(it->tag == 100);
    }

    SECTION("with_hint")
    {
      SECTION("valid_hint_at_begin")
      {
        auto s = fset{3, 5, 7};
        const auto it = s.insert(s.begin(), 1);
        CHECK(*it == 1);
        CHECK(to_vector(s) == std::vector<int>{1, 3, 5, 7});
      }

      SECTION("valid_hint_at_end")
      {
        auto s = fset{1, 3, 5};
        const auto it = s.insert(s.end(), 7);
        CHECK(*it == 7);
        CHECK(to_vector(s) == std::vector<int>{1, 3, 5, 7});
      }

      SECTION("valid_hint_in_middle")
      {
        auto s = fset{1, 3, 5, 7};
        const auto it = s.insert(s.find(5), 4);
        CHECK(*it == 4);
        CHECK(to_vector(s) == std::vector<int>{1, 3, 4, 5, 7});
      }

      SECTION("invalid_hint_falls_back_to_search")
      {
        auto s = fset{1, 3, 5, 7};
        const auto it = s.insert(s.find(3), 6);
        CHECK(*it == 6);
        CHECK(to_vector(s) == std::vector<int>{1, 3, 5, 6, 7});
      }

      SECTION("hint_at_existing_duplicate")
      {
        auto s = fset{1, 3, 5, 7};
        const auto it = s.insert(s.find(3), 3);
        CHECK(*it == 3);
        CHECK(to_vector(s) == std::vector<int>{1, 3, 5, 7});
      }
    }

    SECTION("heterogeneous")
    {
      auto s = flat_set<std::string, std::less<>>{"a", "ccc"};

      const auto [it1, inserted1] = s.insert(std::string_view{"bb"});
      CHECK(inserted1);
      CHECK(*it1 == "bb");
      CHECK(
        std::vector<std::string>(s.begin(), s.end())
        == std::vector<std::string>{"a", "bb", "ccc"});

      const auto [it2, inserted2] = s.insert(std::string_view{"bb"});
      CHECK(!inserted2);
      CHECK(*it2 == "bb");

      const auto it3 = s.insert(s.begin(), std::string_view{"0"});
      CHECK(*it3 == "0");
      CHECK(
        std::vector<std::string>(s.begin(), s.end())
        == std::vector<std::string>{"0", "a", "bb", "ccc"});

      const auto it4 = s.insert(s.begin(), std::string_view{"zzz"});
      CHECK(*it4 == "zzz");
      CHECK(
        std::vector<std::string>(s.begin(), s.end())
        == std::vector<std::string>{"0", "a", "bb", "ccc", "zzz"});
    }

    SECTION("range")
    {
      auto s = fset{1, 3, 5};
      const auto v = std::vector<int>{4, 2, 4, 6};
      s.insert(v.begin(), v.end());
      CHECK(to_vector(s) == std::vector<int>{1, 2, 3, 4, 5, 6});
    }

    SECTION("range_does_not_overwrite_existing_values")
    {
      auto s = tagged_set{tagged{1, 100}, tagged{2, 100}};
      const auto v = std::vector<tagged>{{2, 200}, {3, 200}};
      s.insert(v.begin(), v.end());

      CHECK(s.size() == 3u);
      CHECK(s.find(tagged{1, 0})->tag == 100);
      CHECK(s.find(tagged{2, 0})->tag == 100);
      CHECK(s.find(tagged{3, 0})->tag == 200);
    }

    SECTION("sorted_unique_range")
    {
      auto s = fset{1, 5};
      const auto v = std::vector<int>{2, 3, 4};
      s.insert(sorted_unique, v.begin(), v.end());
      CHECK(to_vector(s) == std::vector<int>{1, 2, 3, 4, 5});
    }

    SECTION("initializer_list")
    {
      auto s = fset{1, 5};
      s.insert({4, 2, 4});
      CHECK(to_vector(s) == std::vector<int>{1, 2, 4, 5});
    }

    SECTION("range_of_values")
    {
      auto s = fset{1, 5};
      const auto v = std::vector<int>{4, 2, 4, 3};
      s.insert_range(v);
      CHECK(to_vector(s) == std::vector<int>{1, 2, 3, 4, 5});
    }
  }

  SECTION("emplace")
  {
    auto s = fset{1, 3};
    const auto [it, inserted] = s.emplace(2);
    CHECK(inserted);
    CHECK(*it == 2);
    CHECK(to_vector(s) == std::vector<int>{1, 2, 3});

    const auto it2 = s.emplace_hint(s.begin(), 4);
    CHECK(*it2 == 4);
    CHECK(to_vector(s) == std::vector<int>{1, 2, 3, 4});
  }

  SECTION("erase")
  {
    SECTION("by_position")
    {
      auto s = fset{1, 2, 3};
      const auto it = s.erase(s.find(2));
      CHECK(*it == 3);
      CHECK(to_vector(s) == std::vector<int>{1, 3});
    }

    SECTION("by_range")
    {
      auto s = fset{1, 2, 3, 4};
      s.erase(s.find(2), s.find(4));
      CHECK(to_vector(s) == std::vector<int>{1, 4});
    }

    SECTION("by_key")
    {
      auto s = fset{1, 2, 3};
      CHECK(s.erase(2) == 1u);
      CHECK(s.erase(2) == 0u);
      CHECK(to_vector(s) == std::vector<int>{1, 3});
    }
  }

  SECTION("swap")
  {
    auto s1 = fset{1, 2, 3};
    auto s2 = fset{4, 5};

    s1.swap(s2);
    CHECK(to_vector(s1) == std::vector<int>{4, 5});
    CHECK(to_vector(s2) == std::vector<int>{1, 2, 3});

    swap(s1, s2);
    CHECK(to_vector(s1) == std::vector<int>{1, 2, 3});
    CHECK(to_vector(s2) == std::vector<int>{4, 5});
  }

  SECTION("lookup")
  {
    const auto s = fset{1, 2, 3};

    CHECK(s.find(2) != s.end());
    CHECK(*s.find(2) == 2);
    CHECK(s.find(4) == s.end());

    CHECK(s.count(2) == 1u);
    CHECK(s.count(4) == 0u);

    CHECK(s.contains(2));
    CHECK(!s.contains(4));

    CHECK(*s.lower_bound(2) == 2);
    CHECK(*s.upper_bound(2) == 3);

    const auto [first, last] = s.equal_range(2);
    CHECK(std::distance(first, last) == 1);
  }

  SECTION("transparent_lookup")
  {
    const auto s = flat_set<std::string, std::less<>>{"a", "bb", "ccc"};

    CHECK(s.contains(std::string_view{"bb"}));
    CHECK(!s.contains(std::string_view{"dddd"}));
    CHECK(s.find(std::string_view{"bb"}) != s.end());
    CHECK(s.count(std::string_view{"bb"}) == 1u);
  }

  SECTION("key_comp_and_value_comp")
  {
    const auto s = fset{};
    CHECK(s.key_comp()(1, 2));
    CHECK(s.value_comp()(1, 2));
  }

  SECTION("extract_and_replace")
  {
    auto s = fset{1, 2, 3};
    auto data = s.extract();
    CHECK(data == std::vector<int>{1, 2, 3});
    CHECK(s.empty());

    s.replace(std::move(data));
    CHECK(to_vector(s) == std::vector<int>{1, 2, 3});
  }

  SECTION("comparison")
  {
    CHECK(fset{1, 2, 3} == fset{3, 2, 1});
    CHECK(fset{1, 2, 3} != fset{1, 2});
    CHECK(fset{1, 2} < fset{1, 2, 3});

    const auto ordering = fset{1, 2, 3} <=> fset{1, 2, 4};
    CHECK(ordering < 0);
  }

  SECTION("erase_if")
  {
    auto s = fset{1, 2, 3, 4, 5};
    const auto removed = erase_if(s, [](const int x) { return x % 2 == 0; });
    CHECK(removed == 2u);
    CHECK(to_vector(s) == std::vector<int>{1, 3, 5});
  }
}
} // namespace kdl
