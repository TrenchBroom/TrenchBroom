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
#include "IO/FileSystemError.h"
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

TEST_CASE("DiskFileSystemTest")
{
  const auto env = makeTestEnvironment();

  const auto fs = DiskFileSystem{env.dir()};

  SECTION("makeAbsolute")
  {

#if defined _WIN32
    CHECK(fs.makeAbsolute("c:\\") == "c:\\");
    CHECK(
      fs.makeAbsolute("C:\\does_not_exist_i_hope.txt")
      == "C:\\does_not_exist_i_hope.txt");
#else
    CHECK(fs.makeAbsolute("/") == "/");
    CHECK(fs.makeAbsolute("/does_not_exist_i_hope.txt") == "/does_not_exist_i_hope.txt");
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
    CHECK(fs.pathInfo("c:\\") == PathInfo::Directory);
    CHECK(fs.pathInfo("C:\\does_not_exist_i_hope.txt") == PathInfo::Unknown);
#else
    CHECK(fs.pathInfo("/") == PathInfo::Directory);
    CHECK(fs.pathInfo("/does_not_exist_i_hope.txt") == PathInfo::Unknown);
#endif
    CHECK(fs.pathInfo("..") == PathInfo::Unknown);

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

    const auto fs = WritableDiskFileSystem{env.dir() / "anotherDir/.."};
    CHECK(fs.makeAbsolute("") == (env.dir() / "anotherDir/..").lexically_normal());
  }

  SECTION("createDirectory")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir()};

#if defined _WIN32
    CHECK(
      fs.createDirectory("c:\\hopefully_nothing_here")
      == kdl::result<bool, FileSystemError>{FileSystemError{}});
#else
    CHECK(
      fs.createDirectory("/hopefully_nothing_here")
      == kdl::result<bool, FileSystemError>{FileSystemError{}});
