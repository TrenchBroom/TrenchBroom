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
#include "fs/IdPakFileSystem.h"
#include "fs/TestUtils.h"
#include "fs/TraversalMode.h"

#include <filesystem>

#include <catch2/catch_test_macros.hpp>

namespace tb::fs
{

TEST_CASE("IdPakFileSystem")
{
  const auto fsTestPath = getFixtureRoot() / "test/fs/Pak/";

  SECTION("doReadDirectory")
  {
    SECTION("reads an entry whose name is not valid UTF-8")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/5494
      const auto fs =
        openFS<IdPakFileSystem>(fsTestPath / "idpak_non_utf8_entry_name.pak");

#ifdef _WIN32
      const auto expectedPath = std::filesystem::path{L"fen\u00EAtre.txt"};
#else
      const auto expectedPath = std::filesystem::path{"fen\xEAtre.txt"};
#endif
      CHECK_THAT(fs->find("", TraversalMode::Flat), MatchesPathsResult({expectedPath}));
    }
  }
}

} // namespace tb::fs
