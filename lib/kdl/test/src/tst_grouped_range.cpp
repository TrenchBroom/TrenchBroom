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

#include "kdl/grouped_range.h"
#include "kdl/range_io.h" // IWYU pragma: keep

#include <vector>

#include <catch2/catch.hpp>

namespace kdl
{

namespace
{
template <typename T, typename D>
auto make_window(const std::vector<T>& v, const D offset, const D length)
{
  return range{std::next(v.begin(), offset), std::next(v.begin(), offset + length)};
}
} // namespace

TEST_CASE("grouped_range")
{
  SECTION("Empty range")
  {
    auto v = std::vector<int>{};
    auto r = make_grouped_range(v, [](const auto& lhs, const auto& rhs) {
      return (lhs < 2 && rhs < 2) || (lhs >= 2 && rhs >= 2 && lhs < 4 && rhs < 4)
             || (lhs >= 4 && rhs >= 4);
    });

    const auto expected = std::vector<decltype(make_window(v, 0, 0))>{};

    CHECK(r == range{expected.begin(), expected.end()});
  }

  SECTION("Range with one group")
  {
    auto v = std::vector<int>{0, 1};
    auto r = make_grouped_range(v, [](const auto& lhs, const auto& rhs) {
      return (lhs < 2 && rhs < 2) || (lhs >= 2 && rhs >= 2 && lhs < 4 && rhs < 4)
             || (lhs >= 4 && rhs >= 4);
    });

    const auto expected = std::vector{
      make_window(v, 0, 2),
    };

    CHECK(r == range{expected.begin(), expected.end()});
  }

  SECTION("Range with three groups")
  {
    auto v = std::vector<int>{0, 1, 2, 3, 4};
    auto r = make_grouped_range(v, [](const auto& lhs, const auto& rhs) {
      return (lhs < 2 && rhs < 2) || (lhs >= 2 && rhs >= 2 && lhs < 4 && rhs < 4)
             || (lhs >= 4 && rhs >= 4);
    });

    const auto expected = std::vector{
      make_window(v, 0, 2),
      make_window(v, 2, 2),
      make_window(v, 4, 1),
    };

    CHECK(r == range{expected.begin(), expected.end()});
  }
}

} // namespace kdl
