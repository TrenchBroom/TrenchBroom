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

#include "MockGame.h"

#include "TestUtils.h"
#include "io/BrushFaceReader.h"
#include "io/DiskFileSystem.h"
#include "io/NodeReader.h"
#include "io/VirtualFileSystem.h"
#include "io/WadFileSystem.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionFileSpec.h"
#include "mdl/GameConfig.h"
#include "mdl/MaterialManager.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep

#include <memory>
#include <vector>

namespace tb::mdl
{

MockGameConfig::MockGameConfig()
  : GameConfig{
      .name = "Test",
      .path = {},
      .icon = {},
      .experimental = false,
      .fileFormats = {},
      .fileSystemConfig = {},
      .materialConfig = {"textures", {".D"}, "fixture/test/palette.lmp", {}, "", {}},
      .entityConfig = {},
      .faceAttribsConfig = {},
      .smartTags = {},
      .softMapBounds = {},
      .compilationTools = {},
      .forceEmptyNewMap = true,
    }
{
}

MockGame::MockGame(MockGameConfig config)
  : m_config{std::move(config)}
  , m_fs{std::make_unique<io::VirtualFileSystem>()}
{
  m_fs->mount("", std::make_unique<io::DiskFileSystem>(std::filesystem::current_path()));
}

MockGame::~MockGame() = default;

const GameConfig& MockGame::config() const
{
  return m_config;
}

GameConfig& MockGame::config()
{
  return m_config;
}

const io::FileSystem& MockGame::gameFileSystem() const
{
  return *m_fs;
}

std::filesystem::path MockGame::gamePath() const
{
  return ".";
}

void MockGame::setGamePath(
  const std::filesystem::path& /* gamePath */, Logger& /* logger */)
{
}

SoftMapBounds MockGame::extractSoftMapBounds(const Entity&) const
{
  return {SoftMapBoundsType::Game, vm::bbox3d()};
}

void MockGame::setAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& /* searchPaths */, Logger& /* logger */)
{
}

Game::PathErrors MockGame::checkAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& /* searchPaths */) const
{
  return {};
}

void MockGame::reloadWads(
  const std::filesystem::path&,
  const std::vector<std::filesystem::path>& wadPaths,
  Logger&)
{
  m_fs->unmountAll();
  m_fs->mount("", std::make_unique<io::DiskFileSystem>(std::filesystem::current_path()));

  for (const auto& wadPath : wadPaths)
  {
    const auto absoluteWadPath = std::filesystem::current_path() / wadPath;
    m_fs->mount("textures", io::openFS<io::WadFileSystem>(absoluteWadPath));
  }
}

bool MockGame::isEntityDefinitionFile(const std::filesystem::path& /* path */) const
{
  return false;
}

std::vector<EntityDefinitionFileSpec> MockGame::allEntityDefinitionFiles() const
{
  return {};
}

std::filesystem::path MockGame::findEntityDefinitionFile(
  const EntityDefinitionFileSpec& /* spec */,
  const std::vector<std::filesystem::path>& /* searchPaths */) const
{
  return {};
}

Result<std::vector<std::string>> MockGame::availableMods() const
{
  return std::vector<std::string>{};
}

std::string MockGame::defaultMod() const
{
  return "";
}

Result<std::vector<EntityDefinition>> MockGame::loadEntityDefinitions(
  io::ParserStatus& /* status */, const std::filesystem::path& /* path */) const
{
  return std::vector<EntityDefinition>{};
}

void MockGame::setSmartTags(std::vector<SmartTag> smartTags)
{
  m_config.smartTags = std::move(smartTags);
}

void MockGame::setDefaultFaceAttributes(const BrushFaceAttributes& defaultFaceAttributes)
{
  m_config.faceAttribsConfig.defaults = defaultFaceAttributes;
}

} // namespace tb::mdl
