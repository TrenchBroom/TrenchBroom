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

#include "IO/DiskFileSystem.h"
#include "Exceptions.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/Path.h"
#include "IO/PathQt.h"
#include "IO/TestEnvironment.h"
#include "Macros.h"

#include <algorithm>

#include <QFileInfo>
#include <QString>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
static TestEnvironment makeTestEnvironment()
{
  // have a non-ASCII character in the directory name to help catch
  // filename encoding bugs
  const auto hiraganaLetterSmallA = QString(static_cast<QChar>(0x3041));
  const auto dir = (QString::fromStdString(Catch::getResultCapture().getCurrentTestName())
                    + hiraganaLetterSmallA)
                     .toStdString();

  return TestEnvironment{
    dir, [](TestEnvironment& env) {
      env.createDirectory(Path("dir1"));
      env.createDirectory(Path("dir2"));
      env.createDirectory(Path("anotherDir"));
      env.createDirectory(Path("anotherDir/subDirTest"));

      env.createFile(Path("test.txt"), "some content");
      env.createFile(Path("test2.map"), "//test file\n{}");
      env.createFile(Path("anotherDir/subDirTest/test2.map"), "//sub dir test file\n{}");
      env.createFile(Path("anotherDir/test3.map"), "//yet another test file\n{}");
    }};
}

TEST_CASE("FileSystemTest.makeAbsolute")
{
  const auto env = makeTestEnvironment();

  auto fs = std::make_shared<DiskFileSystem>(env.dir() + Path("anotherDir"));
  fs = std::make_shared<DiskFileSystem>(fs, env.dir() + Path("dir1"));

  // Existing files should be resolved against the first file system in the chain that
  // contains them:
  const auto absPathExisting = fs->makeAbsolute(Path("test3.map"));
  CHECK(absPathExisting == env.dir() + Path("anotherDir/test3.map"));

  // Non existing files should be resolved against the first filesystem in the fs chain:
  const auto absPathNotExisting = fs->makeAbsolute(Path("asdf.map"));
  CHECK(absPathNotExisting == env.dir() + Path("dir1/asdf.map"));
}

TEST_CASE("DiskTest.fixPath")
{
  const auto env = makeTestEnvironment();

  CHECK_THROWS_AS(Disk::fixPath(Path("asdf/blah")), FileSystemException);
  CHECK_THROWS_AS(Disk::fixPath(Path("/../../test")), FileSystemException);
  if (Disk::isCaseSensitive())
  {
    // FIXME: behaviour should be made consistent between case-sensitive/case-insensitive
    // filesystems fixPath should probably also never throw?
    CHECK_THROWS_AS(
      Disk::fixPath(env.dir() + Path("anotherDir/test3.map/asdf")), FileSystemException);
    CHECK(
      env.dir() + Path("anotherDir/test3.map")
      == Disk::fixPath(env.dir() + Path("ANOTHERdir/TEST3.MAP")));
  }

  CHECK(
    env.dir() + Path("anotherDir/test3.map")
    == Disk::fixPath(env.dir() + Path("anotherDir/subDirTest/../test3.map")));

  // on case sensitive file systems, this should also work
  CHECK(
    QFileInfo::exists(IO::pathAsQString(Disk::fixPath(env.dir() + Path("TEST.txt")))));
  CHECK(QFileInfo::exists(IO::pathAsQString(
    Disk::fixPath(env.dir() + Path("anotHERDIR/./SUBdirTEST/../SubdirTesT/TesT2.MAP")))));
}

TEST_CASE("DiskTest.directoryExists")
{
  const auto env = makeTestEnvironment();

  CHECK_THROWS_AS(Disk::directoryExists(Path("asdf/bleh")), FileSystemException);
  if (Disk::isCaseSensitive())
  {
    // FIXME: behaviour should be made consistent between case-sensitive/case-insensitive
    // filesystems directoryExists should probably also never throw?
    CHECK_THROWS_AS(
      Disk::directoryExists(env.dir() + Path("anotherDir/test3.map/asdf")),
      FileSystemException); // test3.map is a file
  }

  CHECK(Disk::directoryExists(env.dir() + Path("anotherDir")));
  CHECK(Disk::directoryExists(env.dir() + Path("anotherDir/subDirTest")));
  CHECK(
    !Disk::directoryExists(env.dir() + Path("anotherDir/test3.map"))); // not a directory
  CHECK(!Disk::directoryExists(
    env.dir() + Path("anotherDir/asdf"))); // asdf directory doesn't exist
}

