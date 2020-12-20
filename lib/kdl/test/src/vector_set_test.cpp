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

#include "kdl/vector_set.h"

#include "GTestCompat.h"

#include <catch2/catch.hpp>

namespace kdl {
    using vec = std::vector<int>;
    using vset = vector_set<int>;

    static vset create_vset_from_range(const vec& v) {
        return vset(std::begin(v), std::end(v));
    }

    static vset create_vset_from_range(const vset::size_type capacity, const vec& v) {
        return vset(capacity, std::begin(v), std::end(v));
    }

    static vset create_vset_from_list(std::initializer_list<int> l) {
        return vset(std::move(l));
    }

    static vset create_vset_from_list(const vset::size_type capacity, std::initializer_list<int> l) {
        return vset(capacity, std::move(l));
    }

    static vset create_vset_from_vector(std::initializer_list<int> l) {
        return vset(std::vector<int>(l));
    }

    static void ASSERT_VSET_EQ(const vec& expected, const vset& actual) {
        ASSERT_EQ(create_vset_from_range(expected), actual);
    }

    static void ASSERT_VSET_EQ(const vset::size_type capacity, const vec& expected, const vset& actual) {
        ASSERT_EQ(capacity, actual.capacity());
        ASSERT_EQ(create_vset_from_range(expected), actual);
    }

    TEST_CASE("vector_set_test.constructor_default", "[vector_set_test]") {
        vset s;
        ASSERT_TRUE(s.empty());
        ASSERT_EQ(0u, s.size());
    }

    TEST_CASE("vector_set_test.constructor_default_with_capacity", "[vector_set_test]") {
        vset s(7u);
        ASSERT_TRUE(s.empty());
        ASSERT_EQ(0u, s.size());
        ASSERT_EQ(7u, s.capacity());
    }

    TEST_CASE("vector_set_test.constructor_with_range", "[vector_set_test]") {
        ASSERT_VSET_EQ({},          create_vset_from_range({}));
        ASSERT_VSET_EQ({ 1 },       create_vset_from_range({ 1 }));
        ASSERT_VSET_EQ({ 1 },       create_vset_from_range({ 1, 1 }));
        ASSERT_VSET_EQ({ 1, 2 },    create_vset_from_range({ 1, 2 }));
        ASSERT_VSET_EQ({ 1, 2 },    create_vset_from_range({ 2, 1 }));
        ASSERT_VSET_EQ({ 1, 2, 3 }, create_vset_from_range({ 2, 1, 3, 1, 2 }));
    }

    TEST_CASE("vector_set_test.constructor_with_range_and_capacity", "[vector_set_test]") {
        ASSERT_VSET_EQ(10u, {},          create_vset_from_range(10u, {}));
        ASSERT_VSET_EQ(10u, { 1 },       create_vset_from_range(10u, { 1 }));
        ASSERT_VSET_EQ(10u, { 1 },       create_vset_from_range(10u, { 1, 1 }));
        ASSERT_VSET_EQ(10u, { 1, 2 },    create_vset_from_range(10u, { 1, 2 }));
        ASSERT_VSET_EQ(10u, { 1, 2 },    create_vset_from_range(10u, { 2, 1 }));
        ASSERT_VSET_EQ(10u, { 1, 2, 3 }, create_vset_from_range(10u, { 2, 1, 3, 1, 2 }));
    }

    TEST_CASE("vector_set_test.constructor_with_initializer_list", "[vector_set_test]") {
        ASSERT_VSET_EQ({},          create_vset_from_list({}));
        ASSERT_VSET_EQ({ 1 },       create_vset_from_list({ 1 }));
        ASSERT_VSET_EQ({ 1 },       create_vset_from_list({ 1, 1 }));
        ASSERT_VSET_EQ({ 1, 2 },    create_vset_from_list({ 1, 2 }));
        ASSERT_VSET_EQ({ 1, 2 },    create_vset_from_list({ 2, 1 }));
        ASSERT_VSET_EQ({ 1, 2, 3 }, create_vset_from_list({ 2, 1, 3, 1, 2 }));
    }

    TEST_CASE("vector_set_test.constructor_with_initializer_list_and_capacity", "[vector_set_test]") {
        ASSERT_VSET_EQ(10u, {},          create_vset_from_list(10u, {}));
        ASSERT_VSET_EQ(10u, { 1 },       create_vset_from_list(10u, { 1 }));
        ASSERT_VSET_EQ(10u, { 1 },       create_vset_from_list(10u, { 1, 1 }));
        ASSERT_VSET_EQ(10u, { 1, 2 },    create_vset_from_list(10u, { 1, 2 }));
        ASSERT_VSET_EQ(10u, { 1, 2 },    create_vset_from_list(10u, { 2, 1 }));
        ASSERT_VSET_EQ(10u, { 1, 2, 3 }, create_vset_from_list(10u, { 2, 1, 3, 1, 2 }));
    }

