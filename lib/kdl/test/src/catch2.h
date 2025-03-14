/*
 Copyright (C) 2023 Kristian Duske

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

#include "kdl/range_io.h"  // IWYU pragma: export
#include "kdl/result_io.h" // IWYU pragma: export
#include "kdl/std_io.h"    // IWYU pragma: export

// The catch2 header must be included only when all stream insertion
// operators used in assertions are visible. We add this new wrapper header
// that includes these operators for the vm types to ensure that they
// work consistently.

// Include this header instead of <catch2/catch.hpp> to ensure that vm
// stream operators work consistently.

#define CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS 1
#include <catch2/catch.hpp> // IWYU pragma: export
