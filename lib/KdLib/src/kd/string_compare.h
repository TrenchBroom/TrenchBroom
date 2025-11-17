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

#include <string_view>

namespace kdl
{
/**
 * Contains functions for working with strings case sensitively.
 */
namespace cs
{

struct char_less
{
  bool operator()(const char& lhs, const char& rhs) const;
};

struct char_equal
{
  bool operator()(const char& lhs, const char& rhs) const;
};

struct string_less
{
  bool operator()(std::string_view lhs, std::string_view rhs) const;
};

struct string_equal
{
  bool operator()(std::string_view lhs, std::string_view rhs) const;
};

/**
 * Returns the first position at which the given strings differ. Characters are compared
 * with case sensitivity.
 *
 * If the given strings are identical, then the length of the strings is returned.
 *
 * @param s1 the first string
 * @param s2 the second string
 * @return the first position at which the given strings differ
 */
std::size_t str_mismatch(std::string_view s1, std::string_view s2);

/**
 * Checks whether the first string contains the second string. Characters are compared
 * with case sensitivity.
 *
 * @param haystack the string to search in
 * @param needle the string to search for
 * @return true if the first string contains the second string and false otherwise
 */
bool str_contains(std::string_view haystack, std::string_view needle);

/**
 * Checks whether the second string is a prefix of the first string. Characters are
 * compared with case sensitivity.
 *
 * @param haystack the string to search in
 * @param needle the string to search for
 * @return true if needle is a prefix of haystack
 */
bool str_is_prefix(std::string_view haystack, std::string_view needle);

/**
 * Checks whether the second string is a suffix of the first string. Characters are
 * compared with case sensitivity.
 *
 * @param haystack the string to search in
 * @param needle the string to search for
 * @return true if needle is a suffix of haystack
 */
bool str_is_suffix(std::string_view haystack, std::string_view needle);

/**
 * Performs lexicographical comparison of the given strings s1 and s2. Returns -1 if the
 * first string is less than the second string, or +1 in the opposite case, or 0 if both
 * strings are equal. Characters are compared with case sensitivity.
 *
 * @param s1 the first string
 * @param s2 the second string
 * @return an int indicating the result of the comparison
 */
int str_compare(std::string_view s1, std::string_view s2);

/**
 * Checks whether the given strings are equal. Characters are compared with case
 * sensitivity.
 *
 * @param s1 the first string
 * @param s2 the second string
 * @return true if the given strings are equal and false otherwise
 */
bool str_is_equal(std::string_view s1, std::string_view s2);

/**
 * Checks whether the given string in range [s_cur, s_end) matches the glob pattern in
 * range [p_cur, p_end). Characters are compared with case sensitivity.
 *
 * @see kdl::matches_glob(I, I, I, I, CharEqual)
 *
 * @param s the string to match against
 * @param p the patterm
 * @return true if the given pattern matches the given string
 */
bool str_matches_glob(std::string_view s, std::string_view p);

} // namespace cs

/**
 * Contains functions for working with strings case insensitively.
 */
namespace ci
{

struct char_less
{
  bool operator()(const char& lhs, const char& rhs) const;
};

struct char_equal
{
  bool operator()(const char& lhs, const char& rhs) const;
};

struct string_less
{
  bool operator()(std::string_view lhs, std::string_view rhs) const;
};

struct string_equal
{
  bool operator()(std::string_view lhs, std::string_view rhs) const;
};

/**
 * Returns the first position at which the given strings differ. Characters are compared
 * without case sensitivity.
 *
 * If the given strings are identical, then the length of the strings is returned.
 *
 * @param s1 the first string
 * @param s2 the second string
 * @return the first position at which the given strings differ
 */
std::size_t str_mismatch(std::string_view s1, std::string_view s2);

/**
 * Checks whether the first string contains the second string. Characters are compared
 * without case sensitivity.
 *
 * @param haystack the string to search in
 * @param needle the string to search for
 * @return true if the first string contains the second string and false otherwise
 */
bool str_contains(std::string_view haystack, std::string_view needle);

/**
 * Checks whether the second string is a prefix of the first string. Characters are
 * compared without case sensitivity.
 *
 * @param haystack the string to search in
 * @param needle the string to search for
 * @return true if needle is a prefix of haystack
 */
bool str_is_prefix(std::string_view haystack, std::string_view needle);

/**
 * Checks whether the second string is a suffix of the first string. Characters are
 * compared without case sensitivity.
 *
 * @param haystack the string to search in
 * @param needle the string to search for
 * @return true if needle is a suffix of haystack
 */
bool str_is_suffix(std::string_view haystack, std::string_view needle);

/**
 * Performs lexicographical comparison of the given strings s1 and s2. Returns -1 if the
 * first string is less than the second string, or +1 in the opposite case, or 0 if both
 * strings are equal. Characters are compared without case sensitivity.
 *
 * @param s1 the first string
 * @param s2 the second string
 * @return an int indicating the result of the comparison
 */
int str_compare(std::string_view s1, std::string_view s2);

/**
 * Checks whether the given strings are equal. Characters are compared without case
 * sensitivity.
 *
 * @param s1 the first string
 * @param s2 the second string
 * @return true if the given strings are equal and false otherwise
 */
bool str_is_equal(std::string_view s1, std::string_view s2);

/**
 * Checks whether the given string in range [s_cur, s_end) matches the glob pattern in
 * range [p_cur, p_end). Characters are compared without case sensitivity.
 *
 * @see kdl::matches_glob(I, I, I, I, CharEqual)
 *
 * @param s the string to match against
 * @param p the patterm
 * @return true if the given pattern matches the given string
 */
bool str_matches_glob(std::string_view s, std::string_view p);

} // namespace ci
} // namespace kdl
