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

#include <algorithm>
#include <ranges>

namespace kdl
{

template <std::ranges::input_range R1, std::ranges::input_range R2>
auto recursive_ranges_equal(R1&& r1, R2&& r2)
{
  return std::ranges::equal(r1, r2, [](auto&& e1, auto&& e2) {
    if constexpr (std::ranges::range<decltype(e1)> && std::ranges::range<decltype(e2)>)
    {
      return recursive_ranges_equal(e1, e2);
    }
    else
    {
      return std::ranges::equal_to{}(e1, e2);
    }
  });
}

} // namespace kdl
