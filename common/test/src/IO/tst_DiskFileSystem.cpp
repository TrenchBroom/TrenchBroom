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

#include <QFileInfo>
#include <QString>

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

TEST_CASE("DiskFileSystemTest")
{
  const auto env = makeTestEnvironment();

  SECTION("constructor")
  {
    CHECK_THROWS_AS(DiskFileSystem(env.dir() / "asdf", true), FileSystemException);
    CHECK_NOTHROW(DiskFileSystem(env.dir() / "asdf", false));
    CHECK_NOTHROW(DiskFileSystem(env.dir(), true));

    // for case sensitive file systems
    CHECK_NOTHROW(DiskFileSystem(env.dir() / "ANOTHERDIR", true));

    const DiskFileSystem fs(env.dir() / "anotherDir/..", true);
    CHECK(fs.makeAbsolute("") == fs.root());
  }

  const auto fs = DiskFileSystem{env.dir()};

  SECTION("makeAbsolute")
  {

#if defined _WIN32
    CHECK_THROWS_AS(fs.makeAbsolute("c:\\"), FileSystemException);
    CHECK_THROWS_AS(
      fs.makeAbsolute("C:\\does_not_exist_i_hope.txt"), FileSystemException);
#else
    CHECK_THROWS_AS(fs.makeAbsolute("/"), FileSystemException);
    CHECK_THROWS_AS(fs.makeAbsolute("/does_not_exist_i_hope.txt"), FileSystemException);
#endif

    CHECK(
      fs.makeAbsolute("dir1/does_not_exist.txt")
      == env.dir() / "dir1/does_not_exist.txt");
    CHECK(fs.makeAbsolute("test.txt") == env.dir() / "test.txt");
    CHECK(fs.makeAbsolute("anotherDir") == env.dir() / "anotherDir");
  }

  SECTION("pathInfo")
  {
#if defined _WIN32
    CHECK_THROWS_AS(fs.pathInfo("c:\\"), FileSystemException);
    CHECK_THROWS_AS(fs.pathInfo("C:\\does_not_exist_i_hope.txt"), FileSystemException);
#else
    CHECK_THROWS_AS(fs.pathInfo("/"), FileSystemException);
    CHECK_THROWS_AS(fs.pathInfo("/does_not_exist_i_hope.txt"), FileSystemException);
#endif
    CHECK_THROWS_AS(fs.pathInfo(".."), FileSystemException);

    CHECK(fs.pathInfo(".") == PathInfo::Directory);
    CHECK(fs.pathInfo("anotherDir") == PathInfo::Directory);
    CHECK(fs.pathInfo("anotherDir/subDirTest") == PathInfo::Directory);
    CHECK(fs.pathInfo("anotherDir/./subDirTest/..") == PathInfo::Directory);
    CHECK(fs.pathInfo("ANOTHerDir") == PathInfo::Directory);
    CHECK(fs.pathInfo("test.txt") == PathInfo::File);
    CHECK(fs.pathInfo("fasdf") == PathInfo::Unknown);

    CHECK(fs.pathInfo("test.txt") == PathInfo::File);
    CHECK(fs.pathInfo("./test.txt") == PathInfo::File);
    CHECK(fs.pathInfo("anotherDir/test3.map") == PathInfo::File);
    CHECK(
      fs.pathInfo("anotherDir/./subDirTest/../subDirTest/test2.map") == PathInfo::File);
    CHECK(fs.pathInfo("ANOtherDir/test3.MAP") == PathInfo::File);
    CHECK(fs.pathInfo("anotherDir/whatever.txt") == PathInfo::Unknown);
    CHECK(fs.pathInfo("fdfdf.blah") == PathInfo::Unknown);
  }

  SECTION("directoryContents")
  {
#if defined _WIN32
    CHECK_THROWS_AS(fs.directoryContents("c:\\"), FileSystemException);
#else
    CHECK_THROWS_AS(fs.directoryContents("/"), FileSystemException);
#endif
    CHECK_THROWS_AS(fs.directoryContents(".."), FileSystemException);
    CHECK_THROWS_AS(fs.directoryContents("asdf/bleh"), FileSystemException);

    CHECK_THAT(
      fs.directoryContents("."),
      Catch::UnorderedEquals(std::vector<std::filesystem::path>{
        "dir1",
        "dir2",
        "anotherDir",
        "test.txt",
        "test2.map",
      }));

    CHECK_THAT(
      fs.directoryContents("anotherDir"),
      Catch::UnorderedEquals(std::vector<std::filesystem::path>{
        "subDirTest",
        "test3.map",
      }));
  }

  SECTION("openFile")
  {
#if defined _WIN32
    CHECK_THROWS_AS(fs.openFile("c:\\hopefully_nothing.here"), FileSystemException);
#else
    CHECK_THROWS_AS(fs.openFile("/hopefully_nothing.here"), FileSystemException);
#endif
    CHECK_THROWS_AS(fs.openFile(".."), FileSystemException);
    CHECK_THROWS_AS(fs.openFile("."), FileSystemException);
    CHECK_THROWS_AS(fs.openFile("anotherDir"), FileSystemException);

    const auto checkOpenFile = [&](const auto& path) {
      const auto file = fs.openFile(path);
      CHECK(file != nullptr);
      CHECK(file->path() == path);
    };

    checkOpenFile("test.txt");
    checkOpenFile("anotherDir/test3.map");
    checkOpenFile("anotherDir/../anotherDir/./test3.map");
  }
}

