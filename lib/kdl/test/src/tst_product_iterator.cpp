/*
 Copyright 2023 Kristian Duske

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

#include "kdl/product_iterator.h"
#include "kdl/std_io.h"

#include <vector>

#include "catch2.h"

namespace kdl
{
TEST_CASE("product_iterator")
{
  using Catch::Matchers::UnorderedEquals;


  SECTION("with a single range")
  {
    using T = std::tuple<std::vector<int>, std::vector<std::tuple<int>>>;
    const auto& [range, expected] = GENERATE(values<T>({
      {{}, {}},
      {{1}, {{1}}},
      {{1, 2}, {{1}, {2}}},
    }));

    const auto r = make_product_range(range);
    const auto v = std::vector<std::tuple<int>>(r.begin(), r.end());
    CHECK_THAT(v, UnorderedEquals(expected));
  }

  SECTION("with two ranges")
  {
    using T =
      std::tuple<std::vector<int>, std::vector<char>, std::vector<std::tuple<int, char>>>;

    // clang-format off
    const auto& 
    [range1,    range2,     expected] = GENERATE(values<T>({
    {{},        {},         {}},
    {{},        {'a'},      {}},
    {{1},       {},         {}},
    {{1},       {'a'},      {{1, 'a'}}},
    {{1, 2, 3}, {'a', 'b'}, {{1, 'a'}, {1, 'b'}, 
                             {2, 'a'}, {2, 'b'}, 
                             {3, 'a'}, {3, 'b'}}},
    }));
    // clang-format on

    const auto r = make_product_range(range1, range2);
    const auto v = std::vector<std::tuple<int, char>>(r.begin(), r.end());
    CHECK_THAT(v, UnorderedEquals(expected));
  }

  SECTION("with three ranges")
  {
    enum class X
    {
      x,
      y
    };

    using T = std::tuple<
      std::vector<int>,
      std::vector<char>,
      std::vector<X>,
      std::vector<std::tuple<int, char, X>>>;

    // clang-format off
    const auto& 
    [range1, range2,     range3,       expected] = GENERATE(values<T>({
    {{},     {},         {},           {}},
    {{},     {'a'},      {},           {}},
    {{1, 2}, {'a', 'b'}, {X::x, X::y}, {{1, 'a', X::x}, {1, 'a', X::y},
                                        {1, 'b', X::x}, {1, 'b', X::y},
                                        {2, 'a', X::x}, {2, 'a', X::y},
                                        {2, 'b', X::x}, {2, 'b', X::y},}},
    }));
    // clang-format on

    const auto r = make_product_range(range1, range2, range3);
    const auto v = std::vector<std::tuple<int, char, X>>(r.begin(), r.end());
    CHECK_THAT(v, UnorderedEquals(expected));
  }
}
} // namespace kdl
