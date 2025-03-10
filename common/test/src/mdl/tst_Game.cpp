/*
 Copyright (C) 2010 Kristian Duske

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
#include "io/GameConfigParser.h"
#include "mdl/GameImpl.h"

#include <filesystem>

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("GameTest.loadCorruptPackages")
{
  // https://github.com/TrenchBroom/TrenchBroom/issues/2496

  const auto games = std::vector<std::filesystem::path>{
    "Quake",
    "Daikatana",
    "Quake3",
  };

  for (const auto& game : games)
  {
    const auto configPath =
      std::filesystem::current_path() / "fixture/games/" / game / "GameConfig.cfg";
    const auto configStr = io::readTextFile(configPath);
    auto configParser = io::GameConfigParser(configStr, configPath);
    auto config = configParser.parse().value();

    const auto gamePath =
      std::filesystem::current_path() / "fixture/test/mdl/Game/CorruptPak";
    auto logger = NullLogger();
    UNSCOPED_INFO(
      "Should not throw when loading corrupted package file for game " << game);
    CHECK_NOTHROW(GameImpl(config, gamePath, logger));
  }
}

} // namespace tb::mdl
