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

#include "io/File.h"
#include "io/FileSystemMetadata.h"
#include "io/TestFileSystem.h"
#include "io/TraversalMode.h"
#include "io/VirtualFileSystem.h"

#include "kdl/result.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "catch/Matchers.h"

#include "Catch2.h"

namespace tb::io
{

TEST_CASE("VirtualFileSystem")
{
  auto vfs = VirtualFileSystem{};

  SECTION("if nothing is mounted")
  {
    SECTION("makeAbsolute")
    {
      CHECK(
        vfs.makeAbsolute("")
        == Result<std::filesystem::path>{Error{fmt::format(
          "Failed to make absolute path of {}",
          fmt::streamed(std::filesystem::path{""}))}});
      CHECK(
        vfs.makeAbsolute("foo/bar")
        == Result<std::filesystem::path>{Error{fmt::format(
          "Failed to make absolute path of {}",
          fmt::streamed(std::filesystem::path{"foo/bar"}))}});
    }

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo("") == PathInfo::Unknown);
      CHECK(vfs.pathInfo("foo/bar") == PathInfo::Unknown);
    }

    SECTION("find")
    {
      CHECK(
        vfs.find("", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
          "Path {} does not denote a directory",
          fmt::streamed(std::filesystem::path{""}))}});
      CHECK(
        vfs.find("foo/bar", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
          "Path {} does not denote a directory",
          fmt::streamed(std::filesystem::path{"foo/bar"}))}});
      ;
    }

    SECTION("openFile")
    {
      CHECK(
        vfs.openFile("")
        == Result<std::shared_ptr<File>>{
          Error{fmt::format("{} not found", fmt::streamed(std::filesystem::path{""}))}});
      CHECK(
        vfs.openFile("foo")
        == Result<std::shared_ptr<File>>{Error{
          fmt::format("{} not found", fmt::streamed(std::filesystem::path{"foo"}))}});
      CHECK(
        vfs.openFile("foo/bar")
        == Result<std::shared_ptr<File>>{Error{
          fmt::format("{} not found", fmt::streamed(std::filesystem::path{"foo/bar"}))}});
    }
  }

  SECTION("with a file system mounted at the root")
  {
    auto foo_bar_baz = makeObjectFile(1);
    auto bar_foo = makeObjectFile(2);
    auto md = std::unordered_map<std::string, FileSystemMetadata>{
      {"key1", FileSystemMetadata{std::filesystem::path{"/some/path"}}},
    };

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
              }},
          }}},
        md));

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

    SECTION("metadata")
    {
      CHECK_THAT(vfs.metadata("", "key1"), MatchesPointer(md.at("key1")));
      CHECK_THAT(vfs.metadata("foo", "key1"), MatchesPointer(md.at("key1")));
      CHECK_THAT(vfs.metadata("foo/bar", "key1"), MatchesPointer(md.at("key1")));
      CHECK(vfs.metadata("foo/bar", "key2") == nullptr);
      CHECK(vfs.metadata("does/not/exist", "key1") == nullptr);
    }

    SECTION("find")
    {
      CHECK(
        vfs.find("", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "bar",
        }});
      CHECK(
        vfs.find("foo", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar",
        }});
      CHECK(
        vfs.find("foo/bar", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar/baz",
        }});
      CHECK(
        vfs.find("", TraversalMode::Recursive)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "foo/bar",
          "foo/bar/baz",
          "bar",
          "bar/foo",
        }});
      CHECK(
        vfs.find("", TraversalMode{0})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "bar",
        }});
      CHECK(
        vfs.find("", TraversalMode{1})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "foo/bar",
          "bar",
          "bar/foo",
        }});
      CHECK(
        vfs.find("foo", TraversalMode{1})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar",
          "foo/bar/baz",
        }});
      CHECK(
        vfs.find("foo/bar", TraversalMode{1})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar/baz",
        }});
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile("foo/bar/baz") == Result<std::shared_ptr<File>>{foo_bar_baz});
      CHECK(vfs.openFile("bar/foo") == Result<std::shared_ptr<File>>{bar_foo});
    }
  }

  SECTION("with two file systems mounted at the root")
  {
    auto foo_bar_baz = makeObjectFile(1);
    auto bar_foo = makeObjectFile(2);
    auto bar_bat_fs1 = makeObjectFile(3);
    auto bar_bat_fs2 = makeObjectFile(4);
    auto bar_cat = makeObjectFile(5);

    auto md_fs1 = std::unordered_map<std::string, FileSystemMetadata>{
      {"key1", FileSystemMetadata{std::filesystem::path{"/some/path"}}},
    };
    auto md_fs2 = std::unordered_map<std::string, FileSystemMetadata>{
      {"key1", FileSystemMetadata{std::filesystem::path{"/some/other/path"}}},
      {"key2", FileSystemMetadata{std::filesystem::path{"/yet_another/path"}}},
    };

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
        md_fs1,
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
        md_fs2,
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

    SECTION("metadata")
    {
      CHECK_THAT(vfs.metadata("", "key1"), MatchesPointer(md_fs2.at("key1")));
      CHECK_THAT(vfs.metadata("bar/foo", "key1"), MatchesPointer(md_fs1.at("key1")));
      CHECK(vfs.metadata("bar/foo", "key2") == nullptr);
      CHECK_THAT(vfs.metadata("bar/bat", "key1"), MatchesPointer(md_fs2.at("key1")));
      CHECK_THAT(vfs.metadata("bar/bat", "key2"), MatchesPointer(md_fs2.at("key2")));
      CHECK_THAT(vfs.metadata("bar/baz", "key1"), MatchesPointer(md_fs2.at("key1")));
      CHECK_THAT(vfs.metadata("bar/baz", "key2"), MatchesPointer(md_fs2.at("key2")));
    }

    SECTION("find")
    {
      CHECK(
        vfs.find("", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "bar",
          "baz",
        }});
      CHECK(
        vfs.find("foo", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar",
        }});
      CHECK(
        vfs.find("foo/bar", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{
          std::vector<std::filesystem::path>{"foo/bar/baz"}});
      CHECK(
        vfs.find("bar", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "bar/foo",
          "bar/bat",
          "bar/baz",
          "bar/cat",
        }});
      CHECK(
        vfs.find("", TraversalMode::Recursive)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "foo/bar",
          "foo/bar/baz",
          "bar/foo",
          "bar",
          "bar/bat",
          "bar/baz",
          "bar/cat",
          "baz",
          "baz/foo",
        }});
      CHECK(
        vfs.find("", TraversalMode{0})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "bar",
          "baz",
        }});
      CHECK(
        vfs.find("", TraversalMode{1})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "foo/bar",
          "bar/foo",
          "bar",
          "bar/bat",
          "bar/baz",
          "bar/cat",
          "baz",
          "baz/foo",
        }});
      CHECK(
        vfs.find("bar", TraversalMode{1})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "bar/foo",
          "bar/bat",
          "bar/baz",
          "bar/cat",
        }});
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile("foo/bar/baz") == Result<std::shared_ptr<File>>{foo_bar_baz});
      CHECK(vfs.openFile("bar/foo") == Result<std::shared_ptr<File>>{bar_foo});
      CHECK(vfs.openFile("bar/bat") == Result<std::shared_ptr<File>>{bar_bat_fs2});
      CHECK(
        vfs.openFile("bar/cat")
        == Result<std::shared_ptr<File>>{Error{
          fmt::format("{} not found", fmt::streamed(std::filesystem::path{"bar/cat"}))}});
    }
  }

  SECTION("with two file systems mounted at different mount points")
  {
    auto foo_bar_baz = makeObjectFile(1);
    auto bar_foo = makeObjectFile(2);

    auto md_fs1 = std::unordered_map<std::string, FileSystemMetadata>{
      {"key1", FileSystemMetadata{std::filesystem::path{"/some/path"}}},
    };
    auto md_fs2 = std::unordered_map<std::string, FileSystemMetadata>{
      {"key1", FileSystemMetadata{std::filesystem::path{"/some/other/path"}}},
      {"key2", FileSystemMetadata{std::filesystem::path{"/yet_another/path"}}},
    };


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
        md_fs1,
        "/fs1"));
    vfs.mount(
      "bar",
      std::make_unique<TestFileSystem>(
        Entry{DirectoryEntry{
          "",
          {
            FileEntry{"foo", bar_foo},
          }}},
        md_fs2,
        "/fs2"));

    SECTION("makeAbsolute")
    {
      CHECK(
        vfs.makeAbsolute("")
        == Result<std::filesystem::path>{Error{fmt::format(
          "Failed to make absolute path of {}",
          fmt::streamed(std::filesystem::path{""}))}});
      CHECK(vfs.makeAbsolute("foo/bar") == Result<std::filesystem::path>{"/fs1/bar"});
      CHECK(vfs.makeAbsolute("bar/foo") == Result<std::filesystem::path>{"/fs2/foo"});
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

    SECTION("metadata")
    {
      CHECK(vfs.metadata("", "key1") == nullptr);
      CHECK(vfs.metadata("baz", "key1") == nullptr);
      CHECK_THAT(vfs.metadata("foo", "key1"), MatchesPointer(md_fs1.at("key1")));
      CHECK_THAT(vfs.metadata("foo/bar", "key1"), MatchesPointer(md_fs1.at("key1")));
      CHECK_THAT(vfs.metadata("bar", "key2"), MatchesPointer(md_fs2.at("key2")));
      CHECK_THAT(vfs.metadata("bar/foo", "key2"), MatchesPointer(md_fs2.at("key2")));
    }

    SECTION("find")
    {
      CHECK(
        vfs.find("", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "bar",
        }});
      CHECK(
        vfs.find("foo", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar",
        }});
      CHECK(
        vfs.find("foo/bar", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar/baz",
        }});
      CHECK(
        vfs.find("bar", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "bar/foo",
        }});
      CHECK(
        vfs.find("", TraversalMode::Recursive)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "foo/bar",
          "foo/bar/baz",
          "bar",
          "bar/foo",
        }});
      CHECK(
        vfs.find("", TraversalMode{1})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "foo/bar",
          "bar",
          "bar/foo",
        }});
      CHECK(
        vfs.find("foo", TraversalMode{1})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar",
          "foo/bar/baz",
        }});
      CHECK(
        vfs.find("foo/bar", TraversalMode{1})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar/baz",
        }});
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile("foo/bar/baz") == Result<std::shared_ptr<File>>{foo_bar_baz});
      CHECK(vfs.openFile("bar/foo") == Result<std::shared_ptr<File>>{bar_foo});
    }
  }

  SECTION("with two file systems mounted at nested mount points")
  {
    auto foo_bar_baz = makeObjectFile(1);
    auto foo_bar_foo = makeObjectFile(2);

    auto md_fs1 = std::unordered_map<std::string, FileSystemMetadata>{
      {"key1", FileSystemMetadata{std::filesystem::path{"/some/path"}}},
    };
    auto md_fs2 = std::unordered_map<std::string, FileSystemMetadata>{
      {"key1", FileSystemMetadata{std::filesystem::path{"/some/other/path"}}},
    };


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
        md_fs1,
        "/fs1"));
    vfs.mount(
      "foo/bar",
      std::make_unique<TestFileSystem>(
        Entry{DirectoryEntry{
          "",
          {
            FileEntry{"foo", foo_bar_foo},
          }}},
        md_fs2,
        "/fs2"));

    SECTION("makeAbsolute")
    {
      CHECK(
        vfs.makeAbsolute("")
        == Result<std::filesystem::path>{Error{fmt::format(
          "Failed to make absolute path of {}",
          fmt::streamed(std::filesystem::path{""}))}});
      CHECK(vfs.makeAbsolute("foo/bar") == Result<std::filesystem::path>{"/fs2/"});
      CHECK(vfs.makeAbsolute("foo/bar/foo") == Result<std::filesystem::path>{"/fs2/foo"});
      CHECK(
        vfs.makeAbsolute("foo/bar/baz") == Result<std::filesystem::path>{"/fs1/bar/baz"});
    }

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo("") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo/bar") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo/bar/foo") == PathInfo::File);
      CHECK(vfs.pathInfo("foo/bar/baz") == PathInfo::File);
    }

    SECTION("metadata")
    {
      CHECK(vfs.metadata("", "key1") == nullptr);
      CHECK(vfs.metadata("baz", "key1") == nullptr);
      CHECK_THAT(vfs.metadata("foo", "key1"), MatchesPointer(md_fs1.at("key1")));
      CHECK_THAT(vfs.metadata("foo/bar", "key1"), MatchesPointer(md_fs2.at("key1")));
      CHECK_THAT(vfs.metadata("foo/bar/foo", "key1"), MatchesPointer(md_fs2.at("key1")));
    }

    SECTION("find")
    {
      CHECK(
        vfs.find("", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
        }});
      CHECK(
        vfs.find("foo", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar",
        }});
      CHECK(
        vfs.find("foo/bar", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar/baz",
          "foo/bar/foo",
        }});
      CHECK(
        vfs.find("", TraversalMode::Recursive)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar/baz",
          "foo",
          "foo/bar",
          "foo/bar/foo",
        }});
      CHECK(
        vfs.find("", TraversalMode{1})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "foo/bar",
        }});
      CHECK(
        vfs.find("foo", TraversalMode{0})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar",
        }});
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile("foo/bar/baz") == Result<std::shared_ptr<File>>{foo_bar_baz});
      CHECK(vfs.openFile("foo/bar/foo") == Result<std::shared_ptr<File>>{foo_bar_foo});
    }
  }

  SECTION("with two file systems mounted at nested mount points and overriding")
  {
    auto fs1_foo_bar_a = makeObjectFile(1);
    auto fs1_foo_bar_c = makeObjectFile(2);
    auto fs1_foo_bar_e = makeObjectFile(3);
    auto fs1_foo_bar_f = makeObjectFile(4);

    auto fs2_foo_bar_b = makeObjectFile(5);
    auto fs2_foo_bar_c = makeObjectFile(6);
    auto fs2_foo_bar_d = makeObjectFile(7);
    auto fs2_foo_bar_g = makeObjectFile(8);

    auto md_fs1 = std::unordered_map<std::string, FileSystemMetadata>{
      {"key1", FileSystemMetadata{std::filesystem::path{"/some/path"}}},
    };
    auto md_fs2 = std::unordered_map<std::string, FileSystemMetadata>{
      {"key1", FileSystemMetadata{std::filesystem::path{"/some/other/path"}}},
    };


    vfs.mount(
      "foo",
      std::make_unique<TestFileSystem>(
        Entry{DirectoryEntry{
          "",
          {
            DirectoryEntry{
              "bar",
              {
                FileEntry{"a", fs1_foo_bar_a},
                FileEntry{"c", fs1_foo_bar_c}, // overridden by fs2_foo_bar_c
                FileEntry{"e", fs1_foo_bar_e},
                FileEntry{"f", fs1_foo_bar_f}, // overridden by directory in fs2
                DirectoryEntry{"g", {}},       // overridden by fs2_foo_bar_g
              }},
          }}},
        md_fs1,
        "/fs1"));
    vfs.mount(
      "foo/bar",
      std::make_unique<TestFileSystem>(
        Entry{DirectoryEntry{
          "",
          {
            FileEntry{"b", fs2_foo_bar_b},
            FileEntry{"c", fs2_foo_bar_c}, // overrides fs1_foo_bar_c
            FileEntry{"d", fs2_foo_bar_d},
            DirectoryEntry{"f", {}},       // overrides fs1_foo_bar_f
            FileEntry{"g", fs2_foo_bar_g}, // overrides directory in fs1
          }}},
        md_fs2,
        "/fs2"));

    SECTION("pathInfo")
    {
      CHECK(vfs.pathInfo("foo/bar/f") == PathInfo::Directory);
      CHECK(vfs.pathInfo("foo/bar/g") == PathInfo::File);
    }

    SECTION("metadata")
    {
      CHECK_THAT(vfs.metadata("foo", "key1"), MatchesPointer(md_fs1.at("key1")));
      CHECK_THAT(vfs.metadata("foo/bar", "key1"), MatchesPointer(md_fs2.at("key1")));
      CHECK_THAT(vfs.metadata("foo/bar/a", "key1"), MatchesPointer(md_fs1.at("key1")));
      CHECK_THAT(vfs.metadata("foo/bar/b", "key1"), MatchesPointer(md_fs2.at("key1")));
      CHECK_THAT(vfs.metadata("foo/bar/c", "key1"), MatchesPointer(md_fs2.at("key1")));
      CHECK_THAT(vfs.metadata("foo/bar/d", "key1"), MatchesPointer(md_fs2.at("key1")));
      CHECK_THAT(vfs.metadata("foo/bar/e", "key1"), MatchesPointer(md_fs1.at("key1")));
      CHECK_THAT(vfs.metadata("foo/bar/f", "key1"), MatchesPointer(md_fs2.at("key1")));
      CHECK_THAT(vfs.metadata("foo/bar/g", "key1"), MatchesPointer(md_fs2.at("key1")));
    }

    SECTION("find")
    {
      CHECK(
        vfs.find("", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
        }});
      CHECK(
        vfs.find("foo", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar",
        }});
      CHECK(
        vfs.find("foo/bar", TraversalMode::Flat)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar/a",
          "foo/bar/e",
          "foo/bar/b",
          "foo/bar/c",
          "foo/bar/d",
          "foo/bar/f",
          "foo/bar/g",
        }});
      CHECK(
        vfs.find("", TraversalMode::Recursive)
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar/a",
          "foo/bar/e",
          "foo",
          "foo/bar",
          "foo/bar/b",
          "foo/bar/c",
          "foo/bar/d",
          "foo/bar/f",
          "foo/bar/g",
        }});
      CHECK(
        vfs.find("", TraversalMode{1})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo",
          "foo/bar",
        }});
      CHECK(
        vfs.find("foo", TraversalMode{0})
        == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
          "foo/bar",
        }});
    }

    SECTION("openFile")
    {
      CHECK(vfs.openFile("foo/bar/a") == Result<std::shared_ptr<File>>{fs1_foo_bar_a});
      CHECK(vfs.openFile("foo/bar/b") == Result<std::shared_ptr<File>>{fs2_foo_bar_b});
      CHECK(vfs.openFile("foo/bar/c") == Result<std::shared_ptr<File>>{fs2_foo_bar_c});
      CHECK(vfs.openFile("foo/bar/d") == Result<std::shared_ptr<File>>{fs2_foo_bar_d});
      CHECK(vfs.openFile("foo/bar/e") == Result<std::shared_ptr<File>>{fs1_foo_bar_e});
      CHECK(
        vfs.openFile("foo/bar/f")
        == Result<std::shared_ptr<File>>{Error{fmt::format(
          "{} not found", fmt::streamed(std::filesystem::path{"foo/bar/f"}))}});
      CHECK(vfs.openFile("foo/bar/g") == Result<std::shared_ptr<File>>{fs2_foo_bar_g});
    }
  }
}

} // namespace tb::io
