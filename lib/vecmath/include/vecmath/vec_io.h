/*
 Copyright 2010-2019 Kristian Duske
 Copyright 2015-2019 Eric Wasylishen

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

#pragma once

#include "vec.h"

#include <optional>
#include <ostream>
#include <string>
#include <string_view>

namespace vm {
namespace detail {
template <typename T, std::size_t S>
std::optional<vec<T, S>> doParse(const std::string_view str, size_t& pos) {
  constexpr auto blank = " \t\n\r()";

  auto result = vec<T, S>{};
  for (std::size_t i = 0; i < S; ++i) {
    if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos) {
      return std::nullopt;
    }
    result[i] = static_cast<T>(std::atof(str.data() + pos));
    if ((pos = str.find_first_of(blank, pos)) == std::string::npos) {
      if (i < S - 1) {
        return std::nullopt;
      }
    }
  }
  return result;
}
} // namespace detail

/**
 * Parses the given string representation. The syntax of the given string is as follows
 *
 *   VEC ::= S * COMP;
 *     S ::= number of components
 *  COMP ::= WS, FLOAT;
 *    WS ::= " " | \\t | \\n | \\r | "(" | ")";
 * FLOAT ::= any floating point number parseable by std::atof
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param str the string to parse
 * @return the vector parsed from the string
 */
template <typename T, std::size_t S> std::optional<vec<T, S>> parse(const std::string_view str) {
  std::size_t pos = 0;
  return detail::doParse<T, S>(str, pos);
}

/**
 * Parses the given string for a list of vectors. The syntax of the given string is as follows:
 *
 * LIST ::= VEC, { SEP, VEC }
 *  SEP ::= " " | \\t | \\n |t \\r | "," | ";";
 *
 * Note that the list can be separated by whitespace or commas or semicolons, or a mix of these
 * separators. Only vectors which conform to the vector syntax are added to the given output
 * iterator.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @tparam O the type of the output iterator
 * @param str the string to parse
 * @param out the output iterator add the parsed vectors to
 */
template <typename T, std::size_t S, typename O> void parse_all(const std::string_view str, O out) {
  constexpr auto blank = " \t\n\r,;";

  std::size_t pos = 0;
  while (pos != std::string::npos) {
    if (const auto result = detail::doParse<T, S>(str, pos)) {
      out = *result;
      ++out;
    }
    pos = str.find_first_of(blank, pos);
    pos = str.find_first_not_of(blank, pos);
  }
}

/**
 * Prints a textual representation of the given vector to the given output stream.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param stream the output stream
 * @param vec the vector to print
 * @return the given output stream
 */
template <typename T, std::size_t S>
std::ostream& operator<<(std::ostream& stream, const vec<T, S>& vec) {
  if constexpr (S > 0) {
    stream << vec[0];
    for (size_t i = 1; i < S; ++i) {
      stream << " " << vec[i];
    }
  }
  return stream;
}
} // namespace vm
