/*
 Copyright 2010-2019 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "kdl/compact_trie.h"
#include "kdl/vector_utils.h"

#include <iterator>

#include <catch2/catch.hpp>

namespace kdl {
using test_index = compact_trie<std::string>;

static void assertMatches(
  const test_index& index, const std::string& pattern, std::vector<std::string> expectedMatches) {
  std::vector<std::string> matches;
  index.find_matches(pattern, std::back_inserter(matches));

  CHECK_THAT(matches, Catch::UnorderedEquals(expectedMatches));
}

TEST_CASE("compact_trie_test.insert", "[compact_trie_test]") {
  test_index index;
  index.insert("key", "value");
  index.insert("key2", "value");
  index.insert("key22", "value2");
  index.insert("k1", "value3");
  index.insert("test", "value4");

  assertMatches(index, "whoops", {});

  assertMatches(index, "key222", {});
  assertMatches(index, "key22?", {});
  assertMatches(index, "key22*", {"value2"});
  assertMatches(index, "key%%*", {"value", "value2"});
  assertMatches(index, "key%*", {"value", "value", "value2"});
  assertMatches(index, "key*", {"value", "value", "value2"});

  assertMatches(index, "k*", {"value", "value", "value2", "value3"});
  assertMatches(index, "k*2", {"value", "value2"});

  assertMatches(index, "test", {"value4"});
  assertMatches(index, "test*", {"value4"});
  assertMatches(index, "test?", {});
  assertMatches(index, "test%", {});
  assertMatches(index, "test%*", {"value4"});

  index.insert("k", "value4");

  assertMatches(index, "k", {"value4"});
  assertMatches(index, "k%", {"value3"});
  assertMatches(index, "k*", {"value", "value", "value2", "value3", "value4"});

  assertMatches(index, "*", {"value", "value", "value2", "value3", "value4", "value4"});
}

TEST_CASE("compact_trie_test.remove", "[compact_trie_test]") {
  test_index index;
  index.insert("andrew", "value");
  index.insert("andreas", "value");
  index.insert("andrar", "value2");
  index.insert("andrary", "value3");
  index.insert("andy", "value4");

  assertMatches(index, "*", {"value", "value", "value2", "value3", "value4"});

  CHECK_FALSE(index.remove("andrary", "value2"));

  CHECK(index.remove("andrary", "value3"));
  assertMatches(index, "andrary*", {});

  assertMatches(index, "andrar*", {"value2"});
  CHECK(index.remove("andrar", "value2"));
  assertMatches(index, "andrar*", {});

  assertMatches(index, "andy", {"value4"});
  CHECK(index.remove("andy", "value4"));
  assertMatches(index, "andy", {});

  assertMatches(index, "andre*", {"value", "value"});
  assertMatches(index, "andreas", {"value"});
  CHECK(index.remove("andreas", "value"));
  assertMatches(index, "andre*", {"value"});
  assertMatches(index, "andreas", {});

  assertMatches(index, "andrew", {"value"});
  CHECK(index.remove("andrew", "value"));
  assertMatches(index, "andrew", {});

  assertMatches(index, "*", {});
}

TEST_CASE("compact_trie_test.find_matches_with_exact_pattern", "[compact_trie_test]") {
  test_index index;
  index.insert("key", "value");
  index.insert("key2", "value");
  index.insert("key22", "value2");
  index.insert("k1", "value3");

  assertMatches(index, "whoops", {});
  assertMatches(index, "key222", {});
  assertMatches(index, "key", {"value"});
  assertMatches(index, "k", {});
  assertMatches(index, "k1", {"value3"});

  index.insert("key", "value4");
  assertMatches(index, "key", {"value", "value4"});

  assertMatches(index, "", {});
}

TEST_CASE("compact_trie_test.find_matches_with_wildcards", "[compact_trie_test]") {
  test_index index;
  index.insert("key", "value");
  index.insert("key2", "value");
  index.insert("key22", "value2");
  index.insert("k1", "value3");
  index.insert("test", "value4");

  assertMatches(index, "whoops", {});
  assertMatches(index, "k??%*", {"value", "value", "value2"});
  assertMatches(index, "?ey", {"value"});
  assertMatches(index, "?ey*", {"value", "value", "value2"});
  assertMatches(index, "?*", {"value", "value", "value2", "value3", "value4"});
  assertMatches(index, "*??", {"value", "value", "value2", "value3", "value4"});
  assertMatches(index, "*???", {"value", "value", "value2", "value4"});
  assertMatches(index, "k*2", {"value", "value2"});
  assertMatches(index, "k*", {"value", "value", "value2", "value3"});
  assertMatches(index, "t??t", {"value4"});
  assertMatches(index, "t??*", {"value4"});
  assertMatches(index, "t*", {"value4"});
  assertMatches(index, "*st", {"value4"});
  assertMatches(index, "t*t", {"value4"});
  assertMatches(index, "t??t", {"value4"});

  index.insert("this2345that", "value5");
  assertMatches(index, "t*%%%%that", {"value5"});
  assertMatches(index, "t*%*that", {"value5"});
  assertMatches(index, "t*%**t", {"value4", "value5"});
  assertMatches(index, "t*%**", {"value4", "value5"});
  assertMatches(index, "t*", {"value4", "value5"});
  assertMatches(index, "t**", {"value4", "value5"});
  assertMatches(index, "t?*", {"value4", "value5"});
  assertMatches(index, "t??*", {"value4", "value5"});
  assertMatches(index, "t???*", {"value4", "value5"});
  assertMatches(index, "t????*", {"value5"});
  assertMatches(index, "t*%*", {});
}

TEST_CASE("compact_trie_test.find_matches_with_digit_suffix", "[compact_trie_test]") {
  test_index index;
  index.insert("key", "value");
  index.insert("key2", "value");
  index.insert("key22", "value2");
  index.insert("key22bs", "value4");
  index.insert("k1", "value3");

  assertMatches(index, "whoops", {});
  assertMatches(index, "key%*", {"value", "value", "value2"});
  assertMatches(index, "key%%*", {"value", "value2"});
  assertMatches(index, "key2%*", {"value", "value2"});
  assertMatches(index, "k%*", {"value3"});

  index.remove("k1", "value3");
  assertMatches(index, "k%*", {});
}

TEST_CASE("compact_trie_test.get_keys", "[compact_trie_test]") {
  test_index index;
  index.insert("key", "value");
  index.insert("key2", "value");
  index.insert("key22", "value2");
  index.insert("key22bs", "value4");
  index.insert("k1", "value3");

  std::vector<std::string> keys;
  index.get_keys(std::back_inserter(keys));

  CHECK_THAT(
    keys,
    Catch::UnorderedEquals(std::vector<std::string>{"key", "key2", "key22", "key22bs", "k1"}));
}
} // namespace kdl
