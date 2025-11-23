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

#include "fs/DiskIO.h"
#include "fs/File.h"
#include "fs/PathInfo.h"
#include "fs/TestEnvironment.h"
#include "fs/TraversalMode.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <filesystem>
#include <iostream>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::fs
{
using namespace Catch::Matchers;

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
  return TestEnvironment{[](TestEnvironment& env) {
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
    CHECK(fs::Disk::fixPath("asdf/blah") == "asdf/blah");
    CHECK(fs::Disk::fixPath("/../../test") == "/test");

    if (fs::Disk::isCaseSensitive())
    {
      CHECK(fs::Disk::fixPath(env.dir() / "TEST.txt") == env.dir() / "test.txt");
      CHECK(
        fs::Disk::fixPath(env.dir() / "anotHERDIR/./SUBdirTEST/../SubdirTesT/TesT2.MAP")
        == env.dir() / "anotherDir/subDirTest/test2.map");
    }
  }

  SECTION("pathInfo")
  {
    CHECK(fs::Disk::pathInfo("asdf/bleh") == fs::PathInfo::Unknown);
    CHECK(fs::Disk::pathInfo(env.dir() / "anotherDir/asdf.map") == fs::PathInfo::Unknown);
    CHECK(
      fs::Disk::pathInfo(env.dir() / "anotherDir/test3.map/asdf")
      == fs::PathInfo::Unknown);

    CHECK(fs::Disk::pathInfo(env.dir() / "anotherDir") == fs::PathInfo::Directory);
    CHECK(fs::Disk::pathInfo(env.dir() / "ANOTHERDIR") == fs::PathInfo::Directory);
    CHECK(
      fs::Disk::pathInfo(env.dir() / "anotherDir/subDirTest") == fs::PathInfo::Directory);

    CHECK(fs::Disk::pathInfo(env.dir() / "anotherDir/test3.map") == fs::PathInfo::File);
    CHECK(fs::Disk::pathInfo(env.dir() / "anotherDir/TEST3.map") == fs::PathInfo::File);
    CHECK(
      fs::Disk::pathInfo(env.dir() / "anotherDir/subDirTest/test2.map")
      == fs::PathInfo::File);

    CHECK(fs::Disk::pathInfo(env.dir() / "linkedDir") == fs::PathInfo::Directory);
    CHECK(fs::Disk::pathInfo(env.dir() / "linkedTest2.map") == fs::PathInfo::File);
  }

  SECTION("find")
  {
    CHECK(
      fs::Disk::find("asdf/bleh", fs::TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Failed to open {}: path does not denote a directory",
        std::filesystem::path{"asdf/bleh"})}});
    CHECK(
      fs::Disk::find(env.dir() / "does/not/exist", fs::TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Failed to open {}: path does not denote a directory",
        env.dir() / "does/not/exist")}});

    CHECK_THAT(
      fs::Disk::find(env.dir(), fs::TraversalMode::Flat) | kdl::value(),
      UnorderedEquals(std::vector<std::filesystem::path>{
        env.dir() / "dir1",
        env.dir() / "dir2",
        env.dir() / "anotherDir",
        env.dir() / "test.txt",
        env.dir() / "test2.map",
        env.dir() / "linkedDir",
        env.dir() / "linkedTest2.map",
      }));

    CHECK_THAT(
      fs::Disk::find(env.dir(), fs::TraversalMode::Recursive) | kdl::value(),
      UnorderedEquals(std::vector<std::filesystem::path>{
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
      fs::Disk::find(env.dir(), TraversalMode{0}) | kdl::value(),
      UnorderedEquals(std::vector<std::filesystem::path>{
        env.dir() / "dir1",
        env.dir() / "dir2",
        env.dir() / "anotherDir",
        env.dir() / "test.txt",
        env.dir() / "test2.map",
        env.dir() / "linkedDir",
        env.dir() / "linkedTest2.map",
      }));

    CHECK_THAT(
      fs::Disk::find(env.dir(), TraversalMode{1}) | kdl::value(),
      UnorderedEquals(std::vector<std::filesystem::path>{
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

    CHECK(
      fs::Disk::openFile("asdf/bleh")
      == Result<std::shared_ptr<CFile>>{Error{fmt::format(
        "Failed to open {}: path does not denote a file",
        std::filesystem::path{"asdf/bleh"})}});
    CHECK(
      fs::Disk::openFile(env.dir() / "does/not/exist")
      == Result<std::shared_ptr<CFile>>{Error{fmt::format(
        "Failed to open {}: path does not denote a file",
        env.dir() / "does/not/exist")}});

    CHECK(
      fs::Disk::openFile(env.dir() / "does_not_exist.txt")
      == Result<std::shared_ptr<CFile>>{Error{fmt::format(
        "Failed to open {}: path does not denote a file",
        env.dir() / "does_not_exist.txt")}});

    CHECK(fs::Disk::openFile(env.dir() / "test.txt"));
    CHECK(fs::Disk::openFile(env.dir() / "anotherDir/subDirTest/test2.map"));
    CHECK(fs::Disk::openFile(env.dir() / "linkedDir/test2.map"));
    CHECK(fs::Disk::openFile(env.dir() / "linkedTest2.map"));
  }

  SECTION("withStream")
  {
    SECTION("withInputStream")
    {
      CHECK(
        fs::Disk::withInputStream(env.dir() / "does not exist.txt", readAll)
        == Error{"Failed to open stream"});

      CHECK(fs::Disk::withInputStream(env.dir() / "test.txt", readAll) == "some content");
      CHECK(
        fs::Disk::withInputStream(env.dir() / "linkedTest2.map", readAll)
        == "//test file\n{}");
    }

    SECTION("withOutputStream")
    {
      REQUIRE(fs::Disk::withOutputStream(
        env.dir() / "test.txt", std::ios::out | std::ios::app, [](auto& stream) {
          stream << "\nmore content";
        }));
      CHECK(
        fs::Disk::withInputStream(env.dir() / "test.txt", readAll)
        == "some content\nmore content");

      REQUIRE(fs::Disk::withOutputStream(
        env.dir() / "some_other_name.txt",
        [](auto& stream) { stream << "some text..."; }));
      CHECK(
        fs::Disk::withInputStream(env.dir() / "some_other_name.txt", readAll)
        == "some text...");

      REQUIRE(fs::Disk::withOutputStream(
        env.dir() / "linkedTest2.map", std::ios::out | std::ios::app, [](auto& stream) {
          stream << "\nwow even more content";
        }));
      CHECK(
        fs::Disk::withInputStream(env.dir() / "test2.map", readAll)
        == "//test file\n{}\nwow even more content");
      CHECK(
        fs::Disk::withInputStream(env.dir() / "linkedTest2.map", readAll)
        == "//test file\n{}\nwow even more content");
    }
  }

  SECTION("createDirectory")
  {
    CHECK(fs::Disk::createDirectory(env.dir() / "anotherDir") == Result<bool>{false});

    CHECK(fs::Disk::createDirectory(env.dir() / "yetAnotherDir") == Result<bool>{true});
    CHECK(std::filesystem::exists(env.dir() / "yetAnotherDir"));

    CHECK(
      fs::Disk::createDirectory(env.dir() / "yetAnotherDir/and/a/nested/directory")
      == Result<bool>{true});
    CHECK(std::filesystem::exists(env.dir() / "yetAnotherDir/and/a/nested/directory"));

    CHECK(
      fs::Disk::createDirectory(env.dir() / "linkedDir/nestedDir") == Result<bool>{true});
    CHECK(std::filesystem::exists(env.dir() / "linkedDir/nestedDir"));

    CHECK(
      fs::Disk::createDirectory(env.dir() / "test.txt")
      == Result<bool>{Error{fmt::format(
        "Failed to create {}: path denotes a file", env.dir() / "test.txt")}});

#ifndef _WIN32
    // These tests don't work on Windows due to differences in permissions
    const auto setPermissions =
      SetPermissions{env.dir() / "anotherDir", std::filesystem::perms::owner_read};
    CHECK(fs::Disk::createDirectory(env.dir() / "anotherDir/nestedDir").is_error());
#endif
  }

  SECTION("deleteFile")
  {
    REQUIRE(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
    CHECK(fs::Disk::deleteFile(env.dir() / "test.txt") == Result<bool>{true});
    CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::Unknown);

    CHECK(
      fs::Disk::deleteFile(env.dir() / "anotherDir")
      == Result<bool>{Error{fmt::format(
        "Failed to delete {}: path denotes a directory", env.dir() / "anotherDir")}});
    CHECK(fs::Disk::deleteFile(env.dir() / "does_not_exist") == Result<bool>{false});

#ifndef _WIN32
    // These tests don't work on Windows due to differences in permissions
    const auto setPermissions =
      SetPermissions{env.dir() / "anotherDir", std::filesystem::perms::owner_exec};

    REQUIRE(fs::Disk::pathInfo(env.dir() / "anotherDir/test3.map") == fs::PathInfo::File);
    CHECK(fs::Disk::deleteFile(env.dir() / "anotherDir/test3.map").is_error());
#endif

    SECTION("Delete symlink")
    {
      REQUIRE(fs::Disk::pathInfo(env.dir() / "linkedTest2.map") == fs::PathInfo::File);
      CHECK(fs::Disk::deleteFile(env.dir() / "linkedTest2.map") == Result<bool>{true});
      CHECK(fs::Disk::pathInfo(env.dir() / "linkedTest2.map") == fs::PathInfo::Unknown);
      CHECK(fs::Disk::pathInfo(env.dir() / "test2.map") == fs::PathInfo::File);
    }

    SECTION("Delete linked file")
    {
      REQUIRE(fs::Disk::pathInfo(env.dir() / "test2.map") == fs::PathInfo::File);
      CHECK(fs::Disk::deleteFile(env.dir() / "test2.map") == Result<bool>{true});
      CHECK(fs::Disk::pathInfo(env.dir() / "linkedTest2.map") == fs::PathInfo::Unknown);
      CHECK(fs::Disk::pathInfo(env.dir() / "test2.map") == fs::PathInfo::Unknown);
    }
  }

  SECTION("copyFile")
  {
    SECTION("copy non existing file")
    {
      REQUIRE(
        fs::Disk::pathInfo(env.dir() / "does_not_exist.txt") == fs::PathInfo::Unknown);

      CHECK(
        fs::Disk::copyFile(env.dir() / "does_not_exist.txt", env.dir() / "dir1")
        == Result<void>{Error{fmt::format(
          "Failed to copy {}: path does not denote a file",
          env.dir() / "does_not_exist.txt")}});
    }

    SECTION("copy directory")
    {
      REQUIRE(fs::Disk::pathInfo(env.dir() / "anotherDir") == fs::PathInfo::Directory);

      CHECK(
        fs::Disk::copyFile(env.dir() / "anotherDir", env.dir() / "dir1")
        == Result<void>{Error{fmt::format(
          "Failed to copy {}: path does not denote a file", env.dir() / "anotherDir")}});
    }

    SECTION("copy file into directory")
    {
      REQUIRE(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
      REQUIRE(
        fs::Disk::pathInfo(env.dir() / "anotherDir/test.txt") == fs::PathInfo::Unknown);

      CHECK(
        fs::Disk::copyFile(env.dir() / "test.txt", env.dir() / "anotherDir")
        == Result<void>{});

      CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
      CHECK(fs::Disk::pathInfo(env.dir() / "anotherDir/test.txt") == fs::PathInfo::File);
    }

    SECTION("copy file to non existing file")
    {
      SECTION("when the file can be created")
      {
        REQUIRE(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
        REQUIRE(
          fs::Disk::pathInfo(env.dir() / "anotherDir/asdf.txt") == fs::PathInfo::Unknown);

        CHECK(
          fs::Disk::copyFile(env.dir() / "test.txt", env.dir() / "anotherDir/asdf.txt")
          == Result<void>{});

        CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
        CHECK(
          fs::Disk::pathInfo(env.dir() / "anotherDir/asdf.txt") == fs::PathInfo::File);
      }

      SECTION("when the file cannot be created")
      {
#ifndef _WIN32
        // These tests don't work on Windows due to differences in permissions
        REQUIRE(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
        REQUIRE(
          fs::Disk::pathInfo(env.dir() / "anotherDir/asdf.txt") == fs::PathInfo::Unknown);

        const auto setPermissions =
          SetPermissions{env.dir() / "anotherDir", std::filesystem::perms::owner_exec};

        CHECK(
          fs::Disk::copyFile(env.dir() / "test.txt", env.dir() / "anotherDir/asdf.txt")
            .is_error());
        CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
#endif
      }
    }

    SECTION("copy file over existing file")
    {
      REQUIRE(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
      REQUIRE(
        fs::Disk::pathInfo(env.dir() / "anotherDir/test3.map") == fs::PathInfo::File);
      REQUIRE(
        fs::Disk::withInputStream(env.dir() / "anotherDir/test3.map", readAll)
        != "some content");

      SECTION("when the file can be overwritten")
      {
        CHECK(
          fs::Disk::copyFile(env.dir() / "test.txt", env.dir() / "anotherDir/test3.map")
          == Result<void>{});

        CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
        CHECK(
          fs::Disk::pathInfo(env.dir() / "anotherDir/test3.map") == fs::PathInfo::File);
        CHECK(
          fs::Disk::withInputStream(env.dir() / "anotherDir/test3.map", readAll)
          == "some content");
      }

      SECTION("when file cannot be overwritte")
      {
#ifndef _WIN32
        // These tests don't work on Windows due to differences in permissions
        const auto setPermissions = SetPermissions{
          env.dir() / "anotherDir/test3.map", std::filesystem::perms::none};

        CHECK(
          fs::Disk::copyFile(env.dir() / "test.txt", env.dir() / "anotherDir/test3.map")
            .is_error());
        CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
#endif
      }
    }
  }

  SECTION("moveFile")
  {
    SECTION("move non existing file")
    {
      REQUIRE(
        fs::Disk::pathInfo(env.dir() / "does_not_exist.txt") == fs::PathInfo::Unknown);

      CHECK(
        fs::Disk::moveFile(env.dir() / "does_not_exist.txt", env.dir() / "dir1")
        == Result<void>{Error{fmt::format(
          "Failed to move {}: path does not denote a file",
          env.dir() / "does_not_exist.txt")}});
    }

    SECTION("move directory")
    {
      REQUIRE(fs::Disk::pathInfo(env.dir() / "anotherDir") == fs::PathInfo::Directory);

      CHECK(
        fs::Disk::moveFile(env.dir() / "anotherDir", env.dir() / "dir1")
        == Result<void>{Error{fmt::format(
          "Failed to move {}: path does not denote a file", env.dir() / "anotherDir")}});
      CHECK(fs::Disk::pathInfo(env.dir() / "anotherDir") == fs::PathInfo::Directory);
    }

    SECTION("move file into directory")
    {
      REQUIRE(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
      REQUIRE(
        fs::Disk::pathInfo(env.dir() / "anotherDir/test.txt") == fs::PathInfo::Unknown);

      CHECK(
        fs::Disk::moveFile(env.dir() / "test.txt", env.dir() / "anotherDir")
        == Result<void>{});

      CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::Unknown);
      CHECK(fs::Disk::pathInfo(env.dir() / "anotherDir/test.txt") == fs::PathInfo::File);
    }

    SECTION("move file to non existing file")
    {
      SECTION("when the file can be created")
      {
        REQUIRE(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
        REQUIRE(
          fs::Disk::pathInfo(env.dir() / "anotherDir/asdf.txt") == fs::PathInfo::Unknown);

        CHECK(
          fs::Disk::moveFile(env.dir() / "test.txt", env.dir() / "anotherDir/asdf.txt")
          == Result<void>{});

        CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::Unknown);
        CHECK(
          fs::Disk::pathInfo(env.dir() / "anotherDir/asdf.txt") == fs::PathInfo::File);
      }

      SECTION("when the file cannot be created")
      {
#ifndef _WIN32
        // These tests don't work on Windows due to differences in permissions
        REQUIRE(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
        REQUIRE(
          fs::Disk::pathInfo(env.dir() / "anotherDir/asdf.txt") == fs::PathInfo::Unknown);

        const auto setPermissions =
          SetPermissions{env.dir() / "anotherDir", std::filesystem::perms::owner_exec};

        CHECK(
          fs::Disk::moveFile(env.dir() / "test.txt", env.dir() / "anotherDir/asdf.txt")
            .is_error());
        CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
#endif
      }
    }

    SECTION("move file over existing file")
    {
      REQUIRE(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
      REQUIRE(
        fs::Disk::pathInfo(env.dir() / "anotherDir/test3.map") == fs::PathInfo::File);
      REQUIRE(
        fs::Disk::withInputStream(env.dir() / "anotherDir/test3.map", readAll)
        != "some content");

      SECTION("when the file can be overwritten")
      {
        CHECK(
          fs::Disk::moveFile(env.dir() / "test.txt", env.dir() / "anotherDir/test3.map")
          == Result<void>{});

        CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::Unknown);
        CHECK(
          fs::Disk::pathInfo(env.dir() / "anotherDir/test3.map") == fs::PathInfo::File);
        CHECK(
          fs::Disk::withInputStream(env.dir() / "anotherDir/test3.map", readAll)
          == "some content");
      }

      SECTION("when the file cannot be overwritten")
      {
#ifndef _WIN32
        // These tests don't work on Windows due to differences in permissions
        const auto setPermissions =
          SetPermissions{env.dir() / "anotherDir", std::filesystem::perms::owner_exec};

        CHECK(
          fs::Disk::moveFile(env.dir() / "test.txt", env.dir() / "anotherDir/test3.map")
            .is_error());
        CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
#endif
      }
    }
  }

  SECTION("renameDirectory")
  {
    SECTION("rename non existing directory")
    {
      REQUIRE(fs::Disk::pathInfo(env.dir() / "does_not_exist") == fs::PathInfo::Unknown);

      CHECK(
        fs::Disk::renameDirectory(
          env.dir() / "does_not_exist", env.dir() / "dir1/does_not_exist")
        == Result<void>{Error{fmt::format(
          "Failed to rename {}: path does not denote a directory",
          env.dir() / "does_not_exist")}});
    }

    SECTION("rename file")
    {
      REQUIRE(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);

      CHECK(
        fs::Disk::renameDirectory(env.dir() / "test.txt", env.dir() / "dir1")
        == Result<void>{Error{fmt::format(
          "Failed to rename {}: path does not denote a directory",
          env.dir() / "test.txt")}});
      CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
    }

    SECTION("target is existing file")
    {
      REQUIRE(fs::Disk::pathInfo(env.dir() / "anotherDir") == fs::PathInfo::Directory);
      REQUIRE(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);

      CHECK(
        fs::Disk::renameDirectory(env.dir() / "anotherDir", env.dir() / "test.txt")
        == Result<void>{Error{fmt::format(
          "Failed to rename {} to {}: target path already exists",
          env.dir() / "anotherDir",
          env.dir() / "test.txt")}});

      CHECK(fs::Disk::pathInfo(env.dir() / "anotherDir") == fs::PathInfo::Directory);
      CHECK(fs::Disk::pathInfo(env.dir() / "test.txt") == fs::PathInfo::File);
    }

    SECTION("target is existing directory")
    {
      REQUIRE(fs::Disk::pathInfo(env.dir() / "anotherDir") == fs::PathInfo::Directory);
      REQUIRE(fs::Disk::pathInfo(env.dir() / "dir1") == fs::PathInfo::Directory);

      CHECK(
        fs::Disk::renameDirectory(env.dir() / "anotherDir", env.dir() / "dir1")
        == Result<void>{Error{fmt::format(
          "Failed to rename {} to {}: target path already exists",
          env.dir() / "anotherDir",
          env.dir() / "dir1")}});

      CHECK(fs::Disk::pathInfo(env.dir() / "anotherDir") == fs::PathInfo::Directory);
      CHECK(fs::Disk::pathInfo(env.dir() / "dir1") == fs::PathInfo::Directory);
    }

    SECTION("rename directory")
    {
      SECTION("when the directory can be created")
      {
        REQUIRE(fs::Disk::pathInfo(env.dir() / "anotherDir") == fs::PathInfo::Directory);
        REQUIRE(fs::Disk::pathInfo(env.dir() / "dir1/newDir1") == fs::PathInfo::Unknown);

        CHECK(
          fs::Disk::renameDirectory(env.dir() / "anotherDir", env.dir() / "dir1/newDir1")
          == Result<void>{});

        CHECK(fs::Disk::pathInfo(env.dir() / "anotherDir") == fs::PathInfo::Unknown);
        CHECK(fs::Disk::pathInfo(env.dir() / "dir1/newDir1") == fs::PathInfo::Directory);
      }

      SECTION("when the directory cannot be created")
      {
#ifndef _WIN32
        // These tests don't work on Windows due to differences in permissions
        REQUIRE(fs::Disk::pathInfo(env.dir() / "anotherDir") == fs::PathInfo::Directory);
        REQUIRE(fs::Disk::pathInfo(env.dir() / "dir1/newDir1") == fs::PathInfo::Unknown);

        const auto setPermissions =
          SetPermissions{env.dir() / "dir1", std::filesystem::perms::owner_exec};

        CHECK(
          fs::Disk::renameDirectory(env.dir() / "anotherDir", env.dir() / "dir1/newDir1")
            .is_error());
        CHECK(fs::Disk::pathInfo(env.dir() / "anotherDir") == fs::PathInfo::Directory);
#endif
      }
    }
  }

  SECTION("resolvePath")
  {
    const auto rootPaths =
      std::vector<std::filesystem::path>{env.dir(), env.dir() / "anotherDir"};

    CHECK(fs::Disk::resolvePath(rootPaths, "test.txt") == env.dir() / "test.txt");
    CHECK(
      fs::Disk::resolvePath(rootPaths, "test3.map")
      == env.dir() / "anotherDir/test3.map");
    CHECK(
      fs::Disk::resolvePath(rootPaths, "subDirTest/test2.map")
      == env.dir() / "anotherDir/subDirTest/test2.map");
    CHECK(fs::Disk::resolvePath(rootPaths, "/asfd/blah") == "");
    CHECK(fs::Disk::resolvePath(rootPaths, "adk3kdk/bhb") == "");

    CHECK(
      fs::Disk::resolvePath(rootPaths, "linkedTest2.map")
      == env.dir() / "linkedTest2.map");

    CHECK(
      fs::Disk::resolvePath(rootPaths, "linkedDir/test2.map")
      == env.dir() / "linkedDir/test2.map");
  }

  SECTION("makeUniqueFilename")
  {
    CHECK(fs::Disk::makeUniqueFilename("/does/not/exist").is_success());
    CHECK(
      fs::Disk::makeUniqueFilename(std::filesystem::temp_directory_path()).is_success());
  }
}

} // namespace tb::fs
