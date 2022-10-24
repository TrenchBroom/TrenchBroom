/*
 Copyright 2010-2019 Kristian Duske
 Copyright 2015-2019 Eric Wasylishen

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

#include "mat.h"

#include <optional>
#include <ostream>

namespace vm
{
/**
 * Parses the given string representation. The syntax of the given string is as follows
 *
 *   MAT ::= R * C * COMP;
 *     R ::= number of rows
 *     C ::= number of columns
 *  COMP ::= WS, FLOAT;
 *    WS ::= " " | \\t | \\n | \\r | "(" | ")";
 * FLOAT ::= any floating point number parseable by std::atof
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param str the string to parse
 * @return the matrix parsed from the string
 */
template <typename T, std::size_t R, std::size_t C>
std::optional<mat<T, R, C>> parse(const std::string_view str)
{
  constexpr auto blank = " \t\n\r()";

  auto result = mat<T, R, C>{};
  std::size_t pos = 0u;
  for (std::size_t r = 0u; r < R; ++r)
  {
    for (std::size_t c = 0u; c < C; ++c)
    {
      if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos)
      {
        return std::nullopt;
      }
      result[c][r] = static_cast<T>(std::atof(str.data() + pos));
      if ((pos = str.find_first_of(blank, pos)) == std::string::npos)
      {
        if ((r * C) + c < R * C - 1u)
        {
          return std::nullopt;
        }
      }
    }
  }
  return result;
}

/**
 * Prints a textual representation of the given matrix on the given stream.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param stream the stream to print to
 * @param mat the matrix to print
 * @return the given stream
 */
template <typename T, size_t R, size_t C>
std::ostream& operator<<(std::ostream& stream, const mat<T, R, C>& mat)
{
  for (size_t r = 0u; r < R; ++r)
  {
    for (size_t c = 0u; c < C; ++c)
    {
      stream << mat[c][r];
      if (c < C - 1u)
      {
        stream << " ";
      }
    }
    if (r < R - 1u)
    {
      stream << " ";
    }
  }
  return stream;
}
} // namespace vm
