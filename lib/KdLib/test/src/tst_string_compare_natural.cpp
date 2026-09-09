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

#include "kd/collection_utils.h"
#include "kd/string_compare.h"
#include "kd/string_compare_natural.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace kdl::ci
{
using namespace Catch::Matchers;

namespace
{
template <typename C>
C sorted_natural(C c)
{
  return kdl::col_sort(std::move(c), string_less_natural());
}
} // namespace

TEST_CASE("string_compare_natural")
{
  SECTION("str_compare_natural")
  {
    // runs of digits compare by numeric value
    CHECK(str_compare_natural("16", "128") == -1);
    CHECK(str_compare_natural("128", "16") == +1);
    CHECK(str_compare_natural("16", "16") == 0);
    CHECK(str_compare_natural("wall16", "wall128") == -1);
    CHECK(str_compare_natural("16wall", "128wall") == -1);
    CHECK(str_compare_natural("e1m2", "e1m10") == -1);
    CHECK(str_compare_natural("a1b2", "a1b10") == -1);
    CHECK(str_compare_natural("door1_1", "door10_1") == -1);
    CHECK(str_compare_natural("img9", "img10") == -1);

    // digits with a leading zero compare as a fraction, so zero padded numbers of
    // different width do not compare by value
    CHECK(str_compare_natural("007", "8") == -1);
    CHECK(str_compare_natural("007", "7") == -1);
    CHECK(str_compare_natural("7", "007") == +1);
    CHECK(str_compare_natural("wall08", "wall1") == -1);
    CHECK(str_compare_natural("img09", "img010") == +1);
    CHECK(str_compare_natural("1", "09") == +1);

    // whitespace is skipped, not compared
    CHECK(str_compare_natural("my mod", "mymod") == 0);
    CHECK(str_compare_natural("a b", "ab") == 0);
    CHECK(str_compare_natural(" x", "x") == 0);
    CHECK(str_compare_natural("wall 16", "wall128") == -1);

    // a prefix sorts before the longer string
    CHECK(str_compare_natural("", "") == 0);
    CHECK(str_compare_natural("", "wall") == -1);
    CHECK(str_compare_natural("wall", "wall1") == -1);
    CHECK(str_compare_natural("wall1", "wall") == +1);

    // case is ignored
    CHECK(str_compare_natural("WALL16", "wall128") == -1);
    CHECK(str_compare_natural("wall16", "WALL128") == -1);
    CHECK(str_compare_natural("WALL16", "wall16") == 0);

    // strings without digits or whitespace compare like str_compare. This
    // matters for characters between 'Z' and 'a', such as '_', which is common
    // in material and entity names. Upper case folding would sort these
    // characters after the letters.
    CHECK(str_compare_natural("asdf", "wxyt") == str_compare("asdf", "wxyt"));
    CHECK(str_compare_natural("asdf", "Wxyt") == str_compare("asdf", "Wxyt"));
    CHECK(str_compare_natural("Asdf", "Wxyt") == str_compare("Asdf", "Wxyt"));
    CHECK(str_compare_natural("item_health", "items") == -1);
    CHECK(str_compare_natural("items", "item_health") == +1);
    CHECK(str_compare_natural("ITEM_HEALTH", "items") == -1);
    CHECK(str_compare_natural("wall_x", "wallax") == str_compare("wall_x", "wallax"));
    CHECK(str_compare_natural("a[b", "aab") == str_compare("a[b", "aab"));
    CHECK(str_compare_natural("a`b", "aab") == str_compare("a`b", "aab"));
  }

  SECTION("sort_natural")
  {
    CHECK_THAT(
      sorted_natural(std::vector<std::string>{}), Equals(std::vector<std::string>{}));

    CHECK_THAT(
      sorted_natural(std::vector<std::string>{
        "tex128",
        "tex16",
        "tex2",
        "tex",
      }),
      Equals(std::vector<std::string>{
        "tex",
        "tex2",
        "tex16",
        "tex128",
      }));

    // string_less_natural is a total order. str_compare_natural is not. If two
    // different strings compare equal, string_less_natural orders them by their exact
    // value. Thus a sort gives the same result each time, and a group by the result is
    // correct.
    CHECK(string_less_natural{}("my mod", "mymod"));
    CHECK_FALSE(string_less_natural{}("mymod", "my mod"));
    CHECK_FALSE(string_less_natural{}("my mod", "my mod"));
    CHECK(string_less_natural{}("Wall", "wall"));
    CHECK_FALSE(string_less_natural{}("wall", "Wall"));

    CHECK_THAT(
      sorted_natural(std::vector<std::string>{
        "mymod",
        "id1",
        "my mod",
        "id10",
        "id2",
      }),
      Equals(std::vector<std::string>{
        "id1",
        "id2",
        "id10",
        "my mod",
        "mymod",
      }));
  }
}

} // namespace kdl::ci
