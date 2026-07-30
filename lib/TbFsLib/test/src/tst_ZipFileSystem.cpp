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
#include "fs/ZipFileSystem.h"

#include "kd/result.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::fs
{

TEST_CASE("ZipFileSystem")
{
  const auto fsTestPath = getFixtureRoot() / "test/fs/Zip/";

  SECTION("doReadDirectory")
  {
    SECTION("returns an error if the archive cannot be opened")
    {
      const auto file = Disk::openFile(fsTestPath / "not_a_zip.zip") | kdl::value();
      CHECK(createImageFileSystem<ZipFileSystem>(file).is_error());
    }

    SECTION("returns an error if extraction fails for a discovered entry")
    {
      auto fs = openFS<ZipFileSystem>(fsTestPath / "corrupted_data.zip");
      CHECK(fs->openFile("data.txt").is_error());
    }

    SECTION("skips an entry with an empty filename")
    {
      auto fs = openFS<ZipFileSystem>(fsTestPath / "empty_filename.zip");
      CHECK_THAT(fs->find("", TraversalMode::Recursive), MatchesPathsResult({}));
    }
  }
}

} // namespace tb::fs
