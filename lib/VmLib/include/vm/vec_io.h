/*
 Copyright (C) 2010 Kristian Duske
 Copyright (C) 2015 Eric Wasylishen

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

#include "vm/from_chars.h"
#include "vm/vec.h"

#include <optional>
#include <ostream>
#include <string>
#include <string_view>

namespace vm
{
/**
 * Determines how strictly vm::parse accepts the given string.
 */
enum class parse_mode
{
  /**
   * Parentheses count as whitespace, a component may be followed by other characters, and
   * anything after the last component is ignored.
   */
  lenient,
  /**
   * The string must consist of nothing but the components separated by whitespace, with
   * optional leading and trailing whitespace.
   */
  strict,
};

namespace detail
{
template <typename T, std::size_t S>
std::optional<vec<T, S>> doParse(
  const std::string_view str, size_t& pos, const parse_mode mode)
{
  const auto* blank = mode == parse_mode::strict ? " \t\n\r" : " \t\n\r()";

  auto result = vec<T, S>{};
  for (std::size_t i = 0; i < S; ++i)
  {
    if ((pos = str.find_first_not_of(blank, pos)) == std::string::npos)
    {
      return std::nullopt;
    }
    const auto [ptr, ec] =
      from_chars(str.data() + pos, str.data() + str.length(), result[i]);
    if (ec != std::errc{})
    {
      return std::nullopt;
    }
    if ((pos = str.find_first_of(blank, pos)) == std::string::npos)
    {
      if (i < S - 1)
      {
        return std::nullopt;
      }
    }
    if (
      mode == parse_mode::strict
      && ptr != str.data() + (pos == std::string::npos ? str.length() : pos))
    {
      return std::nullopt;
    }
  }

  if (
    mode == parse_mode::strict && str.find_first_not_of(blank, pos) != std::string::npos)
  {
    return std::nullopt;
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
 * FLOAT ::= any floating point number parseable by std::from_chars
 *
 * In strict mode, parentheses don't count as whitespace, each FLOAT must be followed by
 * whitespace or the end of the string, and only whitespace may follow the last component.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param str the string to parse
 * @param mode how strictly to parse the string
 * @return the vector parsed from the string
 */
template <typename T, std::size_t S>
std::optional<vec<T, S>> parse(
  const std::string_view str, const parse_mode mode = parse_mode::lenient)
{
  std::size_t pos = 0;
  return detail::doParse<T, S>(str, pos, mode);
}

/**
 * Parses the given string for a list of vectors. The syntax of the given string is as
 * follows:
 *
 * LIST ::= VEC, { SEP, VEC }
 *  SEP ::= " " | \\t | \\n |t \\r | "," | ";";
 *
 * Note that the list can be separated by whitespace or commas or semicolons, or a mix of
 * these separators. Only vectors which conform to the vector syntax are added to the
 * given output iterator.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @tparam O the type of the output iterator
 * @param str the string to parse
 * @param out the output iterator add the parsed vectors to
 */
template <typename T, std::size_t S, typename O>
void parse_all(const std::string_view str, O out)
{
  constexpr auto blank = " \t\n\r,;";

  std::size_t pos = 0;
  while (pos != std::string::npos)
  {
    if (const auto result = detail::doParse<T, S>(str, pos, parse_mode::lenient))
    {
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
std::ostream& operator<<(std::ostream& stream, const vec<T, S>& vec)
{
  if constexpr (S > 0)
  {
    stream << vec[0];
    for (size_t i = 1; i < S; ++i)
    {
      stream << " " << vec[i];
    }
  }
  return stream;
}
} // namespace vm
