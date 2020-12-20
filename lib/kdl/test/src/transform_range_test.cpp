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

#include <kdl/transform_range.h>

#include <vector>

#include "GTestCompat.h"

#include <catch2/catch.hpp>

namespace kdl {
    TEST_CASE("transform_iterator_test.operator_less_than", "[transform_iterator_test]") {
        const auto v1 = std::vector<int>({});
        const auto t1 = transform_adapter(v1, [](const auto& i) { return i + 2; });
        ASSERT_FALSE(std::begin(t1) < std::end(t1));

        const auto v2 = std::vector<int>({ 1 });
        const auto t2 = transform_adapter(v2, [](const auto& i) { return i + 2; });

        auto it = std::begin(t2);
        auto end = std::end(t2);
        ASSERT_TRUE(it < end);

        ++it;
        ASSERT_FALSE(it < end);
    }

    TEST_CASE("transform_iterator_test.operator_greater_than", "[transform_iterator_test]") {
        const auto v1 = std::vector<int>({});
        const auto t1 = transform_adapter(v1, [](const auto& i) { return i + 2; });
        ASSERT_FALSE(std::end(t1) > std::begin(t1));

        const auto v2 = std::vector<int>({ 1 });
        const auto t2 = transform_adapter(v2, [](const auto& i) { return i + 2; });

        auto it = std::begin(t2);
        auto end = std::end(t2);
        ASSERT_TRUE(end > it);

        ++it;
        ASSERT_FALSE(end > it);
    }

    TEST_CASE("transform_iterator_test.operator_equal", "[transform_iterator_test]") {
        const auto v1 = std::vector<int>({});
        const auto t1 = transform_adapter(v1, [](const auto& i) { return i + 2; });
        ASSERT_TRUE(std::begin(t1) == std::end(t1));

        const auto v2 = std::vector<int>({ 1 });
        const auto t2 = transform_adapter(v2, [](const auto& i) { return i + 2; });

        auto it = std::begin(t2);
        auto end = std::end(t2);
        ASSERT_FALSE(it == end);

        ++it;
        ASSERT_TRUE(it == end);
    }

    TEST_CASE("transform_iterator_test.operator_not_equal", "[transform_iterator_test]") {
        const auto v1 = std::vector<int>({});
        const auto t1 = transform_adapter(v1, [](const auto& i) { return i + 2; });
        ASSERT_FALSE(std::begin(t1) != std::end(t1));

        const auto v2 = std::vector<int>({ 1 });
        const auto t2 = transform_adapter(v2, [](const auto& i) { return i + 2; });

        auto it = std::begin(t2);
        auto end = std::end(t2);
        ASSERT_TRUE(it != end);

        ++it;
        ASSERT_FALSE(it != end);
    }

    TEST_CASE("transform_iterator_test.operator_prefix_increment", "[transform_iterator_test]") {
        const auto v = std::vector<int>({ 1 });
        const auto t = transform_adapter(v, [](const auto& i) { return i + 2; });
        ASSERT_EQ(std::end(t), ++std::begin(t));
    }

    TEST_CASE("transform_iterator_test.operator_prefix_decrement", "[transform_iterator_test]") {
        const auto v = std::vector<int>({ 1 });
        const auto t = transform_adapter(v, [](const auto& i) { return i + 2; });
        ASSERT_EQ(std::begin(t), --std::end(t));
    }

    TEST_CASE("transform_iterator_test.operator_postfix_increment", "[transform_iterator_test]") {
        const auto v = std::vector<int>({ 1 });
        const auto t = transform_adapter(v, [](const auto& i) { return i + 2; });

        auto it = std::begin(t);
        ASSERT_EQ(std::begin(t), it++);
        ASSERT_EQ(std::end(t), it);
    }

    TEST_CASE("transform_iterator_test.operator_postfix_decrement", "[transform_iterator_test]") {
        const auto v = std::vector<int>({ 1 });
        const auto t = transform_adapter(v, [](const auto& i) { return i + 2; });

        auto it = std::end(t);
        ASSERT_EQ(std::end(t), it--);
        ASSERT_EQ(std::begin(t), it);
    }

    TEST_CASE("transform_iterator_test.operator_star", "[transform_iterator_test]") {
        const auto v = std::vector<int>({ 1 });
        const auto t = transform_adapter(v, [](const auto& i) { return i + 2; });
        ASSERT_EQ(3, *std::begin(t));
    }

    TEST_CASE("transform_adapter_test.empty", "[transform_adapter_test]") {
        const auto v1 = std::vector<int>({});
        const auto t1 = transform_adapter(v1, [](const auto& i) { return i + 2; });
        ASSERT_TRUE(t1.empty());

        const auto v2 = std::vector<int>({ 1, 2, 3 });
        const auto t2 = transform_adapter(v2, [](const auto& i) { return i + 2; });
        ASSERT_FALSE(t2.empty());
    }

    TEST_CASE("transform_adapter_test.size", "[transform_adapter_test]") {
        const auto v1 = std::vector<int>({});
        const auto t1 = transform_adapter(v1, [](const auto& i) { return i + 2; });
        ASSERT_EQ(0u, t1.size());

        const auto v2 = std::vector<int>({ 1, 2, 3 });
        const auto t2 = transform_adapter(v2, [](const auto& i) { return i + 2; });
        ASSERT_EQ(3u, t2.size());
    }

    TEST_CASE("transform_adapter_test.iterators", "[transform_adapter_test]") {
        const auto v1 = std::vector<int>({});
        const auto t1 = transform_adapter(v1, [](const auto& i) { return i + 2; });
        ASSERT_EQ(std::begin(t1), std::end(t1));

        const auto v2 = std::vector<int>({ 1, 2, 3 });
        const auto t2 = transform_adapter(v2, [](const auto& i) { return i + 2; });

        auto it = std::begin(t2);
        auto end = std::end(t2);
        ASSERT_NE(it, end);
        ASSERT_EQ(3, *it);

        ++it;
        ASSERT_NE(it, end);
        ASSERT_EQ(4, *it);

        ++it;
        ASSERT_NE(it, end);
        ASSERT_EQ(5, *it);

        ++it;
        ASSERT_EQ(it, end);
    }

    TEST_CASE("transform_adapter_test.reverse_iterators", "[transform_adapter_test]") {
        const auto v1 = std::vector<int>({});
        const auto t1 = transform_adapter(v1, [](const auto& i) { return i + 2; });
        ASSERT_EQ(std::rbegin(t1), std::rend(t1));

        const auto v2 = std::vector<int>({ 1, 2, 3 });
        const auto t2 = transform_adapter(v2, [](const auto& i) { return i + 2; });

        auto it = std::rbegin(t2);
        auto end = std::rend(t2);
        ASSERT_NE(it, end);
        ASSERT_EQ(5, *it);

        ++it;
        ASSERT_NE(it, end);
        ASSERT_EQ(4, *it);

        ++it;
        ASSERT_NE(it, end);
        ASSERT_EQ(3, *it);

        ++it;
        ASSERT_EQ(it, end);
    }
}
