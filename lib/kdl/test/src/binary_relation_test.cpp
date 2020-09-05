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

#include "kdl/binary_relation.h"

#include <algorithm>

namespace kdl {
    template <typename L, typename R>
    void assertRelation(const std::vector<std::pair<L, R>>& exp, const binary_relation<L, R>& act) {
        ASSERT_TRUE(std::equal(std::begin(exp), std::end(exp), std::begin(act), std::end(act)));
    }

    TEST_CASE("binary_relation_test.constructor_default", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        relation r;
        ASSERT_TRUE(r.empty());
    }

    TEST_CASE("binary_relation_test.constructor_intializer_list", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        relation r({
            { 1, "a" },
            { 1, "b" },
            { 2, "b" },
            { 3, "b" },
            { 4, "c" },
            { 4, "c" },
        });

        assertRelation({
            { 1, "a" },
            { 1, "b" },
            { 2, "b" },
            { 3, "b" },
            { 4, "c" },
        }, r);
    }

    TEST_CASE("binary_relation_test.empty", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        ASSERT_TRUE(relation().empty());
        ASSERT_FALSE(relation({{ 1, "a" }}).empty());
    }

    TEST_CASE("binary_relation_test.size", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        ASSERT_EQ(0u, relation().size());
        ASSERT_EQ(1u, relation({{ 1, "a" }}).size());
        ASSERT_EQ(2u, relation({{ 1, "a" }, { 1, "b" }}).size());
        ASSERT_EQ(3u, relation({{ 1, "a" }, { 1, "b" }, { 2, "c" }}).size());
    }

    TEST_CASE("binary_relation_test.contains", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        ASSERT_FALSE(relation().contains(1, "a"));
        ASSERT_FALSE(relation({{ 1, "b" }}).contains(1, "a"));
        ASSERT_FALSE(relation({{ 2, "a" }}).contains(1, "a"));
        ASSERT_TRUE(relation({{ 1, "a" }}).contains(1, "a"));
    }

    TEST_CASE("binary_relation_test.count_left", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        ASSERT_EQ(0u, relation().count_left("a"));
        ASSERT_EQ(0u, relation({{ 1, "b" }}).count_left("a"));
        ASSERT_EQ(1u, relation({{ 1, "a" }}).count_left("a"));
        ASSERT_EQ(1u, relation({{ 1, "a" }, { 1, "b" }}).count_left("a"));
        ASSERT_EQ(2u, relation({{ 1, "a" }, { 1, "b" }, { 2, "a" }}).count_left("a"));
    }

    TEST_CASE("binary_relation_test.count_right", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        ASSERT_EQ(0u, relation().count_right(1));
        ASSERT_EQ(0u, relation({{ 2, "a" }}).count_right(1));
        ASSERT_EQ(1u, relation({{ 1, "a" }}).count_right(1));
        ASSERT_EQ(1u, relation({{ 1, "a" }, { 2, "a" }}).count_right(1));
        ASSERT_EQ(2u, relation({{ 1, "a" }, { 1, "b" }, { 2, "a" }}).count_right(1));
    }

    TEST_CASE("binary_relation_test.iterator", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        relation r;
        ASSERT_TRUE(std::begin(r) == std::end(r));
        ASSERT_FALSE(std::begin(r) != std::end(r));

        r.insert(1, "a");
        r.insert(1, "b");
        r.insert(2, "b");
        r.insert(3, "c");

        auto it = std::begin(r);
        auto end = std::end(r);

        ASSERT_FALSE(it == end);
        ASSERT_TRUE(it != end);
        ASSERT_EQ(std::make_pair(1, std::string("a")), *it);

        ++it;
        ASSERT_FALSE(it == end);
        ASSERT_TRUE(it != end);
        ASSERT_EQ(std::make_pair(1, std::string("b")), *it);

        ++it;
        ASSERT_FALSE(it == end);
        ASSERT_TRUE(it != end);
        ASSERT_EQ(std::make_pair(2, std::string("b")), *it);

        ++it;
        ASSERT_FALSE(it == end);
        ASSERT_TRUE(it != end);
        ASSERT_EQ(std::make_pair(3, std::string("c")), *it);

        ++it;
        ASSERT_TRUE(it == end);
        ASSERT_FALSE(it != end);
    }

    template <typename T, typename I>
    void assertRange(const std::vector<T>& exp, const std::pair<I, I>& act) {
        ASSERT_TRUE(std::equal(std::begin(exp), std::end(exp), act.first, act.second));
    }

