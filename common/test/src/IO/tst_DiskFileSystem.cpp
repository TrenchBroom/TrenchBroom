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
#include "IO/TraversalMode.h"
#include "Macros.h"
#include "Matchers.h"

#include "kdl/regex_utils.h"

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

  SECTION("find")
  {
#if defined _WIN32
    CHECK(
      fs.find("c:\\", TraversalMode::Flat)
      == kdl::result<std::vector<std::filesystem::path>, Error>{
        Error{"Path is absolute: 'c:\\'"}});
#else
    CHECK(
      fs.find("/", TraversalMode::Flat)
      == kdl::result<std::vector<std::filesystem::path>, Error>{
        Error{"Path is absolute: '/'"}});
#endif
    CHECK(
      fs.find("..", TraversalMode::Flat)
      == kdl::result<std::vector<std::filesystem::path>, Error>{
        Error{"Path does not denote a directory: '..'"}});
    CHECK(
      fs.find("asdf/bleh", TraversalMode::Flat)
      == kdl::result<std::vector<std::filesystem::path>, Error>{
        Error{"Path does not denote a directory: 'asdf/bleh'"}});

    CHECK(
      fs.find(".", TraversalMode::Flat)
      == kdl::result<std::vector<std::filesystem::path>, Error>{
        std::vector<std::filesystem::path>{
          "anotherDir",
          "dir1",
          "dir2",
          "test.txt",
          "test2.map",
        }});

    CHECK(
      fs.find("anotherDir", TraversalMode::Flat)
      == kdl::result<std::vector<std::filesystem::path>, Error>{
        std::vector<std::filesystem::path>{
          "anotherDir/subDirTest",
          "anotherDir/test3.map",
        }});

    CHECK(
      fs.find(".", TraversalMode::Recursive)
      == kdl::result<std::vector<std::filesystem::path>, Error>{
        std::vector<std::filesystem::path>{
          "anotherDir",
          "anotherDir/subDirTest",
          "anotherDir/subDirTest/test2.map",
          "anotherDir/test3.map",
          "dir1",
          "dir2",
          "test.txt",
          "test2.map",
        }});
  }

  SECTION("openFile")
  {
#if defined _WIN32
    CHECK(
      fs.openFile("c:\\hopefully_nothing.here")
      == kdl::result<std::shared_ptr<File>, Error>{
        Error{"Path is absolute: 'c:\\hopefully_nothing.here'"}});
#else
    CHECK(
      fs.openFile("/hopefully_nothing.here")
      == kdl::result<std::shared_ptr<File>, Error>{
        Error{"Path is absolute: '/hopefully_nothing.here'"}});
#endif
    CHECK(
      fs.openFile("..")
      == kdl::result<std::shared_ptr<File>, Error>{Error{"File not found: '..'"}});
    CHECK(
      fs.openFile(".")
      == kdl::result<std::shared_ptr<File>, Error>{Error{"File not found: '.'"}});
    CHECK(
      fs.openFile("anotherDir")
      == kdl::result<std::shared_ptr<File>, Error>{
        Error{"File not found: 'anotherDir'"}});

    const auto checkOpenFile = [&](const auto& path) {
      const auto file = fs.openFile(path).value();
      const auto expected = Disk::openFile(env.dir() / path).value();
      CHECK(
        file->reader().readString(file->size())
        == expected->reader().readString(expected->size()));
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
      == kdl::result<bool, Error>{
        Error{"Path is absolute: 'c:\\hopefully_nothing_here'"}});
#else
    CHECK(
      fs.createDirectory("/hopefully_nothing_here")
      == kdl::result<bool, Error>{Error{"Path is absolute: '/hopefully_nothing_here'"}});
#endif
    CHECK(
      fs.createDirectory("..")
      == kdl::result<bool, Error>{Error{"Cannot make absolute path of '..'"}});
    CHECK_THAT(
      fs.createDirectory("test.txt"),
      MatchesAnyOf({
        // macOS
        kdl::result<bool, Error>{Error{"Could not create directory: File exists"}},
        // Linux
        kdl::result<bool, Error>{Error{"Could not create directory: Not a directory"}},
        // Windows
        kdl::result<bool, Error>{Error{"Could not create directory: Cannot create a file "
                                       "when that file already exists."}},
      }));

    CHECK(fs.createDirectory("") == kdl::result<bool, Error>{false});
    CHECK(fs.createDirectory(".") == kdl::result<bool, Error>{false});
    CHECK(fs.createDirectory("dir1") == kdl::result<bool, Error>{false});

    CHECK(fs.createDirectory("newDir") == kdl::result<bool, Error>{true});
    CHECK(fs.pathInfo("newDir") == PathInfo::Directory);

    CHECK(fs.createDirectory("newDir/someOtherDir") == kdl::result<bool, Error>{true});
    CHECK(fs.pathInfo("newDir/someOtherDir") == PathInfo::Directory);

    CHECK(
      fs.createDirectory("someDir/someOtherDir/.././yetAnotherDir")
      == kdl::result<bool, Error>{true});
    CHECK(fs.pathInfo("someDir/someOtherDir/.././yetAnotherDir") == PathInfo::Directory);
  }

  SECTION("deleteFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir()};

#if defined _WIN32
    CHECK(
      fs.deleteFile("c:\\hopefully_nothing_here.txt")
      == kdl::result<bool, Error>{
        Error{"Path is absolute: 'c:\\hopefully_nothing_here.txt'"}});
    CHECK(
      fs.deleteFile("c:\\dir1\\asdf.txt")
      == kdl::result<bool, Error>{Error{"Path is absolute: 'c:\\dir1\\asdf.txt'"}});
#else
    CHECK(
      fs.deleteFile("/hopefully_nothing_here.txt")
      == kdl::result<bool, Error>{
        Error{"Path is absolute: '/hopefully_nothing_here.txt'"}});
#endif
    CHECK(
      fs.deleteFile("")
      == kdl::result<bool, Error>{Error{
        "Could not delete file '" + (env.dir()).string() + "': path is a directory"}});
    CHECK(
      fs.deleteFile(".")
      == kdl::result<bool, Error>{Error{
        "Could not delete file '" + (env.dir() / "").string()
        + "': path is a directory"}});
    CHECK(
      fs.deleteFile("..")
      == kdl::result<bool, Error>{Error{"Cannot make absolute path of '..'"}});
    CHECK(
      fs.deleteFile("dir1")
      == kdl::result<bool, Error>{Error{
        "Could not delete file '" + (env.dir() / "dir1").string()
        + "': path is a directory"}});

    CHECK(fs.deleteFile("asdf.txt") == kdl::result<bool, Error>{false});
    CHECK(fs.deleteFile("test.txt") == kdl::result<bool, Error>{true});
    CHECK(fs.pathInfo("test.txt") == PathInfo::Unknown);

    CHECK(fs.deleteFile("anotherDir/test3.map") == kdl::result<bool, Error>{true});
    CHECK(fs.pathInfo("anotherDir/test3.map") == PathInfo::Unknown);

    CHECK(
      fs.deleteFile("anotherDir/subDirTest/.././subDirTest/./test2.map")
      == kdl::result<bool, Error>{true});
    CHECK(fs.pathInfo("anotherDir/subDirTest/test2.map") == PathInfo::Unknown);
  }

  SECTION("moveFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir()};

#if defined _WIN32
    CHECK(
      fs.moveFile("c:\\hopefully_nothing_here.txt", "dest.txt")
      == kdl::result<void, Error>{
        Error{"Source path is absolute: 'c:\\hopefully_nothing_here.txt'"}});
    CHECK(
      fs.moveFile("test.txt", "C:\\dest.txt")
      == kdl::result<void, Error>{Error{"Destination path is absolute: 'C:\\dest.txt'"}});
#else
    CHECK(
      fs.moveFile("/hopefully_nothing_here.txt", "dest.txt")
      == kdl::result<void, Error>{
        Error{"Source path is absolute: '/hopefully_nothing_here.txt'"}});
    CHECK(
      fs.moveFile("test.txt", "/dest.txt")
      == kdl::result<void, Error>{Error{"Destination path is absolute: '/dest.txt'"}});
#endif

    CHECK(fs.moveFile("test.txt", "test2.txt") == kdl::result<void, Error>{});
    CHECK(fs.pathInfo("test.txt") == PathInfo::Unknown);
    CHECK(fs.pathInfo("test2.txt") == PathInfo::File);

    CHECK(fs.moveFile("test2.txt", "test2.map") == kdl::result<void, Error>{});
    CHECK(fs.pathInfo("test2.txt") == PathInfo::Unknown);
    CHECK(fs.pathInfo("test2.map") == PathInfo::File);
    // we're trusting that the file is actually overwritten (should really test the
    // contents here...)

    CHECK(fs.moveFile("test2.map", "dir1/test2.map") == kdl::result<void, Error>{});
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
      == kdl::result<void, Error>{
        Error{"Source path is absolute: 'c:\\hopefully_nothing_here.txt'"}});
    CHECK(
      fs.copyFile("test.txt", "C:\\dest.txt")
      == kdl::result<void, Error>{Error{"Destination path is absolute: 'C:\\dest.txt'"}});
