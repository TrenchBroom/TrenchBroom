/*
 Copyright (C) 2010-2016 Kristian Duske
 
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
#include "IO/DiskFileSystem.h"
#include "IO/FileMatcher.h"
#include "IO/FileSystem.h"
#include "IO/GameConfigParser.h"
#include "IO/IOUtils.h"
#include "IO/Path.h"
#include "IO/SystemPaths.h"
#include "Model/Game.h"
#include "Model/GameImpl.h"

#include "Exceptions.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
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

        GamePtr GameFactory::createGame(const String& gameName) const {
            return GamePtr(new GameImpl(gameConfig(gameName), gamePath(gameName)));
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
            if (it == m_gamePaths.end())
                throw GameException("Unknown game: " + gameName);
            Preference<IO::Path>& pref = it->second;
            return PreferenceManager::instance().get(pref);
        }
        
        void GameFactory::setGamePath(const String& gameName, const IO::Path& gamePath) {
            GamePathMap::iterator it = m_gamePaths.find(gameName);
            if (it == m_gamePaths.end())
                throw GameException("Unknown game: " + gameName);
            Preference<IO::Path>& pref = it->second;
            PreferenceManager::instance().set(pref, gamePath);
        }

        bool GameFactory::isGamePathPreference(const String& gameName, const IO::Path& prefPath) const {
            GamePathMap::iterator it = m_gamePaths.find(gameName);
            if (it == m_gamePaths.end())
                throw GameException("Unknown game: " + gameName);
            Preference<IO::Path>& pref = it->second;
            return pref.path() == prefPath;
        }

        std::pair<String, MapFormat::Type> GameFactory::detectGame(const IO::Path& path) const {
            if (path.isEmpty() || !IO::Disk::fileExists(IO::Disk::fixPath(path)))
                return std::make_pair("", MapFormat::Unknown);
            
            const IO::OpenFile file(path, false);
            const String gameName = IO::readGameComment(file.file());
            const String formatName = IO::readFormatComment(file.file());
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
            
            IO::Path::List::const_iterator it, end;
            for (it = configFiles.begin(), end = configFiles.end(); it != end; ++it) {
                const IO::Path& configFilePath = *it;
                loadGameConfig(configFilePath);
            }
            
            StringUtils::sortCaseSensitive(m_names);
        }

        void GameFactory::loadGameConfig(const IO::Path& path) {
            try {
                const IO::MappedFile::Ptr configFile = m_configFS.openFile(path);
                IO::GameConfigParser parser(configFile->begin(), configFile->end(), m_configFS.makeAbsolute(path));
                GameConfig config = parser.parse();
                loadCompilationConfig(config);

                m_configs.insert(std::make_pair(config.name(), config));
                m_names.push_back(config.name());

                const IO::Path prefPath = IO::Path("Games") + IO::Path(config.name()) + IO::Path("Path");
                m_gamePaths.insert(std::make_pair(config.name(), Preference<IO::Path>(prefPath, IO::Path(""))));
            } catch (const Exception& e) {
                throw GameException("Cannot load game configuration '" + path.asString() + "': " + String(e.what()));
            }
        }

        void GameFactory::loadCompilationConfig(GameConfig& gameConfig) {
            const IO::Path profilesPath = IO::Path(gameConfig.name()) + IO::Path("CompilationProfiles.cfg");
            if (m_configFS.fileExists(profilesPath)) {
                const IO::MappedFile::Ptr profilesFile = m_configFS.openFile(profilesPath);
                IO::CompilationConfigParser parser(profilesFile->begin(), profilesFile->end(), m_configFS.makeAbsolute(profilesPath));
                gameConfig.setCompilationConfig(parser.parse());
            }
        }

        const GameConfig& GameFactory::gameConfig(const String& name) const {
            ConfigMap::const_iterator cIt = m_configs.find(name);
            if (cIt == m_configs.end())
                throw GameException("Unknown game: " + name);
            return cIt->second;
        }
    }
}
