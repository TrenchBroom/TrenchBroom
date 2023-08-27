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

#include "Error.h"
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
#include "IO/TraversalMode.h"
#include "Logger.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/GameImpl.h"
#include "PreferenceManager.h"

#include <kdl/collection_utils.h>
#include <kdl/path_utils.h>
#include <kdl/result.h>
#include <kdl/string_compare.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace TrenchBroom::Model
{
GameFactory& GameFactory::instance()
{
  static auto instance = GameFactory{};
  return instance;
}

Result<std::vector<std::string>> GameFactory::initialize(
  const GamePathConfig& gamePathConfig)
{
  return initializeFileSystem(gamePathConfig).and_then([&]() {
    return loadGameConfigs();
  });
}

void GameFactory::reset()
{
  m_userGameDir = std::filesystem::path{};
  m_configFs.reset();

  m_names.clear();
  m_configs.clear();
  m_gamePaths.clear();
  m_defaultEngines.clear();
}

void GameFactory::saveGameEngineConfig(
  const std::string& gameName, const GameEngineConfig& gameEngineConfig, Logger& logger)
{
  auto& config = gameConfig(gameName);
  writeGameEngineConfig(config, gameEngineConfig, logger);
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

Result<std::pair<std::string, MapFormat>> GameFactory::detectGame(
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

Result<void> GameFactory::initializeFileSystem(const GamePathConfig& gamePathConfig)
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
    virtualFs.mount({}, std::make_unique<IO::DiskFileSystem>(path));
  }

  m_userGameDir = userGameDir;
  return IO::Disk::createDirectory(m_userGameDir).transform([&](auto) {
    m_configFs = std::make_unique<IO::WritableVirtualFileSystem>(
      std::move(virtualFs), std::make_unique<IO::WritableDiskFileSystem>(m_userGameDir));
  });
}

Result<std::vector<std::string>> GameFactory::loadGameConfigs()
{
  return m_configFs
    ->find(
      {}, IO::TraversalMode::Recursive, IO::makeFilenamePathMatcher("GameConfig.cfg"))
    .transform([&](auto configFiles) {
      auto errors = std::vector<std::string>{};
      kdl::vec_transform(configFiles, [&](const auto& configFilePath) {
        return loadGameConfig(configFilePath).transform_error([&](auto e) {
          errors.push_back(
            "Failed to load game configuration file '" + configFilePath.string()
            + "': " + e.msg);
        });
      });
      return errors;
    });
}

Result<void> GameFactory::loadGameConfig(const std::filesystem::path& path)
{
  return m_configFs->openFile(path)
    .join(m_configFs->makeAbsolute(path))
    .and_then([&](auto configFile, auto absolutePath) -> Result<void> {
      auto reader = configFile->reader().buffer();
      auto parser = IO::GameConfigParser{reader.stringView(), absolutePath};
      try
      {
        auto config = parser.parse();

        loadCompilationConfig(config);
        loadGameEngineConfig(config);

        const auto configName = config.name;
        m_configs.emplace(configName, std::move(config));
        m_names.push_back(configName);

        const auto gamePathPrefPath =
          std::filesystem::path{"Games"} / configName / "Path";
        m_gamePaths.emplace(
          configName, Preference<std::filesystem::path>{gamePathPrefPath, {}});

        const auto defaultEnginePrefPath =
          std::filesystem::path{"Games"} / configName / "Default Engine";
        m_defaultEngines.emplace(
          configName, Preference<std::filesystem::path>{defaultEnginePrefPath, {}});

        return Result<void>{};
      }
      catch (const ParserException& e)
      {
        return Error{e.what()};
      }
    });
}

void GameFactory::loadCompilationConfig(GameConfig& gameConfig)
{
  const auto path = std::filesystem::path{gameConfig.name} / "CompilationProfiles.cfg";
  try
  {
    if (m_configFs->pathInfo(path) == IO::PathInfo::File)
    {
      m_configFs->openFile(path)
        .join(m_configFs->makeAbsolute(path))
        .transform([&](auto profilesFile, auto absolutePath) {
          auto reader = profilesFile->reader().buffer();
          auto parser = IO::CompilationConfigParser{reader.stringView(), absolutePath};
          gameConfig.compilationConfig = parser.parse();
          gameConfig.compilationConfigParseFailed = false;
        })
        .transform_error([&](auto e) {
          std::cerr << "Could not load compilation configuration '" << path
                    << "': " << e.msg << "\n";
          gameConfig.compilationConfigParseFailed = true;
        });
    }
  }
  catch (const ParserException& e)
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
      m_configFs->openFile(path)
        .join(m_configFs->makeAbsolute(path))
        .transform([&](auto profilesFile, auto absolutePath) {
          auto reader = profilesFile->reader().buffer();
          auto parser = IO::GameEngineConfigParser{reader.stringView(), absolutePath};
          gameConfig.gameEngineConfig = parser.parse();
          gameConfig.gameEngineConfigParseFailed = false;
        })
        .transform_error([&](auto e) {
          std::cerr << "Could not load game engine configuration '" << path
                    << "': " << e.msg << "\n";
          gameConfig.gameEngineConfigParseFailed = true;
        });
    }
  }
  catch (const ParserException& e)
  {
    std::cerr << "Could not load game engine configuration '" << path << "': " << e.what()
              << "\n";
    gameConfig.gameEngineConfigParseFailed = true;
  }
}

