/*
 Copyright 2024 Kristian Duske

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

#include <functional>

namespace kdl
{

template <typename T>
auto combine_hash(const T hash)
{
  return hash;
}

template <typename T, typename Rest>
auto combine_hash(const T hash, const Rest& rest)
{
  return hash ^ combine_hash(rest) << 1;
}

template <typename A>
auto hash(const A& a)
{
  return std::hash<A>{}(a);
}

template <typename A, typename... Rest>
auto hash(const A& arg, const Rest&... rest)
{
  return combine_hash(hash(arg), hash(rest...));
}

} // namespace kdl
