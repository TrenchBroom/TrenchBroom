/*
 Copyright (C) 2025 Kristian Duske

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

#include "Logger.h"
#include "TestUtils.h"
#include "fs/PathInfo.h"
#include "fs/TestUtils.h"
#include "mdl/GameConfig.h"
#include "mdl/GameFileSystem.h"

#include <filesystem>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("GameFileSystem")
{
  /*
  id1
    id1_pak0_loose_file - contents: "id1"
    pak0.pak
      id1_pak0_1.txt - contents: "id1_pak0_1"
      id1_pak0_2.txt - contents: "id1_pak0_2"
      id1_pak0_loose_file - contents: "pak0"
  mod1
    id1_pak0_loose_file - contents: "mod1"
    pak0.pak
      id1_pak0_2.txt - contents: "mod1_pak0_2", overrides id1/pak0.pak
      mod1_pak0_1.txt - contents: "mod1_pak0_1"
      mod1_pak0_2.txt - contents: "mod1_pak0_2"
    pak1.PAK
      mod1_pak0_2.txt - contents: "mod1_pak1_2", overrides mod1/pak0.pak
  */
  const auto fixturePath =
    std::filesystem::current_path() / "fixture/test/mdl/GameFileSystem";

  auto logger = NullLogger{};

  auto fs = GameFileSystem{};

  const auto config = GameConfig{
    "some game",
    {},
    {},
    false,
    {},
    {
      "id1",
      {
        {".pak"},
        "idpak",
      },
    },
    {},
    {},
    {},
    {},
    {},
    {},
  };

  SECTION("Mounts packages in game path")
  {
    fs.initialize(config, fixturePath, {}, logger);

    CHECK(fs.pathInfo("id1_pak0_1.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("id1_pak0_2.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("id1_pak0_loose_file.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("mod1_pak0_1.txt") == fs::PathInfo::Unknown);
  }

  SECTION("Packages files override loose files")
  {
    fs.initialize(config, fixturePath, {}, logger);

    CHECK(fs::readTextFile(fs, "id1_pak0_loose_file.txt") == "pak0");
  }

  SECTION("Mounts packages in additional search paths")
  {
    fs.initialize(config, fixturePath, {fixturePath / "mod1"}, logger);

    CHECK(fs.pathInfo("id1_pak0_1.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("id1_pak0_2.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("id1_pak0_loose_file.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("mod1_pak0_1.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("mod1_pak0_2.txt") == fs::PathInfo::File);
  }

  SECTION("Additional search paths override game path")
  {
    fs.initialize(config, fixturePath, {fixturePath / "mod1"}, logger);

    CHECK(fs::readTextFile(fs, "id1_pak0_loose_file.txt") == "mod1");
    CHECK(fs::readTextFile(fs, "id1_pak0_1.txt") == "id1_pak0_1");
    CHECK(fs::readTextFile(fs, "id1_pak0_2.txt") == "mod1_pak0_2");
    CHECK(fs::readTextFile(fs, "mod1_pak0_1.txt") == "mod1_pak0_1");
    CHECK(fs::readTextFile(fs, "mod1_pak0_2.txt") == "mod1_pak1_2");
  }

  SECTION("Game path is case insensitive")
  {

    const auto ucConfig = GameConfig{
      "some game",
      {},
      {},
      false,
      {},
      {
        "ID1",
        {
          {".pak"},
          "idpak",
        },
      },
      {},
      {},
      {},
      {},
      {},
      {},
    };

    fs.initialize(ucConfig, fixturePath, {}, logger);

    CHECK(fs.pathInfo("id1_pak0_1.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("id1_pak0_2.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("id1_pak0_loose_file.txt") == fs::PathInfo::File);
    CHECK(fs.pathInfo("mod1_pak0_1.txt") == fs::PathInfo::Unknown);
  }
}

} // namespace tb::mdl
