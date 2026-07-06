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
#include <QProcess>
#include <QTemporaryDir>
#include <QtSystemDetection>
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

TEST_CASE("Unzip fails when archive does not exist")
{
  auto tempDir = QTemporaryDir{};
  REQUIRE(tempDir.isValid());

  const auto zipPath = tempDir.filePath("missing.zip");
  const auto destPath = tempDir.filePath("extracted");

  CHECK_FALSE(unzip(zipPath, destPath, std::nullopt));
  CHECK_FALSE(QFileInfo{destPath}.exists());
}

TEST_CASE("Unzip fails when destination path is a file")
{
  const auto zipPath = findFixturePath("../fixture/unzip/archive.zip");
  auto tempDir = QTemporaryDir{};
  REQUIRE(tempDir.isValid());

  const auto destPath = tempDir.filePath("not_a_directory");
  auto destFile = QFile{destPath};
  REQUIRE(destFile.open(QIODevice::WriteOnly));
  REQUIRE(destFile.write("x") == 1);
  destFile.close();

  CHECK_FALSE(unzip(zipPath, destPath, std::nullopt));
}

TEST_CASE("Unzip fails for invalid archive content")
{
  auto tempDir = QTemporaryDir{};
  REQUIRE(tempDir.isValid());

  const auto zipPath = writeZipFixture(tempDir, "invalid.zip", QByteArray{"not a zip"});
  const auto destPath = tempDir.filePath("extracted");

  CHECK_FALSE(unzip(zipPath, destPath, std::nullopt));
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

#if defined(Q_OS_MACOS)
TEST_CASE("Unzip preserves extended attributes on macOS")
{
  // Test that macOS extended attributes (including code signatures) are preserved
  // This prevents the regression where minizip would strip extended attributes
  // causing code-signed app bundles to lose their signatures on macOS Sequoia
  auto tempDir = QTemporaryDir{};
  REQUIRE(tempDir.isValid());

  // Create a test file and set an extended attribute on it
  const auto testFilePath = tempDir.filePath("test.txt");
  auto testFile = QFile{testFilePath};
  REQUIRE(testFile.open(QIODevice::WriteOnly));
  REQUIRE(testFile.write("test content") > 0);
  testFile.close();

  // Set an extended attribute on the file
  auto setAttrProcess = QProcess{};
  setAttrProcess.setProgram("/usr/bin/xattr");
  setAttrProcess.setArguments(
    QStringList{"-w", "com.apple.test", "testvalue", testFilePath});
  setAttrProcess.start();
  REQUIRE(setAttrProcess.waitForFinished(5000));
  REQUIRE(setAttrProcess.exitCode() == 0);

  // Verify the attribute was set
  auto checkProcess = QProcess{};
  checkProcess.setProgram("/usr/bin/xattr");
  checkProcess.setArguments(QStringList{"-p", "com.apple.test", testFilePath});
  checkProcess.start();
  REQUIRE(checkProcess.waitForFinished(5000));
  REQUIRE(checkProcess.exitCode() == 0);
  REQUIRE(
    QString::fromUtf8(checkProcess.readAllStandardOutput()).trimmed() == "testvalue");

  // Create a directory with the test file and zip it using ditto
  const auto srcDir = tempDir.filePath("src");
  REQUIRE(QDir{}.mkpath(srcDir));
  REQUIRE(QFile::copy(testFilePath, srcDir + "/test.txt"));

  // Set the extended attribute on the file in src dir
  setAttrProcess.setArguments(
    QStringList{"-w", "com.apple.test", "testvalue", srcDir + "/test.txt"});
  setAttrProcess.start();
  REQUIRE(setAttrProcess.waitForFinished(5000));
  REQUIRE(setAttrProcess.exitCode() == 0);

  // Create zip using ditto
  const auto zipPath = tempDir.filePath("archive.zip");
  auto dittoProcess = QProcess{};
  dittoProcess.setProgram("/usr/bin/ditto");
  dittoProcess.setArguments(
    QStringList{"-c", "-k", "--sequesterRsrc", "--keepParent", srcDir, zipPath});
  dittoProcess.start();
  REQUIRE(dittoProcess.waitForFinished(5000));
  REQUIRE(dittoProcess.exitCode() == 0);

  // Extract using our unzip function
  const auto extractDir = tempDir.filePath("extracted");
  REQUIRE(unzip(zipPath, extractDir, std::nullopt));

  // Verify the extracted file exists
  const auto extractedFilePath = extractDir + "/src/test.txt";
  REQUIRE(QFileInfo{extractedFilePath}.exists());

  // Verify the extended attribute was preserved
  checkProcess.setArguments(QStringList{"-p", "com.apple.test", extractedFilePath});
  checkProcess.start();
  REQUIRE(checkProcess.waitForFinished(5000));
  REQUIRE(checkProcess.exitCode() == 0);
  REQUIRE(
    QString::fromUtf8(checkProcess.readAllStandardOutput()).trimmed() == "testvalue");
}
#endif

} // namespace upd
