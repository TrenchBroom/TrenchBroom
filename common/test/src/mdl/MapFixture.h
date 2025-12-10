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

#pragma once

#include "GameConfigFixture.h"
#include "Macros.h"
#include "mdl/GameInfo.h"
#include "mdl/MapFormat.h"

#include <filesystem>
#include <memory>
#include <optional>

namespace kdl
{
class task_manager;
}

namespace tb
{
class Logger;

namespace mdl
{
class Map;

struct MapFixtureConfig
{
  // nullopt means use the default (Standard for new map, Unknown for loading)
  std::optional<MapFormat> mapFormat = std::nullopt;
  GameInfo gameInfo = DefaultGameInfo;
};

extern const MapFixtureConfig QuakeFixtureConfig;
extern const MapFixtureConfig Quake2FixtureConfig;

class MapFixture
{
private:
  std::unique_ptr<kdl::task_manager> m_taskManager;
  std::unique_ptr<Logger> m_logger;
  std::unique_ptr<Map> m_map;

  std::optional<MapFixtureConfig> m_config;

public:
  explicit MapFixture();
  ~MapFixture();

  defineMove(MapFixture);

  Map& create(MapFixtureConfig = {});
  Map& load(const std::filesystem::path& path, MapFixtureConfig = {});
};

} // namespace mdl
} // namespace tb