    TEST_CASE("binary_relation_test.left_range", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        assertRange<int>({}, relation().left_range("a"));
        assertRange<int>({}, relation({{ 1, "b" }}).left_range("a"));
        assertRange<int>({ 1 }, relation({{ 1, "a" }}).left_range("a"));
        assertRange<int>({ 1, 2 }, relation({{ 1, "a" }, { 2, "a" }, { 3, "b" }}).left_range("a"));
    }

    TEST_CASE("binary_relation_test.right_range", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        assertRange<std::string>({}, relation().right_range(1));
        assertRange<std::string>({}, relation({{ 2, "b" }}).right_range(1));
        assertRange<std::string>({ "a" }, relation({{ 1, "a" }}).right_range(1));
        assertRange<std::string>({ "a", "b" }, relation({{ 1, "a" }, { 1, "b" }, { 2, "c" }}).right_range(1));
    }

    TEST_CASE("binary_relation_test.insert_relation", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        relation r;
        r.insert(relation({
            { 1, "a" },
            { 1, "b" },
            { 2, "b" },
            { 3, "b" },
            { 4, "c" },
            { 4, "c" },
        }));


        assertRelation({
            { 1, "a" },
            { 1, "b" },
            { 2, "b" },
            { 3, "b" },
            { 4, "c" },
        }, r);
    }


    TEST_CASE("binary_relation_test.insert_right_range", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        relation r;

        const size_t left_1 = 1;
        const std::vector<std::string> right_1({ "a", "b" });

        r.insert(left_1, std::begin(right_1), std::end(right_1));

        ASSERT_EQ(2u, r.size());
        ASSERT_TRUE(r.contains(left_1, right_1[0]));
        ASSERT_TRUE(r.contains(left_1, right_1[1]));
        ASSERT_EQ(1u, r.count_left(right_1[0]));
        ASSERT_EQ(1u, r.count_left(right_1[1]));
        ASSERT_EQ(2u, r.count_right(left_1));
        ASSERT_TRUE(std::equal(std::begin(right_1), std::end(right_1), r.right_begin(left_1)));

        const size_t left_2 = 2;
        const std::vector<std::string> right_2({ "b", "c" });

        r.insert(left_2, std::begin(right_2), std::end(right_2));

        ASSERT_EQ(4u, r.size());
        ASSERT_TRUE(r.contains(left_2, right_2[0]));
        ASSERT_TRUE(r.contains(left_2, right_2[1]));
        ASSERT_EQ(2u, r.count_left(right_2[0]));
        ASSERT_EQ(1u, r.count_left(right_2[1]));
        ASSERT_EQ(2u, r.count_right(left_2));
        ASSERT_TRUE(std::equal(std::begin(right_2), std::end(right_2), r.right_begin(left_2)));

        const size_t left_3 = left_1;
        const std::vector<std::string> right_3({ "a", "b", "c" });
        r.insert(left_1, std::begin(right_3), std::end(right_3));

        ASSERT_EQ(5u, r.size());
        ASSERT_TRUE(r.contains(left_3, right_3[0]));
        ASSERT_TRUE(r.contains(left_3, right_3[1]));
        ASSERT_TRUE(r.contains(left_3, right_3[2]));
        ASSERT_EQ(1u, r.count_left(right_3[0]));
        ASSERT_EQ(2u, r.count_left(right_3[1]));
        ASSERT_EQ(2u, r.count_left(right_3[2]));
        ASSERT_EQ(3u, r.count_right(left_3));
        ASSERT_TRUE(std::equal(std::begin(right_3), std::end(right_3), r.right_begin(left_3)));
    }

    TEST_CASE("binary_relation_test.insert_left_range", "[binary_relation_test]") {
        using relation = binary_relation<std::string, size_t>;

        relation r;

        const std::vector<std::string> left_1({ "a", "b" });
        const size_t right_1 = 1;

        r.insert(std::begin(left_1), std::end(left_1), right_1);

        ASSERT_EQ(2u, r.size());
        ASSERT_TRUE(r.contains(left_1[0], right_1));
        ASSERT_TRUE(r.contains(left_1[1], right_1));
        ASSERT_EQ(1u, r.count_right(left_1[0]));
        ASSERT_EQ(1u, r.count_right(left_1[1]));
        ASSERT_EQ(2u, r.count_left(right_1));
        ASSERT_TRUE(std::equal(std::begin(left_1), std::end(left_1), r.left_begin(right_1)));

        const std::vector<std::string> left_2({ "b", "c" });
        const size_t right_2 = 2;

        r.insert(std::begin(left_2), std::end(left_2), right_2);

        ASSERT_EQ(4u, r.size());
        ASSERT_TRUE(r.contains(left_2[0], right_2));
        ASSERT_TRUE(r.contains(left_2[1], right_2));
        ASSERT_EQ(2u, r.count_right(left_2[0]));
        ASSERT_EQ(1u, r.count_right(left_2[1]));
        ASSERT_EQ(2u, r.count_left(right_2));
        ASSERT_TRUE(std::equal(std::begin(left_2), std::end(left_2), r.left_begin(right_2)));

        const std::vector<std::string> left_3({ "a", "b", "c" });
        const size_t right_3 = right_1;
        r.insert(std::begin(left_3), std::end(left_3), right_3);

        ASSERT_EQ(5u, r.size());
        ASSERT_TRUE(r.contains(left_3[0], right_3));
        ASSERT_TRUE(r.contains(left_3[1], right_3));
        ASSERT_TRUE(r.contains(left_3[2], right_3));
        ASSERT_EQ(1u, r.count_right(left_3[0]));
        ASSERT_EQ(2u, r.count_right(left_3[1]));
        ASSERT_EQ(2u, r.count_right(left_3[2]));
        ASSERT_EQ(3u, r.count_left(right_3));
        ASSERT_TRUE(std::equal(std::begin(left_3), std::end(left_3), r.left_begin(right_3)));
    }