TEST_CASE("DiskTest.fileExists")
{
  const auto env = makeTestEnvironment();

  CHECK_THROWS_AS(Disk::fileExists(Path("asdf/bleh")), FileSystemException);

  CHECK(Disk::fileExists(env.dir() + Path("test.txt")));
  CHECK(Disk::fileExists(env.dir() + Path("anotherDir/subDirTest/test2.map")));
}

TEST_CASE("DiskTest.getDirectoryContents")
{
  const auto env = makeTestEnvironment();

  CHECK_THROWS_AS(Disk::getDirectoryContents(Path("asdf/bleh")), FileSystemException);
  CHECK_THROWS_AS(
    Disk::getDirectoryContents(env.dir() + Path("does/not/exist")), FileSystemException);

  CHECK_THAT(
    Disk::getDirectoryContents(env.dir()),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("dir1"),
      Path("dir2"),
      Path("anotherDir"),
      Path("test.txt"),
      Path("test2.map"),
    }));
}

TEST_CASE("DiskTest.openFile")
{
  const auto env = makeTestEnvironment();

  CHECK_THROWS_AS(Disk::openFile(Path("asdf/bleh")), FileSystemException);
  CHECK_THROWS_AS(
    Disk::openFile(env.dir() + Path("does/not/exist")), FileNotFoundException);

  CHECK_THROWS_AS(
    Disk::openFile(env.dir() + Path("does_not_exist.txt")), FileNotFoundException);
  CHECK(Disk::openFile(env.dir() + Path("test.txt")) != nullptr);
  CHECK(Disk::openFile(env.dir() + Path("anotherDir/subDirTest/test2.map")) != nullptr);
}

TEST_CASE("DiskTest.resolvePath")
{
  const auto env = makeTestEnvironment();

  std::vector<Path> rootPaths;
  rootPaths.push_back(env.dir());
  rootPaths.push_back(env.dir() + Path("anotherDir"));

  std::vector<Path> paths;
  paths.push_back(Path("test.txt"));
  paths.push_back(Path("test3.map"));
  paths.push_back(Path("subDirTest/test2.map"));
  paths.push_back(Path("/asfd/blah"));
  paths.push_back(Path("adk3kdk/bhb"));

  CHECK(Disk::resolvePath(rootPaths, paths[0]) == env.dir() + Path("test.txt"));
  CHECK(
    Disk::resolvePath(rootPaths, paths[1]) == env.dir() + Path("anotherDir/test3.map"));
  CHECK(
    Disk::resolvePath(rootPaths, paths[2])
    == env.dir() + Path("anotherDir/subDirTest/test2.map"));
  CHECK(Disk::resolvePath(rootPaths, paths[3]) == Path(""));
  CHECK(Disk::resolvePath(rootPaths, paths[4]) == Path(""));
}

TEST_CASE("DiskFileSystemTest.createDiskFileSystem")
{
  const auto env = makeTestEnvironment();

  CHECK_THROWS_AS(DiskFileSystem(env.dir() + Path("asdf"), true), FileSystemException);
  CHECK_NOTHROW(DiskFileSystem(env.dir() + Path("asdf"), false));
  CHECK_NOTHROW(DiskFileSystem(env.dir(), true));

  // for case sensitive file systems
  CHECK_NOTHROW(DiskFileSystem(env.dir() + Path("ANOTHERDIR"), true));

  const DiskFileSystem fs(env.dir() + Path("anotherDir/.."), true);
  CHECK(fs.makeAbsolute(Path("")) == fs.root());
}

TEST_CASE("DiskFileSystemTest.directoryExists")
{
  const auto env = makeTestEnvironment();
  const DiskFileSystem fs(env.dir());

#if defined _WIN32
  CHECK_THROWS_AS(fs.directoryExists(Path("c:\\")), FileSystemException);
#else
  CHECK_THROWS_AS(fs.directoryExists(Path("/")), FileSystemException);
#endif
  CHECK_THROWS_AS(fs.directoryExists(Path("..")), FileSystemException);

  CHECK(fs.directoryExists(Path(".")));
  CHECK(fs.directoryExists(Path("anotherDir")));
  CHECK(fs.directoryExists(Path("anotherDir/subDirTest")));
  CHECK(fs.directoryExists(Path("anotherDir/./subDirTest/..")));
  CHECK(fs.directoryExists(Path("ANOTHerDir")));
  CHECK_FALSE(fs.directoryExists(Path("test.txt")));
  CHECK_FALSE(fs.directoryExists(Path("fasdf")));
}

