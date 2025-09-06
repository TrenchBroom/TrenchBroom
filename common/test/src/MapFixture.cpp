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

#include "MapFixture.h"

#include "Ensure.h"
#include "Logger.h"
#include "TestUtils.h"
#include "mdl/Map.h"
#include "mdl/MockGame.h"
#include "mdl/Resource.h"

#include "kdl/overload.h"

namespace tb::mdl
{
namespace
{
std::unique_ptr<Game> createGame(const MapFixtureConfig& config)
{
  return std::visit(
    kdl::overload(
      [](const MockGameFixture& mockGameConfig) -> std::unique_ptr<Game> {
        auto game = std::make_unique<MockGame>();
        if (const auto& gameConfig = mockGameConfig.config)
        {
          game->config() = *gameConfig;
        }
        return game;
      },
      [](const LoadGameFixture& loadGameConfig) -> std::unique_ptr<Game> {
        return loadGame(loadGameConfig.name);
      }),
    config.game);
}

} // namespace

MapFixture::MapFixture()
  : m_taskManager{createTestTaskManager()}
  , m_logger{std::make_unique<NullLogger>()}
  , m_map{std::make_unique<Map>(*m_taskManager, *m_logger)}
{
  m_map->setIsCommandCollationEnabled(false);
}

MapFixture::~MapFixture() = default;

void MapFixture::create(const MapFixtureConfig& config)
{
  const auto mapFormat = config.mapFormat.value_or(MapFormat::Standard);
  auto game = createGame(config);

  ensure(
    m_map->create(mapFormat, vm::bbox3d{8192.0}, std::move(game)),
    "Map was created successfully");
}

void MapFixture::load(const std::filesystem::path& path, const MapFixtureConfig& config)
{
  const auto absPath = path.is_absolute() ? path : std::filesystem::current_path() / path;

  const auto mapFormat = config.mapFormat.value_or(MapFormat::Unknown);
  auto game = createGame(config);

  m_map->load(mapFormat, vm::bbox3d{8192.0}, std::move(game), absPath)
    .transform_error([](const auto& e) { throw std::runtime_error{e.msg}; });
  m_map->processResourcesSync(ProcessContext{false, [](auto, auto) {}});
}

Map& MapFixture::map()
{
  return *m_map;
}

} // namespace tb::mdl
