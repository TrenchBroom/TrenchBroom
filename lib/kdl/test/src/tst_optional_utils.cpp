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

#include "kdl/optional_utils.h"

#include "catch2.h"

namespace kdl
{

TEST_CASE("optional_and_then")
{
  const auto f = [](int x) { return std::optional{x * 2}; };
  CHECK(optional_and_then(std::optional<int>{42}, f) == 84);
  CHECK(optional_and_then(std::optional<int>{}, f) == std::nullopt);
}

TEST_CASE("optional_transform")
{
  const auto f = [](int x) { return x * 2; };
  CHECK(optional_transform(std::optional<int>{42}, f) == 84);
  CHECK(optional_transform(std::optional<int>{}, f) == std::nullopt);
}

} // namespace kdl
