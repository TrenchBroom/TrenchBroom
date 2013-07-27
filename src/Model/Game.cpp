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

namespace TrenchBroom {
    namespace Model {
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
