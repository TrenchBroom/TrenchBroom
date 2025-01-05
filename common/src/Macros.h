/*
 Copyright (C) 2010 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cassert>

// This macro is used to silence compiler warnings about unused variables. These are
// usually only used in assertions and thus may become unused in release builds.
#define unused(x) ((void)x)

// The following macro is used to silence a compiler warning in MSVC and GCC when a switch
// is used in a function to compute a return value, and there is no default path.
#ifdef __clang__
#define switchDefault()                                                                  \
  do                                                                                     \
  {                                                                                      \
  } while (0)
#else
#define switchDefault()                                                                  \
  default:                                                                               \
    assert(false);                                                                       \
    throw "Unhandled switch case"
#endif

// Annotate an intended switch fallthrough
#define switchFallthrough() [[fallthrough]]

#define assertResult(funexp)                                                             \
  do                                                                                     \
  {                                                                                      \
    const bool result_ = (funexp);                                                       \
    unused(result_);                                                                     \
    assert(result_);                                                                     \
  } while (0)

#define defineCopy(classname)                                                            \
public:                                                                                  \
  classname(const classname& other) = default;                                           \
  classname& operator=(const classname& other) = default
#define defineMove(classname)                                                            \
public:                                                                                  \
  classname(classname&& other) noexcept = default;                                       \
  classname& operator=(classname&& other) = default
#define defineCopyAndMove(classname)                                                     \
public:                                                                                  \
  classname(const classname& other) = default;                                           \
  classname(classname&& other) noexcept = default;                                       \
  classname& operator=(const classname& other) = default;                                \
  classname& operator=(classname&& other) = default

#define deleteCopy(classname)                                                            \
public:                                                                                  \
  classname(const classname& other) = delete;                                            \
  classname& operator=(const classname& other) = delete
#define deleteMove(classname)                                                            \
public:                                                                                  \
  classname(classname&& other) = delete;                                                 \
  classname& operator=(classname&& other) = delete
#define deleteCopyAndMove(classname)                                                     \
public:                                                                                  \
  classname(const classname& other) = delete;                                            \
  classname(classname&& other) noexcept = delete;                                        \
  classname& operator=(const classname& other) = delete;                                 \
  classname& operator=(classname&& other) = delete

#define moveOnly(classname)                                                              \
  defineMove(classname);                                                                 \
  deleteCopy(classname)
