/*
 Copyright 2010-2019 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "kdl/compact_trie.h"
#include "kdl/vector_utils.h"

#include <iterator>

namespace kdl {
    using test_index = compact_trie<std::string>;

#define ASSERT_MATCHES(exp_, index, pattern) {\
    std::vector<std::string> exp(exp_);\
    std::vector<std::string> act;\
    index.find_matches(pattern, std::back_inserter(act));\
    exp = vec_sort(std::move(exp));\
    act = vec_sort(std::move(act));\
    ASSERT_EQ(exp, act);\
}

    TEST_CASE("compact_trie_test.insert", "[compact_trie_test]") {
        test_index index;
        index.insert("key", "value");
        index.insert("key2", "value");
        index.insert("key22", "value2");
        index.insert("k1", "value3");
        index.insert("test", "value4");

        ASSERT_MATCHES(std::vector<std::string>({}), index, "whoops")

        ASSERT_MATCHES(std::vector<std::string>({}), index, "key222")
        ASSERT_MATCHES(std::vector<std::string>({}), index, "key22?")
        ASSERT_MATCHES({ "value2" }, index, "key22*")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value2" }), index, "key%%*")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2" }), index, "key%*")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2" }), index, "key*")

        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2", "value3" }), index, "k*")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value2" }), index, "k*2")

        ASSERT_MATCHES(std::vector<std::string>({ "value4" }), index, "test")
        ASSERT_MATCHES(std::vector<std::string>({ "value4" }), index, "test*")
        ASSERT_MATCHES(std::vector<std::string>({}), index, "test?")
        ASSERT_MATCHES(std::vector<std::string>({}), index, "test%")
        ASSERT_MATCHES(std::vector<std::string>({ "value4" }), index, "test%*")

        index.insert("k", "value4");

        ASSERT_MATCHES(std::vector<std::string>({ "value4" }), index, "k")
        ASSERT_MATCHES(std::vector<std::string>({ "value3" }), index, "k%")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2", "value3", "value4" }), index, "k*")

        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2", "value3", "value4", "value4" }), index, "*")
    }

    TEST_CASE("compact_trie_test.remove", "[compact_trie_test]") {
        test_index index;
        index.insert("andrew", "value");
        index.insert("andreas", "value");
        index.insert("andrar", "value2");
        index.insert("andrary", "value3");
        index.insert("andy", "value4");

        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2", "value3", "value4" }), index, "*")

        ASSERT_FALSE(index.remove("andrary", "value2"));

        ASSERT_TRUE(index.remove("andrary", "value3"));
        ASSERT_MATCHES(std::vector<std::string>({}), index, "andrary*")

        ASSERT_MATCHES(std::vector<std::string>({ "value2" }), index, "andrar*")
        ASSERT_TRUE(index.remove("andrar", "value2"));
        ASSERT_MATCHES(std::vector<std::string>({}), index, "andrar*")

        ASSERT_MATCHES(std::vector<std::string>({ "value4" }), index, "andy")
        ASSERT_TRUE(index.remove("andy", "value4"));
        ASSERT_MATCHES(std::vector<std::string>({}), index, "andy")

        ASSERT_MATCHES(std::vector<std::string>({ "value", "value" }), index, "andre*")
        ASSERT_MATCHES(std::vector<std::string>({ "value" }), index, "andreas")
        ASSERT_TRUE(index.remove("andreas", "value"));
        ASSERT_MATCHES(std::vector<std::string>({ "value" }), index, "andre*")
        ASSERT_MATCHES(std::vector<std::string>({}), index, "andreas")

        ASSERT_MATCHES(std::vector<std::string>({ "value" }), index, "andrew")
        ASSERT_TRUE(index.remove("andrew", "value"));
        ASSERT_MATCHES(std::vector<std::string>({}), index, "andrew")

        ASSERT_MATCHES(std::vector<std::string>({}), index, "*")
    }

    TEST_CASE("compact_trie_test.find_matches_with_exact_pattern", "[compact_trie_test]") {
        test_index index;
        index.insert("key", "value");
        index.insert("key2", "value");
        index.insert("key22", "value2");
        index.insert("k1", "value3");

        ASSERT_MATCHES(std::vector<std::string>({}), index, "whoops")
        ASSERT_MATCHES(std::vector<std::string>({}), index, "key222")
        ASSERT_MATCHES(std::vector<std::string>({ "value" }), index, "key")
        ASSERT_MATCHES(std::vector<std::string>({}), index, "k")
        ASSERT_MATCHES(std::vector<std::string>({ "value3" }), index, "k1")

        index.insert("key", "value4");
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value4" }), index, "key")

        ASSERT_MATCHES(std::vector<std::string>({}), index, "")
    }

    TEST_CASE("compact_trie_test.find_matches_with_wildcards", "[compact_trie_test]") {
        test_index index;
        index.insert("key", "value");
        index.insert("key2", "value");
        index.insert("key22", "value2");
        index.insert("k1", "value3");
        index.insert("test", "value4");

        ASSERT_MATCHES(std::vector<std::string>({}), index, "whoops")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2" }), index, "k??%*")
        ASSERT_MATCHES(std::vector<std::string>({ "value" }), index, "?ey")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2" }), index, "?ey*")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2", "value3", "value4" }), index, "?*")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2", "value3", "value4" }), index, "*??")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2", "value4" }), index, "*???")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value2" }), index, "k*2")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2", "value3" }), index, "k*")
        ASSERT_MATCHES(std::vector<std::string>({ "value4" }), index, "t??t")
        ASSERT_MATCHES(std::vector<std::string>({ "value4" }), index, "t??*")
        ASSERT_MATCHES(std::vector<std::string>({ "value4" }), index, "t*")
        ASSERT_MATCHES(std::vector<std::string>({ "value4" }), index, "*st")
        ASSERT_MATCHES(std::vector<std::string>({ "value4" }), index, "t*t")
        ASSERT_MATCHES(std::vector<std::string>({ "value4" }), index, "t??t")

        index.insert("this2345that", "value5");
        ASSERT_MATCHES(std::vector<std::string>({ "value5" }), index, "t*%%%%that")
        ASSERT_MATCHES(std::vector<std::string>({ "value5" }), index, "t*%*that")
        ASSERT_MATCHES(std::vector<std::string>({ "value4", "value5" }), index, "t*%**t")
        ASSERT_MATCHES(std::vector<std::string>({ "value4", "value5" }), index, "t*%**")
        ASSERT_MATCHES(std::vector<std::string>({ "value4", "value5" }), index, "t*")
        ASSERT_MATCHES(std::vector<std::string>({ "value4", "value5" }), index, "t**")
        ASSERT_MATCHES(std::vector<std::string>({ "value4", "value5" }), index, "t?*")
        ASSERT_MATCHES(std::vector<std::string>({ "value4", "value5" }), index, "t??*")
        ASSERT_MATCHES(std::vector<std::string>({ "value4", "value5" }), index, "t???*")
        ASSERT_MATCHES(std::vector<std::string>({ "value5" }), index, "t????*")
        ASSERT_MATCHES(std::vector<std::string>({}), index, "t*%*")
    }

    TEST_CASE("compact_trie_test.find_matches_with_digit_suffix", "[compact_trie_test]") {
        test_index index;
        index.insert("key", "value");
        index.insert("key2", "value");
        index.insert("key22", "value2");
        index.insert("key22bs", "value4");
        index.insert("k1", "value3");

        ASSERT_MATCHES(std::vector<std::string>({}), index, "whoops")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value", "value2" }), index, "key%*")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value2" }), index, "key%%*")
        ASSERT_MATCHES(std::vector<std::string>({ "value", "value2" }), index, "key2%*")
        ASSERT_MATCHES(std::vector<std::string>({ "value3" }), index, "k%*")

        index.remove("k1", "value3");
        ASSERT_MATCHES(std::vector<std::string>({}), index, "k%*")
    }

    TEST_CASE("compact_trie_test.get_keys", "[compact_trie_test]") {
        test_index index;
        index.insert("key", "value");
        index.insert("key2", "value");
        index.insert("key22", "value2");
        index.insert("key22bs", "value4");
        index.insert("k1", "value3");

        std::vector<std::string> actual;
        index.get_keys(std::back_inserter(actual));
        actual = kdl::col_sort(std::move(actual));

        const auto expected = kdl::col_sort(std::vector<std::string>{ "key", "key2", "key22", "key22bs", "k1" });
        ASSERT_EQ(expected, actual);
    }
}
