/*
 Copyright (C) 2025 Kristian Duske

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

#include "kd/ranges/detail/tuple_common_reference.h" // IWYU pragma: keep

#include <catch2/catch_test_macros.hpp>

namespace kdl::ranges
{

TEST_CASE("tuple_common_reference")
{
  static_assert(std::same_as<
                int&,
                std::common_reference_t<
                  std::add_lvalue_reference_t<int>,
                  std::add_lvalue_reference_t<int>&,
                  std::add_lvalue_reference_t<int>&&>>);

  static_assert(std::same_as<
                std::tuple<int&>,
                std::common_reference_t<
                  std::tuple<std::add_lvalue_reference_t<int>>,
                  std::tuple<std::add_lvalue_reference_t<int>&>,
                  std::tuple<std::add_lvalue_reference_t<int>&&>>>);

  static_assert(
    std::same_as<
      std::tuple<const long&, int&>,
      std::common_reference_t<std::tuple<long, int&>&&, std::tuple<long, int>&>>);
}

} // namespace kdl::ranges
