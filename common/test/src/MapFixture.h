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

#include "Macros.h"
#include "mdl/MapFormat.h"
#include "mdl/MockGame.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace kdl
{
class task_manager;
}

namespace tb
{
class Logger;
}

namespace tb::mdl
{
class Game;
class Map;

struct LoadGameFixture
{
  std::string name;
};

struct MockGameFixture
{
  std::optional<MockGameConfig> config;
};

using GameFixture = std::variant<MockGameFixture, LoadGameFixture>;

struct MapFixtureConfig
{
  // nullopt means use the default (Standard for new map, Unknown for loading)
  std::optional<MapFormat> mapFormat = std::nullopt;

  GameFixture game = MockGameFixture{};
};

class MapFixture
{
private:
  std::unique_ptr<kdl::task_manager> m_taskManager;
  std::unique_ptr<Logger> m_logger;
  std::unique_ptr<Map> m_map;

public:
  explicit MapFixture();
  ~MapFixture();

  defineMove(MapFixture);

  void create(const MapFixtureConfig& = {});
  void load(const std::filesystem::path& path, const MapFixtureConfig& = {});

  Map& map();
};

} // namespace tb::mdl
