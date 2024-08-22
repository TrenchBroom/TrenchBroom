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

#include "kdl/pair_iterator.h"
#include "kdl/std_io.h"

#include <vector>

#include "catch2.h"

namespace kdl
{
TEST_CASE("pair_iterator")
{
  using Catch::Matchers::UnorderedEquals;

  using T = std::tuple<std::vector<int>, std::vector<std::tuple<int, int>>>;
  const auto& [range, expected] = GENERATE(values<T>({
    {{}, {}},
    {{1}, {}},
    {{1, 2}, {{1, 2}}},
    {{1, 2, 3}, {{1, 2}, {1, 3}, {2, 3}}},
  }));

  CAPTURE(range);

  const auto r = make_pair_range(range);
  const auto v = std::vector<std::tuple<int, int>>(r.begin(), r.end());
  CHECK_THAT(v, UnorderedEquals(expected));
}
} // namespace kdl
