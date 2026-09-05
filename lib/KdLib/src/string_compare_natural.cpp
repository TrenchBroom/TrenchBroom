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

#include "kd/string_format.h"

#include <strnatcmp.h>

#include <string>
#include <string_view>

namespace kdl::ci
{

int str_compare_natural(const std::string_view s1, const std::string_view s2)
{
  // strnatcmp needs null terminated strings, so we must copy anyway. We fold to lower
  // case here instead of using strnatcasecmp, which folds to upper case and would sort
  // '_' after the letters, unlike ci::char_less.
  return strnatcmp(str_to_lower(s1).c_str(), str_to_lower(s2).c_str());
}

bool string_less_natural::operator()(
  const std::string_view lhs, const std::string_view rhs) const
{
  // break ties by exact string to get a total order
  const auto compareResult = str_compare_natural(lhs, rhs);
  return compareResult != 0 ? compareResult < 0 : lhs < rhs;
}

} // namespace kdl::ci
