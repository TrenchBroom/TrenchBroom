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

#include "Matchers.h"
#include "TestEnvironment.h"
#include "fs/DiskIO.h"
#include "fs/TestUtils.h"
#include "fs/TraversalMode.h"
#include "fs/WadFileSystem.h"

#include "kd/result.h"

#include <filesystem>
#include <system_error>

#include <catch2/catch_test_macros.hpp>

namespace tb::fs
{
namespace
{

bool openWadFails(const std::filesystem::path& path)
{
  const auto file = Disk::openFile(path) | kdl::value();
  return createImageFileSystem<WadFileSystem>(file).is_error();
}

} // namespace

TEST_CASE("WadFileSystem")
{
  SECTION("doReadDirectory")
  {
    const auto fsTestPath = getFixtureRoot() / "test/fs/Wad/";

    SECTION("returns an error if the file is smaller than the header size")
    {
      CHECK(openWadFails(fsTestPath / "too_small.wad"));
    }

    SECTION("returns an error for an unrecognized magic")
    {
      CHECK(openWadFails(fsTestPath / "bad_magic.wad"));
    }

    SECTION("returns an error if the declared entry count exceeds the file size")
    {
      CHECK(openWadFails(fsTestPath / "entry_count_too_large.wad"));
    }

    SECTION("returns an error if the directory is out of bounds")
    {
      CHECK(openWadFails(fsTestPath / "directory_out_of_bounds.wad"));
    }

    SECTION("returns an error if an entry is out of bounds")
    {
      CHECK(openWadFails(fsTestPath / "entry_out_of_bounds.wad"));
    }

    SECTION("accepts the wad3 magic")
    {
      // cr8_czg.wad already covers the "wad2" magic; this covers "wad3"
      // independently, since both are valid and the check short-circuits on the
      // first match
      CHECK(!openWadFails(fsTestPath / "wad3_empty.wad"));
    }

    SECTION("skips an entry with an empty name and continues with the next entry")
    {
      const auto fs = openFS<WadFileSystem>(fsTestPath / "skips_empty_entry_name.wad");
      CHECK_THAT(fs->find("", TraversalMode::Flat), MatchesPathsResult({"real.D"}));
    }
  }

  SECTION("file can be replaced while wad file system exists")
  {
    const auto wadPath = getFixtureRoot() / "test/fs/Wad/cr8_czg.wad";
    const auto copyPath = getFixtureRoot() / "test/fs/Wad/cr8_czg_2.wad";

    REQUIRE_FALSE(std::filesystem::is_regular_file(copyPath));
    REQUIRE_NOTHROW(std::filesystem::copy(wadPath, copyPath));
    REQUIRE(std::filesystem::is_regular_file(copyPath));

    {
      const auto fs = openFS<WadFileSystem>(copyPath);

      auto errorCode = std::error_code{};
      CHECK(std::filesystem::remove(copyPath, errorCode));
      CHECK(errorCode == std::error_code{});
    }

    if (std::filesystem::is_regular_file(copyPath))
    {
      REQUIRE(std::filesystem::remove(copyPath));
    }
  }
}

} // namespace tb::fs
