/*
 Copyright (C) 2026 Kristian Duske

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

#include "fs/PathInfo.h"
#include "fs/PathMatcher.h"

#include <filesystem>

#include <catch2/catch_test_macros.hpp>

namespace tb::fs
{
namespace
{

struct CountingGetPathInfo
{
  int callCount = 0;

  PathInfo operator()(const std::filesystem::path&)
  {
    ++callCount;
    return PathInfo::Unknown;
  }
};

} // namespace

TEST_CASE("PathMatcher")
{
  SECTION("makeExtensionPathMatcher")
  {
    auto unusedGetPathInfo = CountingGetPathInfo{};
    const auto matcher = makeExtensionPathMatcher({".txt", ".map"});

    CHECK(matcher("foo.txt", unusedGetPathInfo));
    CHECK(matcher("foo.map", unusedGetPathInfo));
    CHECK(matcher("dir/foo.map", unusedGetPathInfo));
    CHECK(!matcher("foo.wad", unusedGetPathInfo));
    CHECK(!matcher("foo", unusedGetPathInfo));

    // both the path's and the extensions' cases are ignored
    CHECK(matcher("FOO.TXT", unusedGetPathInfo));
    CHECK(matcher("foo.TXT", unusedGetPathInfo));

    const auto emptyMatcher = makeExtensionPathMatcher({});
    CHECK(!emptyMatcher("foo.txt", unusedGetPathInfo));

    CHECK(unusedGetPathInfo.callCount == 0);
  }

  SECTION("makeFilenamePathMatcher")
  {
    auto unusedGetPathInfo = CountingGetPathInfo{};
    const auto matcher = makeFilenamePathMatcher("foo.*");

    // matches on the filename alone, regardless of the directory part
    CHECK(matcher("foo.txt", unusedGetPathInfo));
    CHECK(matcher("dir/foo.map", unusedGetPathInfo));
    CHECK(matcher("dir/subdir/foo.wad", unusedGetPathInfo));

    // glob matching is case insensitive
    CHECK(matcher("FOO.TXT", unusedGetPathInfo));

    CHECK(!matcher("bar.txt", unusedGetPathInfo));
    CHECK(!matcher("dir/foobar.txt", unusedGetPathInfo));

    CHECK(unusedGetPathInfo.callCount == 0);
  }

  SECTION("makePathInfoPathMatcher")
  {
    const auto matcher = makePathInfoPathMatcher({PathInfo::Directory, PathInfo::File});

    auto queriedPath = std::filesystem::path{};
    const auto getPathInfo = [&](const std::filesystem::path& path) {
      queriedPath = path;
      return path == "dir"        ? PathInfo::Directory
             : path == "file.txt" ? PathInfo::File
                                  : PathInfo::Unknown;
    };

    CHECK(matcher("dir", getPathInfo));
    CHECK(queriedPath == "dir");

    CHECK(matcher("file.txt", getPathInfo));
    CHECK(queriedPath == "file.txt");

    CHECK(!matcher("does_not_exist", getPathInfo));

    const auto directoryOnlyMatcher = makePathInfoPathMatcher({PathInfo::Directory});
    CHECK(!directoryOnlyMatcher("file.txt", getPathInfo));
  }

  SECTION("matchAnyPath")
  {
    auto unusedGetPathInfo = CountingGetPathInfo{};

    CHECK(matchAnyPath("foo.txt", unusedGetPathInfo));
    CHECK(matchAnyPath("", unusedGetPathInfo));

    CHECK(unusedGetPathInfo.callCount == 0);
  }
}

} // namespace tb::fs
