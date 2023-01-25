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

#pragma once

#include "IO/Path.h"
#include "Model/MapFormat.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
class Logger;

template <typename T>
class Preference;

namespace IO
{
class Path;
class WritableDiskFileSystem;
} // namespace IO

namespace Model
{
struct CompilationConfig;
class Game;
struct GameConfig;
class GameEngineConfig;

struct GamePathConfig
{
  std::vector<IO::Path> gameConfigSearchDirs;
  IO::Path userGameDir;
};

class GameFactory
{
private:
  using ConfigMap = std::map<std::string, GameConfig>;
  using GamePathMap = std::map<std::string, Preference<IO::Path>>;

  IO::Path m_userGameDir;
  std::unique_ptr<IO::WritableDiskFileSystem> m_configFS;

  std::vector<std::string> m_names;
  ConfigMap m_configs;
  mutable GamePathMap m_gamePaths;
  mutable GamePathMap m_defaultEngines;

public:
  static GameFactory& instance();

  /**
   * Initializes the game factory, must be called once when the application starts.
   * Initialization comprises building a file system to find the builtin and user-provided
   * game configurations and loading them.
   *
   * If the file system cannot be built, a FileSystemException is thrown. Since this is a
   * fatal error, the caller should inform the user of the error and terminate the
   * application.
   *
   * If a game configuration cannot be loaded due to parsing errors, the errors are
   * collected in a string list, but loading game configurations continues. The string
   * list is then thrown and should be caught by the caller to inform the user of any
   * errors.
   *
   * The given path config is used to build the file systems.
   *
   * @throw FileSystemException if the file system cannot be built.
   * @throw std::vector<std::string> if loading game configurations fails
   */
  void initialize(const GamePathConfig& gamePathConfig);
  /**
   * Saves the game engine configurations for the game with the given name.
   *
   * @param gameName the game for which the configurations should be saved
   * @param gameEngineConfig new config to save
   *
   * @throw GameException if no game with the given name exists
   */
  void saveGameEngineConfig(
    const std::string& gameName, const GameEngineConfig& gameEngineConfig);
  /**
   * Saves the compilation configurations for the game with the given name.
   *
   * @param gameName the game for which the configurations should be saved
   * @param compilationConfig new config to save
   * @param logger the logger
   *
   * @throw GameException if no config with the given name exists
   */
  void saveCompilationConfig(
    const std::string& gameName,
    const CompilationConfig& compilationConfig,
    Logger& logger);

  const std::vector<std::string>& gameList() const;
  size_t gameCount() const;
  std::shared_ptr<Game> createGame(const std::string& gameName, Logger& logger);

  std::vector<std::string> fileFormats(const std::string& gameName) const;
  IO::Path iconPath(const std::string& gameName) const;
  IO::Path gamePath(const std::string& gameName) const;
  bool setGamePath(const std::string& gameName, const IO::Path& gamePath);
  bool isGamePathPreference(const std::string& gameName, const IO::Path& prefPath) const;

  IO::Path compilationToolPath(
    const std::string& gameName, const std::string& toolName) const;
  bool setCompilationToolPath(
    const std::string& gameName, const std::string& toolName, const IO::Path& gamePath);

  GameConfig& gameConfig(const std::string& gameName);
  const GameConfig& gameConfig(const std::string& gameName) const;

  /**
   * Scans the map file at the given path to find game type and map format comments and
   * returns the name of the game and the map format.
   *
   * If no game comment is found or the game is unknown, an empty string is returned as
   * the game name. If no map format comment is found or the format is unknown,
   * MapFormat::Unknown is returned as the map format.
   */
  std::pair<std::string, MapFormat> detectGame(const IO::Path& path) const;

  /**
   * Returns the directory for user game configurations.
   * Solely for showing these to the user.
   *
   * Must not be called before initialize() was called.
   */
  const IO::Path& userGameConfigsPath() const;

private:
  GameFactory();
  void initializeFileSystem(const GamePathConfig& gamePathConfig);
  void loadGameConfigs();
  void loadGameConfig(const IO::Path& path);
  void doLoadGameConfig(const IO::Path& path);
  void loadCompilationConfig(GameConfig& gameConfig);
  void loadGameEngineConfig(GameConfig& gameConfig);

  void writeCompilationConfig(
    GameConfig& gameConfig, const CompilationConfig& compilationConfig, Logger& logger);
  void writeGameEngineConfig(
    GameConfig& gameConfig, const GameEngineConfig& gameEngineConfig);
};
} // namespace Model
} // namespace TrenchBroom
