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

#include "mdl/GameManager.h"

#include "Logger.h"
#include "fs/DiskFileSystem.h"
#include "fs/DiskIO.h"
#include "fs/PathInfo.h"
#include "fs/TraversalMode.h"
#include "fs/VirtualFileSystem.h"
#include "mdl/CompilationConfigParser.h"
#include "mdl/GameConfigParser.h"
#include "mdl/GameEngineConfigParser.h"
#include "mdl/GameEngineConfigWriter.h"
#include "mdl/GameInfo.h"

#include "kd/const_overload.h"
#include "kd/path_utils.h"
#include "kd/result_fold.h"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace tb::mdl
{
namespace
{

const auto gameConfigFilename = "GameConfig.cfg";
const auto compilationConfigFilename = "CompilationProfiles.cfg";
const auto gameEngineConfigFilename = "GameEngineProfiles.cfg";

Result<std::unique_ptr<fs::WritableVirtualFileSystem>> createFileSystem(
  const std::vector<std::filesystem::path>& gameConfigSearchDirs,
  const std::filesystem::path& userGameDir)
{
  auto virtualFs = fs::VirtualFileSystem{};

  // All of the current search paths from highest to lowest priority
  for (auto it = gameConfigSearchDirs.rbegin(); it != gameConfigSearchDirs.rend(); ++it)
  {
    const auto path = *it;
    virtualFs.mount({}, std::make_unique<fs::DiskFileSystem>(path));
  }

  return fs::Disk::createDirectory(userGameDir) | kdl::transform([&](auto) {
           return std::make_unique<fs::WritableVirtualFileSystem>(
             std::move(virtualFs),
             std::make_unique<fs::WritableDiskFileSystem>(userGameDir));
         });
}

Result<void> migrateConfigFiles(
  const std::filesystem::path& userGameDir, const GameConfig& config)
{
  const auto legacyDir = userGameDir / config.name;
  const auto newDir = userGameDir / config.configFileFolder();

  if (fs::Disk::pathInfo(legacyDir) == fs::PathInfo::Directory)
  {
    switch (fs::Disk::pathInfo(newDir))
    {
    case fs::PathInfo::File:
      return Error{"User config folder for '" + config.name + "' is a file"};
    case fs::PathInfo::Directory:
      break;
    case fs::PathInfo::Unknown:
      return fs::Disk::renameDirectory(legacyDir, newDir);
    }
  }
  return Result<void>{};
}

Result<void> loadCompilationConfig(const fs::FileSystem& fs, GameInfo& gameInfo)
{
  const auto path = gameInfo.gameConfig.configFileFolder() / compilationConfigFilename;
  if (fs.pathInfo(path) == fs::PathInfo::File)
  {
    return fs.openFile(path) | kdl::and_then([&](auto profilesFile) {
             auto reader = profilesFile->reader().buffer();
             auto parser = CompilationConfigParser{reader.stringView()};
             return parser.parse();
           })
           | kdl::transform([&](auto compilationConfig) {
               gameInfo.compilationConfig = std::move(compilationConfig);
             })
           | kdl::if_error(
             [&](const auto&) { gameInfo.compilationConfigParseFailed = true; });
  }

  return Result<void>{};
}

Result<void> loadGameEngineConfig(const fs::FileSystem& fs, GameInfo& gameInfo)
{
  const auto path = gameInfo.gameConfig.configFileFolder() / gameEngineConfigFilename;
  if (fs.pathInfo(path) == fs::PathInfo::File)
  {
    return fs.openFile(path) | kdl::and_then([&](auto profilesFile) {
             auto reader = profilesFile->reader().buffer();
             auto parser = GameEngineConfigParser{reader.stringView()};
             return parser.parse();
           })
           | kdl::transform([&](auto gameEngineConfig) {
               gameInfo.gameEngineConfig = std::move(gameEngineConfig);
             })
           | kdl::if_error(
             [&](const auto&) { gameInfo.gameEngineConfigParseFailed = true; });
  }

  return Result<void>{};
}

Result<GameConfig> loadGameConfig(
  const fs::FileSystem& fs,
  const std::filesystem::path& userGameDir,
  const std::filesystem::path& path)
{
  return fs.openFile(path).join(fs.makeAbsolute(path))
         | kdl::and_then([&](auto configFile, auto absolutePath) {
             auto reader = configFile->reader().buffer();
             auto parser = GameConfigParser{reader.stringView(), absolutePath};
             return parser.parse();
           })
         | kdl::transform([&](auto config) {
             migrateConfigFiles(userGameDir, config) | kdl::transform_error([&](auto e) {
               std::cerr << "Could not migrate user config files: '" << e.msg << "\n";
             });
             return config;
           });
}

Result<GameInfo> loadGameInfo(
  const fs::FileSystem& fs,
  const std::filesystem::path& userGameDir,
  const std::filesystem::path& path,
  std::vector<std::string>& warnings)
{
  const auto saveWarning = [&](const auto& e) { warnings.push_back(e.msg); };

  return loadGameConfig(fs, userGameDir, path) | kdl::transform([&](auto gameConfig) {
           auto gameInfo = makeGameInfo(std::move(gameConfig));

           loadCompilationConfig(fs, gameInfo) | kdl::transform_error(saveWarning);
           loadGameEngineConfig(fs, gameInfo) | kdl::transform_error(saveWarning);

           return gameInfo;
         });
}

Result<std::vector<GameInfo>> loadGameInfos(
  const fs::FileSystem& fs,
  const std::filesystem::path& userGameDir,
  std::vector<std::string>& warnings)
{
  return fs.find(
           {},
           fs::TraversalMode::Recursive,
           fs::makeFilenamePathMatcher(gameConfigFilename))
         | kdl::transform([&](auto configFiles) {
             auto [gameInfos, errors] =
               configFiles | std::views::transform([&](const auto& configFilePath) {
                 return loadGameInfo(fs, userGameDir, configFilePath, warnings);
               })
               | kdl::collect();

             return std::move(gameInfos);
           });
}

void backupFile(
  fs::WritableFileSystem& fs,
  const std::filesystem::path& profilesPath,
  bool& parseFailed,
  Logger& logger)
{
  if (parseFailed)
  {
    const auto backupPath = kdl::path_add_extension(profilesPath, ".bak");
    fs.copyFile(profilesPath, backupPath) | kdl::and_then([&]() {
      return fs.makeAbsolute(profilesPath).join(fs.makeAbsolute(backupPath))
             | kdl::transform([&](auto absProfilesPath, auto absBackupPath) {
                 logger.warn() << "Backed up malformed config file " << absProfilesPath
                               << " to " << absBackupPath;
               });
    }) | kdl::transform_error([&](auto) {
      logger.error() << "Could not back up malformed config file";
    });

    parseFailed = false;
  }
}

Result<void> writeCompilationConfig(
  fs::WritableFileSystem& fs,
  GameInfo& gameInfo,
  CompilationConfig compilationConfig,
  Logger& logger)
{
  if (
    !gameInfo.compilationConfigParseFailed
    && gameInfo.compilationConfig == compilationConfig)
  {
    // NOTE: this is not just an optimization, but important for ensuring that
    // we don't clobber data saved by a newer version of TB, unless we actually
    // make changes to the config in this version of TB (see:
    // https://github.com/TrenchBroom/TrenchBroom/issues/3424)
    logger.debug() << "Skipping writing unchanged compilation config for "
                   << gameInfo.gameConfig.name;
    return Result<void>{};
  }

  const auto profilesPath =
    gameInfo.gameConfig.configFileFolder() / compilationConfigFilename;
  backupFile(fs, profilesPath, gameInfo.compilationConfigParseFailed, logger);

  return fs.createDirectory(profilesPath.parent_path()) | kdl::and_then([&](auto) {
           auto stream = std::stringstream{};
           stream << toValue(compilationConfig) << "\n";
           return fs.createFileAtomic(profilesPath, stream.str());
         })
         | kdl::transform([&]() {
             gameInfo.compilationConfig = std::move(compilationConfig);
             logger.debug() << "Wrote compilation config to " << profilesPath;
           });
}

Result<void> writeGameEngineConfig(
  fs::WritableFileSystem& fs,
  GameInfo& gameInfo,
  GameEngineConfig gameEngineConfig,
  Logger& logger)
{
  if (
    !gameInfo.gameEngineConfigParseFailed
    && gameInfo.gameEngineConfig == gameEngineConfig)
  {
    // NOTE: this is not just an optimization, but important for ensuring that
    // we don't clobber data saved by a newer version of TB, unless we actually
    // make changes to the config in this version of TB (see:
    // https://github.com/TrenchBroom/TrenchBroom/issues/3424)
    logger.debug() << "Skipping writing unchanged game engine config for "
                   << gameInfo.gameConfig.name;
    return Result<void>{};
  }

  const auto profilesPath =
    gameInfo.gameConfig.configFileFolder() / gameEngineConfigFilename;
  backupFile(fs, profilesPath, gameInfo.gameEngineConfigParseFailed, logger);

  return fs.createDirectory(profilesPath.parent_path()) | kdl::and_then([&](auto) {
           auto stream = std::stringstream{};
           auto writer = GameEngineConfigWriter{gameEngineConfig, stream};
           writer.writeConfig();

           return fs.createFileAtomic(profilesPath, stream.str());
         })
         | kdl::transform([&]() {
             gameInfo.gameEngineConfig = std::move(gameEngineConfig);
             logger.debug() << "Wrote game engine config to " << profilesPath;
           });
}

} // namespace

GameManager::GameManager(
  std::unique_ptr<fs::WritableFileSystem> configFs, std::vector<GameInfo> gameInfos)
  : m_configFs{std::move(configFs)}
  , m_gameInfos{std::move(gameInfos)}
{
  std::ranges::sort(m_gameInfos, [](const auto& lhs, const auto& rhs) {
    return lhs.gameConfig.name < rhs.gameConfig.name;
  });
}

GameManager::GameManager(GameManager&&) noexcept = default;
GameManager& GameManager::operator=(GameManager&&) noexcept = default;
GameManager::~GameManager() = default;

const std::vector<GameInfo>& GameManager::gameInfos() const
{
  return m_gameInfos;
}

const GameInfo* GameManager::gameInfo(const std::string_view gameName) const
{
  if (const auto iGameInfo = std::ranges::find_if(
        m_gameInfos,
        [&](const auto& gameInfo) { return gameInfo.gameConfig.name == gameName; });
      iGameInfo != m_gameInfos.end())
  {
    return &*iGameInfo;
  }
  return nullptr;
}

GameInfo* GameManager::gameInfo(const std::string_view gameName)
{
  return KDL_CONST_OVERLOAD(gameInfo(gameName));
}

Result<void> GameManager::updateCompilationConfig(
  const std::string_view gameName, CompilationConfig compilationConfig, Logger& logger)
{
  if (auto* info = gameInfo(gameName))
  {
    return writeCompilationConfig(
      *m_configFs, *info, std::move(compilationConfig), logger);
  }
  return Error{fmt::format("Unknown game: {}", gameName)};
}

Result<void> GameManager::updateGameEngineConfig(
  const std::string_view gameName, GameEngineConfig gameEngineConfig, Logger& logger)
{
  if (auto* info = gameInfo(gameName))
  {
    return writeGameEngineConfig(*m_configFs, *info, std::move(gameEngineConfig), logger);
  }
  return Error{fmt::format("Unknown game: {}", gameName)};
}

Result<kdl::multi_value<GameManager, std::vector<std::string>>> initializeGameManager(
  const std::vector<std::filesystem::path>& gameConfigSearchDirs,
  const std::filesystem::path& userGameDir)
{
  return createFileSystem(gameConfigSearchDirs, userGameDir)
         | kdl::and_then([&](auto fs) {
             auto warnings = std::vector<std::string>{};
             return loadGameInfos(*fs, userGameDir, warnings)
                    | kdl::transform([&](auto gameInfos) {
                        return kdl::multi_value{
                          GameManager{std::move(fs), std::move(gameInfos)},
                          std::move(warnings)};
                      });
           });
}

} // namespace tb::mdl
