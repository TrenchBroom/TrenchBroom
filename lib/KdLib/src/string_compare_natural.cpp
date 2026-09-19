/*
 Copyright (C) 2010 Kristian Duske

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

#include "kd/string_compare_natural.h"

#include "kd/string_compare.h"
#include "kd/string_compare_detail.h"

#include <string_view>

namespace kdl
{
namespace cs
{

int str_compare_natural(const std::string_view s1, const std::string_view s2)
{
  return kdl::str_compare_natural(s1, s2, char_less());
}

bool string_less_natural::operator()(
  const std::string_view lhs, const std::string_view rhs) const
{
  // break ties by exact string to get a total order
  const auto compareResult = str_compare_natural(lhs, rhs);
  return compareResult != 0 ? compareResult < 0 : lhs < rhs;
}

} // namespace cs

namespace ci
{

int str_compare_natural(const std::string_view s1, const std::string_view s2)
{
  return kdl::str_compare_natural(s1, s2, char_less());
}

bool string_less_natural::operator()(
  const std::string_view lhs, const std::string_view rhs) const
{
  // break ties by exact string to get a total order
  const auto compareResult = str_compare_natural(lhs, rhs);
  return compareResult != 0 ? compareResult < 0 : lhs < rhs;
}

} // namespace ci
} // namespace kdl
