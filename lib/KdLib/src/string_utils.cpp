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

#include "kd/string_utils.h"

#if defined(__APPLE__)
#include "fast_float/fast_float.h"
#endif

#include "kd/reflection_impl.h"
#include "kd/string_format.h"

#include <algorithm>
#include <cassert>
#include <charconv>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace kdl
{
namespace
{

auto skip_whitespace(const std::string_view str)
{
  const auto first = str.find_first_not_of(Whitespace);
  return first != std::string::npos ? str.substr(first) : std::string_view{};
}

} // namespace

kdl_reflect_impl(delimited_string);

std::optional<delimited_string> str_find_next_delimited_string(
  const std::string_view str,
  const std::string_view start_delim,
  const std::string_view end_delim,
  const std::optional<char> escape_char)
{
  std::optional<std::size_t> start;
  std::size_t depth = 0;
  auto escaped = false;

  for (size_t i = 0; i < str.size(); ++i)
  {
    const auto c = str[i];
    if (c == escape_char && !escaped)
    {
      escaped = true;
    }
    else
    {
      if (str.substr(i, start_delim.size()) == start_delim && !escaped)
      {
        if (!start)
        {
          start = i;
        }
        else
        {
          ++depth;
        }
      }
      else if (start && str.substr(i, end_delim.size()) == end_delim && !escaped)
      {
        if (depth == 0)
        {
          return delimited_string{*start, i + end_delim.size() - *start};
        }
        else
        {
          --depth;
        }
      }
      escaped = false;
    }
  }

  if (start)
  {
    return delimited_string{*start, std::nullopt};
  }

  return std::nullopt;
}

std::optional<std::tuple<size_t, size_t>> str_next_token(
  const std::string_view str, const std::string_view delims)
{
  if (str.empty())
  {
    return std::nullopt;
  }

  if (delims.empty())
  {
    return std::tuple{0, str.length()};
  }

  // skip leading delimiters
  const auto start = str.find_first_not_of(delims);
  if (start == std::string_view::npos)
  {
    return std::nullopt;
  }

  for (auto i = start; i < str.size(); ++i)
  {
    const auto c = str[i];
    if (c == '\\' && i < str.size() - 1u)
    {
      // maybe escaped delimiter or backslash
      const auto n = str[i + 1];
      if (n == '\\' || delims.find(n) != std::string_view::npos)
      {
        ++i;
        continue;
      }
    }

    if (delims.find(c) != std::string_view::npos)
    {
      return std::tuple{start, i};
    }
  }

  return std::tuple{start, str.length()};
}

std::tuple<std::vector<std::string>, size_t> str_next_tokens(
  const std::string_view str, const std::string_view delims, const size_t max)
{
  if (max == 0)
  {
    return {std::vector<std::string>{}, 0};
  }

  auto cur = str;
  auto result = std::vector<std::string>{};
  result.reserve(max);

  size_t i = 0;
  size_t end = 0;
  while (auto position = str_next_token(cur, delims))
  {
    const auto [tokenStart, tokenEnd] = *position;
    const auto token = cur.substr(tokenStart, tokenEnd - tokenStart);
    result.emplace_back(token);
    cur = cur.substr(tokenEnd);
    end += tokenEnd;

    if (++i == max)
    {
      break;
    }
  }

  return {result, end};
}

std::vector<std::string> str_split(
  const std::string_view str, const std::string_view delims)
{
  auto cur = str;
  auto result = std::vector<std::string>{};
  while (auto position = str_next_token(cur, delims))
  {
    const auto [start, end] = *position;
    const auto token = cur.substr(start, end - start);
    result.emplace_back(token);
    cur = cur.substr(end);
  }
  return result;
}

std::string str_replace_every(
  const std::string_view haystack,
  const std::string_view needle,
  const std::string_view replacement)
{
  if (haystack.empty() || needle.empty() || needle == replacement)
  {
    return std::string(haystack);
  }

  std::ostringstream result;
  auto it = std::begin(haystack);
  auto end = std::search(
    std::begin(haystack), std::end(haystack), std::begin(needle), std::end(needle));
  while (end != std::end(haystack))
  {
    // copy everything up to needle
    std::copy(it, end, std::ostream_iterator<char>(result));

    // copy replacement
    result << replacement;

    // advance to just after needle
    it =
      std::next(end, static_cast<std::string::iterator::difference_type>(needle.size()));
    end = std::search(it, std::end(haystack), std::begin(needle), std::end(needle));
  }

  // copy the remainder
  std::copy(it, end, std::ostream_iterator<char>(result));
  return result.str();
}

std::optional<int> str_to_int(std::string_view str)
{
  str = skip_whitespace(str);
  int value;
  return std::from_chars(str.data(), str.data() + str.size(), value).ec == std::errc{}
           ? std::optional{value}
           : std::nullopt;
}

std::optional<long> str_to_long(std::string_view str)
{
  str = skip_whitespace(str);
  long value;
  return std::from_chars(str.data(), str.data() + str.size(), value).ec == std::errc{}
           ? std::optional{value}
           : std::nullopt;
}

std::optional<long long> str_to_long_long(std::string_view str)
{
  str = skip_whitespace(str);
  long long value;
  return std::from_chars(str.data(), str.data() + str.size(), value).ec == std::errc{}
           ? std::optional{value}
           : std::nullopt;
}

std::optional<unsigned long> str_to_u_long(std::string_view str)
{
  str = skip_whitespace(str);
  unsigned long value;
  return std::from_chars(str.data(), str.data() + str.size(), value).ec == std::errc{}
           ? std::optional{value}
           : std::nullopt;
}

std::optional<unsigned long long> str_to_u_long_long(std::string_view str)
{
  str = skip_whitespace(str);
  unsigned long long value;
  return std::from_chars(str.data(), str.data() + str.size(), value).ec == std::errc{}
           ? std::optional{value}
           : std::nullopt;
}

std::optional<std::size_t> str_to_size(std::string_view str)
{
  str = skip_whitespace(str);
  size_t value;
  return std::from_chars(str.data(), str.data() + str.size(), value).ec == std::errc{}
           ? std::optional{value}
           : std::nullopt;
}

std::optional<float> str_to_float(std::string_view str)
{
  str = skip_whitespace(str);

  float value;
#if defined(__APPLE__)
  return fast_float::from_chars(str.data(), str.data() + str.size(), value).ec
             == std::errc{}
           ? std::optional{value}
           : std::nullopt;
#else
  return std::from_chars(str.data(), str.data() + str.size(), value).ec == std::errc{}
           ? std::optional{value}
           : std::nullopt;
#endif
}

std::optional<double> str_to_double(std::string_view str)
{
  str = skip_whitespace(str);

  double value;
#if defined(__APPLE__)
  return fast_float::from_chars(str.data(), str.data() + str.size(), value).ec
             == std::errc{}
           ? std::optional{value}
           : std::nullopt;
#else
  return std::from_chars(str.data(), str.data() + str.size(), value).ec == std::errc{}
           ? std::optional{value}
           : std::nullopt;
#endif
}

} // namespace kdl