TEST_CASE("DiskFileSystemTest.fileExists")
{
  const auto env = makeTestEnvironment();
  const DiskFileSystem fs(env.dir());

#if defined _WIN32
  CHECK_THROWS_AS(
    fs.fileExists(Path("C:\\does_not_exist_i_hope.txt")), FileSystemException);
#else
  CHECK_THROWS_AS(fs.fileExists(Path("/does_not_exist_i_hope.txt")), FileSystemException);
#endif
  CHECK_THROWS_AS(fs.fileExists(Path("../test.txt")), FileSystemException);

  CHECK(fs.fileExists(Path("test.txt")));
  CHECK(fs.fileExists(Path("./test.txt")));
  CHECK(fs.fileExists(Path("anotherDir/test3.map")));
  CHECK(fs.fileExists(Path("anotherDir/./subDirTest/../subDirTest/test2.map")));
  CHECK(fs.fileExists(Path("ANOtherDir/test3.MAP")));
  CHECK_FALSE(fs.fileExists(Path("anotherDir/whatever.txt")));
  CHECK_FALSE(fs.fileExists(Path("fdfdf.blah")));
}

TEST_CASE("DiskFileSystemTest.getDirectoryContents")
{
  const auto env = makeTestEnvironment();
  const DiskFileSystem fs(env.dir());

  CHECK_THROWS_AS(fs.getDirectoryContents(Path("asdf/bleh")), FileSystemException);

  CHECK_THAT(
    fs.getDirectoryContents(Path("anotherDir")),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("subDirTest"),
      Path("test3.map"),
    }));
}

TEST_CASE("DiskFileSystemTest.findItems")
{
  const auto env = makeTestEnvironment();
  const DiskFileSystem fs(env.dir());

#if defined _WIN32
  CHECK_THROWS_AS(fs.findItems(Path("c:\\")), FileSystemException);
#else
  CHECK_THROWS_AS(fs.findItems(Path("/")), FileSystemException);
#endif
  CHECK_THROWS_AS(fs.findItems(Path("..")), FileSystemException);

  CHECK_THAT(
    fs.findItems(Path(".")),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("./dir1"),
      Path("./dir2"),
      Path("./anotherDir"),
      Path("./test.txt"),
      Path("./test2.map"),
    }));

  CHECK_THAT(
    fs.findItems(Path(""), FileExtensionMatcher("TXT")),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("test.txt"),
    }));

  CHECK_THAT(
    fs.findItems(Path("anotherDir")),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("anotherDir/subDirTest"),
      Path("anotherDir/test3.map"),
    }));
}

TEST_CASE("DiskFileSystemTest.findItemsRecursively")
{
  const auto env = makeTestEnvironment();
  const DiskFileSystem fs(env.dir());

#if defined _WIN32
  CHECK_THROWS_AS(fs.findItemsRecursively(Path("c:\\")), FileSystemException);
#else
  CHECK_THROWS_AS(fs.findItemsRecursively(Path("/")), FileSystemException);
#endif
  CHECK_THROWS_AS(fs.findItemsRecursively(Path("..")), FileSystemException);

  CHECK_THAT(
    fs.findItemsRecursively(Path(".")),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("./dir1"),
      Path("./dir2"),
      Path("./anotherDir"),
      Path("./anotherDir/subDirTest"),
      Path("./anotherDir/subDirTest/test2.map"),
      Path("./anotherDir/test3.map"),
      Path("./test.txt"),
      Path("./test2.map"),
    }));

  CHECK_THAT(
    fs.findItemsRecursively(Path(""), FileExtensionMatcher("MAP")),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("anotherDir/subDirTest/test2.map"),
      Path("anotherDir/test3.map"),
      Path("test2.map"),
    }));

  CHECK_THAT(
    fs.findItemsRecursively(Path("anotherDir")),
    Catch::UnorderedEquals(std::vector<Path>{
      Path("anotherDir/subDirTest"),
      Path("anotherDir/subDirTest/test2.map"),
      Path("anotherDir/test3.map"),
    }));
}

// getDirectoryContents gets tested thoroughly by the tests for the find* methods

