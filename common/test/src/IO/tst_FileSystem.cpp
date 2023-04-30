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
                makeObjectFile(Path{"some_dir/nested_dir/nested_dir_file_1.txt"}, 1)},
              FileEntry{
                "nested_dir_file_2.map",
                makeObjectFile(Path{"some_dir/nested_dir/nested_dir_file_2.map"}, 2)},
            }},
          FileEntry{
            "some_dir_file_1.TXT",
            makeObjectFile(Path{"some_dir/some_dir_file_1.TXT"}, 3)},
          FileEntry{
            "some_dir_file_2.doc",
            makeObjectFile(Path{"some_dir/some_dir_file_2.doc"}, 4)},
        }},
      FileEntry{"root_file_1.map", makeObjectFile(Path{"root_file_1.map"}, 5)},
      FileEntry{"root_file_2.jpg", makeObjectFile(Path{"root_file_2.jpg"}, 6)},
    }}};

  SECTION("makeAbsolute")
  {
    CHECK_THROWS_AS(fs.makeAbsolute(Path{"/"}), FileSystemException);
    CHECK_THROWS_AS(fs.makeAbsolute(Path{"/foo"}), FileSystemException);
  }

  SECTION("pathInfo")
  {
    CHECK_THROWS_AS(fs.pathInfo(Path{"/"}), FileSystemException);
    CHECK_THROWS_AS(fs.pathInfo(Path{"/foo"}), FileSystemException);
  }

  SECTION("find")
  {
    CHECK_THROWS_AS(fs.find(Path{"/"}), FileSystemException);
    CHECK_THROWS_AS(fs.find(Path{"/foo"}), FileSystemException);
    CHECK_THROWS_AS(fs.find(Path{"does_not_exist"}), FileSystemException);
    CHECK_THROWS_AS(fs.find(Path{"root_file_1.map"}), FileSystemException);

    CHECK_THAT(
      fs.find(Path{}),
      Catch::Matchers::UnorderedEquals(std::vector<Path>{
        Path{"some_dir"},
        Path{"root_file_1.map"},
        Path{"root_file_2.jpg"},
      }));

    CHECK_THAT(
      fs.findRecursively(Path{}),
      Catch::Matchers::UnorderedEquals(std::vector<Path>{
        Path{"some_dir"},
        Path{"some_dir/nested_dir"},
        Path{"some_dir/nested_dir/nested_dir_file_1.txt"},
        Path{"some_dir/nested_dir/nested_dir_file_2.map"},
        Path{"some_dir/some_dir_file_1.TXT"},
        Path{"some_dir/some_dir_file_2.doc"},
        Path{"root_file_1.map"},
        Path{"root_file_2.jpg"},
      }));

    CHECK_THAT(
      fs.find(Path{"some_dir"}),
      Catch::Matchers::UnorderedEquals(std::vector<Path>{
        Path{"some_dir/nested_dir"},
        Path{"some_dir/some_dir_file_1.TXT"},
        Path{"some_dir/some_dir_file_2.doc"},
      }));

    CHECK_THAT(
      fs.findRecursively(Path{"some_dir"}),
      Catch::Matchers::UnorderedEquals(std::vector<Path>{
        Path{"some_dir/nested_dir"},
        Path{"some_dir/nested_dir/nested_dir_file_1.txt"},
        Path{"some_dir/nested_dir/nested_dir_file_2.map"},
        Path{"some_dir/some_dir_file_1.TXT"},
        Path{"some_dir/some_dir_file_2.doc"},
      }));

    CHECK_THAT(
      fs.findRecursively(Path{}, makeExtensionPathMatcher({"txt", "map"})),
      Catch::Matchers::UnorderedEquals(std::vector<Path>{
        Path{"some_dir/nested_dir/nested_dir_file_1.txt"},
        Path{"some_dir/nested_dir/nested_dir_file_2.map"},
        Path{"some_dir/some_dir_file_1.TXT"},
        Path{"root_file_1.map"},
      }));
  }

  SECTION("directoryContents")
  {
    CHECK_THROWS_AS(fs.directoryContents(Path{"/"}), FileSystemException);
    CHECK_THROWS_AS(fs.directoryContents(Path{"/foo"}), FileSystemException);
    CHECK_THROWS_AS(fs.directoryContents(Path{"does_not_exist"}), FileSystemException);
    CHECK_THROWS_AS(fs.directoryContents(Path{"root_file_1.map"}), FileSystemException);
  }

  SECTION("openFile")
  {
    CHECK_THROWS_AS(fs.openFile(Path{"/"}), FileSystemException);
    CHECK_THROWS_AS(fs.openFile(Path{"/foo"}), FileSystemException);
    CHECK_THROWS_AS(fs.openFile(Path{"does_not_exist"}), FileSystemException);
  }
}

} // namespace IO
} // namespace TrenchBroom
