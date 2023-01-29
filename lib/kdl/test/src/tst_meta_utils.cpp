/*
 Copyright 2010-2019 Kristian Duske

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

#include "kdl/meta_utils.h"

#include <catch2/catch_all.hpp>

namespace kdl
{
TEST_CASE("meta_utils_test.contains")
{
  static_assert(meta_contains_v<int, int>);
  static_assert(!meta_contains_v<int, float>);
  static_assert(meta_contains_v<int, float, int>);
  static_assert(!meta_contains_v<int, float, double>);
}

TEST_CASE("meta_utils_test.append")
{
  static_assert(std::is_same_v<
                meta_append<int, float, double>::result,
                meta_type_list<float, double, int>>);
  static_assert(std::is_same_v<
                meta_append<int, int, float, double>::result,
                meta_type_list<int, float, double, int>>);
}

TEST_CASE("meta_utils_test.append_if")
{
  static_assert(std::is_same_v<
                meta_append_if<true, int, float, double>::result,
                meta_type_list<float, double, int>>);
  static_assert(std::is_same_v<
                meta_append_if<false, int, float, double>::result,
                meta_type_list<float, double>>);
}

TEST_CASE("meta_utils_test.front")
{
  static_assert(std::is_same_v<meta_front<int>::front, int>);
  static_assert(std::is_same_v<meta_front<int>::remainder, meta_type_list<>>);
  static_assert(std::is_same_v<meta_front<int, float, double>::front, int>);
  static_assert(std::is_same_v<
                meta_front<int, float, double>::remainder,
                meta_type_list<float, double>>);
}

TEST_CASE("meta_utils_test.remove_duplicates")
{
  static_assert(std::is_same_v<
                meta_remove_duplicates<int, float, double>::result,
                meta_type_list<int, float, double>>);
  static_assert(std::is_same_v<
                meta_remove_duplicates<int, float, int, double, bool, float>::result,
                meta_type_list<int, float, double, bool>>);
}
} // namespace kdl
