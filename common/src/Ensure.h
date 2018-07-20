/*
Copyright (C) 2010-2017 Kristian Duske

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

#ifndef TrenchBroom_Ensure_h
#define TrenchBroom_Ensure_h

#include <string>

namespace TrenchBroom {
    void ensureFailed(const char *file, int line, const char *condition, const std::string& message);
}

// These are ugly but necessary to stringify an expression, see: https://en.wikipedia.org/wiki/C_preprocessor#Token_stringification
#define stringification(expression) #expression
#define stringification2(expression) stringification(expression)

#ifndef NDEBUG
    // for debug builds, ensure is just an assertion
    #define ensure(condition, message) assert(condition)
#else
    // for release builds, ensure generates an exception
    #define ensure(condition, message) do { if (!(condition)) { TrenchBroom::ensureFailed(__FILE__, __LINE__, stringification2(condition), message); } } while (false)
#endif

#endif /* defined(TrenchBroom_Ensure) */
