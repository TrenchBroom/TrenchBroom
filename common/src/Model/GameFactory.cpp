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
#include "PreferenceManager.h"
#include "RecoverableExceptions.h"
#include "IO/CompilationConfigParser.h"
#include "IO/CompilationConfigWriter.h"
#include "IO/DiskFileSystem.h"
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

        void GameFactory::saveAllConfigs() {
            writeCompilationConfigs();
            writeGameEngineConfigs();
        }

        void GameFactory::saveConfigs(const std::string& gameName) {
            const auto& config = gameConfig(gameName);
            writeCompilationConfig(config);
            writeGameEngineConfig(config);
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
            std::fstream stream(path.asString().c_str(), std::ios::in);
            if (!stream.is_open()) {
                throw FileSystemException("Cannot open file: " + path.asString());
            }

            const std::string gameName = IO::readGameComment(stream);
            const std::string formatName = IO::readFormatComment(stream);

            const MapFormat format = mapFormat(formatName);
            if (gameName.empty() || format == MapFormat::Unknown) {
                return std::make_pair("", MapFormat::Unknown);
            } else {
                return std::make_pair(gameName, format);
            }
        }

        GameFactory::GameFactory() = default;

        void GameFactory::initializeFileSystem() {
            // Gather the search paths we're going to use.
            // The rest of this function will be chaining together TB filesystem objects for these search paths.
            const IO::Path userGameDir = IO::SystemPaths::userDataDirectory() + IO::Path("games");
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

            kdl::sort(m_names, kdl::cs::string_less());

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
            IO::GameConfigParser parser(std::begin(reader), std::end(reader), absolutePath);
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
                    IO::CompilationConfigParser parser(std::begin(reader), std::end(reader), m_configFS->makeAbsolute(path));
                    gameConfig.setCompilationConfig(parser.parse());
                }
            } catch (const Exception& e) {
                throw FileDeletingException("Could not load compilation configuration '" + path.asString() + "': " + std::string(e.what()), m_configFS->makeAbsolute(path));
            }
        }

        void GameFactory::loadGameEngineConfig(GameConfig& gameConfig) {
            const auto path = IO::Path(gameConfig.name()) + IO::Path("GameEngineProfiles.cfg");
            try {
                if (m_configFS->fileExists(path)) {
                    const auto profilesFile = m_configFS->openFile(path);
                    auto reader = profilesFile->reader().buffer();
                    IO::GameEngineConfigParser parser(std::begin(reader), std::end(reader), m_configFS->makeAbsolute(path));
                    gameConfig.setGameEngineConfig(parser.parse());
                }
            } catch (const Exception& e) {
                throw FileDeletingException("Could not load game engine configuration '" + path.asString() + "': " + std::string(e.what()), m_configFS->makeAbsolute(path));
            }
        }

        void GameFactory::writeCompilationConfigs() {
            for (const auto& entry : m_configs) {
                const auto& gameConfig = entry.second;
                writeCompilationConfig(gameConfig);
            }
        }

        void GameFactory::writeCompilationConfig(const GameConfig& gameConfig) {
            std::stringstream stream;
            IO::CompilationConfigWriter writer(gameConfig.compilationConfig(), stream);
            writer.writeConfig();

            const auto profilesPath = IO::Path(gameConfig.name()) + IO::Path("CompilationProfiles.cfg");
            m_configFS->createFile(profilesPath, stream.str());
        }

        void GameFactory::writeGameEngineConfigs() {
            for (const auto& entry : m_configs) {
                const auto& gameConfig = entry.second;
                writeGameEngineConfig(gameConfig);
            }
        }

        void GameFactory::writeGameEngineConfig(const GameConfig& gameConfig) {
            std::stringstream stream;
            IO::GameEngineConfigWriter writer(gameConfig.gameEngineConfig(), stream);
            writer.writeConfig();

            const auto profilesPath = IO::Path(gameConfig.name()) + IO::Path("GameEngineProfiles.cfg");
            m_configFS->createFile(profilesPath, stream.str());
        }
    }
}
