/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "PreferenceManager.h"
#include "Preferences.h"
#include "IO/GameConfigParser.h"
#include "IO/DiskFileSystem.h"
#include "IO/FileSystem.h"
#include "IO/IOUtils.h"
#include "IO/Path.h"
#include "IO/SystemPaths.h"
#include "Model/Game.h"
#include "Model/GameImpl.h"

#include "Exceptions.h"

namespace TrenchBroom {
    namespace Model {
        const GameFactory& GameFactory::instance() {
            static const GameFactory instance;
            return instance;
        }
        
        StringList GameFactory::gameList() const {
            StringList names;
            ConfigMap::const_iterator it, end;
            for (it = m_configs.begin(), end = m_configs.end(); it != end; ++it)
                names.push_back(it->first);
            StringUtils::sortCaseInsensitive(names);
            return names;
        }
        
        GamePtr GameFactory::createDefaultGame() const {
            if (m_configs.empty())
                throw GameException("Cannot create default game because no game configs have been loaded");
            
            PreferenceManager& prefs = PreferenceManager::instance();
            const String defaultGame = prefs.get(Preferences::DefaultGame);
            return createGame(defaultGame);
        }

        GamePtr GameFactory::createGame(const String& name) const {
            ConfigMap::const_iterator cIt = m_configs.find(name);
            if (cIt == m_configs.end())
                throw GameException("Unknown game: " + name);
            const GameConfig& config = cIt->second;
            
            PreferenceManager& prefs = PreferenceManager::instance();
            const StringMap& gamePaths = prefs.get(Preferences::GamePaths);
            StringMap::const_iterator pIt = gamePaths.find(name);
            const IO::Path gamePath(pIt == gamePaths.end() ? "" : pIt->second);
            
            return GamePtr(new GameImpl(config, gamePath));
        }
        
        GamePtr GameFactory::detectGame(const IO::Path& path) const {
            if (path.isEmpty() || !IO::Disk::fileExists(IO::Disk::fixPath(path)))
                return GamePtr();
            
            const IO::MappedFile::Ptr file = IO::Disk::openFile(IO::Disk::fixPath(path));
            if (file == NULL)
                return GamePtr();
            
            // we will try to detect a comment in the beginning of the file formatted like so:
            // // Game: <string>\n
            
            // where <string> is the name of a game in the GameName array.
            if (file->end() - file->begin() < 9)
                return GamePtr();
            
            const char* cursor = file->begin();
            char comment[10];
            comment[9] = 0;
            IO::readBytes(cursor, comment, 9);
            
            const String commentStr(comment);
            if (commentStr != "// Game: ")
                return GamePtr();
            
            StringStream name;
            while (cursor < file->end() && *cursor != '\n')
                name << *(cursor++);
            
            const String nameStr = StringUtils::trim(name.str());
            return createGame(nameStr);
        }

        GameFactory::GameFactory() {
            loadGameConfigs();
        }
        
        void GameFactory::loadGameConfigs() {
            const IO::Path resourceDir = IO::SystemPaths::resourceDirectory();
            const IO::DiskFileSystem fs(resourceDir);
            const IO::Path::List configFiles = fs.findItems(IO::Path("games"), IO::FileSystem::ExtensionMatcher("cfg"));
            
            IO::Path::List::const_iterator it, end;
            for (it = configFiles.begin(), end = configFiles.end(); it != end; ++it) {
                const IO::Path& configFilePath = *it;
                loadGameConfig(fs, configFilePath);
            }
        }

        void GameFactory::loadGameConfig(const IO::FileSystem& fs, const IO::Path& path) {
            try {
                const IO::MappedFile::Ptr configFile = fs.openFile(path);
                IO::GameConfigParser parser(configFile->begin(), configFile->end());
                GameConfig config = parser.parse();
                m_configs[config.name()] = config;
            } catch (const Exception& e) {
                throw GameException("Cannot load game configuration '" + path.asString() + "': " + String(e.what()));
            }
        }
    }
}
