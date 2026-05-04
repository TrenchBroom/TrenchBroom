
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

#include "vm/segment.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <ostream>

namespace vm
{

/**
 * Prints a textual representation of the given segment to the given output stream.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param stream the output stream
 * @param segment the segment to print
 * @return the given output stream
 */
template <typename T, std::size_t S>
std::ostream& operator<<(std::ostream& stream, const segment<T, S>& segment)
{
  return stream << "segment{start: " << segment.start() << ", end: " << segment.end()
                << "}";
}

} // namespace vm
