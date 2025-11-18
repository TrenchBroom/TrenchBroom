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

#include <QDir>

#include "update/TestUtils.h"
#include "update/Unzip.h"

#include <catch2/catch_test_macros.hpp>

namespace upd
{

TEST_CASE("Unzip")
{
  const auto fixturePath = QDir::currentPath() + "/fixture/unzip";
  const auto zipPath = fixturePath + "/archive.zip";
  const auto destPath = fixturePath + "/extracted";

  REQUIRE(QDir{destPath}.removeRecursively());

  REQUIRE(!QFileInfo{destPath + "/test1.txt"}.exists());
  REQUIRE(!QFileInfo{destPath + "/folder/test2.txt"}.exists());

  CHECK(unzip(zipPath, destPath, std::nullopt));
  CHECK(readFileIntoString(destPath + "/test1.txt") == "test1");
  CHECK(readFileIntoString(destPath + "/folder/test2.txt") == "test2");
}

} // namespace upd
