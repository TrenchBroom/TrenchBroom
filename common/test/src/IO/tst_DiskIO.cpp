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

#include "kdl/regex_utils.h"

#include <algorithm>
#include <filesystem>
#include <fstream>

#include "CatchUtils/Matchers.h"

#include "Catch2.h"

namespace TrenchBroom::IO
{

namespace
{
class SetPermissions
{
private:
  std::filesystem::path m_path;
  std::filesystem::perms m_permissions;

public:
  explicit SetPermissions(
    std::filesystem::path path, const std::filesystem::perms permissions)
    : m_path{std::move(path)}
    , m_permissions{std::filesystem::status(m_path).permissions()}
  {
    std::filesystem::permissions(m_path, permissions);
  }

  ~SetPermissions()
  {
    try
    {
      std::filesystem::permissions(m_path, m_permissions);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
      std::cout << "Could not reset file permissions for " << m_path << ": " << e.what()
                << "\n";
    }
  }
};

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

      env.createSymLink("anotherDir/subDirTest", "linkedDir");
      env.createSymLink("test2.map", "linkedTest2.map");
    }};
}

const auto readAll = [](auto& stream) {
  return std::string{
    std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
};

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

    CHECK(Disk::pathInfo(env.dir() / "linkedDir") == PathInfo::Directory);
    CHECK(Disk::pathInfo(env.dir() / "linkedTest2.map") == PathInfo::File);
  }

  SECTION("find")
  {
    CHECK_THAT(
      Disk::find("asdf/bleh", TraversalMode::Flat),
      MatchesAnyOf({
        // macOS
        Result<std::vector<std::filesystem::path>>{
          Error{"Failed to open 'asdf/bleh': No such file or directory"}},
        // Windows
        Result<std::vector<std::filesystem::path>>{Error{
          "Failed to open 'asdf\\bleh': The system cannot find the path specified."}},
      }));
    CHECK_THAT(
      Disk::find(env.dir() / "does/not/exist", TraversalMode::Flat),
      MatchesAnyOf({
        // macOS
        Result<std::vector<std::filesystem::path>>{Error{
          "Failed to open '" + (env.dir() / "does/not/exist").string()
          + "': No such file or directory"}},
        // Windows
        Result<std::vector<std::filesystem::path>>{Error{
          "Failed to open '" + (env.dir() / "does\\not\\exist").string()
          + "': The system cannot find the path specified."}},
      }));

    CHECK_THAT(
      Disk::find(env.dir(), TraversalMode::Flat) | kdl::value(),
      Catch::UnorderedEquals(std::vector<std::filesystem::path>{
        env.dir() / "dir1",
        env.dir() / "dir2",
        env.dir() / "anotherDir",
        env.dir() / "test.txt",
        env.dir() / "test2.map",
        env.dir() / "linkedDir",
        env.dir() / "linkedTest2.map",
      }));

    CHECK_THAT(
      Disk::find(env.dir(), TraversalMode::Recursive) | kdl::value(),
      Catch::UnorderedEquals(std::vector<std::filesystem::path>{
        env.dir() / "dir1",
        env.dir() / "dir2",
        env.dir() / "anotherDir",
        env.dir() / "anotherDir/subDirTest",
        env.dir() / "anotherDir/subDirTest/test2.map",
        env.dir() / "anotherDir/test3.map",
        env.dir() / "test.txt",
        env.dir() / "test2.map",
        env.dir() / "linkedDir",
        env.dir() / "linkedDir/test2.map",
        env.dir() / "linkedTest2.map",
      }));

    CHECK_THAT(
      Disk::find(env.dir(), TraversalMode{0}) | kdl::value(),
      Catch::UnorderedEquals(std::vector<std::filesystem::path>{
        env.dir() / "dir1",
        env.dir() / "dir2",
        env.dir() / "anotherDir",
        env.dir() / "test.txt",
        env.dir() / "test2.map",
        env.dir() / "linkedDir",
        env.dir() / "linkedTest2.map",
      }));

    CHECK_THAT(
      Disk::find(env.dir(), TraversalMode{1}) | kdl::value(),
      Catch::UnorderedEquals(std::vector<std::filesystem::path>{
        env.dir() / "dir1",
        env.dir() / "dir2",
        env.dir() / "anotherDir",
        env.dir() / "anotherDir/subDirTest",
        env.dir() / "anotherDir/test3.map",
        env.dir() / "test.txt",
        env.dir() / "test2.map",
        env.dir() / "linkedDir",
        env.dir() / "linkedDir/test2.map",
        env.dir() / "linkedTest2.map",
      }));
  }

  SECTION("openFile")
  {

    CHECK_THAT(
      Disk::openFile("asdf/bleh"),
      MatchesAnyOf({
        // macOS / Linux
        Result<std::shared_ptr<CFile>>{
          Error{"Failed to open 'asdf/bleh': path does not denote a file"}},
        // Windows
        Result<std::shared_ptr<CFile>>{
          Error{"Failed to open 'asdf\\bleh': path does not denote a file"}},
      }));
    CHECK_THAT(
      Disk::openFile(env.dir() / "does/not/exist"),
      MatchesAnyOf({
        // macOS / Linux
        Result<std::shared_ptr<CFile>>{Error{
          "Failed to open '" + (env.dir() / "does/not/exist").string()
          + "': path does not denote a file"}},
        // Windows
        Result<std::shared_ptr<CFile>>{Error{
          "Failed to open '" + (env.dir() / "does\\not\\exist").string()
          + "': path does not denote a file"}},
      }));
    CHECK(
      Disk::openFile(env.dir() / "does_not_exist.txt")
      == Result<std::shared_ptr<CFile>>{Error{
        "Failed to open '" + (env.dir() / "does_not_exist.txt").string()
        + "': path does not denote a file"}});

    auto file = Disk::openFile(env.dir() / "test.txt");
    CHECK(file.is_success());

    file = Disk::openFile(env.dir() / "anotherDir/subDirTest/test2.map");
    CHECK(file.is_success());

    file = Disk::openFile(env.dir() / "linkedDir/test2.map");
    CHECK(file.is_success());

    file = Disk::openFile(env.dir() / "linkedTest2.map");
    CHECK(file.is_success());
  }

  SECTION("withStream")
  {
    SECTION("withInputStream")
    {
      CHECK(
        Disk::withInputStream(env.dir() / "does not exist.txt", readAll)
        == Error{
          "Could not open stream for file '" + (env.dir() / "does not exist.txt").string()
          + "'"});

      CHECK(Disk::withInputStream(env.dir() / "test.txt", readAll) == "some content");
      CHECK(
        Disk::withInputStream(env.dir() / "linkedTest2.map", readAll)
        == "//test file\n{}");
    }

    SECTION("withOutputStream")
    {
      REQUIRE(Disk::withOutputStream(
                env.dir() / "test.txt",
                std::ios::out | std::ios::app,
                [](auto& stream) { stream << "\nmore content"; })
                .is_success());
      CHECK(
        Disk::withInputStream(env.dir() / "test.txt", readAll)
        == "some content\nmore content");

      REQUIRE(Disk::withOutputStream(env.dir() / "some_other_name.txt", [](auto& stream) {
                stream << "some text...";
              }).is_success());
      CHECK(
        Disk::withInputStream(env.dir() / "some_other_name.txt", readAll)
        == "some text...");

      REQUIRE(Disk::withOutputStream(
                env.dir() / "linkedTest2.map",
                std::ios::out | std::ios::app,
                [](auto& stream) { stream << "\nwow even more content"; })
                .is_success());
      CHECK(
        Disk::withInputStream(env.dir() / "test2.map", readAll)
        == "//test file\n{}\nwow even more content");
      CHECK(
        Disk::withInputStream(env.dir() / "linkedTest2.map", readAll)
        == "//test file\n{}\nwow even more content");
    }
  }

  SECTION("createDirectory")
  {
    CHECK(Disk::createDirectory(env.dir() / "anotherDir") == Result<bool>{false});

    CHECK(Disk::createDirectory(env.dir() / "yetAnotherDir") == Result<bool>{true});
    CHECK(std::filesystem::exists(env.dir() / "yetAnotherDir"));

    CHECK(
      Disk::createDirectory(env.dir() / "yetAnotherDir/and/a/nested/directory")
      == Result<bool>{true});
    CHECK(std::filesystem::exists(env.dir() / "yetAnotherDir/and/a/nested/directory"));

    CHECK(Disk::createDirectory(env.dir() / "linkedDir/nestedDir") == Result<bool>{true});
    CHECK(std::filesystem::exists(env.dir() / "linkedDir/nestedDir"));

    CHECK_THAT(
      Disk::createDirectory(env.dir() / "test.txt"),
      MatchesAnyOf({
        // macOS
        Result<bool>{Error{
          "Failed to create '" + (env.dir() / "test.txt").string() + "': File exists"}},
        // Linux
        Result<bool>{Error{
          "Failed to create '" + (env.dir() / "test.txt").string()
          + "': Not a directory"}},
        // Windows
        Result<bool>{Error{
          "Failed to create '" + (env.dir() / "test.txt").string()
          + "': Cannot create a file when that file already exists."}},
      }));

#ifndef _WIN32
    // These tests don't work on Windows due to differences in permissions
    const auto setPermissions =
      SetPermissions{env.dir() / "anotherDir", std::filesystem::perms::owner_read};
    CHECK(
      Disk::createDirectory(env.dir() / "anotherDir/nestedDir")
      == Result<bool>{Error{
        "Failed to create '" + (env.dir() / "anotherDir/nestedDir").string()
        + "': Permission denied"}});
#endif
  }

  SECTION("deleteFile")
  {
    REQUIRE(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
    CHECK(Disk::deleteFile(env.dir() / "test.txt") == Result<bool>{true});
    CHECK(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::Unknown);

    CHECK(
      Disk::deleteFile(env.dir() / "anotherDir")
      == Result<bool>{Error{
        "Failed to delete '" + (env.dir() / "anotherDir").string()
        + "': path denotes a directory"}});
    CHECK(Disk::deleteFile(env.dir() / "does_not_exist") == Result<bool>{false});

#ifndef _WIN32
    // These tests don't work on Windows due to differences in permissions
    const auto setPermissions =
      SetPermissions{env.dir() / "anotherDir", std::filesystem::perms::owner_exec};

    REQUIRE(Disk::pathInfo(env.dir() / "anotherDir/test3.map") == PathInfo::File);
    CHECK(
      Disk::deleteFile(env.dir() / "anotherDir/test3.map")
      == Result<bool>{Error{
        "Failed to delete '" + (env.dir() / "anotherDir/test3.map").string()
        + "': Permission denied"}});
#endif

    SECTION("Delete symlink")
    {
      REQUIRE(Disk::pathInfo(env.dir() / "linkedTest2.map") == PathInfo::File);
      CHECK(Disk::deleteFile(env.dir() / "linkedTest2.map") == Result<bool>{true});
      CHECK(Disk::pathInfo(env.dir() / "linkedTest2.map") == PathInfo::Unknown);
      CHECK(Disk::pathInfo(env.dir() / "test2.map") == PathInfo::File);
    }

    SECTION("Delete linked file")
    {
      REQUIRE(Disk::pathInfo(env.dir() / "test2.map") == PathInfo::File);
      CHECK(Disk::deleteFile(env.dir() / "test2.map") == Result<bool>{true});
      CHECK(Disk::pathInfo(env.dir() / "linkedTest2.map") == PathInfo::Unknown);
      CHECK(Disk::pathInfo(env.dir() / "test2.map") == PathInfo::Unknown);
    }
  }

  SECTION("copyFile")
  {
    SECTION("copy non existing file")
    {
      REQUIRE(Disk::pathInfo(env.dir() / "does_not_exist.txt") == PathInfo::Unknown);

      CHECK_THAT(
        Disk::copyFile(env.dir() / "does_not_exist.txt", env.dir() / "dir1"),
        MatchesAnyOf({
          // macOS / Linux
          Result<void>{Error{
            "Failed to copy '" + (env.dir() / "does_not_exist.txt").string() + "' to '"
            + (env.dir() / "dir1/does_not_exist.txt").string()
            + "': No such file or directory"}},
          // Windows
          Result<void>{Error{
            "Failed to copy '" + (env.dir() / "does_not_exist.txt").string() + "' to '"
            + (env.dir() / "dir1\\does_not_exist.txt").string()
            + "': The system cannot find the file specified."}},
        }));
    }

    SECTION("copy directory")
    {
      REQUIRE(Disk::pathInfo(env.dir() / "anotherDir") == PathInfo::Directory);

      CHECK_THAT(
        Disk::copyFile(env.dir() / "anotherDir", env.dir() / "dir1"),
        MatchesAnyOf({
          // macOS
          kdl::result<void, Error>{Error{
            "Failed to copy '" + (env.dir() / "anotherDir").string() + "' to '"
            + (env.dir() / "dir1/anotherDir").string() + "': Operation not supported"}},
          // Linux
          kdl::result<void, Error>{Error{
            "Failed to copy '" + (env.dir() / "anotherDir").string() + "' to '"
            + (env.dir() / "dir1/anotherDir").string() + "': Invalid argument"}},
          // Windows
          Result<void>{Error{
            "Failed to copy '" + (env.dir() / "anotherDir").string() + "' to '"
            + (env.dir() / "dir1\\anotherDir").string() + "': Access is denied."}},
        }));
    }

    SECTION("copy file into directory")
    {
      REQUIRE(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
      REQUIRE(Disk::pathInfo(env.dir() / "anotherDir/test.txt") == PathInfo::Unknown);

      CHECK(
        Disk::copyFile(env.dir() / "test.txt", env.dir() / "anotherDir")
        == Result<void>{});

      CHECK(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
      CHECK(Disk::pathInfo(env.dir() / "anotherDir/test.txt") == PathInfo::File);
    }

    SECTION("copy file to non existing file")
    {
      SECTION("when the file can be created")
      {
        REQUIRE(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
        REQUIRE(Disk::pathInfo(env.dir() / "anotherDir/asdf.txt") == PathInfo::Unknown);

        CHECK(
          Disk::copyFile(env.dir() / "test.txt", env.dir() / "anotherDir/asdf.txt")
          == Result<void>{});

        CHECK(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
        CHECK(Disk::pathInfo(env.dir() / "anotherDir/asdf.txt") == PathInfo::File);
      }

      SECTION("when the file cannot be created")
      {
#ifndef _WIN32
        // These tests don't work on Windows due to differences in permissions
        REQUIRE(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
        REQUIRE(Disk::pathInfo(env.dir() / "anotherDir/asdf.txt") == PathInfo::Unknown);

        const auto setPermissions =
          SetPermissions{env.dir() / "anotherDir", std::filesystem::perms::owner_exec};

        CHECK(
          Disk::copyFile(env.dir() / "test.txt", env.dir() / "anotherDir/asdf.txt")
          == Result<void>{Error{
            "Failed to copy '" + (env.dir() / "test.txt").string() + "' to '"
            + (env.dir() / "anotherDir/asdf.txt").string() + "': Permission denied"}});
        CHECK(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
#endif
      }
    }

    SECTION("copy file over existing file")
    {
      REQUIRE(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
      REQUIRE(Disk::pathInfo(env.dir() / "anotherDir/test3.map") == PathInfo::File);
      REQUIRE(
        Disk::withInputStream(env.dir() / "anotherDir/test3.map", readAll)
        != "some content");

      SECTION("when the file can be overwritten")
      {
        CHECK(
          Disk::copyFile(env.dir() / "test.txt", env.dir() / "anotherDir/test3.map")
          == Result<void>{});

        CHECK(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
        CHECK(Disk::pathInfo(env.dir() / "anotherDir/test3.map") == PathInfo::File);
        CHECK(
          Disk::withInputStream(env.dir() / "anotherDir/test3.map", readAll)
          == "some content");
      }

      SECTION("when file cannot be overwritte")
      {
#ifndef _WIN32
        // These tests don't work on Windows due to differences in permissions
        const auto setPermissions = SetPermissions{
          env.dir() / "anotherDir/test3.map", std::filesystem::perms::none};

        CHECK(
          Disk::copyFile(env.dir() / "test.txt", env.dir() / "anotherDir/test3.map")
          == Result<void>{Error{
            "Failed to copy '" + (env.dir() / "test.txt").string() + "' to '"
            + (env.dir() / "anotherDir/test3.map").string() + "': Permission denied"}});
        CHECK(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
#endif
      }
    }
  }

  SECTION("moveFile")
  {
    SECTION("move non existing file")
    {
      REQUIRE(Disk::pathInfo(env.dir() / "does_not_exist.txt") == PathInfo::Unknown);

      CHECK_THAT(
        Disk::moveFile(env.dir() / "does_not_exist.txt", env.dir() / "dir1"),
        MatchesAnyOf({
          // macOS / Linux
          Result<void>{Error{
            "Failed to move '" + (env.dir() / "does_not_exist.txt").string() + "' to '"
            + (env.dir() / "dir1/does_not_exist.txt").string()
            + "': No such file or directory"}},
          // Windows
          Result<void>{Error{
            "Failed to move '" + (env.dir() / "does_not_exist.txt").string() + "' to '"
            + (env.dir() / "dir1\\does_not_exist.txt").string()
            + "': The system cannot find the file specified."}},
        }));
    }

    SECTION("move directory")
    {
      REQUIRE(Disk::pathInfo(env.dir() / "anotherDir") == PathInfo::Directory);

      CHECK(
        Disk::moveFile(env.dir() / "anotherDir", env.dir() / "dir1")
        == Result<void>{Error{
          "Failed to move '" + (env.dir() / "anotherDir").string()
          + "': path denotes a directory"}});
      CHECK(Disk::pathInfo(env.dir() / "anotherDir") == PathInfo::Directory);
    }

    SECTION("move file into directory")
    {
      REQUIRE(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
      REQUIRE(Disk::pathInfo(env.dir() / "anotherDir/test.txt") == PathInfo::Unknown);

      CHECK(
        Disk::moveFile(env.dir() / "test.txt", env.dir() / "anotherDir")
        == Result<void>{});

      CHECK(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::Unknown);
      CHECK(Disk::pathInfo(env.dir() / "anotherDir/test.txt") == PathInfo::File);
    }

    SECTION("move file to non existing file")
    {
      SECTION("when the file can be created")
      {
        REQUIRE(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
        REQUIRE(Disk::pathInfo(env.dir() / "anotherDir/asdf.txt") == PathInfo::Unknown);

        CHECK(
          Disk::moveFile(env.dir() / "test.txt", env.dir() / "anotherDir/asdf.txt")
          == Result<void>{});

        CHECK(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::Unknown);
        CHECK(Disk::pathInfo(env.dir() / "anotherDir/asdf.txt") == PathInfo::File);
      }

      SECTION("when the file cannot be created")
      {
#ifndef _WIN32
        // These tests don't work on Windows due to differences in permissions
        REQUIRE(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
        REQUIRE(Disk::pathInfo(env.dir() / "anotherDir/asdf.txt") == PathInfo::Unknown);

        const auto setPermissions =
          SetPermissions{env.dir() / "anotherDir", std::filesystem::perms::owner_exec};

        CHECK(
          Disk::moveFile(env.dir() / "test.txt", env.dir() / "anotherDir/asdf.txt")
          == Result<void>{Error{
            "Failed to move '" + (env.dir() / "test.txt").string() + "' to '"
            + (env.dir() / "anotherDir/asdf.txt").string() + "': Permission denied"}});
        CHECK(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
#endif
      }
    }

    SECTION("move file over existing file")
    {
      REQUIRE(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
      REQUIRE(Disk::pathInfo(env.dir() / "anotherDir/test3.map") == PathInfo::File);
      REQUIRE(
        Disk::withInputStream(env.dir() / "anotherDir/test3.map", readAll)
        != "some content");

      SECTION("when the file can be overwritten")
      {
        CHECK(
          Disk::moveFile(env.dir() / "test.txt", env.dir() / "anotherDir/test3.map")
          == Result<void>{});

        CHECK(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::Unknown);
        CHECK(Disk::pathInfo(env.dir() / "anotherDir/test3.map") == PathInfo::File);
        CHECK(
          Disk::withInputStream(env.dir() / "anotherDir/test3.map", readAll)
          == "some content");
      }

      SECTION("when the file cannot be overwritten")
      {
#ifndef _WIN32
        // These tests don't work on Windows due to differences in permissions
        const auto setPermissions =
          SetPermissions{env.dir() / "anotherDir", std::filesystem::perms::owner_exec};

        CHECK(
          Disk::moveFile(env.dir() / "test.txt", env.dir() / "anotherDir/test3.map")
          == Result<void>{Error{
            "Failed to move '" + (env.dir() / "test.txt").string() + "' to '"
            + (env.dir() / "anotherDir/test3.map").string() + "': Permission denied"}});
        CHECK(Disk::pathInfo(env.dir() / "test.txt") == PathInfo::File);
#endif
      }
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

    CHECK(
      Disk::resolvePath(rootPaths, "linkedTest2.map") == env.dir() / "linkedTest2.map");

    CHECK(
      Disk::resolvePath(rootPaths, "linkedDir/test2.map")
      == env.dir() / "linkedDir/test2.map");
  }
}

} // namespace TrenchBroom::IO
