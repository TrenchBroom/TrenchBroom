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

#include "mdl/MapFixture.h"

#include "Logger.h"
#include "TestUtils.h"
#include "mdl/Map.h"
#include "mdl/Resource.h"

#include "kd/contracts.h"

namespace tb::mdl
{

const MapFixtureConfig QuakeFixtureConfig = MapFixtureConfig{
  .mapFormat = MapFormat::Valve,
  .gameInfo = QuakeGameInfo,
};

const MapFixtureConfig Quake2FixtureConfig = MapFixtureConfig{
  .mapFormat = MapFormat::Quake2,
  .gameInfo = Quake2GameInfo,
};

MapFixture::MapFixture()
  : m_taskManager{createTestTaskManager()}
  , m_logger{std::make_unique<NullLogger>()}
{
}

MapFixture::~MapFixture() = default;

Map& MapFixture::create(MapFixtureConfig config)
{
  m_config = std::move(config);

  const auto mapFormat = m_config->mapFormat.value_or(MapFormat::Standard);

  contract_assert(
    Map::createMap(
      mapFormat,
      m_config->gameInfo,
      m_config->gameInfo.gamePathPreference.value(),
      vm::bbox3d{8129.0},
      *m_taskManager,
      *m_logger)
    | kdl::transform([&](auto map) {
        m_map = std::move(map);
        m_map->setIsCommandCollationEnabled(false);
      })
    | kdl::is_success());

  return *m_map;
}

Map& MapFixture::load(const std::filesystem::path& path, MapFixtureConfig config)
{
  m_config = std::move(config);

  const auto absPath = path.is_absolute() ? path : std::filesystem::current_path() / path;

  const auto mapFormat = m_config->mapFormat.value_or(MapFormat::Unknown);

  contract_assert(
    Map::loadMap(
      absPath,
      mapFormat,
      m_config->gameInfo,
      m_config->gameInfo.gamePathPreference.value(),
      vm::bbox3d{8129.0},
      *m_taskManager,
      *m_logger)
    | kdl::transform([&](auto map) {
        m_map = std::move(map);
        m_map->setIsCommandCollationEnabled(false);
        m_map->processResourcesSync(ProcessContext{false, [](auto, auto) {}});
      })
    | kdl::is_success());

  return *m_map;
}

} // namespace tb::mdl
