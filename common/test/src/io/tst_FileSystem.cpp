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
#include "io/File.h"
#include "io/FileSystem.h"
#include "io/TraversalMode.h"

#include "kdl/result.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "Catch2.h"

namespace tb::io
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
    CHECK(fs.pathInfo("c:\\") == PathInfo::Unknown);
    CHECK(fs.pathInfo("c:\\foo") == PathInfo::Unknown);
    CHECK(fs.pathInfo("c:") == PathInfo::Unknown);
    CHECK(fs.pathInfo("/") == PathInfo::Unknown);
    CHECK(fs.pathInfo("/foo") == PathInfo::Unknown);
#else
    CHECK(fs.pathInfo("/") == PathInfo::Unknown);
    CHECK(fs.pathInfo("/foo") == PathInfo::Unknown);
#endif
  }

  SECTION("find")
  {
#if defined(_WIN32)
    CHECK(
      fs.find("c:\\", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"c:\\"}))}});
    CHECK(
      fs.find("c:\\foo", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"c:\\foo"}))}});
#else
    CHECK(
      fs.find("/", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{
        fmt::format("Path {} is absolute", fmt::streamed(std::filesystem::path{"/"}))}});
    CHECK(
      fs.find("/foo", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"/foo"}))}});
#endif
    CHECK(
      fs.find("does_not_exist", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Path {} does not denote a directory",
        fmt::streamed(std::filesystem::path{"does_not_exist"}))}});
    CHECK(
      fs.find("root_file.map", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{Error{fmt::format(
        "Path {} does not denote a directory",
        fmt::streamed(std::filesystem::path{"root_file.map"}))}});

    CHECK(
      fs.find("", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir",
        "root_file.map",
        "root_file.jpg",
      }});

    CHECK(
      fs.find("", TraversalMode::Recursive)
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
      fs.find("some_dir", TraversalMode::Flat)
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir/nested_dir",
        "some_dir/some_dir_file_1.TXT",
        "some_dir/some_dir_file_2.doc",
      }});

    CHECK(
      fs.find("some_dir", TraversalMode::Recursive)
      == Result<std::vector<std::filesystem::path>>{std::vector<std::filesystem::path>{
        "some_dir/nested_dir",
        "some_dir/nested_dir/nested_dir_file_2.map",
        "some_dir/nested_dir/nested_dir_file_1.txt",
        "some_dir/some_dir_file_1.TXT",
        "some_dir/some_dir_file_2.doc",
      }});

    CHECK(
      fs.find("", TraversalMode::Recursive, makeExtensionPathMatcher({".txt", ".map"}))
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
      == Result<std::shared_ptr<File>>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"c:\\"}))}});
    CHECK(
      fs.openFile("c:\\foo")
      == Result<std::shared_ptr<File>>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"c:\\foo"}))}});
#else
    CHECK(
      fs.openFile("/")
      == Result<std::shared_ptr<File>>{Error{
        fmt::format("Path {} is absolute", fmt::streamed(std::filesystem::path{"/"}))}});

    CHECK(
      fs.openFile("/foo")
      == Result<std::shared_ptr<File>>{Error{fmt::format(
        "Path {} is absolute", fmt::streamed(std::filesystem::path{"/foo"}))}});
#endif
    CHECK(
      fs.openFile("does_not_exist")
      == Result<std::shared_ptr<File>>{Error{fmt::format(
        "{} not found", fmt::streamed(std::filesystem::path{"does_not_exist"}))}});
  }
}

} // namespace tb::io
