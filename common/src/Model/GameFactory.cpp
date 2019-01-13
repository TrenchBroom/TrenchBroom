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
#include "Model/Tutorial.h"

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
        
        const StringList& GameFactory::gameList() const {
            return m_names;
        }

        size_t GameFactory::gameCount() const {
            return m_configs.size();
        }

        GameSPtr GameFactory::createGame(const String& gameName, Logger* logger) {
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

        GameFactory::GameFactory() {
            initializeFileSystem();
            loadGameConfigs();
        }
        
        void GameFactory::initializeFileSystem() {
            const IO::Path resourceGameDir = IO::SystemPaths::findResourceDirectory(IO::Path("games"));
            const IO::Path userGameDir = IO::SystemPaths::userDataDirectory() + IO::Path("games");
            if (IO::Disk::directoryExists(resourceGameDir)) {
                auto resourceFS = std::make_unique<IO::DiskFileSystem>(resourceGameDir);
                m_configFS = std::make_unique<IO::WritableDiskFileSystem>(std::move(resourceFS), userGameDir, true);
            } else {
                m_configFS = std::make_unique<IO::WritableDiskFileSystem>(userGameDir, true);
            }
        }

        void GameFactory::loadGameConfigs() {
            const auto configFiles = m_configFS->findItemsRecursively(IO::Path(""), IO::FileNameMatcher("GameConfig.cfg"));
            for (const auto& configFilePath : configFiles) {
                loadGameConfig(configFilePath);
            }

            StringUtils::sortCaseSensitive(m_names);
        }

        void GameFactory::loadGameConfig(const IO::Path& path) {
            GameConfig config;
            try {
                const auto configFile = m_configFS->openFile(path);
                const auto absolutePath = m_configFS->makeAbsolute(path);
                IO::GameConfigParser parser(configFile->begin(), configFile->end(), absolutePath);
                config = parser.parse();
            } catch (const Exception& e) {
                throw GameException("Could not load game configuration '" + path.asString() + "': " + String(e.what()));
            }

            loadCompilationConfig(config);
            loadGameEngineConfig(config);
            
            // sneak in the brush content type for tutorial brushes
            const auto flag = 1 << config.brushContentTypes().size();
            config.addBrushContentType(Tutorial::createTutorialBrushContentType(flag));
            
            m_configs.insert(std::make_pair(config.name(), config));
            m_names.push_back(config.name());
            
            const auto gamePathPrefPath = IO::Path("Games") + IO::Path(config.name()) + IO::Path("Path");
            m_gamePaths.insert(std::make_pair(config.name(), Preference<IO::Path>(gamePathPrefPath, IO::Path())));
            
            const auto defaultEnginePrefPath = IO::Path("Games") + IO::Path(config.name()) + IO::Path("Default Engine");
            m_defaultEngines.insert(std::make_pair(config.name(), Preference<IO::Path>(defaultEnginePrefPath, IO::Path())));
        }

        void GameFactory::loadCompilationConfig(GameConfig& gameConfig) {
            const auto path = IO::Path(gameConfig.name()) + IO::Path("CompilationProfiles.cfg");
            try {
                if (m_configFS->fileExists(path)) {
                    const auto profilesFile = m_configFS->openFile(path);
                    IO::CompilationConfigParser parser(profilesFile->begin(), profilesFile->end(), m_configFS->makeAbsolute(path));
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
                    IO::GameEngineConfigParser parser(profilesFile->begin(), profilesFile->end(), m_configFS->makeAbsolute(path));
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
