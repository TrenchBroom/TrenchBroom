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
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest/QTest>

#include "TestUtils.h"
#include "update/Unzip.h"

#include <catch2/catch_test_macros.hpp>

namespace upd
{

namespace
{

QString findFixturePath(const char* relativePath)
{
  const auto path = QFINDTESTDATA(relativePath);
  REQUIRE(!path.isEmpty());
  return path;
}

QString writeZipFixture(
  const QTemporaryDir& tempDir, const QString& fileName, const QByteArray& archiveData)
{
  const auto zipPath = tempDir.filePath(fileName);
  auto file = QFile{zipPath};
  REQUIRE(file.open(QIODevice::WriteOnly));
  REQUIRE(file.write(archiveData) == archiveData.size());
  return zipPath;
}

} // namespace

TEST_CASE("Unzip")
{
  const auto zipPath = findFixturePath("../fixture/unzip/archive.zip");
  auto tempDir = QTemporaryDir{};
  REQUIRE(tempDir.isValid());
  const auto destPath = tempDir.filePath("extracted");

  REQUIRE(!QFileInfo{destPath + "/test1.txt"}.exists());
  REQUIRE(!QFileInfo{destPath + "/folder/test2.txt"}.exists());

  CHECK(unzip(zipPath, destPath, std::nullopt));
  CHECK(readFileIntoString(destPath + "/test1.txt") == "test1");
  CHECK(readFileIntoString(destPath + "/folder/test2.txt") == "test2");
}

#if defined(Q_OS_UNIX)
TEST_CASE("Unzip preserves executable permissions")
{
  static const auto archiveBase64 = QByteArray{
    "UEsDBBQAAAAAAAAAIQAAAAAAAAAAAAAAAAAEAAAAYmluL1BLAwQUAAAAAAAAACEAKE6j3hIAAAAS"
    "AAAACwAAAGJpbi90b29sLnNoIyEvYmluL3NoCmVjaG8gb2sKUEsBAhQDFAAAAAAAAAAhAAAAAAAA"
    "AAAAAAAAAAQAAAAAAAAAAAAQAO1BAAAAAGJpbi9QSwECFAMUAAAAAAAAACEAKE6j3hIAAAASAAAA"
    "CwAAAAAAAAAAAAAA7YEiAAAAYmluL3Rvb2wuc2hQSwUGAAAAAAIAAgBrAAAAXQAAAAAA"};

  auto tempDir = QTemporaryDir{};
  REQUIRE(tempDir.isValid());

  const auto zipPath =
    writeZipFixture(tempDir, "permissions.zip", QByteArray::fromBase64(archiveBase64));
  const auto destPath = tempDir.filePath("extracted");

  REQUIRE(unzip(zipPath, destPath, std::nullopt));

  const auto extractedFile = QFileInfo{destPath + "/bin/tool.sh"};
  REQUIRE(extractedFile.exists());

  const auto permissions = extractedFile.permissions();
  CHECK((permissions & QFileDevice::ReadOwner) == QFileDevice::ReadOwner);
  CHECK((permissions & QFileDevice::WriteOwner) == QFileDevice::WriteOwner);
  CHECK((permissions & QFileDevice::ExeOwner) == QFileDevice::ExeOwner);
  CHECK((permissions & QFileDevice::ReadGroup) == QFileDevice::ReadGroup);
  CHECK((permissions & QFileDevice::ExeGroup) == QFileDevice::ExeGroup);
  CHECK((permissions & QFileDevice::ReadOther) == QFileDevice::ReadOther);
  CHECK((permissions & QFileDevice::ExeOther) == QFileDevice::ExeOther);
}
#endif

} // namespace upd
