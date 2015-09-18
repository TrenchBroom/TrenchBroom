/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_Macros_h
#define TrenchBroom_Macros_h

// This macro is used to silence compiler warnings about unused variables. These are usually only used in assertions
// and thus may become unused in release builds.
#define unused(x) ((void)x)

// The following macro is used to silence a compiler warning in MSVC and GCC when a switch is used in a function to compute
// a return value, and there is no default path.
#ifdef __clang__
#define switchDefault()
#else
#define switchDefault() default: assert(false); throw "Unhandled switch case";
#endif

#define assertResult(funexp) if (!(funexp)) assert(false);

#endif
