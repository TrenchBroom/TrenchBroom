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

#include "kdl/skip_iterator.h"

#include <vector>

namespace kdl {
    TEST_CASE("skip_iterator_test.prefix_increment", "[skip_iterator_test]") {
        std::vector<int> vec({ 1, 2, 3, 4, 5 });
        auto it = skip_iterator(std::begin(vec), std::end(vec), 1, 2);

        ASSERT_EQ(it, std::next(std::begin(vec), 1));
        ASSERT_EQ(++it, std::next(std::begin(vec), 3));
        ASSERT_EQ(++it, std::end(vec));
    }

    TEST_CASE("skip_iterator_test.postfix_increment", "[skip_iterator_test]") {
        std::vector<int> vec({ 1, 2, 3, 4, 5 });
        auto it = skip_iterator(std::begin(vec), std::end(vec), 1, 2);

        ASSERT_EQ(it++, std::next(std::begin(vec), 1));
        ASSERT_EQ(it++, std::next(std::begin(vec), 3));
        ASSERT_EQ(it, std::end(vec));
    }

    TEST_CASE("skip_iterator_test.empty_sequence", "[skip_iterator_test]") {
        std::vector<size_t> vec;

        ASSERT_EQ(std::begin(vec), skip_iterator(std::begin(vec), std::end(vec)));
        ASSERT_EQ(std::end(vec), skip_iterator(std::begin(vec), std::end(vec)));
        ASSERT_EQ(std::begin(vec), skip_iterator(std::begin(vec), std::end(vec), 1));
        ASSERT_EQ(std::end(vec), skip_iterator(std::begin(vec), std::end(vec), 1));
    }

    TEST_CASE("skip_iterator_test.zero_stride", "[skip_iterator_test]") {
        std::vector<size_t> vec({ 1 });

        ASSERT_EQ(std::begin(vec), skip_iterator(std::begin(vec), std::end(vec), 0, 0));
        ASSERT_EQ(std::begin(vec), std::next(skip_iterator(std::begin(vec), std::end(vec), 0, 0)));
    }

    TEST_CASE("skip_iterator_test.oneElement_sequence", "[skip_iterator_test]") {
        std::vector<size_t> vec({ 1 });

        ASSERT_EQ(std::begin(vec), skip_iterator(std::begin(vec), std::end(vec)));
        ASSERT_EQ(std::end(vec), skip_iterator(std::begin(vec), std::end(vec), 1));
        ASSERT_EQ(std::end(vec), skip_iterator(std::begin(vec), std::end(vec), 2));

        ASSERT_EQ(std::begin(vec), skip_iterator(std::begin(vec), std::end(vec), 0, 2));
        ASSERT_EQ(std::end(vec), skip_iterator(std::begin(vec), std::end(vec), 1, 2));
        ASSERT_EQ(std::end(vec), skip_iterator(std::begin(vec), std::end(vec), 2, 2));

        ASSERT_EQ(std::end(vec), std::next(skip_iterator(std::begin(vec), std::end(vec), 0)));
        ASSERT_EQ(std::end(vec), std::next(skip_iterator(std::begin(vec), std::end(vec), 1)));
        ASSERT_EQ(std::end(vec), std::next(skip_iterator(std::begin(vec), std::end(vec), 2)));

        ASSERT_EQ(std::end(vec), std::next(skip_iterator(std::begin(vec), std::end(vec), 0, 2)));
        ASSERT_EQ(std::end(vec), std::next(skip_iterator(std::begin(vec), std::end(vec), 1, 2)));
        ASSERT_EQ(std::end(vec), std::next(skip_iterator(std::begin(vec), std::end(vec), 2, 2)));
    }
}
