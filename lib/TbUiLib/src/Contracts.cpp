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
#include "ui/Contracts.h"

#include "kd/contracts.h"

#ifndef NDEBUG
#include <cassert>

namespace tb::ui
{
namespace
{
CrashReporter* crashReporterForContractViolationHandler = nullptr;

// for debug builds, ensure is just an assertion
void contractViolated(
  const std::string_view /* file */,
  const int /* line */,
  const std::string_view /* type */,
  const std::string_view /* condition */)
{
  assert(0);
}

} // namespace
} // namespace tb::ui

#else
#include "ui/CrashReporter.h"

#include <fmt/format.h>

namespace tb::ui
{
namespace
{
CrashReporter* crashReporterForContractViolationHandler = nullptr;

// for release builds, ensure generates a crash report
void contractViolated(
  const std::string_view file,
  const int line,
  const std::string_view type,
  const std::string_view condition)
{
  const auto reason =
    fmt::format("{} line {}: {} '{}' failed", file, line, type, condition);
  crashReporterForContractViolationHandler->reportCrashAndExit(reason);
}

} // namespace
} // namespace tb::ui
#endif

namespace tb::ui
{

void setContractViolationHandler(CrashReporter& crashReporter)
{
  crashReporterForContractViolationHandler = &crashReporter;
  kd::set_contract_violation_handler(contractViolated);
}

} // namespace tb::ui