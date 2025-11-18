/*
 Copyright (C) 2025 Kristian Duske

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

#include <QCoreApplication>
#include <QEventLoop>
#include <QNetworkAccessManager>

#include "upd/QtHttpClient.h"
#include "upd/TestUtils.h"

#include <catch2/catch_test_macros.hpp>

namespace upd
{

TEST_CASE("QtHttpClient", "[.][SKIP]")
{
  int argc = 0;
  char** argv = nullptr;
  auto app = QCoreApplication{argc, argv};
  auto loop = QEventLoop{};
  auto networkManager = QNetworkAccessManager{};
  auto client = QtHttpClient{networkManager};

  SECTION("get")
  {
    client.get(
      QUrl{"https://api.github.com/repos/TrenchBroom/TrenchBroom/releases"},
      [&](const auto&) { loop.quit(); },
      [](const auto& error) { FAIL(error.toStdString()); });

    loop.exec();
  }

  SECTION("download")
  {
    auto* download = client.download(
      QUrl{
        R"(https://github.com/TrenchBroom/TrenchBroom/releases/download/v2024.2/TrenchBroom-macOS-v2024.2-Release.dmg.md5)"},
      [&](auto& file) {
        loop.quit();

        CHECK(
          readFileIntoString(file)
          == R"(f9843fc03c931488c69b1366d14a2ec7  TrenchBroom-macOS-v2024.2-Release.dmg
)");
      },
      [](const auto& error) { FAIL(error.toStdString()); });


    loop.exec();
    CHECK(download->progress() == 1.0f);
  }
}

} // namespace upd
