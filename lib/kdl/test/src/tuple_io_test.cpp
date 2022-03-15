/*
 Copyright 2022 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "kdl/tuple_io.h"
#include <kdl/string_utils.h>

#include <sstream>
#include <tuple>

#include <catch2/catch.hpp>

namespace kdl {

TEST_CASE("tuple IO") {
  CHECK(str_to_string(make_streamable(std::tuple<int>{0})) == "{0}");
  CHECK(str_to_string(make_streamable(std::tuple<int, std::string>{0, "asdf"})) == "{0, asdf}");
}

} // namespace kdl

namespace sibling {

TEST_CASE("tuple IO - ADL from sibling namespace") {
  auto str = std::stringstream{};
  str << kdl::make_streamable(std::tuple<int>{0});
}

} // namespace sibling
