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

#include "fs/File.h"
#include "fs/FileSystem.h"
#include "fs/TestFileSystem.h"
#include "fs/TraversalMode.h"

#include "kd/result.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::fs
{

TEST_CASE("FileSystem")
{
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
                FileEntry{"nested_dir_file_2.map", makeObjectFile(2)},
                FileEntry{"nested_dir_file_1.txt", makeObjectFile(1)},
              }},
            FileEntry{"some_dir_file_1.TXT", makeObjectFile(3)},
            FileEntry{"some_dir_file_2.doc", makeObjectFile(4)},
          }},
        FileEntry{"root_file.map", makeObjectFile(5)},
        FileEntry{"root_file.jpg", makeObjectFile(6)},
      }},
    {}};

  SECTION("makeAbsolute")
  {
    CHECK(fs.makeAbsolute("/") == Result<std::filesystem::path>{"/"});
    CHECK(fs.makeAbsolute("/foo") == Result<std::filesystem::path>{"/foo"});
  }

  SECTION("pathInfo")
  {
#if defined(_WIN32)
    CHECK(fs.pathInfo("c:\\") == fs::PathInfo::Unknown);
    CHECK(fs.pathInfo("c:\\foo") == fs::PathInfo::Unknown);
    CHECK(fs.pathInfo("c:") == fs::PathInfo::Unknown);
    CHECK(fs.pathInfo("/") == fs::PathInfo::Unknown);
    CHECK(fs.pathInfo("/foo") == fs::PathInfo::Unknown);
#else
    CHECK(fs.pathInfo("/") == fs::PathInfo::Unknown);
    CHECK(fs.pathInfo("/foo") == fs::PathInfo::Unknown);
#endif
  }

  SECTION("find")
  {
#if defined(_WIN32)
    CHECK(
      fs.find("c:\\", fs::TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{
        Error{fmt::format("Path {} is absolute", std::filesystem::path{"c:\\"})}});
    CHECK(
      fs.find("c:\\foo", fs::TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{
        Error{fmt::format("Path {} is absolute", std::filesystem::path{"c:\\foo"})}});
#else
    CHECK(
      fs.find("/", fs::TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{
        Error{fmt::format("Path {} is absolute", std::filesystem::path{"/"})}});
    CHECK(
      fs.find("/foo", fs::TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{
        Error{fmt::format("Path {} is absolute", std::filesystem::path{"/foo"})}});
#endif
    CHECK(
      fs.find("does_not_exist", fs::TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Path {} does not denote a directory",
        std::filesystem::path{"does_not_exist"})}});
    CHECK(
      fs.find("root_file.map", fs::TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Path {} does not denote a directory", std::filesystem::path{"root_file.map"})}});

    CHECK(
      fs.find("", fs::TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir",
        "root_file.map",
        "root_file.jpg",
      }});

    CHECK(
      fs.find("", fs::TraversalMode::Recursive)
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir",
        "some_dir/nested_dir",
        "some_dir/nested_dir/nested_dir_file_2.map",
        "some_dir/nested_dir/nested_dir_file_1.txt",
        "some_dir/some_dir_file_1.TXT",
        "some_dir/some_dir_file_2.doc",
        "root_file.map",
        "root_file.jpg",
      }});

    CHECK(
      fs.find("some_dir", fs::TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir/nested_dir",
        "some_dir/some_dir_file_1.TXT",
        "some_dir/some_dir_file_2.doc",
      }});

    CHECK(
      fs.find("some_dir", fs::TraversalMode::Recursive)
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir/nested_dir",
        "some_dir/nested_dir/nested_dir_file_2.map",
        "some_dir/nested_dir/nested_dir_file_1.txt",
        "some_dir/some_dir_file_1.TXT",
        "some_dir/some_dir_file_2.doc",
      }});

    CHECK(
      fs.find(
        "", fs::TraversalMode::Recursive, makeExtensionPathMatcher({".txt", ".map"}))
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir/nested_dir/nested_dir_file_2.map",
        "some_dir/nested_dir/nested_dir_file_1.txt",
        "some_dir/some_dir_file_1.TXT",
        "root_file.map",
      }});
  }

  SECTION("openFile")
  {
#if defined(_WIN32)
    CHECK(
      fs.openFile("c:\\")
      == Result<std::shared_ptr<File>>{
        Error{fmt::format("Path {} is absolute", std::filesystem::path{"c:\\"})}});
    CHECK(
      fs.openFile("c:\\foo")
      == Result<std::shared_ptr<File>>{
        Error{fmt::format("Path {} is absolute", std::filesystem::path{"c:\\foo"})}});
#else
    CHECK(
      fs.openFile("/")
      == Result<std::shared_ptr<File>>{
        Error{fmt::format("Path {} is absolute", std::filesystem::path{"/"})}});

    CHECK(
      fs.openFile("/foo")
      == Result<std::shared_ptr<File>>{
        Error{fmt::format("Path {} is absolute", std::filesystem::path{"/foo"})}});
#endif
    CHECK(
      fs.openFile("does_not_exist")
      == Result<std::shared_ptr<File>>{
        Error{fmt::format("{} not found", std::filesystem::path{"does_not_exist"})}});
  }
}

} // namespace tb::fs
