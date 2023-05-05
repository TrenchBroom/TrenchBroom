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
#include "IO/FileSystem.h"
#include "TestFileSystem.h"

#include "Catch2.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("FileSystem")
{
  auto fs = TestFileSystem{DirectoryEntry{
    "",
    {
      DirectoryEntry{
        "some_dir",
        {
          DirectoryEntry{
            "nested_dir",
            {
              FileEntry{
                "nested_dir_file_1.txt",
                makeObjectFile("some_dir/nested_dir/nested_dir_file_1.txt", 1)},
              FileEntry{
                "nested_dir_file_2.map",
                makeObjectFile("some_dir/nested_dir/nested_dir_file_2.map", 2)},
            }},
          FileEntry{
            "some_dir_file_1.TXT", makeObjectFile("some_dir/some_dir_file_1.TXT", 3)},
          FileEntry{
            "some_dir_file_2.doc", makeObjectFile("some_dir/some_dir_file_2.doc", 4)},
        }},
      FileEntry{"root_file_1.map", makeObjectFile("root_file_1.map", 5)},
      FileEntry{"root_file_2.jpg", makeObjectFile("root_file_2.jpg", 6)},
    }}};

  SECTION("makeAbsolute")
  {
    CHECK_THROWS_AS(fs.makeAbsolute("/"), FileSystemException);
    CHECK_THROWS_AS(fs.makeAbsolute("/foo"), FileSystemException);
  }

  SECTION("pathInfo")
  {
#if defined(_WIN32)
    CHECK_THROWS_AS(fs.pathInfo("c:\\"), FileSystemException);
    CHECK_THROWS_AS(fs.pathInfo("c:\\foo"), FileSystemException);
    CHECK(fs.pathInfo("c:") == PathInfo::Unknown);
    CHECK(fs.pathInfo("/") == PathInfo::Unknown);
    CHECK(fs.pathInfo("/foo") == PathInfo::Unknown);
#else
    CHECK_THROWS_AS(fs.pathInfo("/"), FileSystemException);
    CHECK_THROWS_AS(fs.pathInfo("/foo"), FileSystemException);
#endif
  }

  SECTION("find")
  {
    CHECK_THROWS_AS(fs.find("/"), FileSystemException);
    CHECK_THROWS_AS(fs.find("/foo"), FileSystemException);
    CHECK_THROWS_AS(fs.find("does_not_exist"), FileSystemException);
    CHECK_THROWS_AS(fs.find("root_file_1.map"), FileSystemException);

    CHECK_THAT(
      fs.find(""),
      Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
        "some_dir",
        "root_file_1.map",
        "root_file_2.jpg",
      }));

    CHECK_THAT(
      fs.findRecursively(""),
      Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
        "some_dir",
        "some_dir/nested_dir",
        "some_dir/nested_dir/nested_dir_file_1.txt",
        "some_dir/nested_dir/nested_dir_file_2.map",
        "some_dir/some_dir_file_1.TXT",
        "some_dir/some_dir_file_2.doc",
        "root_file_1.map",
        "root_file_2.jpg",
      }));

    CHECK_THAT(
      fs.find("some_dir"),
      Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
        "some_dir/nested_dir",
        "some_dir/some_dir_file_1.TXT",
        "some_dir/some_dir_file_2.doc",
      }));

    CHECK_THAT(
      fs.findRecursively("some_dir"),
      Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
        "some_dir/nested_dir",
        "some_dir/nested_dir/nested_dir_file_1.txt",
        "some_dir/nested_dir/nested_dir_file_2.map",
        "some_dir/some_dir_file_1.TXT",
        "some_dir/some_dir_file_2.doc",
      }));

    CHECK_THAT(
      fs.findRecursively("", makeExtensionPathMatcher({".txt", ".map"})),
      Catch::Matchers::UnorderedEquals(std::vector<std::filesystem::path>{
        "some_dir/nested_dir/nested_dir_file_1.txt",
        "some_dir/nested_dir/nested_dir_file_2.map",
        "some_dir/some_dir_file_1.TXT",
        "root_file_1.map",
      }));
  }

  SECTION("directoryContents")
  {
    CHECK_THROWS_AS(fs.directoryContents("/"), FileSystemException);
    CHECK_THROWS_AS(fs.directoryContents("/foo"), FileSystemException);
    CHECK_THROWS_AS(fs.directoryContents("does_not_exist"), FileSystemException);
    CHECK_THROWS_AS(fs.directoryContents("root_file_1.map"), FileSystemException);
  }

  SECTION("openFile")
  {
    CHECK_THROWS_AS(fs.openFile("/"), FileSystemException);
    CHECK_THROWS_AS(fs.openFile("/foo"), FileSystemException);
    CHECK_THROWS_AS(fs.openFile("does_not_exist"), FileSystemException);
  }
}

} // namespace IO
} // namespace TrenchBroom
