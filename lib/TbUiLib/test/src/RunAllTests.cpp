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

#define CATCH_CONFIG_RUNNER

#include <QApplication>

#include "PreferenceManager.h"
#include "TestPreferenceStore.h"

#include "kd/contracts.h"
#include "kd/k.h"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

namespace
{
// for release builds, ensure generates a crash report
void contractViolated(
  const std::string_view file,
  const int line,
  const std::string_view type,
  const std::string_view condition)
{
  const auto reason =
    fmt::format("{} line {}: {} '{}' failed", file, line, type, condition);
  FAIL(reason);
}

} // namespace

int main(int argc, char** argv)
{
  kd::set_contract_violation_handler(contractViolated);

  // Needs to be set for the QSettings instance used by RecentDocuments
  QApplication::setApplicationName("TrenchBroom");
  QApplication::setOrganizationName("");
  QApplication::setOrganizationDomain("io.github.trenchbroom");

  tb::PreferenceManager::createInstance(
    std::make_unique<tb::TestPreferenceStore>(), K(saveInstantly));

  auto app = QApplication{argc, argv};

  return Catch::Session().run(argc, argv);
}
