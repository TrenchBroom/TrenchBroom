/*
 Copyright (C) 2024 Kristian Duske

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

#include "kd/range_utils.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace kdl
{

TEST_CASE("range_utils.index_of")
{
  using vec = std::vector<int>;

  CHECK(index_of(vec{}, 1) == std::nullopt);
  CHECK(index_of(vec{2}, 1) == std::nullopt);
  CHECK(index_of(vec{1}, 1) == 0u);
  CHECK(index_of(vec{1, 2, 3}, 1) == 0u);
  CHECK(index_of(vec{1, 2, 3}, 2) == 1u);
  CHECK(index_of(vec{1, 2, 3}, 3) == 2u);
  CHECK(index_of(vec{1, 2, 2}, 2) == 1u);
  CHECK(index_of(vec{1, 2, 3}, 4) == std::nullopt);

  CHECK(index_of(vec{}, [](const auto& i) { return i == 1; }) == std::nullopt);
  CHECK(index_of(vec{2}, [](const auto& i) { return i == 1; }) == std::nullopt);
  CHECK(index_of(vec{1}, [](const auto& i) { return i == 1; }) == 0u);
  CHECK(index_of(vec{1, 2, 3}, [](const auto& i) { return i == 1; }) == 0u);
  CHECK(index_of(vec{1, 2, 3}, [](const auto& i) { return i == 2; }) == 1u);
  CHECK(index_of(vec{1, 2, 3}, [](const auto& i) { return i == 3; }) == 2u);
  CHECK(index_of(vec{1, 2, 2}, [](const auto& i) { return i == 2; }) == 1u);
  CHECK(index_of(vec{1, 2, 3}, [](const auto& i) { return i == 4; }) == std::nullopt);
}

} // namespace kdl
