/*
 Copyright 2022 Kristian Duske

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

#include "kdl/string_utils.h"
#include "kdl/struct_io.h"

#include <optional>
#include <sstream>
#include <tuple>
#include <vector>

#include <catch2/catch.hpp>

namespace kdl
{

template <typename... Args>
std::string build_string(const Args&... args)
{
  auto str = std::stringstream{};
  (struct_stream{str} << ... << args);
  return str.str();
}

TEST_CASE("streamable_struct")
{
  CHECK(build_string("type") == "type{}");
  CHECK(build_string("type", "a", "x") == "type{a: x}");
  CHECK(build_string("type", "a", "x", "b", "y") == "type{a: x, b: y}");

  CHECK(build_string("type", "a", std::vector<int>{1, 2, 3}) == "type{a: [1,2,3]}");
  CHECK(build_string("type", "a", std::optional<int>{1}) == "type{a: 1}");

  CHECK(
    build_string("type", "a", std::tuple<int, std::string>{1, "asdf"})
    == "type{a: {1, asdf}}");

  CHECK(
    build_string(
      "type",
      "a",
      std::tuple<std::vector<int>, std::optional<std::string>>{{1, 2}, "asdf"})
    == "type{a: {[1,2], asdf}}");

  CHECK(
    build_string("type", "a", std::optional<std::tuple<int, std::string>>{{1, "asdf"}})
    == "type{a: {1, asdf}}");

  CHECK(
    build_string("type", "a", std::vector<std::optional<int>>{{1, std::nullopt, 2}})
    == "type{a: [1,nullopt,2]}");
}
} // namespace kdl