#else
    CHECK(
      fs.copyFile("/hopefully_nothing_here.txt", "dest.txt")
      == kdl::result<void, Error>{
        Error{"Source path is absolute: '/hopefully_nothing_here.txt'"}});
    CHECK(
      fs.copyFile("test.txt", "/dest.txt")
      == kdl::result<void, Error>{Error{"Destination path is absolute: '/dest.txt'"}});
#endif

    CHECK(fs.copyFile("test.txt", "test2.txt") == kdl::result<void, Error>{});
    CHECK(fs.pathInfo("test.txt") == PathInfo::File);
    CHECK(fs.pathInfo("test2.txt") == PathInfo::File);

    CHECK(fs.copyFile("test2.txt", "test2.map") == kdl::result<void, Error>{});
    CHECK(fs.pathInfo("test2.txt") == PathInfo::File);
    CHECK(fs.pathInfo("test2.map") == PathInfo::File);
    // we're trusting that the file is actually overwritten (should really test the
    // contents here...)

    CHECK(fs.copyFile("test2.map", "dir1/test2.map") == kdl::result<void, Error>{});
    CHECK(fs.pathInfo("test2.map") == PathInfo::File);
    CHECK(fs.pathInfo("dir1/test2.map") == PathInfo::File);
  }
}

} // namespace TrenchBroom::IO
