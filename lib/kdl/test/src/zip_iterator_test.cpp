/*
 Copyright 2020 Kristian Duske

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

#include "kdl/tuple_io.h"
#include "kdl/zip_iterator.h"

#include <vector>

#include <catch2/catch.hpp>

namespace kdl {
    TEST_CASE("zip_iterator_test.construct", "[zip_iterator_test]") {
        using Catch::Matchers::Equals;

        const auto v1 = std::vector<int>{1, 2, 3};
        const auto v2 = std::vector<int>{4, 5, 6};

        auto cur = zip_iterator(std::begin(v1), std::begin(v2));
        auto end = zip_iterator(std::end(v1), std::end(v2));

        const auto vz = std::vector<std::tuple<int, int>>(cur, end);
        CHECK_THAT(vz, Equals(std::vector<std::tuple<int, int>>{{1, 4}, {2, 5}, {3, 6}}));
    }

    TEST_CASE("zip_iterator_test.make_zip_range", "[zip_iterator_test]") {
        using Catch::Matchers::Equals;

        const auto v1 = std::vector<int>{1, 2, 3};
        const auto v2 = std::vector<int>{4, 5, 6};
        const auto v3 = std::vector<int>{7, 8, 9};

        auto r = make_zip_range(v1, v2, v3);

        const auto vz = std::vector<std::tuple<int, int, int>>(std::begin(r), std::end(r));
        CHECK_THAT(vz, Equals(std::vector<std::tuple<int, int, int>>{{1, 4, 7}, {2, 5, 8}, {3, 6, 9}}));
    }
}
