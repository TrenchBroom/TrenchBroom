/*
 Copyright (C) 2026 Kristian Duske

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
 * Contains functions for comparing strings in natural order case sensitively.
 */
namespace cs
{

/**
 * Compares the given strings in natural order: runs of digits compare by numeric value,
 * ignoring any leading zeros, so "tex9" sorts before "tex10", and "tex1" sorts before
 * "tex02". All other characters, including whitespace, compare with case sensitivity.
 *
 * If two digit runs have the same numeric value only because of different zero padding,
 * the padding breaks the tie by comparing the digits verbatim, so e.g. "tex01" sorts
 * before "tex1". Together with case sensitivity, this means two different strings never
 * compare equal, so this function is already a total order; string_less_natural is
 * provided for consistency with ci::string_less_natural and as a convenient sort
 * predicate.
 *
 * @param s1 the first string
 * @param s2 the second string
 * @return -1 if s1 is less than s2, +1 if s1 is greater than s2, 0 if they are equal
 */
int str_compare_natural(std::string_view s1, std::string_view s2);

/**
 * Orders strings by str_compare_natural and breaks ties by exact value, so different
 * strings never compare equal. This is safe to use as a sort predicate.
 */
struct string_less_natural
{
  bool operator()(std::string_view lhs, std::string_view rhs) const;
};

} // namespace cs

/**
 * Contains functions for comparing strings in natural order case insensitively.
 */
namespace ci
{

/**
 * Compares the given strings in natural order: runs of digits compare by numeric value,
 * ignoring any leading zeros, so "tex9" sorts before "tex10", and "tex1" sorts before
 * "tex02". All other characters, including whitespace, compare without case sensitivity.
 *
 * If two digit runs have the same numeric value only because of different zero padding,
 * the padding breaks the tie by comparing the digits verbatim, so e.g. "tex01" sorts
 * before "tex1". Strings that differ only in case still compare equal, though. Use
 * string_less_natural for a total order.
 *
 * @param s1 the first string
 * @param s2 the second string
 * @return -1 if s1 is less than s2, +1 if s1 is greater than s2, 0 if they are equal
 */
int str_compare_natural(std::string_view s1, std::string_view s2);

/**
 * Orders strings by str_compare_natural and breaks ties by exact value, so different
 * strings never compare equal. This is safe to use as a sort predicate.
 */
struct string_less_natural
{
  bool operator()(std::string_view lhs, std::string_view rhs) const;
};

} // namespace ci
} // namespace kdl
