/*
 Copyright 2024 Kristian Duske

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

#include "kd/stable_remove_duplicates.h"

#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace kdl
{

TEST_CASE("col_stable_remove_duplicates")
{
  using T = std::tuple<std::vector<int>, std::vector<int>>;

  const auto [input, expectedOutput] = GENERATE(values<T>({
    {{}, {}},
    {{1}, {1}},
    {{1, 2}, {1, 2}},
    {{2, 1}, {2, 1}},
    {{1, 2, 2, 3}, {1, 2, 3}},
    {{1, 2, 3, 2}, {1, 2, 3}},
    {{1, 2, 3, 3, 2, 1}, {1, 2, 3}},
    {{3, 2, 1, 1, 2, 3}, {3, 2, 1}},
  }));

  CAPTURE(input);

  CHECK(col_stable_remove_duplicates(input) == expectedOutput);
}

} // namespace kdl
