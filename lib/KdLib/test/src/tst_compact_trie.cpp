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

#include "kd/compact_trie.h"

#include <iterator>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace kdl
{
using namespace Catch::Matchers;

namespace
{
using test_index = compact_trie<std::string>;

std::vector<std::string> find(const test_index& index, const std::string& pattern)
{
  auto matches = std::vector<std::string>{};
  index.find_matches(pattern, std::back_inserter(matches));
  return matches;
}

} // namespace

TEST_CASE("compact_trie")
{
  test_index index;

  SECTION("escape")
  {
    CHECK(test_index::escape("") == "");
    CHECK(test_index::escape("plain text") == "plain text");
    CHECK(test_index::escape("literal*?%\\") == "literal\\*\\?\\%\\\\");

    index.insert("literal*?%\\", "value");
    index.insert("literal1234", "other");

    CHECK_THAT(
      find(index, test_index::escape("literal*?%\\")), UnorderedRangeEquals({"value"}));
  }

  SECTION("insert")
  {
    index.insert("key", "value");
    index.insert("key2", "value");
    index.insert("key22", "value2");
    index.insert("k1", "value3");
    index.insert("test", "value4");

    CHECK(find(index, "whoops") == std::vector<std::string>{});

    CHECK(find(index, "key222") == std::vector<std::string>{});
    CHECK(find(index, "key22?") == std::vector<std::string>{});
    CHECK_THAT(find(index, "key22*"), UnorderedRangeEquals({"value2"}));
    CHECK_THAT(find(index, "key%%*"), UnorderedRangeEquals({"value", "value2"}));
    CHECK_THAT(find(index, "key%*"), UnorderedRangeEquals({"value", "value", "value2"}));
    CHECK_THAT(find(index, "key*"), UnorderedRangeEquals({"value", "value", "value2"}));

    CHECK_THAT(
      find(index, "k*"), UnorderedRangeEquals({"value", "value", "value2", "value3"}));
    CHECK_THAT(find(index, "k*2"), UnorderedRangeEquals({"value", "value2"}));

    CHECK_THAT(find(index, "test"), UnorderedRangeEquals({"value4"}));
    CHECK_THAT(find(index, "test*"), UnorderedRangeEquals({"value4"}));
    CHECK(find(index, "test?") == std::vector<std::string>{});
    CHECK(find(index, "test%") == std::vector<std::string>{});
    CHECK_THAT(find(index, "test%*"), UnorderedRangeEquals({"value4"}));

    index.insert("k", "value4");

    CHECK_THAT(find(index, "k"), UnorderedRangeEquals({"value4"}));
    CHECK_THAT(find(index, "k%"), UnorderedRangeEquals({"value3"}));
    CHECK_THAT(
      find(index, "k*"),
      UnorderedRangeEquals({"value", "value", "value2", "value3", "value4"}));

    CHECK_THAT(
      find(index, "*"),
      UnorderedRangeEquals({"value", "value", "value2", "value3", "value4", "value4"}));
  }

  SECTION("remove")
  {
    index.insert("andrew", "value");
    index.insert("andreas", "value");
    index.insert("andrar", "value2");
    index.insert("andrary", "value3");
    index.insert("andy", "value4");

    CHECK_THAT(
      find(index, "*"),
      UnorderedRangeEquals({"value", "value", "value2", "value3", "value4"}));

    CHECK(!index.remove("andrary", "value2"));

    CHECK(index.remove("andrary", "value3"));
    CHECK(find(index, "andrary*") == std::vector<std::string>{});

    CHECK_THAT(find(index, "andrar*"), UnorderedRangeEquals({"value2"}));
    CHECK(index.remove("andrar", "value2"));
    CHECK(find(index, "andrar*") == std::vector<std::string>{});

    CHECK_THAT(find(index, "andy"), UnorderedRangeEquals({"value4"}));
    CHECK(index.remove("andy", "value4"));
    CHECK(find(index, "andy") == std::vector<std::string>{});

    CHECK_THAT(find(index, "andre*"), UnorderedRangeEquals({"value", "value"}));
    CHECK_THAT(find(index, "andreas"), UnorderedRangeEquals({"value"}));
    CHECK(index.remove("andreas", "value"));
    CHECK_THAT(find(index, "andre*"), UnorderedRangeEquals({"value"}));
    CHECK(find(index, "andreas") == std::vector<std::string>{});

    CHECK_THAT(find(index, "andrew"), UnorderedRangeEquals({"value"}));
    CHECK(index.remove("andrew", "value"));
    CHECK(find(index, "andrew") == std::vector<std::string>{});

    CHECK(find(index, "*") == std::vector<std::string>{});
  }

  SECTION("find_matches_with_exact_pattern")
  {
    index.insert("key", "value");
    index.insert("key2", "value");
    index.insert("key22", "value2");
    index.insert("k1", "value3");

    CHECK(find(index, "whoops") == std::vector<std::string>{});
    CHECK(find(index, "key222") == std::vector<std::string>{});
    CHECK_THAT(find(index, "key"), UnorderedRangeEquals({"value"}));
    CHECK(find(index, "k") == std::vector<std::string>{});
    CHECK_THAT(find(index, "k1"), UnorderedRangeEquals({"value3"}));

    index.insert("key", "value4");
    CHECK_THAT(find(index, "key"), UnorderedRangeEquals({"value", "value4"}));

    CHECK(find(index, "") == std::vector<std::string>{});
  }

  SECTION("find_matches_with_wildcards")
  {
    index.insert("key", "value");
    index.insert("key2", "value");
    index.insert("key22", "value2");
    index.insert("k1", "value3");
    index.insert("test", "value4");

    CHECK(find(index, "whoops") == std::vector<std::string>{});
    CHECK_THAT(find(index, "k??%*"), UnorderedRangeEquals({"value", "value", "value2"}));
    CHECK_THAT(find(index, "?ey"), UnorderedRangeEquals({"value"}));
    CHECK_THAT(find(index, "?ey*"), UnorderedRangeEquals({"value", "value", "value2"}));
    CHECK_THAT(
      find(index, "?*"),
      UnorderedRangeEquals({"value", "value", "value2", "value3", "value4"}));
    CHECK_THAT(
      find(index, "*??"),
      UnorderedRangeEquals({"value", "value", "value2", "value3", "value4"}));
    CHECK_THAT(
      find(index, "*???"), UnorderedRangeEquals({"value", "value", "value2", "value4"}));
    CHECK_THAT(find(index, "k*2"), UnorderedRangeEquals({"value", "value2"}));
    CHECK_THAT(
      find(index, "k*"), UnorderedRangeEquals({"value", "value", "value2", "value3"}));
    CHECK_THAT(find(index, "t??t"), UnorderedRangeEquals({"value4"}));
    CHECK_THAT(find(index, "t??*"), UnorderedRangeEquals({"value4"}));
    CHECK_THAT(find(index, "t*"), UnorderedRangeEquals({"value4"}));
    CHECK_THAT(find(index, "*st"), UnorderedRangeEquals({"value4"}));
    CHECK_THAT(find(index, "t*t"), UnorderedRangeEquals({"value4"}));
    CHECK_THAT(find(index, "t??t"), UnorderedRangeEquals({"value4"}));

    index.insert("this2345that", "value5");
    CHECK_THAT(find(index, "t*%%%%that"), UnorderedRangeEquals({"value5"}));
    CHECK_THAT(find(index, "t*%*that"), UnorderedRangeEquals({"value5"}));
    CHECK_THAT(find(index, "t*%**t"), UnorderedRangeEquals({"value4", "value5"}));
    CHECK_THAT(find(index, "t*%**"), UnorderedRangeEquals({"value4", "value5"}));
    CHECK_THAT(find(index, "t*"), UnorderedRangeEquals({"value4", "value5"}));
    CHECK_THAT(find(index, "t**"), UnorderedRangeEquals({"value4", "value5"}));
    CHECK_THAT(find(index, "t?*"), UnorderedRangeEquals({"value4", "value5"}));
    CHECK_THAT(find(index, "t??*"), UnorderedRangeEquals({"value4", "value5"}));
    CHECK_THAT(find(index, "t???*"), UnorderedRangeEquals({"value4", "value5"}));
    CHECK_THAT(find(index, "t????*"), UnorderedRangeEquals({"value5"}));
    CHECK(find(index, "t*%*") == std::vector<std::string>{});
  }

  SECTION("find_matches_with_digit_suffix")
  {
    index.insert("key", "value");
    index.insert("key2", "value");
    index.insert("key22", "value2");
    index.insert("key22bs", "value4");
    index.insert("k1", "value3");

    CHECK(find(index, "whoops") == std::vector<std::string>{});
    CHECK_THAT(find(index, "key%*"), UnorderedRangeEquals({"value", "value", "value2"}));
    CHECK_THAT(find(index, "key%%*"), UnorderedRangeEquals({"value", "value2"}));
    CHECK_THAT(find(index, "key2%*"), UnorderedRangeEquals({"value", "value2"}));
    CHECK_THAT(find(index, "k%*"), UnorderedRangeEquals({"value3"}));

    index.remove("k1", "value3");
    CHECK(find(index, "k%*") == std::vector<std::string>{});
  }

  SECTION("get_keys")
  {
    index.insert("key", "value");
    index.insert("key2", "value");
    index.insert("key22", "value2");
    index.insert("key22bs", "value4");
    index.insert("k1", "value3");

    std::vector<std::string> keys;
    index.get_keys(std::back_inserter(keys));

    CHECK_THAT(keys, UnorderedRangeEquals({"key", "key2", "key22", "key22bs", "k1"}));
  }
}

} // namespace kdl
