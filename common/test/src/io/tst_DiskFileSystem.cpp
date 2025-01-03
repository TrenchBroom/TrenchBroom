/*
 Copyright (C) 2010 Kristian Duske

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

#include "io/DiskFileSystem.h"
#include "io/DiskIO.h"
#include "io/File.h"
#include "io/PathInfo.h"
#include "io/TestEnvironment.h"
#include "io/TraversalMode.h"

#include <filesystem>

#include "catch/Matchers.h"

#include "Catch2.h"

namespace tb::io
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
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"c:\\"}))}});
#else
    CHECK(
      fs.find("/", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{
        fmt::format("Path {} is absolute", fmt::streamed(std::filesystem::path{"/"}))}});
#endif
    CHECK(
      fs.find("..", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Path {} does not denote a directory",
        fmt::streamed(std::filesystem::path{".."}))}});
    CHECK(
      fs.find("asdf/bleh", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Path {} does not denote a directory",
        fmt::streamed(std::filesystem::path{"asdf/bleh"}))}});

    CHECK_THAT(
      fs.find(".", TraversalMode::Flat),
      MatchesPathsResult({
        "anotherDir",
        "dir1",
        "dir2",
        "test.txt",
        "test2.map",
      }));

    CHECK_THAT(
      fs.find("anotherDir", TraversalMode::Flat),
      MatchesPathsResult({
        "anotherDir/subDirTest",
        "anotherDir/test3.map",
      }));

    CHECK_THAT(
      fs.find(".", TraversalMode::Recursive),
      MatchesPathsResult({
        "anotherDir",
        "anotherDir/subDirTest",
        "anotherDir/subDirTest/test2.map",
        "anotherDir/test3.map",
        "dir1",
        "dir2",
        "test.txt",
        "test2.map",
      }));

    CHECK_THAT(
      fs.find(".", TraversalMode::Recursive),
      MatchesPathsResult({
        "anotherDir",
        "anotherDir/subDirTest",
        "anotherDir/subDirTest/test2.map",
        "anotherDir/test3.map",
        "dir1",
        "dir2",
        "test.txt",
        "test2.map",
      }));
  }

  SECTION("openFile")
  {
#if defined _WIN32
    CHECK(
      fs.openFile("c:\\hopefully_nothing.here")
      == Result<std::shared_ptr<File>>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"c:\\hopefully_nothing.here"}))}});
#else
    CHECK(
      fs.openFile("/hopefully_nothing.here")
      == Result<std::shared_ptr<File>>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"/hopefully_nothing.here"}))}});
#endif
    CHECK(
      fs.openFile("..")
      == Result<std::shared_ptr<File>>{
        Error{fmt::format("{} not found", fmt::streamed(std::filesystem::path{".."}))}});
    CHECK(
      fs.openFile(".")
      == Result<std::shared_ptr<File>>{
        Error{fmt::format("{} not found", fmt::streamed(std::filesystem::path{"."}))}});
    CHECK(
      fs.openFile("anotherDir")
      == Result<std::shared_ptr<File>>{Error{fmt::format(
        "{} not found", fmt::streamed(std::filesystem::path{"anotherDir"}))}});

    const auto checkOpenFile = [&](const auto& path) {
      const auto file = fs.openFile(path) | kdl::value();
      const auto expected = Disk::openFile(env.dir() / path) | kdl::value();
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
  } // namespace tb::io

  SECTION("createDirectory")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir()};

#if defined _WIN32
    CHECK(
      fs.createDirectory("c:\\hopefully_nothing_here")
      == Result<bool>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"c:\\hopefully_nothing_here"}))}});
#else
    CHECK(
      fs.createDirectory("/hopefully_nothing_here")
      == Result<bool>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"/hopefully_nothing_here"}))}});
#endif
    CHECK(
      fs.createDirectory("..")
      == Result<bool>{Error{fmt::format(
        "Failed to make absolute path of {}",
        fmt::streamed(std::filesystem::path{".."}))}});
    CHECK(
      fs.createDirectory("test.txt")
      == Result<bool>{Error{fmt::format(
        "Failed to create {}: path denotes a file",
        fmt::streamed(env.dir() / "test.txt"))}});

    CHECK(fs.createDirectory("") == Result<bool>{false});
    CHECK(fs.createDirectory(".") == Result<bool>{false});
    CHECK(fs.createDirectory("dir1") == Result<bool>{false});

    CHECK(fs.createDirectory("newDir") == Result<bool>{true});
    CHECK(fs.pathInfo("newDir") == PathInfo::Directory);

    CHECK(fs.createDirectory("newDir/someOtherDir") == Result<bool>{true});
    CHECK(fs.pathInfo("newDir/someOtherDir") == PathInfo::Directory);

    CHECK(
      fs.createDirectory("someDir/someOtherDir/.././yetAnotherDir")
      == Result<bool>{true});
    CHECK(fs.pathInfo("someDir/someOtherDir/.././yetAnotherDir") == PathInfo::Directory);
  }

  SECTION("deleteFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir()};

#if defined _WIN32
    CHECK(
      fs.deleteFile("c:\\hopefully_nothing_here.txt")
      == Result<bool>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"c:\\hopefully_nothing_here.txt"}))}});
    CHECK(
      fs.deleteFile("c:\\dir1\\asdf.txt")
      == Result<bool>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"c:\\dir1\\asdf.txt"}))}});
#else
    CHECK(
      fs.deleteFile("/hopefully_nothing_here.txt")
      == Result<bool>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"/hopefully_nothing_here.txt"}))}});
#endif
    CHECK(
      fs.deleteFile("")
      == Result<bool>{Error{fmt::format(
        "Failed to delete {}: path denotes a directory", fmt::streamed(env.dir()))}});
    CHECK(
      fs.deleteFile(".")
      == Result<bool>{Error{fmt::format(
        "Failed to delete {}: path denotes a directory",
        fmt::streamed(env.dir() / "."))}});
    CHECK(
      fs.deleteFile("..")
      == Result<bool>{Error{fmt::format(
        "Failed to make absolute path of {}",
        fmt::streamed(std::filesystem::path{".."}))}});
    CHECK(
      fs.deleteFile("dir1")
      == Result<bool>{Error{fmt::format(
        "Failed to delete {}: path denotes a directory",
        fmt::streamed(env.dir() / "dir1"))}});

    CHECK(fs.deleteFile("asdf.txt") == Result<bool>{false});
    CHECK(fs.deleteFile("test.txt") == Result<bool>{true});
    CHECK(fs.pathInfo("test.txt") == PathInfo::Unknown);

    CHECK(fs.deleteFile("anotherDir/test3.map") == Result<bool>{true});
    CHECK(fs.pathInfo("anotherDir/test3.map") == PathInfo::Unknown);

    CHECK(
      fs.deleteFile("anotherDir/subDirTest/.././subDirTest/./test2.map")
      == Result<bool>{true});
    CHECK(fs.pathInfo("anotherDir/subDirTest/test2.map") == PathInfo::Unknown);
  }

  SECTION("moveFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir()};

#if defined _WIN32
    CHECK(
      fs.moveFile("c:\\hopefully_nothing_here.txt", "dest.txt")
      == Result<void>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"c:\\hopefully_nothing_here.txt"}))}});
    CHECK(
      fs.moveFile("test.txt", "C:\\dest.txt")
      == Result<void>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"C:\\dest.txt"}))}});
#else
    CHECK(
      fs.moveFile("/hopefully_nothing_here.txt", "dest.txt")
      == Result<void>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"/hopefully_nothing_here.txt"}))}});
    CHECK(
      fs.moveFile("test.txt", "/dest.txt")
      == Result<void>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"/dest.txt"}))}});
#endif

    CHECK(fs.moveFile("test.txt", "test2.txt") == Result<void>{});
    CHECK(fs.pathInfo("test.txt") == PathInfo::Unknown);
    CHECK(fs.pathInfo("test2.txt") == PathInfo::File);

    CHECK(fs.moveFile("test2.txt", "test2.map") == Result<void>{});
    CHECK(fs.pathInfo("test2.txt") == PathInfo::Unknown);
    CHECK(fs.pathInfo("test2.map") == PathInfo::File);
    // we're trusting that the file is actually overwritten (should really test the
    // contents here...)

    CHECK(fs.moveFile("test2.map", "dir1/test2.map") == Result<void>{});
    CHECK(fs.pathInfo("test2.map") == PathInfo::Unknown);
    CHECK(fs.pathInfo("dir1/test2.map") == PathInfo::File);
  }

  SECTION("renameDirectory")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir()};

#if defined _WIN32
    CHECK(
      fs.renameDirectory("c:\\hopefully_nothing_here", "dest")
      == Result<void>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"c:\\hopefully_nothing_here"}))}});
    CHECK(
      fs.renameDirectory("test", "C:\\dest")
      == Result<void>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"C:\\dest"}))}});
#else
    CHECK(
      fs.renameDirectory("/hopefully_nothing_here", "dir1/newDir")
      == Result<void>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"/hopefully_nothing_here"}))}});
    CHECK(
      fs.renameDirectory("anotherDir", "/dir1/newDir")
      == Result<void>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"/dir1/newDir"}))}});
#endif

    CHECK(fs.renameDirectory("anotherDir", "dir1/newDir") == Result<void>{});
    CHECK(fs.pathInfo("anotherDir") == PathInfo::Unknown);
    CHECK(fs.pathInfo("dir1/newDir") == PathInfo::Directory);
  }

  SECTION("copyFile")
  {
    const auto env = makeTestEnvironment();
    auto fs = WritableDiskFileSystem{env.dir()};

#if defined _WIN32
    CHECK(
      fs.copyFile("c:\\hopefully_nothing_here.txt", "dest.txt")
      == Result<void>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"c:\\hopefully_nothing_here.txt"}))}});
    CHECK(
      fs.copyFile("test.txt", "C:\\dest.txt")
      == Result<void>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"C:\\dest.txt"}))}});
#else
    CHECK(
      fs.copyFile("/hopefully_nothing_here.txt", "dest.txt")
      == Result<void>{Error{fmt::format(
        "Path {} is absolute",
        fmt::streamed(std::filesystem::path{"/hopefully_nothing_here.txt"}))}});
    CHECK(
      fs.copyFile("test.txt", "/dest.txt")
      == Result<void>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"/dest.txt"}))}});
#endif

    CHECK(fs.copyFile("test.txt", "test2.txt") == Result<void>{});
    CHECK(fs.pathInfo("test.txt") == PathInfo::File);
    CHECK(fs.pathInfo("test2.txt") == PathInfo::File);

    CHECK(fs.copyFile("test2.txt", "test2.map") == Result<void>{});
    CHECK(fs.pathInfo("test2.txt") == PathInfo::File);
    CHECK(fs.pathInfo("test2.map") == PathInfo::File);
    // we're trusting that the file is actually overwritten (should really test the
    // contents here...)

    CHECK(fs.copyFile("test2.map", "dir1/test2.map") == Result<void>{});
    CHECK(fs.pathInfo("test2.map") == PathInfo::File);
    CHECK(fs.pathInfo("dir1/test2.map") == PathInfo::File);
  }
}

} // namespace tb::io
