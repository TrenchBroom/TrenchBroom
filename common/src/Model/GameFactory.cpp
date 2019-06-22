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

#include "CollectionUtils.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "IO/CompilationConfigParser.h"
#include "IO/CompilationConfigWriter.h"
#include "IO/DiskFileSystem.h"
#include "IO/File.h"
#include "IO/FileMatcher.h"
#include "IO/FileSystem.h"
#include "IO/GameConfigParser.h"
#include "IO/GameEngineConfigParser.h"
#include "IO/GameEngineConfigWriter.h"
#include "IO/IOUtils.h"
#include "IO/Path.h"
#include "IO/SystemPaths.h"
#include "Model/Game.h"
#include "Model/GameImpl.h"

#include "Exceptions.h"
#include "RecoverableExceptions.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        GameFactory::~GameFactory() {
            writeCompilationConfigs();
            writeGameEngineConfigs();
        }

        GameFactory& GameFactory::instance() {
            static GameFactory instance;
            return instance;
        }

        void GameFactory::initialize() {
            initializeFileSystem();
            loadGameConfigs();
        }

        void GameFactory::saveConfigs(const String& gameName) {
            const auto& config = gameConfig(gameName);
            writeCompilationConfig(config);
            writeGameEngineConfig(config);
        }

        const StringList& GameFactory::gameList() const {
            return m_names;
        }

        size_t GameFactory::gameCount() const {
            return m_configs.size();
        }

        GameSPtr GameFactory::createGame(const String& gameName, Logger& logger) {
            return GameSPtr(new GameImpl(gameConfig(gameName), gamePath(gameName), logger));
        }

        StringList GameFactory::fileFormats(const String& gameName) const {
            StringList result;
            for (const auto& format : gameConfig(gameName).fileFormats()) {
                result.push_back(format.format);
            }
            return result;
        }

        IO::Path GameFactory::iconPath(const String& gameName) const {
            const auto& config = gameConfig(gameName);
            return config.findConfigFile(config.icon());
        }

        IO::Path GameFactory::gamePath(const String& gameName) const {
            const auto it = m_gamePaths.find(gameName);
            if (it == std::end(m_gamePaths)) {
                throw GameException("Unknown game: " + gameName);
            }
            auto& pref = it->second;
            return PreferenceManager::instance().get(pref);
        }

        bool GameFactory::setGamePath(const String& gameName, const IO::Path& gamePath) {
            const auto it = m_gamePaths.find(gameName);
            if (it == std::end(m_gamePaths)) {
                throw GameException("Unknown game: " + gameName);
            }
            auto& pref = it->second;
            return PreferenceManager::instance().set(pref, gamePath);
        }

        bool GameFactory::isGamePathPreference(const String& gameName, const IO::Path& prefPath) const {
            const auto it = m_gamePaths.find(gameName);
            if (it == std::end(m_gamePaths)) {
                throw GameException("Unknown game: " + gameName);
            }
            auto& pref = it->second;
            return pref.path() == prefPath;
        }

        GameConfig& GameFactory::gameConfig(const String& name) {
            const auto cIt = m_configs.find(name);
            if (cIt == std::end(m_configs)) {
                throw GameException("Unknown game: " + name);
            }
            return cIt->second;
        }

        const GameConfig& GameFactory::gameConfig(const String& name) const {
            const auto cIt = m_configs.find(name);
            if (cIt == std::end(m_configs)) {
                throw GameException("Unknown game: " + name);
            }
            return cIt->second;
        }

        std::pair<String, MapFormat> GameFactory::detectGame(const IO::Path& path) const {
            if (path.isEmpty() || !IO::Disk::fileExists(IO::Disk::fixPath(path)))
                return std::make_pair("", MapFormat::Unknown);

            IO::OpenStream open(path, false);
            const String gameName = IO::readGameComment(open.stream);
            const String formatName = IO::readFormatComment(open.stream);
            const MapFormat format = mapFormat(formatName);
            if (gameName.empty() || format == MapFormat::Unknown) {
                return std::make_pair("", MapFormat::Unknown);
            } else {
                return std::make_pair(gameName, format);
            }
        }

        GameFactory::GameFactory() = default;

        void GameFactory::initializeFileSystem() {
            const IO::Path resourceGameDir = IO::SystemPaths::findResourceDirectory(IO::Path("games"));
            // FIXME: Change back to "games". the problems is the userDataDirectory() overlaps with and has
            // a higher priority than the actual resources directory (TrenchBroom.app/Contents/Resources)
            // when searching for resources.
            const IO::Path userGameDir = IO::SystemPaths::userDataDirectory() + IO::Path("gameUserData");
            if (!resourceGameDir.isEmpty() &&IO::Disk::directoryExists(resourceGameDir)) {
                auto resourceFS = std::make_shared<IO::DiskFileSystem>(resourceGameDir);
                m_configFS = std::make_unique<IO::WritableDiskFileSystem>(std::move(resourceFS), userGameDir, true);
            } else {
                m_configFS = std::make_unique<IO::WritableDiskFileSystem>(userGameDir, true);
            }
        }

        void GameFactory::loadGameConfigs() {
            StringList errors;

            const auto configFiles = m_configFS->findItemsRecursively(IO::Path(""), IO::FileNameMatcher("GameConfig.cfg"));
            for (const auto& configFilePath : configFiles) {
                try {
                loadGameConfig(configFilePath);
                } catch (const std::exception& e) {
                    StringStream str;
                    str << "Could not load game configuration file " << configFilePath << ": " << e.what();
                    errors.push_back(str.str());
                }
            }

            StringUtils::sortCaseSensitive(m_names);

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
                throw FileDeletingException("Could not load compilation configuration '" + path.asString() + "': " + String(e.what()), m_configFS->makeAbsolute(path));
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
                throw FileDeletingException("Could not load game engine configuration '" + path.asString() + "': " + String(e.what()), m_configFS->makeAbsolute(path));
            }
        }

        void GameFactory::writeCompilationConfigs() {
            for (const auto& entry : m_configs) {
                const auto& gameConfig = entry.second;
                writeCompilationConfig(gameConfig);
            }
        }

        void GameFactory::writeCompilationConfig(const GameConfig& gameConfig) {
            StringStream stream;
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
            StringStream stream;
            IO::GameEngineConfigWriter writer(gameConfig.gameEngineConfig(), stream);
            writer.writeConfig();

            const auto profilesPath = IO::Path(gameConfig.name()) + IO::Path("GameEngineProfiles.cfg");
            m_configFS->createFile(profilesPath, stream.str());
        }
    }
}
