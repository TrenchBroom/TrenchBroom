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

namespace kdl
{

/**
 * Tag type used to indicate that a sequence of values passed to a flat_set or flat_map
 * constructor or modifying member function is already sorted with respect to the
 * container's comparator and does not contain any two equivalent values.
 *
 * Passing a sequence that does not satisfy this precondition together with this tag
 * results in undefined behavior.
 */
struct sorted_unique_t
{
  explicit sorted_unique_t() = default;
};

/**
 * An instance of sorted_unique_t for convenient use as a function argument.
 */
inline constexpr sorted_unique_t sorted_unique{};

} // namespace kdl
