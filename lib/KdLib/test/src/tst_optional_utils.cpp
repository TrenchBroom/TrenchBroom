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

#include "kd/optional_utils.h"

#include <catch2/catch_test_macros.hpp>

namespace kdl
{

TEST_CASE("optional_and_then")
{
  const auto f = [](int x) { return std::optional{x * 2}; };
  CHECK((std::optional<int>{42} | optional_and_then(f)) == 84);
  CHECK((std::optional<int>{} | optional_and_then(f)) == std::nullopt);
}

TEST_CASE("optional_transform")
{
  const auto f = [](int x) { return x * 2; };
  CHECK((std::optional<int>{42} | optional_transform(f)) == 84);
  CHECK((std::optional<int>{} | optional_transform(f)) == std::nullopt);
}

TEST_CASE("optional_value_or")
{
  CHECK((std::optional<int>{42} | optional_value_or(43)) == 42);
  CHECK((std::optional<int>{} | optional_value_or(43)) == 43);
}

} // namespace kdl
