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
      CHECK_THROWS_AS(vfs.makeAbsolute(Path{}), FileSystemException);
      CHECK_THROWS_AS(vfs.makeAbsolute(Path{"foo/bar"}), FileSystemException);
    }

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo(Path{}) == PathInfo::Unknown);
      CHECK(vfs.pathInfo(Path{"foo/bar"}) == PathInfo::Unknown);
    }

    SECTION("directoryContents")
    {
      CHECK_THROWS_AS(vfs.directoryContents(Path{}), FileSystemException);
      CHECK_THROWS_AS(vfs.directoryContents(Path{"foo/bar"}), FileSystemException);
    }

    SECTION("openFile")
    {
      CHECK_THROWS_AS(vfs.openFile(Path{}), FileSystemException);
      CHECK_THROWS_AS(vfs.openFile(Path{"foo"}), FileSystemException);
      CHECK_THROWS_AS(vfs.openFile(Path{"foo/bar"}), FileSystemException);
    }
  }

  SECTION("with a file system mounted at the root")
  {
    auto foo_bar_baz =
      std::make_shared<ObjectFile<Object>>(Path{"foo/bar/baz"}, Object{1});
    auto bar_foo = std::make_shared<ObjectFile<Object>>(Path{"bar/foo"}, Object{2});

    vfs.mount(
      Path{},
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
      CHECK(vfs.makeAbsolute(Path{}) == Path{"/"});
      CHECK(vfs.makeAbsolute(Path{"foo"}) == Path{"/foo"});
      CHECK(vfs.makeAbsolute(Path{"foo/bar"}) == Path{"/foo/bar"});
    }

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo(Path{}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"foo"}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"foo/bar"}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"foo/bar/baz"}) == PathInfo::File);
      CHECK(vfs.pathInfo(Path{"foo/baz"}) == PathInfo::Unknown);
    }

    SECTION("directoryContents")
    {
      CHECK_THAT(
        vfs.directoryContents(Path{}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"foo"},
          Path{"bar"},
        }));
      CHECK_THAT(
        vfs.directoryContents(Path{"foo"}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"bar"},
        }));
      CHECK_THAT(
        vfs.directoryContents(Path{"foo/bar"}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"baz"},
        }));
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile(Path{"foo/bar/baz"}) == foo_bar_baz);
      CHECK(vfs.openFile(Path{"bar/foo"}) == bar_foo);
    }
  }

  SECTION("with two file systems mounted at the root")
  {
    auto foo_bar_baz =
      std::make_shared<ObjectFile<Object>>(Path{"foo/bar/baz"}, Object{1});
    auto bar_foo = std::make_shared<ObjectFile<Object>>(Path{"bar/foo"}, Object{2});
    auto bar_bat_fs1 = std::make_shared<ObjectFile<Object>>(Path{"bar/bat"}, Object{3});
    auto bar_bat_fs2 = std::make_shared<ObjectFile<Object>>(Path{"bar/bat"}, Object{4});
    auto bar_cat = std::make_shared<ObjectFile<Object>>(Path{"bar/cat"}, Object{5});

    vfs.mount(
      Path{},
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
        Path{"/fs1"}));
    vfs.mount(
      Path{},
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
        Path{"/fs2"}));

    SECTION("makeAbsolute")
    {
      CHECK(vfs.makeAbsolute(Path{}) == Path{"/fs2/"});
      CHECK(vfs.makeAbsolute(Path{"foo"}) == Path{"/fs1/foo"});
      CHECK(vfs.makeAbsolute(Path{"foo/bar"}) == Path{"/fs1/foo/bar"});
      CHECK(vfs.makeAbsolute(Path{"bar"}) == Path{"/fs2/bar"});
      CHECK(vfs.makeAbsolute(Path{"bar/foo"}) == Path{"/fs1/bar/foo"});
      CHECK(vfs.makeAbsolute(Path{"bar/bat"}) == Path{"/fs2/bar/bat"});
      CHECK(vfs.makeAbsolute(Path{"bar/baz"}) == Path{"/fs2/bar/baz"});
      CHECK(vfs.makeAbsolute(Path{"bar/cat"}) == Path{"/fs2/bar/cat"});
      CHECK(vfs.makeAbsolute(Path{"baz"}) == Path{"/fs2/baz"});
      CHECK(vfs.makeAbsolute(Path{"baz/foo"}) == Path{"/fs2/baz/foo"});
    }

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo(Path{}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"foo"}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"foo/bar"}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"foo/bar/baz"}) == PathInfo::File);
      CHECK(vfs.pathInfo(Path{"bar"}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"bar/foo"}) == PathInfo::File);
      CHECK(vfs.pathInfo(Path{"bar/bat"}) == PathInfo::File);
      CHECK(vfs.pathInfo(Path{"bar/baz"}) == PathInfo::File);
      CHECK(vfs.pathInfo(Path{"baz"}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"bar/foo"}) == PathInfo::File);
      CHECK(vfs.pathInfo(Path{"bar/baz"}) == PathInfo::File);
      CHECK(vfs.pathInfo(Path{"bar/cat"}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"bat"}) == PathInfo::Unknown);
      CHECK(vfs.pathInfo(Path{"bar/dat"}) == PathInfo::Unknown);
      CHECK(vfs.pathInfo(Path{"bat/foo"}) == PathInfo::Unknown);
    }

    SECTION("directoryContents")
    {
      CHECK_THAT(
        vfs.directoryContents(Path{}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"foo"},
          Path{"bar"},
          Path{"baz"},
        }));
      CHECK_THAT(
        vfs.directoryContents(Path{"foo"}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"bar"},
        }));
      CHECK_THAT(
        vfs.directoryContents(Path{"foo/bar"}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{Path{"baz"}}));
      CHECK_THAT(
        vfs.directoryContents(Path{"bar"}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"foo"},
          Path{"baz"},
          Path{"bat"},
          Path{"cat"},
        }));
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile(Path{"foo/bar/baz"}) == foo_bar_baz);
      CHECK(vfs.openFile(Path{"bar/foo"}) == bar_foo);
      CHECK(vfs.openFile(Path{"bar/bat"}) == bar_bat_fs2);
      CHECK_THROWS_AS(vfs.openFile(Path{"bar/cat"}), FileSystemException);
    }
  }

  SECTION("with two file systems mounted at different mount points")
  {
    auto foo_bar_baz =
      std::make_shared<ObjectFile<Object>>(Path{"foo/bar/baz"}, Object{1});
    auto bar_foo = std::make_shared<ObjectFile<Object>>(Path{"bar/foo"}, Object{2});

    vfs.mount(
      Path{"foo"},
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
        Path{"/fs1"}));
    vfs.mount(
      Path{"bar"},
      std::make_unique<TestFileSystem>(
        Entry{DirectoryEntry{
          "",
          {
            FileEntry{"foo", bar_foo},
          }}},
        Path{"/fs2"}));

    SECTION("makeAbsolute")
    {
      CHECK_THROWS_AS(vfs.makeAbsolute(Path{}), FileSystemException);
      CHECK(vfs.makeAbsolute(Path{"foo/bar"}) == Path{"/fs1/bar"});
      CHECK(vfs.makeAbsolute(Path{"bar/foo"}) == Path{"/fs2/foo"});
    }

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo(Path{}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"foo"}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"foo/bar"}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"foo/bar/baz"}) == PathInfo::File);
      CHECK(vfs.pathInfo(Path{"bar"}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"bar/foo"}) == PathInfo::File);
      CHECK(vfs.pathInfo(Path{"baz"}) == PathInfo::Unknown);
    }

    SECTION("directoryContents")
    {
      /*
      CHECK_THAT(
        vfs.directoryContents(Path{}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"foo"},
          Path{"bar"},
        }));
        */
      CHECK_THAT(
        vfs.directoryContents(Path{"foo"}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"bar"},
        }));
      CHECK_THAT(
        vfs.directoryContents(Path{"foo/bar"}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"baz"},
        }));
      CHECK_THAT(
        vfs.directoryContents(Path{"bar"}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"foo"},
        }));
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile(Path{"foo/bar/baz"}) == foo_bar_baz);
      CHECK(vfs.openFile(Path{"bar/foo"}) == bar_foo);
    }
  }

  SECTION("with two file systems mounted at nested mount points")
  {
    auto foo_bar_baz =
      std::make_shared<ObjectFile<Object>>(Path{"foo/bar/baz"}, Object{1});
    auto foo_bar_foo =
      std::make_shared<ObjectFile<Object>>(Path{"foo/bar/foo"}, Object{2});

    vfs.mount(
      Path{"foo"},
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
        Path{"/fs1"}));
    vfs.mount(
      Path{"foo/bar"},
      std::make_unique<TestFileSystem>(
        Entry{DirectoryEntry{
          "",
          {
            FileEntry{"foo", foo_bar_foo},
          }}},
        Path{"/fs2"}));

    SECTION("makeAbsolute")
    {
      CHECK_THROWS_AS(vfs.makeAbsolute(Path{}), FileSystemException);
      CHECK(vfs.makeAbsolute(Path{"foo/bar"}) == Path{"/fs2/"});
      CHECK(vfs.makeAbsolute(Path{"foo/bar/foo"}) == Path{"/fs2/foo"});
      CHECK(vfs.makeAbsolute(Path{"foo/bar/baz"}) == Path{"/fs1/bar/baz"});
    }

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo(Path{}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"foo"}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"foo/bar"}) == PathInfo::Directory);
      CHECK(vfs.pathInfo(Path{"foo/bar/foo"}) == PathInfo::File);
      CHECK(vfs.pathInfo(Path{"foo/bar/baz"}) == PathInfo::File);
    }

    SECTION("directoryContents")
    {
      CHECK_THAT(
        vfs.directoryContents(Path{}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"foo"},
        }));
      CHECK_THAT(
        vfs.directoryContents(Path{"foo"}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"bar"},
        }));
      CHECK_THAT(
        vfs.directoryContents(Path{"foo/bar"}),
        Catch::Matchers::UnorderedEquals(std::vector<Path>{
          Path{"baz"},
          Path{"foo"},
        }));
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile(Path{"foo/bar/baz"}) == foo_bar_baz);
      CHECK(vfs.openFile(Path{"foo/bar/foo"}) == foo_bar_foo);
    }
  }
}

} // namespace IO
} // namespace TrenchBroom
