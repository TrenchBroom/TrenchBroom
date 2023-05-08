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

#include "Catch2.h"
#include "IO/File.h"
#include "IO/FileSystem.h"
#include "TestFileSystem.h"

namespace TrenchBroom
{
namespace IO
{
TEST_CASE("TestFileSystem")
{
  auto root_file_1 = makeObjectFile(Path{"root_file_1"}, 1);
  auto root_file_2 = makeObjectFile(Path{"root_file_2"}, 2);
  auto some_dir_file_1 = makeObjectFile(Path{"some_dir/some_dir_file_1"}, 3);
  auto some_dir_file_2 = makeObjectFile(Path{"some_dir/some_dir_file_2"}, 4);
  auto nested_dir_file_1 =
    makeObjectFile(Path{"some_dir/nested_dir/nested_dir_file_1"}, 5);
  auto nested_dir_file_2 =
    makeObjectFile(Path{"some_dir/nested_dir/nested_dir_file_2"}, 6);

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
    CHECK(fs.makeAbsolute(Path{"root_file_1"}) == Path{"/root_file_1"});
    CHECK(fs.makeAbsolute(Path{"some_dir"}) == Path{"/some_dir"});
    CHECK(
      fs.makeAbsolute(Path{"some_dir/some_dir_file_1"})
      == Path{"/some_dir/some_dir_file_1"});
  }

  SECTION("pathInfo")
  {
    CHECK(fs.pathInfo(Path{"root_file_1"}) == PathInfo::File);
    CHECK(fs.pathInfo(Path{"some_dir"}) == PathInfo::Directory);
    CHECK(fs.pathInfo(Path{"does_not_exist"}) == PathInfo::Unknown);
    CHECK(fs.pathInfo(Path{"some_dir/some_dir_file_1"}) == PathInfo::File);
    CHECK(fs.pathInfo(Path{"some_dir/nested_dir"}) == PathInfo::Directory);
    CHECK(fs.pathInfo(Path{"some_dir/does_not_exist"}) == PathInfo::Unknown);
    CHECK(fs.pathInfo(Path{"some_dir/nested_dir/nested_dir_file_1"}) == PathInfo::File);
    CHECK(fs.pathInfo(Path{"some_dir/nested_dir/does_not_exist"}) == PathInfo::Unknown);
  }

  SECTION("directoryContents")
  {
    CHECK_THAT(
      fs.directoryContents(Path{}),
      Catch::Matchers::UnorderedEquals(std::vector<Path>{
        Path{"some_dir"},
        Path{"root_file_1"},
        Path{"root_file_2"},
      }));

    CHECK_THAT(
      fs.directoryContents(Path{"some_dir"}),
      Catch::Matchers::UnorderedEquals(std::vector<Path>{
        Path{"nested_dir"},
        Path{"some_dir_file_1"},
        Path{"some_dir_file_2"},
      }));

    CHECK_THAT(
      fs.directoryContents(Path{"some_dir/nested_dir"}),
      Catch::Matchers::UnorderedEquals(std::vector<Path>{
        Path{"nested_dir_file_1"},
        Path{"nested_dir_file_2"},
      }));
  }

  SECTION("openFile")
  {
    CHECK(fs.openFile(Path{"root_file_1"}) == root_file_1);
    CHECK(fs.openFile(Path{"some_dir/some_dir_file_1"}) == some_dir_file_1);
    CHECK(
      fs.openFile(Path{"some_dir/nested_dir/nested_dir_file_1"}) == nested_dir_file_1);
  }
}

} // namespace IO
} // namespace TrenchBroom
