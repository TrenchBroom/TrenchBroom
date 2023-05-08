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

#include "Catch2.h"
#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "IO/PathInfo.h"
#include "IO/PathQt.h"
#include "IO/TestEnvironment.h"
#include "Macros.h"

#include <algorithm>

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
      env.createDirectory(Path{"dir1"});
      env.createDirectory(Path{"dir2"});
      env.createDirectory(Path{"anotherDir"});
      env.createDirectory(Path{"anotherDir/subDirTest"});

      env.createFile(Path{"test.txt"}, "some content");
      env.createFile(Path{"test2.map"}, "//test file\n{}");
      env.createFile(Path{"anotherDir/subDirTest/test2.map"}, "//sub dir test file\n{}");
      env.createFile(Path{"anotherDir/test3.map"}, "//yet another test file\n{}");
    }};
}
} // namespace

TEST_CASE("DiskFileSystemTest")
{
  const auto env = makeTestEnvironment();

  SECTION("constructor")
  {
    CHECK_THROWS_AS(DiskFileSystem(env.dir() + Path{"asdf"}, true), FileSystemException);
    CHECK_NOTHROW(DiskFileSystem(env.dir() + Path{"asdf"}, false));
    CHECK_NOTHROW(DiskFileSystem(env.dir(), true));

    // for case sensitive file systems
    CHECK_NOTHROW(DiskFileSystem(env.dir() + Path{"ANOTHERDIR"}, true));

    const DiskFileSystem fs(env.dir() + Path{"anotherDir/.."}, true);
    CHECK(fs.makeAbsolute(Path{""}) == fs.root());
  }

  const auto fs = DiskFileSystem{env.dir()};

  SECTION("makeAbsolute")
  {

#if defined _WIN32
    CHECK_THROWS_AS(fs.makeAbsolute(Path{"c:\\"}), FileSystemException);
    CHECK_THROWS_AS(
      fs.makeAbsolute(Path{"C:\\does_not_exist_i_hope.txt"}), FileSystemException);
#else
    CHECK_THROWS_AS(fs.makeAbsolute(Path{"/"}), FileSystemException);
    CHECK_THROWS_AS(
      fs.makeAbsolute(Path{"/does_not_exist_i_hope.txt"}), FileSystemException);
#endif

    CHECK(
      fs.makeAbsolute(Path{"dir1/does_not_exist.txt"})
      == env.dir() + Path("dir1/does_not_exist.txt"));
    CHECK(fs.makeAbsolute(Path{"test.txt"}) == env.dir() + Path{"test.txt"});
    CHECK(fs.makeAbsolute(Path{"anotherDir"}) == env.dir() + Path{"anotherDir"});
  }

  SECTION("pathInfo")
  {
#if defined _WIN32
    CHECK_THROWS_AS(fs.pathInfo(Path{"c:\\"}), FileSystemException);
    CHECK_THROWS_AS(
      fs.pathInfo(Path{"C:\\does_not_exist_i_hope.txt"}), FileSystemException);
#else
    CHECK_THROWS_AS(fs.pathInfo(Path{"/"}), FileSystemException);
    CHECK_THROWS_AS(fs.pathInfo(Path{"/does_not_exist_i_hope.txt"}), FileSystemException);
#endif
    CHECK_THROWS_AS(fs.pathInfo(Path{".."}), FileSystemException);

    CHECK(fs.pathInfo(Path{"."}) == PathInfo::Directory);
    CHECK(fs.pathInfo(Path{"anotherDir"}) == PathInfo::Directory);
    CHECK(fs.pathInfo(Path{"anotherDir/subDirTest"}) == PathInfo::Directory);
    CHECK(fs.pathInfo(Path{"anotherDir/./subDirTest/.."}) == PathInfo::Directory);
    CHECK(fs.pathInfo(Path{"ANOTHerDir"}) == PathInfo::Directory);
    CHECK(fs.pathInfo(Path{"test.txt"}) == PathInfo::File);
    CHECK(fs.pathInfo(Path{"fasdf"}) == PathInfo::Unknown);

    CHECK(fs.pathInfo(Path{"test.txt"}) == PathInfo::File);
    CHECK(fs.pathInfo(Path{"./test.txt"}) == PathInfo::File);
    CHECK(fs.pathInfo(Path{"anotherDir/test3.map"}) == PathInfo::File);
    CHECK(
      fs.pathInfo(Path{"anotherDir/./subDirTest/../subDirTest/test2.map"})
      == PathInfo::File);
    CHECK(fs.pathInfo(Path{"ANOtherDir/test3.MAP"}) == PathInfo::File);
    CHECK(fs.pathInfo(Path{"anotherDir/whatever.txt"}) == PathInfo::Unknown);
    CHECK(fs.pathInfo(Path{"fdfdf.blah"}) == PathInfo::Unknown);
  }

  SECTION("directoryContents")
  {
#if defined _WIN32
    CHECK_THROWS_AS(fs.directoryContents(Path{"c:\\"}), FileSystemException);
#else
    CHECK_THROWS_AS(fs.directoryContents(Path{"/"}), FileSystemException);
#endif
    CHECK_THROWS_AS(fs.directoryContents(Path{".."}), FileSystemException);
    CHECK_THROWS_AS(fs.directoryContents(Path{"asdf/bleh"}), FileSystemException);

    CHECK_THAT(
      fs.directoryContents(Path{"."}),
      Catch::UnorderedEquals(std::vector<Path>{
        Path{"dir1"},
        Path{"dir2"},
        Path{"anotherDir"},
        Path{"test.txt"},
        Path{"test2.map"},
      }));

    CHECK_THAT(
      fs.directoryContents(Path{"anotherDir"}),
      Catch::UnorderedEquals(std::vector<Path>{
        Path{"subDirTest"},
        Path{"test3.map"},
      }));
  }

  SECTION("openFile")
  {
#if defined _WIN32
    CHECK_THROWS_AS(fs.openFile(Path{"c:\\hopefully_nothing.here"}), FileSystemException);
#else
    CHECK_THROWS_AS(fs.openFile(Path{"/hopefully_nothing.here"}), FileSystemException);
#endif
    CHECK_THROWS_AS(fs.openFile(Path{".."}), FileSystemException);
    CHECK_THROWS_AS(fs.openFile(Path{"."}), FileSystemException);
    CHECK_THROWS_AS(fs.openFile(Path{"anotherDir"}), FileSystemException);

    const auto checkOpenFile = [&](const auto& path) {
      const auto file = fs.openFile(path);
      CHECK(file != nullptr);
      CHECK(file->path() == path);
    };

    checkOpenFile(Path{"test.txt"});
    checkOpenFile(Path{"anotherDir/test3.map"});
    checkOpenFile(Path{"anotherDir/../anotherDir/./test3.map"});
  }
}

