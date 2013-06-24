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

#include "QuakeGame.h"

#include "IO/FileSystem.h"
#include "IO/Path.h"
#include "IO/QuakeMapParser.h"
#include "IO/WadTextureLoader.h"

namespace TrenchBroom {
    namespace Model {
        const BBox3 QuakeGame::WorldBounds = BBox3(Vec3(-16384.0, -16384.0, -16384.0),
                                                   Vec3(+16384.0, +16384.0, +16384.0));
        
        Game::Ptr QuakeGame::newGame() {
            return Ptr(new QuakeGame());
        }

        Map::Ptr QuakeGame::doLoadMap(const IO::Path& path) const {
            IO::FileSystem fs;
            IO::MappedFile::Ptr file = fs.mapFile(path, std::ios::in);
            IO::QuakeMapParser parser(file->begin(), file->end());
            return parser.parseMap(WorldBounds);
        }

        TextureCollection::Ptr QuakeGame::doLoadTextureCollection(const IO::Path& path) const {
            IO::WadTextureLoader loader;
            return loader.loadTextureCollection(path);
        }
    }
}
