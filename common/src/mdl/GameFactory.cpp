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

#include "GameFactory.h"

#include "Exceptions.h"
#include "Logger.h"
#include "PreferenceManager.h"
#include "io/CompilationConfigParser.h"
#include "io/CompilationConfigWriter.h"
#include "io/DiskFileSystem.h"
#include "io/DiskIO.h"
#include "io/GameConfigParser.h"
#include "io/GameEngineConfigParser.h"
#include "io/GameEngineConfigWriter.h"
#include "io/PathInfo.h"
#include "io/TraversalMode.h"
#include "mdl/Game.h"
#include "mdl/GameConfig.h"
#include "mdl/GameImpl.h"

#include "kdl/path_utils.h"
#include "kdl/result.h"
#include "kdl/vector_utils.h"

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace tb::mdl
{
namespace
{

Result<void> migrateConfigFiles(
  const std::filesystem::path& userGameDir, const GameConfig& config)
{
  const auto legacyDir = userGameDir / config.name;
  const auto newDir = userGameDir / config.configFileFolder();

  if (io::Disk::pathInfo(legacyDir) == io::PathInfo::Directory)
  {
    switch (io::Disk::pathInfo(newDir))
    {
    case io::PathInfo::File:
      return Error{"User config folder for '" + config.name + "' is a file"};
    case io::PathInfo::Directory:
      break;
    case io::PathInfo::Unknown:
      std::cout << "Migrating user config files for '" << config.name << "'\n";
      return io::Disk::renameDirectory(legacyDir, newDir);
    }
  }
  return Result<void>{};
}

} // namespace

GameFactory& GameFactory::instance()
{
  static auto instance = GameFactory{};
  return instance;
}

Result<std::vector<std::string>> GameFactory::initialize(
  const GamePathConfig& gamePathConfig)
{
  return initializeFileSystem(gamePathConfig)
         | kdl::and_then([&]() { return loadGameConfigs(gamePathConfig); });
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

  auto virtualFs = io::VirtualFileSystem{};

  // All of the current search paths from highest to lowest priority
  for (auto it = gameConfigSearchDirs.rbegin(); it != gameConfigSearchDirs.rend(); ++it)
  {
    const auto path = *it;
    virtualFs.mount({}, std::make_unique<io::DiskFileSystem>(path));
  }

  m_userGameDir = userGameDir;
  return io::Disk::createDirectory(m_userGameDir) | kdl::transform([&](auto) {
           m_configFs = std::make_unique<io::WritableVirtualFileSystem>(
             std::move(virtualFs),
             std::make_unique<io::WritableDiskFileSystem>(m_userGameDir));
         });
}

Result<std::vector<std::string>> GameFactory::loadGameConfigs(
  const GamePathConfig& gamePathConfig)
{
  return m_configFs->find(
           {},
           io::TraversalMode::Recursive,
           io::makeFilenamePathMatcher("GameConfig.cfg"))
         | kdl::transform([&](auto configFiles) {
             auto errors = std::vector<std::string>{};
             kdl::vec_transform(configFiles, [&](const auto& configFilePath) {
               return loadGameConfig(gamePathConfig, configFilePath)
                      | kdl::transform_error([&](auto e) {
                          errors.push_back(
                            "Failed to load game configuration file '"
                            + configFilePath.string() + "': " + e.msg);
                        });
             });
             return errors;
           });
}

Result<void> GameFactory::loadGameConfig(
  const GamePathConfig& gamePathConfig, const std::filesystem::path& path)
{
  return m_configFs->openFile(path).join(m_configFs->makeAbsolute(path))
         | kdl::and_then([&](auto configFile, auto absolutePath) -> Result<void> {
             auto reader = configFile->reader().buffer();
             auto parser = io::GameConfigParser{reader.stringView(), absolutePath};
             try
             {
               auto config = parser.parse();

               migrateConfigFiles(gamePathConfig.userGameDir, config)
                 | kdl::transform_error([&](auto e) {
                     std::cerr << "Could not migrate user config files: '" << e.msg
                               << "\n";
                   });

               loadCompilationConfig(config);
               loadGameEngineConfig(config);

               const auto configName = config.name;
               m_configs.emplace(configName, std::move(config));
               kdl::wrap_set(m_names).insert(configName);

               const auto gamePathPrefPath =
                 std::filesystem::path{"Games"} / configName / "Path";
               m_gamePaths.emplace(
                 configName, Preference<std::filesystem::path>{gamePathPrefPath, {}});

               const auto defaultEnginePrefPath =
                 std::filesystem::path{"Games"} / configName / "Default Engine";
               m_defaultEngines.emplace(
                 configName,
                 Preference<std::filesystem::path>{defaultEnginePrefPath, {}});

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
  const auto path = gameConfig.configFileFolder() / "CompilationProfiles.cfg";
  try
  {
    if (m_configFs->pathInfo(path) == io::PathInfo::File)
    {
      m_configFs->openFile(path).join(m_configFs->makeAbsolute(path))
        | kdl::transform([&](auto profilesFile, auto absolutePath) {
            auto reader = profilesFile->reader().buffer();
            auto parser = io::CompilationConfigParser{reader.stringView(), absolutePath};
            gameConfig.compilationConfig = parser.parse();
            gameConfig.compilationConfigParseFailed = false;
          })
        | kdl::transform_error([&](auto e) {
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
  const auto path = gameConfig.configFileFolder() / "GameEngineProfiles.cfg";
  try
  {
    if (m_configFs->pathInfo(path) == io::PathInfo::File)
    {
      m_configFs->openFile(path).join(m_configFs->makeAbsolute(path))
        | kdl::transform([&](auto profilesFile, auto absolutePath) {
            auto reader = profilesFile->reader().buffer();
            auto parser = io::GameEngineConfigParser{reader.stringView(), absolutePath};
            gameConfig.gameEngineConfig = parser.parse();
            gameConfig.gameEngineConfigParseFailed = false;
          })
        | kdl::transform_error([&](auto e) {
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
  io::WritableFileSystem& fs, const std::filesystem::path& path)
{
  const auto backupPath = kdl::path_add_extension(path, ".bak");
  return fs.copyFile(path, backupPath) | kdl::transform([&]() { return backupPath; });
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
  auto writer = io::CompilationConfigWriter{compilationConfig, stream};
  writer.writeConfig();

  const auto profilesPath = gameConfig.configFileFolder() / "CompilationProfiles.cfg";
  if (gameConfig.compilationConfigParseFailed)
  {
    backupFile(*m_configFs, profilesPath) | kdl::and_then([&](auto backupPath) {
      return m_configFs->makeAbsolute(profilesPath)
               .join(m_configFs->makeAbsolute(backupPath))
             | kdl::transform([&](auto absProfilesPath, auto absBackupPath) {
                 logger.warn() << "Backed up malformed compilation config "
                               << absProfilesPath << " to " << absBackupPath;
               });
    }) | kdl::transform_error([&](auto) {
      logger.error() << "Could not back up malformed compilation config";
    });

    gameConfig.compilationConfigParseFailed = false;
  }

  m_configFs->createDirectory(profilesPath.parent_path()) | kdl::and_then([&](auto) {
    return m_configFs->createFileAtomic(profilesPath, stream.str());
  }) | kdl::transform([&]() {
    gameConfig.compilationConfig = std::move(compilationConfig);

    m_configFs->makeAbsolute(profilesPath) | kdl::transform([&](auto absProfilesPath) {
      logger.debug() << "Wrote compilation config to " << absProfilesPath;
    }) | kdl::transform_error([](auto) {
      // Can't really do anything
    });
  }) | kdl::transform_error([&](const auto& e) {
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
  auto writer = io::GameEngineConfigWriter{gameEngineConfig, stream};
  writer.writeConfig();

  const auto profilesPath = gameConfig.configFileFolder() / "GameEngineProfiles.cfg";
  if (gameConfig.gameEngineConfigParseFailed)
  {
    backupFile(*m_configFs, profilesPath) | kdl::and_then([&](auto backupPath) {
      return m_configFs->makeAbsolute(profilesPath)
               .join(m_configFs->makeAbsolute(backupPath))
             | kdl::transform([&](auto absProfilesPath, auto absBackupPath) {
                 logger.warn() << "Backed up malformed game engine config "
                               << absProfilesPath << " to " << absBackupPath;
               });
    }) | kdl::transform_error([&](auto) {
      logger.error() << "Could not back up malformed game engine config";
    });

    gameConfig.gameEngineConfigParseFailed = false;
  }

  m_configFs->createDirectory(profilesPath.parent_path()) | kdl::and_then([&](auto) {
    return m_configFs->createFileAtomic(profilesPath, stream.str());
  }) | kdl::transform([&]() {
    gameConfig.gameEngineConfig = std::move(gameEngineConfig);
    m_configFs->makeAbsolute(profilesPath) | kdl::transform([&](auto absProfilesPath) {
      logger.debug() << "Wrote game engine config to " << absProfilesPath;
    }) | kdl::transform_error([](auto) {
      // Can't really do anything
    });
  }) | kdl::transform_error([&](const auto& e) {
    logger.error() << "Could not write game engine config: " << e.msg;
  });
}
} // namespace tb::mdl
