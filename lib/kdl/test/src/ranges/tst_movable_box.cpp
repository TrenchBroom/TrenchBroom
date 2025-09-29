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

#include "kdl/ranges/detail/movable_box.h"

#include <string>

#include <catch2/catch_test_macros.hpp>

namespace kdl::ranges::detail
{

TEST_CASE("movable_box")
{
  const auto f = []() {};

  static_assert(std::is_move_constructible_v<movable_box<int>>);
  static_assert(std::is_move_constructible_v<movable_box<std::string>>);
  static_assert(std::is_move_constructible_v<movable_box<decltype(f)>>);

  static_assert(std::is_move_assignable_v<movable_box<int>>);
  static_assert(std::is_move_assignable_v<movable_box<std::string>>);
  static_assert(std::is_move_assignable_v<movable_box<decltype(f)>>);
}

} // namespace kdl::ranges::detail
