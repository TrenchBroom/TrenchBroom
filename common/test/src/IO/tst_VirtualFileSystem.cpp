/*
 Copyright (C) 2023 Kristian Duske

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

#include "IO/File.h"
#include "IO/TestFileSystem.h"
#include "IO/VirtualFileSystem.h"

#include <kdl/overload.h>
#include <kdl/reflection_impl.h>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{

TEST_CASE("VirtualFileSystem")
{
  auto vfs = VirtualFileSystem{};

  SECTION("if nothing is mounted")
  {
    SECTION("makeAbsolute")
    {
      CHECK_THROWS_AS(vfs.makeAbsolute(""), FileSystemException);
      CHECK_THROWS_AS(vfs.makeAbsolute("foo/bar"), FileSystemException);
    }

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo("") == PathInfo::Unknown);
      CHECK(vfs.pathInfo("foo/bar") == PathInfo::Unknown);
    }

    SECTION("directoryContents")
    {
      CHECK_THROWS_AS(vfs.directoryContents(""), FileSystemException);
      CHECK_THROWS_AS(vfs.directoryContents("foo/bar"), FileSystemException);
    }

    SECTION("openFile")
    {
      CHECK_THROWS_AS(vfs.openFile(""), FileSystemException);
      CHECK_THROWS_AS(vfs.openFile("foo"), FileSystemException);
      CHECK_THROWS_AS(vfs.openFile("foo/bar"), FileSystemException);
    }
  }

  SECTION("with a file system mounted at the root")
  {
    auto foo_bar_baz = std::make_shared<ObjectFile<Object>>("foo/bar/baz", Object{1});
    auto bar_foo = std::make_shared<ObjectFile<Object>>("bar/foo", Object{2});

    vfs.mount(
      "",
      std::make_unique<TestFileSystem>(Entry{DirectoryEntry{
        "",
        {
          DirectoryEntry{
            "foo",
            {
              DirectoryEntry{
                "bar",
                {
                  FileEntry{"baz", foo_bar_baz},
                }},
            }},
          DirectoryEntry{
            "bar",
            {
              FileEntry{"foo", bar_foo},
            }},
        }}}));

    SECTION("makeAbsolute")
    {
      CHECK(vfs.makeAbsolute("") == "/");
      CHECK(vfs.makeAbsolute("foo") == "/foo");
      CHECK(vfs.makeAbsolute("foo/bar") == "/foo/bar");
    }

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo("") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo/bar") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo/bar/baz") == PathInfo::File);
      CHECK(vfs.pathInfo("foo/baz") == PathInfo::Unknown);
    }

    SECTION("directoryContents")
    {
      CHECK_THAT(
        vfs.directoryContents(""),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "foo",
          "bar",
        }));
      CHECK_THAT(
        vfs.directoryContents("foo"),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "bar",
        }));
      CHECK_THAT(
        vfs.directoryContents("foo/bar"),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "baz",
        }));
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile("foo/bar/baz") == foo_bar_baz);
      CHECK(vfs.openFile("bar/foo") == bar_foo);
    }
  }

  SECTION("with two file systems mounted at the root")
  {
    auto foo_bar_baz = std::make_shared<ObjectFile<Object>>("foo/bar/baz", Object{1});
    auto bar_foo = std::make_shared<ObjectFile<Object>>("bar/foo", Object{2});
    auto bar_bat_fs1 = std::make_shared<ObjectFile<Object>>("bar/bat", Object{3});
    auto bar_bat_fs2 = std::make_shared<ObjectFile<Object>>("bar/bat", Object{4});
    auto bar_cat = std::make_shared<ObjectFile<Object>>("bar/cat", Object{5});

    vfs.mount(
      "",
      std::make_unique<TestFileSystem>(
        Entry{DirectoryEntry{
          "",
          {
            DirectoryEntry{
              "foo",
              {
                DirectoryEntry{
                  "bar",
                  {
                    FileEntry{"baz", foo_bar_baz},
                  }},
              }},
            DirectoryEntry{
              "bar",
              {
                FileEntry{"foo", bar_foo},
                FileEntry{"bat", bar_bat_fs1},
                FileEntry{"cat", nullptr},
              }},
          }}},
        "/fs1"));
    vfs.mount(
      "",
      std::make_unique<TestFileSystem>(
        Entry{DirectoryEntry{
          "",
          {
            DirectoryEntry{
              "bar",
              {
                FileEntry{"bat", bar_bat_fs2},
                FileEntry{"baz", nullptr},
                DirectoryEntry{"cat", {}},
              }},
            DirectoryEntry{
              "baz",
              {
                FileEntry{"foo", nullptr},
              }},
          }}},
        "/fs2"));

    SECTION("makeAbsolute")
    {
      CHECK(vfs.makeAbsolute("") == "/fs2/");
      CHECK(vfs.makeAbsolute("foo") == "/fs1/foo");
      CHECK(vfs.makeAbsolute("foo/bar") == "/fs1/foo/bar");
      CHECK(vfs.makeAbsolute("bar") == "/fs2/bar");
      CHECK(vfs.makeAbsolute("bar/foo") == "/fs1/bar/foo");
      CHECK(vfs.makeAbsolute("bar/bat") == "/fs2/bar/bat");
      CHECK(vfs.makeAbsolute("bar/baz") == "/fs2/bar/baz");
      CHECK(vfs.makeAbsolute("bar/cat") == "/fs2/bar/cat");
      CHECK(vfs.makeAbsolute("baz") == "/fs2/baz");
      CHECK(vfs.makeAbsolute("baz/foo") == "/fs2/baz/foo");
    }

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo("") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo/bar") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo/bar/baz") == PathInfo::File);
      CHECK(vfs.pathInfo("bar") == PathInfo::Directory);
      CHECK(vfs.pathInfo("bar/foo") == PathInfo::File);
      CHECK(vfs.pathInfo("bar/bat") == PathInfo::File);
      CHECK(vfs.pathInfo("bar/baz") == PathInfo::File);
      CHECK(vfs.pathInfo("baz") == PathInfo::Directory);
      CHECK(vfs.pathInfo("bar/foo") == PathInfo::File);
      CHECK(vfs.pathInfo("bar/baz") == PathInfo::File);
      CHECK(vfs.pathInfo("bar/cat") == PathInfo::Directory);
      CHECK(vfs.pathInfo("bat") == PathInfo::Unknown);
      CHECK(vfs.pathInfo("bar/dat") == PathInfo::Unknown);
      CHECK(vfs.pathInfo("bat/foo") == PathInfo::Unknown);
    }

    SECTION("directoryContents")
    {
      CHECK_THAT(
        vfs.directoryContents(""),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "foo",
          "bar",
          "baz",
        }));
      CHECK_THAT(
        vfs.directoryContents("foo"),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "bar",
        }));
      CHECK_THAT(
        vfs.directoryContents("foo/bar"),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{"baz"}));
      CHECK_THAT(
        vfs.directoryContents("bar"),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "foo",
          "baz",
          "bat",
          "cat",
        }));
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile("foo/bar/baz") == foo_bar_baz);
      CHECK(vfs.openFile("bar/foo") == bar_foo);
      CHECK(vfs.openFile("bar/bat") == bar_bat_fs2);
      CHECK_THROWS_AS(vfs.openFile("bar/cat"), FileSystemException);
    }
  }

  SECTION("with two file systems mounted at different mount points")
  {
    auto foo_bar_baz = std::make_shared<ObjectFile<Object>>("foo/bar/baz", Object{1});
    auto bar_foo = std::make_shared<ObjectFile<Object>>("bar/foo", Object{2});

    vfs.mount(
      "foo",
      std::make_unique<TestFileSystem>(
        Entry{DirectoryEntry{
          "",
          {
            DirectoryEntry{
              "bar",
              {
                FileEntry{"baz", foo_bar_baz},
              }},
          }}},
        "/fs1"));
    vfs.mount(
      "bar",
      std::make_unique<TestFileSystem>(
        Entry{DirectoryEntry{
          "",
          {
            FileEntry{"foo", bar_foo},
          }}},
        "/fs2"));

    SECTION("makeAbsolute")
    {
      CHECK_THROWS_AS(vfs.makeAbsolute(""), FileSystemException);
      CHECK(vfs.makeAbsolute("foo/bar") == "/fs1/bar");
      CHECK(vfs.makeAbsolute("bar/foo") == "/fs2/foo");
    }

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo("") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo/bar") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo/bar/baz") == PathInfo::File);
      CHECK(vfs.pathInfo("bar") == PathInfo::Directory);
      CHECK(vfs.pathInfo("bar/foo") == PathInfo::File);
      CHECK(vfs.pathInfo("baz") == PathInfo::Unknown);
    }

    SECTION("directoryContents")
    {
      /*
      CHECK_THAT(
        vfs.directoryContents(""),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "foo",
          "bar",
        }));
        */
      CHECK_THAT(
        vfs.directoryContents("foo"),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "bar",
        }));
      CHECK_THAT(
        vfs.directoryContents("foo/bar"),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "baz",
        }));
      CHECK_THAT(
        vfs.directoryContents("bar"),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "foo",
        }));
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile("foo/bar/baz") == foo_bar_baz);
      CHECK(vfs.openFile("bar/foo") == bar_foo);
    }
  }

  SECTION("with two file systems mounted at nested mount points")
  {
    auto foo_bar_baz = std::make_shared<ObjectFile<Object>>("foo/bar/baz", Object{1});
    auto foo_bar_foo = std::make_shared<ObjectFile<Object>>("foo/bar/foo", Object{2});

    vfs.mount(
      "foo",
      std::make_unique<TestFileSystem>(
        Entry{DirectoryEntry{
          "",
          {
            DirectoryEntry{
              "bar",
              {
                FileEntry{"baz", foo_bar_baz},
              }},
          }}},
        "/fs1"));
    vfs.mount(
      "foo/bar",
      std::make_unique<TestFileSystem>(
        Entry{DirectoryEntry{
          "",
          {
            FileEntry{"foo", foo_bar_foo},
          }}},
        "/fs2"));

    SECTION("makeAbsolute")
    {
      CHECK_THROWS_AS(vfs.makeAbsolute(""), FileSystemException);
      CHECK(vfs.makeAbsolute("foo/bar") == "/fs2/");
      CHECK(vfs.makeAbsolute("foo/bar/foo") == "/fs2/foo");
      CHECK(vfs.makeAbsolute("foo/bar/baz") == "/fs1/bar/baz");
    }

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo("") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo/bar") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo/bar/foo") == PathInfo::File);
      CHECK(vfs.pathInfo("foo/bar/baz") == PathInfo::File);
    }

    SECTION("directoryContents")
    {
      CHECK_THAT(
        vfs.directoryContents(""),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "foo",
        }));
      CHECK_THAT(
        vfs.directoryContents("foo"),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "bar",
        }));
      CHECK_THAT(
        vfs.directoryContents("foo/bar"),
        Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
          "baz",
          "foo",
        }));
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile("foo/bar/baz") == foo_bar_baz);
      CHECK(vfs.openFile("foo/bar/foo") == foo_bar_foo);
    }
  }
}

} // namespace IO
} // namespace TrenchBroom
