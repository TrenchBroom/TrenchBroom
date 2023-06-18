/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "GameFactory.h"

#include "Exceptions.h"
#include "IO/CompilationConfigParser.h"
#include "IO/CompilationConfigWriter.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/File.h"
#include "IO/GameConfigParser.h"
#include "IO/GameEngineConfigParser.h"
#include "IO/GameEngineConfigWriter.h"
#include "IO/PathInfo.h"
#include "IO/SystemPaths.h"
#include "Logger.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/GameImpl.h"
#include "PreferenceManager.h"

#include <kdl/collection_utils.h>
#include <kdl/path_utils.h>
#include <kdl/string_compare.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
GameFactory& GameFactory::instance()
{
  static auto instance = GameFactory{};
  return instance;
}

void GameFactory::initialize(const GamePathConfig& gamePathConfig)
{
  initializeFileSystem(gamePathConfig);
  loadGameConfigs();
}

void GameFactory::saveGameEngineConfig(
  const std::string& gameName, const GameEngineConfig& gameEngineConfig)
{
  auto& config = gameConfig(gameName);
  writeGameEngineConfig(config, gameEngineConfig);
}

void GameFactory::saveCompilationConfig(
  const std::string& gameName, const CompilationConfig& compilationConfig, Logger& logger)
{
  auto& config = gameConfig(gameName);
  writeCompilationConfig(config, compilationConfig, logger);
}

const std::vector<std::string>& GameFactory::gameList() const
{
  return m_names;
}

size_t GameFactory::gameCount() const
{
  return m_configs.size();
}

std::shared_ptr<Game> GameFactory::createGame(const std::string& gameName, Logger& logger)
{
  return std::make_shared<GameImpl>(gameConfig(gameName), gamePath(gameName), logger);
}

std::vector<std::string> GameFactory::fileFormats(const std::string& gameName) const
{
  return kdl::vec_transform(
    gameConfig(gameName).fileFormats, [](const auto& format) { return format.format; });
}

std::filesystem::path GameFactory::iconPath(const std::string& gameName) const
{
  const auto& config = gameConfig(gameName);
  return config.findConfigFile(config.icon);
}

std::filesystem::path GameFactory::gamePath(const std::string& gameName) const
{
  const auto it = m_gamePaths.find(gameName);
  if (it == std::end(m_gamePaths))
  {
    throw GameException{"Unknown game: " + gameName};
  }
  auto& pref = it->second;
  return PreferenceManager::instance().get(pref);
}

bool GameFactory::setGamePath(
  const std::string& gameName, const std::filesystem::path& gamePath)
{
  const auto it = m_gamePaths.find(gameName);
  if (it == std::end(m_gamePaths))
  {
    throw GameException{"Unknown game: " + gameName};
  }
  auto& pref = it->second;
  return PreferenceManager::instance().set(pref, gamePath);
}

bool GameFactory::isGamePathPreference(
  const std::string& gameName, const std::filesystem::path& prefPath) const
{
  const auto it = m_gamePaths.find(gameName);
  if (it == std::end(m_gamePaths))
  {
    throw GameException{"Unknown game: " + gameName};
  }
  auto& pref = it->second;
  return pref.path() == prefPath;
}

static Preference<std::filesystem::path>& compilationToolPathPref(
  const std::string& gameName, const std::string& toolName)
{
  auto& prefs = PreferenceManager::instance();
  auto& pref = prefs.dynamicPreference(
    std::filesystem::path{"Games"} / gameName / "Tool Path" / toolName,
    std::filesystem::path{});
  return pref;
}

std::filesystem::path GameFactory::compilationToolPath(
  const std::string& gameName, const std::string& toolName) const
{
  return PreferenceManager::instance().get(compilationToolPathPref(gameName, toolName));
}

bool GameFactory::setCompilationToolPath(
  const std::string& gameName,
  const std::string& toolName,
  const std::filesystem::path& gamePath)
{
  return PreferenceManager::instance().set(
    compilationToolPathPref(gameName, toolName), gamePath);
}

GameConfig& GameFactory::gameConfig(const std::string& name)
{
  const auto cIt = m_configs.find(name);
  if (cIt == std::end(m_configs))
  {
    throw GameException{"Unknown game: " + name};
  }
  return cIt->second;
}

const GameConfig& GameFactory::gameConfig(const std::string& name) const
{
  const auto cIt = m_configs.find(name);
  if (cIt == std::end(m_configs))
  {
    throw GameException{"Unknown game: " + name};
  }
  return cIt->second;
}

