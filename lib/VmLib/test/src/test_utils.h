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

#define CE_CHECK(...)                                                                    \
  {                                                                                      \
    constexpr auto _r_r = (__VA_ARGS__);                                                 \
    CHECK(_r_r);                                                                         \
  }
#define CER_CHECK(...)                                                                   \
  CHECK((__VA_ARGS__));                                                                  \
  CE_CHECK(__VA_ARGS__);

#define CE_CHECK_FALSE(...)                                                              \
  {                                                                                      \
    constexpr auto _r_r = (__VA_ARGS__);                                                 \
    CHECK_FALSE(_r_r);                                                                   \
  }
#define CER_CHECK_FALSE(...)                                                             \
  CHECK_FALSE((__VA_ARGS__));                                                            \
  CE_CHECK_FALSE(__VA_ARGS__);