#endif
    CHECK(
      fs.createDirectory("..") == kdl::result<bool, FileSystemError>{FileSystemError{}});
    CHECK(
      fs.createDirectory("test.txt")
      == kdl::result<bool, FileSystemError>{FileSystemError{}});

    CHECK(fs.createDirectory("") == kdl::result<bool, FileSystemError>{false});
    CHECK(fs.createDirectory(".") == kdl::result<bool, FileSystemError>{false});
    CHECK(fs.createDirectory("dir1") == kdl::result<bool, FileSystemError>{false});

    CHECK(fs.createDirectory("newDir") == kdl::result<bool, FileSystemError>{true});
    CHECK(fs.pathInfo("newDir") == PathInfo::Directory);

    CHECK(
      fs.createDirectory("newDir/someOtherDir")
      == kdl::result<bool, FileSystemError>{true});
    CHECK(fs.pathInfo("newDir/someOtherDir") == PathInfo::Directory);

    CHECK(
      fs.createDirectory("someDir/someOtherDir/.././yetAnotherDir")
      == kdl::result<bool, FileSystemError>{true});
    CHECK(fs.pathInfo("someDir/someOtherDir/.././yetAnotherDir") == PathInfo::Directory);
  }

  SECTION("deleteFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir()};

#if defined _WIN32
    CHECK(
      fs.deleteFile("c:\\hopefully_nothing_here.txt")
      == kdl::result<bool, FileSystemError>{FileSystemError{}});
    CHECK(
      fs.deleteFile("c:\\dir1\\asdf.txt")
      == kdl::result<bool, FileSystemError>{FileSystemError{}});
#else
    CHECK(
      fs.deleteFile("/hopefully_nothing_here.txt")
      == kdl::result<bool, FileSystemError>{FileSystemError{}});
    CHECK(
      fs.deleteFile("/dir1/asdf.txt")
      == kdl::result<bool, FileSystemError>{FileSystemError{}});
#endif
    CHECK(fs.deleteFile("") == kdl::result<bool, FileSystemError>{FileSystemError{}});
    CHECK(fs.deleteFile(".") == kdl::result<bool, FileSystemError>{FileSystemError{}});
    CHECK(fs.deleteFile("..") == kdl::result<bool, FileSystemError>{FileSystemError{}});
    CHECK(fs.deleteFile("dir1") == kdl::result<bool, FileSystemError>{FileSystemError{}});

    CHECK(fs.deleteFile("asdf.txt") == kdl::result<bool, FileSystemError>{false});
    CHECK(fs.deleteFile("test.txt") == kdl::result<bool, FileSystemError>{true});
    CHECK(fs.pathInfo("test.txt") == PathInfo::Unknown);

    CHECK(
      fs.deleteFile("anotherDir/test3.map") == kdl::result<bool, FileSystemError>{true});
    CHECK(fs.pathInfo("anotherDir/test3.map") == PathInfo::Unknown);

    CHECK(
      fs.deleteFile("anotherDir/subDirTest/.././subDirTest/./test2.map")
      == kdl::result<bool, FileSystemError>{true});
    CHECK(fs.pathInfo("anotherDir/subDirTest/test2.map") == PathInfo::Unknown);
  }

  SECTION("moveFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir()};

#if defined _WIN32
    CHECK_THROWS_AS(
      fs.moveFile("c:\\hopefully_nothing_here.txt", "dest.txt"), FileSystemException);
    CHECK_THROWS_AS(fs.moveFile("test.txt", "C:\\dest.txt"), FileSystemException);
#else
    CHECK_THROWS_AS(
      fs.moveFile("/hopefully_nothing_here.txt", "dest.txt"), FileSystemException);
    CHECK_THROWS_AS(fs.moveFile("test.txt", "/dest.txt"), FileSystemException);
#endif

    fs.moveFile("test.txt", "test2.txt");
    CHECK(fs.pathInfo("test.txt") == PathInfo::Unknown);
    CHECK(fs.pathInfo("test2.txt") == PathInfo::File);

    fs.moveFile("test2.txt", "test2.map");
    CHECK(fs.pathInfo("test2.txt") == PathInfo::Unknown);
    CHECK(fs.pathInfo("test2.map") == PathInfo::File);
    // we're trusting that the file is actually overwritten (should really test the
    // contents here...)

    fs.moveFile("test2.map", "dir1/test2.map");
    CHECK(fs.pathInfo("test2.map") == PathInfo::Unknown);
    CHECK(fs.pathInfo("dir1/test2.map") == PathInfo::File);
  }

  SECTION("copyFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir()};

#if defined _WIN32
    CHECK(
      fs.copyFile("c:\\hopefully_nothing_here.txt", "dest.txt")
      == kdl::result<void, FileSystemError>{FileSystemError{}});
    CHECK(
      fs.copyFile("test.txt", "C:\\dest.txt")
      == kdl::result<void, FileSystemError>{FileSystemError{}});
#else
    CHECK(
      fs.copyFile("/hopefully_nothing_here.txt", "dest.txt")
      == kdl::result<void, FileSystemError>{FileSystemError{}});
    CHECK(
      fs.copyFile("test.txt", "/dest.txt")
      == kdl::result<void, FileSystemError>{FileSystemError{}});
#endif

    CHECK(fs.copyFile("test.txt", "test2.txt") == kdl::result<void, FileSystemError>{});
    CHECK(fs.pathInfo("test.txt") == PathInfo::File);
    CHECK(fs.pathInfo("test2.txt") == PathInfo::File);

    CHECK(fs.copyFile("test2.txt", "test2.map") == kdl::result<void, FileSystemError>{});
    CHECK(fs.pathInfo("test2.txt") == PathInfo::File);
    CHECK(fs.pathInfo("test2.map") == PathInfo::File);
    // we're trusting that the file is actually overwritten (should really test the
    // contents here...)

    CHECK(
      fs.copyFile("test2.map", "dir1/test2.map") == kdl::result<void, FileSystemError>{});
    CHECK(fs.pathInfo("test2.map") == PathInfo::File);
    CHECK(fs.pathInfo("dir1/test2.map") == PathInfo::File);
  }
}

} // namespace TrenchBroom::IO