namespace
{
std::string readInfoComment(std::istream& stream, const std::string& name)
{
  const auto expectedHeader = "// " + name + ": ";

  auto lineBuffer = std::string{};
  std::getline(stream, lineBuffer);
  if (stream.fail())
  {
    return "";
  }

  const auto lineView = std::string_view{lineBuffer};
  if (!kdl::cs::str_is_prefix(lineView, expectedHeader))
  {
    return "";
  }

  auto result = lineView.substr(expectedHeader.size());
  if (!result.empty() && result.back() == '\r')
  {
    result = result.substr(0, result.length() - 1);
  }
  return std::string{result};
}
} // namespace

std::pair<std::string, MapFormat> GameFactory::detectGame(
  const std::filesystem::path& path) const
{
  return IO::Disk::withInputStream(path, [&](auto& stream) {
    auto gameName = readInfoComment(stream, "Game");
    if (m_configs.find(gameName) == std::end(m_configs))
    {
      gameName = "";
    }

    const auto formatName = readInfoComment(stream, "Format");
    const auto format = formatFromName(formatName);

    return std::pair{gameName, format};
  });
}

const std::filesystem::path& GameFactory::userGameConfigsPath() const
{
  return m_userGameDir;
}

GameFactory::GameFactory() = default;

void GameFactory::initializeFileSystem(const GamePathConfig& gamePathConfig)
{
  // Gather the search paths we're going to use.
  // The rest of this function will be mounting TB filesystems for these search paths.
  const auto& userGameDir = gamePathConfig.userGameDir;
  const auto& gameConfigSearchDirs = gamePathConfig.gameConfigSearchDirs;

  auto virtualFs = IO::VirtualFileSystem{};

  // All of the current search paths from highest to lowest priority
  for (auto it = gameConfigSearchDirs.rbegin(); it != gameConfigSearchDirs.rend(); ++it)
  {
    const auto path = *it;
    virtualFs.mount({}, std::make_unique<IO::DiskFileSystem>(path, false));
  }

  m_userGameDir = userGameDir;
  m_configFs = std::make_unique<IO::WritableVirtualFileSystem>(
    std::move(virtualFs),
    std::make_unique<IO::WritableDiskFileSystem>(m_userGameDir, true));
}

void GameFactory::loadGameConfigs()
{
  auto errors = std::vector<std::string>{};

  const auto configFiles =
    m_configFs->findRecursively({}, IO::makeFilenamePathMatcher("GameConfig.cfg"));
  for (const auto& configFilePath : configFiles)
  {
    try
    {
      loadGameConfig(configFilePath);
    }
    catch (const std::exception& e)
    {
      errors.push_back(kdl::str_to_string(
        "Could not load game configuration file ", configFilePath, ": ", e.what()));
    }
  }

  m_names = kdl::col_sort(std::move(m_names), kdl::cs::string_less());

  if (!errors.empty())
  {
    throw errors;
  }
}

void GameFactory::loadGameConfig(const std::filesystem::path& path)
{
  const auto configFile = m_configFs->openFile(path);
  const auto absolutePath = m_configFs->makeAbsolute(path);

  auto reader = configFile->reader().buffer();
  auto parser = IO::GameConfigParser{reader.stringView(), absolutePath};
  auto config = parser.parse();

  loadCompilationConfig(config);
  loadGameEngineConfig(config);

  const auto configName = config.name;
  m_configs.emplace(configName, std::move(config));
  m_names.push_back(configName);

  const auto gamePathPrefPath = std::filesystem::path{"Games"} / configName / "Path";
  m_gamePaths.emplace(
    configName, Preference<std::filesystem::path>{gamePathPrefPath, {}});

  const auto defaultEnginePrefPath =
    std::filesystem::path{"Games"} / configName / "Default Engine";
  m_defaultEngines.emplace(
    configName, Preference<std::filesystem::path>{defaultEnginePrefPath, {}});
}

