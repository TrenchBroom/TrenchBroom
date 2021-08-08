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
#include "Logger.h"
#include "PreferenceManager.h"
#include "RecoverableExceptions.h"
#include "IO/CompilationConfigParser.h"
#include "IO/CompilationConfigWriter.h"
#include "IO/DiskFileSystem.h"
#include "IO/IOUtils.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/GameConfigParser.h"
#include "IO/GameEngineConfigParser.h"
#include "IO/GameEngineConfigWriter.h"
#include "IO/IOUtils.h"
#include "IO/Path.h"
#include "IO/SystemPaths.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/GameImpl.h"

#include <kdl/collection_utils.h>
#include <kdl/string_compare.h>
#include <kdl/string_utils.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        GameFactory& GameFactory::instance() {
            static GameFactory instance;
            return instance;
        }

        void GameFactory::initialize() {
            initializeFileSystem();
            loadGameConfigs();
        }

        void GameFactory::saveGameEngineConfig(const std::string& gameName, const GameEngineConfig& gameEngineConfig) {
            auto& config = gameConfig(gameName);
            writeGameEngineConfig(config, gameEngineConfig);
        }

        void GameFactory::saveCompilationConfig(const std::string& gameName, const CompilationConfig& compilationConfig, Logger& logger) {
            auto& config = gameConfig(gameName);
            writeCompilationConfig(config, compilationConfig, logger);
        }

        const std::vector<std::string>& GameFactory::gameList() const {
            return m_names;
        }

        size_t GameFactory::gameCount() const {
            return m_configs.size();
        }

        std::shared_ptr<Game> GameFactory::createGame(const std::string& gameName, Logger& logger) {
            return std::make_shared<GameImpl>(gameConfig(gameName), gamePath(gameName), logger);
        }

        std::vector<std::string> GameFactory::fileFormats(const std::string& gameName) const {
            std::vector<std::string> result;
            for (const auto& format : gameConfig(gameName).fileFormats()) {
                result.push_back(format.format);
            }
            return result;
        }

        IO::Path GameFactory::iconPath(const std::string& gameName) const {
            const auto& config = gameConfig(gameName);
            return config.findConfigFile(config.icon());
        }

        IO::Path GameFactory::gamePath(const std::string& gameName) const {
            const auto it = m_gamePaths.find(gameName);
            if (it == std::end(m_gamePaths)) {
                throw GameException("Unknown game: " + gameName);
            }
            auto& pref = it->second;
            return PreferenceManager::instance().get(pref);
        }

        bool GameFactory::setGamePath(const std::string& gameName, const IO::Path& gamePath) {
            const auto it = m_gamePaths.find(gameName);
            if (it == std::end(m_gamePaths)) {
                throw GameException("Unknown game: " + gameName);
            }
            auto& pref = it->second;
            return PreferenceManager::instance().set(pref, gamePath);
        }

        bool GameFactory::isGamePathPreference(const std::string& gameName, const IO::Path& prefPath) const {
            const auto it = m_gamePaths.find(gameName);
            if (it == std::end(m_gamePaths)) {
                throw GameException("Unknown game: " + gameName);
            }
            auto& pref = it->second;
            return pref.path() == prefPath;
        }

        static Preference<IO::Path>& compilationToolPathPref(const std::string& gameName, const std::string& toolName) {
            auto& prefs = PreferenceManager::instance();
            auto& pref = prefs.dynamicPreference(IO::Path("Games") + IO::Path(gameName) + IO::Path("Tool Path") + IO::Path(toolName), IO::Path());
            return pref;
        }

        IO::Path GameFactory::compilationToolPath(const std::string& gameName, const std::string& toolName) const {
            return PreferenceManager::instance().get(compilationToolPathPref(gameName, toolName));
        }

        bool GameFactory::setCompilationToolPath(const std::string& gameName, const std::string& toolName, const IO::Path& gamePath) {
            return PreferenceManager::instance().set(compilationToolPathPref(gameName, toolName), gamePath);
        }

        GameConfig& GameFactory::gameConfig(const std::string& name) {
            const auto cIt = m_configs.find(name);
            if (cIt == std::end(m_configs)) {
                throw GameException("Unknown game: " + name);
            }
            return cIt->second;
        }

        const GameConfig& GameFactory::gameConfig(const std::string& name) const {
            const auto cIt = m_configs.find(name);
            if (cIt == std::end(m_configs)) {
                throw GameException("Unknown game: " + name);
            }
            return cIt->second;
        }

        std::pair<std::string, MapFormat> GameFactory::detectGame(const IO::Path& path) const {
            std::ifstream stream = openPathAsInputStream(path);
            if (!stream.is_open()) {
                throw FileSystemException("Cannot open file: " + path.asString());
            }

            std::string gameName = IO::readGameComment(stream);
            if (m_configs.find(gameName) == std::end(m_configs)) {
                gameName = "";
            }
            
            const std::string formatName = IO::readFormatComment(stream);
            const MapFormat format = formatFromName(formatName);
            
            return std::make_pair(gameName, format);
        }

        IO::Path GameFactory::userGameConfigsPath() const {
            return IO::SystemPaths::userDataDirectory() + IO::Path("games");
        }

        GameFactory::GameFactory() = default;

        void GameFactory::initializeFileSystem() {
            // Gather the search paths we're going to use.
            // The rest of this function will be chaining together TB filesystem objects for these search paths.
            const IO::Path userGameDir = userGameConfigsPath();
            const std::vector<IO::Path> gameConfigSearchDirs = IO::SystemPaths::findResourceDirectories(IO::Path("games"));

            // All of the current search paths from highest to lowest priority
            std::unique_ptr<IO::DiskFileSystem> chain;
            for (auto it = gameConfigSearchDirs.rbegin(); it != gameConfigSearchDirs.rend(); ++it) {
                const IO::Path path = *it;

                if (chain != nullptr) {
                    chain = std::make_unique<IO::DiskFileSystem>(std::move(chain), path, false);
                } else {
                    chain = std::make_unique<IO::DiskFileSystem>(path, false);
                }
            }

            // This is where we write configs
            if (chain != nullptr) {
                m_configFS = std::make_unique<IO::WritableDiskFileSystem>(std::move(chain), userGameDir, true);
            } else {
                m_configFS = std::make_unique<IO::WritableDiskFileSystem>(userGameDir, true);
            }
        }

        void GameFactory::loadGameConfigs() {
            std::vector<std::string> errors;

            const auto configFiles = m_configFS->findItemsRecursively(IO::Path(""), IO::FileNameMatcher("GameConfig.cfg"));
            for (const auto& configFilePath : configFiles) {
                try {
                loadGameConfig(configFilePath);
                } catch (const std::exception& e) {
                    errors.push_back(kdl::str_to_string("Could not load game configuration file ", configFilePath, ": ", e.what()));
                }
            }

            m_names = kdl::col_sort(std::move(m_names), kdl::cs::string_less());

            if (!errors.empty()) {
                throw errors;
            }
        }

        void GameFactory::loadGameConfig(const IO::Path& path) {
            try {
                doLoadGameConfig(path);
            } catch (const RecoverableException& e) {
                e.recover();
                doLoadGameConfig(path);
            }
        }

        void GameFactory::doLoadGameConfig(const IO::Path& path) {
            const auto configFile = m_configFS->openFile(path);
            const auto absolutePath = m_configFS->makeAbsolute(path);
            auto reader = configFile->reader().buffer();
            IO::GameConfigParser parser(reader.stringView(), absolutePath);
            GameConfig config = parser.parse();

            loadCompilationConfig(config);
            loadGameEngineConfig(config);

            const auto configName = config.name();
            m_configs.emplace(std::make_pair(configName, std::move(config)));
            m_names.push_back(configName);

            const auto gamePathPrefPath = IO::Path("Games") + IO::Path(configName) + IO::Path("Path");
            m_gamePaths.insert(std::make_pair(configName, Preference<IO::Path>(gamePathPrefPath, IO::Path())));

            const auto defaultEnginePrefPath = IO::Path("Games") + IO::Path(configName) + IO::Path("Default Engine");
            m_defaultEngines.insert(std::make_pair(configName, Preference<IO::Path>(defaultEnginePrefPath, IO::Path())));
        }

        void GameFactory::loadCompilationConfig(GameConfig& gameConfig) {
            const auto path = IO::Path(gameConfig.name()) + IO::Path("CompilationProfiles.cfg");
            try {
                if (m_configFS->fileExists(path)) {
                    const auto profilesFile = m_configFS->openFile(path);
                    auto reader = profilesFile->reader().buffer();
                    IO::CompilationConfigParser parser(reader.stringView(), m_configFS->makeAbsolute(path));
                    gameConfig.setCompilationConfig(parser.parse());
                }
            } catch (const Exception& e) {
                std::cerr << "Could not load compilation configuration '" + path.asString() + "': " + std::string(e.what()) << "\n";
                gameConfig.setCompilationConfigParseFailed(true);
            }
        }

        void GameFactory::loadGameEngineConfig(GameConfig& gameConfig) {
            const auto path = IO::Path(gameConfig.name()) + IO::Path("GameEngineProfiles.cfg");
            try {
                if (m_configFS->fileExists(path)) {
                    const auto profilesFile = m_configFS->openFile(path);
                    auto reader = profilesFile->reader().buffer();
                    IO::GameEngineConfigParser parser(reader.stringView(), m_configFS->makeAbsolute(path));
                    gameConfig.setGameEngineConfig(parser.parse());
                }
            } catch (const Exception& e) {
                std::cerr << "Could not load game engine configuration '" + path.asString() + "': " + std::string(e.what()) << "\n";
                gameConfig.setGameEngineConfigParseFailed(true);
            }
        }

        static IO::Path backupFile(IO::WritableDiskFileSystem& fs, const IO::Path& path) {
            const IO::Path backupPath = path.addExtension("bak");
            fs.copyFile(path, backupPath, true);
            return backupPath;
        }

        void GameFactory::writeCompilationConfig(GameConfig& gameConfig, const CompilationConfig& compilationConfig, Logger& logger) {
            if (!gameConfig.compilationConfigParseFailed()
                && gameConfig.compilationConfig() == compilationConfig) {
                // NOTE: this is not just an optimization, but important for ensuring that
                // we don't clobber data saved by a newer version of TB, unless we actually make changes
                // to the config in this version of TB (see: https://github.com/TrenchBroom/TrenchBroom/issues/3424)
                logger.debug() << "Skipping writing unchanged compilation config for " << gameConfig.name();
                return;
            }

            std::stringstream stream;
            IO::CompilationConfigWriter writer(compilationConfig, stream);
            writer.writeConfig();

            const auto profilesPath = IO::Path(gameConfig.name()) + IO::Path("CompilationProfiles.cfg");
            if (gameConfig.compilationConfigParseFailed()) {
                const IO::Path backupPath = backupFile(*m_configFS, profilesPath);

                logger.warn() << "Backed up malformed compilation config " << m_configFS->makeAbsolute(profilesPath).asString()
                              << " to " << m_configFS->makeAbsolute(backupPath).asString();

                gameConfig.setCompilationConfigParseFailed(false);
            }
            m_configFS->createFileAtomic(profilesPath, stream.str());
            gameConfig.setCompilationConfig(compilationConfig);
            logger.debug() << "Wrote compilation config to " << m_configFS->makeAbsolute(profilesPath).asString();
        }

        void GameFactory::writeGameEngineConfig(GameConfig& gameConfig, const GameEngineConfig& gameEngineConfig) {
            if (!gameConfig.gameEngineConfigParseFailed()
                && gameConfig.gameEngineConfig() == gameEngineConfig) {
                std::cout << "Skipping writing unchanged game engine config for " << gameConfig.name();
                return;
            }

            std::stringstream stream;
            IO::GameEngineConfigWriter writer(gameEngineConfig, stream);
            writer.writeConfig();

            const auto profilesPath = IO::Path(gameConfig.name()) + IO::Path("GameEngineProfiles.cfg");
            if (gameConfig.gameEngineConfigParseFailed()) {
                const IO::Path backupPath = backupFile(*m_configFS, profilesPath);

                std::cerr << "Backed up malformed game engine config " << m_configFS->makeAbsolute(profilesPath).asString()
                          << " to " << m_configFS->makeAbsolute(backupPath).asString() << std::endl;

                gameConfig.setGameEngineConfigParseFailed(false);
            }
            m_configFS->createFileAtomic(profilesPath, stream.str());
            gameConfig.setGameEngineConfig(gameEngineConfig);
            std::cout << "Wrote game engine config to " << m_configFS->makeAbsolute(profilesPath).asString() << std::endl;
        }
    }
}
