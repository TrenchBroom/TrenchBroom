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

#ifndef TrenchBroom_GameUtils_h
#define TrenchBroom_GameUtils_h

#include "Assets/AssetTypes.h"
#include "IO/Path.h"
#include "Model/EntityProperties.h"

class Color;

namespace TrenchBroom {
    namespace Assets {
        class Palette;
    }
    
    namespace IO {
        class GameFileSystem;
    }
    
    namespace Model {
        class Map;
        
        IO::Path::List extractTexturePaths(const Map* map, const PropertyKey& key);
        Assets::EntityDefinitionList loadEntityDefinitions(const IO::Path& path, const Color& defaultEntityColor);
        IO::Path extractEntityDefinitionFile(const Map* map, const IO::Path& defaultFile);
        Assets::EntityModel* loadModel(const IO::GameFileSystem& gameFS, const Assets::Palette& palette, const IO::Path& path);
    }
}

#endif
