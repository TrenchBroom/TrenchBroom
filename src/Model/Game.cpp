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

#include "Exceptions.h"
#include "Logger.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/FaceTextureCollection.h"
#include "IO/FileSystem.h"
#include "IO/IOUtils.h"
#include "Model/QuakeGame.h"
#include "Model/Quake2Game.h"
#include "Model/Hexen2Game.h"

namespace TrenchBroom {
    namespace Model {
        const String Game::GameNames[] = {"Quake", "Quake 2", "Hexen 2"};
        const size_t Game::GameCount = 3;
        
        GamePtr Game::game(const String& gameName, Logger* logger) {
            for (size_t i = 0; i < GameCount; ++i)
                if (gameName == GameNames[i])
                    return game(i, logger);
            if (logger != NULL)
                logger->error("Unknown game type: " + gameName);
            return GamePtr();
        }
        
        GamePtr Game::game(const size_t gameIndex, Logger* logger) {
            PreferenceManager& prefs = PreferenceManager::instance();
            switch (gameIndex) {
                case 0:
                    return QuakeGame::newGame(prefs.getString(Preferences::QuakePath), prefs.getColor(Preferences::UndefinedEntityColor), logger);
                case 1:
                    return Quake2Game::newGame(prefs.getString(Preferences::Quake2Path), prefs.getColor(Preferences::UndefinedEntityColor), logger);
                case 2:
                    return Hexen2Game::newGame(prefs.getString(Preferences::Hexen2Path), prefs.getColor(Preferences::UndefinedEntityColor), logger);
                default:
                    if (logger != NULL)
                        logger->error("Game index out of bounds: %i", gameIndex);
                    return GamePtr();
            }
        }
        
        GamePtr Game::detectGame(const IO::Path& path, Logger* logger) {
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
            return game(nameStr, logger);
        }
        
        Map* Game::loadMap(const BBox3& worldBounds, const IO::Path& path) const {
            return doLoadMap(worldBounds, path);
        }

        IO::Path::List Game::extractTexturePaths(const Map* map) const {
            return doExtractTexturePaths(map);
        }

        Assets::FaceTextureCollection* Game::loadTextureCollection(const IO::Path& path) const {
            try {
                return doLoadTextureCollection(path);
            } catch (ResourceNotFoundException e) {
                if (m_logger != NULL)
                    m_logger->error("Error loading texture collection %s: %s", path.asString().c_str(), e.what());
                return NULL;
            }
        }

        void Game::uploadTextureCollection(Assets::FaceTextureCollection* collection) const {
            try {
                doUploadTextureCollection(collection);
            } catch (ResourceNotFoundException e) {
                if (m_logger != NULL)
                    m_logger->error("Error uploading texture collection %s: %s", collection->path().asString().c_str(), e.what());
            }
        }

        Assets::EntityDefinitionList Game::loadEntityDefinitions(const IO::Path& path) const {
            try {
                return doLoadEntityDefinitions(path);
            } catch (ParserException e) {
                if (m_logger != NULL)
                    m_logger->error("Error loading entity definitions from %s: %s", path.asString().c_str(), e.what());
                return Assets::EntityDefinitionList();
            } catch (GameException e) {
                if (m_logger != NULL)
                    m_logger->error("Error loading entity definitions from %s: %s", path.asString().c_str(), e.what());
                return Assets::EntityDefinitionList();
            }
        }

        IO::Path Game::defaultEntityDefinitionFile() const {
            return doDefaultEntityDefinitionFile();
        }

        IO::Path Game::extractEntityDefinitionFile(const Map* map) const {
            return doExtractEntityDefinitionFile(map);
        }

        Assets::EntityModel* Game::loadModel(const IO::Path& path) const {
            try {
                return doLoadModel(path);
            } catch (ResourceNotFoundException e) {
                if (m_logger != NULL)
                    m_logger->error("Error loading model %s: %s", path.asString().c_str(), e.what());
                return NULL;
            }
        }

        Game::Game(Logger* logger) :
        m_logger(logger) {}

        Game::~Game() {}
    }
}
