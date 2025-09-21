/*
 Copyright 2023 Kristian Duske

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

#pragma once

#include "kdl/range.h"
#include "kdl/std_io.h"

#include <ostream>

namespace kdl
{

#if !defined(__clang__) && defined(__GNUC__)
// MSVC and GCC issue a warning about infinite recursion with a range of grouped iterators
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4717)
#endif

template <typename I>
std::ostream& operator<<(std::ostream& lhs, const range<I>& rhs)
{
  return lhs << make_streamable(rhs);
}

#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

} // namespace kdl
