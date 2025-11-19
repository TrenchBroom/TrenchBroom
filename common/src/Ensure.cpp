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

#include "Ensure.h"

#ifndef NDEBUG
#include <cassert>

// for debug builds, ensure is just an assertion
void tb::ensureFailed(
  const char* /* file */,
  const int /* line */,
  const char* /* condition */,
  const char* /* message */)
{
  assert(0);
}
#else
#include "ui/CrashReporter.h"

#include <fmt/format.h>

// for release builds, ensure generates a crash report
void tb::ensureFailed(
  const char* file, const int line, const char* condition, const char* message)
{
  const auto reason =
    fmt::format("{} line {}: Condition '{}' failed ({})", file, line, condition, message);
  tb::ui::reportCrashAndExit(reason);
}
#endif