TEST_CASE("WritableDiskFileSystemTest")
{
  SECTION("createWritableDiskFileSystem")
  {
    const auto env = makeTestEnvironment();

    CHECK_THROWS_AS(
      WritableDiskFileSystem(env.dir() + Path{"asdf"}, false), FileSystemException);
    CHECK_NOTHROW(WritableDiskFileSystem{env.dir() + Path{"asdf"}, true});
    CHECK_NOTHROW(WritableDiskFileSystem{env.dir(), true});

    // for case sensitive file systems
    CHECK_NOTHROW(WritableDiskFileSystem{env.dir() + Path{"ANOTHERDIR"}, false});

    const auto fs = WritableDiskFileSystem{env.dir() + Path{"anotherDir/.."}, false};
    CHECK(fs.makeAbsolute(Path{""}) == env.dir());
  }

  SECTION("createDirectory")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir(), false};

#if defined _WIN32
    CHECK_THROWS_AS(
      fs.createDirectory(Path{"c:\\hopefully_nothing_here"}), FileSystemException);
#else
    CHECK_THROWS_AS(
      fs.createDirectory(Path{"/hopefully_nothing_here"}), FileSystemException);
#endif
    CHECK_THROWS_AS(fs.createDirectory(Path{""}), FileSystemException);
    CHECK_THROWS_AS(fs.createDirectory(Path{"."}), FileSystemException);
    CHECK_THROWS_AS(fs.createDirectory(Path{".."}), FileSystemException);
    CHECK_THROWS_AS(fs.createDirectory(Path{"dir1"}), FileSystemException);
    CHECK_THROWS_AS(fs.createDirectory(Path{"test.txt"}), FileSystemException);

    fs.createDirectory(Path{"newDir"});
    CHECK(fs.pathInfo(Path{"newDir"}) == PathInfo::Directory);

    fs.createDirectory(Path{"newDir/someOtherDir"});
    CHECK(fs.pathInfo(Path{"newDir/someOtherDir"}) == PathInfo::Directory);

    fs.createDirectory(Path{"newDir/someOtherDir/.././yetAnotherDir/."});
    CHECK(fs.pathInfo(Path{"newDir/yetAnotherDir"}) == PathInfo::Directory);
  }

  SECTION("deleteFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir(), false};

#if defined _WIN32
    CHECK_THROWS_AS(
      fs.deleteFile(Path{"c:\\hopefully_nothing_here.txt"}), FileSystemException);
#else
    CHECK_THROWS_AS(
      fs.deleteFile(Path{"/hopefully_nothing_here.txt"}), FileSystemException);
#endif
    CHECK_THROWS_AS(fs.deleteFile(Path{""}), FileSystemException);
    CHECK_THROWS_AS(fs.deleteFile(Path{"."}), FileSystemException);
    CHECK_THROWS_AS(fs.deleteFile(Path{".."}), FileSystemException);
    CHECK_THROWS_AS(fs.deleteFile(Path{"dir1"}), FileSystemException);
    CHECK_THROWS_AS(fs.deleteFile(Path{"asdf.txt"}), FileSystemException);
    CHECK_THROWS_AS(fs.deleteFile(Path{"/dir1/asdf.txt"}), FileSystemException);

    fs.deleteFile(Path{"test.txt"});
    CHECK(fs.pathInfo(Path{"test.txt"}) == PathInfo::Unknown);

    fs.deleteFile(Path{"anotherDir/test3.map"});
    CHECK(fs.pathInfo(Path{"anotherDir/test3.map"}) == PathInfo::Unknown);

    fs.deleteFile(Path{"anotherDir/subDirTest/.././subDirTest/./test2.map"});
    CHECK(fs.pathInfo(Path{"anotherDir/subDirTest/test2.map"}) == PathInfo::Unknown);
  }

  SECTION("moveFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir(), false};

#if defined _WIN32
    CHECK_THROWS_AS(
      fs.moveFile(Path{"c:\\hopefully_nothing_here.txt"}, Path{"dest.txt"}, false),
      FileSystemException);
    CHECK_THROWS_AS(
      fs.moveFile(Path{"test.txt"}, Path{"C:\\dest.txt"}, false), FileSystemException);
#else
    CHECK_THROWS_AS(
      fs.moveFile(Path{"/hopefully_nothing_here.txt"}, Path{"dest.txt"}, false),
      FileSystemException);
    CHECK_THROWS_AS(
      fs.moveFile(Path{"test.txt"}, Path{"/dest.txt"}, false), FileSystemException);
#endif

    CHECK_THROWS_AS(
      fs.moveFile(Path{"test.txt"}, Path{"test2.map"}, false), FileSystemException);
    CHECK_THROWS_AS(
      fs.moveFile(Path{"test.txt"}, Path{"anotherDir/test3.map"}, false),
      FileSystemException);
    CHECK_THROWS_AS(
      fs.moveFile(Path{"test.txt"}, Path{"anotherDir/../anotherDir/./test3.map"}, false),
      FileSystemException);

    fs.moveFile(Path{"test.txt"}, Path{"test2.txt"}, true);
    CHECK(fs.pathInfo(Path{"test.txt"}) == PathInfo::Unknown);
    CHECK(fs.pathInfo(Path{"test2.txt"}) == PathInfo::File);

    fs.moveFile(Path{"test2.txt"}, Path{"test2.map"}, true);
    CHECK(fs.pathInfo(Path{"test2.txt"}) == PathInfo::Unknown);
    CHECK(fs.pathInfo(Path{"test2.map"}) == PathInfo::File);
    // we're trusting that the file is actually overwritten (should really test the
    // contents here...)

    fs.moveFile(Path{"test2.map"}, Path{"dir1/test2.map"}, true);
    CHECK(fs.pathInfo(Path{"test2.map"}) == PathInfo::Unknown);
    CHECK(fs.pathInfo(Path{"dir1/test2.map"}) == PathInfo::File);
  }

  SECTION("copyFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir(), false};

#if defined _WIN32
    CHECK_THROWS_AS(
      fs.copyFile(Path{"c:\\hopefully_nothing_here.txt"}, Path{"dest.txt"}, false),
      FileSystemException);
    CHECK_THROWS_AS(
      fs.copyFile(Path{"test.txt"}, Path{"C:\\dest.txt"}, false), FileSystemException);
#else
    CHECK_THROWS_AS(
      fs.copyFile(Path{"/hopefully_nothing_here.txt"}, Path{"dest.txt"}, false),
      FileSystemException);
    CHECK_THROWS_AS(
      fs.copyFile(Path{"test.txt"}, Path{"/dest.txt"}, false), FileSystemException);
#endif

    CHECK_THROWS_AS(
      fs.copyFile(Path{"test.txt"}, Path{"test2.map"}, false), FileSystemException);
    CHECK_THROWS_AS(
      fs.copyFile(Path{"test.txt"}, Path{"anotherDir/test3.map"}, false),
      FileSystemException);
    CHECK_THROWS_AS(
      fs.copyFile(Path{"test.txt"}, Path{"anotherDir/../anotherDir/./test3.map"}, false),
      FileSystemException);

    fs.copyFile(Path{"test.txt"}, Path{"test2.txt"}, true);
    CHECK(fs.pathInfo(Path{"test.txt"}) == PathInfo::File);
    CHECK(fs.pathInfo(Path{"test2.txt"}) == PathInfo::File);

    fs.copyFile(Path{"test2.txt"}, Path{"test2.map"}, true);
    CHECK(fs.pathInfo(Path{"test2.txt"}) == PathInfo::File);
    CHECK(fs.pathInfo(Path{"test2.map"}) == PathInfo::File);
    // we're trusting that the file is actually overwritten (should really test the
    // contents here...)

    fs.copyFile(Path{"test2.map"}, Path{"dir1/test2.map"}, true);
    CHECK(fs.pathInfo(Path{"test2.map"}) == PathInfo::File);
    CHECK(fs.pathInfo(Path{"dir1/test2.map"}) == PathInfo::File);
  }
}

} // namespace TrenchBroom::IO
