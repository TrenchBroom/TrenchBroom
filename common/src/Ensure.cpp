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

#include "Ensure.h"

#include "TrenchBroomStackWalker.h"
#include "TrenchBroomApp.h"

#include <sstream>
#include <cassert>

#ifndef NDEBUG
// for debug builds, ensure is just an assertion
void TrenchBroom::ensureFailed(const char* /* file */, const int /* line */, const char* /* condition */, const char* /* message */) {
    assert(0);
}
#else
// for release builds, ensure generates a crash report
void TrenchBroom::ensureFailed(const char* file, const int line, const char* condition, const char* message) {
    std::stringstream reason;
    reason << file << ":" << line << ": Condition '" << condition << "' failed: " << message;

    const std::string stacktrace = TrenchBroomStackWalker::getStackTrace();
    TrenchBroom::View::reportCrashAndExit(stacktrace, reason.str());
}
#endif
