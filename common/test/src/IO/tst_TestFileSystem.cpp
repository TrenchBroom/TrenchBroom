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

#include "Error.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "IO/TraversalMode.h"
#include "TestFileSystem.h"

#include <kdl/result.h>
#include <kdl/result_io.h>

#include <filesystem>

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("TestFileSystem")
{
  auto root_file_1 = makeObjectFile(1);
  auto root_file_2 = makeObjectFile(2);
  auto some_dir_file_1 = makeObjectFile(3);
  auto some_dir_file_2 = makeObjectFile(4);
  auto nested_dir_file_1 = makeObjectFile(5);
  auto nested_dir_file_2 = makeObjectFile(6);

  auto fs = TestFileSystem{DirectoryEntry{
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
    }}};

  SECTION("makeAbsolute")
  {
    CHECK(
      fs.makeAbsolute("root_file_1")
      == kdl::result<std::filesystem::path, Error>{"/root_file_1"});
    CHECK(
      fs.makeAbsolute("some_dir")
      == kdl::result<std::filesystem::path, Error>{"/some_dir"});
    CHECK(
      fs.makeAbsolute("some_dir/some_dir_file_1")
      == kdl::result<std::filesystem::path, Error>{"/some_dir/some_dir_file_1"});
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

  SECTION("find")
  {
    CHECK(
      fs.find("does_not_exist", TraversalMode::Flat)
      == kdl::result<std::vector<std::filesystem::path>, Error>{
        Error{"Path does not denote a directory: 'does_not_exist'"}});

    CHECK(
      fs.find("", TraversalMode::Flat)
      == kdl::result<std::vector<std::filesystem::path>, Error>{
        std::vector<std::filesystem::path>{
          "root_file_1",
          "root_file_2",
          "some_dir",
        }});

    CHECK(
      fs.find("some_dir", TraversalMode::Flat)
      == kdl::result<std::vector<std::filesystem::path>, Error>{
        std::vector<std::filesystem::path>{
          "some_dir/nested_dir",
          "some_dir/some_dir_file_1",
          "some_dir/some_dir_file_2",
        }});

    CHECK(
      fs.find("some_dir/nested_dir", TraversalMode::Flat)
      == kdl::result<std::vector<std::filesystem::path>, Error>{
        std::vector<std::filesystem::path>{
          "some_dir/nested_dir/nested_dir_file_1",
          "some_dir/nested_dir/nested_dir_file_2",
        }});

    CHECK(
      fs.find("", TraversalMode::Recursive)
      == kdl::result<std::vector<std::filesystem::path>, Error>{
        std::vector<std::filesystem::path>{
          "root_file_1",
          "root_file_2",
          "some_dir",
          "some_dir/nested_dir",
          "some_dir/nested_dir/nested_dir_file_1",
          "some_dir/nested_dir/nested_dir_file_2",
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

} // namespace IO
} // namespace TrenchBroom
