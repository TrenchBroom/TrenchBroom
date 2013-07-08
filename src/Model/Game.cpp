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
        Map::Ptr Game::loadMap(const IO::Path& path) const {
            return doLoadMap(path);
        }

        IO::Path::List Game::extractTexturePaths(Model::Map::Ptr map) const {
            return doExtractTexturePaths(map);
        }

        TextureCollection::Ptr Game::loadTextureCollection(const IO::Path& path) const {
            return doLoadTextureCollection(path);
        }

        void Game::uploadTextureCollection(TextureCollection::Ptr collection) const {
            doUploadTextureCollection(collection);
        }

        Game::Game() {}

        Game::~Game() {}
    }
}
