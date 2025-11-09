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

#pragma once

#include "kdl/reflection_decl.h"

#include <cassert>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace kdl
{
struct delimited_string
{
  std::size_t start;
  std::optional<std::size_t> length;

  kdl_reflect_decl(delimited_string, start, length);
};

/**
 * Find the next occurrence of a delimited substring in the given string. The substring is
 * delimited by the given start and end delimiters, which can be escaped using the given
 * escape character.
 *
 * Nested delimeted strings are skipped, that is, only the outermost delimited string is
 * returned.
 *
 * If an unterminated delimited string is found, the function returns a delimited_string
 * with its length set to std::nullopt.
 *
 * @param str the string to search
 * @param start_delim the start delimiter
 * @param end_delim the end delimiter
 * @param escape_char the escape character
 * @return the position of the start delimiter and the length of the delimited string
 * (incl. the delimiters), or nullopt if no delimited string can be found
 */
std::optional<delimited_string> str_find_next_delimited_string(
  std::string_view str,
  std::string_view start_delim,
  std::string_view end_delim,
  std::optional<char> escape_char = std::nullopt);

/**
 * Returns the next token from the given string. Leading delimiters are skipped and the
 * part up until and not including the next delimiter is included.
 *
 * Delimiters can be escaped with a backslash ('\'). Backslashes can be escaped with
 * backslashes too. Escaped delimiters are not unescaped.
 *
 * Examples:
 *   str_next_token("", " ") == std::nullopt
 *   str_next_token("  ", " ") == std::nullopt
 *   str_next_token("asdf", " ") == {0, 4}
 *   str_next_token(" asdf ", " ") == {1, 5}
 *   str_next_token("asdf;qwer", ";") == {0, 4}
 *   str_next_token("as\;df", ";") == {0, 6}
 *
 * @param str the string to extract a token from
 * @param delims the token delimiters
 * @return the start and end indices of the token, or nullopt if no token could be
 * extracted
 */
std::optional<std::tuple<size_t, size_t>> str_next_token(
  std::string_view str, std::string_view delims);

/**
 * Splits the given strings along the given delimiters and returns a list of the nonempty
 * parts.
 *
 * Delimiters can be escaped with a backslash ('\'). Backslashes can be escaped with
 * backslashes too. Escaped delimiters are not unescaped.
 *
 * @param str the string to split
 * @param delims the delimiters to split with
 * @return the parts
 */
std::vector<std::string> str_split(std::string_view str, std::string_view delims);

/**
 * Joins the objects in the given range [it, end) using the given delimiters. The
 * objects are converted to string using the stream insertion operator and a string
 * stream.
 *
 * Given an object o, let "o" be the string representation of o obtained using the
 * stream insertion operator <<.
 *
 * If the given range is [], an empty string is returned.
 * If the given range is [o1], the result "o1".
 * If the given range is [o1, o2], then the result is "o1" + delim_for_two + "o2".
 * If the given range is [o1, o2, ..., on] with n > 2, the result is "o1" + delim + "o2"
 * + delim +
 * ... + delim + "on-1" + last_delim + "on".
 *
 * @tparam I the range iterator type
 * @param it the beginning of the range
 * @param end the end of the range
 * @param delim the delimiter to insert for ranges of length > 2
 * @param last_delim the delimter to insert for ranges of length 2
 * @param delim_for_two the delimiter to insert before the last object in ranges of
 * length > 2
 * @return the joined string
 */
template <typename I>
std::string str_join(
  I it,
  I end,
  const std::string_view delim,
  const std::string_view last_delim,
  const std::string_view delim_for_two)
{
  if (it == end)
  {
    return "";
  }

  auto result = std::stringstream{};
  result << *it++;

  if (it == end)
  {
    return result.str();
  }

  auto prev = it++;
  if (it == end)
  {
    result << delim_for_two << *prev;
    return result.str();
  }
  result << delim << *prev;

  prev = it++;
  while (it != end)
  {
    result << delim << *prev;
    prev = it++;
  }

  result << last_delim << *prev;
  return result.str();
}

/**
 * Joins the objects in the given range [it, end) using the given delimiter. The delimiter
 * is used as the delimiter for collections of two objects as well as for the last two
 * objects in collections of more than two objects.
 *
 * @see str_join(I, I, const std::string_view, const std::string_view, const
 * std::string_view)
 *
 * @tparam I the range iterator type
 * @param it the beginning of the range
 * @param end the end of the range
 * @param delim the delimiter to insert
 * @return the joined string
 */
template <typename I>
std::string str_join(I it, I end, const std::string_view delim)
{
  return str_join(it, end, delim, delim, delim);
}

/**
 * Joins the objects in the given collection using the given delimiters.
 *
 * @see str_join(I, I, const std::string_view, const std::string_view, const
 * std::string_view)
 *
 * @tparam C the collection type
 * @param c the collection of objects to join
 * @param delim the delimiter to insert for collections of size > 2
 * @param last_delim the delimter to insert for collections of size 2
 * @param delim_for_two the delimiter to insert before the last object in collections of
 * size > 2
 * @return the joined string
 */
template <typename C>
std::string str_join(
  const C& c,
  const std::string_view delim,
  const std::string_view last_delim,
  const std::string_view delim_for_two)
{
  return str_join(std::begin(c), std::end(c), delim, last_delim, delim_for_two);
}

/**
 * Joins the objects in the given collection using the given delimiter. The delimiter is
 * used as the delimiter for collections of two objects as well as for the last two
 * objects in collections of more than two objects.
 *
 * @see str_join(I, I, const std::string_view, const std::string_view, const
 * std::string_view)
 *
 * @tparam C the collection type
 * @param c the collection of objects to join
 * @param delim the delimiter to insert
 * @return the joined string
 */
template <typename C>
std::string str_join(const C& c, const std::string_view delim = ", ")
{
  return str_join(std::begin(c), std::end(c), delim, delim, delim);
}

/**
 * Replaces every occurence of needle in string haystack with the given replacement, and
 * returns the result.
 *
 * @param haystack the string to modify
 * @param needle the string to search for
 * @param replacement the string to replace needle with
 * @return the modified string
 */
std::string str_replace_every(
  std::string_view haystack, std::string_view needle, std::string_view replacement);

/**
 * Returns a concatenation of the string representations of the given objects by means of
 * the stream insertion operator.
 *
 * @tparam Args the type of the objects
 * @param args the objects
 * @return the concatenated string representations
 */
template <typename... Args>
std::string str_to_string(Args&&... args)
{
  std::stringstream str;
  (str << ... << args);
  return str.str();
}

/**
 * Interprets the given string as a signed integer and returns it. If the given string
 * cannot be parsed, returns an empty optional.
 *
 * @param str the string
 * @return the signed integer value or an empty optional if the given string cannot be
 * interpreted as a signed integer
 */
std::optional<int> str_to_int(std::string_view str);

/**
 * Interprets the given string as a signed long integer and returns it. If the given
 * string cannot be parsed, returns an empty optional.
 *
 * @param str the string
 * @return the signed long integer value or an empty optional if the given string cannot
 * be interpreted as a signed long integer
 */
std::optional<long> str_to_long(std::string_view str);

/**
 * Interprets the given string as a signed long long integer and returns it. If the given
 * string cannot be parsed, returns an empty optional.
 *
 * @param str the string
 * @return the signed long long integer value or an empty optional if the given string
 * cannot be interpreted as a signed long long integer
 */
std::optional<long long> str_to_long_long(std::string_view str);

/**
 * Interprets the given string as an unsigned long integer and returns it. If the given
 * string cannot be parsed, returns an empty optional.
 *
 * @param str the string
 * @return the unsigned long integer value or an empty optional if the given string cannot
 * be interpreted as an unsigned long integer
 */
std::optional<unsigned long> str_to_u_long(std::string_view str);

/**
 * Interprets the given string as an unsigned long long integer and returns it. If the
 * given string cannot be parsed, returns an empty optional.
 *
 * @param str the string
 * @return the unsigned long long integer value or an empty optional if the given string
 * cannot be interpreted as an unsigned long long integer
 */
std::optional<unsigned long long> str_to_u_long_long(std::string_view str);

/**
 * Interprets the given string as a std::size_t and returns it. If the given string cannot
 * be parsed, returns an empty optional.
 *
 * @param str the string
 * @return the std::size_t value or an empty optional if the given string cannot be
 * interpreted as an std::size_t
 */
std::optional<std::size_t> str_to_size(std::string_view str);

/**
 * Interprets the given string as a 32 bit floating point value and returns it. If the
 * given string cannot be parsed, returns an empty optional.
 *
 * @param str the string
 * @return the 32 bit floating point value value or an empty optional if the given string
 * cannot be interpreted as an 32 bit floating point value
 */
std::optional<float> str_to_float(std::string_view str);

/**
 * Interprets the given string as a 64 bit floating point value and returns it. If the
 * given string cannot be parsed, returns an empty optional.
 *
 * @param str the string
 * @return the 64 bit floating point value value or an empty optional if the given string
 * cannot be interpreted as an 64 bit floating point value
 */
std::optional<double> str_to_double(std::string_view str);

} // namespace kdl