TEST_CASE("DiskFileSystemTest.openFile")
{
  const auto env = makeTestEnvironment();
  const DiskFileSystem fs(env.dir());

#if defined _WIN32
  CHECK_THROWS_AS(fs.openFile(Path("c:\\hopefully_nothing.here")), FileSystemException);
#else
  CHECK_THROWS_AS(fs.openFile(Path("/hopefully_nothing.here")), FileSystemException);
#endif
  CHECK_THROWS_AS(fs.openFile(Path("..")), FileSystemException);
  CHECK_THROWS_AS(fs.openFile(Path(".")), FileSystemException);
  CHECK_THROWS_AS(fs.openFile(Path("anotherDir")), FileSystemException);

  const auto checkOpenFile = [&](const auto& path) {
    const auto file = fs.openFile(path);
    CHECK(file != nullptr);
    CHECK(file->path() == path);
  };

  checkOpenFile(Path("test.txt"));
  checkOpenFile(Path("anotherDir/test3.map"));
  checkOpenFile(Path("anotherDir/../anotherDir/./test3.map"));
}

TEST_CASE("WritableDiskFileSystemTest.createWritableDiskFileSystem")
{
  const auto env = makeTestEnvironment();

  CHECK_THROWS_AS(
    WritableDiskFileSystem(env.dir() + Path("asdf"), false), FileSystemException);
  CHECK_NOTHROW(WritableDiskFileSystem(env.dir() + Path("asdf"), true));
  CHECK_NOTHROW(WritableDiskFileSystem(env.dir(), true));

  // for case sensitive file systems
  CHECK_NOTHROW(WritableDiskFileSystem(env.dir() + Path("ANOTHERDIR"), false));

  const WritableDiskFileSystem fs(env.dir() + Path("anotherDir/.."), false);
  CHECK(fs.makeAbsolute(Path("")) == env.dir());
}

TEST_CASE("WritableDiskFileSystemTest.createDirectory")
{
  const auto env = makeTestEnvironment();
  WritableDiskFileSystem fs(env.dir(), false);

#if defined _WIN32
  CHECK_THROWS_AS(
    fs.createDirectory(Path("c:\\hopefully_nothing_here")), FileSystemException);
#else
  CHECK_THROWS_AS(
    fs.createDirectory(Path("/hopefully_nothing_here")), FileSystemException);
#endif
  CHECK_THROWS_AS(fs.createDirectory(Path("")), FileSystemException);
  CHECK_THROWS_AS(fs.createDirectory(Path(".")), FileSystemException);
  CHECK_THROWS_AS(fs.createDirectory(Path("..")), FileSystemException);
  CHECK_THROWS_AS(fs.createDirectory(Path("dir1")), FileSystemException);
  CHECK_THROWS_AS(fs.createDirectory(Path("test.txt")), FileSystemException);

  fs.createDirectory(Path("newDir"));
  CHECK(fs.directoryExists(Path("newDir")));

  fs.createDirectory(Path("newDir/someOtherDir"));
  CHECK(fs.directoryExists(Path("newDir/someOtherDir")));

  fs.createDirectory(Path("newDir/someOtherDir/.././yetAnotherDir/."));
  CHECK(fs.directoryExists(Path("newDir/yetAnotherDir")));
}

TEST_CASE("WritableDiskFileSystemTest.deleteFile")
{
  const auto env = makeTestEnvironment();
  WritableDiskFileSystem fs(env.dir(), false);

#if defined _WIN32
  CHECK_THROWS_AS(
    fs.deleteFile(Path("c:\\hopefully_nothing_here.txt")), FileSystemException);
#else
  CHECK_THROWS_AS(
    fs.deleteFile(Path("/hopefully_nothing_here.txt")), FileSystemException);
#endif
  CHECK_THROWS_AS(fs.deleteFile(Path("")), FileSystemException);
  CHECK_THROWS_AS(fs.deleteFile(Path(".")), FileSystemException);
  CHECK_THROWS_AS(fs.deleteFile(Path("..")), FileSystemException);
  CHECK_THROWS_AS(fs.deleteFile(Path("dir1")), FileSystemException);
  CHECK_THROWS_AS(fs.deleteFile(Path("asdf.txt")), FileSystemException);
  CHECK_THROWS_AS(fs.deleteFile(Path("/dir1/asdf.txt")), FileSystemException);

  fs.deleteFile(Path("test.txt"));
  CHECK_FALSE(fs.fileExists(Path("test.txt")));

  fs.deleteFile(Path("anotherDir/test3.map"));
  CHECK_FALSE(fs.fileExists(Path("anotherDir/test3.map")));

  fs.deleteFile(Path("anotherDir/subDirTest/.././subDirTest/./test2.map"));
  CHECK_FALSE(fs.fileExists(Path("anotherDir/subDirTest/test2.map")));
}

