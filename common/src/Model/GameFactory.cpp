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
        
        const StringList& GameFactory::fileFormats(const String& gameName) const {
            const GameConfig& config = gameConfig(gameName);
            return config.fileFormats();
        }

        IO::Path GameFactory::iconPath(const String& gameName) const {
            const GameConfig& config = gameConfig(gameName);
            return config.findConfigFile(config.icon());
        }
        
        IO::Path GameFactory::gamePath(const String& gameName) const {
            GamePathMap::iterator it = m_gamePaths.find(gameName);
            if (it == std::end(m_gamePaths))
                throw GameException("Unknown game: " + gameName);
            Preference<IO::Path>& pref = it->second;
            return PreferenceManager::instance().get(pref);
        }
        
        bool GameFactory::setGamePath(const String& gameName, const IO::Path& gamePath) {
            GamePathMap::iterator it = m_gamePaths.find(gameName);
            if (it == std::end(m_gamePaths))
                throw GameException("Unknown game: " + gameName);
            Preference<IO::Path>& pref = it->second;
            return PreferenceManager::instance().set(pref, gamePath);
        }

        bool GameFactory::isGamePathPreference(const String& gameName, const IO::Path& prefPath) const {
            GamePathMap::iterator it = m_gamePaths.find(gameName);
            if (it == std::end(m_gamePaths))
                throw GameException("Unknown game: " + gameName);
            Preference<IO::Path>& pref = it->second;
            return pref.path() == prefPath;
        }
        
        GameConfig& GameFactory::gameConfig(const String& name) {
            ConfigMap::iterator cIt = m_configs.find(name);
            if (cIt == std::end(m_configs))
                throw GameException("Unknown game: " + name);
            return cIt->second;
        }
        
        const GameConfig& GameFactory::gameConfig(const String& name) const {
            ConfigMap::const_iterator cIt = m_configs.find(name);
            if (cIt == std::end(m_configs))
                throw GameException("Unknown game: " + name);
            return cIt->second;
        }

        std::pair<String, MapFormat::Type> GameFactory::detectGame(const IO::Path& path) const {
            if (path.isEmpty() || !IO::Disk::fileExists(IO::Disk::fixPath(path)))
                return std::make_pair("", MapFormat::Unknown);
            
            IO::OpenStream open(path, false);
            const String gameName = IO::readGameComment(open.stream);
            const String formatName = IO::readFormatComment(open.stream);
            const MapFormat::Type format = mapFormat(formatName);
            if (gameName.empty() || format == MapFormat::Unknown)
                return std::make_pair("", MapFormat::Unknown);
            
            return std::make_pair(gameName, format);
        }

        GameFactory::GameFactory() {
            initializeFileSystem();
            loadGameConfigs();
        }
        
        void GameFactory::initializeFileSystem() {
            const IO::Path resourceGameDir = IO::SystemPaths::resourceDirectory() + IO::Path("games");
            if (IO::Disk::directoryExists(resourceGameDir))
                m_configFS.addReadableFileSystem(new IO::DiskFileSystem(resourceGameDir));

            const IO::Path userGameDir = IO::SystemPaths::userDataDirectory() + IO::Path("games");
            m_configFS.addWritableFileSystem(new IO::WritableDiskFileSystem(userGameDir, true));
        }

        void GameFactory::loadGameConfigs() {
            const IO::Path::List configFiles = m_configFS.findItems(IO::Path(""), IO::FileExtensionMatcher("cfg"));
            
            for (const IO::Path& configFilePath : configFiles)
                loadGameConfig(configFilePath);
            
            StringUtils::sortCaseSensitive(m_names);
        }

        void GameFactory::loadGameConfig(const IO::Path& path) {
            GameConfig config;
            try {
                const IO::MappedFile::Ptr configFile = m_configFS.openFile(path);
                IO::GameConfigParser parser(configFile->begin(), configFile->end(), m_configFS.makeAbsolute(path));
                config = parser.parse();
            } catch (const Exception& e) {
                throw GameException("Cannot load game configuration '" + path.asString() + "': " + String(e.what()));
            }

            loadCompilationConfig(config);
            loadGameEngineConfig(config);
            
            // sneak in the brush content type for tutorial brushes
            const BrushContentType::FlagType flag = 1 << config.brushContentTypes().size();
            config.addBrushContentType(Tutorial::createTutorialBrushContentType(flag));
            
            m_configs.insert(std::make_pair(config.name(), config));
            m_names.push_back(config.name());
            
            const IO::Path gamePathPrefPath = IO::Path("Games") + IO::Path(config.name()) + IO::Path("Path");
            m_gamePaths.insert(std::make_pair(config.name(), Preference<IO::Path>(gamePathPrefPath, IO::Path())));
            
            const IO::Path defaultEnginePrefPath = IO::Path("Games") + IO::Path(config.name()) + IO::Path("Default Engine");
            m_defaultEngines.insert(std::make_pair(config.name(), Preference<IO::Path>(defaultEnginePrefPath, IO::Path())));
        }

        void GameFactory::loadCompilationConfig(GameConfig& gameConfig) {
            const IO::Path profilesPath = IO::Path(gameConfig.name()) + IO::Path("CompilationProfiles.cfg");
            try {
                if (m_configFS.fileExists(profilesPath)) {
                    const IO::MappedFile::Ptr profilesFile = m_configFS.openFile(profilesPath);
                    IO::CompilationConfigParser parser(profilesFile->begin(), profilesFile->end(), m_configFS.makeAbsolute(profilesPath));
                    gameConfig.setCompilationConfig(parser.parse());
                }
            } catch (const Exception& e) {
                throw GameException("Cannot load compilation configuration '" + profilesPath.asString() + "': " + String(e.what()));
            }
        }
        
        void GameFactory::loadGameEngineConfig(GameConfig& gameConfig) {
            const IO::Path profilesPath = IO::Path(gameConfig.name()) + IO::Path("GameEngineProfiles.cfg");
            try {
                if (m_configFS.fileExists(profilesPath)) {
                    const IO::MappedFile::Ptr profilesFile = m_configFS.openFile(profilesPath);
                    IO::GameEngineConfigParser parser(profilesFile->begin(), profilesFile->end(), m_configFS.makeAbsolute(profilesPath));
                    gameConfig.setGameEngineConfig(parser.parse());
                }
            } catch (const Exception& e) {
                throw GameException("Cannot load game engine configuration '" + profilesPath.asString() + "': " + String(e.what()));
            }
        }
        
        void GameFactory::writeCompilationConfigs() {
            for (const auto& entry : m_configs) {
                const GameConfig& gameConfig = entry.second;
                writeCompilationConfig(gameConfig);
            }
        }
        
        void GameFactory::writeCompilationConfig(const GameConfig& gameConfig) {
            StringStream stream;
            IO::CompilationConfigWriter writer(gameConfig.compilationConfig(), stream);
            writer.writeConfig();
            
            const IO::Path profilesPath = IO::Path(gameConfig.name()) + IO::Path("CompilationProfiles.cfg");
            m_configFS.createFile(profilesPath, stream.str());
        }

        void GameFactory::writeGameEngineConfigs() {
            for (const auto& entry : m_configs) {
                const GameConfig& gameConfig = entry.second;
                writeGameEngineConfig(gameConfig);
            }
        }
        
        void GameFactory::writeGameEngineConfig(const GameConfig& gameConfig) {
            StringStream stream;
            IO::GameEngineConfigWriter writer(gameConfig.gameEngineConfig(), stream);
            writer.writeConfig();
            
            const IO::Path profilesPath = IO::Path(gameConfig.name()) + IO::Path("GameEngineProfiles.cfg");
            m_configFS.createFile(profilesPath, stream.str());
        }
    }
}
