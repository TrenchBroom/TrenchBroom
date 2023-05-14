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

#include <QFileInfo>
#include <QString>

#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/PathInfo.h"
#include "IO/PathQt.h"
#include "IO/TestEnvironment.h"
#include "Macros.h"

#include <algorithm>
#include <filesystem>

#include "Catch2.h"

namespace TrenchBroom::IO
{

namespace
{
TestEnvironment makeTestEnvironment()
{
  // have a non-ASCII character in the directory name to help catch
  // filename encoding bugs
  const auto hiraganaLetterSmallA = QString(static_cast<QChar>(0x3041));
  const auto dir = (QString::fromStdString(Catch::getResultCapture().getCurrentTestName())
                    + hiraganaLetterSmallA)
                     .toStdString();

  return TestEnvironment{
    dir, [](TestEnvironment& env) {
      env.createDirectory("dir1");
      env.createDirectory("dir2");
      env.createDirectory("anotherDir");
      env.createDirectory("anotherDir/subDirTest");

      env.createFile("test.txt", "some content");
      env.createFile("test2.map", "//test file\n{}");
      env.createFile("anotherDir/subDirTest/test2.map", "//sub dir test file\n{}");
      env.createFile("anotherDir/test3.map", "//yet another test file\n{}");
    }};
}
} // namespace

TEST_CASE("DiskIO")
{
  const auto env = makeTestEnvironment();

  SECTION("fixPath")
  {
    CHECK(Disk::fixPath("asdf/blah") == "asdf/blah");
    CHECK(Disk::fixPath("/../../test") == "/test");

    if (Disk::isCaseSensitive())
    {
      CHECK(Disk::fixPath(env.dir() / "TEST.txt") == env.dir() / "test.txt");
      CHECK(
        Disk::fixPath(env.dir() / "anotHERDIR/./SUBdirTEST/../SubdirTesT/TesT2.MAP")
        == env.dir() / "anotherDir/subDirTest/test2.map");
    }
  }

  SECTION("pathInfo")
  {
    CHECK(Disk::pathInfo("asdf/bleh") == PathInfo::Unknown);
    CHECK(Disk::pathInfo(env.dir() / "anotherDir/asdf.map") == PathInfo::Unknown);
    CHECK(Disk::pathInfo(env.dir() / "anotherDir/test3.map/asdf") == PathInfo::Unknown);

    CHECK(Disk::pathInfo(env.dir() / "anotherDir") == PathInfo::Directory);
    CHECK(Disk::pathInfo(env.dir() / "ANOTHERDIR") == PathInfo::Directory);
    CHECK(Disk::pathInfo(env.dir() / "anotherDir/subDirTest") == PathInfo::Directory);

    CHECK(Disk::pathInfo(env.dir() / "anotherDir/test3.map") == PathInfo::File);
    CHECK(Disk::pathInfo(env.dir() / "anotherDir/TEST3.map") == PathInfo::File);
    CHECK(
      Disk::pathInfo(env.dir() / "anotherDir/subDirTest/test2.map") == PathInfo::File);
  }

  SECTION("directoryContents")
  {
    CHECK_THROWS_AS(Disk::directoryContents("asdf/bleh"), FileSystemException);
    CHECK_THROWS_AS(
      Disk::directoryContents(env.dir() / "does/not/exist"), FileSystemException);

    CHECK_THAT(
      Disk::directoryContents(env.dir()),
      Catch::UnorderedEquals(std::vector<std::filesystem::path>{
        "dir1",
        "dir2",
        "anotherDir",
        "test.txt",
        "test2.map",
      }));
  }

  SECTION("openFile")
  {

    CHECK_THROWS_AS(Disk::openFile("asdf/bleh"), FileNotFoundException);
    CHECK_THROWS_AS(Disk::openFile(env.dir() / "does/not/exist"), FileNotFoundException);

    CHECK_THROWS_AS(
      Disk::openFile(env.dir() / "does_not_exist.txt"), FileNotFoundException);
    CHECK(Disk::openFile(env.dir() / "test.txt") != nullptr);
    CHECK(Disk::openFile(env.dir() / "anotherDir/subDirTest/test2.map") != nullptr);
  }

  SECTION("withStream")
  {
    const auto readAll = [](auto& stream) {
      return std::string{
        std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
    };

    SECTION("withInputStream")
    {
      CHECK_THROWS_AS(
        Disk::withInputStream(env.dir() / "does not exist.txt", readAll),
        FileSystemException);

      CHECK(Disk::withInputStream(env.dir() / "test.txt", readAll) == "some content");
    }

    SECTION("withOutputStream")
    {
      Disk::withOutputStream(
        env.dir() / "test.txt", std::ios::out | std::ios::app, [](auto& stream) {
          stream << "\nmore content";
        });
      CHECK(
        Disk::withInputStream(env.dir() / "test.txt", readAll)
        == "some content\nmore content");

      Disk::withOutputStream(env.dir() / "some_other_name.txt", [](auto& stream) {
        stream << "some text...";
      });
      CHECK(
        Disk::withInputStream(env.dir() / "some_other_name.txt", readAll)
        == "some text...");
    }
  }

  SECTION("resolvePath")
  {
    const auto rootPaths =
      std::vector<std::filesystem::path>{env.dir(), env.dir() / "anotherDir"};

    CHECK(Disk::resolvePath(rootPaths, "test.txt") == env.dir() / "test.txt");
    CHECK(
      Disk::resolvePath(rootPaths, "test3.map") == env.dir() / "anotherDir/test3.map");
    CHECK(
      Disk::resolvePath(rootPaths, "subDirTest/test2.map")
      == env.dir() / "anotherDir/subDirTest/test2.map");
    CHECK(Disk::resolvePath(rootPaths, "/asfd/blah") == "");
    CHECK(Disk::resolvePath(rootPaths, "adk3kdk/bhb") == "");
  }
}
} // namespace TrenchBroom::IO