TEST_CASE("WritableDiskFileSystemTest.moveFile")
{
  const auto env = makeTestEnvironment();
  WritableDiskFileSystem fs(env.dir(), false);

#if defined _WIN32
  CHECK_THROWS_AS(
    fs.moveFile(Path("c:\\hopefully_nothing_here.txt"), Path("dest.txt"), false),
    FileSystemException);
  CHECK_THROWS_AS(
    fs.moveFile(Path("test.txt"), Path("C:\\dest.txt"), false), FileSystemException);
#else
  CHECK_THROWS_AS(
    fs.moveFile(Path("/hopefully_nothing_here.txt"), Path("dest.txt"), false),
    FileSystemException);
  CHECK_THROWS_AS(
    fs.moveFile(Path("test.txt"), Path("/dest.txt"), false), FileSystemException);
#endif

  CHECK_THROWS_AS(
    fs.moveFile(Path("test.txt"), Path("test2.map"), false), FileSystemException);
  CHECK_THROWS_AS(
    fs.moveFile(Path("test.txt"), Path("anotherDir/test3.map"), false),
    FileSystemException);
  CHECK_THROWS_AS(
    fs.moveFile(Path("test.txt"), Path("anotherDir/../anotherDir/./test3.map"), false),
    FileSystemException);

  fs.moveFile(Path("test.txt"), Path("test2.txt"), true);
  CHECK_FALSE(fs.fileExists(Path("test.txt")));
  CHECK(fs.fileExists(Path("test2.txt")));

  fs.moveFile(Path("test2.txt"), Path("test2.map"), true);
  CHECK_FALSE(fs.fileExists(Path("test2.txt")));
  CHECK(fs.fileExists(Path("test2.map")));
  // we're trusting that the file is actually overwritten (should really test the contents
  // here...)

  fs.moveFile(Path("test2.map"), Path("dir1/test2.map"), true);
  CHECK_FALSE(fs.fileExists(Path("test2.map")));
  CHECK(fs.fileExists(Path("dir1/test2.map")));
}

TEST_CASE("WritableDiskFileSystemTest.copyFile")
{
  const auto env = makeTestEnvironment();
  WritableDiskFileSystem fs(env.dir(), false);

#if defined _WIN32
  CHECK_THROWS_AS(
    fs.copyFile(Path("c:\\hopefully_nothing_here.txt"), Path("dest.txt"), false),
    FileSystemException);
  CHECK_THROWS_AS(
    fs.copyFile(Path("test.txt"), Path("C:\\dest.txt"), false), FileSystemException);
#else
  CHECK_THROWS_AS(
    fs.copyFile(Path("/hopefully_nothing_here.txt"), Path("dest.txt"), false),
    FileSystemException);
  CHECK_THROWS_AS(
    fs.copyFile(Path("test.txt"), Path("/dest.txt"), false), FileSystemException);
#endif

  CHECK_THROWS_AS(
    fs.copyFile(Path("test.txt"), Path("test2.map"), false), FileSystemException);
  CHECK_THROWS_AS(
    fs.copyFile(Path("test.txt"), Path("anotherDir/test3.map"), false),
    FileSystemException);
  CHECK_THROWS_AS(
    fs.copyFile(Path("test.txt"), Path("anotherDir/../anotherDir/./test3.map"), false),
    FileSystemException);

  fs.copyFile(Path("test.txt"), Path("test2.txt"), true);
  CHECK(fs.fileExists(Path("test.txt")));
  CHECK(fs.fileExists(Path("test2.txt")));

  fs.copyFile(Path("test2.txt"), Path("test2.map"), true);
  CHECK(fs.fileExists(Path("test2.txt")));
  CHECK(fs.fileExists(Path("test2.map")));
  // we're trusting that the file is actually overwritten (should really test the contents
  // here...)

  fs.copyFile(Path("test2.map"), Path("dir1/test2.map"), true);
  CHECK(fs.fileExists(Path("test2.map")));
  CHECK(fs.fileExists(Path("dir1/test2.map")));
}
} // namespace IO
} // namespace TrenchBroom
