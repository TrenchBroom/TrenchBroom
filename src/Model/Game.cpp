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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Game.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "IO/FileSystem.h"
#include "IO/IOUtils.h"
#include "Model/QuakeGame.h"
#include "Model/Quake2Game.h"

namespace TrenchBroom {
    namespace Model {
        const String Game::GameNames[] = {"Quake", "Quake 2"};
        const size_t Game::GameCount = 2;
        
        GamePtr Game::game(const String& gameName) {
            for (size_t i = 0; i < GameCount; ++i)
                if (gameName == GameNames[i])
                    return game(i);
            return GamePtr();
        }
        
        GamePtr Game::game(const size_t gameIndex) {
            PreferenceManager& prefs = PreferenceManager::instance();
            switch (gameIndex) {
                case 0:
                    return QuakeGame::newGame(prefs.getString(Preferences::QuakePath), prefs.getColor(Preferences::UndefinedEntityColor));
                case 1:
                    return Quake2Game::newGame(prefs.getString(Preferences::Quake2Path), prefs.getColor(Preferences::UndefinedEntityColor));
                default:
                    return GamePtr();
            }
        }
        
        GamePtr Game::detectGame(const IO::Path& path) {
            IO::FileSystem fs;
            if (!fs.exists(path) || fs.isDirectory(path))
                return GamePtr();
            
            IO::MappedFile::Ptr file = fs.mapFile(path, std::ios::in);
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
            return game(nameStr);
        }
        
        Map* Game::loadMap(const BBox3& worldBounds, const IO::Path& path) const {
            return doLoadMap(worldBounds, path);
        }

        IO::Path::List Game::extractTexturePaths(const Map* map) const {
            return doExtractTexturePaths(map);
        }

        Assets::FaceTextureCollection* Game::loadTextureCollection(const IO::Path& path) const {
            return doLoadTextureCollection(path);
        }

        void Game::uploadTextureCollection(Assets::FaceTextureCollection* collection) const {
            doUploadTextureCollection(collection);
        }

        Assets::EntityDefinitionList Game::loadEntityDefinitions(const IO::Path& path) const {
            return doLoadEntityDefinitions(path);
        }

        IO::Path Game::defaultEntityDefinitionFile() const {
            return doDefaultEntityDefinitionFile();
        }

        IO::Path Game::extractEntityDefinitionFile(const Map* map) const {
            return doExtractEntityDefinitionFile(map);
        }

        Assets::EntityModel* Game::loadModel(const IO::Path& path) const {
            return doLoadModel(path);
        }

        Game::Game() {}

        Game::~Game() {}
    }
}