TEST_CASE("WritableDiskFileSystemTest")
{
  SECTION("createWritableDiskFileSystem")
  {
    const auto env = makeTestEnvironment();

    CHECK_THROWS_AS(
      WritableDiskFileSystem(env.dir() / "asdf", false), FileSystemException);
    CHECK_NOTHROW(WritableDiskFileSystem{env.dir() / "asdf", true});
    CHECK_NOTHROW(WritableDiskFileSystem{env.dir(), true});

    // for case sensitive file systems
    CHECK_NOTHROW(WritableDiskFileSystem{env.dir() / "ANOTHERDIR", false});

    const auto fs = WritableDiskFileSystem{env.dir() / "anotherDir/..", false};
    CHECK(fs.makeAbsolute("") == (env.dir() / "anotherDir/..").lexically_normal());
  }

  SECTION("createDirectory")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir(), false};

#if defined _WIN32
    CHECK_THROWS_AS(
      fs.createDirectory("c:\\hopefully_nothing_here"), FileSystemException);
#else
    CHECK_THROWS_AS(fs.createDirectory("/hopefully_nothing_here"), FileSystemException);
#endif
    CHECK_THROWS_AS(fs.createDirectory(""), FileSystemException);
    CHECK_THROWS_AS(fs.createDirectory("."), FileSystemException);
    CHECK_THROWS_AS(fs.createDirectory(".."), FileSystemException);
    CHECK_THROWS_AS(fs.createDirectory("dir1"), FileSystemException);
    CHECK_THROWS_AS(fs.createDirectory("test.txt"), FileSystemException);
    CHECK_THROWS_AS(
      fs.createDirectory("someDir/someOtherDir/.././yetAnotherDir/."),
      FileSystemException);

    fs.createDirectory("newDir");
    CHECK(fs.pathInfo("newDir") == PathInfo::Directory);

    fs.createDirectory("newDir/someOtherDir");
    CHECK(fs.pathInfo("newDir/someOtherDir") == PathInfo::Directory);
  }

  SECTION("deleteFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir(), false};

#if defined _WIN32
    CHECK_THROWS_AS(fs.deleteFile("c:\\hopefully_nothing_here.txt"), FileSystemException);
#else
    CHECK_THROWS_AS(fs.deleteFile("/hopefully_nothing_here.txt"), FileSystemException);
#endif
    CHECK_THROWS_AS(fs.deleteFile(""), FileSystemException);
    CHECK_THROWS_AS(fs.deleteFile("."), FileSystemException);
    CHECK_THROWS_AS(fs.deleteFile(".."), FileSystemException);
    CHECK_THROWS_AS(fs.deleteFile("dir1"), FileSystemException);
    CHECK_THROWS_AS(fs.deleteFile("asdf.txt"), FileSystemException);
    CHECK_THROWS_AS(fs.deleteFile("/dir1/asdf.txt"), FileSystemException);

    fs.deleteFile("test.txt");
    CHECK(fs.pathInfo("test.txt") == PathInfo::Unknown);

    fs.deleteFile("anotherDir/test3.map");
    CHECK(fs.pathInfo("anotherDir/test3.map") == PathInfo::Unknown);

    fs.deleteFile("anotherDir/subDirTest/.././subDirTest/./test2.map");
    CHECK(fs.pathInfo("anotherDir/subDirTest/test2.map") == PathInfo::Unknown);
  }

  SECTION("moveFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir(), false};

#if defined _WIN32
    CHECK_THROWS_AS(
      fs.moveFile("c:\\hopefully_nothing_here.txt", "dest.txt", false),
      FileSystemException);
    CHECK_THROWS_AS(fs.moveFile("test.txt", "C:\\dest.txt", false), FileSystemException);
#else
    CHECK_THROWS_AS(
      fs.moveFile("/hopefully_nothing_here.txt", "dest.txt", false), FileSystemException);
    CHECK_THROWS_AS(fs.moveFile("test.txt", "/dest.txt", false), FileSystemException);
#endif

    CHECK_THROWS_AS(fs.moveFile("test.txt", "test2.map", false), FileSystemException);
    CHECK_THROWS_AS(
      fs.moveFile("test.txt", "anotherDir/test3.map", false), FileSystemException);
    CHECK_THROWS_AS(
      fs.moveFile("test.txt", "anotherDir/../anotherDir/./test3.map", false),
      FileSystemException);

    fs.moveFile("test.txt", "test2.txt", true);
    CHECK(fs.pathInfo("test.txt") == PathInfo::Unknown);
    CHECK(fs.pathInfo("test2.txt") == PathInfo::File);

    fs.moveFile("test2.txt", "test2.map", true);
    CHECK(fs.pathInfo("test2.txt") == PathInfo::Unknown);
    CHECK(fs.pathInfo("test2.map") == PathInfo::File);
    // we're trusting that the file is actually overwritten (should really test the
    // contents here...)

    fs.moveFile("test2.map", "dir1/test2.map", true);
    CHECK(fs.pathInfo("test2.map") == PathInfo::Unknown);
    CHECK(fs.pathInfo("dir1/test2.map") == PathInfo::File);
  }

  SECTION("copyFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir(), false};

#if defined _WIN32
    CHECK_THROWS_AS(
      fs.copyFile("c:\\hopefully_nothing_here.txt", "dest.txt", false),
      FileSystemException);
    CHECK_THROWS_AS(fs.copyFile("test.txt", "C:\\dest.txt", false), FileSystemException);
#else
    CHECK_THROWS_AS(
      fs.copyFile("/hopefully_nothing_here.txt", "dest.txt", false), FileSystemException);
    CHECK_THROWS_AS(fs.copyFile("test.txt", "/dest.txt", false), FileSystemException);
#endif

    CHECK_THROWS_AS(fs.copyFile("test.txt", "test2.map", false), FileSystemException);
    CHECK_THROWS_AS(
      fs.copyFile("test.txt", "anotherDir/test3.map", false), FileSystemException);
    CHECK_THROWS_AS(
      fs.copyFile("test.txt", "anotherDir/../anotherDir/./test3.map", false),
      FileSystemException);

    fs.copyFile("test.txt", "test2.txt", true);
    CHECK(fs.pathInfo("test.txt") == PathInfo::File);
    CHECK(fs.pathInfo("test2.txt") == PathInfo::File);

    fs.copyFile("test2.txt", "test2.map", true);
    CHECK(fs.pathInfo("test2.txt") == PathInfo::File);
    CHECK(fs.pathInfo("test2.map") == PathInfo::File);
    // we're trusting that the file is actually overwritten (should really test the
    // contents here...)

    fs.copyFile("test2.map", "dir1/test2.map", true);
    CHECK(fs.pathInfo("test2.map") == PathInfo::File);
    CHECK(fs.pathInfo("dir1/test2.map") == PathInfo::File);
  }
}

} // namespace TrenchBroom::IO
