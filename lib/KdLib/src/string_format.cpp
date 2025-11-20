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

#include "kd/string_format.h"

#include "kd/contracts.h"

#include <algorithm>
#include <sstream>

namespace kdl
{

std::string str_select(
  const bool predicate, const std::string_view positive, const std::string_view negative)
{
  return std::string{predicate ? positive : negative};
}

std::string str_plural(
  const std::size_t count, const std::string_view singular, const std::string_view plural)
{
  return str_select(count == 1u, singular, plural);
}

/**
 * Returns either of the given strings depending on the given count. The returned string
 * is prefixed with the given prefix and suffixed with the given suffix.
 *
 * @tparam C the type of the count parameter
 * @param prefix the prefix
 * @param count the count which determines the string to return
 * @param singular the string to return if count == T(1)
 * @param plural the string to return otherwise
 * @param suffix the suffix
 * @return one of the given strings depending on the given count, with the given prefix
 * and suffix
 */
std::string str_plural(
  const std::string_view prefix,
  const std::size_t count,
  const std::string_view singular,
  const std::string_view plural,
  const std::string_view suffix)
{
  auto result = std::stringstream{};
  result << prefix << str_plural(count, singular, plural) << suffix;
  return result.str();
}

std::string str_trim(const std::string_view s, const std::string_view chars)
{
  if (s.empty() || chars.empty())
  {
    return std::string{s};
  }

  const auto first = s.find_first_not_of(chars);
  if (first == std::string::npos)
  {
    return "";
  }

  const auto last = s.find_last_not_of(chars);
  if (first > last)
  {
    return "";
  }

  return std::string{s.substr(first, last - first + 1)};
}

char str_to_lower(const char c)
{
  return (c < 'A' || c > 'Z') ? c : static_cast<char>(c + 'a' - 'A');
}

char str_to_upper(const char c)
{
  return (c < 'a' || c > 'z') ? c : static_cast<char>(c - 'a' + 'A');
}

std::string str_to_lower(const std::string_view str)
{
  auto result = std::string{};
  result.reserve(str.size());

  std::ranges::transform(
    str, std::back_inserter(result), [](const auto c) { return str_to_lower(c); });

  return result;
}

std::string str_to_upper(const std::string_view str)
{
  auto result = std::string{};
  result.reserve(str.size());

  std::ranges::transform(
    str, std::back_inserter(result), [](const auto c) { return str_to_upper(c); });

  return result;
}

std::string str_capitalize(const std::string_view str, const std::string_view delims)
{
  auto result = std::string{};
  result.reserve(str.size());

  auto initial = true;
  for (const auto c : str)
  {
    if (delims.find(c) != std::string::npos)
    {
      initial = true;
      result.push_back(c);
    }
    else if (initial)
    {
      result.push_back(str_to_upper(c));
      initial = false;
    }
    else
    {
      result.push_back(c);
      initial = false;
    }
  }

  return result;
}

std::string str_escape(
  const std::string_view str, const std::string_view chars, const char esc)
{
  if (str.empty())
  {
    return "";
  }

  auto buffer = std::stringstream{};
  for (const auto c : str)
  {
    if (c == esc || chars.find_first_of(c) != std::string::npos)
    {
      buffer << esc;
    }
    buffer << c;
  }
  return buffer.str();
}

std::string str_escape_if_necessary(
  const std::string_view str, const std::string_view chars, const char esc)
{
  contract_pre(chars.find(esc) == std::string::npos);

  if (str.empty())
  {
    return "";
  }

  auto buffer = std::stringstream{};
  auto escaped = false;
  for (const auto c : str)
  {
    const auto needsEscaping = (chars.find(c) != std::string::npos);
    if (needsEscaping)
    {
      // if 'c' is not prefixed by 'esc', insert an 'esc'
      if (!escaped)
      {
        buffer << esc;
      }
    }

    if (c == esc)
    {
      escaped = !escaped;
    }
    else
    {
      escaped = false;
    }
    buffer << c;
  }
  return buffer.str();
}

std::string str_unescape(
  const std::string_view str, const std::string_view chars, const char esc)
{
  if (str.empty())
  {
    return "";
  }

  auto buffer = std::stringstream{};
  auto escaped = false;
  for (const auto c : str)
  {
    if (c == esc)
    {
      if (escaped)
      {
        buffer << c;
      }
      escaped = !escaped;
    }
    else
    {
      if (escaped && chars.find_first_of(c) == std::string::npos)
      {
        buffer << '\\';
      }
      buffer << c;
      escaped = false;
    }
  }

  if (escaped)
  {
    buffer << '\\';
  }

  return buffer.str();
}

bool str_is_blank(const std::string_view str, const std::string_view whitespace)
{
  return str.find_first_not_of(whitespace) == std::string::npos;
}

bool str_is_numeric(const std::string_view str)
{
  for (const auto c : str)
  {
    if (c < '0' || c > '9')
    {
      return false;
    }
  }
  return true;
}

} // namespace kdl
