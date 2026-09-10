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

#include "kd/flat_map.h"
#include "kd/ranges/to.h"

#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{
namespace
{
using fmap = flat_map<int, std::string>;

std::vector<std::pair<int, std::string>> to_vector(const fmap& m)
{
  return {m.begin(), m.end()};
}

} // namespace

TEST_CASE("flat_map")
{
  SECTION("constructor")
  {
    SECTION("default")
    {
      const auto m = fmap{};
      CHECK(m.empty());
      CHECK(m.size() == 0u);
    }

    SECTION("with_comparator")
    {
      const auto m = fmap{std::less<int>{}};
      CHECK(m.empty());
    }

    SECTION("with_containers")
    {
      const auto m =
        fmap{fmap::containers{{2, 1, 3, 1}, {"two", "one_a", "three", "one_b"}}};
      CHECK(
        to_vector(m)
        == std::vector<std::pair<int, std::string>>{
          {1, "one_a"}, {2, "two"}, {3, "three"}});
    }

    SECTION("with_sorted_unique_containers")
    {
      const auto m =
        fmap{sorted_unique, fmap::containers{{1, 2, 3}, {"one", "two", "three"}}};
      CHECK(
        to_vector(m)
        == std::vector<std::pair<int, std::string>>{
          {1, "one"}, {2, "two"}, {3, "three"}});
    }

    SECTION("with_range")
    {
      const auto v =
        std::vector<std::pair<int, std::string>>{{2, "two"}, {1, "one"}, {3, "three"}};
      const auto m = fmap{v.begin(), v.end()};
      CHECK(
        to_vector(m)
        == std::vector<std::pair<int, std::string>>{
          {1, "one"}, {2, "two"}, {3, "three"}});
    }

    SECTION("with_sorted_unique_range")
    {
      const auto v =
        std::vector<std::pair<int, std::string>>{{1, "one"}, {2, "two"}, {3, "three"}};
      const auto m = fmap{sorted_unique, v.begin(), v.end()};
      CHECK(
        to_vector(m)
        == std::vector<std::pair<int, std::string>>{
          {1, "one"}, {2, "two"}, {3, "three"}});
    }

    SECTION("with_initializer_list")
    {
      const auto m = fmap{{2, "two"}, {1, "one"}, {3, "three"}, {1, "one_dup"}};
      CHECK(
        to_vector(m)
        == std::vector<std::pair<int, std::string>>{
          {1, "one"}, {2, "two"}, {3, "three"}});
    }

    SECTION("with_sorted_unique_initializer_list")
    {
      const auto m = fmap{sorted_unique, {{1, "one"}, {2, "two"}, {3, "three"}}};
      CHECK(
        to_vector(m)
        == std::vector<std::pair<int, std::string>>{
          {1, "one"}, {2, "two"}, {3, "three"}});
    }

    SECTION("deduction_guide")
    {
      const auto v =
        std::vector<std::pair<int, std::string>>{{2, "two"}, {1, "one"}, {3, "three"}};
      const auto m1 = flat_map(v.begin(), v.end());
      CHECK(
        std::vector<std::pair<int, std::string>>(m1.begin(), m1.end())
        == std::vector<std::pair<int, std::string>>{
          {1, "one"}, {2, "two"}, {3, "three"}});

      const auto m2 =
        flat_map{std::pair{1, std::string{"one"}}, std::pair{2, std::string{"two"}}};
      CHECK(
        std::vector<std::pair<int, std::string>>(m2.begin(), m2.end())
        == std::vector<std::pair<int, std::string>>{{1, "one"}, {2, "two"}});
    }
  }

  SECTION("assignment")
  {
    auto m = fmap{{7, "seven"}, {8, "eight"}};
    m = {{2, "two"}, {1, "one"}, {1, "one_dup"}};
    CHECK(
      to_vector(m) == std::vector<std::pair<int, std::string>>{{1, "one"}, {2, "two"}});
  }

  SECTION("iterators")
  {
    auto m = fmap{{3, "three"}, {1, "one"}, {2, "two"}};
    CHECK(
      to_vector(m)
      == std::vector<std::pair<int, std::string>>{{1, "one"}, {2, "two"}, {3, "three"}});

    // mutate a value through a non-const iterator
    m.begin()->second = "ONE";
    CHECK(m.at(1) == "ONE");

    const auto& cm = m;
    CHECK(cm.begin()->first == 1);
    CHECK(cm.begin()->second == "ONE");

    auto reversed = std::vector<int>{};
    for (auto it = m.rbegin(); it != m.rend(); ++it)
    {
      reversed.push_back(it->first);
    }
    CHECK(reversed == std::vector<int>{3, 2, 1});
  }

  SECTION("capacity")
  {
    CHECK(fmap{}.empty());
    CHECK(fmap{}.size() == 0u);
    CHECK(!fmap{{1, "one"}}.empty());
    CHECK(fmap{{1, "one"}, {2, "two"}}.size() == 2u);
  }

  SECTION("clear")
  {
    auto m = fmap{{1, "one"}, {2, "two"}};
    m.clear();
    CHECK(m.empty());
  }

  SECTION("insert")
  {
    SECTION("single_pair")
    {
      auto m = fmap{{1, "one"}, {3, "three"}};

      const auto [it1, inserted1] = m.insert({2, "two"});
      CHECK(inserted1);
      CHECK(it1->second == "two");

      const auto [it2, inserted2] = m.insert(std::pair{2, std::string{"TWO"}});
      CHECK(!inserted2);
      CHECK(it2->second == "two");
    }

    SECTION("with_hint")
    {
      SECTION("valid_hint_in_middle")
      {
        auto m = fmap{{1, "one"}, {3, "three"}, {5, "five"}, {7, "seven"}};
        const auto it = m.insert(m.find(5), {4, "four"});
        CHECK(it->second == "four");
        CHECK(
          to_vector(m)
          == std::vector<std::pair<int, std::string>>{
            {1, "one"}, {3, "three"}, {4, "four"}, {5, "five"}, {7, "seven"}});
      }

      SECTION("invalid_hint_falls_back_to_search")
      {
        auto m = fmap{{1, "one"}, {3, "three"}, {5, "five"}, {7, "seven"}};
        const auto it = m.insert(m.find(3), {6, "six"});
        CHECK(it->second == "six");
        CHECK(
          to_vector(m)
          == std::vector<std::pair<int, std::string>>{
            {1, "one"}, {3, "three"}, {5, "five"}, {6, "six"}, {7, "seven"}});
      }
    }

    SECTION("range")
    {
      auto m = fmap{{1, "one"}, {3, "three"}};
      const auto v = std::vector<std::pair<int, std::string>>{{2, "two"}, {3, "THREE"}};
      m.insert(v.begin(), v.end());

      // key 3 already existed, so its value must not have been overwritten
      CHECK(
        to_vector(m)
        == std::vector<std::pair<int, std::string>>{
          {1, "one"}, {2, "two"}, {3, "three"}});
    }

    SECTION("sorted_unique_range")
    {
      auto m = fmap{{1, "one"}, {5, "five"}};
      const auto v = std::vector<std::pair<int, std::string>>{{2, "two"}, {3, "three"}};
      m.insert(sorted_unique, v.begin(), v.end());
      CHECK(
        to_vector(m)
        == std::vector<std::pair<int, std::string>>{
          {1, "one"}, {2, "two"}, {3, "three"}, {5, "five"}});
    }

    SECTION("initializer_list")
    {
      auto m = fmap{{1, "one"}, {5, "five"}};
      m.insert({{4, "four"}, {2, "two"}});
      CHECK(
        to_vector(m)
        == std::vector<std::pair<int, std::string>>{
          {1, "one"}, {2, "two"}, {4, "four"}, {5, "five"}});
    }

    SECTION("range_of_values")
    {
      auto m = fmap{{1, "one"}, {5, "five"}};
      const auto v = std::vector<std::pair<int, std::string>>{{4, "four"}, {3, "three"}};
      m.insert_range(v);
      CHECK(
        to_vector(m)
        == std::vector<std::pair<int, std::string>>{
          {1, "one"}, {3, "three"}, {4, "four"}, {5, "five"}});
    }
  }

  SECTION("insert_or_assign")
  {
    auto m = fmap{{1, "one"}};

    const auto [it1, inserted1] = m.insert_or_assign(2, std::string{"two"});
    CHECK(inserted1);
    CHECK(it1->second == "two");

    const auto [it2, inserted2] = m.insert_or_assign(2, std::string{"TWO"});
    CHECK(!inserted2);
    CHECK(it2->second == "TWO");
    CHECK(m.at(2) == "TWO");
  }

  SECTION("try_emplace")
  {
    auto m = fmap{{1, "one"}};

    const auto [it1, inserted1] = m.try_emplace(2, "two");
    CHECK(inserted1);
    CHECK(it1->second == "two");

    const auto [it2, inserted2] = m.try_emplace(2, "TWO");
    CHECK(!inserted2);
    CHECK(it2->second == "two");
  }

  SECTION("emplace")
  {
    auto m = fmap{{1, "one"}, {3, "three"}};
    const auto [it, inserted] = m.emplace(2, "two");
    CHECK(inserted);
    CHECK(it->second == "two");

    const auto it2 = m.emplace_hint(m.begin(), 4, "four");
    CHECK(it2->second == "four");
    CHECK(
      to_vector(m)
      == std::vector<std::pair<int, std::string>>{
        {1, "one"}, {2, "two"}, {3, "three"}, {4, "four"}});
  }

  SECTION("erase")
  {
    SECTION("by_position")
    {
      auto m = fmap{{1, "one"}, {2, "two"}, {3, "three"}};
      const auto it = m.erase(m.find(2));
      CHECK(it->first == 3);
      CHECK(
        to_vector(m)
        == std::vector<std::pair<int, std::string>>{{1, "one"}, {3, "three"}});
    }

    SECTION("by_range")
    {
      auto m = fmap{{1, "one"}, {2, "two"}, {3, "three"}, {4, "four"}};
      m.erase(m.find(2), m.find(4));
      CHECK(
        to_vector(m)
        == std::vector<std::pair<int, std::string>>{{1, "one"}, {4, "four"}});
    }

    SECTION("by_key")
    {
      auto m = fmap{{1, "one"}, {2, "two"}};
      CHECK(m.erase(2) == 1u);
      CHECK(m.erase(2) == 0u);
      CHECK(to_vector(m) == std::vector<std::pair<int, std::string>>{{1, "one"}});
    }
  }

  SECTION("swap")
  {
    auto m1 = fmap{{1, "one"}, {2, "two"}};
    auto m2 = fmap{{3, "three"}};

    m1.swap(m2);
    CHECK(to_vector(m1) == std::vector<std::pair<int, std::string>>{{3, "three"}});
    CHECK(
      to_vector(m2) == std::vector<std::pair<int, std::string>>{{1, "one"}, {2, "two"}});

    swap(m1, m2);
    CHECK(
      to_vector(m1) == std::vector<std::pair<int, std::string>>{{1, "one"}, {2, "two"}});
    CHECK(to_vector(m2) == std::vector<std::pair<int, std::string>>{{3, "three"}});
  }

  SECTION("operator_index")
  {
    auto m = fmap{{1, "one"}};
    m[1] = "ONE";
    CHECK(m.at(1) == "ONE");

    m[2] = "two";
    CHECK(m.at(2) == "two");
    CHECK(m.size() == 2u);
  }

  SECTION("at")
  {
    auto m = fmap{{1, "one"}};
    CHECK(m.at(1) == "one");
    CHECK_THROWS_AS(m.at(2), std::out_of_range);

    const auto& cm = m;
    CHECK(cm.at(1) == "one");
    CHECK_THROWS_AS(cm.at(2), std::out_of_range);
  }

  SECTION("lookup")
  {
    auto m = fmap{{1, "one"}, {2, "two"}, {3, "three"}};

    CHECK(m.find(2) != m.end());
    CHECK(m.find(2)->second == "two");
    CHECK(m.find(4) == m.end());

    CHECK(m.count(2) == 1u);
    CHECK(m.count(4) == 0u);

    CHECK(m.contains(2));
    CHECK(!m.contains(4));

    CHECK(m.lower_bound(2)->first == 2);
    CHECK(m.upper_bound(2)->first == 3);

    const auto [first, last] = m.equal_range(2);
    CHECK(std::distance(first, last) == 1);
  }

  SECTION("transparent_lookup")
  {
    auto m = flat_map<std::string, int, std::less<>>{{"a", 1}, {"bb", 2}, {"ccc", 3}};

    CHECK(m.contains(std::string_view{"bb"}));
    CHECK(!m.contains(std::string_view{"dddd"}));
    CHECK(m.find(std::string_view{"bb"})->second == 2);
    CHECK(m.count(std::string_view{"bb"}) == 1u);
  }

  SECTION("key_comp_and_value_comp")
  {
    const auto m = fmap{};
    CHECK(m.key_comp()(1, 2));
    CHECK(m.value_comp()({1, "a"}, {2, "b"}));
  }

  SECTION("extract_and_replace")
  {
    auto m = fmap{{1, "one"}, {2, "two"}};
    auto [keys, values] = m.extract();
    CHECK(keys == std::vector<int>{1, 2});
    CHECK(values == std::vector<std::string>{"one", "two"});
    CHECK(m.empty());

    m.replace(std::move(keys), std::move(values));
    CHECK(
      to_vector(m) == std::vector<std::pair<int, std::string>>{{1, "one"}, {2, "two"}});
  }

  SECTION("keys_and_values")
  {
    const auto m = fmap{{1, "one"}, {2, "two"}};
    CHECK(m.keys() == std::vector<int>{1, 2});
    CHECK(m.values() == std::vector<std::string>{"one", "two"});

    // unlike `m | std::views::keys`, which does not compile against kdl::flat_map (see
    // the comment on keys()/values() in flat_map.h for why), this does, because keys()
    // exposes the real underlying container rather than going through the map's proxy
    // iterator
    const auto doubledKeys = m.keys()
                             | std::views::transform([](const int k) { return k * 2; })
                             | kdl::ranges::to<std::vector>();
    CHECK(doubledKeys == std::vector<int>{2, 4});
  }

  SECTION("comparison")
  {
    CHECK(fmap{{1, "one"}, {2, "two"}} == fmap{{2, "two"}, {1, "one"}});
    CHECK(fmap{{1, "one"}} != fmap{{1, "one"}, {2, "two"}});
    CHECK(fmap{{1, "one"}} < fmap{{1, "one"}, {2, "two"}});

    const auto ordering = fmap{{1, "one"}} <=> fmap{{1, "two"}};
    CHECK(ordering < 0);
  }

  SECTION("erase_if")
  {
    auto m = fmap{{1, "one"}, {2, "two"}, {3, "three"}, {4, "four"}};
    const auto removed = erase_if(m, [](const auto& e) { return e.first % 2 == 0; });
    CHECK(removed == 2u);
    CHECK(
      to_vector(m) == std::vector<std::pair<int, std::string>>{{1, "one"}, {3, "three"}});
  }
}
} // namespace kdl