    TEST_CASE("binary_relation_test.insert_values", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        relation r;
        ASSERT_TRUE(r.insert(1, "a"));

        ASSERT_EQ(1u, r.size());
        ASSERT_FALSE(r.empty());
        ASSERT_TRUE(r.contains(1, "a"));
        ASSERT_EQ(1u, r.count_left("a"));
        ASSERT_EQ(1u, r.count_right(1));

        ASSERT_FALSE(r.insert(1, "a"));
        ASSERT_EQ(1u, r.size());

        ASSERT_TRUE(r.insert(1, "b"));
        ASSERT_EQ(2u, r.size());
        ASSERT_TRUE(r.contains(1, "b"));
        ASSERT_EQ(1u, r.count_left("a"));
        ASSERT_EQ(1u, r.count_left("b"));
        ASSERT_EQ(2u, r.count_right(1));

        ASSERT_TRUE(r.insert(2, "b"));
        ASSERT_EQ(3u, r.size());
        ASSERT_EQ(1u, r.count_left("a"));
        ASSERT_EQ(2u, r.count_left("b"));
        ASSERT_EQ(2u, r.count_right(1));
        ASSERT_EQ(1u, r.count_right(2));
    }

    TEST_CASE("binary_relation_test.erase", "[binary_relation_test]") {
        using relation = binary_relation<int, std::string>;

        relation r;
        r.insert(1, "a");
        r.insert(1, "b");
        r.insert(2, "b");
        r.insert(3, "c");

        // just to make sure
        ASSERT_EQ(4u, r.size());
        ASSERT_TRUE(r.contains(1, "a"));
        ASSERT_TRUE(r.contains(1, "b"));
        ASSERT_TRUE(r.contains(2, "b"));
        ASSERT_TRUE(r.contains(3, "c"));

        ASSERT_FALSE(r.erase(3, "a"));
        ASSERT_FALSE(r.erase(4, ""));
        ASSERT_FALSE(r.erase(3, "a"));

        ASSERT_TRUE(r.erase(1, "a"));
        ASSERT_EQ(3u, r.size());
        ASSERT_FALSE(r.contains(1, "a"));
        ASSERT_TRUE(r.contains(1, "b"));
        ASSERT_TRUE(r.contains(2, "b"));
        ASSERT_TRUE(r.contains(3, "c"));
        ASSERT_FALSE(r.erase(1, "a"));

        ASSERT_TRUE(r.erase(3, "c"));
        ASSERT_EQ(2u, r.size());
        ASSERT_FALSE(r.contains(1, "a"));
        ASSERT_TRUE(r.contains(1, "b"));
        ASSERT_TRUE(r.contains(2, "b"));
        ASSERT_FALSE(r.contains(3, "c"));
        ASSERT_FALSE(r.erase(3, "c"));

        ASSERT_TRUE(r.erase(1, "b"));
        ASSERT_EQ(1u, r.size());
        ASSERT_FALSE(r.contains(1, "a"));
        ASSERT_FALSE(r.contains(1, "b"));
        ASSERT_TRUE(r.contains(2, "b"));
        ASSERT_FALSE(r.contains(3, "c"));
        ASSERT_FALSE(r.erase(1, "b"));

        ASSERT_TRUE(r.erase(2, "b"));
        ASSERT_EQ(0u, r.size());
        ASSERT_FALSE(r.contains(1, "a"));
        ASSERT_FALSE(r.contains(1, "b"));
        ASSERT_FALSE(r.contains(2, "b"));
        ASSERT_FALSE(r.contains(3, "c"));
        ASSERT_FALSE(r.erase(2, "b"));
    }
}
