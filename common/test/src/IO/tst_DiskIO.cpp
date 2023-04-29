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
#include "IO/Path.h"
#include "IO/PathInfo.h"
#include "IO/PathQt.h"
#include "IO/TestEnvironment.h"
#include "Macros.h"

#include <algorithm>

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

TEST_CASE("DiskIO")
{
  const auto env = makeTestEnvironment();

  SECTION("fixPath")
  {
    CHECK(Disk::fixPath(Path{"asdf/blah"}) == Path{"asdf/blah"});
    CHECK(Disk::fixPath(Path{"/../../test"}) == Path{"/test"});

    if (Disk::isCaseSensitive())
    {
      CHECK(Disk::fixPath(env.dir() + Path{"TEST.txt"}) == env.dir() + Path{"test.txt"});
      CHECK(
        Disk::fixPath(env.dir() + Path{"anotHERDIR/./SUBdirTEST/../SubdirTesT/TesT2.MAP"})
        == env.dir() + Path{"anotherDir/subDirTest/test2.map"});
    }
  }

  SECTION("pathInfo")
  {
    CHECK(Disk::pathInfo(Path{"asdf/bleh"}) == PathInfo::Unknown);
    CHECK(Disk::pathInfo(env.dir() + Path{"anotherDir/asdf.map"}) == PathInfo::Unknown);
    CHECK(
      Disk::pathInfo(env.dir() + Path{"anotherDir/test3.map/asdf"}) == PathInfo::Unknown);

    CHECK(Disk::pathInfo(env.dir() + Path{"anotherDir"}) == PathInfo::Directory);
    CHECK(Disk::pathInfo(env.dir() + Path{"ANOTHERDIR"}) == PathInfo::Directory);
    CHECK(
      Disk::pathInfo(env.dir() + Path{"anotherDir/subDirTest"}) == PathInfo::Directory);

    CHECK(Disk::pathInfo(env.dir() + Path{"anotherDir/test3.map"}) == PathInfo::File);
    CHECK(Disk::pathInfo(env.dir() + Path{"anotherDir/TEST3.map"}) == PathInfo::File);
    CHECK(
      Disk::pathInfo(env.dir() + Path{"anotherDir/subDirTest/test2.map"})
      == PathInfo::File);
  }

  SECTION("directoryContents")
  {
    CHECK_THROWS_AS(Disk::directoryContents(Path{"asdf/bleh"}), FileSystemException);
    CHECK_THROWS_AS(
      Disk::directoryContents(env.dir() + Path{"does/not/exist"}), FileSystemException);

    CHECK_THAT(
      Disk::directoryContents(env.dir()),
      Catch::UnorderedEquals(std::vector<Path>{
        Path{"dir1"},
        Path{"dir2"},
        Path{"anotherDir"},
        Path{"test.txt"},
        Path{"test2.map"},
      }));
  }

  SECTION("openFile")
  {

    CHECK_THROWS_AS(Disk::openFile(Path{"asdf/bleh"}), FileNotFoundException);
    CHECK_THROWS_AS(
      Disk::openFile(env.dir() + Path{"does/not/exist"}), FileNotFoundException);

    CHECK_THROWS_AS(
      Disk::openFile(env.dir() + Path{"does_not_exist.txt"}), FileNotFoundException);
    CHECK(Disk::openFile(env.dir() + Path{"test.txt"}) != nullptr);
    CHECK(Disk::openFile(env.dir() + Path{"anotherDir/subDirTest/test2.map"}) != nullptr);
  }

  SECTION("resolvePath")
  {
    const auto rootPaths = std::vector<Path>{env.dir(), env.dir() + Path{"anotherDir"}};

    CHECK(Disk::resolvePath(rootPaths, Path{"test.txt"}) == env.dir() + Path{"test.txt"});
    CHECK(
      Disk::resolvePath(rootPaths, Path{"test3.map"})
      == env.dir() + Path{"anotherDir/test3.map"});
    CHECK(
      Disk::resolvePath(rootPaths, Path{"subDirTest/test2.map"})
      == env.dir() + Path{"anotherDir/subDirTest/test2.map"});
    CHECK(Disk::resolvePath(rootPaths, Path{"/asfd/blah"}) == Path{});
    CHECK(Disk::resolvePath(rootPaths, Path{"adk3kdk/bhb"}) == Path{});
  }
}
} // namespace TrenchBroom::IO
