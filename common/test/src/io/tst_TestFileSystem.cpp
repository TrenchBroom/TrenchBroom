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

#include "TestFileSystem.h"
#include "io/FileSystem.h"
#include "io/TraversalMode.h"

#include "kdl/result.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <filesystem>

#include "catch/Matchers.h"

#include "Catch2.h"

namespace tb::io
{

TEST_CASE("TestFileSystem")
{
  auto root_file_1 = makeObjectFile(1);
  auto root_file_2 = makeObjectFile(2);
  auto some_dir_file_1 = makeObjectFile(3);
  auto some_dir_file_2 = makeObjectFile(4);
  auto nested_dir_file_1 = makeObjectFile(5);
  auto nested_dir_file_2 = makeObjectFile(6);

  auto metadata = std::unordered_map<std::string, FileSystemMetadata>{
    {"key1", FileSystemMetadata{std::filesystem::path{"/some/path"}}},
  };

  auto fs = TestFileSystem{
    DirectoryEntry{
      "",
      {
        DirectoryEntry{
          "some_dir",
          {
            DirectoryEntry{
              "nested_dir",
              {
                FileEntry{"nested_dir_file_1", nested_dir_file_1},
                FileEntry{"nested_dir_file_2", nested_dir_file_2},
              }},
            FileEntry{"some_dir_file_1", some_dir_file_1},
            FileEntry{"some_dir_file_2", some_dir_file_2},
          }},
        FileEntry{"root_file_1", root_file_1},
        FileEntry{"root_file_2", root_file_2},
      }},
    metadata};

  SECTION("makeAbsolute")
  {
    CHECK(
      fs.makeAbsolute("root_file_1") == Result<std::filesystem::path>{"/root_file_1"});
    CHECK(fs.makeAbsolute("some_dir") == Result<std::filesystem::path>{"/some_dir"});
    CHECK(
      fs.makeAbsolute("some_dir/some_dir_file_1")
      == Result<std::filesystem::path>{"/some_dir/some_dir_file_1"});
  }

  SECTION("pathInfo")
  {
    CHECK(fs.pathInfo("root_file_1") == PathInfo::File);
    CHECK(fs.pathInfo("some_dir") == PathInfo::Directory);
    CHECK(fs.pathInfo("does_not_exist") == PathInfo::Unknown);
    CHECK(fs.pathInfo("some_dir/some_dir_file_1") == PathInfo::File);
    CHECK(fs.pathInfo("some_dir/nested_dir") == PathInfo::Directory);
    CHECK(fs.pathInfo("some_dir/does_not_exist") == PathInfo::Unknown);
    CHECK(fs.pathInfo("some_dir/nested_dir/nested_dir_file_1") == PathInfo::File);
    CHECK(fs.pathInfo("some_dir/nested_dir/does_not_exist") == PathInfo::Unknown);
  }

  SECTION("metadata")
  {
    CHECK_THAT(fs.metadata("root_file_1", "key1"), MatchesPointer(metadata.at("key1")));
    CHECK_THAT(fs.metadata("some_dir", "key1"), MatchesPointer(metadata.at("key1")));
    CHECK_THAT(
      fs.metadata("some_dir/some_dir_file_1", "key1"),
      MatchesPointer(metadata.at("key1")));
    CHECK_THAT(
      fs.metadata("some_dir/nested_dir", "key1"), MatchesPointer(metadata.at("key1")));
    CHECK(fs.metadata("root_file_1", "key2") == nullptr);
    CHECK(fs.metadata("does_not_exist", "key1") == nullptr);
    CHECK(fs.metadata("some_dir/does_not_exist", "key1") == nullptr);
  }

  SECTION("find")
  {
    CHECK(
      fs.find("does_not_exist", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Path {} does not denote a directory",
        fmt::streamed(std::filesystem::path{"does_not_exist"}))}});

    CHECK(
      fs.find("", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir",
        "root_file_1",
        "root_file_2",
      }});

    CHECK(
      fs.find("some_dir", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir/nested_dir",
        "some_dir/some_dir_file_1",
        "some_dir/some_dir_file_2",
      }});

    CHECK(
      fs.find("some_dir/nested_dir", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir/nested_dir/nested_dir_file_1",
        "some_dir/nested_dir/nested_dir_file_2",
      }});

    CHECK(
      fs.find("", TraversalMode::Recursive)
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir",
        "some_dir/nested_dir",
        "some_dir/nested_dir/nested_dir_file_1",
        "some_dir/nested_dir/nested_dir_file_2",
        "some_dir/some_dir_file_1",
        "some_dir/some_dir_file_2",
        "root_file_1",
        "root_file_2",
      }});

    CHECK(
      fs.find("", TraversalMode{0})
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir",
        "root_file_1",
        "root_file_2",
      }});

    CHECK(
      fs.find("", TraversalMode{1})
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir",
        "some_dir/nested_dir",
        "some_dir/some_dir_file_1",
        "some_dir/some_dir_file_2",
        "root_file_1",
        "root_file_2",
      }});

    CHECK(
      fs.find("some_dir", TraversalMode{0})
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir/nested_dir",
        "some_dir/some_dir_file_1",
        "some_dir/some_dir_file_2",
      }});
  }

  SECTION("openFile")
  {
    CHECK(fs.openFile("root_file_1") == root_file_1);
    CHECK(fs.openFile("some_dir/some_dir_file_1") == some_dir_file_1);
    CHECK(fs.openFile("some_dir/nested_dir/nested_dir_file_1") == nested_dir_file_1);
  }
}

} // namespace tb::io
