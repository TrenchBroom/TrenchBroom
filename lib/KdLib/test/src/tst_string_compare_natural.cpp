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

#include "kd/collection_utils.h"
#include "kd/string_compare.h"
#include "kd/string_compare_natural.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace kdl
{
using namespace Catch::Matchers;

namespace cs
{
template <typename C>
C sorted_natural(C c)
{
  return kdl::col_sort(std::move(c), string_less_natural());
}
} // namespace cs

namespace ci
{
template <typename C>
C sorted_natural(C c)
{
  return kdl::col_sort(std::move(c), string_less_natural());
}
} // namespace ci

TEST_CASE("string_compare_natural")
{
  SECTION("cs")
  {
    using namespace cs;

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

      // leading zeros are ignored when comparing the numeric value of a run
      CHECK(str_compare_natural("test1", "test02") == -1);
      CHECK(str_compare_natural("007", "8") == -1);
      CHECK(str_compare_natural("8", "007") == +1);
      CHECK(str_compare_natural("wall08", "wall1") == +1);
      CHECK(str_compare_natural("wall1", "wall08") == -1);
      CHECK(str_compare_natural("img09", "img010") == -1);

      // if two runs have the same numeric value only because of different zero
      // padding, the padding breaks the tie by comparing the digits verbatim, so
      // different digit runs never compare equal
      CHECK(str_compare_natural("test01", "test1") == -1);
      CHECK(str_compare_natural("test1", "test01") == +1);
      CHECK(str_compare_natural("007", "7") == -1);
      CHECK(str_compare_natural("7", "007") == +1);

      // a prefix sorts before the longer string
      CHECK(str_compare_natural("", "") == 0);
      CHECK(str_compare_natural("", "wall") == -1);
      CHECK(str_compare_natural("wall", "wall1") == -1);
      CHECK(str_compare_natural("wall1", "wall") == +1);

      // unlike ci::str_compare_natural, case is not ignored
      CHECK(str_compare_natural("WALL16", "wall16") == -1);
      CHECK(str_compare_natural("wall16", "WALL16") == +1);
      CHECK(str_compare_natural("wall16", "WALL128") == +1);

      // strings without digits compare like str_compare. This matters for
      // characters between 'Z' and 'a', such as '_', which is common in material
      // and entity names. Upper case folding would sort these characters after
      // the letters.
      CHECK(str_compare_natural("asdf", "wxyt") == str_compare("asdf", "wxyt"));
      CHECK(str_compare_natural("asdf", "Wxyt") == str_compare("asdf", "Wxyt"));
      CHECK(str_compare_natural("Asdf", "Wxyt") == str_compare("Asdf", "Wxyt"));
      CHECK(str_compare_natural("item_health", "items") == -1);
      CHECK(str_compare_natural("items", "item_health") == +1);
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

      // cs::str_compare_natural never returns 0 for two different strings, so
      // string_less_natural's exact-value tie-break only matters for identical
      // strings, which it correctly treats as not less than themselves
      CHECK(!string_less_natural{}("mymod", "mymod"));

      // case is compared like any other character, so upper case sorts before
      // lower case, ahead of any digits that follow it
      CHECK(string_less_natural{}("Wall16", "wall2"));
      CHECK(!string_less_natural{}("wall2", "Wall16"));

      CHECK_THAT(
        sorted_natural(std::vector<std::string>{
          "wall2",
          "Wall128",
          "wall",
          "Wall16",
        }),
        Equals(std::vector<std::string>{
          "Wall16",
          "Wall128",
          "wall",
          "wall2",
        }));
    }
  }

  SECTION("ci")
  {
    using namespace ci;

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

      // leading zeros are ignored when comparing the numeric value of a run
      CHECK(str_compare_natural("test1", "test02") == -1);
      CHECK(str_compare_natural("007", "8") == -1);
      CHECK(str_compare_natural("8", "007") == +1);
      CHECK(str_compare_natural("wall08", "wall1") == +1);
      CHECK(str_compare_natural("wall1", "wall08") == -1);
      CHECK(str_compare_natural("img09", "img010") == -1);

      // if two runs have the same numeric value only because of different zero
      // padding, the padding breaks the tie by comparing the digits verbatim, so
      // different digit runs never compare equal
      CHECK(str_compare_natural("test01", "test1") == -1);
      CHECK(str_compare_natural("test1", "test01") == +1);
      CHECK(str_compare_natural("007", "7") == -1);
      CHECK(str_compare_natural("7", "007") == +1);

      // whitespace is not skipped: it compares like any other character
      CHECK(str_compare_natural("my mod", "mymod") == str_compare("my mod", "mymod"));
      CHECK(str_compare_natural("a b", "ab") == str_compare("a b", "ab"));
      CHECK(str_compare_natural(" x", "x") == str_compare(" x", "x"));

      // a prefix sorts before the longer string
      CHECK(str_compare_natural("", "") == 0);
      CHECK(str_compare_natural("", "wall") == -1);
      CHECK(str_compare_natural("wall", "wall1") == -1);
      CHECK(str_compare_natural("wall1", "wall") == +1);

      // case is ignored
      CHECK(str_compare_natural("WALL16", "wall128") == -1);
      CHECK(str_compare_natural("wall16", "WALL128") == -1);
      CHECK(str_compare_natural("WALL16", "wall16") == 0);

      // strings without digits compare like str_compare. This matters for
      // characters between 'Z' and 'a', such as '_', which is common in material
      // and entity names. Upper case folding would sort these characters after
      // the letters.
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

      // string_less_natural is a total order. str_compare_natural is not: strings
      // that differ only in case still compare equal. string_less_natural breaks
      // such ties by exact value, so a sort gives the same result each time, and a
      // group by the result is correct.
      CHECK(string_less_natural{}("Wall", "wall"));
      CHECK(!string_less_natural{}("wall", "Wall"));
      CHECK(!string_less_natural{}("wall", "wall"));

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
}

} // namespace kdl
