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

#include "test_utils.h"

#include "kdl/collection_utils.h"

#include <vector>

namespace kdl {
    TEST_CASE("collection_utils_test.col_total_size", "[collection_utils_test]") {
        using vec = std::vector<int>;

        ASSERT_EQ(0u, size(vec({})));
        ASSERT_EQ(1u, size(vec({ 2 })));
        ASSERT_EQ(2u, size(vec({ 2, 1 })));
        ASSERT_EQ(2u, col_total_size(vec({ 2 }), vec({ 2 })));
        ASSERT_EQ(3u, col_total_size(vec({ 2 }), vec({ 2, 1 })));
    }

    template <typename T>
    void test_range_remove_all(const std::vector<T> exp1, std::vector<T> col, const std::vector<T> rem) {
        auto it = range_remove_all(std::begin(col), std::end(col), std::begin(rem), std::end(rem));
        ASSERT_EQ(exp1, std::vector<T>(std::begin(col), it));
    }

    TEST_CASE("collection_utils_test.range_remove_all", "[collection_utils_test]") {
        test_range_remove_all<int>({ 1, 2, 3, 4, 5, 6, 7, 8, 9 }, { 1, 2, 3, 4, 5, 6, 7, 8, 9 }, {});
        test_range_remove_all<int>({ 1, 2, 4, 5, 6, 7, 8, 9 }, { 1, 2, 3, 4, 5, 6, 7, 8, 9 }, { 3 });
        test_range_remove_all<int>({ 1, 2, 5, 6, 8, 9 }, { 1, 2, 3, 4, 5, 6, 7, 8, 9 }, { 7, 3, 4 });
    }

    TEST_CASE("collection_utils_test.range_delete_all", "[collection_utils_test]") {
        bool d1 = false;
        bool d2 = false;
        bool d3 = false;
        auto d = std::vector<deletable*>({ new deletable(d1), new deletable(d2), new deletable(d3) });
        range_delete_all(std::begin(d), std::end(d));

        ASSERT_TRUE(d1);
        ASSERT_TRUE(d2);
        ASSERT_TRUE(d3);
    }

    template <typename T>
    void test_range_lexicographical_compare(const int exp, const std::vector<T>& lhs, const std::vector<T>& rhs) {
        ASSERT_EQ(exp, col_lexicographical_compare(lhs, rhs));
    }

    TEST_CASE("collection_utils_test.range_lexicographical_compare", "[collection_utils_test]") {
        test_range_lexicographical_compare<int>(0, {}, {});
        test_range_lexicographical_compare<int>(-1, {}, { 1 });
        test_range_lexicographical_compare<int>(0, { 1 }, { 1 });
        test_range_lexicographical_compare<int>(+1, { 1 }, {});
        test_range_lexicographical_compare<int>(-1, { 1 }, { 1, 2 });
        test_range_lexicographical_compare<int>(0, { 1, 2 }, { 1, 2 });
        test_range_lexicographical_compare<int>(+1, { 1, 2 }, { 1 });
        test_range_lexicographical_compare<int>(+1, { 1, 3 }, { 1, 2, 3 });
        test_range_lexicographical_compare<int>(+1, { 2 }, { 1, 2, 3 });
        test_range_lexicographical_compare<int>(-1, { 1, 2, 3 }, { 3 });
    }

    TEST_CASE("collection_utils_test.col_size", "[collection_utils_test]") {
        ASSERT_EQ(2, col_size<int>(std::vector<int>({ 1, 2 })));
        ASSERT_EQ(2u, col_size<unsigned>(std::vector<int>({ 1, 2 })));
    }

    TEST_CASE("collection_utils_test.col_delete_all", "[collection_utils_test]") {
        bool d1 = false;
        bool d2 = false;
        bool d3 = false;
        auto d = std::vector<deletable*>({ new deletable(d1), new deletable(d2), new deletable(d3) });
        col_delete_all(d);

        ASSERT_TRUE(d1);
        ASSERT_TRUE(d2);
        ASSERT_TRUE(d3);
    }

    template <typename T>
    void test_col_is_equivalent(const bool exp, const std::vector<T>& lhs, const std::vector<T>& rhs) {
        ASSERT_EQ(exp, col_is_equivalent(lhs, rhs));
    }

    TEST_CASE("collection_utils_test.col_is_equivalent", "[collection_utils_test]") {
        test_col_is_equivalent<int>(true, {}, {});
        test_col_is_equivalent<int>(false, {}, { 1 });
        test_col_is_equivalent<int>(true, { 1 }, { 1 });
        test_col_is_equivalent<int>(false, { 1 }, {});
        test_col_is_equivalent<int>(false, { 1 }, { 1, 2 });
        test_col_is_equivalent<int>(true, { 1, 2 }, { 1, 2 });
        test_col_is_equivalent<int>(true, { 3, 4, 1 }, { 3, 4, 1 });
        test_col_is_equivalent<int>(false, { 1, 2 }, { 1 });
        test_col_is_equivalent<int>(false, { 1, 3 }, { 1, 2, 3 });
        test_col_is_equivalent<int>(false, { 2 }, { 1, 2, 3 });
        test_col_is_equivalent<int>(false, { 1, 2, 3 }, { 3 });
    }


}