void GameFactory::loadCompilationConfig(GameConfig& gameConfig)
{
  const auto path = std::filesystem::path{gameConfig.name} / "CompilationProfiles.cfg";
  try
  {
    if (m_configFs->pathInfo(path) == IO::PathInfo::File)
    {
      const auto profilesFile = m_configFs->openFile(path);
      auto reader = profilesFile->reader().buffer();
      auto parser =
        IO::CompilationConfigParser{reader.stringView(), m_configFs->makeAbsolute(path)};
      gameConfig.compilationConfig = parser.parse();
      gameConfig.compilationConfigParseFailed = false;
    }
  }
  catch (const Exception& e)
  {
    std::cerr << "Could not load compilation configuration '" << path << "': " << e.what()
              << "\n";
    gameConfig.compilationConfigParseFailed = true;
  }
}

void GameFactory::loadGameEngineConfig(GameConfig& gameConfig)
{
  const auto path = std::filesystem::path{gameConfig.name} / "GameEngineProfiles.cfg";
  try
  {
    if (m_configFs->pathInfo(path) == IO::PathInfo::File)
    {
      const auto profilesFile = m_configFs->openFile(path);
      auto reader = profilesFile->reader().buffer();
      auto parser =
        IO::GameEngineConfigParser{reader.stringView(), m_configFs->makeAbsolute(path)};
      gameConfig.gameEngineConfig = parser.parse();
      gameConfig.gameEngineConfigParseFailed = false;
    }
  }
  catch (const Exception& e)
  {
    std::cerr << "Could not load game engine configuration '" << path << "': " << e.what()
              << "\n";
    gameConfig.gameEngineConfigParseFailed = true;
  }
}

static std::filesystem::path backupFile(
  IO::WritableFileSystem& fs, const std::filesystem::path& path)
{
  const auto backupPath = kdl::path_add_extension(path, ".bak");
  fs.copyFile(path, backupPath);
  return backupPath;
}

void GameFactory::writeCompilationConfig(
  GameConfig& gameConfig, CompilationConfig compilationConfig, Logger& logger)
{
  if (
    !gameConfig.compilationConfigParseFailed
    && gameConfig.compilationConfig == compilationConfig)
  {
    // NOTE: this is not just an optimization, but important for ensuring that
    // we don't clobber data saved by a newer version of TB, unless we actually make
    // changes to the config in this version of TB (see:
    // https://github.com/TrenchBroom/TrenchBroom/issues/3424)
    logger.debug() << "Skipping writing unchanged compilation config for "
                   << gameConfig.name;
    return;
  }

  auto stream = std::stringstream{};
  auto writer = IO::CompilationConfigWriter{compilationConfig, stream};
  writer.writeConfig();

  const auto profilesPath =
    std::filesystem::path{gameConfig.name} / "CompilationProfiles.cfg";
  if (gameConfig.compilationConfigParseFailed)
  {
    const auto backupPath = backupFile(*m_configFs, profilesPath);

    logger.warn() << "Backed up malformed compilation config "
                  << m_configFs->makeAbsolute(profilesPath) << " to "
                  << m_configFs->makeAbsolute(backupPath);

    gameConfig.compilationConfigParseFailed = false;
  }

  m_configFs->createFileAtomic(profilesPath, stream.str());
  gameConfig.compilationConfig = std::move(compilationConfig);
  logger.debug() << "Wrote compilation config to "
                 << m_configFs->makeAbsolute(profilesPath);
}

void GameFactory::writeGameEngineConfig(
  GameConfig& gameConfig, GameEngineConfig gameEngineConfig)
{
  if (
    !gameConfig.gameEngineConfigParseFailed
    && gameConfig.gameEngineConfig == gameEngineConfig)
  {
    std::cout << "Skipping writing unchanged game engine config for " << gameConfig.name;
    return;
  }

  auto stream = std::stringstream{};
  auto writer = IO::GameEngineConfigWriter{gameEngineConfig, stream};
  writer.writeConfig();

  const auto profilesPath =
    std::filesystem::path{gameConfig.name} / "GameEngineProfiles.cfg";
  if (gameConfig.gameEngineConfigParseFailed)
  {
    const auto backupPath = backupFile(*m_configFs, profilesPath);

    std::cerr << "Backed up malformed game engine config "
              << m_configFs->makeAbsolute(profilesPath) << " to "
              << m_configFs->makeAbsolute(backupPath) << std::endl;

    gameConfig.gameEngineConfigParseFailed = false;
  }
  m_configFs->createFileAtomic(profilesPath, stream.str());
  gameConfig.gameEngineConfig = std::move(gameEngineConfig);
  std::cout << "Wrote game engine config to " << m_configFs->makeAbsolute(profilesPath)
            << std::endl;
}
} // namespace Model
} // namespace TrenchBroom