    TEST_CASE("vector_set_test.constructor_with_vector", "[vector_set_test]") {
        ASSERT_VSET_EQ({},          create_vset_from_vector({}));
        ASSERT_VSET_EQ({ 1 },       create_vset_from_vector({ 1 }));
        ASSERT_VSET_EQ({ 1 },       create_vset_from_vector({ 1, 1 }));
        ASSERT_VSET_EQ({ 1, 2 },    create_vset_from_vector({ 1, 2 }));
        ASSERT_VSET_EQ({ 1, 2 },    create_vset_from_vector({ 2, 1 }));
        ASSERT_VSET_EQ({ 1, 2, 3 }, create_vset_from_vector({ 2, 1, 3, 1, 2 }));
    }

    TEST_CASE("vector_set_test.assignment_from_initializer_list", "[vector_set_test]") {
        ASSERT_VSET_EQ({},          vset() = {});
        ASSERT_VSET_EQ({ 1 },       vset() = { 1 });
        ASSERT_VSET_EQ({ 1 },       vset() = { 1, 1 });
        ASSERT_VSET_EQ({ 1, 2 },    vset() = { 1, 2 });
        ASSERT_VSET_EQ({ 1, 2 },    vset() = { 2, 1 });
        ASSERT_VSET_EQ({ 1, 2, 3 }, vset() = { 2, 1, 3, 1, 2 });

        ASSERT_VSET_EQ({},          vset({ 7, 8, 9 }) = {});
        ASSERT_VSET_EQ({ 1 },       vset({ 7, 8, 9 }) = { 1 });
        ASSERT_VSET_EQ({ 1 },       vset({ 7, 8, 9 }) = { 1, 1 });
        ASSERT_VSET_EQ({ 1, 2 },    vset({ 7, 8, 9 }) = { 1, 2 });
        ASSERT_VSET_EQ({ 1, 2 },    vset({ 7, 8, 9 }) = { 2, 1 });
        ASSERT_VSET_EQ({ 1, 2, 3 }, vset({ 7, 8, 9 }) = { 2, 1, 3, 1, 2 });
    }

    TEST_CASE("vector_set_test.assignment_from_vector", "[vector_set_test]") {
        ASSERT_VSET_EQ({},          vset() = std::vector<int>({}));
        ASSERT_VSET_EQ({ 1 },       vset() = std::vector<int>({ 1 }));
        ASSERT_VSET_EQ({ 1 },       vset() = std::vector<int>({ 1, 1 }));
        ASSERT_VSET_EQ({ 1, 2 },    vset() = std::vector<int>({ 1, 2 }));
        ASSERT_VSET_EQ({ 1, 2 },    vset() = std::vector<int>({ 2, 1 }));
        ASSERT_VSET_EQ({ 1, 2, 3 }, vset() = std::vector<int>({ 2, 1, 3, 1, 2 }));

        ASSERT_VSET_EQ({},          vset({ 7, 8, 9 }) = std::vector<int>({}));
        ASSERT_VSET_EQ({ 1 },       vset({ 7, 8, 9 }) = std::vector<int>({ 1 }));
        ASSERT_VSET_EQ({ 1 },       vset({ 7, 8, 9 }) = std::vector<int>({ 1, 1 }));
        ASSERT_VSET_EQ({ 1, 2 },    vset({ 7, 8, 9 }) = std::vector<int>({ 1, 2 }));
        ASSERT_VSET_EQ({ 1, 2 },    vset({ 7, 8, 9 }) = std::vector<int>({ 2, 1 }));
        ASSERT_VSET_EQ({ 1, 2, 3 }, vset({ 7, 8, 9 }) = std::vector<int>({ 2, 1, 3, 1, 2 }));
    }

    TEST_CASE("vector_set_test.deduction_guide_range", "[vector_set_test]") {
        std::vector<int> v({ 1, 2, 3 });
        vector_set s(std::begin(v), std::end(v));
    }

    TEST_CASE("vector_set_test.deduction_guide_range_and_capacity", "[vector_set_test]") {
        std::vector<int> v({ 1, 2, 3 });
        vector_set s(3u, std::begin(v), std::end(v));
    }
}