static Result<std::filesystem::path> backupFile(
  IO::WritableFileSystem& fs, const std::filesystem::path& path)
{
  const auto backupPath = kdl::path_add_extension(path, ".bak");
  return fs.copyFile(path, backupPath).transform([&]() { return backupPath; });
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
    backupFile(*m_configFs, profilesPath)
      .and_then([&](auto backupPath) {
        return m_configFs->makeAbsolute(profilesPath)
          .join(m_configFs->makeAbsolute(backupPath))
          .transform([&](auto absProfilesPath, auto absBackupPath) {
            logger.warn() << "Backed up malformed compilation config " << absProfilesPath
                          << " to " << absBackupPath;
          });
      })
      .transform_error([&](auto) {
        logger.error() << "Could not back up malformed compilation config";
      });

    gameConfig.compilationConfigParseFailed = false;
  }

  m_configFs->createFileAtomic(profilesPath, stream.str())
    .transform([&]() {
      gameConfig.compilationConfig = std::move(compilationConfig);

      m_configFs->makeAbsolute(profilesPath)
        .transform([&](auto absProfilesPath) {
          logger.debug() << "Wrote compilation config to " << absProfilesPath;
        })
        .transform_error([](auto) {
          // Can't really do anything
        });
    })
    .transform_error([&](const auto& e) {
      logger.error() << "Could not write compilation config: " << e.msg;
    });
}

void GameFactory::writeGameEngineConfig(
  GameConfig& gameConfig, GameEngineConfig gameEngineConfig, Logger& logger)
{
  if (
    !gameConfig.gameEngineConfigParseFailed
    && gameConfig.gameEngineConfig == gameEngineConfig)
  {
    logger.debug() << "Skipping writing unchanged game engine config for "
                   << gameConfig.name;
    return;
  }

  auto stream = std::stringstream{};
  auto writer = IO::GameEngineConfigWriter{gameEngineConfig, stream};
  writer.writeConfig();

  const auto profilesPath =
    std::filesystem::path{gameConfig.name} / "GameEngineProfiles.cfg";
  if (gameConfig.gameEngineConfigParseFailed)
  {
    backupFile(*m_configFs, profilesPath)
      .and_then([&](auto backupPath) {
        return m_configFs->makeAbsolute(profilesPath)
          .join(m_configFs->makeAbsolute(backupPath))
          .transform([&](auto absProfilesPath, auto absBackupPath) {
            logger.warn() << "Backed up malformed game engine config " << absProfilesPath
                          << " to " << absBackupPath;
          });
      })
      .transform_error([&](auto) {
        logger.error() << "Could not back up malformed game engine config";
      });

    gameConfig.gameEngineConfigParseFailed = false;
  }

  m_configFs->createFileAtomic(profilesPath, stream.str())
    .transform([&]() {
      gameConfig.gameEngineConfig = std::move(gameEngineConfig);
      m_configFs->makeAbsolute(profilesPath)
        .transform([&](auto absProfilesPath) {
          logger.debug() << "Wrote game engine config to " << absProfilesPath;
        })
        .transform_error([](auto) {
          // Can't really do anything
        });
    })
    .transform_error([&](const auto& e) {
      logger.error() << "Could not write game engine config: " << e.msg;
    });
}
} // namespace TrenchBroom::Model
